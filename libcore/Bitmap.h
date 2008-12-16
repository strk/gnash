// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#include "character.h" 
#include "BitmapInfo.h"
#include "flash/display/BitmapData_as.h"
#include "render.h"

#include "DynamicShape.h"


namespace gnash {


/// \brief
/// Represents the outline of one or more shapes, along with
/// information on fill and line styles.
class Bitmap : public character
{
public:

	Bitmap(boost::intrusive_ptr<BitmapData_as> bd, character* parent, int id);

    ~Bitmap();

    void update();

    virtual void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

    virtual void display();

    virtual rect getBounds() const;

protected:

    void markReachableObjects() const {
        if (_bitmapData) _bitmapData->setReachable();
        if (_bitmapInfo) _bitmapInfo->setReachable();
    }

private:

    boost::intrusive_ptr<BitmapData_as> _bitmapData;

    /// The current bitmap information is stored here.
    boost::intrusive_ptr<BitmapInfo> _bitmapInfo;

    /// FIXME: using shape_character_def is unpleasant.
    boost::intrusive_ptr<DynamicShape> _shapeDef;

};

}	// end namespace gnash


#endif // GNASH_DYNAMIC_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
