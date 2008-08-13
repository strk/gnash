// GnashImagePng.cpp: libpng wrapper for Gnash.
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

#include "utility.h"
#include "GnashImagePng.h"
#include "log.h"
#include "GnashException.h"
#include "IOChannel.h"

#include <sstream>
#include <cstring> // std::memcpy
#include <boost/scoped_array.hpp>

namespace gnash {

static void
error(png_struct*, const char* msg)
{
    std::ostringstream ss;
    ss << "PNG error: " << msg;
    throw ParserException(ss.str());
}

static void
warning(png_struct*, const char* msg)
{
    log_debug(_("PNG warning: %s"), msg);
}

static void
readData(png_structp pngptr, png_bytep data, png_size_t length)
{
    // Do not call unless the PNG exists.
    assert(pngptr);
    IOChannel* in = reinterpret_cast<IOChannel*>(png_get_io_ptr(pngptr));
    in->read(reinterpret_cast<char*>(data), length);
}

static void
writeData(png_structp pngptr, png_bytep data, png_size_t length)
{
    // Do not call unless the PNG exists.
    assert(pngptr);
    IOChannel* out = reinterpret_cast<IOChannel*>(png_get_io_ptr(pngptr));
    out->write(reinterpret_cast<char*>(data), length);
}

static void
flushData(png_structp /*pngptr*/)
{

}

PngImageInput::PngImageInput(boost::shared_ptr<IOChannel> in) :
    ImageInput(in),
    _pngPtr(0),
    _infoPtr(0),
    _rowPtrs(0),
    _currentRow(0)
{
    init();
}

PngImageInput::~PngImageInput()
{
    png_destroy_read_struct(&_pngPtr, &_infoPtr, (png_infopp)NULL);
}

size_t
PngImageInput::getHeight() const
{
    assert (_pngPtr && _infoPtr);
    return png_get_image_height(_pngPtr, _infoPtr);
}

size_t
PngImageInput::getWidth() const
{
    assert (_pngPtr && _infoPtr);
    return png_get_image_width(_pngPtr, _infoPtr);
}

void
PngImageInput::readScanline(unsigned char* rgbData)
{
    assert (_currentRow < getHeight());
    assert (_rowPtrs);
   
    const size_t components = getComponents();

    // Bit-depth must be 8!
    assert (png_get_rowbytes(_pngPtr, _infoPtr) == components * getWidth());
    
    switch (components)
    {

        case 3:
            // Data packed as RGB
            std::memcpy(rgbData, _rowPtrs[_currentRow],
                        getWidth() * components);
            break;

        case 1:
            // Greyscale data: we have to convert to RGB
            for (size_t x = 0; x < getWidth(); ++x)
            {
                std::memset(rgbData, _rowPtrs[_currentRow][x], 3);
                rgbData += 3;
            }
            break;

        default:
            boost::format fmt = boost::format("Cannot handle pixels "
                                     "with %d channels!") % components;
            throw ParserException(fmt.str());

    }
    
    ++_currentRow;
}

size_t
PngImageInput::getComponents() const
{
    return png_get_channels(_pngPtr, _infoPtr);
}

void
PngImageInput::init()
{
    // Initialize png library.
    _pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                        NULL, &error, &warning);
    if (!_pngPtr) return;

    _infoPtr = png_create_info_struct(_pngPtr);

    if (!_infoPtr)
    {
        png_destroy_read_struct(&_pngPtr, (png_infopp)NULL, (png_infopp)NULL);
        return;
    }
}

void
PngImageInput::read()
{
    // Set our user-defined reader function
    png_set_read_fn(_pngPtr, _inStream.get(), &readData);
    
    // read PNG into memory.
    // TODO: sort out transform options.
    // At present we strip alpha because Gnash can't handle it and reduce the bit depth
    // to 8 bits, as that's all the image class can handle.
    png_read_png(_pngPtr, _infoPtr, PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_STRIP_16, NULL);

    // Allocate and fill array of pointers to each row. This should be
    // cleaned up by png_destroy_read_struct.
    _rowPtrs = png_get_rows(_pngPtr, _infoPtr);    
}

///
/// PNG output
///

PngImageOutput::PngImageOutput(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int /* quality */)
    :
    ImageOutput(out, width, height),
    _pngPtr(0),
    _infoPtr(0)
{
    init();
}


PngImageOutput::~PngImageOutput()
{
    png_destroy_write_struct(&_pngPtr, &_infoPtr);
}


void
PngImageOutput::init()
{
    // Initialize png library.
    _pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                        NULL, &error, &warning);
    if (!_pngPtr) return;

    _infoPtr = png_create_info_struct(_pngPtr);

    if (!_infoPtr)
    {
        png_destroy_write_struct(&_pngPtr, (png_infopp)NULL);
        return;
    }
}

void
PngImageOutput::writeImageRGBA(unsigned char* rgbaData)
{
    png_set_write_fn(_pngPtr, _outStream.get(), &writeData, &flushData);

    boost::scoped_array<png_bytep> rows(new png_bytep[_height]);

    // RGBA
    const size_t components = 4;

    for (size_t y = 0; y < _height; ++y)
    {
        rows[y] = rgbaData + _width * y * components;
    }

    png_set_rows(_pngPtr, _infoPtr, rows.get());

    png_set_IHDR(_pngPtr, _infoPtr, _width, _height,
       8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_png(_pngPtr, _infoPtr, PNG_TRANSFORM_IDENTITY, NULL);
}


void
PngImageOutput::writeImageRGB(unsigned char* rgbData)
{
    png_set_write_fn(_pngPtr, _outStream.get(), &writeData, &flushData);

    boost::scoped_array<png_bytep> rows(new png_bytep[_height]);

    // RGB
    const size_t components = 3;

    for (size_t y = 0; y < _height; ++y)
    {
        rows[y] = rgbData + _width * y * components;
    }

    png_set_rows(_pngPtr, _infoPtr, rows.get());

    png_set_IHDR(_pngPtr, _infoPtr, _width, _height,
       8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_png(_pngPtr, _infoPtr, PNG_TRANSFORM_IDENTITY, NULL);
}


std::auto_ptr<ImageOutput>
PngImageOutput::create(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int quality)
{
    std::auto_ptr<ImageOutput> outChannel(new PngImageOutput(out, width, height, quality));
    return outChannel;
}


} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
