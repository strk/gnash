// BitmapData_as.h:  ActionScript "BitmapData" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ_BITMAPDATA_H
#define GNASH_ASOBJ_BITMAPDATA_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h"
#include "as_object.h"

namespace gnash {

class as_function;
class as_object;
class Bitmap;


class BitmapData_as: public as_object
{

public:

    typedef std::vector<boost::uint32_t> BitmapArray;

    // The constructor sets the fill colour and the
    // immutable size of the bitmap, as well as whether
    // it can handle transparency or not.
	BitmapData_as(size_t width, size_t height,
	              bool transparent, boost::uint32_t fillColor);

    size_t getWidth() const { return _width; }
    size_t getHeight() const { return _height; }
    bool isTransparent() const { return _transparent; }
    
    const BitmapArray& getBitmapData() const
    {
        return _bitmapData;
    }
 
    /// Set a specified pixel to the specified color.
    //
    /// Callers must make sure the pixel is in range. Retains transparency
    /// (which is opaque, for non-transparent BitmapData objects).
    void setPixel(int x, int y, boost::uint32_t color) {
        const BitmapArray::size_type index = x * _width + y;
        _bitmapData[index] = (_bitmapData[index] & 0xff000000) | color;
    }

    /// Set a specified pixel to the specified color.
    //
    /// Callers must make sure the pixel is in range. Set to opaque for
    /// non-transparent BitmapData objects
    void setPixel32(int x, int y, boost::uint32_t color) {
        _bitmapData[x * _width + y] = _transparent ? color : color | 0xff000000;
    }


    /// Returns an unsigned int representation of the pixel
    /// at (x, y) either with or without transparency.
    boost::int32_t getPixel(int x, int y, bool transparency) const;

    void update(const boost::uint8_t* data);

    // Fill the bitmap with a colour starting at x, y
    void fillRect(int x, int y, int w, int h, boost::uint32_t color);
    
    // Free the bitmap data (clear the array)
    void dispose();

    void registerBitmap(Bitmap* bitmap) {
        _attachedBitmaps.push_back(bitmap);
    }

protected:

    void markReachableResources() const;

private:

    void updateAttachedBitmaps();

    // The width of the image, max 2880. This is immutable.
    const size_t _width;
    
    // The height of the image, max 2880. This is immutable.
    const size_t _height;
    
    // Whether the image is transparent. This is immutable.
    const bool _transparent;

    // A static array of 32-bit values holding the actual bitmap data.
    // The maximum size is 2880 x 2880 * 4 bytes = 33177600 bytes.
    BitmapArray _bitmapData;

    std::list<Bitmap*> _attachedBitmaps;

};


/// Initialize the global BitmapData class
void BitmapData_class_init(as_object& global);

as_function* getFlashDisplayBitmapDataConstructor();

} // end of gnash namespace

#endif
