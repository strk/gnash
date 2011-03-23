// DefineBitsTag.cpp: bitmap tag loading for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "DefineBitsTag.h"

#include <limits>
#include <cassert>
#include <boost/static_assert.hpp>
#include <boost/scoped_array.hpp>

#include "IOChannel.h"
#include "utility.h"
#include "log.h"
#include "SWFStream.h"
#include "zlib_adapter.h"
#include "SWFMovieDefinition.h"
#include "SWF.h"
#include "swf/TagLoadersTable.h"
#include "GnashException.h"
#include "RunResources.h"
#include "Renderer.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "GnashImageJpeg.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

namespace gnash {
namespace SWF {

// Forward declarations
namespace {
    void inflateWrapper(SWFStream& in, void* buffer, size_t buffer_bytes);

    std::auto_ptr<image::GnashImage> readDefineBitsJpeg(SWFStream& in,
            movie_definition& m);
    std::auto_ptr<image::GnashImage> readDefineBitsJpeg2(SWFStream& in);
    /// DefineBitsJpeg3, also DefineBitsJpeg4!
    std::auto_ptr<image::GnashImage> readDefineBitsJpeg3(SWFStream& in, TagType tag);
    std::auto_ptr<image::GnashImage> readLossless(SWFStream& in, TagType tag);

}

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

    virtual ~StreamAdapter() {}

    virtual std::streamsize read(void* dst, std::streamsize bytes) {
        std::streamsize bytesLeft = endPos - currPos;
        if (bytesLeft < bytes) {
            if (!bytesLeft) return 0;
            //log_debug("Requested to read past end of stream range");
            bytes = bytesLeft;
        }
        std::streamsize actuallyRead = s.read(static_cast<char*>(dst), bytes);
        currPos += actuallyRead;
        return actuallyRead;
    }

    virtual void go_to_end() {
        s.seek(endPos);
    }

    virtual bool eof() const {
        return (currPos == endPos);
    }

    // Return false on failure, true on success
    virtual bool seek(std::streampos pos) {
        // SWFStream::seek() returns true on success
        if (s.seek(pos)) {
            currPos = pos;
            return true;
        }
        return false;
    }

    virtual size_t size() const {
        return (endPos - startPos);
    }

    virtual std::streampos tell() const {
        return currPos;
    }
    
    virtual bool bad() const {
        // Is there any point in this?
        return false;
    }

public:

    /// Get an IOChannel from a gnash::SWFStream
    static std::auto_ptr<IOChannel> getFile(SWFStream& str,
            unsigned long endPos) {
        std::auto_ptr<IOChannel> ret (new StreamAdapter(str, endPos));
        return ret;
    }
};

} // anonymous namespace

// Load JPEG compression tables that can be used to load
// images further along in the SWFStream.
void
jpeg_tables_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& /*r*/)
{
    assert(tag == SWF::JPEGTABLES);

    IF_VERBOSE_PARSE(
        log_parse(_("  jpeg_tables_loader"));
    );

    const std::streampos currPos = in.tell();
    const std::streampos endPos = in.get_tag_end_position();

    assert(endPos >= currPos);

    const unsigned long jpegHeaderSize = endPos - currPos;

    if (!jpegHeaderSize) {
        log_debug(_("No bytes to read in JPEGTABLES tag at offset %d"),
                currPos);
    }

    std::auto_ptr<image::JpegInput> input;

    try {
        // NOTE: we cannot limit input SWFStream here as the same jpeg::input
        // instance will be used for reading subsequent DEFINEBITS and similar
        // tags, which are *different* tags, so have different boundaries !!
        //
        // Anyway the actual reads are limited to currently opened tag as 
        // of gnash::SWFStream::read(), so this is not a problem.
        //
        boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                    std::numeric_limits<std::streamsize>::max()).release());
        //  transfer ownership to the image::JpegInput
        input = image::JpegInput::createSWFJpeg2HeaderOnly(ad, jpegHeaderSize);

    }
    catch (const std::exception& e) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Error creating header-only jpeg2 input: %s",
                e.what());
        );
        return;
    }

    log_debug("Setting jpeg loader to %p", (void*)input.get());
    m.set_jpeg_loader(input);
}

void
DefineBitsTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& r)
{
    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    if (m.getBitmap(id)) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITS: Duplicate id (%d) for bitmap "
                    "DisplayObject - discarding it"), id);
        );
        return;
    }

    std::auto_ptr<image::GnashImage> im;

    switch (tag) {
        case SWF::DEFINEBITS:
            im = readDefineBitsJpeg(in, m);
            break;
        case SWF::DEFINEBITSJPEG2:
            im = readDefineBitsJpeg2(in);
            break;
        case SWF::DEFINEBITSJPEG3:
        case SWF::DEFINEBITSJPEG4:
            im = readDefineBitsJpeg3(in, tag);
            break;
        case SWF::DEFINELOSSLESS:
        case SWF::DEFINELOSSLESS2:
            im = readLossless(in, tag);
            break;
        default:
            std::abort();
    }

    if (!im.get()) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Failed to parse bitmap for character %1%"), id);
        );
        return;
    }

    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(
            log_parse(_("No renderer, not adding bitmap %1%"), id)
        );
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(im);

    IF_VERBOSE_PARSE(
        log_parse(_("Adding bitmap id %1%"), id);
    );
    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
}

namespace {

// A JPEG image without included tables; those should be in an
// existing image::JpegInput object stored in the movie.
std::auto_ptr<image::GnashImage>
readDefineBitsJpeg(SWFStream& /*in*/, movie_definition& m)
{
    std::auto_ptr<image::GnashImage> im;

    // Read the image data.
    image::JpegInput* j_in = m.get_jpeg_loader();
    if (!j_in) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITS: No jpeg loader registered in movie "
                    "definition - discarding bitmap"));
        );
        return im;
    }

    j_in->discardPartialBuffer();
    
    try {
        im = image::JpegInput::readSWFJpeg2WithTables(*j_in);
    }
    catch (const std::exception& e) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Error reading jpeg2 with headers for DisplayObject "
                "%s", e.what());
        );
    }
    
    return im;
}

/// Check the file type of the stream
//
/// This peeks at the first three bytes. The stream position after
/// the function returns is the same as at the call time.
//
/// If the stream is too short to determine the type, a ParserException
/// is thrown.
FileType
checkFileType(SWFStream& in)
{
#ifndef NDEBUG
    const size_t start = in.tell();
#endif

    const size_t bytes = 3;
    char buf[bytes];

    const size_t read = in.read(buf, bytes);
    in.seek(in.tell() - read);

    if (read < bytes) {
        throw ParserException("DefineBits data is much too short!");
    }

    // Check the data type. The pp version 9,0,115,0 supports PNG and GIF
    // in DefineBits tags, though it is not documented. The version makes
    // no difference.
    if (std::equal(buf, buf + 3, "\x89PN")) {
        return GNASH_FILETYPE_PNG;
    }

    if (std::equal(buf, buf + 3, "GIF")) {
        return GNASH_FILETYPE_GIF;
    }
    return GNASH_FILETYPE_JPEG;  

    assert(in.tell() == start);
}


std::auto_ptr<image::GnashImage>
readDefineBitsJpeg2(SWFStream& in)
{
    const FileType ft = checkFileType(in);

    // Read the image data.
    boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                in.get_tag_end_position()).release());

    return image::Input::readImageData(ad, ft);
}


/// Parse a DefineBitsJpeg3 or 4 tag.
std::auto_ptr<image::GnashImage>
readDefineBitsJpeg3(SWFStream& in, TagType tag)
{
    in.ensureBytes(4);
    const boost::uint32_t jpeg_size = in.read_u32();

    if (tag == DEFINEBITSJPEG4) {
        in.ensureBytes(2);
        const float deblocking = in.read_short_ufixed();
        IF_VERBOSE_PARSE(
            log_parse("DefineBitsJpeg4 deblocking: %1%", deblocking);
        );
    }

    const FileType ft = checkFileType(in);

    // If the image doesn't contain JPEG data, it also has no alpha
    // data.
    if (ft != GNASH_FILETYPE_JPEG) {
        log_debug("TESTING: non-JPEG data in DefineBitsJpeg3");
        // Read the image data.
        boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                    in.get_tag_end_position()).release());
        return image::Input::readImageData(ad, ft);
    }

    // We assume it's a JPEG with alpha data.
    const boost::uint32_t alpha_position = in.tell() + jpeg_size;

#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load jpeg3 image data"));
    return std::auto_ptr<image::GnashImage>();
#else

    // Read rgb data.
    boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                alpha_position).release());
    std::auto_ptr<image::ImageRGBA> im = image::Input::readSWFJpeg3(ad);
    
    /// Failure to read the jpeg.
    if (!im.get()) return std::auto_ptr<image::GnashImage>();

    // Read alpha channel.
    in.seek(alpha_position);

    const size_t imWidth = im->width();
    const size_t imHeight = im->height();
    const size_t bufferLength = imWidth * imHeight;

    boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[bufferLength]);

    inflateWrapper(in, buffer.get(), bufferLength);

    // TESTING:
    // magical trevor contains this tag
    //  ea8bbad50ccbc52dd734dfc93a7f06a7  6964trev3c.swf
    image::mergeAlpha(*im, buffer.get(), bufferLength);

#endif
    return static_cast<std::auto_ptr<image::GnashImage> >(im);
}


std::auto_ptr<image::GnashImage>
readLossless(SWFStream& in, TagType tag)
{
    assert(tag == SWF::DEFINELOSSLESS || tag == SWF::DEFINELOSSLESS2);
    in.ensureBytes(2 + 2 + 1); // the initial header 

    // 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    const boost::uint8_t bitmap_format = in.read_u8();
    const boost::uint16_t width = in.read_u16();
    const boost::uint16_t height = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  defbitslossless2: tag = %d, fmt = %d, "
                "w = %d, h = %d"), tag, bitmap_format, width, height);
    );

    std::auto_ptr<image::GnashImage> image;  
    if (!width || !height) {
         IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Bitmap has a height or width of 0"));
        );
        return image;
    } 
    
#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load zipped image data"));
    return image;
#else

    unsigned short channels;
    bool alpha = false;

    try {
        switch (tag) {
            case SWF::DEFINELOSSLESS:
                image.reset(new image::ImageRGB(width, height));
                channels = 3;
                break;
            case SWF::DEFINELOSSLESS2:
                image.reset(new image::ImageRGBA(width, height));
                channels = 4;
                alpha = true;
                break;
            default:
                // This is already asserted.
                std::abort();
        }
    }
    catch (const std::bad_alloc&) {
        // Image constructors will throw bad_alloc if they don't like the
        // size. This isn't usually from operator new.
        log_error(_("Will not allocate %1%x%2% image in DefineBitsLossless "
                "tag"), width, height);
        return image;
    }

    unsigned short bytes_per_pixel;
    int colorTableSize = 0;

    switch (bitmap_format) {
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
            return std::auto_ptr<image::GnashImage>();
    }

    const size_t pitch = (width * bytes_per_pixel + 3) &~ 3;
    const size_t bufSize = colorTableSize * channels + pitch * height;
    boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[bufSize]);

    inflateWrapper(in, buffer.get(), bufSize);
    assert(in.tell() <= in.get_tag_end_position());

    switch (bitmap_format) {

        case 3:
        {
            // 8-bit data, preceded by a palette.
            boost::uint8_t* colorTable = buffer.get();

            for (size_t j = 0; j < height; ++j) {
                boost::uint8_t* inRow = buffer.get() + 
                    colorTableSize * channels + j * pitch;

                boost::uint8_t* outRow = scanline(*image, j);
                for (size_t i = 0; i < width; ++i) {
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

            for (size_t j = 0; j < height; ++j) {

                boost::uint8_t* inRow = buffer.get() + j * pitch;
                boost::uint8_t* outRow = scanline(*image, j);
                for (size_t i = 0; i < width; ++i) {
                    const boost::uint16_t pixel = inRow[i * 2] |
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
            for (size_t j = 0; j < height; ++j) {
                boost::uint8_t* inRow = buffer.get() + j * pitch;
                boost::uint8_t* outRow = scanline(*image, j);
                const int inChannels = 4;

                for (size_t i = 0; i < width; ++i) {
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

#endif // HAVE_ZLIB_H
    
    return image;

}

#ifdef HAVE_ZLIB_H
// Wrapper function -- uses Zlib to uncompress in_bytes worth
// of data from the input file into buffer_bytes worth of data
// into *buffer.
void
inflateWrapper(SWFStream& in, void* buffer, size_t buffer_bytes)
{
    assert(buffer);

    z_stream d_stream;

    d_stream.zalloc = 0;
    d_stream.zfree = 0;
    d_stream.opaque = 0;
    d_stream.next_in  = 0;
    d_stream.avail_in = 0;

    d_stream.next_out = static_cast<Byte*>(buffer);
    d_stream.avail_out = static_cast<uInt>(buffer_bytes);

    int err = inflateInit(&d_stream);
    if (err != Z_OK) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("inflateWrapper() inflateInit() returned %d (%s)"),
                err, d_stream.msg);
        );
        return;
    }

    const size_t CHUNKSIZE = 256;

    boost::uint8_t buf[CHUNKSIZE];
    unsigned long endTagPos = in.get_tag_end_position();

    for (;;) {
        unsigned int chunkSize = CHUNKSIZE;
        assert(in.tell() <= endTagPos);
        const size_t availableBytes =  endTagPos - in.tell();

        if (availableBytes < chunkSize) {
            if (!availableBytes) {
                // nothing more to read
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("inflateWrapper(): no end of zstream "
                        "found within swf tag boundaries"));
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
        if (err == Z_STREAM_END) {
            // correct end
            break;
        }

        if (err != Z_OK) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("inflateWrapper() inflate() returned %d (%s)"),
                    err, d_stream.msg);
            );
            break;
        }
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK) {
        log_error(_("inflateWrapper() inflateEnd() return %d (%s)"),
                err, d_stream.msg);
    }
}
#endif // HAVE_ZLIB_H

} // unnamed namespace

} // namespace SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

