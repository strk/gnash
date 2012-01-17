// DefineShapeTag.cpp:  Quadratic bezier outline shapes, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "RunResources.h"
#include "DefineShapeTag.h"
#include "log.h"
#include "Shape.h"
#include "SWFStream.h"
#include "MovieClip.h"
#include "SWF.h"
#include "Renderer.h"
#include "Global_as.h"
#include "Transform.h"

#include <algorithm>

// Define the macro below to always compute bounds for shape DisplayObjects
// and compare them with the bounds encoded in the SWF
//#define GNASH_DEBUG_SHAPE_BOUNDS 1

namespace gnash {
namespace SWF {

void
DefineShapeTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& r)
{
    assert(tag == DEFINESHAPE ||
           tag == DEFINESHAPE2 ||
           tag == DEFINESHAPE3 ||
           tag == DEFINESHAPE4 ||
           tag == DEFINESHAPE4_);

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("DefineShapeTag(%s): id = %d"), tag, id);
    );

    DefineShapeTag* ch = new DefineShapeTag(in, tag, m, r, id);
    m.addDisplayObject(id, ch);

}

DisplayObject*
DefineShapeTag::createDisplayObject(Global_as& gl, DisplayObject* parent) const
{
	return new Shape(getRoot(gl), 0, this, parent);
}
    
bool
DefineShapeTag::pointTestLocal(boost::int32_t x, boost::int32_t y, 
     const SWFMatrix& wm) const
{
    return geometry::pointTest(_shape.paths(), _shape.lineStyles(), x, y, wm);
}


DefineShapeTag::DefineShapeTag(SWFStream& in, TagType tag,
        movie_definition& m, const RunResources& r, boost::uint16_t id)
    :
    DefinitionTag(id),
    _shape(in, tag, m, r)
{
}

void
DefineShapeTag::display(Renderer& renderer, const Transform& xform) const
{
    renderer.drawShape(_shape, xform);
}

} // namespace SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
