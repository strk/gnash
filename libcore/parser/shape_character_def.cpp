// shape_character_def.cpp:  Quadratic bezier outline shapes, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


// Based on the public domain shape.cpp of Thatcher Ulrich <tu@tulrich.com> 2003

// Quadratic bezier outline shapes are the basis for most SWF rendering.


#include "shape_character_def.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "impl.h"
#include "log.h"
#include "render.h"
#include "Shape.h"
#include "SWFStream.h"
#include "MovieClip.h"

#include <algorithm>

// Define the macro below to always compute bounds for shape DisplayObjects
// and compare them with the bounds encoded in the SWF
//#define GNASH_DEBUG_SHAPE_BOUNDS 1

namespace gnash
{

DisplayObject*
shape_character_def::createDisplayObject(DisplayObject* parent, int id)
{
	return new Shape(this, parent, id);
}
    
bool
shape_character_def::pointTestLocal(boost::int32_t x, boost::int32_t y, 
     const SWFMatrix& wm) const
{
    return geometry::pointTest(_shape.paths(), _shape.lineStyles(), x, y, wm);
}


shape_character_def::shape_character_def(SWFStream& in, SWF::TagType tag,
        movie_definition& m)
    :
    character_def(),
    _shape(in, tag, m)
{
}

void
shape_character_def::display(const DisplayObject& inst) const
{
    render::drawShape(_shape, inst.get_world_cxform(), inst.getWorldMatrix());
}

void
shape_character_def::markReachableResources() const
{}

} // end namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
