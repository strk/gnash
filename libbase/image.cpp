// image.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

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

namespace gnash
{
namespace image
{
	//
	// image_base
	//

	/// Create an image taking ownership of the given buffer, supposedly of height*pitch bytes
	image_base::image_base(boost::uint8_t* data, int width, int height, int pitch, id_image type)
		:
		m_type(type),
		m_size(height*pitch),
		m_width(width),
		m_height(height),
		m_pitch(pitch),
		m_data(data)
	{
	}

	/// Create an image allocating a buffer of height*pitch bytes
	image_base::image_base(int width, int height, int pitch, id_image type)
		:
		m_type(type),
		m_size(height*pitch),
		m_width(width),
		m_height(height),
		m_pitch(pitch),
		m_data(new boost::uint8_t[m_size])
	{
		assert(pitch >= width);
	}

	void image_base::update(boost::uint8_t* data)
	{
		std::memcpy(m_data.get(), data, m_size);
	}

	void image_base::update(const image_base& from)
	{
		assert(from.m_pitch == m_pitch);
		assert(m_size <= from.m_size);
		assert(m_type == from.m_type);
		std::memcpy(m_data.get(), const_cast<image_base&>(from).data(), m_size);
	}

    void image_base::clear(const boost::uint8_t byteValue)
    {
        std::memset(m_data.get(), byteValue, m_size);
    }

	boost::uint8_t* image_base::scanline(size_t y)
	{
		assert(y < m_height);
		return m_data.get() + m_pitch * y;
	}


	//
	// rgb
	//

	rgb::rgb(int width, int height)
		:
		image_base( width, height,
			(width * 3 + 3) & ~3, // round pitch up to nearest 4-byte boundary
			RGB)
	{
		assert(width > 0);
		assert(height > 0);
		assert(m_pitch >= m_width * 3);
		assert((m_pitch & 3) == 0);
	}

	rgb::~rgb()
	{
	}


	//
	// rgba
	//


	rgba::rgba(int width, int height)
		:
		image_base(width, height, width * 4, RGBA)
	{
		assert(width > 0);
		assert(height > 0);
		assert(m_pitch >= m_width * 4);
		assert((m_pitch & 3) == 0);
	}

	rgba::~rgba()
	{
	}


	void rgba::set_pixel(size_t x, size_t y, boost::uint8_t r, boost::uint8_t g, boost::uint8_t b, boost::uint8_t a)
	// Set the pixel at the given position.
	{
		assert(x < m_width);
		assert(y < m_height);

		boost::uint8_t*	data = scanline(y) + 4 * x;

		data[0] = r;
		data[1] = g;
		data[2] = b;
		data[3] = a;
	}


    void rgba::mergeAlpha(const boost::uint8_t* alphaData, const size_t bufferLength)
    {
        assert (bufferLength * 4 <= m_size);

        for (size_t i = 0; i < bufferLength; i++) {
            m_data[4 * i + 3] = alphaData[i];
        }
    }

	//
	// alpha
	//


	alpha::alpha(int width, int height)
		:
		image_base(width, height, width, ALPHA)
	{
		assert(width > 0);
		assert(height > 0);
	}


	alpha::~alpha()
	{
	}


	bool	alpha::operator==(const alpha& a) const
	// Bitwise content comparison.
	{
		if (m_width != a.m_width
		    || m_height != a.m_height)
		{
			return false;
		}

		for (int j = 0, n = m_height; j < n; j++)
		{
			if (memcmp(scanline(j), a.scanline(j), m_width))
			{
				// Mismatch.
				return false;
			}
		}

		// Images are identical.
		return true;
	}

	//
	// utility
	//


	// Write the given image to the given out stream, in jpeg format.
	void	write_jpeg(gnash::IOChannel* out, rgb* image, int quality)
	{
		size_t height = image->height();

		std::auto_ptr<JpegImageOutput> j_out ( JpegImageOutput::create(out, image->width(), height, quality) );

		for (size_t y = 0; y < height; ++y)
		{
			j_out->write_scanline(image->scanline(y));
		}

	}

	// Create and read a new image from the stream.
	//
	// TODO: return by auto_ptr !
	//
	rgb*	read_jpeg(boost::shared_ptr<gnash::IOChannel> in)
	{
		std::auto_ptr<ImageInput> j_in (JpegImageInput::create(in));
		if (!j_in.get()) return 0;
		
		std::auto_ptr<rgb> im ( new image::rgb(j_in->getWidth(), j_in->getHeight()) );

		for (size_t y = 0; y < j_in->getHeight(); y++)
		{
			j_in->readScanline(im->scanline(y));
		}

		return im.release();
	}

    // See gnash.h for file types.
    std::auto_ptr<rgb> readImageData(boost::shared_ptr<IOChannel> in, FileType type)
    {
        std::auto_ptr<rgb> im (NULL);
        std::auto_ptr<ImageInput> infile;

        switch (type)
        {
            case GNASH_FILETYPE_PNG:
                infile = PngImageInput::create(in);
                break;
            case GNASH_FILETYPE_GIF:
                infile = GifImageInput::create(in);
                break;
            case GNASH_FILETYPE_JPEG:
                infile = JpegImageInput::create(in);
                break;
            default:
                break;
        }
        
        if (!infile.get()) return im;
        
        im.reset(new image::rgb(infile->getWidth(), infile->getHeight()));
        
        for (size_t i = 0, e = infile->getHeight(); i < e; ++i)
        {
            infile->readScanline(im->scanline(i));
        }
        return im;
    }

	rgb*	read_swf_jpeg2_with_tables(JpegImageInput* j_in)
	// Create and read a new image, using a input object that
	// already has tables loaded.  The IJG documentation describes
	// this as "abbreviated" format.
	{
		assert(j_in);

		j_in->start_image();

		std::auto_ptr<rgb> im(new image::rgb(j_in->getWidth(), j_in->getHeight()));

		for (size_t y = 0; y < j_in->getHeight(); y++) {
			j_in->readScanline(im->scanline(y));
		}

		j_in->finish_image();

		return im.release();
	}


	// For reading SWF JPEG3-style image data, like ordinary JPEG, 
	// but stores the data in rgba format.
	std::auto_ptr<rgba> readSWFJpeg3(boost::shared_ptr<gnash::IOChannel> in)
	{
	
	    std::auto_ptr<rgba> im(NULL);

        // Calling with headerBytes as 0 has a special effect...
		std::auto_ptr<JpegImageInput> j_in ( JpegImageInput::create_swf_jpeg2_header_only(in, 0) );
		if ( ! j_in.get() ) return im;
		
		j_in->start_image();

		im.reset(new image::rgba(j_in->getWidth(), j_in->getHeight()));

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

		j_in->finish_image();

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
