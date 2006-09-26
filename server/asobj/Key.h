// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.

/* $Id: Key.h,v 1.4 2006/09/26 21:17:04 nihilus Exp $ */

#ifndef __KEY_H__
#define __KEY_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
//#include "as_object.h" // for inheritance

#ifdef WIN32
#	undef _CONTROL
#	undef _SPACE
#	undef _UP
#endif

namespace gnash {
  
class Key {
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

void key_addlistener(const fn_call& fn);
void key_getascii(const fn_call& fn);
void key_getcode(const fn_call& fn);
void key_isdown(const fn_call& fn);
void key_istoggled(const fn_call& fn);
void key_removelistener(const fn_call& fn);

/************************************************************************
 *
 * This has been moved from action.cpp, when things are clean
 * everything should have been moved up
 *
 ************************************************************************/

class key_as_object : public as_object
{

private:
	uint8_t	m_keymap[key::KEYCOUNT / 8 + 1];	// bit-array
	std::vector<weak_ptr<as_object> >	m_listeners;
	int	m_last_key_pressed;

	void notify_listeners(const tu_stringi& funcname);
public:
	key_as_object();

	bool is_key_down(int code);

	void set_key_down(int code);

	void set_key_up(int code);

	void add_listener(as_object* listener);

	void remove_listener(as_object* listener);

	int get_last_key_pressed() const;
};

void key_init(as_object* global);

} // end of gnash namespace

// __KEY_H__
#endif

