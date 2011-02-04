// MorphShape.h: the MorphShape DisplayObject implementation for Gnash.
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

#ifndef GNASH_MORPH_SHAPE_H
#define GNASH_MORPH_SHAPE_H

#include "smart_ptr.h" 
#include "DisplayObject.h"
#include "swf/DefineMorphShapeTag.h"
#include <boost/intrusive_ptr.hpp>
#include <cassert>

namespace gnash {
    class Renderer;
}

namespace gnash {

/// A DisplayObject that tweens between two shapes.
//
/// A MorphShape has no properties of its own, but its inherited properties
/// may be read in AS3 using a reference to the object created with
/// getChildAt().
//
/// Morphing is controlled using a SWF::PlaceObject2 tag with a ratio flag.
/// The most common and efficient way is to combine this with a move flag,
/// so that this object is moved to the position specified by the ratio.
/// However, it is also possible to remove and replace, in which case a 
/// new MorphShape is created.
//
/// The starting position is not necessarily identical with shape1, though in
/// practice it often is.
class MorphShape : public DisplayObject
{

public:

    MorphShape(movie_root& mr, as_object* object,
            const SWF::DefineMorphShapeTag* def, DisplayObject* parent);

    virtual void display(Renderer& renderer, const Transform& xform);

    virtual SWFRect getBounds() const;
    
    virtual bool pointInShape(boost::int32_t  x, boost::int32_t  y) const;
 
    const SWF::ShapeRecord& shape() const {
        return _shape;
    }

private:
    
    void morph();

    double currentRatio() const;

    const boost::intrusive_ptr<const SWF::DefineMorphShapeTag> _def;
	
    SWF::ShapeRecord _shape;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
