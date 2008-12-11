// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_GENERIC_CHARACTER_H
#define GNASH_GENERIC_CHARACTER_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character.h" // for inheritance
#include "shape_character_def.h" // for add_invalidated_bounds 

#include <cassert>

namespace gnash {

// Forward declarations
class character_def;

/// For characters that don't store unusual state in their instances.
//
/// @@AFAICT this is only used for shape characters
///
class generic_character : public character
{

protected:

	boost::intrusive_ptr<character_def> m_def;

#ifdef GNASH_USE_GC
	/// Mark reachabe resources (for the GC)
	//
	/// These are:
	///	- this char's definition (m_def)
	///
	void markReachableResources() const
	{
		assert(isReachable());
		m_def->setReachable();

		markCharacterReachable();
	}
#endif // GNASH_USE_GC

public:

	generic_character(character_def* def, character* parent, int id)
		:
		character(parent, id),
		m_def(def)
	{
	    assert(m_def);
	}

	/// generic characters can not handle mouse events, so
	/// the default implementation returns false.
	/// override in your subclass to change this
	virtual bool can_handle_mouse_event() const {
		return false;
	}

	virtual void display();

	rect getBounds() const
	{
		return m_def->get_bound();
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

	/// \brief
	/// Return the character definition from which this
	/// instance derive. 
	character_def* get_character_def() { return m_def.get(); }
  
	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
    

};


}	// end namespace gnash


#endif // GNASH_GENERIC_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
