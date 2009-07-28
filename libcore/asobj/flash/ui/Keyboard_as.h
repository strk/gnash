// Keyboard_as.h:  ActionScript 3 "Keyboard" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_KEYBOARD_H
#define GNASH_ASOBJ3_KEYBOARD_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_object.h" // for inheritance
#include "GnashKey.h" // for key::code
#include "dsodefs.h"
#include <bitset>

namespace gnash {

// Forward declarations
class event_id;

class Keyboard_as : public as_object
{
protected:

#ifdef GNASH_USE_GC
    // Mark all key listeners as reachable
    void markReachableResources() const;
#endif // def GNASH_USE_GC

public:

    Keyboard_as();	
	static void init(as_object& where, const ObjectURI& uri);
	    // Pass SWF keycode, returns true if currently pressed.
    bool is_key_down(int keycode);

    // Pass gnash::key::code. Changes m_last_key_event
    // and adds appropriate SWF keycode to bit array of keys
    // pressed (_unreleasedKeys)
    void set_key_down(key::code code);

    // Pass gnash::key::code. Changes m_last_key_event
    // and removes appropriate SWF keycode from bit array of keys
    // pressed (_unreleasedKeys)
    void set_key_up(key::code code);
    
    int get_last_key() const;

    /// Responsible for user defined key events handlers only;
    /// take over both DisplayObjects and non-DisplayObjects object.
    void notify_listeners(const event_id& key_event_type);

private:
    /// bit-array for recording the unreleased keys
    std::bitset<key::KEYCOUNT> _unreleasedKeys;   

    typedef std::list<boost::intrusive_ptr<as_object> > Listeners;
    Listeners _listeners;

    int _lastKeyEvent;
};

} // gnash namespace

// GNASH_ASOBJ3_KEYBOARD_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

