// image.h	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

#ifndef IMAGE_H
#define IMAGE_H


#include "tu_config.h"
#include "tu_types.h"

#include <boost/scoped_array.hpp>

class tu_file;
namespace jpeg { class input; }


/// Handy image utilities for RGB surfaces.
namespace image
{
	/// Base class for different types of images
	class DSOEXPORT image_base
	{
	public:
		enum id_image
		{
			INVALID,
			RGB,
			RGBA,
			ALPHA,
			ROW,
			YUV
		};

		id_image m_type;

		// TODO FIXME: m_data allocation is currently managed
		// by subclasses (see rgb and rgba), this is really unsafe.
		// Rather, *this* calss should manage it, using a scoped
		// pointer
		// USE A SCOPED POINTER FOR THIS !
		uint8_t*	m_data;

		int	m_width;
		int	m_height;
		int	m_pitch;	// byte offset from one row to the next

		image_base(uint8_t* data, int width, int height, int pitch, id_image type);

		/// Copy image data from a buffer.
		//
		/// Note that this buffer MUST have the same m_pitch, or unexpected things
		/// will happen. In general, it is only safe to copy from another image_base
		/// (or derivative thereof) or unexpected things will happen. 
		///
		/// @param data buffer to copy data from.
		///
		void update(uint8_t* data);

		virtual ~image_base() {}
	};

	/// 24-bit RGB image.  Packed data, red byte first (RGBRGB...)
	class DSOEXPORT rgb : public image_base
	{
	public:
		rgb(int width, int height);
		~rgb();
	};

	/// 32-bit RGBA image.  Packed data, red byte first (RGBARGBA...)
	class DSOEXPORT rgba : public image_base
	{
	public:
		rgba(int width, int height);
		~rgba();

		void	set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	};

	/// 8-bit alpha image.
	class DSOEXPORT alpha : public image_base
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

class DSOEXPORT yuv : public image_base
{

public:

	enum {Y, U, V, T, NB_TEXS};

	yuv(int w, int h);
	~yuv() {}
	void update(uint8_t* data);
	unsigned int video_nlpo2(unsigned int x) const;
	int size() const;

	struct plane {
		unsigned int w, h, p2w, p2h, offset, size;
		int unit;
		int id;
		float coords[4][2];
	} planes[4];

	int m_size;

};
	/// Make a system-memory 24-bit bitmap surface.  24-bit packed
	/// data, red byte first.
	DSOEXPORT rgb*	create_rgb(int width, int height);


	/// \brief
	/// Make a system-memory 32-bit bitmap surface.  Packed data,
	/// red byte first.
	DSOEXPORT rgba*	create_rgba(int width, int height);


	/// Make a system-memory 8-bit bitmap surface.
	DSOEXPORT alpha*	create_alpha(int width, int height);

	
	DSOEXPORT uint8_t*	scanline(image_base* surf, int y);
	DSOEXPORT const uint8_t*	scanline(const image_base* surf, int y);


	DSOEXPORT void	resample(rgb* out, int out_x0, int out_y0, int out_x1, int out_y1,
			 rgb* in, float in_x0, float in_y0, float in_x1, float in_y1);

	DSOEXPORT void	resample(rgba* out, int out_x0, int out_y0, int out_x1, int out_y1,
			 rgba* in, float in_x0, float in_y0, float in_x1, float in_y1);

	/// Write the given image to the given out stream, in jpeg format.
	DSOEXPORT void	write_jpeg(tu_file* out, rgb* image, int quality);

	/// Write a 32-bit Targa format bitmap.  Dead simple, no compression.
	DSOEXPORT void	write_tga(tu_file* out, rgba* image);

	/// Create and read a new image from the given filename, if possible.
	DSOEXPORT rgb*	read_jpeg(const char* filename);

	/// Create and read a new image from the stream.
	//
	/// @param in
	/// 	Stream to read from. Ownership to the caller,
	///	not needed after return.
	///
	DSOEXPORT rgb*	read_jpeg(tu_file* in);

	/// \brief
	/// For reading SWF JPEG2-style image data (slight variation on
	/// ordinary JPEG).
	DSOEXPORT rgb*	read_swf_jpeg2(tu_file* in);

	/// \brief
	/// For reading SWF JPEG2-style image data, using pre-loaded
	/// headers stored in the given jpeg::input object.
	DSOEXPORT rgb*	read_swf_jpeg2_with_tables(jpeg::input* loader);

	/// \brief
	/// For reading SWF JPEG3-style image data, like ordinary JPEG, 
	/// but stores the data in rgba format.
	DSOEXPORT rgba*	read_swf_jpeg3(tu_file* in);

	/// \brief
	/// Fast, in-place, DESTRUCTIVE resample.  For making mip-maps.
	/// Munges the input image to produce the output image.
	DSOEXPORT void	make_next_miplevel(rgb* image);

	/// \brief
	/// Fast, in-place resample.  For making mip-maps.  Munges the
	/// input image to produce the output image.
	DSOEXPORT void	make_next_miplevel(rgba* image);
}


#endif // IMAGE_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
