// BitmapData_as.h:  ActionScript "BitmapData" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include <list>
#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include <cassert>
#include <boost/intrusive_ptr.hpp>
#include <memory>

#include "Relay.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "ImageIterators.h"

namespace gnash {
    class as_object;
    struct ObjectURI;
    class MovieClip;
    class Transform;
    class DisplayObject;
    namespace image {
        class GnashImage;
    }
}

namespace gnash {

/// Implements the BitmapData native type.
//
/// This class gives access to a Bitmap that may be attached to a MovieClip
/// and rendered directly. The underlying data may therefore be stored
/// in a Renderer, for instance, and only retrieved from there when a
/// BitmapData instance requires access to it.
//
/// Because this retrieval can be expensive, it is advisable not to call
/// member functions frequently, but rather to access the data through
/// iterators. To facilitate this, iterators are random access.
//
/// There is also overhead to calling functions such as width() and height(),
/// again because the image data is retrieve from the Renderer. The size is
/// immutable, so these dimensions can safely be cached.
class BitmapData_as : public Relay
{
public:

    enum Channel {
        CHANNEL_RED = 1,
        CHANNEL_GREEN = 2,
        CHANNEL_BLUE = 4,
        CHANNEL_ALPHA = 8
    };

    typedef image::pixel_iterator<image::ARGB> iterator;

    /// Construct a BitmapData.
    //
    /// The constructor sets the immutable size of the
    /// bitmap, as well as whether it can handle transparency or not.
	BitmapData_as(as_object* owner, std::auto_ptr<image::GnashImage> im);

    virtual ~BitmapData_as() {}

    /// Return the width of the image
    //
    /// Do not call if disposed!
    size_t width() const {
        assert(data());
        return data()->width();
    }
    
    /// Return the height of the image
    //
    /// Do not call if disposed!
    size_t height() const {
        assert(data());
        return data()->height();
    }

    /// Whether the BitmapData_as has transparency.
    //
    /// Do not call if disposed!
    bool transparent() const {
        assert(data());
        return (data()->type() == image::TYPE_RGBA);
    }

    /// Return the image data
    //
    /// This is only for use by containes
    const CachedBitmap* bitmapInfo() const {
        return _cachedBitmap.get();
    }

    /// Free the bitmap data
    //
    /// This potentially frees the data.
    void dispose();
    
    /// Draw a MovieClip to a BitmapData
    void draw(MovieClip& mc, const Transform& transform);

    /// Attach this BitmapData to an object
    //
    /// This may be either as a fill or an attached Bitmap.
    void attach(DisplayObject* obj) {
        _attachedObjects.push_back(obj);
    }

    /// Overrides Relay::setReachable().
    virtual void setReachable();

    /// Whether the BitmapData has been disposed.
    //
    /// Any callers requiring access to the data or any properties should
    /// check that this is false first. Particularly width(), height(), 
    /// transparent(), begin(), end() may only be called if the BitmapData_as
    /// has not been disposed.
    bool disposed() const {
        return !data();
    }
 
    /// Return a BitmapData_as::iterator to the first pixel in the data.
    iterator begin() const {
        assert(!disposed());
        return image::begin<image::ARGB>(*data());
    }
    
    /// Return a BitmapData_as::iterator to a one-past-the end pixel.
    iterator end() const {
        assert(!disposed());
        return image::end<image::ARGB>(*data());
    }

    /// Inform any attached objects that the data has changed.
    void updateObjects() const;

private:
    
    image::GnashImage* data() const {
        return _cachedBitmap.get() ? &_cachedBitmap->image() : _image.get();
    }

    /// The object to which this native type class belongs to.
    as_object* _owner;

    boost::intrusive_ptr<CachedBitmap> _cachedBitmap;

    boost::scoped_ptr<image::GnashImage> _image;

    std::list<DisplayObject*> _attachedObjects;

};

/// Initialize the global BitmapData class
void bitmapdata_class_init(as_object& where, const ObjectURI& uri);

void registerBitmapDataNative(as_object& global);

} // end of gnash namespace

#endif
