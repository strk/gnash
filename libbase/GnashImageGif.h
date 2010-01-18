// GnashImageGif.h: gif_lib wrapper for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

// Forward declarations
namespace gnash { class IOChannel; }

namespace gnash {

class GifImageInput : public ImageInput
{

public:

    /// Construct a GifImageInput object to read from an IOChannel.
    //
    /// @param in   The stream to read GIF data from. Ownership is shared
    ///             between caller and GifImageInput, so it is freed
    ///             automatically when the last owner is destroyed.
    GifImageInput(boost::shared_ptr<IOChannel> in);
    
    ~GifImageInput();

    /// Begin processing the image data.
    void read();

    /// Get the image's height in pixels.
    //
    /// @return     The height of the image in pixels.
    size_t getHeight() const;

    /// Get the image's width in pixels.
    //
    /// @return     The width of the image in pixels.
    size_t getWidth() const;

    /// Get number of components (channels)
    //
    /// @return     The number of components, e.g. 3 for RGB
    size_t getComponents() const { return 3; }

    /// Read a scanline's worth of image data into the given buffer.
    //
    /// The amount of data read is getWidth() * getComponents().
    ///
    /// @param rgbData  The buffer for writing raw RGB data to.
    void readScanline(unsigned char* rgb_data);


    /// Create a GifImageInput and transfer ownership to the caller.
    //
    /// @param in   The IOChannel to read GIF data from.
    DSOEXPORT static std::auto_ptr<ImageInput> create(
            boost::shared_ptr<IOChannel> in)
    {
        std::auto_ptr<ImageInput> ret ( new GifImageInput(in) );
        if ( ret.get() ) ret->read();
        return ret;
    }

private:
    
    /// Initialize gif_lib
    void init();

    // State needed for input.
    GifFileType* _gif;
    
    // A counter for keeping track of the last row copied.
    size_t _currentRow;
    
    typedef boost::scoped_array<GifPixelType> PixelRow;

    // A 2-dimensional scoped array holding the unpacked pixel data.
    boost::scoped_array<PixelRow> _gifData;



};

} // namespace gnash


#endif
