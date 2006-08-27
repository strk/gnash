// image.h	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

#ifndef IMAGE_H
#define IMAGE_H


#include "tu_config.h"
#include "tu_types.h"
class tu_file;
namespace jpeg { class input; }


/// Handy image utilities for RGB surfaces.
namespace image
{
	/// Base class for different types of images
	class image_base
	{
	public:
		enum id_image
		{
			INVALID,
			RGB,
			RGBA,
			ALPHA,
			ROW
		};

		id_image m_type;

		uint8_t*	m_data;
		int	m_width;
		int	m_height;
		int	m_pitch;	// byte offset from one row to the next

		image_base(uint8_t* data, int width, int height, int pitch, id_image type);
	};

	/// 24-bit RGB image.  Packed data, red byte first (RGBRGB...)
	class rgb : public image_base
	{
	public:
		rgb(int width, int height);
		~rgb();
	};

	/// 32-bit RGBA image.  Packed data, red byte first (RGBARGBA...)
	class rgba : public image_base
	{
	public:
		rgba(int width, int height);
		~rgba();

		void	set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	};

	/// 8-bit alpha image.
	class alpha : public image_base
	{
	public:
		alpha(int width, int height);
		~alpha();

		void	set_pixel(int x, int y, uint8_t a);

		// Bitwise content comparison.
		bool	operator==(const alpha& a) const;

		// Return a hash code based on the image contents.
		unsigned int	compute_hash() const;
	};


	/// Make a system-memory 24-bit bitmap surface.  24-bit packed
	/// data, red byte first.
	rgb*	create_rgb(int width, int height);


	/// \brief
	/// Make a system-memory 32-bit bitmap surface.  Packed data,
	/// red byte first.
	rgba*	create_rgba(int width, int height);


	/// Make a system-memory 8-bit bitmap surface.
	alpha*	create_alpha(int width, int height);

	
	uint8_t*	scanline(image_base* surf, int y);
	const uint8_t*	scanline(const image_base* surf, int y);


	void	resample(rgb* out, int out_x0, int out_y0, int out_x1, int out_y1,
			 rgb* in, float in_x0, float in_y0, float in_x1, float in_y1);

	void	resample(rgba* out, int out_x0, int out_y0, int out_x1, int out_y1,
			 rgba* in, float in_x0, float in_y0, float in_x1, float in_y1);

	/// Write the given image to the given out stream, in jpeg format.
	void	write_jpeg(tu_file* out, rgb* image, int quality);

	/// Write a 32-bit Targa format bitmap.  Dead simple, no compression.
	void	write_tga(tu_file* out, rgba* image);

	/// Create and read a new image from the given filename, if possible.
	rgb*	read_jpeg(const char* filename);

	/// Create and read a new image from the stream.
	rgb*	read_jpeg(tu_file* in);

	/// \brief
	/// For reading SWF JPEG2-style image data (slight variation on
	/// ordinary JPEG).
	rgb*	read_swf_jpeg2(tu_file* in);

	/// \brief
	/// For reading SWF JPEG2-style image data, using pre-loaded
	/// headers stored in the given jpeg::input object.
	rgb*	read_swf_jpeg2_with_tables(jpeg::input* loader);

	/// \brief
	/// For reading SWF JPEG3-style image data, like ordinary JPEG, 
	/// but stores the data in rgba format.
	rgba*	read_swf_jpeg3(tu_file* in);

	/// \brief
	/// Fast, in-place, DESTRUCTIVE resample.  For making mip-maps.
	/// Munges the input image to produce the output image.
	void	make_next_miplevel(rgb* image);

	/// \brief
	/// Fast, in-place resample.  For making mip-maps.  Munges the
	/// input image to produce the output image.
	void	make_next_miplevel(rgba* image);
}


#endif // IMAGE_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
