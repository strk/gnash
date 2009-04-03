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

#ifndef GNASH_INTERACTIVE_DISPLAY_OBJECT_H
#define GNASH_INTERACTIVE_DISPLAY_OBJECT_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "DisplayObject.h" // for inheritance
#include "character_def.h"

#include <cassert>

namespace gnash {
    class character_def;
    class StaticText;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {

/// The base class for all rendered objects on the Stage.
//
/// Objects of type DisplayObject are non-interactive.
class InteractiveDisplayObject : public DisplayObject
{

public:

	InteractiveDisplayObject(DisplayObject* parent, int id)
		:
		DisplayObject(parent, id)
	{
	}

    virtual ~InteractiveDisplayObject() {}

    /// Render this DisplayObject
    virtual void display() {}

    /// Whether the DisplayObject can handle a mouse event.
    //
    /// Normal DisplayObjects apparently cannot handle
    /// mouse events.
    /// @return     true if the DisplayObject can handle mouse
    ///             events
	virtual bool can_handle_mouse_event() const
    {
		return false;
	}

    /// Allow extraction of static text.
    //
    /// Default returns 0, implemented only for DefineText though
    /// DisplayObject.
    virtual StaticText* getStaticText(std::vector<const SWF::TextRecord*>&,
            size_t&)
    {
        return 0;
    }

    /// Returns local, untransformed bounds of this DisplayObject in TWIPS
    //
    /// The default implementation prints an error and returns a NULL rect.
    ///
    /// Container DisplayObjects (sprite and buttons) return the composite
    /// bounds of all their childrens, appropriaterly transformed with
    /// their local SWFMatrix.
    ///
    virtual rect getBounds() const
    {
        log_error("FIXME: InteractiveDisplayObject %s did not override the "
                "getBounds() method", typeName(*this));
        return rect();
    }

    /// \brief
    /// Return the topmost entity covering the given point
    /// and enabled to receive mouse events.
    //
    /// Return NULL if no "active" entity is found under the pointer.
    ///
    /// Coordinates of the point are given in parent's coordinate space.
    /// This means that in order to convert the point to the local coordinate
    /// space you need to apply an inverse transformation using this
    /// DisplayObject SWFMatrix. Example:
    ///
    /// point p(x,y);
    /// getMatrix().transform_by_inverse(p);
    /// -- p is now in local coordinates
    ///
    /// Don't blame me for this mess, I'm just trying to document the existing
    /// functions ... --strk
    ///
    /// @param x
    ///     X ordinate of the pointer, in parent's coordinate space.
    ///
    /// @param y
    ///     Y ordinate of the pointer, in parent's coordiante space.
    ///
    virtual InteractiveDisplayObject* get_topmost_mouse_entity(boost::int32_t /*x*/,
            boost::int32_t /*y*/)
    {
        return 0;
    }

    /// Return true if the given point falls in this DisplayObject's shape
    //
    /// Point coordinates are in world TWIPS
    ///
    /// The default implementation warns about a missing
    /// override and invokes pointInBounds().
    ///
    ///
    virtual bool pointInShape(boost::int32_t x, boost::int32_t y) const
    {
        log_error("Character %s did not override pointInShape() - "
                "using pointInBounds() instead", typeid(*this).name());
        return pointInBounds(x, y);
    }	// See dox in DisplayObject.h


	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force) = 0;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
