// tag_loaders.cpp: SWF tags loaders, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "IOChannel.h" // for StreamAdapter inheritance
#include "utility.h"
#include "action.h"
#include "action_buffer.h"
#include "button_character_def.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "morph2_character_def.h"
#include "shape.h"
#include "SWFStream.h"
#include "styles.h"
#include "timers.h"
#include "image.h"
#include "zlib_adapter.h"
#include "sprite_definition.h"
#include "sprite_instance.h"
#include "swf_function.h"
#include "swf_event.h"
#include "as_function.h"
#include "SWFMovieDefinition.h"
#include "swf.h"
#include "swf/TagLoadersTable.h"
#include "text_character_def.h"
#include "edit_text_character_def.h"
#include "URL.h"
#include "GnashException.h"
#include "video_stream_def.h"
#include "sound_definition.h"
#include "abc_block.h"
#include "SoundInfo.h"
#include "gnash.h" // FileType enum
#include "MediaHandler.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#include <map>
#include <limits>
#include <cassert>
#include <utility> // for std::make_pair
#include <boost/static_assert.hpp>

namespace gnash {

namespace SWF {
namespace tag_loaders {


/// Anonymous namespace
namespace {

/// Provide an IOChannel interface around a SWFStream for reading 
/// embedded image data
class StreamAdapter : public IOChannel
{
    SWFStream& s;
    unsigned long startPos;
    unsigned long endPos;
    unsigned long currPos;

    StreamAdapter(SWFStream& str, unsigned long maxPos)
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

    virtual int read(void* dst, int bytes) 
    {
        unsigned bytesLeft = endPos - currPos;
        if ( bytesLeft < (unsigned)bytes )
        {
            if ( ! bytesLeft ) return 0;
            //log_debug("Requested to read past end of stream range");
            bytes = bytesLeft;
        }
        unsigned actuallyRead = s.read((char*)dst, bytes);
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

    // Return -1 on failure, 0 on success
    virtual int seek(int pos)
    {
        // SWFStream::seek() returns true on success
        if (s.seek(pos))
        {
            currPos = pos;
            return 0;
        }
        return -1;
    }

    virtual int size() const
    {
        return (endPos - startPos);
    }

    virtual int tell() const
    {
        return currPos;
    }
    
    virtual int get_error() const
    {
        // Is there any point in this?
        return TU_FILE_NO_ERROR;
    }

public:

    /// Get an IOChannel from a gnash::SWFStream
    static std::auto_ptr<IOChannel> getFile(SWFStream& str, unsigned long endPos)
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
void null_loader(SWFStream& /*in*/, tag_type /*tag*/, movie_definition& /*m*/)
{
}

// Label the current frame of m with the name from the SWFStream.
void
frame_label_loader(SWFStream& in, tag_type tag, movie_definition& m)
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
jpeg_tables_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    //GNASH_REPORT_FUNCTION;
    assert(tag == SWF::JPEGTABLES);

    IF_VERBOSE_PARSE
    (
        log_parse(_("  jpeg_tables_loader"));
    );

    const unsigned long currPos = in.tell();
    const unsigned long endPos = in.get_tag_end_position();

    assert(endPos >= currPos);

    const unsigned long jpegHeaderSize = endPos - currPos;

    if ( ! jpegHeaderSize )
    {
        log_debug(_("No bytes to read in JPEGTABLES tag at offset %d"), currPos);
    }

    std::auto_ptr<JpegImageInput> input;

    try
    {
    // NOTE: we can NOT limit input SWFStream here as the same jpeg::input
    // instance will be used for reading subsequent DEFINEBITS and similar
    // tags, which are *different* tags, so have different boundaries !!
    //
    // Anyway the actual reads are limited to currently opened tag as 
    // of gnash::SWFStream::read(), so this is not a problem.
    //
        boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in, std::numeric_limits<unsigned long>::max()).release());
        //  transfer ownership to the JpegImageInput
        input = JpegImageInput::createSWFJpeg2HeaderOnly(ad, jpegHeaderSize);

    }
    catch (std::exception& e)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Error creating header-only jpeg2 input: %s", e.what());
        );
        return;
    }

    log_debug("Setting jpeg loader to %p", (void*)input.get());
    m.set_jpeg_loader(input);
}


// A JPEG image without included tables; those should be in an
// existing JpegImageInput object stored in the movie.
void
define_bits_jpeg_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEBITS); // 6

    in.ensureBytes(2);
    boost::uint16_t character_id = in.read_u16();

    // Read the image data.
    JpegImageInput* j_in = m.get_jpeg_loader();
    if ( ! j_in )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITS: No jpeg loader registered in movie definition - discarding bitmap character %d"), character_id);
        );
        return;
    }

    j_in->discardPartialBuffer();
    
    std::auto_ptr<image::ImageBase> im;
    try
    {
        im = image::readSWFJpeg2WithTables(*j_in);
    }
    catch (std::exception& e)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror("Error reading jpeg2 with headers for character id %d: %s", character_id, e.what());
        );
        return;
    }
    
    
    boost::intrusive_ptr<bitmap_character_def> ch = new bitmap_character_def(im);
    
    if ( m.get_bitmap_character_def(character_id) )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITS: Duplicate id (%d) for bitmap character - discarding it"), character_id);
        );
    }
    else
    {
        m.add_bitmap_character_def(character_id, ch.get());
    }
}


void
define_bits_jpeg2_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEBITSJPEG2); // 21

    in.ensureBytes(2);
    boost::uint16_t    character_id = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  define_bits_jpeg2_loader: charid = %d pos = %ld"),
          character_id, in.tell());
    );

    // Read the image data.
    if ( m.get_bitmap_character_def(character_id) )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITSJPEG2: Duplicate id (%d) for bitmap character - discarding it"), character_id);
        );
    }
    else
    {
        boost::shared_ptr<IOChannel> ad( StreamAdapter::getFile(in, in.get_tag_end_position()).release() );

        std::auto_ptr<image::ImageBase> im (image::readImageData(ad, GNASH_FILETYPE_JPEG));

        boost::intrusive_ptr<bitmap_character_def> ch = new bitmap_character_def(im);
        m.add_bitmap_character_def(character_id, ch.get());
    }
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

    d_stream.next_out = (Byte*) buffer;
    d_stream.avail_out = (uInt) buffer_bytes;

    int err = inflateInit(&d_stream);
    if (err != Z_OK) {
    IF_VERBOSE_MALFORMED_SWF(
    log_swferror(_("inflate_wrapper() inflateInit() returned %d (%s)"), err, d_stream.msg);
    );
    return;
    }

#define CHUNKSIZE 256

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
                log_swferror(_("inflate_wrapper(): no end of zstream found within swf tag boundaries"));
                );
                break;
            }
            chunkSize = availableBytes;
        }
    
        BOOST_STATIC_ASSERT(sizeof(char) == sizeof(boost::uint8_t));

        // Fill the buffer    
        in.read((char*)buf, chunkSize);
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
            log_swferror(_("inflate_wrapper() inflate() returned %d (%s)"), err, d_stream.msg);
            );
            break;
        }
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK)
    {
        log_error(_("inflate_wrapper() inflateEnd() return %d (%s)"), err, d_stream.msg);
    }
}
#endif // HAVE_ZLIB_H


// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void
define_bits_jpeg3_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEBITSJPEG3); // 35

    in.ensureBytes(2);
    boost::uint16_t    character_id = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  define_bits_jpeg3_loader: charid = %d pos = %lx"),
          character_id, in.tell());
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
    boost::shared_ptr<IOChannel> ad( StreamAdapter::getFile(in, alpha_position).release() );
    std::auto_ptr<image::ImageRGBA> im = image::readSWFJpeg3(ad);
    
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

    // Create bitmap character.
    boost::intrusive_ptr<bitmap_character_def> ch =
            new bitmap_character_def(static_cast<std::auto_ptr<image::ImageBase> >(im));

    m.add_bitmap_character_def(character_id, ch.get());
#endif
}


void
define_bits_lossless_2_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    // tags 20 || 36
    assert(tag == SWF::DEFINELOSSLESS || tag == SWF::DEFINELOSSLESS2);

    in.ensureBytes(2+2+2+1); // the initial header 

    boost::uint16_t    character_id = in.read_u16();
    boost::uint8_t    bitmap_format = in.read_u8();    // 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    boost::uint16_t    width = in.read_u16();
    boost::uint16_t    height = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  defbitslossless2: tag = %d, id = %d, "
            "fmt = %d, w = %d, h = %d"),
          tag, character_id, bitmap_format, width, height);
    );

    if (width == 0 || height == 0)
    {
         IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Bitmap character %d has a height or width of 0."), character_id);
        );   
        return;  
    }

    // TODO: there's a lot of duplicated code in this function, we should clean it up

    //bitmap_info*    bi = NULL;
#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load zipped image data"));
    return;
#else
    if (tag == SWF::DEFINELOSSLESS) // 20
    {

        // RGB image data.
        std::auto_ptr<image::ImageBase> image (new image::ImageRGB(width, height));

        if (bitmap_format == 3)
        {
            // 8-bit data, preceded by a palette.

            const int bytes_per_pixel = 1;

                in.ensureBytes(1); // color table size
            int color_table_size = in.read_u8();
            color_table_size++;    // !! SWF stores one less than the actual size

            int pitch = (width * bytes_per_pixel + 3) & ~3;

            int buffer_bytes = color_table_size * 3 + pitch * height;
            boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

            inflate_wrapper(in, buffer.get(), buffer_bytes);
            assert(in.tell() <= in.get_tag_end_position());

            boost::uint8_t* color_table = buffer.get();

            for (int j = 0; j < height; j++)
            {
                boost::uint8_t*    image_in_row = buffer.get() + color_table_size * 3 + j * pitch;
                boost::uint8_t*    image_out_row = image->scanline(j);
                for (int i = 0; i < width; i++)
                {
                boost::uint8_t    pixel = image_in_row[i * bytes_per_pixel];
                image_out_row[i * 3 + 0] = color_table[pixel * 3 + 0];
                image_out_row[i * 3 + 1] = color_table[pixel * 3 + 1];
                image_out_row[i * 3 + 2] = color_table[pixel * 3 + 2];
                }
            }

        }
        else if (bitmap_format == 4)
        {
            // 16 bits / pixel
            const int bytes_per_pixel = 2;
            int pitch = (width * bytes_per_pixel + 3) & ~3;

            int buffer_bytes = pitch * height;
            boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

            inflate_wrapper(in, buffer.get(), buffer_bytes);
            assert(in.tell() <= in.get_tag_end_position());

            for (int j = 0; j < height; j++)
            {
                boost::uint8_t*    image_in_row = buffer.get() + j * pitch;
                boost::uint8_t*    image_out_row = image->scanline(j);
                for (int i = 0; i < width; i++)
                {
                boost::uint16_t    pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

                // @@ How is the data packed???  I'm just guessing here that it's 565!
                image_out_row[i * 3 + 0] = (pixel >> 8) & 0xF8;    // red
                image_out_row[i * 3 + 1] = (pixel >> 3) & 0xFC;    // green
                image_out_row[i * 3 + 2] = (pixel << 3) & 0xF8;    // blue
                }
            }

        }
        else if (bitmap_format == 5)
        {
            // 32 bits / pixel, input is ARGB format (???)
            const int bytes_per_pixel = 4;
            int pitch = width * bytes_per_pixel;

            int buffer_bytes = pitch * height;
            boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

            inflate_wrapper(in, buffer.get(), buffer_bytes);
            assert(in.tell() <= in.get_tag_end_position());

            // Need to re-arrange ARGB into RGB.
            for (int j = 0; j < height; j++)
            {
                boost::uint8_t*    image_in_row = buffer.get() + j * pitch;
                boost::uint8_t*    image_out_row = image->scanline(j);
                for (int i = 0; i < width; i++)
                {
                boost::uint8_t a = image_in_row[i * 4 + 0];
                boost::uint8_t r = image_in_row[i * 4 + 1];
                boost::uint8_t g = image_in_row[i * 4 + 2];
                boost::uint8_t b = image_in_row[i * 4 + 3];
                image_out_row[i * 3 + 0] = r;
                image_out_row[i * 3 + 1] = g;
                image_out_row[i * 3 + 2] = b;
                a = a;    // Inhibit warning.
                }
            }

        }

        if ( m.get_bitmap_character_def(character_id) )
        {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("DEFINEBITSLOSSLESS: Duplicate id (%d) "
                               "for bitmap character - discarding it"), character_id);
            );
        }
        else
        {
            boost::intrusive_ptr<bitmap_character_def> ch = new bitmap_character_def(image);

            // add image to movie, under character id.
            m.add_bitmap_character_def(character_id, ch.get());
        }
    }
    else
    {
        // RGBA image data.
        assert(tag == SWF::DEFINELOSSLESS2); // 36

        std::auto_ptr<image::ImageBase> image(new image::ImageRGBA(width, height));

        if (bitmap_format == 3)
        {
            // 8-bit data, preceded by a palette.

            const int bytes_per_pixel = 1;
                in.ensureBytes(1); // color table size
            int color_table_size = in.read_u8();
            color_table_size++;    // !! SWF stores one less than the actual size

            int pitch = (width * bytes_per_pixel + 3) & ~3;

            int buffer_bytes = color_table_size * 4 + pitch * height;
            boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

            inflate_wrapper(in, buffer.get(), buffer_bytes);
            assert(in.tell() <= in.get_tag_end_position());

            boost::uint8_t* color_table = buffer.get();

        for (int j = 0; j < height; j++)
        {
            boost::uint8_t*    image_in_row = buffer.get() + color_table_size * 4 + j * pitch;
            boost::uint8_t*    image_out_row = image->scanline(j);
            for (int i = 0; i < width; i++)
            {
                boost::uint8_t    pixel = image_in_row[i * bytes_per_pixel];
                image_out_row[i * 4 + 0] = color_table[pixel * 4 + 0];
                image_out_row[i * 4 + 1] = color_table[pixel * 4 + 1];
                image_out_row[i * 4 + 2] = color_table[pixel * 4 + 2];
                image_out_row[i * 4 + 3] = color_table[pixel * 4 + 3];
            }
        }

        }
        else if (bitmap_format == 4)
        {
            // 16 bits / pixel
            const int bytes_per_pixel = 2;
            int pitch = (width * bytes_per_pixel + 3) & ~3;

            int buffer_bytes = pitch * height;
            boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

            inflate_wrapper(in, buffer.get(), buffer_bytes);
            assert(in.tell() <= in.get_tag_end_position());

            for (int j = 0; j < height; j++)
            {
                boost::uint8_t*    image_in_row = buffer.get() + j * pitch;
                boost::uint8_t*    image_out_row = image->scanline(j);
                for (int i = 0; i < width; i++)
                {
                    boost::uint16_t    pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

                    // @@ How is the data packed???  I'm just guessing here that it's 565!
                    image_out_row[i * 4 + 0] = 255;            // alpha
                    image_out_row[i * 4 + 1] = (pixel >> 8) & 0xF8;    // red
                    image_out_row[i * 4 + 2] = (pixel >> 3) & 0xFC;    // green
                    image_out_row[i * 4 + 3] = (pixel << 3) & 0xF8;    // blue
                }
            }
        }
        else if (bitmap_format == 5)
        {
            // 32 bits / pixel, input is ARGB format

            inflate_wrapper(in, image->data(), width * height * 4);
            assert(in.tell() <= in.get_tag_end_position());

            // Need to re-arrange ARGB into RGBA.
            for (int j = 0; j < height; j++)
            {
                boost::uint8_t*    image_row = image->scanline(j);
                for (int i = 0; i < width; i++)
                {
                    boost::uint8_t    a = image_row[i * 4 + 0];
                    boost::uint8_t    r = image_row[i * 4 + 1];
                    boost::uint8_t    g = image_row[i * 4 + 2];
                    boost::uint8_t    b = image_row[i * 4 + 3];
                    image_row[i * 4 + 0] = r;
                    image_row[i * 4 + 1] = g;
                    image_row[i * 4 + 2] = b;
                    image_row[i * 4 + 3] = a;
                }
            }
        }

        boost::intrusive_ptr<bitmap_character_def> ch = new bitmap_character_def(image);

        // add image to movie, under character id.
        m.add_bitmap_character_def(character_id, ch.get());
    }
#endif // HAVE_ZLIB_H

}

// This is like null_loader except it prints a message to nag us to fix it.
void
fixme_loader(SWFStream& /*in*/, tag_type tag, movie_definition& /*m*/)
{
    static std::map<tag_type, bool> warned;
    if ( ! warned[tag] )
    {
        log_unimpl(_("  FIXME: tagtype = %d"), tag);
        warned[tag] = true;
    }
}

void define_shape_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINESHAPE
       || tag == SWF::DEFINESHAPE2
       || tag == SWF::DEFINESHAPE3
       || tag == SWF::DEFINESHAPE4 || tag == SWF::DEFINESHAPE4_);

    in.ensureBytes(2);
    boost::uint16_t    character_id = in.read_u16();
    IF_VERBOSE_PARSE(
        log_parse(_("  shape_loader: id = %d"), character_id);
    );

    shape_character_def*    ch = new shape_character_def;
    ch->read(in, tag, true, m);

    m.add_character(character_id, ch);
}

void define_shape_morph_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEMORPHSHAPE
        || tag == SWF::DEFINEMORPHSHAPE2
        || tag == SWF::DEFINEMORPHSHAPE2_); 

    in.ensureBytes(2);
    boost::uint16_t character_id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  shape_morph_loader: id = %d"), character_id);
    );

    morph2_character_def* morph = new morph2_character_def;
    morph->read(in, tag, true, m);
    m.add_character(character_id, morph);
}

//
// font loaders
//


void    define_font_loader(SWFStream& in, tag_type tag, movie_definition& m)
    // Load a DefineFont or DefineFont2 tag.
{
    assert(tag == SWF::DEFINEFONT
       || tag == SWF::DEFINEFONT2
       || tag == SWF::DEFINEFONT3 ); // 10 || 48 || 75

    in.ensureBytes(2);
    boost::uint16_t font_id = in.read_u16();

    font* f = new font;
    f->read(in, tag, m);

    m.add_font(font_id, f);

    // Automatically keeping fonts in fontlib is
    // problematic.  The app should be responsible for
    // optionally adding fonts to fontlib.
    // //fontlib::add_font(f);
}


// See description in header
void    define_font_info_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEFONTINFO || tag == SWF::DEFINEFONTINFO2);

    in.ensureBytes(2);
    boost::uint16_t font_id = in.read_u16();

    font* f = m.get_font(font_id);
    if (f)
    {
        f->read_font_info(in, tag, m);
    }
    else
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("define_font_info_loader: "
                   "can't find font w/ id %d"), font_id);
        );
    }
}

// Set font name for a font.
void define_font_name_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEFONTNAME);

    in.ensureBytes(2);
    boost::uint16_t font_id = in.read_u16();

    font* f = m.get_font(font_id);
    if (f)
    {
        f->read_font_name(in, tag, m);
    }
    else
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("define_font_name_loader: "
                           "can't find font w/ id %d"), font_id);
        );
    }
}

// Create and initialize a sprite, and add it to the movie.
void
sprite_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINESPRITE); // 39 - DefineSprite

    in.ensureBytes(2);
    int    character_id = in.read_u16();

    IF_VERBOSE_PARSE
    (
    log_parse(_("  sprite:  char id = %d"), character_id);
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
                           "top-level characters dictionary."));
        }
    );

    // will automatically read the sprite
    sprite_definition* ch = new sprite_definition(m, in);

    IF_VERBOSE_MALFORMED_SWF(
        if (!ch->get_frame_count()) {
            log_swferror(_("Sprite %d advertise no frames"), character_id);
        }
    );


    m.add_character(character_id, ch);
}



//
// end_tag
//

// end_tag doesn't actually need to exist.

void    button_sound_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEBUTTONSOUND); // 17

    in.ensureBytes(2);
    int    button_character_id = in.read_u16();
    character_def* chdef = m.get_character_def(button_character_id);
    if ( ! chdef )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBUTTONSOUND refers to an unknown character def %d"), button_character_id);
        );
        return;
    }

    button_character_definition* ch = dynamic_cast<button_character_definition*> (chdef);
    if ( ! ch )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBUTTONSOUND refers to character id %d, "
            "being a %s (expected a button definition)"),
            button_character_id,
            typeName(*chdef));
        );
        return;
    }

    ch->read(in, tag, m);
}


void    button_character_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    // 7 || 34
    assert(tag == SWF::DEFINEBUTTON || tag == SWF::DEFINEBUTTON2);

    in.ensureBytes(2);
    int    character_id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  button character loader: char_id = %d"), character_id);
    );

    button_character_definition* ch = new button_character_definition(m);
    ch->read(in, tag, m);

    m.add_character(character_id, ch);
}


//
// export
//


void    export_loader(SWFStream& in, tag_type tag, movie_definition& m)
    // Load an export tag (for exposing internal resources of m)
{
    assert(tag == SWF::EXPORTASSETS); // 56

    in.ensureBytes(2);
    int    count = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  export: count = %d"), count);
    );

    // An EXPORT tag as part of a DEFINESPRITE
    // would be a malformed SWF, anyway to be compatible
    // we should still allow that. See bug #22468.
    IF_VERBOSE_MALFORMED_SWF(
        try {
            dynamic_cast<SWFMovieDefinition&>(m);
        }
        catch (std::bad_cast& e) {
            log_swferror(_("EXPORT tag inside DEFINESPRITE. Will export in top-level symbol table."));
        }
    );


    // Read the exports.
    for (int i = 0; i < count; i++)
    {
        in.ensureBytes(2);
        boost::uint16_t    id = in.read_u16();
        std::string symbolName;
        in.read_string(symbolName);

        IF_VERBOSE_PARSE (
            log_parse(_("  export: id = %d, name = %s"), id, symbolName);
        );

        if (font* f = m.get_font(id))
        {
            // Expose this font for export.
            m.export_resource(symbolName, f);
        }
        else if (character_def* ch = m.get_character_def(id))
        {
            // Expose this movie/button/whatever for export.
            m.export_resource(symbolName, ch);
        }
        else if (sound_sample* ch = m.get_sound_sample(id))
        {
            m.export_resource(symbolName, ch);
        }
        else
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("don't know how to export resource '%s' "
                "with id %d (can't find that id)"),
                  symbolName, id);
                );
        }

    }
}


//
// import
//


void import_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::IMPORTASSETS || tag == SWF::IMPORTASSETS2);

    std::string source_url;
    in.read_string(source_url);

    // Resolve relative urls against baseurl
    URL abs_url(source_url, get_base_url());

    unsigned char import_version = 0;

    if ( tag == SWF::IMPORTASSETS2 )
    {
        in.ensureBytes(2);
        import_version = in.read_uint(8);
        unsigned char reserved = in.read_uint(8);
        UNUSED(reserved);
    }

    in.ensureBytes(2);
    int count = in.read_u16();

    IF_VERBOSE_PARSE
    (
        log_parse(_("  import: version = %u, source_url = %s (%s), count = %d"),
            import_version, abs_url.str(), source_url, count);
    );


    // Try to load the source movie into the movie library.
    boost::intrusive_ptr<movie_definition> source_movie;

    try {
        source_movie = create_library_movie(abs_url);
    }
    catch (gnash::GnashException& e) {
        log_error(_("Exception: %s"), e.what());
    }

    if (!source_movie)
    {
        // Give up on imports.
        log_error(_("can't import movie from url %s"), abs_url.str());
        return;
    }

    // Quick consistency check, we might as well do
    // something smarter, if we agree on semantic
    if (source_movie == &m)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Movie attempts to import symbols from itself."));
        );
        return;
    }

    movie_definition::Imports imports;

    // Get the imports.
    for (int i = 0; i < count; i++)
    {
        in.ensureBytes(2);
        boost::uint16_t    id = in.read_u16();
        std::string symbolName;
        in.read_string(symbolName);
        IF_VERBOSE_PARSE (
            log_parse(_("  import: id = %d, name = %s"), id, symbolName);
        );
        imports.push_back( std::make_pair(id, symbolName) );
    }

    m.importResources(source_movie, imports);
}

// Read a DefineText tag.
void    define_edit_text_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEEDITTEXT); // 37

    in.ensureBytes(2);
    boost::uint16_t character_id = in.read_u16();

    edit_text_character_def* ch = new edit_text_character_def();
    IF_VERBOSE_PARSE(
        log_parse(_("edit_text_char, id = %d"), character_id);
    );
    ch->read(in, tag, m);

    m.add_character(character_id, ch);
}

// See description in header
void
define_text_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINETEXT || tag == SWF::DEFINETEXT2);

    in.ensureBytes(2);
    boost::uint16_t    character_id = in.read_u16();

    text_character_def* ch = new text_character_def();
    IF_VERBOSE_PARSE(
        log_parse(_("text_character, id = %d"), character_id);
    );
    ch->read(in, tag, m);

    m.add_character(character_id, ch);
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
define_sound_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINESOUND); // 14

    media::sound_handler* handler = get_sound_handler();

    in.ensureBytes(2+4+1+4); // character id + flags + sample count

    boost::uint16_t    character_id = in.read_u16();

    media::audioCodecType    format = static_cast<media::audioCodecType>(in.read_uint(4));
    unsigned sample_rate_in = in.read_uint(2); // see s_sample_rate_table
    if ( sample_rate_in >= s_sample_rate_table_len ) 
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINESOUNDLOADER: sound sample rate %d (expected 0 to %u"), 
            sample_rate_in, s_sample_rate_table_len);
        );
        sample_rate_in = 0;
    }
    int sample_rate = s_sample_rate_table[sample_rate_in];


    bool    sample_16bit = in.read_bit(); 
    bool    stereo = in.read_bit(); 

    unsigned int    sample_count = in.read_u32();

    if (format == media::AUDIO_CODEC_MP3) {
        in.ensureBytes(2);
        boost::int16_t    delay_seek = in.read_s16();    // FIXME - not implemented/used
        //
        // The DelaySeek field has the following meaning:
        // * If this value is positive, the player seeks this number of
        //   samples into the sound block before the sound is played.
        //   However, the seek is only performed if the player reached
        //   the current frame via a GotoFrame action, otherwise no
        //   seek is performed.
        //
        // * If this value is negative the player plays this number of
        //   silent samples before playing the sound block. The player
        //   behaves this way, regardless of how the current frame was
        //   reached.
        //
        // quoted from
        // http://www-lehre.informatik.uni-osnabrueck.de/~fbstark/diplom/docs/swf/Sounds.htm
        //
        LOG_ONCE ( if ( delay_seek ) log_unimpl("MP3 delay seek") );
    }

    IF_VERBOSE_PARSE
    (
        log_parse(_("define sound: ch=%d, format=%d, "
            "rate=%d, 16=%d, stereo=%d, ct=%d"),
              character_id, int(format), sample_rate,
              int(sample_16bit), int(stereo), sample_count);
    );

    // If we have a sound_handler, ask it to init this sound.

    if (handler)
    {
        // First it is the amount of data from file,
        // then the amount allocated at *data (it may grow)
        const unsigned dataLength = in.get_tag_end_position() - in.tell();

        // Allocate MediaHandler::getInputPadding() bytes more for the SimpleBuffer 
        size_t allocSize = dataLength;
        media::MediaHandler* mh = media::MediaHandler::get(); // TODO: don't use this static !
        if ( mh ) allocSize += mh->getInputPaddingSize();

        std::auto_ptr<SimpleBuffer> data( new SimpleBuffer(allocSize) );

        // dataLength is already calculated from the end of the tag, which
        // should be inside the end of the file. TODO: check that this is the case.
        const unsigned int bytesRead = in.read(reinterpret_cast<char*>(data->data()), dataLength);
        data->resize(bytesRead); // in case it's shorter...
        if (bytesRead < dataLength)
        {
            throw ParserException(_("Tag boundary reported past end of SWFStream!"));
        }

        // Store all the data in a SoundInfo object
        std::auto_ptr<media::SoundInfo> sinfo;
        sinfo.reset(new media::SoundInfo(format, stereo, sample_rate, sample_count, sample_16bit));

        // Stores the sounddata in the soundhandler, and the ID returned
        // can be used to starting, stopping and deleting that sound
        int    handler_id = handler->create_sound(data, sinfo);

        if (handler_id >= 0)
        {
        sound_sample* sam = new sound_sample(handler_id);
        m.add_sound_sample(character_id, sam);
        }

    }
    else
    {
        // is this nice to do?
        log_error(_("There is no sound handler currently active, "
            "so character with id %d will NOT be added to "
            "the dictionary"),
              character_id);
    }
}

// Load a SoundStreamHead(2) tag.
void
sound_stream_head_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    media::sound_handler* handler = get_sound_handler();

    // 18 || 45
    assert(tag == SWF::SOUNDSTREAMHEAD || tag == SWF::SOUNDSTREAMHEAD2);

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

    // FIXME:
    // no character id for soundstreams... so we make one up...
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
        log_swferror(_("SOUNDSTREAMHEAD: SWFStream sample rate %d (expected 0 to %u)"),
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
        LOG_ONCE( log_swferror(_("No samples advertised for sound SWFStream, pretty common so will warn only once")) );
    );
    }

    int latency = 0;
    if (format == media::AUDIO_CODEC_MP3)
    {
        try
        {
            in.ensureBytes(2);
            latency = in.read_s16(); // UNUSED !!
            LOG_ONCE ( if ( latency ) log_unimpl("MP3 SWFStream latency seek") );
        }
        catch (ParserException& ex)
        {
            // See https://savannah.gnu.org/bugs/?21729 for an example 
            // triggering this.
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("MP3 sound SWFStream lacks a 'latency' field");
            );
        }
    }

    IF_VERBOSE_PARSE(
        log_parse(_("sound SWFStream head: format=%d, rate=%d, 16=%d, stereo=%d, ct=%d, latency=%d"),
          int(format), streamSoundRate, int(streamSound16bit), int(streamSoundStereo), sampleCount, latency);
    );

    // Wot about reading the sample_count samples?

    // Store all the data in a SoundInfo object
    std::auto_ptr<media::SoundInfo> sinfo;
    sinfo.reset(new media::SoundInfo(format, streamSoundStereo, streamSoundRate, sampleCount, streamSound16bit));

    // Stores the sounddata in the soundhandler, and the ID returned
    // can be used to starting, stopping and deleting that sound
    int handler_id = handler->create_sound(std::auto_ptr<SimpleBuffer>(0), sinfo);

    m.set_loading_sound_stream_id(handler_id);
}

void
define_video_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::DEFINEVIDEOSTREAM); // 60
    
    in.ensureBytes(2);
    boost::uint16_t character_id = in.read_u16();

    std::auto_ptr<video_stream_definition> chdef ( new video_stream_definition(character_id) );
    chdef->readDefineVideoStream(in, tag, m);

    m.add_character(character_id, chdef.release());

}

void
video_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::VIDEOFRAME); // 61

    in.ensureBytes(2);
    boost::uint16_t character_id = in.read_u16();
    character_def* chdef = m.get_character_def(character_id);

    if (!chdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("VideoFrame tag refers to unknown video SWFStream id %d"), character_id);
        );
        return;
    }

    // TODO: add a character_def::cast_to_video_def ?
    video_stream_definition* vdef = dynamic_cast<video_stream_definition*> (chdef);
    if ( ! vdef )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("VideoFrame tag refers to a non-video character %d (%s)"), character_id, typeName(*chdef));
        );
        return;
    }

    vdef->readDefineVideoFrame(in, tag, m);
}

void
file_attributes_loader(SWFStream& in, tag_type tag, movie_definition& /*m*/)
{
    assert(tag == SWF::FILEATTRIBUTES); // 69

    typedef struct file_attrs_flags_t {
    unsigned reserved1:3;
    unsigned has_metadata:1;
    unsigned reserved2:3;
    unsigned use_network:1;
    unsigned reserved3:24;
    } file_attrs_flags;

    file_attrs_flags flags;

    in.ensureBytes(1 + 3);
    flags.reserved1 = in.read_uint(3);
    flags.has_metadata = in.read_bit(); 
    flags.reserved2 = in.read_uint(3);
    flags.use_network = in.read_bit(); 
    flags.reserved3 = in.read_uint(24);

    IF_VERBOSE_PARSE
    (
    log_parse(_("  file attributes: has_metadata=%s use_network=%s"),
          flags.has_metadata ? _("true") : _("false"),
          flags.use_network ? _("true") : _("false"))
    );

    if ( ! flags.use_network )
    {
    log_unimpl(_("FileAttributes tag in the SWF requests that "
            "network access is not granted to this movie "
            "(or application?) when loaded from the filesystem. "
                "Anyway Gnash won't care; "
            "use white/black listing in your .gnashrc instead"));
    }

    // TODO:
    //     - attach info to movie_definition.
    //     - don't allow later FileAttributes tags in the same movie
    //       to override the first one used.
    //     - only use if it is the *first* tag in the SWFStream.
}


void
metadata_loader(SWFStream& in, tag_type tag, movie_definition& m)
{
    assert(tag == SWF::METADATA); // 77

    // this is supposed to be an XML string
    std::string metadata;
    in.read_string(metadata);

    IF_VERBOSE_PARSE (
    log_parse(_("  RDF metadata (information only): [[\n%s\n]]"), metadata);
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
serialnumber_loader(SWFStream& in, tag_type tag, movie_definition& /*m*/)
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
    boost::uint64_t timestamp = (((boost::uint64_t)timestampH) << 32) + timestampL;

    std::stringstream ss;
    ss << "SERIALNUMBER: Version " << id << "." << edition << "." << major << "." << minor;
    ss << " - Build " << build;
    ss << " - Timestamp " << timestamp;

    log_debug("%s", ss.str());

    // attach to movie_definition ?
}

void
reflex_loader(SWFStream& in, tag_type tag, movie_definition& /*m*/)
{
    assert(tag == SWF::REFLEX); // 777

    in.ensureBytes(3);
    boost::uint8_t first = in.read_u8();
    boost::uint8_t second = in.read_u8();
    boost::uint8_t third = in.read_u8();

    IF_VERBOSE_PARSE (
    log_parse(_("  reflex = \"%c%c%c\""), first, second, third);
    );

    log_unimpl(_("REFLEX tag parsed (\"%c%c%c\") but unused"), first, second, third);

}

void
abc_loader(SWFStream& in, tag_type tag, movie_definition& /*m*/)
{
    assert(tag == SWF::DOABC
        || tag == SWF::DOABCDEFINE); // 72 or 82

    abc_block a;

    if (tag == SWF::DOABCDEFINE)
    {

        // Skip the 'flags' until they are actually used.
        in.ensureBytes(4);
        static_cast<void> (in.read_u32());
        std::string name;
        in.read_string(name);
    }

    //TODO: Move this to execution time so that as_object can be used. bool success = a.read(in);

    log_unimpl(_("%s tag parsed but not yet used"), tag == SWF::DOABC ? "DOABC" : "DOABCDEFINE");
}

void
define_scene_frame_label_loader(SWFStream& /*in*/, tag_type tag, movie_definition& /*m*/)
{
    assert(tag == SWF::DEFINESCENEANDFRAMELABELDATA); //86

    log_unimpl(_("%s tag parsed but not yet used"), "DEFINESCENEANDFRAMELABELDATA");
}

} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:



// VC6 is a recognized pile of crap and no one should
// worry about trying to support it.

class in_stream
{
public:
    const unsigned char*    m_in_data;
    int    m_current_bits;
    int    m_unused_bits;

    in_stream(const unsigned char* data)
        :
        m_in_data(data),
        m_current_bits(0),
        m_unused_bits(0)
    {
    }
};


} // namespace gnash
