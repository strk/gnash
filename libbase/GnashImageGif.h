// GnashImageGif.h: gif_lib wrapper for Gnash.
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

#ifndef GNASH_IMAGE_GIF_H
#define GNASH_IMAGE_GIF_H

#include <memory>

#include "dsodefs.h"
#include "GnashImage.h"
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

extern "C" {
#include <gif_lib.h>
}

// It is convenient to read the gif data into memory as interlaced GIFs
// cannot be interpreted line by line.

// Forward declarations
namespace gnash { class IOChannel; }

namespace gnash {

class GifImageInput : public ImageInput
{

    typedef boost::scoped_array<GifPixelType> PixelRow;

private:
	// State needed for input.
	GifFileType* _gif;
    
    // A counter for keeping track of the last row copied.
    size_t _currentRow;
    
    // A 2-dimensional scoped array holding the unpacked pixel data.
    boost::scoped_array<PixelRow> _gifData;

public:

	/// Constructor.  
	//
	/// @param in
	/// 	The stream to read from.
	GifImageInput(boost::shared_ptr<IOChannel> in);
	
	// Destructor. Free libpng-allocated memory.
	~GifImageInput();
	
    void init();

    void read();

	// Return the height of the image.
	size_t getHeight() const;

	// Return the width of the image.
	size_t getWidth() const;

	// Return number of components (i.e. == 3 for RGB
	// data).  The size of the data for a scanline is
	// get_width() * get_components().
	//
	int	getComponents() const { return 3; }

	// Read a scanline's worth of image data into the
	// given buffer.  The amount of data read is
	// get_width() * get_components().
	//
	void readScanline(unsigned char* rgb_data);


    DSOEXPORT static std::auto_ptr<ImageInput> create(boost::shared_ptr<IOChannel> in)
    {
        std::auto_ptr<ImageInput> ret ( new GifImageInput(in) );
        if ( ret.get() ) ret->read();
        return ret;
    }

};

} // namespace gnash


#endif
