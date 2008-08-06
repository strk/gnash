// GnashImageJpeg.h:  Jpeg reader, for Gnash.
// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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
//
// Original version by Thatcher Ulrich <tu@tulrich.com> 2002
//
// Wrapper for jpeg file operations.  The actual work is done by the
// IJG jpeg lib.

#ifndef JPEG_H
#define JPEG_H

#include "dsodefs.h"
#include <csetjmp> // for jmp_buf
#include "GnashImage.h"

extern "C" {
// jpeglib.h redefines HAVE_STDLIB_H. This silences
// the warnings, but it's not good.
#include <jpeglib.h>
#undef HAVE_STDLIB_H
}

// Forward declarations
namespace gnash { class IOChannel; }

/// Wrapper for jpeg file operations. 
//
/// The actual work is done by the
/// IJG jpeg lib.
///
namespace gnash
{
/// Bascially this is a thin wrapper around jpeg_decompress object.
class JpegImageInput : public gnash::ImageInput
{

private:

    const char* _errorOccurred;

	jmp_buf _jmpBuf;

	// State needed for input.
	jpeg_decompress_struct m_cinfo;
	jpeg_error_mgr m_jerr;

	bool _compressorOpened;
	
	bool _ownStream;

public:

	enum SWF_DEFINE_BITS_JPEG2 { SWF_JPEG2 };
	enum SWF_DEFINE_BITS_JPEG2_HEADER_ONLY { SWF_JPEG2_HEADER_ONLY };

	/// \brief
	/// Constructor.  
	//
	/// @param in
	/// 	The stream to read from. Ownership specified by
	///	second argument.
	JpegImageInput(boost::shared_ptr<IOChannel> in);

	void readHeader(unsigned int maxHeaderBytes);

	// Destructor.  Clean up our jpeg reader state.
	~JpegImageInput();

    void read()
    {
        start_image();
    }

	// Discard any data sitting in our input buffer.  Use
	// this before/after reading headers or partial image
	// data, to avoid screwing up future reads.
	void discard_partial_buffer();

	// This is something you can do with "abbreviated"
	// streams; i.e. if you constructed this inputter
	// using (SWF_JPEG2_HEADER_ONLY) to just load the
	// tables, or if you called finish_image() and want to
	// load another image using the existing tables.
	// 
	void start_image();

	void finish_image();

	// Return the height of the image.  Take the data from our m_cinfo struct.
	size_t getHeight() const;

	// Return the width of the image.  Take the data from our m_cinfo struct.
	size_t getWidth() const;

	// Return number of components (i.e. == 3 for RGB
	// data).  The size of the data for a scanline is
	// get_width() * get_components().
	//
	int	getComponents() const;

	// Read a scanline's worth of image data into the
	// given buffer.  The amount of data read is
	// get_width() * get_components().
	//
	void readScanline(unsigned char* rgb_data);

    static std::auto_ptr<gnash::ImageInput> create(boost::shared_ptr<IOChannel> in)
    {
        std::auto_ptr<gnash::ImageInput> ret ( new JpegImageInput(in) );
        if ( ret.get() ) ret->read(); // might throw an exception (I guess)
        return ret;
    }

    static std::auto_ptr<JpegImageInput> create_swf_jpeg2_header_only(boost::shared_ptr<IOChannel> in, unsigned int maxHeaderBytes)
    {
        std::auto_ptr<JpegImageInput> ret ( new JpegImageInput(in) );
        if ( ret.get() ) ret->readHeader(maxHeaderBytes); // might throw an exception
        return ret;
    }

    void errorOccurred(const char* msg);


};

// Helper object for writing jpeg image data.
class JpegImageOutput
{
public:
	/// Create an output object bount to a gnash::IOChannel
	//
	/// @param quality
	///	Quality goes from 1-100.
	///
	DSOEXPORT static JpegImageOutput*	create(gnash::IOChannel* out, int width, int height, int quality);

	virtual ~JpegImageOutput() {}

	// ...
	virtual void	write_scanline(unsigned char* rgb_data) = 0;
};

} // namespace gnash


#endif // JPEG_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
