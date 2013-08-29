// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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


#ifndef GNASH_BITMAP_H
#define GNASH_BITMAP_H

#include <boost/intrusive_ptr.hpp>
#include "DisplayObject.h" 
#include "flash/display/BitmapData_as.h"
#include "DynamicShape.h"

namespace gnash {
    class CachedBitmap;
    class BitmapMovieDefinition;
}


namespace gnash {


/// A Bitmap DisplayObject. This is not AS-referencable, but can be
/// removed and placed using depths like normal DisplayObjects.
//
/// This can be constructed dynamically from a BitmapData, or non-dynamically
/// as part of a BitmapMovie.
//
/// For non-dynamic Bitmap DisplayObjects, the bitmap data never changes. The
/// update() function is called once on stage placement.
//
/// For dynamic Bitmap DisplayObjects, the attached BitmapData_as should call
/// update() whenever the data changes. This Bitmap registers itself with
/// the BitmapData_as on stage placement.
class Bitmap : public DisplayObject
{
public:

    /// Construct a Bitmap character from a BitmapData.
	Bitmap(movie_root& mr, as_object* object, BitmapData_as* bd,
            DisplayObject* parent);
	
    /// Construct a Bitmap character from a loaded image.
    Bitmap(movie_root& mr, as_object* object, const BitmapMovieDefinition* def,
            DisplayObject* parent);

    virtual ~Bitmap();

    /// Notify the Bitmap that it's been updated during ActionScript execution
    virtual void update();

    virtual void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

    /// Display this Bitmap
	virtual void display(Renderer& renderer, const Transform& xform);

    /// Get the bounds of the Bitmap
    virtual SWFRect getBounds() const;

    /// Test whether a point is in the Bitmap's bounds.
    virtual bool pointInShape(boost::int32_t x, boost::int32_t y) const;

    /// Called when the object is placed on stage.
    virtual void construct(as_object* init = 0);

protected:

    void markReachableObjects() const {
        if (_bitmapData) _bitmapData->setReachable();
    }

private:

    /// Return the bitmap used for this Bitmap DisplayObject.
    //
    /// It comes either from the definition or the BitmapData_as.
    const CachedBitmap* bitmap() const;

    /// Checks whether an attached BitmapData_as is disposed.
    //
    /// If the BitmapData_as has been disposed, deletes _bitmapData.
    /// and clears the DynamicShape.
    void checkBitmapData();

    /// This creates the DynamicShape for rendering.
    //
    /// It should be called every time the underlying bitmap changes; for
    /// non-dynamic Bitmaps, this is only on construction.
    const boost::intrusive_ptr<const BitmapMovieDefinition> _def;

    BitmapData_as* _bitmapData;

    /// A shape to hold the bitmap fill.
    DynamicShape _shape;

    /// This is cached to save querying the BitmapData often
    size_t _width;

    /// This is cached to save querying the BitmapData often
    size_t _height;

};

}	// end namespace gnash


#endif // GNASH_DYNAMIC_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
