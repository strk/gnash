// Key_as.cpp:  ActionScript "Keyboard" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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


#include "Key_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "movie_root.h"
#include "VM.h" // for registerNative
#include "NativeFunction.h" 
#include "AsBroadcaster.h" //for initializing self as a broadcaster
#include "namedStrings.h"
#include "GnashKey.h"
#include "GnashException.h" // for ActionException

#include <bitset>

namespace gnash {

as_value
key_is_accessible(const fn_call& /*fn*/)
{
    log_unimpl(_("Key.isAccessible"));
    return as_value();
}


/// Return the ascii number of the last key pressed.
as_value   
key_get_ascii(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);
    const key::code code = mr.lastKeyEvent();
    return as_value(gnash::key::codeMap[code][key::ASCII]);
}

/// Returns the keycode of the last key pressed.
as_value   
key_get_code(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);
    const key::code code = mr.lastKeyEvent();
    return as_value(key::codeMap[code][key::KEY]);
}

/// Return true if the specified (first arg keycode) key is pressed.
as_value   
key_is_down(const fn_call& fn)
{

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Key.isDown needs one argument (the key code)"));
        );
        return as_value();
    }

    const int keycode = toInt(fn.arg(0), getVM(fn));
    if (keycode < 0 || keycode >= key::KEYCOUNT) {
        // AS coding error !
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Key.isKeyDown(%d): keycode out of range"), keycode);
        );
        return as_value(false);
    }

    movie_root& mr = getRoot(fn);
    const movie_root::Keys& keys = mr.unreleasedKeys();

    return as_value(keys.test(keycode));
}

/// \brief
/// Given the keycode of NUM_LOCK or CAPSLOCK, returns true if
/// the associated state is on.
///
as_value   
key_is_toggled(const fn_call& /* fn */)
{
    log_unimpl(_("Key.isToggled"));
    // @@ TODO
    return as_value(false);
}

void
registerKeyNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(key_get_ascii, 800, 0);
    vm.registerNative(key_get_code, 800, 1);
    vm.registerNative(key_is_down, 800, 2);
    vm.registerNative(key_is_toggled, 800, 3);
}

void
attachKeyInterface(as_object& o)
{
    const int flags = PropFlags::readOnly |
                      PropFlags::dontDelete |
                      PropFlags::dontEnum;

    // constants
#define KEY_CONST(k) o.init_member(#k, key::codeMap[key::k][key::KEY], flags)
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

    VM& vm = getVM(o);
    Global_as& gl = getGlobal(o);

    o.init_member("getAscii", vm.getNative(800, 0), flags);
    o.init_member("getCode", vm.getNative(800, 1), flags);
    o.init_member("isDown", vm.getNative(800, 2), flags);
    o.init_member("isToggled", vm.getNative(800, 3), flags);
    o.init_member("isAccessible", 
            gl.createFunction(key_is_accessible), flags);
}

// extern (used by Global.cpp)
void
key_class_init(as_object& where, const ObjectURI& uri)
{
    as_object* key = registerBuiltinObject(where, attachKeyInterface, uri);

    /// Handles addListener, removeListener, and _listeners.
    AsBroadcaster::initialize(*key);

    // All properties are protected using ASSetPropFlags.
    Global_as& gl = getGlobal(where);
    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, key, null, 7);
}

} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

