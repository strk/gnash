// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


#ifndef HAVE_KEY_H
#define HAVE_KEY_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "GnashKey.h" // for key::code
#include "dsodefs.h"
#include <bitset>

namespace gnash {

class event_id;
    
/************************************************************************
 *
 * This has been moved from action.cpp, when things are clean
 * everything should have been moved up
 *
 ************************************************************************/

class Key_as : public as_object
{

protected:

#ifdef GNASH_USE_GC
    // Mark all key listeners as reachable
    void markReachableResources() const;
#endif // def GNASH_USE_GC

public:

    Key_as();

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
    /// take over both characters and non-characters object.
    void notify_listeners(const event_id& key_event_type);

private:
    /// bit-array for recording the unreleased keys
    std::bitset<key::KEYCOUNT> _unreleasedKeys;   

    typedef std::list<boost::intrusive_ptr<as_object> > Listeners;
    Listeners _listeners;

    int _lastKeyEvent;
};

void key_class_init(as_object& global);

} // end of gnash namespace

// HAVE_KEY_H
#endif

