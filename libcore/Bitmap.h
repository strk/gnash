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

#include "DynamicShape.h"


namespace gnash {


/// A Dynamic Bitmap DisplayObject. This is not AS-referencable, but can be
/// removed and placed using depths like normal DisplayObjects (DisplayObjects).
class Bitmap : public DisplayObject
{
public:

	Bitmap(boost::intrusive_ptr<BitmapData_as> bd, DisplayObject* parent, int id);

    ~Bitmap();

    void update();

    virtual void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

    virtual void display();

    virtual rect getBounds() const;

    virtual bool pointInShape(boost::int32_t x, boost::int32_t y) const;

    virtual void stagePlacementCallback(as_object* initObj = 0);

protected:

    /// This should really have an optional definition.
    virtual character_def* getDefinition() const {
        return _shapeDef.get();
    }

    void markReachableObjects() const {
        if (_bitmapData) _bitmapData->setReachable();
        if (_bitmapInfo) _bitmapInfo->setReachable();
        if (_shapeDef) _shapeDef->setReachable();
    }

private:

    /// This must convert the BitmapData to a BitmapInfo.
    //
    /// The result must be stored in _bitmapInfo.
    void drawBitmap();

    /// Call this before rendering to make sure the BitmapInfo is updated.
    void finalize();

    boost::intrusive_ptr<BitmapData_as> _bitmapData;

    /// The current bitmap information is stored here.
    boost::intrusive_ptr<BitmapInfo> _bitmapInfo;

    /// FIXME: using shape_character_def is unpleasant.
    boost::intrusive_ptr<DynamicShape> _shapeDef;

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
