// MorphShape.cpp:  MorphShape handling for Gnash.
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

#include "MorphShape.h"
#include "VM.h"
#include "swf/ShapeRecord.h"
#include "Geometry.h"
#include "SWFMatrix.h"

namespace gnash
{


MorphShape::MorphShape(movie_root& mr, as_object* object,
        const SWF::DefineMorphShapeTag* def, DisplayObject* parent)
    :
    DisplayObject(mr, object, parent),
    _def(def),
    _shape(_def->shape1())
{
}

bool
MorphShape::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    SWFMatrix wm = getWorldMatrix();
    SWFMatrix wm_inverse = wm.invert();
    point lp(x, y);
    wm_inverse.transform(lp);
    
    // FIXME: if the shape contains non-scaled strokes
    //        we can't rely on boundary itself for a quick
    //        way out. Bounds supposedly already include
    //        thickness, so we might keep a flag telling us
    //        whether *non_scaled* strokes are present
    //        and if not still use the boundary check.
    // NOTE: just skipping this test breaks a corner-case
    //       in DrawingApiTest (kind of a fill-leakage making
    //       the collision detection find you inside a self-crossing
    //       shape).
    if (!_shape.getBounds().point_test(lp.x, lp.y)) return false;

    return geometry::pointTest(_shape.paths(), _shape.lineStyles(),
            lp.x, lp.y, wm);
}

void  
MorphShape::display(Renderer& renderer)
{
    morph();
    _def->display(renderer, *this); 
    clear_invalidated();
}

inline double
MorphShape::currentRatio() const
{
    return get_ratio() / 65535.0;
}

SWFRect
MorphShape::getBounds() const
{
    // TODO: optimize this more.
    SWFRect bounds = _shape.getBounds();
    bounds.expand_to_rect(_def->shape2().getBounds());
    return bounds;
}

void
MorphShape::morph()
{
    _shape.setLerp(_def->shape1(), _def->shape2(), currentRatio());
}


} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
