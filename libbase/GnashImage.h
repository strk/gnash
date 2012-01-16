// GnashImage.h: Base class for reading image data in Gnash.
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

// The GnashImage class and subclasses are partly based on the public domain
// work of Thatcher Ulrich <tu@tulrich.com> 2002

#ifndef GNASH_GNASHIMAGE_H
#define GNASH_GNASHIMAGE_H

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/scoped_array.hpp>
#include <memory> 

#include "GnashEnums.h"
#include "log.h"
#include "dsodefs.h"

// Forward declarations
namespace gnash {
    class IOChannel;
}

namespace gnash {

/// Image handling functions and classes.
namespace image {

/// The types of images handled in Gnash.
enum ImageType
{
    GNASH_IMAGE_INVALID,
    TYPE_RGB,
    TYPE_RGBA
};

/// The locations of images handled in Gnash.
enum ImageLocation
{
    GNASH_IMAGE_CPU = 1,
    GNASH_IMAGE_GPU
};

inline size_t
numChannels(ImageType t)
{
    switch (t) {
        case TYPE_RGBA:
            return 4;
        case TYPE_RGB:
            return 3;
        default:
            std::abort();
    }
}

/// Base class for different types of bitmaps
//
/// 1. Bytes are packed in RGB(A) order.
/// 2. Rowstride is equal to channels * width
class DSOEXPORT GnashImage : boost::noncopyable
{
public:

    typedef boost::uint8_t value_type;
    typedef boost::scoped_array<value_type> container_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    virtual ~GnashImage() {}

    /// Return the ImageType of the image.
    //
    /// This saves guessing when dynamic_cast is used.
    ImageType type() const {
        return _type;
    }

    /// Return the ImageLocation of the image.
    //
    /// This saves guessing when dynamic_cast is used.
    ImageLocation location() const {
        return _location;
    }

    /// Get the size of the image buffer
    //
    /// @return     The size of the buffer in bytes
    size_t size() const {
        return stride() * _height;
    }

    /// Get the pitch of the image buffer
    //
    /// @return     The rowstride of the buffer in bytes
    virtual size_t stride() const {
        return _width * channels();
    }

    /// Get the number of channels
    //
    /// @return     The number of channels
    size_t channels() const {
        return numChannels(_type);
    }

    /// Get the image's width
    //
    /// @return     The image's width in pixels.
    size_t width() const {
        return _width;
    }

    /// Get the image's width
    //
    /// @return     The image's height in pixels.
    size_t height() const {
        return _height;
    }

    /// Copy image data from a buffer.
    //
    /// Note that this buffer MUST have the same rowstride and type, or
    /// unexpected things will happen. In general, it is only safe to copy
    /// from another GnashImage or unexpected things will happen. 
    ///
    /// @param data     buffer to copy data from.
    void update(const_iterator data);

    /// Copy image data from another image data
    //
    /// Note that this buffer must have the same rowstride and type
    ///
    /// @param from     image to copy data from.
    void update(const GnashImage& from);
    
    /// Access the raw data.
    virtual iterator begin() {
        return _data.get();
    }

    /// Access the raw data
    virtual const_iterator begin() const {
        return _data.get();
    }

    /// An iterator to the end of the data.
    iterator end() {
        return begin() + size();
    }

    /// An iterator to the end of the data.
    const_iterator end() const {
        return begin() + size();
    }

protected:

    /// Construct a GnashImage from a data buffer, taking ownership of the data.
    //
    /// @param data     The raw image data. This class takes ownership.
    /// @param width    The width of the image in pixels.
    /// @param height   The height of the image in pixels.
    /// @param pitch    The pitch (rowstride) of the image in bytes.
    /// @param type     The ImageType of the image.
    GnashImage(iterator data, size_t width, size_t height, ImageType type,
            ImageLocation location = GNASH_IMAGE_CPU);

    /// Construct an empty GnashImage
    //
    /// Note: there is an arbitrary limit of boost::int32_t::max bytes for the
    /// total size of the bitmap constructed with this constructor.
    //
    /// @param width    The width of the image in pixels.
    /// @param height   The height of the image in pixels.
    /// @param type     The ImageType of the image.
    GnashImage(size_t width, size_t height, ImageType type,
               ImageLocation location = GNASH_IMAGE_CPU);

    /// The type of the image: RGBA or RGB.
    const ImageType _type;

    /// Image data location (CPU or GPU)
    const ImageLocation _location;

    /// Width of image, in pixels
    const size_t _width;

    /// Height of image, in pixels
    const size_t _height;

    /// Data if held in this class
    container_type _data;

};

/// 24-bit RGB bitmap
//
/// Channels are in RGB order.
class DSOEXPORT ImageRGB : public GnashImage
{
public:

    /// Create an empty RGB image with uninitialized data.
    ImageRGB(size_t width, size_t height);

    /// Create an ImageRGB taking ownership of the data.
    ImageRGB(iterator data, size_t width, size_t height)
        :
        GnashImage(data, width, height, TYPE_RGB)
    {}

    virtual ~ImageRGB();
};

/// 32-bit RGBA bitmap
//
/// Channels are in RGBA order.
class DSOEXPORT ImageRGBA : public GnashImage
{

public:

    /// Create an empty RGB image with uninitialized data.
    ImageRGBA(size_t width, size_t height);

    ImageRGBA(iterator data, size_t width, size_t height)
        :
        GnashImage(data, width, height, TYPE_RGBA)
    {}
    
    ~ImageRGBA();

    /// Set pixel value 
    //
    /// TODO: move in base class ?
    ///
    void setPixel(size_t x, size_t y, value_type r, value_type g, value_type b,
            value_type a);
};

/// The base class for reading image data. 
class Input : boost::noncopyable
{
public:

    /// Construct an Input object to read from an IOChannel.
    //
    /// @param in   The stream to read data from. Ownership is shared
    ///             between caller and Input, so it is freed
    ///             automatically when the last owner is destroyed.
    Input(boost::shared_ptr<IOChannel> in)
        :
        _inStream(in),
        _type(GNASH_IMAGE_INVALID)
    {}

    virtual ~Input() {}

    /// Begin processing the image data.
    virtual void read() = 0;

    /// Get the image's height in pixels.
    //
    /// @return     The height of the image in pixels.
    virtual size_t getHeight() const = 0;

    /// Get the image's width in pixels.
    //
    /// @return     The width of the image in pixels.
    virtual size_t getWidth() const = 0;

    /// Get number of components (channels)
    //
    /// @return     The number of components, e.g. 3 for RGB
    virtual size_t getComponents() const = 0;

    /// Read a scanline's worth of image data into the given buffer.
    //
    /// @param rgbData  The buffer for writing raw RGB data to.
    virtual void readScanline(unsigned char* rgbData) = 0;

    /// Get the ImageType of the image.
    //
    /// @return The type of the image. This is GNASH_IMAGE_INVALID
    ///         at least until read() has been called and reads some
    ///         valid data.
    ImageType imageType() { return _type; }

    /// \brief
    /// For reading SWF JPEG3-style image data, like ordinary JPEG, 
    /// but stores the data in ImageRGBA format.
    DSOEXPORT static std::auto_ptr<ImageRGBA> readSWFJpeg3(
            boost::shared_ptr<gnash::IOChannel> in);

    /// Read image data from an IOChannel into an GnashImage.
    //
    /// @param in   The IOChannel to read the image from.
    /// @param type The type of image to read.
    /// @return     An GnashImage with the read image data. If type
    ///             is an unsupported FileType or image reading fails,
    ///             a NULL auto_ptr is returned.
    DSOEXPORT static std::auto_ptr<GnashImage> readImageData(
            boost::shared_ptr<gnash::IOChannel> in, FileType type);

protected:

    boost::shared_ptr<IOChannel> _inStream;

    ImageType _type;

};

// Base class for writing image data.
class Output : boost::noncopyable
{

public:

    /// Construct an Output for writing to an IOChannel
    //
    /// @param out      The gnash::IOChannel to write the image to. Ownership
    ///                 is shared.
    /// @param width    The width of the resulting image
    /// @param height   The height of the resulting image.
    Output(boost::shared_ptr<IOChannel> out, size_t width, size_t height)
        :
        _width(width),
        _height(height),
        _outStream(out)
    {}

    virtual ~Output() {}
    
    /// Write RGB image data using the parameters supplied at construction.
    //
    /// @param rgbData  The raw RGB image data to write as an image.
    virtual void writeImageRGB(const unsigned char* rgbData) = 0;
    
    /// Write RGBA image data using the parameters supplied at construction.
    //
    /// @param rgbaData  The raw RGBA image data to write as an image.
    virtual void writeImageRGBA(const unsigned char* /*rgbaData*/)
    {
        log_error(_("This image format does not support writing RGBA images"));
    }

    /// Write the given image to the given IOChannel in a specified format.
    //
    /// @param type     The image format to write in (see GnashEnums.h)
    /// @param out      The IOChannel to write to.
    /// @param image    The image to write.
    /// @param quality  The quality of the image output, from 0..100. Values
    ///                 outside this range will be clamped to the minimum or
    ///                 maxium value. The quality is not used for all
    ///                 formats.
    DSOEXPORT static void writeImageData(FileType type,
            boost::shared_ptr<gnash::IOChannel> out, const GnashImage& image,
            int quality);

protected:

    const size_t _width;

    const size_t _height;
    
    boost::shared_ptr<IOChannel> _outStream;

};

/// Get a pointer to a given row of any image.
//
/// @param row    The index of the required row.
/// @return     A pointer to the first byte of the specified row.
inline GnashImage::iterator
scanline(GnashImage& im, size_t row)
{
    assert(row < im.height());
    return im.begin() + im.stride() * row;
}

/// Get a read-only pointer to a given row of any image.
//
/// @param y    The index of the required row.
/// @return     A read-only pointer to the first byte of the specified row.
inline GnashImage::const_iterator
scanline(const GnashImage& im, size_t row)
{
    assert(row < im.height());
    return im.begin() + im.stride() * row;
}

DSOEXPORT void mergeAlpha(ImageRGBA& im, GnashImage::const_iterator alphaData,
        const size_t bufferLength);

} // namespace image
} // namespace gnash

#endif
