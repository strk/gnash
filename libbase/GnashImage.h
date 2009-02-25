// GnashImage.h: Base class for reading image data in Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/scoped_array.hpp>
#include <memory> // for auto_ptr

#include "gnash.h" // for image file types
#include "log.h"
#include "dsodefs.h"


// Forward declarations
namespace gnash {
    class IOChannel;
    class JpegImageInput;
}

namespace gnash
{

/// The types of images handled in Gnash.
enum ImageType
{
	GNASH_IMAGE_INVALID,
	GNASH_IMAGE_RGB,
	GNASH_IMAGE_RGBA,
	GNASH_IMAGE_ALPHA
};


/// Base class for different types of bitmaps
//
/// @todo document layout of the image, like pixel data
///       order in the raw array (rows or columns first?)
///
class DSOEXPORT GnashImage
{
public:

    /// Copy an GnashImage.
    GnashImage(const GnashImage& o) throw (std::bad_alloc)
        :
        _type(o._type),
        _size(o.size()),
        _width(o.width()),
        _height(o.height()),
        _pitch(o._pitch),
        _data(new boost::uint8_t[_size])
    {
        update(o);
    }
    
    /// \brief Construct an GnashImage from a data buffer,
    /// taking ownership of the data.
    //
    /// @param data     The raw image data. This class takes ownership.
    /// @param width    The width of the image in pixels.
    /// @param height   The height of the image in pixels.
    /// @param pitch    The pitch (rowstride) of the image in bytes.
    /// @param type     The ImageType of the image.
    GnashImage(boost::uint8_t *data, int width, int height,
            int pitch, ImageType type);

    /// Construct an empty GnashImage
    //
    /// Once constructed, this image must be updated with update() before
    /// use.
    //
    /// @param width    The width of the image in pixels.
    /// @param height   The height of the image in pixels.
    /// @param pitch    The pitch (rowstride) of the image in bytes.
    /// @param type     The ImageType of the image.
    GnashImage(int width, int height, int pitch, ImageType type);

    /// Return the ImageType of the image.
    //
    /// This saves guessing when dynamic_cast is used.
    ImageType type() const { return _type; }

    /// Get the size of the image buffer
    //
    /// @return     The size of the buffer in bytes
    size_t size() const { return _size; }

    /// Get the pitch of the image buffer
    //
    /// @return     The pitch of the buffer in bytes
    size_t pitch() const { return _pitch; }

    /// Get size of a single pixel
    //
    /// @return     The size of a single pixel in bytes.
    size_t pixelSize() const
    {
        return _pitch / _width;
    }

    /// Get the image's width
    //
    /// @return     The image's width in pixels.
    size_t width() const { return _width; }

    /// Get the image's width
    //
    /// @return     The image's height in pixels.
    size_t height() const { return _height; }

    /// Copy image data from a buffer.
    //
    /// Note that this buffer MUST have the same _pitch, or unexpected things
    /// will happen. In general, it is only safe to copy from another GnashImage
    /// (or derivative thereof) or unexpected things will happen. 
    ///
    /// @param data buffer to copy data from.
    ///
    void update(boost::uint8_t* data);

    /// Copy image data from another image data
    //
    /// Note that this buffer MUST have the same _pitch and _type
    /// or an assertion will fail.
    ///
    /// @param from image to copy data from.
    ///
    void update(const GnashImage& from);
    
    /// Get access to the underlying data
    //
    /// @return     A pointer to the raw image data.
    virtual boost::uint8_t* data() { return _data.get(); }

    /// Get read-only access to the underlying data
    //
    /// @return     A read-only pointer to the raw image data.
    virtual const boost::uint8_t* data() const { return _data.get(); }

    /// Get a pointer to a given row
    //
    /// @param y    The index of the required row.
    /// @return     A pointer to the first byte of the specified row.
    boost::uint8_t* scanline(size_t y);

    /// Get a read-only pointer to a given row
    //
    /// @param y    The index of the required row.
    /// @return     A read-only pointer to the first byte of the specified
    ///             row.
    DSOEXPORT const boost::uint8_t* scanlinePointer(size_t y) const;

    virtual ~GnashImage() {}

    /// Clone the GnashImage.
    //
    /// @return     A copy of the GnashImage, owned by the caller.
    virtual std::auto_ptr<GnashImage> clone() = 0;

protected:

    const ImageType _type;

    /// Size of image buffer in bytes.
    const size_t _size;

    /// Width of image, in pixels
    const size_t _width;

    /// Height of image, in pixels
    const size_t _height;

    /// Byte offset from one row to the next
    //
    /// This is basically width in bytes of each line.
    /// For example, in an alpha image type this is equal to _width
    /// while for an RGB this is 3 times the _width.
    const size_t _pitch;

    /// Data bytes, geometry defined by members below
    boost::scoped_array<boost::uint8_t> _data;

};

/// 24-bit RGB image.  Packed data, red byte first (RGBRGB...)
class DSOEXPORT ImageRGB : public GnashImage
{

public:

    ImageRGB(int width, int height);

    ImageRGB(const ImageRGB& o)
        :
        GnashImage(o)
    {}

    ImageRGB(boost::uint8_t* data, int width, int height, int stride)
        : GnashImage(data, width, height, stride, GNASH_IMAGE_RGB)
    {}

    ~ImageRGB();

    virtual std::auto_ptr<GnashImage> clone()
    {
        return std::auto_ptr<GnashImage>(new ImageRGB(*this));
    };

};

/// 32-bit RGBA image.  Packed data, red byte first (RGBARGBA...)
class DSOEXPORT ImageRGBA : public GnashImage
{

public:

    ImageRGBA(int width, int height);

    ImageRGBA(const ImageRGBA& o)
        :
        GnashImage(o)
    {}

    ~ImageRGBA();

    /// Set pixel value 
    //
    /// TODO: move in base class ?
    ///
    void setPixel(size_t x, size_t y, boost::uint8_t r, boost::uint8_t g, boost::uint8_t b, boost::uint8_t a);

    void mergeAlpha(const boost::uint8_t* alphaData, const size_t bufferLength);

    virtual std::auto_ptr<GnashImage> clone()
    {
        return std::auto_ptr<GnashImage>(new ImageRGBA(*this));
    };

};

/// 8-bit alpha image.
class DSOEXPORT alpha : public GnashImage
{
public:
    alpha(int width, int height);

    alpha(const alpha& o)
        :
        GnashImage(o)
    {}

    ~alpha();

    virtual std::auto_ptr<GnashImage> clone()
    {
        return std::auto_ptr<GnashImage>(new alpha(*this));
    };

};

/// The base class for reading image data. 
class ImageInput : boost::noncopyable
{

public:

    /// Construct an ImageInput object to read from an IOChannel.
	//
	/// @param in   The stream to read data from. Ownership is shared
    ///             between caller and ImageInput, so it is freed
    ///             automatically when the last owner is destroyed.
	ImageInput(boost::shared_ptr<IOChannel> in) :
	    _inStream(in),
	    _type(GNASH_IMAGE_INVALID)
	{}

	virtual ~ImageInput() {}

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
class ImageOutput : boost::noncopyable
{

public:

    /// Construct an ImageOutput for writing to an IOChannel
    //
    /// @param out      The gnash::IOChannel to write the image to. Ownership
    ///                 is shared.
    /// @param width    The width of the resulting image
    /// @param height   The height of the resulting image.
    ImageOutput(boost::shared_ptr<IOChannel> out, size_t width, size_t height) :
        _width(width),
        _height(height),
        _outStream(out)
        {}

    virtual ~ImageOutput() {}
    
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
    /// @param type     The image format to write in (see libcore/gnash.h)
    /// @param out      The IOChannel to write to.
    /// @param image    The image to write.
    /// @param quality  The quality of the image output (not used for all
    ///                 formats)
    static void writeImageData(FileType type,
            boost::shared_ptr<gnash::IOChannel> out, const GnashImage& image,
            int quality);


protected:

    const size_t _width;

    const size_t _height;
    
    boost::shared_ptr<IOChannel> _outStream;

};

} // namespace gnash



#endif
