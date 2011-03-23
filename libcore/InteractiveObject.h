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

#ifndef GNASH_INTERACTIVE_DISPLAY_OBJECT_H
#define GNASH_INTERACTIVE_DISPLAY_OBJECT_H

#include "DisplayObject.h" // for inheritance
#include "log.h"
#include "as_object.h" // for getRoot()

#include <vector>
#include <cassert>

namespace gnash {
    class StaticText;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {

/// The base class for interactive objects.
//
/// Objects of type InteractiveObject can receive focus, mouse events,
/// and key events for user interaction.
//
/// Derived classes include TextField, Button, and MovieClip.
class InteractiveObject : public DisplayObject
{

public:

	InteractiveObject(as_object* object, DisplayObject* parent)
		:
		DisplayObject(getRoot(*object), object, parent)
	{
        // It's a bit too late for this assertion as we've already
        // deferenced it. All InteractiveObjects are AS-referenceable,
        // so they must have an object.
        assert(object);
	}

    virtual ~InteractiveObject() {}

    /// Render this InteractiveObject
	virtual void display(Renderer& renderer, const Transform& xform) = 0;

    /// Whether the DisplayObject can handle a mouse event.
    //
    /// @return     true if the DisplayObject can handle mouse
    ///             events
	virtual bool mouseEnabled() const = 0;

    /// ActionScript property of Buttons and MovieClips altering mouse handling
    virtual bool trackAsMenu() {
        return false;
    }

    /// Allow extraction of static text.
    //
    /// Default returns 0, implemented only for DefineText though
    /// DisplayObject.
    virtual StaticText* getStaticText(std::vector<const SWF::TextRecord*>&,
            size_t&) {
        return 0;
    }

    /// Returns local, untransformed bounds of this DisplayObject in TWIPS
    //
    /// Container DisplayObjects (sprite and buttons) return the composite
    /// bounds of all their children, appropriately transformed with
    /// their local SWFMatrix.
    virtual SWFRect getBounds() const = 0;

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
    virtual InteractiveObject* topmostMouseEntity(boost::int32_t /*x*/,
            boost::int32_t /*y*/) = 0;

    virtual void mouseEvent(const event_id& id)
    {
        notifyEvent(id);
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
    }

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force) = 0;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
