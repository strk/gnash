// GnashImagePng.cpp: libpng wrapper for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
# include "gnashconfig.h"
#endif

#include "GnashImagePng.h"

#include <sstream>

extern "C" {
#ifdef HAVE_PNG_H
# include <png.h>
#endif
}

#include "GnashImage.h"
#include "utility.h"
#include "log.h"
#include "GnashException.h"
#include "IOChannel.h"

namespace gnash {
namespace image {

namespace {

void
error(png_struct*, const char* msg)
{
    std::ostringstream ss;
    ss << _("PNG error: ") << msg;
    throw ParserException(ss.str());
}

void
warning(png_struct*, const char* msg)
{
    log_debug("PNG warning: %s", msg);
}

void
readData(png_structp pngptr, png_bytep data, png_size_t length)
{
    // Do not call unless the PNG exists.
    assert(pngptr);
    IOChannel* in = reinterpret_cast<IOChannel*>(png_get_io_ptr(pngptr));
    in->read(reinterpret_cast<char*>(data), length);
}

void
writeData(png_structp pngptr, png_bytep data, png_size_t length)
{
    // Do not call unless the PNG exists.
    assert(pngptr);
    IOChannel* out = reinterpret_cast<IOChannel*>(png_get_io_ptr(pngptr));
    out->write(reinterpret_cast<char*>(data), length);
}

void
flushData(png_structp /*pngptr*/)
{
}

class PngInput : public Input
{

public:

    /// Construct a PngInput object to read from an IOChannel.
    //
    /// @param in   The stream to read PNG data from. Ownership is shared
    ///             between caller and JpegInput, so it is freed
    ///             automatically when the last owner is destroyed.
    PngInput(std::shared_ptr<IOChannel> in)
        :
        Input(in),
        _pngPtr(nullptr),
        _infoPtr(nullptr),
        _currentRow(0)
    {
        init();
    }
    
    ~PngInput();
    
    /// Begin processing the image data.
    void read();

    /// Get the image's height in pixels.
    //
    /// @return     The height of the image in pixels.
    size_t getHeight() const;

    /// Get the image's width in pixels.
    //
    /// @return     The width of the image in pixels.
    size_t getWidth() const;

    /// Read a scanline's worth of image data into the given buffer.
    //
    /// The amount of data read is getWidth() * getComponents().
    ///
    /// @param rgbData  The buffer for writing raw RGB data to.
    void readScanline(unsigned char* imageData);

private:

    // State needed for input.
    png_structp _pngPtr;
    png_infop _infoPtr;
    std::unique_ptr<png_bytep[]> _rowPtrs;
    std::unique_ptr<png_byte[]> _pixelData;
   
    // A counter for keeping track of the last row copied.
    size_t _currentRow;

    void init();

    // Return number of components (i.e. == 3 for RGB
    // data).
    size_t getComponents() const;

};

// Class object for writing PNG image data.
class PngOutput : public Output
{

public:

    /// Create an output object bound to a gnash::IOChannel
    //
    /// @param out      The IOChannel used for output. Must be kept alive
    ///                 throughout
    /// @param quality Unused in PNG output
    PngOutput(std::shared_ptr<IOChannel> out, size_t width,
            size_t height, int quality);
    
    ~PngOutput();

    void writeImageRGB(const unsigned char* rgbData);
    
    void writeImageRGBA(const unsigned char* rgbaData);
    
private:

    /// Initialize libpng.
    void init();

    /// Libpng structures for image and output state.
    png_structp _pngPtr;
    png_infop _infoPtr;
    
};

PngInput::~PngInput()
{
    png_destroy_read_struct(&_pngPtr, &_infoPtr, nullptr);
}

size_t
PngInput::getHeight() const
{
    assert(_pngPtr && _infoPtr);
    return png_get_image_height(_pngPtr, _infoPtr);
}

size_t
PngInput::getWidth() const
{
    assert(_pngPtr && _infoPtr);
    return png_get_image_width(_pngPtr, _infoPtr);
}

size_t
PngInput::getComponents() const
{
    return png_get_channels(_pngPtr, _infoPtr);
}

void
PngInput::readScanline(unsigned char* imageData)
{
    assert(_currentRow < getHeight());
    assert(_rowPtrs);

    // Data packed as RGB / RGBA
    const size_t size = getWidth() * getComponents();

    std::copy(_rowPtrs[_currentRow], _rowPtrs[_currentRow] + size, imageData);
    
    ++_currentRow;
}


void
PngInput::init()
{
    // Initialize png library.
    _pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, &error,
            &warning);

    if (!_pngPtr) return;

    _infoPtr = png_create_info_struct(_pngPtr);

    if (!_infoPtr) {
        png_destroy_read_struct(&_pngPtr, nullptr, nullptr);
        return;
    }
}

void
PngInput::read()
{
    // Set our user-defined reader function
    png_set_read_fn(_pngPtr, _inStream.get(), &readData);

    png_read_info(_pngPtr, _infoPtr);

    const png_byte type = png_get_color_type(_pngPtr, _infoPtr);
    const png_byte bitDepth = png_get_bit_depth(_pngPtr, _infoPtr);
    
    // Convert indexed images to RGB
    if (type == PNG_COLOR_TYPE_PALETTE) {
        log_debug("Converting palette PNG to RGB(A)");
        png_set_palette_to_rgb(_pngPtr);
    }
    
    // Convert less-than-8-bit greyscale to 8 bit.
    if (type == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
	    log_debug("Setting grey bit depth(%d) to 8", bitDepth);
#if PNG_LIBPNG_VER_MINOR < 4
        png_set_gray_1_2_4_to_8(_pngPtr);
#else
        png_set_expand_gray_1_2_4_to_8(_pngPtr);
#endif
    }

    // Apply the transparency block if it exists.
    if (png_get_valid(_pngPtr, _infoPtr, PNG_INFO_tRNS)) {
	log_debug("Applying transparency block, image is RGBA");
        png_set_tRNS_to_alpha(_pngPtr);
        _type = TYPE_RGBA;
    }

    // Make 16-bit data into 8-bit data
    if (bitDepth == 16) png_set_strip_16(_pngPtr);

    // Set the type of the image if it hasn't been set already.
    if (_type == GNASH_IMAGE_INVALID) {
        if (type & PNG_COLOR_MASK_ALPHA) {
	    log_debug("Loading PNG image with alpha");
            _type = TYPE_RGBA;
        }
        else {
		log_debug("Loading PNG image without alpha");
            _type = TYPE_RGB;
        }
    }

    // Convert 1-channel grey images to 3-channel RGB.
    if (type == PNG_COLOR_TYPE_GRAY || type == PNG_COLOR_TYPE_GRAY_ALPHA) {
	log_debug("Converting greyscale PNG to RGB(A)");
        png_set_gray_to_rgb(_pngPtr);
    }

    png_read_update_info(_pngPtr, _infoPtr);

    const size_t height = getHeight();
    const size_t width = getWidth();
    const size_t components = getComponents();

    // We must have 3 or 4-channel data by this point.
    assert((_type == TYPE_RGB && components == 3) ||
           (_type == TYPE_RGBA && components == 4));

    // Allocate space for the data
    _pixelData.reset(new png_byte[width * height * components]);

    // Allocate an array of pointers to the beginning of
    // each row.    
    _rowPtrs.reset(new png_bytep[height]);
    
    // Fill in the row pointers.
    for (size_t y = 0; y < height; ++y) {
        _rowPtrs[y] = _pixelData.get() + y * width * components;
    }

    // Read in the image using the options set.
    png_read_image(_pngPtr, _rowPtrs.get());

}

///
/// PNG output
///

PngOutput::PngOutput(std::shared_ptr<IOChannel> out, size_t width,
        size_t height, int /*quality*/)
    :
    Output(out, width, height),
    _pngPtr(nullptr),
    _infoPtr(nullptr)
{
    init();
}


PngOutput::~PngOutput()
{
    png_destroy_write_struct(&_pngPtr, &_infoPtr);
}


void
PngOutput::init()
{
    // Initialize png library.
    _pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                        nullptr, &error, &warning);
    if (!_pngPtr) return;

    _infoPtr = png_create_info_struct(_pngPtr);

    if (!_infoPtr) {
        png_destroy_write_struct(&_pngPtr, static_cast<png_infopp>(nullptr));
        return;
    }
}

void
PngOutput::writeImageRGBA(const unsigned char* rgbaData)
{
    png_set_write_fn(_pngPtr, _outStream.get(), &writeData, &flushData);

    std::unique_ptr<const png_byte*[]> rows(new const png_byte*[_height]);

    // RGBA
    const size_t components = 4;

    for (size_t y = 0; y < _height; ++y) {
        rows[y] = rgbaData + _width * y * components;
    }

    // libpng needs non-const. We'll hope it doesn't change our image.
    png_set_rows(_pngPtr, _infoPtr, const_cast<png_bytepp>(rows.get()));

    png_set_IHDR(_pngPtr, _infoPtr, _width, _height,
       8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_png(_pngPtr, _infoPtr, PNG_TRANSFORM_IDENTITY, nullptr);
}


void
PngOutput::writeImageRGB(const unsigned char* rgbData)
{
    png_set_write_fn(_pngPtr, _outStream.get(), &writeData, &flushData);

    std::unique_ptr<const png_byte*[]> rows(new const png_byte*[_height]);

    // RGB
    const size_t components = 3;

    for (size_t y = 0; y < _height; ++y) {
        rows[y] = rgbData + _width * y * components;
    }

    // libpng needs non-const. We'll hope it doesn't change our image.
    png_set_rows(_pngPtr, _infoPtr, const_cast<png_bytepp>(rows.get()));

    png_set_IHDR(_pngPtr, _infoPtr, _width, _height,
       8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_png(_pngPtr, _infoPtr, PNG_TRANSFORM_IDENTITY, nullptr);
}

} // unnamed namespace

std::unique_ptr<Input>
createPngInput(std::shared_ptr<IOChannel> in)
{
    std::unique_ptr<Input> ret(new PngInput(in));
    ret->read();
    return ret;
}

std::unique_ptr<Output>
createPngOutput(std::shared_ptr<IOChannel> o, size_t width,
                       size_t height, int quality)
{
    std::unique_ptr<Output> outChannel(new PngOutput(o, width, height, quality));
    return outChannel;
}

} // namespace image
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
