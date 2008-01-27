// image.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

#include "image.h"

#include "utility.h"
#include "jpeg.h"
#include "tu_file.h"
#include <cstring>
#include <memory>		// for auto_ptr
#include <boost/scoped_array.hpp>

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
		m_data(data),
		m_width(width),
		m_height(height),
		m_pitch(pitch)
	{
	}

	/// Create an image allocating a buffer of height*pitch bytes
	image_base::image_base(int width, int height, int pitch, id_image type)
		:
		m_type(type),
		m_size(height*pitch),
		m_data(new boost::uint8_t[m_size]),
		m_width(width),
		m_height(height),
		m_pitch(pitch)
	{
		assert(pitch >= width);
	}

	void image_base::update(boost::uint8_t* data)
	{
		memcpy(m_data.get(), data, m_size);
	}

	void image_base::update(const image_base& from)
	{
		assert(from.m_pitch == m_pitch);
		assert(m_size <= from.m_size);
		assert(m_type == from.m_type);
		memcpy(m_data.get(), const_cast<image_base&>(from).data(), m_size);
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

	bool rgb::make_next_miplevel()
	{
		assert(m_data.get());
		assert(m_type == RGB);

		size_t imWidth = m_width;
		size_t imHeight = m_height;

		size_t new_w = imWidth >> 1;
		size_t new_h = imHeight >> 1;
		if (new_w < 1) new_w = 1;
		if (new_h < 1) new_h = 1;

		if (new_w * 2 != imWidth  || new_h * 2 != imHeight)
		{
			// Image can't be shrunk along (at least) one
			// of its dimensions, so don't bother
			// resampling.  Technically we should, but
			// it's pretty useless at this point.  Just
			// change the image dimensions and leave the
			// existing pixels.
			return false;
		}

		size_t new_pitch = new_w * 3;

		// Round pitch up to the nearest 4-byte boundary.
		new_pitch = (new_pitch + 3) & ~3;

		// Resample.  Simple average 2x2 --> 1, in-place.
		size_t	pitch = m_pitch;
		for (size_t j = 0; j < new_h; j++) {
			boost::uint8_t*	out = m_data.get() + j * new_pitch;
			boost::uint8_t*	in = m_data.get() + (j << 1) * pitch;
			for (size_t i = 0; i < new_w; i++) {
				int	r, g, b;
				r = (*(in + 0) + *(in + 3) + *(in + 0 + pitch) + *(in + 3 + pitch));
				g = (*(in + 1) + *(in + 4) + *(in + 1 + pitch) + *(in + 4 + pitch));
				b = (*(in + 2) + *(in + 5) + *(in + 2 + pitch) + *(in + 5 + pitch));
				*(out + 0) = r >> 2;
				*(out + 1) = g >> 2;
				*(out + 2) = b >> 2;
				out += 3;
				in += 6;
			}
		}

		// Munge image's members to reflect the shrunken image.
		m_width = new_w;
		m_height = new_h;
		m_pitch = new_pitch;
		m_size = m_height*m_pitch;

		assert(m_pitch >= m_width);

		return true;
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


	rgba*	create_rgba(int width, int height)
	// Create an system-memory rgb surface.  The data order is
	// packed 32-bit, RGBARGBA..., regardless of the endian-ness
	// of the CPU.
	{
		return new rgba(width, height);
	}


	void	rgba::set_pixel(size_t x, size_t y, boost::uint8_t r, boost::uint8_t g, boost::uint8_t b, boost::uint8_t a)
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

	// Set alpha value for given pixel 
	void	rgba::set_alpha(size_t x, size_t y, boost::uint8_t a)
	{
		assert(x < m_width);
		assert(y < m_height);

		boost::uint8_t*	data = scanline(y) + 4 * x;

		data[3] = a;
	}

	bool	rgba::make_next_miplevel()
	{
		assert(m_data.get());
		assert(m_type == RGBA);

		size_t	new_w = m_width >> 1;
		size_t	new_h = m_height >> 1;
		if (new_w < 1) new_w = 1;
		if (new_h < 1) new_h = 1;

		if (new_w * 2 != m_width  || new_h * 2 != m_height)
		{
			// Image can't be shrunk along (at least) one
			// of its dimensions, so don't bother
			// resampling.  Technically we should, but
			// it's pretty useless at this point.  Just
			// change the image dimensions and leave the
			// existing pixels.
			return false;
		}

		size_t	new_pitch = new_w * 4;

		// Resample.  Simple average 2x2 --> 1, in-place.
		size_t	pitch = m_pitch;
		for (size_t j = 0; j < new_h; j++) {
			boost::uint8_t*	out = ((boost::uint8_t*) m_data.get()) + j * new_pitch;
			boost::uint8_t*	in = ((boost::uint8_t*) m_data.get()) + (j << 1) * pitch;
			for (size_t i = 0; i < new_w; i++) {
				int	r, g, b, a;
				r = (*(in + 0) + *(in + 4) + *(in + 0 + pitch) + *(in + 4 + pitch));
				g = (*(in + 1) + *(in + 5) + *(in + 1 + pitch) + *(in + 5 + pitch));
				b = (*(in + 2) + *(in + 6) + *(in + 2 + pitch) + *(in + 6 + pitch));
				a = (*(in + 3) + *(in + 7) + *(in + 3 + pitch) + *(in + 7 + pitch));
				*(out + 0) = r >> 2;
				*(out + 1) = g >> 2;
				*(out + 2) = b >> 2;
				*(out + 3) = a >> 2;
				out += 4;
				in += 8;
			}
		}

		// Munge image's members to reflect the shrunken image.
		m_width = new_w;
		m_height = new_h;
		m_pitch = new_pitch;
		m_size = m_height*m_pitch;

		assert(m_pitch >= m_width);

		return true;
	}

	//
	// alpha
	//


	alpha*	create_alpha(int width, int height)
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

	bool alpha::make_next_miplevel()
	{
		assert(m_data.get());
		assert(m_type == ALPHA);

		size_t	new_w = m_width >> 1;
		size_t	new_h = m_height >> 1;
		if (new_w < 1) new_w = 1;
		if (new_h < 1) new_h = 1;

		if (new_w * 2 != m_width || new_h * 2 != m_height)
		{
			// Image can't be shrunk along (at least) one
			// of its dimensions, so don't bother
			// resampling.	Technically we should, but
			// it's pretty useless at this point.  Just
			// change the image dimensions and leave the
			// existing pixels.
			return false;
		}

		// Resample.  Simple average 2x2 --> 1, in-place.
		for (size_t j = 0; j < new_h; j++)
		{
			boost::uint8_t* out = m_data.get() + j * new_w;
			boost::uint8_t* in = m_data.get() + (j << 1) * m_width;
			for (size_t i = 0; i < new_w; i++)
			{
				int	a;
				a = (*(in + 0) + *(in + 1) + *(in + 0 + m_width) + *(in + 1 + m_width));
				*(out) = a >> 2;
				out++;
				in += 2;
			}
		}

		// Munge parameters to reflect the shrunken image.
		m_width = m_pitch = new_w;
		m_height = new_h;
		m_size = m_height*m_pitch;

		assert(m_pitch >= m_width);

		return true;
	}


	void	alpha::set_pixel(size_t x, size_t y, boost::uint8_t a)
	// Set the pixel at the given position.
	{
		assert(x < m_width);
		assert(y < m_height);

		boost::uint8_t*	data = scanline(y) + x;

		data[0] = a;
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


	unsigned int	alpha::compute_hash() const
	// Compute a hash code based on image contents.  Can be useful
	// for comparing images.
	{
		unsigned int	h = bernstein_hash(&m_width, sizeof(m_width));
		h = bernstein_hash(&m_height, sizeof(m_height), h);

		for (int i = 0, n = m_height; i < n; i++)
		{
			h = bernstein_hash(scanline(i), m_width, h);
		}

		return h;
	}

	//
	// yuv
	//
	yuv::yuv(int w, int h) :
		image_base(0, w, h, w, YUV) // pitch initialized to wrong value, will fix m_size below

	{
		planes[Y].w = m_width;
		planes[Y].h = m_height;
		planes[Y].size = m_width * m_height;
		planes[Y].offset = 0;

		planes[U] = planes[Y];
		planes[U].w >>= 1;
		planes[U].h >>= 1;
		planes[U].size >>= 2;
		planes[U].offset = planes[Y].size;

		planes[V] = planes[U];
		planes[V].offset += planes[U].size;

		m_size = planes[Y].size + (planes[U].size << 1);

		for (int i = 0; i < 3; ++i)
		{
			planes[i].id = 0;	//texids[i];

			unsigned int ww = planes[i].w;
			unsigned int hh = planes[i].h;
			planes[i].unit = 0; // i[units];
			planes[i].p2w = (ww & (ww - 1)) ? video_nlpo2(ww) : ww;
			planes[i].p2h = (hh & (hh - 1)) ? video_nlpo2(hh) : hh;
			float tw = (double) ww / planes[i].p2w;
			float th = (double) hh / planes[i].p2h;

			planes[i].coords[0][0] = 0.0;
			planes[i].coords[0][1] = 0.0;
			planes[i].coords[1][0] = tw;
			planes[i].coords[1][1] = 0.0;
			planes[i].coords[2][0] = tw; 
			planes[i].coords[2][1] = th;
			planes[i].coords[3][0] = 0.0;
			planes[i].coords[3][1] = th;
		}

		m_data.reset( new boost::uint8_t[m_size] );

	//		m_bounds->m_x_min = 0.0f;
	//		m_bounds->m_x_max = 1.0f;
	//		m_bounds->m_y_min = 0.0f;
	//		m_bounds->m_y_max = 1.0f;
	}

	unsigned int yuv::video_nlpo2(unsigned int x) const
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x + 1;
	}

	//
	// utility
	//


	// Write the given image to the given out stream, in jpeg format.
	void	write_jpeg(tu_file* out, rgb* image, int quality)
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
	rgb*	read_jpeg(tu_file* in)
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
	rgba*	read_swf_jpeg3(tu_file* in)
	{
		std::auto_ptr<jpeg::input> j_in ( jpeg::input::create_swf_jpeg2_header_only(in, false) );
		if ( ! j_in.get() ) return 0;
		
		j_in->start_image();

		std::auto_ptr<rgba> im ( image::create_rgba(j_in->get_width(), j_in->get_height()) );

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
	void	write_tga(tu_file* out, rgba* im)
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

}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
