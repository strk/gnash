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
//

// SWF buttons.  Mouse-sensitive update/display, actions, etc.


#ifndef GNASH_BUTTON_H
#define GNASH_BUTTON_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "InteractiveObject.h" // for inheritance

#include <vector>
#include <set>

// Forward declarations.
namespace gnash {
    namespace SWF {
        class DefineButtonTag;
    }
}

namespace gnash {
//
// Button
//

class Button : public InteractiveObject
{
public:

	typedef std::vector<DisplayObject*> DisplayObjects;
	typedef std::vector<const DisplayObject*> ConstDisplayObjects;
	
    /// A container for holding the id of active button records.
    typedef std::set<int> ActiveRecords;

	enum mouse_flags
	{
		FLAG_IDLE = 0,
		FLAG_OVER = 1,
		FLAG_DOWN = 2,
		OVER_DOWN = FLAG_OVER | FLAG_DOWN,

		// aliases
		OVER_UP = FLAG_OVER,
		OUT_DOWN = FLAG_DOWN
	};

	enum MouseState
	{
		MOUSESTATE_UP = 0,
		MOUSESTATE_DOWN,
		MOUSESTATE_OVER,
		MOUSESTATE_HIT
	};

	Button(const SWF::DefineButtonTag* const def, DisplayObject* parent,
            int id);

	~Button();
	
    static const char* mouseStateName(MouseState s);

    /// Initialize the global Button class
    static void init(as_object& global);

	// See dox in as_object.h
	bool get_member(string_table::key name, as_value* val, 
		string_table::key nsname = 0);

	bool mouseEnabled() const { return true; }

    virtual bool trackAsMenu();

	// called from keypress listener only
	bool on_event(const event_id& id);

	void display(Renderer& renderer);
	
	void set_current_state(MouseState new_state);

	/// \brief
	/// Return the topmost entity that the given point covers. 
	/// NULL if none.
	//
	/// I.e. check against ourself.
	///
	virtual InteractiveObject* topmostMouseEntity(boost::int32_t x,
            boost::int32_t y);
	
	virtual bool wantsInstanceName() const
	{
		return true; // buttons can be referenced 
	}
	
	/// Overridden to look in button records for a match
	virtual as_object* get_path_element(string_table::key key);

	virtual void mouseEvent(const event_id& event);

    virtual bool handleFocus();

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
	
	virtual rect getBounds() const;
	
	// See dox in DisplayObject.h
	bool pointInShape(boost::int32_t x, boost::int32_t y) const;

	bool isEnabled();
	
	/// Receive a stage placement notification
	//
	/// This callback will:
	///
	/// (1) Register this button instance as a live DisplayObject
	/// (2) Setup the state DisplayObjects calling stagePlacementCallback
    /// on all [WRONG]
	virtual void stagePlacementCallback(as_object* initObj = 0);

	/// Properly unload contained DisplayObjects
	bool unload();

	/// Properly destroy contained DisplayObjects
	void destroy();

#ifdef USE_SWFTREE
	// Override to append button DisplayObjects info, see dox in DisplayObject.h
	virtual InfoTree::iterator getMovieInfo(InfoTree& tr,
            InfoTree::iterator it);
#endif

protected:

#ifdef GNASH_USE_GC
	/// Mark reachabe resources (for the GC)
	//
	/// These are:
	///	- this char's definition (_def)
	///	- the vector of state DisplayObjects (_stateCharacters)
	///	- the vector of hit DisplayObjects (_hitCharacters)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:

	int	_lastMouseFlags, _mouseFlags;

	MouseState _mouseState;
    
    const boost::intrusive_ptr<const SWF::DefineButtonTag> _def;

	DisplayObjects _stateCharacters;

	DisplayObjects _hitCharacters;

	/// Returns all DisplayObjects that are active based on the current state.
	//
	/// The "_visible" property does not matter here. 
	///
	/// @param list
	///	The container to push active DisplayObjects into
	///
	/// @param includeUnloaded
	///	If true, include unloaded but still reachable chars in the records slot.
	///
	void getActiveCharacters(DisplayObjects& list, bool includeUnloaded=false);

    /// Returns all DisplayObjects that are active based on the current state.
    //
    /// This is a const method because the returned DisplayObjects cannot be
    /// modified.
    ///
    /// @param list     The container to push unmodifiable DisplayObjects into.
	void getActiveCharacters(ConstDisplayObjects& list) const;

	/// Returns all DisplayObjects (record nums) that should be active on
    /// the given state.
	//
	/// @param list
	///	The set to push active DisplayObjects record number into
	///
	/// @param state
	///	The state we're interested in
	///
	void get_active_records(ActiveRecords& list, MouseState state);

	/// Return any state DisplayObject whose name matches the given string
	//
	/// NOTE: both active and inactive childs are scanned for
	///
	/// @param name
	///	Name to match, search is case sensitive for SWF7 and higher,
	///     case insensitive up to SWF6.
	///
	DisplayObject* getChildByName(const std::string& name);

	/// \brief
	/// Return version of the SWF containing
	/// the button definition this is an instance of.
    int getMovieVersion() const;

};

}	// end namespace gnash


#endif // GNASH_BUTTON_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
