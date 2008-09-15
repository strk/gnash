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
#include <memory>		// for auto_ptr
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "gnash.h" // for image file types
#include "image.h"
#include "GnashImage.h"
#include "GnashImagePng.h"
#include "GnashImageGif.h"
#include "GnashImageJpeg.h"
#include "IOChannel.h"
#include "log.h"

namespace gnash
{
namespace image
{
	//
	// ImageBase
	//

	/// Create an image taking ownership of the given buffer, supposedly of height*pitch bytes
	ImageBase::ImageBase(boost::uint8_t* data, int width, int height, int pitch, ImageType type)
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
	ImageBase::ImageBase(int width, int height, int pitch, ImageType type)
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

	void ImageBase::update(boost::uint8_t* data)
	{
		std::memcpy(_data.get(), data, _size);
	}

	void ImageBase::update(const ImageBase& from)
	{
		assert(from._pitch == _pitch);
		assert(_size <= from._size);
		assert(_type == from._type);
		std::memcpy(_data.get(), from._data.get(), _size);
	}

    void ImageBase::clear(const boost::uint8_t byteValue)
    {
        std::memset(_data.get(), byteValue, _size);
    }

	boost::uint8_t* ImageBase::scanline(size_t y)
	{
		assert(y < _height);
		return _data.get() + _pitch * y;
	}

	const boost::uint8_t* ImageBase::scanlinePointer(size_t y) const
	{
		assert(y < _height);
		return _data.get() + _pitch * y;
	}


	//
	// ImageRGB
	//

	ImageRGB::ImageRGB(int width, int height)
		:
		ImageBase( width, height,
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
		ImageBase(width, height, width * 4, GNASH_IMAGE_RGBA)
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

		boost::uint8_t*	data = scanline(y) + 4 * x;

		data[0] = r;
		data[1] = g;
		data[2] = b;
		data[3] = a;
	}


    void ImageRGBA::mergeAlpha(const boost::uint8_t* alphaData, const size_t bufferLength)
    {
        assert (bufferLength * 4 <= _size);

        for (size_t i = 0; i < bufferLength; i++) {
            _data[4 * i + 3] = alphaData[i];
        }
    }

	//
	// alpha
	//


	alpha::alpha(int width, int height)
		:
		ImageBase(width, height, width, GNASH_IMAGE_ALPHA)
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
	void writeImageData(FileType type, boost::shared_ptr<IOChannel> out, image::ImageBase* image, int quality)
	{
		
		const size_t width = image->width();
		const size_t height = image->height();
				
		std::auto_ptr<ImageOutput> outChannel;

        switch (type)
        {
            case GNASH_FILETYPE_PNG:
                outChannel = PngImageOutput::create(out, width, height, quality);
                break;
            case GNASH_FILETYPE_JPEG:
                outChannel = JpegImageOutput::create(out, width, height, quality);
                break;
            default:
                log_error("Requested to write image as unsupported filetype");
                break;
        }

        switch (image->type())
        {
            case GNASH_IMAGE_RGB:
                outChannel->writeImageRGB(image->data());
                break;
            case GNASH_IMAGE_RGBA:
                outChannel->writeImageRGBA(image->data());
                break;
            default:
                break;
        }

	}

    // See gnash.h for file types.
    std::auto_ptr<ImageBase> readImageData(boost::shared_ptr<IOChannel> in, FileType type)
    {
        std::auto_ptr<ImageBase> im (NULL);
        std::auto_ptr<ImageInput> inChannel;

        switch (type)
        {
            case GNASH_FILETYPE_PNG:
                inChannel = PngImageInput::create(in);
                break;
            case GNASH_FILETYPE_GIF:
                inChannel = GifImageInput::create(in);
                break;
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
                    im.reset(new image::ImageRGB(width, height));
                    break;
                case GNASH_IMAGE_RGBA:
                    im.reset(new image::ImageRGBA(width, height));
                    break;
                default:
                    log_error("Invalid image returned");
                    return im;
            }
        }
        catch (std::bad_alloc& e)
        {
            // This should be caught here because ~JpegImageInput can also throw
            // an exception on stack unwinding and this confuses remote catchers.
            log_error("Out of memory while trying to create %dx%d image", width, height);
            return im;
        }
        
        for (size_t i = 0; i < height; ++i)
        {
            inChannel->readScanline(im->scanline(i));
        }
        return im;
    }

	std::auto_ptr<ImageBase> readSWFJpeg2WithTables(JpegImageInput& loader)
	// Create and read a new image, using a input object that
	// already has tables loaded.  The IJG documentation describes
	// this as "abbreviated" format.
	{

		loader.startImage();

		std::auto_ptr<ImageBase> im(new image::ImageRGB(loader.getWidth(), loader.getHeight()));


		for (size_t y = 0, height = loader.getHeight(); y < height; y++) {
			loader.readScanline(im->scanline(y));
		}

		loader.finishImage();

		return im;
	}


	// For reading SWF JPEG3-style image data, like ordinary JPEG, 
	// but stores the data in ImageRGBA format.
	std::auto_ptr<ImageRGBA> readSWFJpeg3(boost::shared_ptr<gnash::IOChannel> in)
	{
	
	    std::auto_ptr<ImageRGBA> im(NULL);

        // Calling with headerBytes as 0 has a special effect...
		std::auto_ptr<JpegImageInput> j_in ( JpegImageInput::createSWFJpeg2HeaderOnly(in, 0) );
		if ( ! j_in.get() ) return im;
		
		j_in->startImage();

		im.reset(new image::ImageRGBA(j_in->getWidth(), j_in->getHeight()));

		boost::scoped_array<boost::uint8_t> line ( new boost::uint8_t[3*j_in->getWidth()] );

		for (size_t y = 0; y < j_in->getHeight(); y++) 
		{
			j_in->readScanline(line.get());

			boost::uint8_t*	data = im->scanline(y);
			for (size_t x = 0; x < j_in->getWidth(); x++) 
			{
				data[4*x+0] = line[3*x+0];
				data[4*x+1] = line[3*x+1];
				data[4*x+2] = line[3*x+2];
				data[4*x+3] = 255;
			}
		}

		j_in->finishImage();

		return im;
	}

} // namespace image
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
