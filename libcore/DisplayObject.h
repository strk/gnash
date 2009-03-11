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

#ifndef GNASH_DISPLAY_OBJECT_H
#define GNASH_DISPLAY_OBJECT_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character.h" // for inheritance
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
class DisplayObject : public character
{

public:

	DisplayObject(character* parent, int id)
		:
		character(parent, id)
	{
	}

    virtual ~DisplayObject() {}

    /// Render the DisplayObject.
    //
    /// All DisplayObjects must have a display() function.
	virtual void display() = 0;

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

    /// Search for StaticText objects
    //
    /// If this is a StaticText object and contains SWF::TextRecords, these
    /// are written to the passed parameter.
    /// @ return    0 if this object is not a StaticText or contains no text.
    virtual StaticText* getStaticText(std::vector<const SWF::TextRecord*>&)
    {
        return 0;
    }

	rect getBounds() const
    {
		return getDefinition()->get_bound();
	}

	/// Generic character is NEVER a mouse entity by default, so
	/// the default implementation of this method always returns NULL.
	/// Override it from subclasses that do can be mouse entities.
	///
	/// If you need to check for a generic character to contain a 
	/// given point, use the pointInShape() function instead.
	/// 
	virtual character* get_topmost_mouse_entity(boost::int32_t /*x*/, 
            boost::int32_t /*y*/)
	{
		return 0;
	}

	// See dox in character.h
	virtual bool pointInShape(boost::int32_t  x, boost::int32_t  y) const;

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

protected:

    /// Retrieve the immutable definition of this DisplayObject.
    //
    /// All subclasses must override this, but may return 0. In
    /// this case, they must also override any functions that
    /// call getDefinition().
    /// @ return    The immutable character_def of this DisplayObject
    ///             or 0 if none exists.
    virtual character_def* getDefinition() const = 0;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
