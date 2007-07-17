// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 

/* $Id: Key.h,v 1.23 2007/07/17 06:04:22 zoulunkai Exp $ */

#ifndef __KEY_H__
#define __KEY_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "event_id.h"
#include "gnash.h" // for gnash::key namespace

#ifdef WIN32
#	undef _CONTROL
#	undef _SPACE
#	undef _UP
#endif

namespace gnash {

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
class KeyListener; //forward declaration
#endif

// TODO: drop this, probably unused
class DSOEXPORT Key {
public:
    Key();
    ~Key();
   void addListener();
   void getAscii();
   void getCode();
   void isDown();
   void isToggled();
   void removeListener();
private:
    bool _BACKSPACE;
    bool _CAPSLOCK;
    bool _CONTROL;
    bool _DELETEKEY;
    bool _DOWN;
    bool _END;
    bool _ENTER;
    bool _ESCAPE;
    bool _HOME;
    bool _INSERT;
    bool _LEFT;
    bool _onKeyDown;
    bool _onKeyUp;
    bool _PGDN;
    bool _PGUP;
    bool _RIGHT;
    bool _SHIFT;
    bool _SPACE;
    bool _TAB;
    bool _UP;
};

//class key_as_object : public as_object
//{
//public:
    //Key obj;
//};

as_value key_addlistener(const fn_call& fn);
as_value key_getascii(const fn_call& fn);
as_value key_getcode(const fn_call& fn);
as_value key_isdown(const fn_call& fn);
as_value key_istoggled(const fn_call& fn);
as_value key_removelistener(const fn_call& fn);

/************************************************************************
 *
 * This has been moved from action.cpp, when things are clean
 * everything should have been moved up
 *
 ************************************************************************/

class DSOEXPORT key_as_object : public as_object
{

private:
	/// bit-array for recording the unreleased keys
	uint8_t	m_unreleased_keys[key::KEYCOUNT / 8 + 1];	

#ifndef NEW_KEY_LISTENER_LIST_DESIGN
	typedef std::vector<boost::intrusive_ptr<as_object> > Listeners;
	Listeners m_listeners;
#endif

	int	m_last_key_pressed;

protected:

#ifdef GNASH_USE_GC
#ifndef NEW_KEY_LISTENER_LIST_DESIGN
	// Mark all key listeners as reachable
	// (this class has no direct pointer to listeners when
	//  NEW_KEY_LISTENER_LIST_DESIGN is defined)
	void markReachableResources() const;
#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN
#endif // def GNASH_USE_GC

public:

	key_as_object();

	bool is_key_down(int code);

	void set_key_down(int code);

	void set_key_up(int code);
	
#ifndef NEW_KEY_LISTENER_LIST_DESIGN
	/// responsible for user defined key events handlers only;
	/// take over both characters and non-characters object.
	void notify_listeners(const event_id key_event_type);
#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	void add_listener(const KeyListener& listener);

	void remove_listener(boost::intrusive_ptr<as_object> listener);
#else  
	void add_listener(boost::intrusive_ptr<as_object> listener);

	void remove_listener(boost::intrusive_ptr<as_object> listener);
#endif

	int get_last_key_pressed() const;

};

void key_class_init(as_object& global);

} // end of gnash namespace

// __KEY_H__
#endif

