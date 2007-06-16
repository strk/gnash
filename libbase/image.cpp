// image.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

#include "image.h"

#include "container.h"
#include "utility.h"
#include "jpeg.h"
#include "tu_file.h"
#include <cstring>
#include <boost/scoped_array.hpp>


namespace image
{
	//
	// image_base
	//
	image_base::image_base(uint8_t* data, int width, int height, int pitch, id_image type)
		:
		m_type(type),
		m_data(data),
		m_width(width),
		m_height(height),
		m_pitch(pitch)
	{
	}

	void image_base::update(uint8_t* data)
	{
		memcpy(m_data, data, m_pitch * m_height);
	}

	uint8_t*	scanline(image_base* surf, int y)
	{
		assert(surf);
		assert(y >= 0 && y < surf->m_height);
		return ((uint8_t*) surf->m_data) + surf->m_pitch * y;
	}


	const uint8_t*	scanline(const image_base* surf, int y)
	{
		assert(surf);
		assert(y >= 0 && y < surf->m_height);
		return ((const uint8_t*) surf->m_data) + surf->m_pitch * y;
	}


	//
	// rgb
	//

	rgb::rgb(int width, int height)
		:
		image_base(
			0,
			width,
			height,
			(width * 3 + 3) & ~3, RGB)	// round pitch up to nearest 4-byte boundary
	{
		assert(width > 0);
		assert(height > 0);
		assert(m_pitch >= m_width * 3);
		assert((m_pitch & 3) == 0);

		m_data = new uint8_t[m_pitch * m_height];
	}

	rgb::~rgb()
	{
		// TODO FIXME: m_data is a member of image_base, 
		// so ONLY image_base should delete it !
		// USE A SCOPED POINTER FOR THIS !
		delete [] m_data;
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
		image_base(0, width, height, width * 4, RGBA)
	{
		assert(width > 0);
		assert(height > 0);
		assert(m_pitch >= m_width * 4);
		assert((m_pitch & 3) == 0);

//		m_data = (uint8_t*) dlmalloc(m_pitch * m_height);
		m_data = new uint8_t[m_pitch * m_height];
	}

	rgba::~rgba()
	{
		// TODO FIXME: m_data is a member of image_base, 
		// so ONLY image_base should delete it !
		// USE A SCOPED POINTER FOR THIS !
		delete [] m_data;
	}


	rgba*	create_rgba(int width, int height)
	// Create an system-memory rgb surface.  The data order is
	// packed 32-bit, RGBARGBA..., regardless of the endian-ness
	// of the CPU.
	{
		return new rgba(width, height);
	}


	void	rgba::set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	// Set the pixel at the given position.
	{
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		uint8_t*	data = scanline(this, y) + 4 * x;

		data[0] = r;
		data[1] = g;
		data[2] = b;
		data[3] = a;
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
		image_base(0, width, height, width, ALPHA)
	{
		assert(width > 0);
		assert(height > 0);

//		m_data = (uint8_t*) dlmalloc(m_pitch * m_height);
		m_data = new uint8_t[m_pitch * m_height];
	}


	alpha::~alpha()
	{
			delete [] m_data;
	}


	void	alpha::set_pixel(int x, int y, uint8_t a)
	// Set the pixel at the given position.
	{
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		uint8_t*	data = scanline(this, y) + x;

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
			if (memcmp(scanline(this, j), scanline(&a, j), m_width))
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
			h = bernstein_hash(scanline(this, i), m_width, h);
		}

		return h;
	}

	//
	// yuv
	//
	yuv::yuv(int w, int h) :
		image_base(0, w, h, w, YUV)

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

		m_data = new uint8_t[m_size];

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

	int yuv::size() const
	{
		return m_size;
	}

	void yuv::update(uint8_t* data)
	{
		memcpy(m_data, data, m_size);
	}

	//
	// utility
	//


	void	write_jpeg(tu_file* out, rgb* image, int quality)
	// Write the given image to the given out stream, in jpeg format.
	{
		std::auto_ptr<jpeg::output> j_out ( jpeg::output::create(out, image->m_width, image->m_height, quality) );

		for (int y = 0; y < image->m_height; y++) {
			j_out->write_scanline(scanline(image, y));
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
			j_in->read_scanline(scanline(im.get(), y));
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
			j_in->read_scanline(scanline(im, y));
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
		std::auto_ptr<jpeg::input> j_in ( jpeg::input::create_swf_jpeg2_header_only(in) );
		if ( ! j_in.get() ) return 0;
		
		j_in->start_image();

		std::auto_ptr<rgba> im ( image::create_rgba(j_in->get_width(), j_in->get_height()) );

		boost::scoped_array<uint8_t> line ( new uint8_t[3*j_in->get_width()] );

		for (int y = 0; y < j_in->get_height(); y++) 
		{
			j_in->read_scanline(line.get());

			uint8_t*	data = scanline(im.get(), y);
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


	void	write_tga(tu_file* out, rgba* im)
	// Write a 32-bit Targa format bitmap.  Dead simple, no compression.
	{
		out->write_byte(0);
		out->write_byte(0);
		out->write_byte(2);	/* uncompressed RGB */
		out->write_le16(0);
		out->write_le16(0);
		out->write_byte(0);
		out->write_le16(0);	/* X origin */
		out->write_le16(0);	/* y origin */
		out->write_le16(im->m_width);
		out->write_le16(im->m_height);
		out->write_byte(32);	/* 32 bit bitmap */
		out->write_byte(0);

		for (int y = 0; y < im->m_height; y++)
		{
			uint8_t*	p = scanline(im, y);
			for (int x = 0; x < im->m_width; x++)
			{
				out->write_byte(p[x * 4]);
				out->write_byte(p[x * 4 + 1]);
				out->write_byte(p[x * 4 + 2]);
				out->write_byte(p[x * 4 + 3]);
			}
		}
	}

	void	make_next_miplevel(rgb* image)
	// Fast, in-place resample.  For making mip-maps.  Munges the
	// input image to produce the output image.
	{
		assert(image->m_data);

		int	new_w = image->m_width >> 1;
		int	new_h = image->m_height >> 1;
		if (new_w < 1) new_w = 1;
		if (new_h < 1) new_h = 1;

		int	new_pitch = new_w * 3;
		// Round pitch up to the nearest 4-byte boundary.
		new_pitch = (new_pitch + 3) & ~3;

		if (new_w * 2 != image->m_width  || new_h * 2 != image->m_height)
		{
			// Image can't be shrunk along (at least) one
			// of its dimensions, so don't bother
			// resampling.  Technically we should, but
			// it's pretty useless at this point.  Just
			// change the image dimensions and leave the
			// existing pixels.
		}
		else
		{
			// Resample.  Simple average 2x2 --> 1, in-place.
			int	pitch = image->m_pitch;
			for (int j = 0; j < new_h; j++) {
				uint8_t*	out = ((uint8_t*) image->m_data) + j * new_pitch;
				uint8_t*	in = ((uint8_t*) image->m_data) + (j << 1) * pitch;
				for (int i = 0; i < new_w; i++) {
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
		}

		// Munge image's members to reflect the shrunken image.
		image->m_width = new_w;
		image->m_height = new_h;
		image->m_pitch = new_pitch;
	}


	void	make_next_miplevel(rgba* image)
	// Fast, in-place resample.  For making mip-maps.  Munges the
	// input image to produce the output image.
	{
		assert(image->m_data);

		int	new_w = image->m_width >> 1;
		int	new_h = image->m_height >> 1;
		if (new_w < 1) new_w = 1;
		if (new_h < 1) new_h = 1;

		int	new_pitch = new_w * 4;

		if (new_w * 2 != image->m_width  || new_h * 2 != image->m_height)
		{
			// Image can't be shrunk along (at least) one
			// of its dimensions, so don't bother
			// resampling.  Technically we should, but
			// it's pretty useless at this point.  Just
			// change the image dimensions and leave the
			// existing pixels.
		}
		else
		{
			// Resample.  Simple average 2x2 --> 1, in-place.
			int	pitch = image->m_pitch;
			for (int j = 0; j < new_h; j++) {
				uint8_t*	out = ((uint8_t*) image->m_data) + j * new_pitch;
				uint8_t*	in = ((uint8_t*) image->m_data) + (j << 1) * pitch;
				for (int i = 0; i < new_w; i++) {
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
		}

		// Munge image's members to reflect the shrunken image.
		image->m_width = new_w;
		image->m_height = new_h;
		image->m_pitch = new_pitch;
	}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
