// Image.cpp: image data class for Gnash.
// 
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2002

#include <cstring>
#include <memory>        // for auto_ptr
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "gnash.h" // for image file types
#include "GnashImage.h"
#ifdef USE_PNG
# include "GnashImagePng.h"
#endif
#ifdef USE_GIF
# include "GnashImageGif.h"
#endif
#include "GnashImageJpeg.h"
#include "IOChannel.h"
#include "log.h"

namespace gnash
{

//
// GnashImage
//

/// Create an image taking ownership of the given buffer, supposedly of height*pitch bytes
GnashImage::GnashImage(boost::uint8_t* data, int width,
        int height, int pitch, ImageType type)
    :
    _type(type),
    _size(height*pitch),
    _width(width),
    _height(height),
    _pitch(pitch),
    _data(data)
{
}

/// Create an image allocating a buffer of height*pitch bytes
GnashImage::GnashImage(int width, int height,
        int pitch, ImageType type)
    :
    _type(type),
    _size(height*pitch),
    _width(width),
    _height(height),
    _pitch(pitch),
    _data(new boost::uint8_t[_size])
{
    assert(pitch >= width);
}

void GnashImage::update(boost::uint8_t* data)
{
    std::memcpy(this->data(), data, _size);
}

void GnashImage::update(const GnashImage& from)
{
    assert(from._pitch == _pitch);
    assert(_size <= from._size);
    assert(_type == from._type);
    std::memcpy(data(), from.data(), _size);
}

boost::uint8_t* GnashImage::scanline(size_t y)
{
    assert(y < _height);
    return data() + _pitch * y;
}

const boost::uint8_t* GnashImage::scanlinePointer(size_t y) const
{
    assert(y < _height);
    return data() + _pitch * y;
}


//
// ImageRGB
//

ImageRGB::ImageRGB(int width, int height)
    :
    GnashImage( width, height,
        width * 3, GNASH_IMAGE_RGB)
{
    assert(width > 0);
    assert(height > 0);
}

ImageRGB::~ImageRGB()
{
}


//
// ImageRGBA
//


ImageRGBA::ImageRGBA(int width, int height)
    :
    GnashImage(width, height, width * 4, GNASH_IMAGE_RGBA)
{
    assert(width > 0);
    assert(height > 0);
    assert(_pitch >= _width * 4);
    assert((_pitch & 3) == 0);
}

ImageRGBA::~ImageRGBA()
{
}


void ImageRGBA::setPixel(size_t x, size_t y, boost::uint8_t r, boost::uint8_t g, boost::uint8_t b, boost::uint8_t a)
// Set the pixel at the given position.
{
    assert(x < _width);
    assert(y < _height);

    boost::uint8_t* data = scanline(y) + 4 * x;

    data[0] = r;
    data[1] = g;
    data[2] = b;
    data[3] = a;
}


void
ImageRGBA::mergeAlpha(const boost::uint8_t* alphaData,
        const size_t bufferLength)
{
    assert (bufferLength * 4 <= _size);

    // Point to the first alpha byte
    boost::uint8_t* p = data();

    // Set each 4th byte to the correct alpha value.
    for (size_t i = 0; i < bufferLength; ++i) {
        p += 3;
        *p = *alphaData;
        ++p;
        ++alphaData;
    }
}

//
// alpha
//


alpha::alpha(int width, int height)
    :
    GnashImage(width, height, width, GNASH_IMAGE_ALPHA)
{
    assert(width > 0);
    assert(height > 0);
}


alpha::~alpha()
{
}

//
// utility
//

// Write the given image to the given out stream, in jpeg format.
void
ImageOutput::writeImageData(FileType type,
    boost::shared_ptr<IOChannel> out, const GnashImage& image, int quality)
{
    
    const size_t width = image.width();
    const size_t height = image.height();
            
    std::auto_ptr<ImageOutput> outChannel;

    switch (type)
    {
#ifdef USE_PNG
        case GNASH_FILETYPE_PNG:
            outChannel = PngImageOutput::create(out, width,
                    height, quality);
            break;
#endif
        case GNASH_FILETYPE_JPEG:
            outChannel = JpegImageOutput::create(out, width,
                    height, quality);
            break;
        default:
            log_error("Requested to write image as unsupported filetype");
            break;
    }

    switch (image.type())
    {
        case GNASH_IMAGE_RGB:
            outChannel->writeImageRGB(image.data());
            break;
        case GNASH_IMAGE_RGBA:
            outChannel->writeImageRGBA(image.data());
            break;
        default:
            break;
    }

}

// See gnash.h for file types.
std::auto_ptr<GnashImage>
ImageInput::readImageData(boost::shared_ptr<IOChannel> in, FileType type)
{
    std::auto_ptr<GnashImage> im (NULL);
    std::auto_ptr<ImageInput> inChannel;

    switch (type)
    {
#ifdef USE_PNG
        case GNASH_FILETYPE_PNG:
            inChannel = PngImageInput::create(in);
            break;
#endif
#ifdef USE_GIF                
        case GNASH_FILETYPE_GIF:
            inChannel = GifImageInput::create(in);
            break;
#endif
        case GNASH_FILETYPE_JPEG:
            inChannel = JpegImageInput::create(in);
            break;
        default:
            break;
    }
    
    if (!inChannel.get()) return im;
    
    const size_t height = inChannel->getHeight();
    const size_t width = inChannel->getWidth();

    try
    {
        switch (inChannel->imageType())
        {
            case GNASH_IMAGE_RGB:
                im.reset(new ImageRGB(width, height));
                break;
            case GNASH_IMAGE_RGBA:
                im.reset(new ImageRGBA(width, height));
                break;
            default:
                log_error("Invalid image returned");
                return im;
        }
    }
    catch (std::bad_alloc& e)
    {
        // This should be caught here because ~JpegImageInput can also
        // throw an exception on stack unwinding and this confuses
        // remote catchers.
        log_error("Out of memory while trying to create %dx%d image",
                width, height);
        return im;
    }
    
    for (size_t i = 0; i < height; ++i)
    {
        inChannel->readScanline(im->scanline(i));
    }
    return im;
}

// For reading SWF JPEG3-style image data, like ordinary JPEG, 
// but stores the data in ImageRGBA format.
std::auto_ptr<ImageRGBA>
ImageInput::readSWFJpeg3(boost::shared_ptr<IOChannel> in)
{

    std::auto_ptr<ImageRGBA> im;

    // Calling with headerBytes as 0 has a special effect...
    std::auto_ptr<JpegImageInput> j_in(
            JpegImageInput::createSWFJpeg2HeaderOnly(in, 0));

    // If this isn't true, we should have thrown.
    assert(j_in.get());

    j_in->read();

    const size_t height = j_in->getHeight();
    const size_t width = j_in->getWidth();

    im.reset(new ImageRGBA(width, height));

    boost::scoped_array<boost::uint8_t> line(new boost::uint8_t[3 * width]);

    for (size_t y = 0; y < height; ++y) 
    {
        j_in->readScanline(line.get());

        boost::uint8_t* data = im->scanline(y);
        for (size_t x = 0; x < width; ++x) 
        {
            data[4*x+0] = line[3*x+0];
            data[4*x+1] = line[3*x+1];
            data[4*x+2] = line[3*x+2];
            data[4*x+3] = 255;
        }
    }

    return im;
}

} // namespace gnash

