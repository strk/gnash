// image.h	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Handy image utilities for RGB surfaces.

#ifndef IMAGE_H
#define IMAGE_H


#include "tu_config.h"
#include <boost/cstdint.hpp>

#include <boost/scoped_array.hpp>
#include <memory> // for auto_ptr

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

		image_base(const image_base& o)
			:
			m_type(o.m_type),
			m_size(o.size()),
			m_data(new uint8_t[m_size]),
			m_width(o.width()),
			m_height(o.height()),
			m_pitch(o.m_pitch)
		{
			update(o);
		}
		
			
			
		image_base(uint8_t *data, int width, int height, int pitch, id_image type);

		/// Construct an image_base allocating data for height*pitch bytes
		image_base(int width, int height, int pitch, id_image type);

		/// Return a copy of this image
		virtual std::auto_ptr<image_base> clone() const=0;

		/// Return size of this image buffer, in bytes
		size_t size() const { return m_size; }

		/// Return size in bytes of a row of this image 
		size_t pitch() const { return m_pitch; }

		/// Return size in bytes of a single pixel value
		size_t pixelSize() const
		{
			return m_pitch/m_width;
		}

		/// Return width of image in pixels
		size_t width() const { return m_width; }

		/// Return height of image in pixels
		size_t height() const { return m_height; }

		/// Copy image data from a buffer.
		//
		/// Note that this buffer MUST have the same m_pitch, or unexpected things
		/// will happen. In general, it is only safe to copy from another image_base
		/// (or derivative thereof) or unexpected things will happen. 
		///
		/// @param data buffer to copy data from.
		///
		void update(uint8_t* data);

		/// Copy image data from another image data
		//
		/// Note that this buffer MUST have the same m_pitch and m_type
		/// or an assertion will fail.
		///
		/// @param from image to copy data from.
		///
		void update(const image_base& from);

		/// Return a pointer to the underlying data
		uint8_t* data() { return m_data.get(); }

		/// Return a pointer to first byte of given line
		DSOEXPORT uint8_t* scanline(size_t y);

		/// Return a read-only pointer to first byte of given line
		DSOEXPORT const uint8_t* scanline(size_t y) const
		{
			return const_cast<image_base*>(this)->scanline(y);
		}

		virtual ~image_base() {}


		/// \brief
		/// Fast, in-place resample.  For making mip-maps.  Munges the
		/// input image to produce the output image.
		//
		/// @return true if resample happened, false otherwise
		///         (image can't be shrinked, for example)
		///
		virtual bool make_next_miplevel() { return false; }

	protected:

		/// Size of image buffer in bytes
		size_t m_size;

		/// Data bytes, geometry defined by members below
		boost::scoped_array<uint8_t> m_data;

		/// Width of image, in pixels
		size_t	m_width;

		/// Height of image, in pixels
		size_t	m_height;

		/// Byte offset from one row to the next
		//
		/// This is basically width in bytes of each line.
		/// For example, in an alpha image type this is equal to m_width
		/// while for an RGB this is 3 times the m_width.
		///
		size_t	m_pitch;

	private:

	};

	/// 24-bit RGB image.  Packed data, red byte first (RGBRGB...)
	class DSOEXPORT rgb : public image_base
	{

	public:

		rgb(int width, int height);

		rgb(const rgb& o)
			:
			image_base(o)
		{}

		std::auto_ptr<image_base> clone() const
		{
			return std::auto_ptr<image_base>(new rgb(*this));
		}

		~rgb();

		// See dox in base class
		bool make_next_miplevel();
	};

	/// 32-bit RGBA image.  Packed data, red byte first (RGBARGBA...)
	class DSOEXPORT rgba : public image_base
	{

	public:

		rgba(int width, int height);

		rgba(const rgba& o)
			:
			image_base(o)
		{}

		~rgba();

		std::auto_ptr<image_base> clone() const
		{
			return std::auto_ptr<image_base>(new rgba(*this));
		}


		/// Set pixel value 
		//
		/// TODO: move in base class ?
		///
		void	set_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		/// Set alpha value for given pixel
		//
		/// TODO: move in base class ?
		///
		void	set_alpha(size_t x, size_t y, uint8_t a);

		// See dox in base class
		bool make_next_miplevel();
	};

	/// 8-bit alpha image.
	class DSOEXPORT alpha : public image_base
	{
	public:
		alpha(int width, int height);

		alpha(const alpha& o)
			:
			image_base(o)
		{}

		std::auto_ptr<image_base> clone() const
		{
			return std::auto_ptr<image_base>(new alpha(*this));
		}

		~alpha();

		// See dox in base class
		bool make_next_miplevel();

		/// Set pixel value 
		//
		/// TODO: move in base class ?
		///
		void	set_pixel(size_t x, size_t y, uint8_t a);

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

	yuv(const yuv& o)
		:
		image_base(o)
	{
		planes[0] = o.planes[0];
		planes[1] = o.planes[1];
		planes[2] = o.planes[2];
		planes[3] = o.planes[3];
	}

	~yuv() {}

	std::auto_ptr<image_base> clone() const
	{
		return std::auto_ptr<image_base>(new yuv(*this));
	}


	unsigned int video_nlpo2(unsigned int x) const;

	struct plane {
		unsigned int w, h, p2w, p2h, offset, size;
		int unit;
		int id;
		float coords[4][2];
	} planes[4];

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
}


#endif // IMAGE_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
