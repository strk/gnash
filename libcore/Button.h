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
//

// SWF buttons.  Mouse-sensitive update/display, actions, etc.


#ifndef GNASH_BUTTON_H
#define GNASH_BUTTON_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character.h" // for inheritance

#include <vector>
#include <set>

// Forward declarations.
namespace gnash {
	class MovieClip;
    namespace SWF {
        class DefineButtonTag;
    }
}

namespace gnash {
//
// Button
//

class Button : public character
{
public:

	typedef std::vector< character* > CharsVect;
	typedef std::set<int> RecSet;

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

	enum MouseState
	{
		UP = 0,
		DOWN,
		OVER,
		HIT
	};

	static const char* mouseStateName(MouseState s);

	MouseState m_mouse_state;

	Button(SWF::DefineButtonTag& def, character* parent, int id);

	~Button();

	// See dox in as_object.h
	bool get_member(string_table::key name, as_value* val, 
		string_table::key nsname = 0);

	bool can_handle_mouse_event() const { return true; }

	// called from keypress listener only
	bool on_event(const event_id& id);

	void restart();

	void display();
	
	void set_current_state(MouseState new_state);

	/// \brief
	/// Return the topmost entity that the given point covers. 
	/// NULL if none.
	//
	/// I.e. check against ourself.
	///
	virtual character* get_topmost_mouse_entity(boost::int32_t x,
            boost::int32_t y);
	
	virtual bool wantsInstanceName() const
	{
		return true; // buttons can be referenced 
	}
	
	/// Overridden to look in button records for a match
	virtual as_object* get_path_element(string_table::key key);

	virtual void on_button_event(const event_id& event);

    virtual bool handleFocus();

	//
	// ActionScript overrides
	//

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
	
	rect getBounds() const;
	
	// See dox in character.h
	bool pointInShape(boost::int32_t x, boost::int32_t y) const;

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
	virtual void stagePlacementCallback(as_object* initObj = 0);

	/// Properly unload contained characters
	bool unload();

	/// Properly destroy contained characters
	void destroy();

#ifdef USE_SWFTREE
	// Override to append button characters info, see dox in character.h
	virtual InfoTree::iterator getMovieInfo(InfoTree& tr, InfoTree::iterator it);
#endif

protected:

#ifdef GNASH_USE_GC
	/// Mark reachabe resources (for the GC)
	//
	/// These are:
	///	- this char's definition (m_def)
	///	- the vector of state characters (_stateCharacters)
	///	- the vector of hit characters (_hitCharacters)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:

    SWF::DefineButtonTag& _def;

	CharsVect _stateCharacters;

	CharsVect _hitCharacters;

	/// Returns all characters that are active based on the current state.
	//
	/// The "_visible" property does not matter here. 
	///
	/// @param list
	///	The container to push active characters into
	///
	/// @param includeUnloaded
	///	If true, include unloaded but still reachable chars in the records slot.
	///
	void getActiveCharacters(std::vector<character*>& list,
			bool includeUnloaded=false);

    /// Returns all characters that are active based on the current state.
    //
    /// This is a const method because the returned characters cannot be
    /// modified.
    ///
    /// @param list     The container to push unmodifiable characters into.
	void getActiveCharacters(std::vector<const character*>& list) const;

	/// Returns all characters (record nums) that should be active on the given state.
	//
	/// @param list
	///	The set to push active characters record number into
	///
	/// @param state
	///	The state we're interested in
	///
	void get_active_records(RecSet& list, MouseState state);

	/// Return any state character whose name matches the given string
	//
	/// NOTE: both active and inactive childs are scanned for
	///
	/// @param name
	///	Name to match, search is case sensitive for SWF7 and higher,
	///     case insensitive up to SWF6.
	///
	character * getChildByName(const std::string& name);

	/// \brief
	/// Return version of the SWF containing
	/// the button definition this is an instance of.
    int getSWFVersion() const;

	bool m_enabled;

};

/// Initialize the global Button class
void button_class_init(as_object& global);

}	// end namespace gnash


#endif // GNASH_BUTTON_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
