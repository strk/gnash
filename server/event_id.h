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


#ifndef GNASH_EVENT_ID_H
#define GNASH_EVENT_ID_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "gnash.h" // for gnash::key namespace

#include <cwchar>

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
		SOCK_CLOSE,
		SOCK_CONNECT,
		SOCK_DATA,
		SOCK_XML,
		
		// These are for the XML ActionScript only
		XML_LOAD,
		XML_DATA,
		
		// This is for setInterval
		TIMER,

		CONSTRUCT,
		SETFOCUS,
		KILLFOCUS,

		EVENT_COUNT
	};

	id_code	m_id;
	unsigned char	m_key_code;
	uint32_t m_unicode;

	event_id() : m_id(INVALID), m_key_code(key::INVALID), m_unicode(0) {}

	event_id(id_code id, key::code c = key::INVALID, uint32_t utf_8 = 0)
		:
		m_id(id),
		m_key_code((unsigned char) c),
		m_unicode ((uint32_t) utf_8)
	{
		// you must supply a key code for KEY_PRESS event
		// 
		// we do have a testcase with m_id == KEY_PRESS, and m_key_code==0(KEY_INVALID)
		// see key_event_test.swf(produced by Ming)
		// 
		//assert((m_key_code == key::INVALID && (m_id != KEY_PRESS))
		//	|| (m_key_code != key::INVALID && (m_id == KEY_PRESS)));
	}

	void setKeyCode(unsigned char key)
	{
		m_key_code = key;
	}

	bool	operator==(const event_id& id) const { return m_id == id.m_id && m_key_code == id.m_key_code; }

	bool operator< (const event_id& id) const
	{
		if ( m_id < id.m_id ) return true;
		if ( m_id > id.m_id ) return false;

		// m_id are equal, check key code
		if ( m_key_code < id.m_key_code ) return true;
		return false;
	}

	/// Return the name of a method-handler function
	/// corresponding to this event.
	const std::string& get_function_name() const;

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
