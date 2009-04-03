// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "DisplayObject.h"
#include "shape_character_def.h"
#include <cassert>

// Forward declarations
namespace gnash {
    class character_def;
}

namespace gnash {

/// For DisplayObjects that don't store unusual state in their instances.
class Shape : public DisplayObject
{

public:

	Shape(shape_character_def* def, DisplayObject* parent, int id)
		:
		DisplayObject(parent, id),
		_def(def)
	{
	    assert(_def);
	}

	virtual void display();

    virtual rect getBounds() const {
        return _def->get_bound();
    }
    
    virtual bool pointInShape(boost::int32_t  x, boost::int32_t  y) const;

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	void markReachableResources() const
	{
		assert(isReachable());
        _def->setReachable();
		markDisplayObjectReachable();
	}
#endif

private:
	
    const boost::intrusive_ptr<shape_character_def> _def;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
