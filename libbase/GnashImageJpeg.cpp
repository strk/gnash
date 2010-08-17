// GnashImageJpeg.cpp:  Jpeg reader, for Gnash.
// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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
//
// Original version by Thatcher Ulrich <tu@tulrich.com> 2002
//
// Wrapper for jpeg file operations.  The actual work is done by the
// IJG jpeg lib.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "utility.h"
#include "GnashImage.h"
#include "IOChannel.h"
#include "log.h"
#include "GnashException.h"
#include "GnashImageJpeg.h"

#include <sstream>
#include <csetjmp>

/// \brief A namespace solely for avoiding name 
/// conflicts with other external headers.
namespace jpeg {

// jpeglib.h redefines HAVE_STDLIB_H. This silences
// the warnings, but it's not good.
#undef HAVE_STDLIB_H
extern "C" {
# include <jpeglib.h>
}		
#undef HAVE_STDLIB_H

}

// jpeglib.h is included in the namespace jpeg because otherwise it
// causes horrible conflicts with qt includes. How do C coders sustain
// the will to live?
using namespace jpeg;

#ifdef _WIN32
typedef jpeg_boolean jpeg_bool_t;
#else
typedef jpeg::boolean jpeg_bool_t;
#endif

namespace gnash
{

static void
jpeg_error_exit(j_common_ptr cinfo)
{

    // Set a flag to stop parsing 
    JpegImageInput* in = static_cast<JpegImageInput*>(cinfo->client_data);
    
    in->errorOccurred(cinfo->err->jpeg_message_table[cinfo->err->msg_code]); 

}


// Set up some error handlers for the jpeg lib.
static void
setup_jpeg_err(jpeg_error_mgr* jerr)
{
    // Set up defaults.
    jpeg_std_error(jerr);

    jerr->error_exit = jpeg_error_exit;
}

// Helper object for reading jpeg image data.  Basically a thin
static const int IO_BUF_SIZE = 4096;

// A jpeglib source manager that reads from a IOChannel.  Paraphrased
// from IJG jpeglib jdatasrc.c.
class rw_source_IOChannel
{
public:
    jpeg_source_mgr    m_pub;        /* public fields */

    // Constructor.
    explicit rw_source_IOChannel(boost::shared_ptr<IOChannel> in)
        :
        _ownSourceStream(false),
        m_in_stream(in),
        m_start_of_file(true)
    {
        init();
    }

    ~rw_source_IOChannel()
    {
    }

    static void init_source(j_decompress_ptr cinfo)
    {
        rw_source_IOChannel*    src = (rw_source_IOChannel*) cinfo->src;
        src->m_start_of_file = true;
    }

    // Read data into our input buffer.  Client calls this
    // when it needs more data from the file.
    static jpeg_bool_t fill_input_buffer(j_decompress_ptr cinfo)
    {
        rw_source_IOChannel*    src = (rw_source_IOChannel*) cinfo->src;

        // TODO: limit read as requested by caller
        size_t    bytes_read = src->m_in_stream->read(src->m_buffer, IO_BUF_SIZE);

        if (bytes_read <= 0) {
            // Is the file completely empty?
            if (src->m_start_of_file) {
                // Treat this as a fatal error.
                log_error(_("JPEG: Empty jpeg source stream."));
                return false;
            }
            // warn("jpeg end-of-stream");

            // Insert a fake EOI marker.
            src->m_buffer[0] = (JOCTET) 0xFF;
            src->m_buffer[1] = (JOCTET) JPEG_EOI;
            bytes_read = 2;
        }

        // Hack to work around SWF bug: sometimes data
        // starts with FFD9FFD8, when it should be
        // FFD8FFD9!
        if (src->m_start_of_file && bytes_read >= 4)
        {
            if (src->m_buffer[0] == 0xFF
                && src->m_buffer[1] == 0xD9 
                && src->m_buffer[2] == 0xFF
                && src->m_buffer[3] == 0xD8)
            {
                src->m_buffer[1] = 0xD8;
                src->m_buffer[3] = 0xD9;
            }
        }

        // Expose buffer state to clients.
        src->m_pub.next_input_byte = src->m_buffer;
        src->m_pub.bytes_in_buffer = bytes_read;
        src->m_start_of_file = false;

        return true;
    }

    // Called by client when it wants to advance past some
    // uninteresting data.
    static void    skip_input_data(j_decompress_ptr cinfo, long num_bytes)
    {
        rw_source_IOChannel* src = (rw_source_IOChannel*) cinfo->src;

        // According to jpeg docs, large skips are
        // infrequent.  So let's just do it the simple
        // way.
        if (num_bytes > 0) {
            while (num_bytes > static_cast<long>(src->m_pub.bytes_in_buffer)) {
                num_bytes -= static_cast<long>(src->m_pub.bytes_in_buffer);
                fill_input_buffer(cinfo);
            }
            // Handle remainder.
            src->m_pub.next_input_byte += (size_t) num_bytes;
            src->m_pub.bytes_in_buffer -= (size_t) num_bytes;
        }
    }

    static void term_source(j_decompress_ptr /* cinfo */)
    {
    }


    void    discardPartialBuffer()
    {
        // Discard existing bytes in our buffer.
        m_pub.bytes_in_buffer = 0;
        m_pub.next_input_byte = NULL;
    }

    /// Set up the given decompress object to read from the given
    /// stream.
    ///
    /// @param instream
    ///     Stream to read from. Ownership always shared with caller.
    ///
    static void setup(jpeg_decompress_struct* cinfo, boost::shared_ptr<IOChannel> instream)
    {
        // assert(cinfo->src == NULL);
        rw_source_IOChannel* source = new rw_source_IOChannel(instream);
        cinfo->src = (jpeg_source_mgr*)source;
    }

private:

    void init()
    {
        // fill in function pointers...
        m_pub.init_source = init_source;
        m_pub.fill_input_buffer = fill_input_buffer;
        m_pub.skip_input_data = skip_input_data;
        m_pub.resync_to_restart = jpeg_resync_to_restart;    // use default method
        m_pub.term_source = term_source;
        m_pub.bytes_in_buffer = 0;
        m_pub.next_input_byte = NULL;
    }

    bool _ownSourceStream;
    
    // Source stream
    boost::shared_ptr<IOChannel> m_in_stream;
    bool    m_start_of_file;        /* have we gotten any data yet? */
    JOCTET    m_buffer[IO_BUF_SIZE];        /* start of buffer */


};


JpegImageInput::JpegImageInput(boost::shared_ptr<IOChannel> in)
    :
    ImageInput(in),
    _errorOccurred(0),
    _compressorOpened(false)
{
    setup_jpeg_err(&m_jerr);
    m_cinfo.err = &m_jerr;
    m_cinfo.client_data = this;

    // Initialize decompression object.
    jpeg_create_decompress(&m_cinfo);

    rw_source_IOChannel::setup(&m_cinfo, in);
}


JpegImageInput::~JpegImageInput()
{
    finishImage();

    rw_source_IOChannel* src =
        reinterpret_cast<rw_source_IOChannel*>(m_cinfo.src);

    delete src;
    m_cinfo.src = NULL;

    jpeg_destroy_decompress(&m_cinfo);
}


void
JpegImageInput::discardPartialBuffer()
{
    rw_source_IOChannel* src = (rw_source_IOChannel*) m_cinfo.src;

    // We only have to discard the input buffer after reading the tables.
    if (src)
    {
        src->discardPartialBuffer();
    }
}


void
JpegImageInput::readHeader(unsigned int maxHeaderBytes)
{
    if (setjmp(_jmpBuf)) {
        std::stringstream ss;
        ss << _("Internal jpeg error: ") << _errorOccurred;
        throw ParserException(ss.str());
    }

    if (maxHeaderBytes) {
        // Read the encoding tables.
        // TODO: how to limit reads ?
        int ret = jpeg_read_header(&m_cinfo, FALSE);
        switch (ret) {
            case JPEG_SUSPENDED: 
                // suspended due to lack of data
                throw ParserException(_("Lack of data during JPEG "
                            "header parsing"));
                break;
            case JPEG_HEADER_OK: 
                // Found valid image datastream
                break;
            case JPEG_HEADER_TABLES_ONLY:
                // Found valid table-specs-only datastream
                break;
            default:
                log_debug(_("unexpected: jpeg_read_header returned %d [%s:%d]"),
                            ret, __FILE__, __LINE__);
                break;
        }

        if (_errorOccurred) {
            std::stringstream ss;
            ss << _("Internal jpeg error: ") << _errorOccurred;
            throw ParserException(ss.str());
        }

    }

    // Don't start reading any image data!
    // App does that manually using start_image.
}


void
JpegImageInput::read()
{
    assert(!_compressorOpened);

    if (setjmp(_jmpBuf)) {
        std::stringstream ss;
        ss << _("Internal jpeg error: ") << _errorOccurred;
        throw ParserException(ss.str());
    }

    // hack, FIXME
    static const int stateReady = 202;    /* found SOS, ready for start_decompress */
    while (m_cinfo.global_state != stateReady) {
        int ret = jpeg_read_header(&m_cinfo, FALSE);
        switch (ret) {
            case JPEG_SUSPENDED: 
                // suspended due to lack of data
                throw ParserException(_("lack of data during JPEG "
                            "header parsing"));
                break;
            case JPEG_HEADER_OK: 
                // Found valid image datastream
                break;
            case JPEG_HEADER_TABLES_ONLY: 
                // Found valid table-specs-only datastream
                break;
            default:
                log_debug(_("unexpected: jpeg_read_header returned %d [%s:%d]"),
                        ret, __FILE__, __LINE__);
                break;
        }
    }

    if (_errorOccurred) {
        std::stringstream ss;
        ss << _("Internal jpeg error during header parsing: ") << _errorOccurred;
        throw ParserException(ss.str());
    }

    jpeg_start_decompress(&m_cinfo);

    if (_errorOccurred) {
        std::stringstream ss;
        ss << _("Internal jpeg error during decompression: ") << _errorOccurred;
        throw ParserException(ss.str());
    }

    _compressorOpened = true;
    
    // Until this point the type should be GNASH_IMAGE_INVALID.
    // It's possible to create transparent JPEG data by merging an
    // alpha channel, but that is handled explicitly elsewhere.
    _type = GNASH_IMAGE_RGB;
}


void
JpegImageInput::finishImage()
{
    if (setjmp(_jmpBuf)) {
        std::stringstream ss;
        ss << _("Internal jpeg error: ") << _errorOccurred;
        throw ParserException(ss.str());
    }

    if (_compressorOpened) {
        jpeg_finish_decompress(&m_cinfo);
        _compressorOpened = false;
    }
}


// Return the height of the image.  Take the data from our m_cinfo struct.
size_t
JpegImageInput::getHeight() const
{
    assert(_compressorOpened);
    return m_cinfo.output_height;
}


// Return the width of the image.  Take the data from our m_cinfo struct.
size_t
JpegImageInput::getWidth() const
{
    assert(_compressorOpened);
    return m_cinfo.output_width;
}


size_t
JpegImageInput::getComponents() const
{
    assert(_compressorOpened);
    return m_cinfo.output_components;
}


void
JpegImageInput::readScanline(unsigned char* rgb_data)
{
    assert(_compressorOpened);
    assert(m_cinfo.output_scanline < m_cinfo.output_height);

    const int lines_read = jpeg_read_scanlines(&m_cinfo, &rgb_data, 1);
    assert(lines_read == 1);

    // Expand grayscale to RGB
    if (m_cinfo.out_color_space == JCS_GRAYSCALE) {
        size_t w = getWidth();
        unsigned char* src = rgb_data + w - 1;
        unsigned char* dst = rgb_data + (w * 3) - 1;
        for (;  w;  w--, src--) {
            *dst-- = *src;
            *dst-- = *src;
            *dst-- = *src;
        }
    }    
}


void
JpegImageInput::errorOccurred(const char* msg)
{
    log_debug("Long jump: banzaaaaaai!");
    _errorOccurred = msg;

    // Mark the compressor as closed so we can open another image
    // with this instance. We should throw on any errors, so there
    // should be no further activity on the current image.
    if (_compressorOpened) _compressorOpened = false;
    std::longjmp(_jmpBuf, 1);
}

// Create and read a new image, using a input object that
// already has tables loaded.  The IJG documentation describes
// this as "abbreviated" format.
std::auto_ptr<GnashImage>
JpegImageInput::readSWFJpeg2WithTables(JpegImageInput& loader)
{

    loader.read();

    std::auto_ptr<GnashImage> im(
            new ImageRGB(loader.getWidth(), loader.getHeight()));

    for (size_t y = 0, height = loader.getHeight(); y < height; y++) {
        loader.readScanline(scanline(*im, y));
    }

    loader.finishImage();

    return im;
}



// A jpeglib destination manager that writes to a IOChannel.
// Paraphrased from IJG jpeglib jdatadst.c.
class rw_dest_IOChannel
{
public:
    struct jpeg_destination_mgr    m_pub;    /* public fields */

    /// Constructor. 
    //
    /// The caller is responsible for closing
    /// the output stream after it's done using us.
    ///
    /// @param out
    ///    The output stream, externally owned.
    ///
    rw_dest_IOChannel(IOChannel& out)
        :
        m_out_stream(out)
    {
        // fill in function pointers...
        m_pub.init_destination = init_destination;
        m_pub.empty_output_buffer = empty_output_buffer;
        m_pub.term_destination = term_destination;

        m_pub.next_output_byte = m_buffer;
        m_pub.free_in_buffer = IO_BUF_SIZE;
    }

    static void init_destination(j_compress_ptr cinfo)
    {
        rw_dest_IOChannel*    dest = (rw_dest_IOChannel*) cinfo->dest;
        assert(dest);

        dest->m_pub.next_output_byte = dest->m_buffer;
        dest->m_pub.free_in_buffer = IO_BUF_SIZE;
    }

    // Set up the given compress object to write to the given
    // output stream.
    static void setup(j_compress_ptr cinfo, IOChannel& outstream)
    {
        cinfo->dest = (jpeg_destination_mgr*) (new rw_dest_IOChannel(outstream));
    }

    /// Write the output buffer into the stream.
    static jpeg_bool_t empty_output_buffer(j_compress_ptr cinfo)
    {
        rw_dest_IOChannel*    dest = (rw_dest_IOChannel*) cinfo->dest;
        assert(dest);

        if (dest->m_out_stream.write(dest->m_buffer, IO_BUF_SIZE) != IO_BUF_SIZE)
        {
            // Error.
            log_error(_("jpeg::rw_dest_IOChannel couldn't write data."));
            return false;
        }

        dest->m_pub.next_output_byte = dest->m_buffer;
        dest->m_pub.free_in_buffer = IO_BUF_SIZE;

        return true;
    }

    /// Terminate the destination. 
    //
    /// Flush any leftover data, and make sure we get deleted.
    ///
    static void term_destination(j_compress_ptr cinfo)
    {
        rw_dest_IOChannel* dest = (rw_dest_IOChannel*) cinfo->dest;
        assert(dest);

        // Write any remaining data.
        int    datacount = IO_BUF_SIZE - dest->m_pub.free_in_buffer;
        if (datacount > 0) {
            if (dest->m_out_stream.write(dest->m_buffer, datacount) != datacount)
            {
                // Error.
                log_error(_("jpeg::rw_dest_IOChannel::term_destination "
                            "couldn't write data."));
            }
        }

        // Clean ourselves up.
        delete dest;
        cinfo->dest = NULL;
    }

private:    

    // Source stream, owned in this context by JpegImageInput
    IOChannel& m_out_stream;    

    JOCTET m_buffer[IO_BUF_SIZE];        /* start of buffer */

};


JpegImageOutput::JpegImageOutput(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int quality)
    :
    ImageOutput(out, width, height)
{
    m_cinfo.err = jpeg_std_error(&m_jerr);

    // Initialize decompression object.
    jpeg_create_compress(&m_cinfo);

    rw_dest_IOChannel::setup(&m_cinfo, *_outStream);
    m_cinfo.image_width = _width;
    m_cinfo.image_height = _height;
    m_cinfo.input_components = 3;
    m_cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&m_cinfo);
    jpeg_set_quality(&m_cinfo, quality, TRUE);

    jpeg_start_compress(&m_cinfo, TRUE);
}


JpegImageOutput::~JpegImageOutput()
{
    jpeg_finish_compress(&m_cinfo);
    jpeg_destroy_compress(&m_cinfo);
}


void
JpegImageOutput::writeImageRGB(const unsigned char* rgbData)
{
    // RGB...
    const size_t components = 3;

    for (size_t y = 0; y < _height; ++y)
    {
        const unsigned char* ypos = &rgbData[y * _width * components];

        // JPEG needs non-const data.
        jpeg_write_scanlines(&m_cinfo, const_cast<unsigned char**>(&ypos), 1);
    }
}


std::auto_ptr<ImageOutput>
JpegImageOutput::create(boost::shared_ptr<IOChannel> out, size_t width,
        size_t height, int quality)
{
    std::auto_ptr<ImageOutput> outChannel(
            new JpegImageOutput(out, width, height, quality));
    return outChannel;
}

} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
