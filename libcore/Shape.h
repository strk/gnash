// Shape.h: Shape DisplayObject implementation for Gnash.
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

#ifndef GNASH_SHAPE_H
#define GNASH_SHAPE_H

#include "DisplayObject.h"
#include "DefineShapeTag.h"
#include "DynamicShape.h"

#include <boost/intrusive_ptr.hpp>
#include <cassert>
#include <boost/shared_ptr.hpp>

namespace gnash {

/// For DisplayObjects that don't store unusual state in their instances.
//
/// A Shape may be either statically constructed during parsing or,
/// in AS3, dynamically constructed. A SWF-parsed Shape has an immutable
/// SWF::DefinitionTag. A dynamic Shape object has a DynamicShape.
class Shape : public DisplayObject
{

public:

    Shape(movie_root& mr, as_object* object, boost::shared_ptr<DynamicShape> sh,
            DisplayObject* parent)
        :
        DisplayObject(mr, object, parent),
        _shape(sh)
    {
        assert(_shape.get());
    }

	Shape(movie_root& mr, as_object* object, const SWF::DefineShapeTag* def,
            DisplayObject* parent)
		:
		DisplayObject(mr, object, parent),
		_def(def)
	{
	    assert(_def);
	}

	virtual void display(Renderer& renderer, const Transform& xform);

    virtual SWFRect getBounds() const {
        return _def ? _def->bounds() : _shape->getBounds();
    }
    
    virtual bool pointInShape(boost::int32_t x, boost::int32_t y) const;

private:
	
    const boost::intrusive_ptr<const SWF::DefineShapeTag> _def;

    boost::shared_ptr<DynamicShape> _shape;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
