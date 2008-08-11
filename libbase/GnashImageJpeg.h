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


namespace jpeg {

extern "C" {
// jpeglib.h redefines HAVE_STDLIB_H. This silences
// the warnings, but it's not good.
#include <jpeglib.h>
#undef HAVE_STDLIB_H
}

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
class JpegImageInput : public ImageInput
{

private:

    const char* _errorOccurred;

	jmp_buf _jmpBuf;

	// State needed for input.
	jpeg::jpeg_decompress_struct m_cinfo;
	jpeg::jpeg_error_mgr m_jerr;

	bool _compressorOpened;
	
	bool _ownStream;

public:

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
        startImage();
    }

	// Discard any data sitting in our input buffer.  Use
	// this before/after reading headers or partial image
	// data, to avoid screwing up future reads.
	void discardPartialBuffer();

	// This is something you can do with "abbreviated"
	// streams; i.e. if you constructed this inputter
	// using (SWF_JPEG2_HEADER_ONLY) to just load the
	// tables, or if you called finish_image() and want to
	// load another image using the existing tables.
	// 
	void startImage();

	void finishImage();

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

    static std::auto_ptr<JpegImageInput> createSWFJpeg2HeaderOnly(boost::shared_ptr<IOChannel> in, unsigned int maxHeaderBytes)
    {
        std::auto_ptr<JpegImageInput> ret ( new JpegImageInput(in) );
        if ( ret.get() ) ret->readHeader(maxHeaderBytes); // might throw an exception
        return ret;
    }

    void errorOccurred(const char* msg);


};

// Helper object for writing jpeg image data.
class JpegImageOutput : public ImageOutput
{

public:

	/// Create an output object bount to a gnash::IOChannel
	//
	/// @param quality
	///	Quality goes from 1-100.
	///
	JpegImageOutput(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int quality);
	
	~JpegImageOutput();

	void writeImageRGB(unsigned char* rgbData);

	DSOEXPORT static std::auto_ptr<ImageOutput> create(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int quality);
	
private:

	jpeg::jpeg_compress_struct m_cinfo;
	jpeg::jpeg_error_mgr m_jerr;
	
};

} // namespace gnash


#endif // JPEG_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
