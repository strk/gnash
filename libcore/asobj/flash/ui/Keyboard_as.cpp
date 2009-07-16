// Keyboard_as.cpp:  ActionScript "Keyboard" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" //GNASH_USE_GC
#include "ui/Keyboard_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "movie_root.h"
#include "action.h" // for call_method
#include "VM.h" // for registerNative
#include "builtin_function.h" // need builtin_function
#include "Object.h"
#include "AsBroadcaster.h" //for initializing self as a broadcaster
#include "namedStrings.h"
#include "GnashKey.h"
#include "GnashException.h" // for ActionException

namespace gnash {

Keyboard_as::Keyboard_as()
    :
    as_object(getObjectInterface()),
    _unreleasedKeys(0),
    _lastKeyEvent(0)
{
    // Key is a broadcaster only in SWF6 and up (correct?)
    int swfversion = getSWFVersion(*this);
    if ( swfversion > 5 )
    {
        AsBroadcaster::initialize(*this);
    }
}

bool
Keyboard_as::is_key_down(int keycode)
{
    // caller must check this
    assert (keycode >= 0 && keycode < key::KEYCOUNT);

    if (_unreleasedKeys.test(keycode)) return true;
    return false;
}

void
Keyboard_as::set_key_down(key::code code)
{
    if (code >= key::KEYCOUNT)
    {
        // programmatic error, as only movie_root calls us
        log_error("Key_as::set_key_down(%d): code out of range", code);
        return;
    }

    // This is used for getAscii() of the last key event, so we store
    // the unique gnash::key::code.
    _lastKeyEvent = code;

    // Key.isDown() only cares about flash keycode, not DisplayObject, so
    // we lookup keycode to add to _unreleasedKeys.   
    size_t keycode = key::codeMap[code][key::KEY];

#ifdef GNASH_DEBUG_KEYEVENTS
    log_debug("Key_as::set_key_down(%d): setting unreleased keycode %d (from code %d)", keycode, code);
#endif
    _unreleasedKeys.set(keycode, 1);
}

void
Keyboard_as::set_key_up(key::code code)
{
    if (code >= key::KEYCOUNT)
    {
        // programmatic error, as only movie_root calls us
        log_error("Key_as::set_key_up(%d): code out of range", code);
        return;
    }

    // This is used for getAscii() of the last key event, so we store
    // the unique gnash::key::code.    
    _lastKeyEvent = code;

    // Key.isDown() only cares about flash keycode, not DisplayObject, so
    // we lookup keycode to add to _unreleasedKeys.
    size_t keycode = key::codeMap[code][key::KEY];

#ifdef GNASH_DEBUG_KEYEVENTS
    log_debug("Key_as::set_key_down(%d): setting released keycode %d (from code %d)", keycode, code);
#endif
    _unreleasedKeys.set(keycode, 0);
}


void 
Keyboard_as::notify_listeners(const event_id& ev)
{  
    // There is no user defined "onKeyPress" event handler
    if((ev.id() != event_id::KEY_DOWN) &&
            (ev.id() != event_id::KEY_UP)) return;

#ifdef GNASH_DEBUG_KEYEVENTS
    log_debug("notify_listeners calling broadcastMessage with arg %s", ev);
#endif
    callMethod(NSV::PROP_BROADCAST_MESSAGE, ev.functionName());
}

int
Keyboard_as::get_last_key() const
{
    return _lastKeyEvent;
}

static as_value
key_is_accessible(const fn_call& fn)
{

    boost::intrusive_ptr<Keyboard_as> ko = 
        ensureType<Keyboard_as>(fn.this_ptr);

    log_unimpl("Key.isAccessible");
    return as_value();
}


/// Return the ascii number of the last key pressed.
static as_value   
key_get_ascii(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ko = 
        ensureType<Keyboard_as>(fn.this_ptr);

    int code = ko->get_last_key();

    return as_value(gnash::key::codeMap[code][key::ASCII]);
}

/// Returns the keycode of the last key pressed.
static as_value   
key_get_code(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ko = 
        ensureType<Keyboard_as>(fn.this_ptr);

    int code = ko->get_last_key();

    return as_value(key::codeMap[code][key::KEY]);
}

/// Return true if the specified (first arg keycode) key is pressed.
static as_value   
key_is_down(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ko = 
        ensureType<Keyboard_as>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Key.isDown needs one argument (the key code)"));
        );
        return as_value();
    }

    int keycode = fn.arg(0).to_int();
    if (keycode < 0 || keycode >= key::KEYCOUNT)
    {
        // AS coding error !
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("Key.isKeyDown(%d): keycode out of range", keycode);
        );
        return as_value(false);
    }

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

#ifdef GNASH_USE_GC
void
Keyboard_as::markReachableResources() const
{
    markAsObjectReachable();
    for (Listeners::const_iterator i=_listeners.begin(), e=_listeners.end();
                i != e; ++i)
    {
        (*i)->setReachable();
    }
}
#endif // def GNASH_USE_GC

// extern (used by Global.cpp)
void Keyboard_as::init(as_object& global, const ObjectURI& uri)
{

    //  GNASH_REPORT_FUNCTION;
    //

    // Create built-in key object.
    // NOTE: _global.Key *is* an object, not a constructor
    as_object*  key_obj = new Keyboard_as;

    const int flags = as_prop_flags::readOnly |
                      as_prop_flags::dontDelete |
                      as_prop_flags::dontEnum;

    // constants
#define KEY_CONST(k) key_obj->init_member(#k, key::codeMap[key::k][key::KEY], flags)
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
    KEY_CONST(ALT);

    // methods

    VM& vm = getVM(global);
    Global_as* gl = getGlobal(global);

    vm.registerNative(key_get_ascii, 800, 0);
    key_obj->init_member("getAscii", vm.getNative(800, 0), flags);

    vm.registerNative(key_get_code, 800, 1);
    key_obj->init_member("getCode", vm.getNative(800, 1), flags);

    vm.registerNative(key_is_down, 800, 2);
    key_obj->init_member("isDown", vm.getNative(800, 2), flags);

    vm.registerNative(key_is_toggled, 800, 3);
    key_obj->init_member("isToggled", vm.getNative(800, 3), flags);

    key_obj->init_member("isAccessible", 
            gl->createFunction(key_is_accessible), flags);

    global.init_member("Key", key_obj);
}

} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

