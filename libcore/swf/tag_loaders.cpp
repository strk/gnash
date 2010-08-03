// tag_loaders.cpp: SWF tags loaders, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // HAVE_ZLIB_H, USE_SWFTREE
#endif

#include "tag_loaders.h"

#include <set>
#include <limits>
#include <cassert>
#include <utility> // for std::make_pair
#include <boost/static_assert.hpp>

#include "IOChannel.h" // for StreamAdapter inheritance
#include "utility.h"
#include "action_buffer.h"
#include "Font.h"
#include "log.h"
#include "SWFStream.h"
#include "GnashImage.h"
#include "zlib_adapter.h"
#include "sprite_definition.h"
#include "MovieClip.h"
#include "SWFMovieDefinition.h"
#include "SWF.h"
#include "swf/TagLoadersTable.h"
#include "URL.h"
#include "GnashException.h"
#include "swf/DefineVideoStreamTag.h"
#include "sound_definition.h"
#include "SoundInfo.h"
#include "MediaHandler.h"
#include "SimpleBuffer.h"
#include "sound_handler.h"
#include "MovieFactory.h"
#include "RunResources.h"
#include "Renderer.h"
#include "Movie.h"
#include "CachedBitmap.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

namespace gnash {

namespace SWF {


/// Anonymous namespace
namespace {

/// Provide an IOChannel interface around a SWFStream for reading 
/// embedded image data
class StreamAdapter : public IOChannel
{
    SWFStream& s;
    std::streampos startPos;
    std::streampos endPos;
    std::streampos currPos;

    StreamAdapter(SWFStream& str, std::streampos maxPos)
        :
        s(str),
        startPos(s.tell()),
        endPos(maxPos),
        currPos(startPos)
    {
        assert(endPos > startPos);
    }

    virtual ~StreamAdapter()
    {
    }

    virtual std::streamsize read(void* dst, std::streamsize bytes) 
    {
        std::streamsize bytesLeft = endPos - currPos;
        if ( bytesLeft < bytes )
        {
            if ( ! bytesLeft ) return 0;
            //log_debug("Requested to read past end of stream range");
            bytes = bytesLeft;
        }
        std::streamsize actuallyRead = s.read(static_cast<char*>(dst), bytes);
        currPos += actuallyRead;
        return actuallyRead;
    }

    virtual void go_to_end()
    {
        s.seek(endPos);
    }

    virtual bool eof() const
    {
        return (currPos == endPos);
    }

    // Return false on failure, true on success
    virtual bool seek(std::streampos pos)
    {
        // SWFStream::seek() returns true on success
        if (s.seek(pos))
        {
            currPos = pos;
            return true;
        }
        return false;
    }

    virtual size_t size() const
    {
        return (endPos - startPos);
    }

    virtual std::streampos tell() const
    {
        return currPos;
    }
    
    virtual bool bad() const
    {
        // Is there any point in this?
        return false;
    }

public:

    /// Get an IOChannel from a gnash::SWFStream
    static std::auto_ptr<IOChannel> getFile(SWFStream& str,
            unsigned long endPos)
    {
        std::auto_ptr<IOChannel> ret (new StreamAdapter(str, endPos));
        return ret;
    }
};

} // anonymous namespace


//
// Tag loaders
//


// Silently ignore the contents of this tag.
void null_loader(SWFStream& /*in*/, TagType /*tag*/, movie_definition& /*m*/,
        const RunResources& /*r*/)
{
}

// Label the current frame of m with the name from the SWFStream.
void
frame_label_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& /*r*/)
{
    assert(tag == SWF::FRAMELABEL); // 43

    std::string name;
    in.read_string(name);

    m.add_frame_name(name);

    // FIXME: support SWF6 "named anchors"
    //
    // If SWF version is >= 6 check the byte after terminating NULL
    // if it is 1 this label can be accessed by #name and it's
    // entrance sets the browser URL with anchor appended
    //
    // To avoid relying on SWFStream::tell (see task #5838)
    // we should add a new method to that class
    // (ie: SWFStream::current_tag_length)
    //
    // See server/sample/test_clipping_layer.swf for a testcase.
    //
    size_t end_tag = in.get_tag_end_position();
    size_t curr_pos = in.tell();
    if ( end_tag != curr_pos )
    {
    if ( end_tag == curr_pos + 1 )
    {
        log_unimpl(_("anchor-labeled frame not supported"));
    }
    else
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("frame_label_loader end position %d, "
                   "read up to %d"),
                 end_tag, curr_pos);
        );
    }
    }
}

// Load JPEG compression tables that can be used to load
// images further along in the SWFStream.
void
jpeg_tables_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& /*r*/)
{
    //GNASH_REPORT_FUNCTION;
    assert(tag == SWF::JPEGTABLES);

    IF_VERBOSE_PARSE
    (
        log_parse(_("  jpeg_tables_loader"));
    );

    const std::streampos currPos = in.tell();
    const std::streampos endPos = in.get_tag_end_position();

    assert(endPos >= currPos);

    const unsigned long jpegHeaderSize = endPos - currPos;

    if ( ! jpegHeaderSize )
    {
        log_debug(_("No bytes to read in JPEGTABLES tag at offset %d"),
                currPos);
    }

    std::auto_ptr<JpegImageInput> input;

    try
    {
    // NOTE: we cannot limit input SWFStream here as the same jpeg::input
    // instance will be used for reading subsequent DEFINEBITS and similar
    // tags, which are *different* tags, so have different boundaries !!
    //
    // Anyway the actual reads are limited to currently opened tag as 
    // of gnash::SWFStream::read(), so this is not a problem.
    //
        boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                    std::numeric_limits<std::streamsize>::max()).release());
        //  transfer ownership to the JpegImageInput
        input = JpegImageInput::createSWFJpeg2HeaderOnly(ad, jpegHeaderSize);

    }
    catch (std::exception& e)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Error creating header-only jpeg2 input: %s",
                e.what());
        );
        return;
    }

    log_debug("Setting jpeg loader to %p", (void*)input.get());
    m.set_jpeg_loader(input);
}


// A JPEG image without included tables; those should be in an
// existing JpegImageInput object stored in the movie.
void
define_bits_jpeg_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINEBITS); // 6

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    if (m.getBitmap(id))
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITS: Duplicate id (%d) for bitmap DisplayObject "
                "- discarding it"), id);
        );
        return;
    }

    // Read the image data.
    JpegImageInput* j_in = m.get_jpeg_loader();
    if ( ! j_in )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITS: No jpeg loader registered in movie "
                "definition - discarding bitmap DisplayObject %d"), id);
        );
        return;
    }

    j_in->discardPartialBuffer();
    
    std::auto_ptr<GnashImage> im;
    try
    {
        im = JpegImageInput::readSWFJpeg2WithTables(*j_in);
    }
    catch (std::exception& e)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror("Error reading jpeg2 with headers for DisplayObject "
            "id %d: %s", id, e.what());
        );
        return;
    }
    
    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(im);

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
}


void
define_bits_jpeg2_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINEBITSJPEG2); // 21

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  define_bits_jpeg2_loader: charid = %d pos = %ld"),
          id, in.tell());
    );

    
    if ( m.getBitmap(id) )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITSJPEG2: Duplicate id (%d) for bitmap "
                "DisplayObject - discarding it"), id);
        );
        return;
    }

    char buf[3];
    if (in.read(buf, 3) < 3) {
        log_swferror(_("DEFINEBITS data too short to read type header"));
        return;
    }
    in.seek(in.tell() - 3);

    FileType ft = GNASH_FILETYPE_JPEG;  

    // Check the data type. The pp version 9,0,115,0 supports PNG and GIF
    // in DefineBits tags, though it is not documented. The version makes
    // no difference.
    if (std::equal(buf, buf + 3, "\x89PN")) {
        ft = GNASH_FILETYPE_PNG;
    }
    else if (std::equal(buf, buf + 3, "GIF")) {
        ft = GNASH_FILETYPE_GIF;
    }

    // Read the image data.
    boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                in.get_tag_end_position()).release() );

    std::auto_ptr<GnashImage> im (ImageInput::readImageData(ad, ft));

    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(im);

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);

}


#ifdef HAVE_ZLIB_H
void inflate_wrapper(SWFStream& in, void* buffer, int buffer_bytes)
    // Wrapper function -- uses Zlib to uncompress in_bytes worth
    // of data from the input file into buffer_bytes worth of data
    // into *buffer.
{
    assert(buffer);
    assert(buffer_bytes > 0);

    z_stream d_stream; /* decompression SWFStream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = 0;
    d_stream.avail_in = 0;

    d_stream.next_out = static_cast<Byte*>(buffer);
    d_stream.avail_out = static_cast<uInt>(buffer_bytes);

    int err = inflateInit(&d_stream);
    if (err != Z_OK) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("inflate_wrapper() inflateInit() returned %d (%s)"),
                err, d_stream.msg);
        );
        return;
    }

    const size_t CHUNKSIZE = 256;

    boost::uint8_t buf[CHUNKSIZE];
    unsigned long endTagPos = in.get_tag_end_position();

    for (;;)
    {
        unsigned int chunkSize = CHUNKSIZE;
        assert(in.tell() <= endTagPos);
        unsigned int availableBytes =  endTagPos - in.tell();
        if ( availableBytes < chunkSize )
        {
            if ( ! availableBytes )
            {
                // nothing more to read
                IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("inflate_wrapper(): no end of zstream found "
                        "within swf tag boundaries"));
                );
                break;
            }
            chunkSize = availableBytes;
        }
    
        BOOST_STATIC_ASSERT(sizeof(char) == sizeof(boost::uint8_t));

        // Fill the buffer    
        in.read(reinterpret_cast<char*>(buf), chunkSize);
        d_stream.next_in = &buf[0];
        d_stream.avail_in = chunkSize;

        err = inflate(&d_stream, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END)
        {
            // correct end
            break;
        }

        if (err != Z_OK)
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("inflate_wrapper() inflate() returned %d (%s)"),
                err, d_stream.msg);
            );
            break;
        }
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK)
    {
        log_error(_("inflate_wrapper() inflateEnd() return %d (%s)"),
                err, d_stream.msg);
    }
}
#endif // HAVE_ZLIB_H


// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void
define_bits_jpeg3_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINEBITSJPEG3); // 35

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  define_bits_jpeg3_loader: charid = %d pos = %lx"),
          id, in.tell());
    );

    in.ensureBytes(4);
    boost::uint32_t    jpeg_size = in.read_u32();
    boost::uint32_t    alpha_position = in.tell() + jpeg_size;

#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load jpeg3 image data"));
    return;
#else
    //
    // Read the image data.
    //

    // Read rgb data.
    boost::shared_ptr<IOChannel> ad( StreamAdapter::getFile(in,
                alpha_position).release() );
    std::auto_ptr<ImageRGBA> im = ImageInput::readSWFJpeg3(ad);
    
    /// Failure to read the jpeg.
    if (!im.get()) return;

    // Read alpha channel.
    in.seek(alpha_position);

    const size_t imWidth = im->width();
    const size_t imHeight = im->height();
    const size_t bufferLength = imWidth * imHeight;

    boost::scoped_array<boost::uint8_t> buffer (new boost::uint8_t[bufferLength]);

    inflate_wrapper(in, buffer.get(), bufferLength);

    // TESTING:
    // magical trevor contains this tag
    //  ea8bbad50ccbc52dd734dfc93a7f06a7  6964trev3c.swf
    im->mergeAlpha(buffer.get(), bufferLength);


    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi =
        renderer->createCachedBitmap(static_cast<std::auto_ptr<GnashImage> >(im));

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
#endif
}


void
define_bits_lossless_2_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    // tags 20 || 36
    assert(tag == SWF::DEFINELOSSLESS || tag == SWF::DEFINELOSSLESS2);

    in.ensureBytes(2+2+2+1); // the initial header 

    boost::uint16_t id = in.read_u16();

    // 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    boost::uint8_t bitmap_format = in.read_u8();
    boost::uint16_t width = in.read_u16();
    boost::uint16_t height = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  defbitslossless2: tag = %d, id = %d, "
            "fmt = %d, w = %d, h = %d"),
            tag, id, bitmap_format, width, height);
    );

    if (!width || !height) {
         IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Bitmap DisplayObject %d has a height or width of 0"),
                id);
        );   
        return;  
    }

    // No need to parse any further if it already exists, as we aren't going
    // to add it.
    if (m.getBitmap(id))
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITSLOSSLESS: Duplicate id (%d) "
                           "for bitmap DisplayObject - discarding it"),
                id);
        );
    }

#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load zipped image data"));
    return;
#else

    unsigned short channels;
    std::auto_ptr<GnashImage> image;
    bool alpha = false;

    switch (tag)
    {
        case SWF::DEFINELOSSLESS:
            image.reset(new ImageRGB(width, height));
            channels = 3;
            break;
        case SWF::DEFINELOSSLESS2:
            image.reset(new ImageRGBA(width, height));
            channels = 4;
            alpha = true;
            break;
        default:
            // This is already asserted.
            std::abort();
    }

    unsigned short bytes_per_pixel;
    int colorTableSize = 0;

    switch (bitmap_format)
    {
        case 3:
            bytes_per_pixel = 1;
            in.ensureBytes(1);
            // SWF stores one less than the actual size.
            colorTableSize = in.read_u8() + 1;
            break;

        case 4:
            bytes_per_pixel = 2;
            break;

        case 5:
            bytes_per_pixel = 4;
            break;

        default:
            log_error(_("Unknown bitmap format. Ignoring"));
            return;
    }

    const size_t pitch = (width * bytes_per_pixel + 3) &~ 3;
    const size_t bufSize = colorTableSize * channels + pitch * height;
    boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[bufSize]);

    inflate_wrapper(in, buffer.get(), bufSize);
    assert(in.tell() <= in.get_tag_end_position());

    switch (bitmap_format) {

        case 3:
        {
            // 8-bit data, preceded by a palette.
            boost::uint8_t* colorTable = buffer.get();

            for (int j = 0; j < height; j++)
            {
                boost::uint8_t* inRow = buffer.get() + 
                    colorTableSize * channels + j * pitch;

                boost::uint8_t* outRow = scanline(*image, j);
                for (int i = 0; i < width; i++)
                {
                    boost::uint8_t pixel = inRow[i * bytes_per_pixel];
                    outRow[i * channels + 0] = colorTable[pixel * channels + 0];
                    outRow[i * channels + 1] = colorTable[pixel * channels + 1];
                    outRow[i * channels + 2] = colorTable[pixel * channels + 2];
                    if (alpha) {
                        outRow[i * channels + 3] =
                            colorTable[pixel * channels + 3];
                    }
                }
            }
            break;
        }

        case 4:
            // 16 bits / pixel

            for (int j = 0; j < height; j++)
            {
                boost::uint8_t* inRow = buffer.get() + j * pitch;
                boost::uint8_t* outRow = scanline(*image, j);
                for (int i = 0; i < width; i++)
                {
                    boost::uint16_t pixel = inRow[i * 2] |
                        (inRow[i * 2 + 1] << 8);

                    // How is the data packed??? Whoever wrote this was
                    // just guessing here that it's 565!
                    outRow[i * channels + 0] = (pixel >> 8) & 0xF8;    // red
                    outRow[i * channels + 1] = (pixel >> 3) & 0xFC;    // green
                    outRow[i * channels + 2] = (pixel << 3) & 0xF8;    // blue
 
                    // This was saved to the first byte before, but that
                    // can hardly be correct.
                    // Real examples of this format are rare to non-existent.
                    if (alpha) {
                        outRow[i * channels + 3] = 255;
                    }
                }
            }
            break;

        case 5:
            // Need to re-arrange ARGB into RGB or RGBA.
            for (int j = 0; j < height; j++)
            {
                boost::uint8_t* inRow = buffer.get() + j * pitch;
                boost::uint8_t* outRow = scanline(*image, j);
                const int inChannels = 4;

                for (int i = 0; i < width; ++i)
                {
                    // Copy pixels 1-3.
                    std::copy(&inRow[i * inChannels + 1],
                            &inRow[i * inChannels + 4], &outRow[i * channels]);

                    // Add the alpha channel if necessary.
                    if (alpha) {
                        outRow[i * channels + 3] = inRow[i * 4];
                    }
                }
            }
            break;

    }

    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(image);

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
#endif // HAVE_ZLIB_H

}

// This is like null_loader except it prints a message to nag us to fix it.
void
fixme_loader(SWFStream& /*in*/, TagType tag, movie_definition& /*m*/,
		const RunResources& /*r*/)
{
    static std::set<TagType> warned;
    if (warned.insert(tag).second) {
        log_unimpl(_("  FIXME: tagtype = %d"), tag);
    }
}

// Create and initialize a sprite, and add it to the movie.
void
sprite_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINESPRITE); // 39 - DefineSprite

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  sprite:  char id = %d"), id);
    );

    // A DEFINESPRITE tag as part of a DEFINESPRITE
    // would be a malformed SWF, anyway to be compatible
    // we should still allow that. See bug #22468.
    IF_VERBOSE_MALFORMED_SWF(
        try {
            dynamic_cast<SWFMovieDefinition&>(m);
        }
        catch (std::bad_cast& e) {
            log_swferror(_("Nested DEFINESPRITE tags. Will add to "
                           "top-level DisplayObjects dictionary."));
        }
    );

    // will automatically read the sprite
    sprite_definition* ch = new sprite_definition(m, in, r, id);

    IF_VERBOSE_MALFORMED_SWF(
        if (!ch->get_frame_count()) {
            log_swferror(_("Sprite %d advertise no frames"), id);
        }
    );


    m.addDisplayObject(id, ch);
}


//
// Sound
//

// Common data

/// Sample rate table for DEFINESOUNDHEAD tags
//
/// The value found in the tag is encoded as 2 bits and
/// represent a multiple of 5512.5.
/// NOTE that the first element of this table lacks the .5
/// portion of the actual value. Dunno what consequences 
/// it could have...
///
static int s_sample_rate_table[] = { 5512, 11025, 22050, 44100 };
static unsigned int s_sample_rate_table_len = 4;

// @@ There are two sets of code to decode/expand/byteswap audio here.
// @@ There should be one (search for ADPCM).

// Load a DefineSound tag.
void
define_sound_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINESOUND); // 14

    sound::sound_handler* handler = r.soundHandler();

    in.ensureBytes(2+4+1+4); // DisplayObject id + flags + sample count

    boost::uint16_t    id = in.read_u16();

    media::audioCodecType format = static_cast<media::audioCodecType>(
            in.read_uint(4));
    unsigned sample_rate_in = in.read_uint(2); // see s_sample_rate_table
    if ( sample_rate_in >= s_sample_rate_table_len ) 
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINESOUNDLOADER: sound sample rate %d (expected "
                "0 to %u"), 
            sample_rate_in, s_sample_rate_table_len);
        );
        sample_rate_in = 0;
    }
    int sample_rate = s_sample_rate_table[sample_rate_in];


    bool    sample_16bit = in.read_bit(); 
    bool    stereo = in.read_bit(); 

    unsigned int    sample_count = in.read_u32();

    boost::int16_t delaySeek = 0;

    if (format == media::AUDIO_CODEC_MP3) {
        in.ensureBytes(2);
        delaySeek = in.read_s16();
    }

    IF_VERBOSE_PARSE
    (
        log_parse(_("define sound: ch=%d, format=%s, "
            "rate=%d, 16=%d, stereo=%d, ct=%d, delay=%d"),
              id, format, sample_rate,
              int(sample_16bit), int(stereo), sample_count, delaySeek);
    );

    // If we have a sound_handler, ask it to init this sound.

    if (handler)
    {
        // First it is the amount of data from file,
        // then the amount allocated at *data (it may grow)
        const unsigned dataLength = in.get_tag_end_position() - in.tell();

        // Allocate MediaHandler::getInputPaddingSize() bytes more for the
        // SimpleBuffer 
        size_t allocSize = dataLength;
        media::MediaHandler* mh = r.mediaHandler();
        if (mh) allocSize += mh->getInputPaddingSize();

        std::auto_ptr<SimpleBuffer> data( new SimpleBuffer(allocSize) );

        // dataLength is already calculated from the end of the tag, which
        // should be inside the end of the file. TODO: check that this is 
        // the case.
        const unsigned int bytesRead = in.read(
                reinterpret_cast<char*>(data->data()), dataLength);
        data->resize(bytesRead); // in case it's shorter...
        if (bytesRead < dataLength)
        {
            throw ParserException(_("Tag boundary reported past end of "
                        "SWFStream!"));
        }

        // Store all the data in a SoundInfo object
        std::auto_ptr<media::SoundInfo> sinfo;
        sinfo.reset(new media::SoundInfo(format, stereo, sample_rate,
                    sample_count, sample_16bit, delaySeek));

        // Stores the sounddata in the soundhandler, and the ID returned
        // can be used to starting, stopping and deleting that sound
        int    handler_id = handler->create_sound(data, sinfo);

        if (handler_id >= 0)
        {
        sound_sample* sam = new sound_sample(handler_id, r);
        m.add_sound_sample(id, sam);
        }

    }
    else
    {
        // is this nice to do?
        log_error(_("There is no sound handler currently active, "
            "so DisplayObject with id %d will not be added to "
            "the dictionary"),
              id);
    }
}

// Load a SoundStreamHead(2) tag.
void
sound_stream_head_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{

    // 18 || 45
    assert(tag == SWF::SOUNDSTREAMHEAD || tag == SWF::SOUNDSTREAMHEAD2);

    sound::sound_handler* handler = r.soundHandler();

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

    // FIXME:
    // no DisplayObject id for soundstreams... so we make one up...
    // This only works if there is only one SWFStream in the movie...
    // The right way to do it is to make seperate structures for streams
    // in SWFMovieDefinition.

    // 1 byte for playback info, 1 for SWFStream info, 2 for sample count
    in.ensureBytes(4);

    // These are all unused by current implementation
    int reserved = in.read_uint(4); UNUSED(reserved);

    unsigned int pbSoundRate = in.read_uint(2);
    if ( pbSoundRate >= s_sample_rate_table_len )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror("SOUNDSTREAMHEAD: playback sound rate %d (expected 0 to %d)",
            pbSoundRate, s_sample_rate_table_len);
        );
        pbSoundRate=0;
    }
    int playbackSoundRate = s_sample_rate_table[pbSoundRate];
    bool playbackSound16bit = in.read_bit();
    bool playbackSoundStereo = in.read_bit();

    // These are the used ones
    media::audioCodecType format = static_cast<media::audioCodecType>(in.read_uint(4)); // TODO: check input !
    unsigned int stSoundRate = in.read_uint(2);
    if ( stSoundRate >= s_sample_rate_table_len )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("SOUNDSTREAMHEAD: stream sample rate %d (expected 0 to %u)"),
            stSoundRate, s_sample_rate_table_len);
        );
        stSoundRate=0;
    }
    int streamSoundRate = s_sample_rate_table[stSoundRate];
    bool streamSound16bit = in.read_bit(); 
    bool streamSoundStereo = in.read_bit(); 

    if ( playbackSoundRate != streamSoundRate )
    {
        LOG_ONCE(
            log_unimpl(_("Different stream/playback sound rate (%d/%d). "
                "This seems common in SWF files, so we'll warn only once."),
                streamSoundRate,playbackSoundRate)
        );
    }

    if ( playbackSound16bit != streamSound16bit )
    {
        LOG_ONCE(
            log_unimpl(_("Different stream/playback sample size (%d/%d). "
            "This seems common in SWF files, so we'll warn only once."),
            streamSound16bit ? 16 : 32,
            playbackSound16bit ? 16 : 32 )
        );
    }
    if ( playbackSoundStereo != streamSoundStereo )
    {
        LOG_ONCE(
            log_unimpl(_("Different stream/playback channels (%s/%s). "
            "This seems common in SWF files, so we'll warn only once."),
                streamSoundStereo ? "stereo" : "mono",
                playbackSoundStereo ? "stereo":"mono")
        );
    }

    // checks if this is a new streams header or just one in the row
    if (format == 0 && streamSoundRate == 0 && !streamSound16bit && !streamSoundStereo) return;

    // 2 bytes here
    unsigned int sampleCount = in.read_u16();

    if ( ! sampleCount )
    {
    // this seems common too, we'd need to reproduce with a custom
    // testcase to really tell if it's a problem or not...
    IF_VERBOSE_MALFORMED_SWF(
        LOG_ONCE( log_swferror(_("No samples advertised for sound stream, pretty common so will warn only once")) );
    );
    }

    int latency = 0;
    if (format == media::AUDIO_CODEC_MP3)
    {
        try
        {
            in.ensureBytes(2);
            latency = in.read_s16(); 
        }
        catch (ParserException& ex)
        {
            // See https://savannah.gnu.org/bugs/?21729 for an example 
            // triggering this.
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("MP3 sound stream lacks a 'latency' field");
            );
        }
    }

    // Check if we did read everything in this tag...
    unsigned long curPos=in.tell(), endTag=in.get_tag_end_position();
    if ( curPos < endTag ) {
        log_unimpl("SOUNDSTREAMHEAD contains %d unparsed bytes", endTag-curPos);
    }


    IF_VERBOSE_PARSE(
        log_parse(_("sound stream head: format=%s, rate=%d, 16=%d, stereo=%d, ct=%d, latency=%d"),
          format, streamSoundRate, int(streamSound16bit), int(streamSoundStereo), sampleCount, latency);
    );

    // Store all the data in a SoundInfo object
    std::auto_ptr<media::SoundInfo> sinfo;
    sinfo.reset(new media::SoundInfo(format, streamSoundStereo, streamSoundRate, sampleCount, streamSound16bit, latency));

    // Stores the sounddata in the soundhandler, and the ID returned
    // can be used to starting, stopping and deleting that sound
    int handler_id = handler->create_sound(std::auto_ptr<SimpleBuffer>(0), sinfo);

    m.set_loading_sound_stream_id(handler_id);
}


void
file_attributes_loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == SWF::FILEATTRIBUTES); // 69

    struct file_attrs_flags {
        unsigned reserved1;
        bool metadata;
        bool as3;
        unsigned reserved2;
        bool network;
        unsigned reserved3;
    };

    file_attrs_flags flags;

    in.ensureBytes(1 + 3);
    flags.reserved1 = in.read_uint(3);
    flags.metadata = in.read_bit(); 
    flags.as3 = in.read_bit();
    flags.reserved2 = in.read_uint(2);
    flags.network = in.read_bit(); 
    flags.reserved3 = in.read_uint(24);

    IF_VERBOSE_PARSE
    (
        log_parse(_("File attributes: metadata=%s network=%s"),
              flags.metadata ? _("true") : _("false"),
              flags.network ? _("true") : _("false"))
    );

    if (!flags.network) {
        log_unimpl(_("FileAttributes tag in the SWF requests that "
                "network access is not granted to this movie "
                "(or application?) when loaded from the filesystem. "
                    "Anyway Gnash won't care; "
                "use white/black listing in your .gnashrc instead"));
    }

    if (flags.as3) {
        log_debug("This SWF uses AVM2");
#ifndef ENABLE_AVM2
        /// Log an error if this build can't interpret AS3.
        log_error(_("This SWF file requires AVM2, which was not enabled at "
                    "compile time."));
#endif
    }
    else log_debug("This SWF uses AVM1");

    // TODO: - don't allow later FileAttributes tags in the same movie
    //         to override the first one used.
    //       - only use if it is the *first* tag in the SWFStream.

    if (flags.as3) m.setAS3();

}


void
metadata_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& /*r*/)
{
    assert(tag == SWF::METADATA); 

    // this is supposed to be an XML string
    std::string metadata;
    in.read_string(metadata);

    IF_VERBOSE_PARSE (
        log_parse(_("  RDF metadata (information only): [[\n%s\n]]"),
            metadata);
    );

    // The metadata tag exists exclusively for external
    // description of the SWF file and should be ignored
    // by the SWF player.
    //
    // Note: the presence of metadata should correspond to the
    // file attributes flag hasMetadata; otherwise the SWF
    // is malformed.
    //
    // This should be in RDF format, so should be easy to parse
    // (knowing how well Adobe conform to XML standards...) if
    // it's worth it.
    // See http://www.w3.org/TR/rdf-syntax-grammar/
    log_debug(_("Descriptive metadata from movie %s: %s"),
            m.get_url(), metadata);

#ifdef USE_SWFTREE
    // If the Movie Properties tree is disabled, the metadata
    // is discarded to save parsing time and memory. There seems
    // to be no limit on its length, although you'd have to be
    // malicious or stupid to put really enormous amounts of
    // descriptive metadata in a SWF. There can be one tag for each
    // loaded SWF, however, so it could mount up. 
    m.storeDescriptiveMetadata(metadata);
#endif

}

void
serialnumber_loader(SWFStream& in, TagType tag, movie_definition& /*m*/, 
        const RunResources& /*r*/)
{
    assert(tag == SWF::SERIALNUMBER); // 41

    in.ensureBytes(26);

    double id = in.read_u32();
    double edition = in.read_u32();
    int major = in.read_u8();
    int minor = in.read_u8();

    boost::uint32_t buildL = in.read_u32();
    boost::uint32_t buildH = in.read_u32();
    boost::uint64_t build = (((boost::uint64_t)buildH) << 32) + buildL;

    boost::uint32_t timestampL = in.read_u32();
    boost::uint32_t timestampH = in.read_u32();
    // This timestamp is number of milliseconds since 1 Jan 1970 (epoch)
    boost::uint64_t timestamp = (((boost::uint64_t)timestampH) << 32)
        + timestampL;

    std::stringstream ss;
    ss << "SERIALNUMBER: Version " << id << "." << edition 
        << "." << major << "." << minor;
    ss << " - Build " << build;
    ss << " - Timestamp " << timestamp;

    log_debug("%s", ss.str());

    // attach to movie_definition ?
}

void
reflex_loader(SWFStream& in, TagType tag, movie_definition& /*m*/,
        const RunResources& /*r*/)
{
    assert(tag == SWF::REFLEX); // 777

    in.ensureBytes(3);
    boost::uint8_t first = in.read_u8();
    boost::uint8_t second = in.read_u8();
    boost::uint8_t third = in.read_u8();

    IF_VERBOSE_PARSE (
    log_parse(_("  reflex = \"%c%c%c\""), first, second, third);
    );

    log_unimpl(_("REFLEX tag parsed (\"%c%c%c\") but unused"),
            first, second, third);

}

} // namespace gnash::SWF

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

} // namespace gnash
