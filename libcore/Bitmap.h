// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "DisplayObject.h" 
#include "BitmapInfo.h"
#include "flash/display/BitmapData_as.h"
#include "render.h"
#include "BitmapMovieDefinition.h"
#include "DynamicShape.h"


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

	Bitmap(boost::intrusive_ptr<BitmapData_as> bd, DisplayObject* parent,
            int id);
	
    Bitmap(const BitmapMovieDefinition* const def, DisplayObject* parent,
            int id);

    ~Bitmap();

    /// Called to update the Bitmap's DynamicShape for display.
    //
    /// For non-dynamic bitmaps, this should only be called once (for
    /// efficiency - there are no harmful side-effects)
    void update();

    virtual void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

    virtual void display(Renderer& renderer);

    virtual rect getBounds() const;

    virtual bool pointInShape(boost::int32_t x, boost::int32_t y) const;

    virtual void stagePlacementCallback(as_object* initObj = 0);

protected:

    void markReachableObjects() const {
        if (_bitmapData) _bitmapData->setReachable();
        if (_bitmapInfo) _bitmapInfo->setReachable();
        if (_def) _def->setReachable();
    }

private:

    /// Return the bitmap used for this Bitmap DisplayObject.
    //
    /// It comes either from the definition or the BitmapData_as.
    const BitmapInfo* bitmap() const;

    /// This updates _bitmapInfo from the BitmapData_as
    void makeBitmap();

    /// Checks whether an attached BitmapData_as is disposed.
    //
    /// If the BitmapData_as has been disposed, deletes _bitmapData.
    /// and clears the DynamicShape.
    void checkBitmapData();

    /// This creates the DynamicShape for rendering.
    //
    /// It should be called every time the underlying bitmap changes; for
    /// non-dynamic Bitmaps, this is only on construction.
    void makeBitmapShape();

    const boost::intrusive_ptr<const BitmapMovieDefinition> _def;

    boost::intrusive_ptr<BitmapData_as> _bitmapData;

    /// The current bitmap information is stored here.
    boost::intrusive_ptr<BitmapInfo> _bitmapInfo;

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
