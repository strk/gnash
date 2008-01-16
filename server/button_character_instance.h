// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
//
//

// SWF buttons.  Mouse-sensitive update/display, actions, etc.

/* $Id: button_character_instance.h,v 1.29 2008/01/16 10:20:29 strk Exp $ */

#ifndef GNASH_BUTTON_CHARACTER_INSTANCE_H
#define GNASH_BUTTON_CHARACTER_INSTANCE_H


#include "character.h" // for inheritance


namespace gnash {

// Forward declarations
	class sprite_instance;
	class button_character_definition;

//
// button characters
//

enum mouse_state
{
	MOUSE_UP,
	MOUSE_DOWN,
	MOUSE_OVER
};

//
// button_character_instance
//

class button_character_instance : public character
{
public:
	button_character_definition*	m_def;

	typedef std::vector< boost::intrusive_ptr<character> > CharsVect;

	CharsVect m_record_character;

	enum mouse_flags
	{
		IDLE = 0,
		FLAG_OVER = 1,
		FLAG_DOWN = 2,
		OVER_DOWN = FLAG_OVER|FLAG_DOWN,

		// aliases
		OVER_UP = FLAG_OVER,
		OUT_DOWN = FLAG_DOWN
	};
	int	m_last_mouse_flags, m_mouse_flags;
	enum e_mouse_state
	{
		UP = 0,
		DOWN,
		OVER
	};
	e_mouse_state m_mouse_state;

	button_character_instance(button_character_definition* def,
			character* parent, int id);

	~button_character_instance();

	bool can_handle_mouse_event() const { return true; }

	// called from keypress listener only
	bool on_event(const event_id& id);

	void	restart();

	virtual void	advance();

	void	display();
	
	void set_current_state(e_mouse_state new_state);
	
	/// Returns all characters that are currently visible based on the
	/// current button state. The "_visible" property does not matter here. 
	void get_active_characters(std::vector<character*>& list);
	void get_active_characters(std::vector<character*>& list, e_mouse_state state);
	

	/// Combine the flags to avoid a conditional.
	//  It would be faster with a macro.
	inline int	transition(int a, int b) const
	{
		return (a << 2) | b;
	}


	/// \brief
	/// Return the topmost entity that the given point covers. 
	/// NULL if none.
	//
	/// I.e. check against ourself.
	///
	virtual character* get_topmost_mouse_entity(float x, float y);
	
	virtual bool wantsInstanceName() const
	{
		return true; // buttons can be referenced 
	}
	
	/// Overridden to look in button records for a match
	virtual as_object* get_path_element(string_table::key key);

	virtual void	on_button_event(const event_id& event);

	//
	// ActionScript overrides
	//

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
	
	geometry::Range2d<float> getBounds() const;
	
	// See dox in character.h
	bool pointInShape(float x, float y) const;

	static as_value enabled_getset(const fn_call& fn);
	
	bool get_enabled();
	void set_enabled(bool value);
	
	/// Receive a stage placement notification
	//
	/// This callback will:
	///
	/// (1) Register this button instance as a live character
	/// (2) Setup the state characters calling stagePlacementCallback on all [WRONG]
	///
	virtual void stagePlacementCallback();

	/// Properly unload contained characters
	bool unload();

protected:

#ifdef GNASH_USE_GC
	/// Mark reachabe resources (for the GC)
	//
	/// These are:
	///	- this char's definition (m_def)
	///	- the vector of state characters (m_record_character)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:
	bool m_enabled;

};

}	// end namespace gnash


#endif // GNASH_BUTTON_CHARACTER_INSTANCE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
