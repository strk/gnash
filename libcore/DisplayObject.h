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
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {

/// For characters that don't store unusual state in their instances.
//
/// @@AFAICT this is only used for shape characters
///
class DisplayObject : public character
{

public:

	DisplayObject(character* parent, int id)
		:
		character(parent, id)
	{
	}

    virtual ~DisplayObject() {}

	/// generic characters can not handle mouse events, so
	/// the default implementation returns false.
	/// override in your subclass to change this
	virtual bool can_handle_mouse_event() const {
		return false;
	}

    virtual DisplayObject* getStaticText(std::vector<const SWF::TextRecord*>&);

	virtual void display() = 0;

	rect getBounds() const {
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
		return NULL;
	}

	// See dox in character.h
	virtual bool pointInShape(boost::int32_t  x, boost::int32_t  y) const;

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

protected:

    virtual character_def* getDefinition() const = 0;

#ifdef GNASH_USE_GC
	/// Mark reachabe resources (for the GC)
	//
	/// These are:
	///	- this char's definition (m_def)
	///
	void markReachableResources() const
	{
		assert(isReachable());
		getDefinition()->setReachable();

		markCharacterReachable();
	}
#endif // GNASH_USE_GC

};


}	// end namespace gnash


#endif // GNASH_GENERIC_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
