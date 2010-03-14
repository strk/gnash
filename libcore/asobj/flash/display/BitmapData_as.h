// BitmapData_as.h:  ActionScript "BitmapData" class, for Gnash.
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

#ifndef GNASH_ASOBJ_BITMAPDATA_H
#define GNASH_ASOBJ_BITMAPDATA_H

#include "Relay.h"

#include <list>
#include <vector>
#include <boost/cstdint.hpp>
#include <cassert>

namespace gnash {

class as_function;
class as_object;
class ObjectURI;
class Bitmap;


/// Implements the BitmapData native type.
//
/// This holds a vector of bitmap data. The vector's size does not change
/// from construction until disposal. Disposal is signified by the clearing
/// of the vector. All callers should check whether the BitmapData has been
/// disposed before attempting to access any stored pixel data.
class BitmapData_as : public Relay
{

public:

    typedef std::vector<boost::uint32_t> BitmapArray;

    // The constructor sets the fill colour and the
    // immutable size of the bitmap, as well as whether
    // it can handle transparency or not.
	BitmapData_as(as_object* owner, size_t width, size_t height,
	              bool transparent, boost::uint32_t fillColor);

    size_t getWidth() const { return _width; }
    size_t getHeight() const { return _height; }
    bool isTransparent() const { return _transparent; }
    
    const BitmapArray& getBitmapData() const {
        return _bitmapData;
    }
 
    /// Set a specified pixel to the specified color.
    //
    /// Callers must make sure the pixel is in range and that the
    /// BitmapData has not been disposed. Retains transparency
    /// (which is opaque, for non-transparent BitmapData objects).
    void setPixel(int x, int y, boost::uint32_t color) {
        assert(!_bitmapData.empty());
        const BitmapArray::size_type index = x * _width + y;
        _bitmapData[index] = (_bitmapData[index] & 0xff000000) | color;
    }

    /// Set a specified pixel to the specified color.
    //
    /// Callers must make sure the pixel is in range and that the BitmapData
    /// has not been disposed. Set to opaque for non-transparent BitmapData
    /// objects
    void setPixel32(int x, int y, boost::uint32_t color) {
        assert(!_bitmapData.empty());
        _bitmapData[x * _width + y] = _transparent ? color : color | 0xff000000;
    }

    /// Returns the value of the pixel at (x, y) optionally with transparency.
    //
    /// Callers must make that dispose() has not been called.
    /// Returns 0 if the pixel is out of range.
    boost::int32_t getPixel(int x, int y, bool transparency) const;

    void update(const boost::uint8_t* data);

    /// Fill the bitmap with a colour starting at x, y
    //
    /// Callers must check that arguments are within the BitmapData's range
    /// and that dispose() has not been called.
    void fillRect(int x, int y, int w, int h, boost::uint32_t color);
    
    // Free the bitmap data (clear the array)
    void dispose();

    void registerBitmap(Bitmap* bitmap) {
        _attachedBitmaps.push_back(bitmap);
    }

    /// Overrides Relay::setReachable().
    virtual void setReachable();

private:

    void updateAttachedBitmaps();

    /// The object to which this native type class belongs to.
    as_object* _owner;

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

inline bool
disposed(const BitmapData_as& bm)
{
    return bm.getBitmapData().empty();
}


/// Initialize the global BitmapData class
void bitmapdata_class_init(as_object& where, const ObjectURI& uri);

} // end of gnash namespace

#endif
