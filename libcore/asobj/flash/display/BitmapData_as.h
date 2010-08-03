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
#include "smart_ptr.h"
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
/// All functions can be called if the BitmapData has been disposed. Callers
/// do not need to check.
class BitmapData_as : public Relay
{

public:

    /// Construct a BitmapData.
    //
    /// The constructor sets the fill colour and the immutable size of the
    /// bitmap, as well as whether it can handle transparency or not.
	BitmapData_as(as_object* owner, size_t width, size_t height,
	              bool transparent, boost::uint32_t fillColor,
                  Renderer* r);

    ~BitmapData_as();

    /// Return the width of the image
    size_t width() const {
        return data() ? data()->width() : 0;
    }
    
    /// Return the height of the image
    //
    /// The unusual int type for a size is there to mirror the AS interface.
    size_t height() const {
        return data() ? data()->height() : 0;
    }

    bool transparent() const {
        return _transparent;
    }

    const BitmapInfo* bitmapInfo() const {
        return _bitmapData.get();
    }

    /// Set a specified pixel to the specified color.
    //
    /// Retains transparency value for BitmapDatas with transparency.
    void setPixel(size_t x, size_t y, boost::uint32_t color);

    /// Set a specified pixel to the specified color.
    void setPixel32(size_t x, size_t y, boost::uint32_t color);

    /// Returns the value of the pixel at (x, y) optionally with transparency.
    //
    /// Returns 0 if the pixel is out of range or the image has been disposed.
    boost::uint32_t getPixel(size_t x, size_t y) const;

    /// Fill the bitmap with a colour starting at x, y
    //
    /// Negative values are handled correctly.
    void fillRect(int x, int y, int w, int h, boost::uint32_t color);
    
    // Free the bitmap data (clear the array)
    void dispose();

    void attach(DisplayObject* obj) {
        _attachedObjects.push_back(obj);
    }

    /// Overrides Relay::setReachable().
    virtual void setReachable();

    /// Whether the BitmapData has been disposed.
    bool disposed() const {
        return !data();
    }

private:
    
    GnashImage* data() const {
        return _bitmapData.get() ? &_bitmapData->image() : _image.get();
    }

    void updateAttachedBitmaps();

    /// The object to which this native type class belongs to.
    as_object* _owner;

    // Whether the image is transparent. This is immutable.
    const bool _transparent;

    boost::intrusive_ptr<BitmapInfo> _bitmapData;

    boost::scoped_ptr<GnashImage> _image;

    std::list<DisplayObject*> _attachedObjects;

};

/// Initialize the global BitmapData class
void bitmapdata_class_init(as_object& where, const ObjectURI& uri);

} // end of gnash namespace

#endif
