// Key.cpp:  ActionScript "Key" class (keyboards), for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Key.h"
#include "fn_call.h"
#include "movie_root.h"
#include "action.h" // for call_method
#include "VM.h"
#include "builtin_function.h"
#include "Object.h" // for getObjectInterface()
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"

#include <boost/algorithm/string/case_conv.hpp>

namespace gnash {

/************************************************************************
*
* This has been moved from action.cpp, when things are clean
* everything should have been moved up
*
************************************************************************/

key_as_object::key_as_object()
    :
    as_object(getObjectInterface()),
    m_last_key_event(0)
{
	memset(m_unreleased_keys, 0, sizeof(m_unreleased_keys));

	// Key is a broadcaster only in SWF6 and up (correct?)
	int swfversion = VM::get().getSWFVersion();
	if ( swfversion > 5 )
	{
		AsBroadcaster::initialize(*this);
	}
}

bool
key_as_object::is_key_down(int keycode)
{
    if (keycode < 0 || keycode >= key::KEYCOUNT) return false;

    // Select the relevant byte of the bit array:
    int byte_index = keycode >> 3;
    // Find bit within the byte:
    int bit_index = keycode - (byte_index << 3);

    uint8_t mask = 1 << bit_index;

    if ((m_unreleased_keys[byte_index] & mask) != 0 ) return true;

    return false;
}

void
    key_as_object::set_key_down(int code)
{
    if (code < 0 || code >= key::KEYCOUNT) return;

    // This is used for getAscii() of the last key event, so we use gnash's
    // internal code.
    m_last_key_event = code;

    // Key.isDown() only cares about flash keycode, not character, so
    // we lookup keycode to add to m_unreleased_keys.   

    int byte_index = key::codeMap[code][1] >> 3;
    int bit_index = key::codeMap[code][1] - (byte_index << 3);
    int mask = 1 << bit_index;

    assert(byte_index >= 0 && byte_index < int(sizeof(m_unreleased_keys)/sizeof(m_unreleased_keys[0])));

    m_unreleased_keys[byte_index] |= mask;
}

void
key_as_object::set_key_up(int code)
{
    if (code < 0 || code >= key::KEYCOUNT) return;

    // This is used for getAscii() of the last key event, so we use gnash's
    // internal code.    
    m_last_key_event = code;

    // Key.isDown() only cares about flash keycode, not character, so
    // we lookup keycode to add to m_unreleased_keys.
    int byte_index = key::codeMap[code][1] >> 3;
    int bit_index = key::codeMap[code][1] - (byte_index << 3);
    int mask = 1 << bit_index;

    assert(byte_index >= 0 && byte_index < int(sizeof(m_unreleased_keys)/sizeof(m_unreleased_keys[0])));

    m_unreleased_keys[byte_index] &= ~mask;
}


void 
key_as_object::notify_listeners(const event_id& key_event)
{  
	// There is no user defined "onKeyPress" event handler
	if( (key_event.m_id != event_id::KEY_DOWN) && (key_event.m_id != event_id::KEY_UP) ) return;

	std::string handler_name = PROPNAME(key_event.get_function_name());

	as_value ev(handler_name);

	/// no environment to start with...
	as_environment env;

	log_debug("notify_listeners calling broadcastMessage with arg %s", ev.to_debug_string().c_str());
	callMethod(NSV::PROP_BROADCAST_MESSAGE, ev);
}

int
key_as_object::get_last_key() const
{
    return m_last_key_event;
}


/// Return the ascii number of the last key pressed.
static as_value   
key_get_ascii(const fn_call& fn)
{
    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);

    int code = ko->get_last_key();

    return as_value(gnash::key::codeMap[code][2]);
}

/// Returns the keycode of the last key pressed.
static as_value   
    key_get_code(const fn_call& fn)
{
    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);

    int code = ko->get_last_key();

    return as_value(key::codeMap[code][1]);
}

/// Return true if the specified (first arg keycode) key is pressed.
static as_value   
key_is_down(const fn_call& fn)
{
    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Key.isDown needs one argument (the key code)"));
        );
        return as_value();
    }

    int keycode = fn.arg(0).to_number<int>();

    return as_value(ko->is_key_down(keycode));
}

/// \brief
/// Given the keycode of NUM_LOCK or CAPSLOCK, returns true if
/// the associated state is on.
///
static as_value   
key_is_toggled(const fn_call& /* fn */)
{
    log_unimpl("Key.isToggled");
    // @@ TODO
    return as_value(false);
}

void key_class_init(as_object& global)
{

    //  GNASH_REPORT_FUNCTION;
    //

    // Create built-in key object.
    // NOTE: _global.Key *is* an object, not a constructor
    as_object*  key_obj = new key_as_object;

    // constants
#define KEY_CONST(k) key_obj->init_member(#k, key::codeMap[key::k][1])
    KEY_CONST(BACKSPACE);
    KEY_CONST(CAPSLOCK);
    KEY_CONST(CONTROL);
    KEY_CONST(DELETEKEY);
    KEY_CONST(DOWN);
    KEY_CONST(END);
    KEY_CONST(ENTER);
    KEY_CONST(ESCAPE);
    KEY_CONST(HOME);
    KEY_CONST(INSERT);
    KEY_CONST(LEFT);
    KEY_CONST(PGDN);
    KEY_CONST(PGUP);
    KEY_CONST(RIGHT);
    KEY_CONST(SHIFT);
    KEY_CONST(SPACE);
    KEY_CONST(TAB);
    KEY_CONST(UP);

    // methods
    key_obj->init_member("getAscii", new builtin_function(key_get_ascii));
    key_obj->init_member("getCode", new builtin_function(key_get_code));
    key_obj->init_member("isDown", new builtin_function(key_is_down));
    key_obj->init_member("isToggled", new builtin_function(key_is_toggled));

    // These are only for SWF6 and up
#if 0 // done by AsBroadcaster
    int swfversion = VM::get().getSWFVersion();
    if ( swfversion > 5 )
    {
        key_obj->init_member("addListener", new builtin_function(key_add_listener));
        key_obj->init_member("removeListener", new builtin_function(key_remove_listener));
    }
#endif

    global.init_member("Key", key_obj);
}

#ifdef GNASH_USE_GC
void
key_as_object::markReachableResources() const
{
    markAsObjectReachable();
    for (Listeners::const_iterator i=m_listeners.begin(), e=m_listeners.end();
                i != e; ++i)
    {
        (*i)->setReachable();
    }
}
#endif // def GNASH_USE_GC

} // end of gnash namespace

