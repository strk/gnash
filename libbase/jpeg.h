// jpeg.h	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Wrapper for jpeg file operations.  The actual work is done by the
// IJG jpeg lib.

#ifndef JPEG_H
#define JPEG_H


#include "tu_config.h"

struct jpeg_decompress_struct;
struct jpeg_compress_struct;
class tu_file;


/// Wrapper for jpeg file operations. 
//
/// The actual work is done by the
/// IJG jpeg lib.
///
namespace jpeg
{
	/// Wrapper around jpeg_decompress_struct.
	class input {

	public:

		input()
			:
			_errorOccurred(false)
		{}

		virtual ~input() {}


		/// \brief
		/// Create and return a jpeg-input object that will read from the
		/// given input stream.
		//
		/// The created input reads the jpeg header
		///
		/// @return NULL on error
		///
		DSOEXPORT static input*	create(tu_file* in);

		/// Read SWF JPEG2-style header. 
		//
		/// App needs to call start_image() before loading any
		/// image data.  Multiple images can be loaded by
		/// bracketing within start_image()/finish_image() pairs.
		///
		/// @return NULL on error
		///
		DSOEXPORT static input*	create_swf_jpeg2_header_only(tu_file* in);

		/// Discard existing bytes in our buffer.
		virtual void	discard_partial_buffer() = 0;

		virtual void	start_image() = 0;
		virtual void	finish_image() = 0;

		virtual int	get_height() const = 0;
		virtual int	get_width() const = 0;
		virtual void	read_scanline(unsigned char* rgb_data) = 0;

		void    errorOccurred()
		{
			_errorOccurred = true;
		}

	protected:

		/// This flag will be set to true by the error callback
		/// invoked by jpeg lib. Will be later used to throw
		/// a ParserException.
		///
		bool _errorOccurred;
	};


	// Helper object for writing jpeg image data.
	class output
	{
	public:
		/// Create an output object bount to a tu_file
		//
		/// @param quality
		///	Quality goes from 1-100.
		///
		DSOEXPORT static output*	create(tu_file* out, int width, int height, int quality);

		virtual ~output() {}

		// ...
		virtual void	write_scanline(unsigned char* rgb_data) = 0;
	};

} // namespace jpeg


#endif // JPEG_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
