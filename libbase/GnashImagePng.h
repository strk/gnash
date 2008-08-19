// GnashImagePng.h: libpng wrapper for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_IMAGE_PNG_H
#define GNASH_IMAGE_PNG_H

#include <memory>

#include "dsodefs.h"
#include "GnashImage.h"
#include "boost/scoped_array.hpp"


extern "C" {
#include <png.h>
}

// Forward declarations
namespace gnash { class IOChannel; }

namespace gnash {

class PngImageInput : public ImageInput
{

private:
	// State needed for input.
    png_structp _pngPtr;
    png_infop _infoPtr;
    boost::scoped_array<png_bytep> _rowPtrs;
    boost::scoped_array<png_byte> _pixelData;
   
    // A counter for keeping track of the last row copied.
    size_t _currentRow;

    void init();

	// Return number of components (i.e. == 3 for RGB
	// data).
    size_t getComponents() const;

public:

	/// Constructor.  
	//
	/// @param in
	/// 	The stream to read from.
	PngImageInput(boost::shared_ptr<IOChannel> in);
	
	// Destructor. Free libpng-allocated memory.
	~PngImageInput();
	
    void read();

	// Return the height of the image.
	size_t getHeight() const;

	// Return the width of the image.
	size_t getWidth() const;

	// Read a scanline's worth of image data into the
	// given buffer.  The amount of data read is
	// getWidth() * getComponents().
	//
	void readScanline(unsigned char* imageData);


    DSOEXPORT static std::auto_ptr<ImageInput> create(boost::shared_ptr<IOChannel> in)
    {
        std::auto_ptr<ImageInput> ret ( new PngImageInput(in) );
        if ( ret.get() ) ret->read();
        return ret;
    }

};

// Helper object for writing jpeg image data.
class PngImageOutput : public ImageOutput
{

public:

	/// Create an output object bount to a gnash::IOChannel
	//
	/// @param out     The IOChannel used for output. Must be kept alive throughout
	///
	/// @param quality Unused in PNG output
	
	PngImageOutput(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int quality);
	
	~PngImageOutput();

	void writeImageRGB(unsigned char* rgbData);
	
	void writeImageRGBA(unsigned char* rgbaData);

	DSOEXPORT static std::auto_ptr<ImageOutput> create(boost::shared_ptr<IOChannel> out, size_t width, size_t height, int quality);
	
private:

    void init();

    /// Libpng structures for image and output state.
    png_structp _pngPtr;
    png_infop _infoPtr;
	
};

} // namespace gnash



#endif
