// image.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

#include "image.h"

#include "utility.h"
#include "jpeg.h"
#include "IOChannel.h"
#include "tu_file.h" // some functions take a filename, tu_file is created in that case..

#include <cstring>
#include <memory>		// for auto_ptr
#include <boost/scoped_array.hpp>

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


	rgb*	create_rgb(int width, int height)
	// Create an system-memory rgb surface.  The data order is
	// packed 24-bit, RGBRGB..., regardless of the endian-ness of
	// the CPU.
	{
		return new rgb(width, height);
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


    void rgba::mergeAlpha(const alpha& a)
    {
        const size_t size = a.size();

        assert (a.size() * 4 <= m_size);

        for (size_t i = 0; i < size; i++) {
            // These const casts are horrible, but agg has problems with
            // making data() const.
            m_data[4 * i + 3] = const_cast<image::alpha&>(a).data()[i];
        }
    }

	//
	// alpha
	//


	alpha* create_alpha(int width, int height)
	// Create an system-memory 8-bit alpha surface.
	{
		return new alpha(width, height);
	}


	alpha::alpha(int width, int height)
		:
		image_base(width, height, width, ALPHA)
	{
		assert(width > 0);
		assert(height > 0);

		//m_data = new boost::uint8_t[m_pitch * m_height];
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

		std::auto_ptr<jpeg::output> j_out ( jpeg::output::create(out, image->width(), height, quality) );

		for (size_t y = 0; y < height; ++y)
		{
			j_out->write_scanline(image->scanline(y));
		}

	}


	rgb*	read_jpeg(const char* filename)
	// Create and read a new image from the given filename, if possible.
	{
		tu_file	in(filename, "rb");	// file automatically closes when 'in' goes out of scope.
		if (! in.get_error())
		{
			return read_jpeg(&in);
		}
		else
		{
			return NULL;
		}
	}


	// Create and read a new image from the stream.
	//
	// TODO: return by auto_ptr !
	//
	rgb*	read_jpeg(gnash::IOChannel* in)
	{
		std::auto_ptr<jpeg::input> j_in ( jpeg::input::create(in) );
		if (!j_in.get()) return 0;
		
		std::auto_ptr<rgb> im ( image::create_rgb(j_in->get_width(), j_in->get_height()) );

		for (int y = 0; y < j_in->get_height(); y++)
		{
			j_in->read_scanline(im->scanline(y));
		}

		return im.release();
	}


	rgb*	read_swf_jpeg2_with_tables(jpeg::input* j_in)
	// Create and read a new image, using a input object that
	// already has tables loaded.  The IJG documentation describes
	// this as "abbreviated" format.
	{
		assert(j_in);

		j_in->start_image();

		rgb*	im = image::create_rgb(j_in->get_width(), j_in->get_height());

		for (int y = 0; y < j_in->get_height(); y++) {
			j_in->read_scanline(im->scanline(y));
		}

		j_in->finish_image();

		return im;
	}


	// For reading SWF JPEG3-style image data, like ordinary JPEG, 
	// but stores the data in rgba format.
	//
	// TODO: return by auto_ptr !
	//
	rgba* read_swf_jpeg3(gnash::IOChannel* in)
	{
		std::auto_ptr<jpeg::input> j_in ( jpeg::input::create_swf_jpeg2_header_only(in, false) );
		if ( ! j_in.get() ) return 0;
		
		j_in->start_image();

		std::auto_ptr<rgba> im ( new image::rgba(j_in->get_width(), j_in->get_height()) );

		boost::scoped_array<boost::uint8_t> line ( new boost::uint8_t[3*j_in->get_width()] );

		for (int y = 0; y < j_in->get_height(); y++) 
		{
			j_in->read_scanline(line.get());

			boost::uint8_t*	data = im->scanline(y);
			for (int x = 0; x < j_in->get_width(); x++) 
			{
				data[4*x+0] = line[3*x+0];
				data[4*x+1] = line[3*x+1];
				data[4*x+2] = line[3*x+2];
				data[4*x+3] = 255;
			}
		}

		j_in->finish_image();

		return im.release(); // TODO: return by auto_ptr !
	}


	// Write a 32-bit Targa format bitmap.  Dead simple, no compression.
	void	write_tga(gnash::IOChannel* out, rgba* im)
	{
		size_t imWidth = im->width();
		size_t imHeight = im->height();

		out->write_byte(0);
		out->write_byte(0);
		out->write_byte(2);	/* uncompressed RGB */
		out->write_le16(0);
		out->write_le16(0);
		out->write_byte(0);
		out->write_le16(0);	/* X origin */
		out->write_le16(0);	/* y origin */
		out->write_le16(imWidth);
		out->write_le16(imHeight);
		out->write_byte(32);	/* 32 bit bitmap */
		out->write_byte(0);

		for (size_t y = 0; y < imHeight; y++)
		{
			boost::uint8_t*	p = im->scanline(y);
			for (size_t x = 0; x < imWidth; x++)
			{
				out->write_byte(p[x * 4]);
				out->write_byte(p[x * 4 + 1]);
				out->write_byte(p[x * 4 + 2]);
				out->write_byte(p[x * 4 + 3]);
			}
		}
	}

} // namespace image
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
