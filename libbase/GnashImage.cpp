// Image.cpp: image data class for Gnash.
// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2002

#include "GnashImage.h"

#include <memory> 
#include <algorithm>
#include <cassert>

#ifdef USE_PNG
# include "GnashImagePng.h"
#endif
#ifdef USE_GIF
# include "GnashImageGif.h"
#endif
#include "GnashEnums.h"
#include "GnashImageJpeg.h"
#include "IOChannel.h"
#include "log.h"
#include "GnashNumeric.h"

namespace gnash {
namespace image {

namespace {
    void processAlpha(GnashImage::iterator imageData, size_t pixels);
    bool checkValidSize(size_t width, size_t height, size_t channels) {

        if (width == 0 || height == 0) return false;

        assert(channels > 0);

        std::uint32_t maxSize = std::numeric_limits<std::int32_t>::max();
        if (width >= maxSize || height >= maxSize) return false;

        maxSize /= channels;
        maxSize /= width;
        maxSize /= height;

        return maxSize > 0;
    }
}

GnashImage::GnashImage(iterator data, size_t width, size_t height,
        ImageType type, ImageLocation location)
    :
    _type(type),
    _location(location),
    _width(width),
    _height(height),
    _data(data)
{
    // Callers should check dimensions
    assert(checkValidSize(_width, _height, channels()));
}

/// Create an image allocating a buffer of height*pitch bytes
GnashImage::GnashImage(size_t width, size_t height, ImageType type,
        ImageLocation location)
    :
    _type(type),
    _location(location),
    _width(width),
    _height(height)
{
    // Constructed from external input, so restrict dimensions to avoid
    // overflow in size calculations
    if (!checkValidSize(_width, _height, channels())) {
        throw std::bad_alloc();
    }
    _data.reset(new value_type[size()]);
}

void
GnashImage::update(const_iterator data)
{
    std::copy(data, data + size(), _data.get());
}

void
GnashImage::update(const GnashImage& from)
{
    assert(size() <= from.size());
    assert(width() == from.width());
    assert(_type == from._type);
    assert(_location == from._location);
    std::copy(from.begin(), from.begin() + size(), begin());
}

ImageRGB::ImageRGB(size_t width, size_t height)
    :
    GnashImage(width, height, TYPE_RGB)
{
}

ImageRGB::~ImageRGB()
{
}

ImageRGBA::ImageRGBA(size_t width, size_t height)
    :
    GnashImage(width, height, TYPE_RGBA)
{
}

ImageRGBA::~ImageRGBA()
{
}

void
ImageRGBA::setPixel(size_t x, size_t y, value_type r, value_type g,
        value_type b, value_type a)
{
    assert(x < _width);
    assert(y < _height);

    iterator data = scanline(*this, y) + 4 * x;

    *data = r;
    *(data + 1) = g;
    *(data + 2) = b;
    *(data + 3) = a;
}


void
mergeAlpha(ImageRGBA& im, GnashImage::const_iterator alphaData,
        const size_t bufferLength)
{
    assert(bufferLength * 4 <= im.size());

    // Point to the first alpha byte
    GnashImage::iterator p = im.begin();

    // Premultiplication is also done at rendering time (at least by the
    // agg renderer).
    // TODO: use BitmapData.loadBitmap to check whether it's also done here.
    for (size_t i = 0; i < bufferLength; ++i, ++alphaData) {
        *p = std::min(*p, *alphaData);
        ++p;
        *p = std::min(*p, *alphaData);
        ++p;
        *p = std::min(*p, *alphaData);
        ++p;
        *p = *alphaData;
        ++p;
    }
}

//
// utility
//

// Write the given image to the given out stream, in jpeg format.
void
Output::writeImageData(FileType type,
    std::shared_ptr<IOChannel> out, const GnashImage& image, int quality)
{
    
    const size_t width = image.width();
    const size_t height = image.height();
    
    quality = clamp<int>(quality, 0, 100);    

    std::unique_ptr<Output> outChannel;

    switch (type) {
#ifdef USE_PNG
        case GNASH_FILETYPE_PNG:
            outChannel = createPngOutput(out, width, height, quality);
            break;
#endif
        case GNASH_FILETYPE_JPEG:
            outChannel = JpegOutput::create(out, width, height, quality);
            break;
        default:
            log_error(_("Requested to write image as unsupported filetype"));
            break;
    }

    switch (image.type()) {
        case TYPE_RGB:
            outChannel->writeImageRGB(image.begin());
            break;
        case TYPE_RGBA:
            outChannel->writeImageRGBA(image.begin());
            break;
        default:
            break;
    }

}

// See GnashEnums.h for file types.
std::unique_ptr<GnashImage>
Input::readImageData(std::shared_ptr<IOChannel> in, FileType type)
{
    std::unique_ptr<GnashImage> im;
    std::unique_ptr<Input> inChannel;

    switch (type) {
#ifdef USE_PNG
        case GNASH_FILETYPE_PNG:
            inChannel = createPngInput(in);
            break;
#endif
#ifdef USE_GIF                
        case GNASH_FILETYPE_GIF:
            inChannel = createGifInput(in);
            break;
#endif
        case GNASH_FILETYPE_JPEG:
            inChannel = JpegInput::create(in);
            break;
        default:
            break;
    }
    
    if (!inChannel.get()) return im;
    
    const size_t height = inChannel->getHeight();
    const size_t width = inChannel->getWidth();

    try {
        switch (inChannel->imageType()) {
            case TYPE_RGB:
                im.reset(new ImageRGB(width, height));
                break;
            case TYPE_RGBA:
                im.reset(new ImageRGBA(width, height));
                break;
            default:
                log_error(_("Invalid image returned"));
                return im;
        }
    }
    catch (std::bad_alloc& e) {
        // This should be caught here because ~JpegInput can also
        // throw an exception on stack unwinding and this confuses
        // remote catchers.
        log_error(_("Out of memory while trying to create %dx%d image"),
                width, height);
        return im;
    }
    

    for (size_t i = 0; i < height; ++i) {
        inChannel->readScanline(scanline(*im, i));
    }

    // The renderers expect RGBA data to be preprocessed. JPEG images are
    // never transparent, but the addition of alpha data stored elsewhere
    // in the SWF is possible; in that case, the processing happens during
    // mergeAlpha().
    if (im->type() == TYPE_RGBA) {
        processAlpha(im->begin(), width * height);
    }
    return im;
}

// For reading SWF JPEG3-style image data, like ordinary JPEG, 
// but stores the data in ImageRGBA format.
std::unique_ptr<ImageRGBA>
Input::readSWFJpeg3(std::shared_ptr<IOChannel> in)
{

    std::unique_ptr<ImageRGBA> im;

    // Calling with headerBytes as 0 has a special effect...
    std::unique_ptr<JpegInput> j_in(
            JpegInput::createSWFJpeg2HeaderOnly(in, 0));

    // If this isn't true, we should have thrown.
    assert(j_in.get());

    j_in->read();

    const size_t height = j_in->getHeight();
    const size_t width = j_in->getWidth();

    im.reset(new ImageRGBA(width, height));

    if (j_in->imageType() == TYPE_RGBA) {

        for (size_t y = 0; y < height; ++y) {
            j_in->readScanline(scanline(*im, y));
        }
    } else {
        std::unique_ptr<GnashImage::value_type[]> line(
            new GnashImage::value_type[3 * width]);

        for (size_t y = 0; y < height; ++y) {
            j_in->readScanline(line.get());

            GnashImage::iterator data = scanline(*im, y);
            for (size_t x = 0; x < width; ++x) {
                data[4*x+0] = line[3*x+0];
                data[4*x+1] = line[3*x+1];
                data[4*x+2] = line[3*x+2];
                data[4*x+3] = 255;
            }
        }
    }

    return im;
}

namespace {

void
processAlpha(GnashImage::iterator imageData, size_t pixels)
{
    GnashImage::iterator p = imageData;
    for (size_t i = 0; i < pixels; ++i) {
        GnashImage::value_type alpha = *(p + 3);
        *p = std::min(*p, alpha);
        ++p;
        *p = std::min(*p, alpha);
        ++p;
        *p = std::min(*p, alpha);
        p += 2;
    }
}

} // anonymous namespace
} // namespace image
} // namespace gnash

