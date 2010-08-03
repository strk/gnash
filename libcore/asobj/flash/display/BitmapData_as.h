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
#include <boost/scoped_ptr.hpp>
#include <cassert>
#include "GnashImage.h"
#include <boost/intrusive_ptr.hpp>

namespace gnash {
    class as_object;
    struct ObjectURI;
    class DisplayObject;
    class Renderer;
}

namespace gnash {

/// Implements the BitmapData native type.
//
/// This holds a vector of bitmap data. The vector's size does not change
/// from construction until disposal. Disposal is signified by the clearing
/// of the vector. All callers should check whether the BitmapData has been
/// disposed before attempting to access any stored pixel data.
class BitmapData_as : public Relay
{

public:

    // The constructor sets the fill colour and the
    // immutable size of the bitmap, as well as whether
    // it can handle transparency or not.
	BitmapData_as(as_object* owner, size_t width, size_t height,
	              bool transparent, boost::uint32_t fillColor,
                  Renderer* r);

    size_t getWidth() const { return _width; }
    size_t getHeight() const { return _height; }
    bool isTransparent() const { return _transparent; }
 
    const BitmapInfo* bitmapInfo() const {
        return _bitmapData.get();
    }

    /// Set a specified pixel to the specified color.
    //
    /// Callers must make sure the pixel is in range and that the
    /// BitmapData has not been disposed. Retains transparency
    /// (which is opaque, for non-transparent BitmapData objects).
    void setPixel(size_t x, size_t y, boost::uint32_t color);

    /// Set a specified pixel to the specified color.
    //
    /// Callers must make sure the pixel is in range and that the BitmapData
    /// has not been disposed. Set to opaque for non-transparent BitmapData
    /// objects
    void setPixel32(size_t x, size_t y, boost::uint32_t color);

    /// Returns the value of the pixel at (x, y) optionally with transparency.
    //
    /// Callers must make that dispose() has not been called.
    /// Returns 0 if the pixel is out of range.
    boost::int32_t getPixel(int x, int y, bool transparency) const;

    /// Fill the bitmap with a colour starting at x, y
    //
    /// Callers must check that arguments are within the BitmapData's range
    /// and that dispose() has not been called.
    void fillRect(int x, int y, int w, int h, boost::uint32_t color);
    
    // Free the bitmap data (clear the array)
    void dispose();

    void attach(DisplayObject* obj) {
        _attachedObjects.push_back(obj);
    }

    /// Overrides Relay::setReachable().
    virtual void setReachable();
    
    GnashImage* data() const {
        return _bitmapData.get() ? &_bitmapData->image() : _image.get();
    }

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

    boost::intrusive_ptr<BitmapInfo> _bitmapData;

    boost::scoped_ptr<GnashImage> _image;

    std::list<DisplayObject*> _attachedObjects;

};

inline bool
disposed(const BitmapData_as& bm)
{
    return !bm.data();
}


/// Initialize the global BitmapData class
void bitmapdata_class_init(as_object& where, const ObjectURI& uri);

} // end of gnash namespace

#endif
