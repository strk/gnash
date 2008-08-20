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


#ifndef GNASH_EVENT_ID_H
#define GNASH_EVENT_ID_H

#include "string_table.h"
#include "dsodefs.h"

#include "GnashKey.h" // for gnash::key::code

namespace gnash {

/// For keyDown and stuff like that.
//
/// Implementation is currently in action.cpp
///
class DSOEXPORT event_id
{
public:
	/// These must match the function names in event_id::get_function_name()
	enum id_code
	{
		INVALID,

		// These are for buttons & sprites.
		PRESS,
		RELEASE,
		RELEASE_OUTSIDE,
		ROLL_OVER,
		ROLL_OUT,
		DRAG_OVER,
		DRAG_OUT,
		KEY_PRESS,

		// These are for sprites only.
		INITIALIZE,
		LOAD,
		UNLOAD,
		ENTER_FRAME,
		MOUSE_DOWN,
		MOUSE_UP,
		MOUSE_MOVE,
		KEY_DOWN,
		KEY_UP,
		DATA,
		
		// These are for the MoveClipLoader ActionScript only
		LOAD_START,
		LOAD_ERROR,
		LOAD_PROGRESS,
		LOAD_INIT,
		
		// These are for the XMLSocket ActionScript only
		CLOSE,
		CONNECT,
		XML,
		
		// This is for setInterval
		TIMER,

		CONSTRUCT,
		SETFOCUS,
		KILLFOCUS,

		EVENT_COUNT
	};

	id_code	m_id;
	
	// keyCode must be the unique gnash key identifier
	// gnash::key::code.
	// edit_text_character has to be able to work out the
	// ASCII value from keyCode, while other users need 
	// the SWF code or the Flash key code.
	key::code keyCode;

	event_id() : m_id(INVALID), keyCode(key::INVALID) {}

	event_id(id_code id, key::code c = key::INVALID)
		:
		m_id(id),
		keyCode(c)
	{
		// you must supply a key code for KEY_PRESS event
		// 
		// we do have a testcase with m_id == KEY_PRESS, and keyCode==0(KEY_INVALID)
		// see key_event_test.swf(produced by Ming)
		// 
		//assert((keyCode == key::INVALID && (m_id != KEY_PRESS))
		//	|| (keyCode != key::INVALID && (m_id == KEY_PRESS)));
	}

	///
	/// @param SWFKey The SWF code matched to the event. This
	/// must be converted to a unique gnash::key::code.
	void setKeyCode(boost::uint8_t SWFkey)
	{
		// Lookup the SWFcode in the gnash::key::code table.
		// Some are not unique (keypad numbers are the
		// same as normal numbers), so we take the first match.
		// As long as we can work out the SWFCode from the
		// gnash::key::code it's all right.
		int i = 0;
		while (key::codeMap[i][key::SWF] != SWFkey && i < key::KEYCOUNT) i++;

		if (i == key::KEYCOUNT) keyCode = key::INVALID;
		else keyCode = static_cast<key::code>(i);
	}

	bool	operator==(const event_id& id) const
	{
	    return m_id == id.m_id && keyCode == id.keyCode;
	}

	bool operator< (const event_id& id) const
	{
		if ( m_id < id.m_id ) return true;
		if ( m_id > id.m_id ) return false;

		// m_id are equal, check key code
		if ( keyCode < id.keyCode ) return true;
		return false;
	}

	/// Return the name of a method-handler function
	/// corresponding to this event.
	const std::string& get_function_name() const;

	/// Return the string_table key of a method-handler function
	/// corresponding to this event.
	string_table::key get_function_key() const;

	/// \brief
	/// Return true if this is a mouse event
	/// (triggerable with a mouse activity)
	bool is_mouse_event() const;
  
	/// Return true if this is a key event
	bool is_key_event() const;

	/// Return true if this is a button-like event
	//
	/// Button-like events are: PRESS, RELEASE, RELEASE_OUTSIDE,
	///                         ROLL_OVER, ROLL_OUT,
	///                         DRAG_OVER, DRAG_OUT,
	///                         KEY_PRESS
	///
	/// TODO: check if we need anything more
	///       The way to test is using the 'enabled'
	///       property to see which ones are disabled
	///       by setting it to false.
	///
	bool is_button_event() const;

	id_code id() const { return m_id; }
};

}	// end namespace gnash


#endif // GNASH_EVENT_ID_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
