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

// Forward declarations
namespace {
    as_value keyboard_BACKSPACE(const fn_call& fn);
    as_value keyboard_C(const fn_call& fn);
    as_value keyboard_CAPS_LOCK(const fn_call& fn);
    as_value keyboard_CONTROL(const fn_call& fn);
    as_value keyboard_DELETE(const fn_call& fn);
    as_value keyboard_DOWN(const fn_call& fn);
    as_value keyboard_END(const fn_call& fn);
    as_value keyboard_ENTER(const fn_call& fn);
    as_value keyboard_ESCAPE(const fn_call& fn);
    as_value keyboard_F1(const fn_call& fn);
    as_value keyboard_F10(const fn_call& fn);
    as_value keyboard_F11(const fn_call& fn);
    as_value keyboard_F12(const fn_call& fn);
    as_value keyboard_F13(const fn_call& fn);
    as_value keyboard_F14(const fn_call& fn);
    as_value keyboard_F15(const fn_call& fn);
    as_value keyboard_F2(const fn_call& fn);
    as_value keyboard_F3(const fn_call& fn);
    as_value keyboard_F4(const fn_call& fn);
    as_value keyboard_F5(const fn_call& fn);
    as_value keyboard_F6(const fn_call& fn);
    as_value keyboard_F7(const fn_call& fn);
    as_value keyboard_F8(const fn_call& fn);
    as_value keyboard_F9(const fn_call& fn);
    as_value keyboard_G(const fn_call& fn);
    as_value keyboard_HOME(const fn_call& fn);
    as_value keyboard_INSERT(const fn_call& fn);
    as_value keyboard_LEFT(const fn_call& fn);
    as_value keyboard_NUMPAD_0(const fn_call& fn);
    as_value keyboard_NUMPAD_1(const fn_call& fn);
    as_value keyboard_NUMPAD_2(const fn_call& fn);
    as_value keyboard_NUMPAD_3(const fn_call& fn);
    as_value keyboard_NUMPAD_4(const fn_call& fn);
    as_value keyboard_NUMPAD_5(const fn_call& fn);
    as_value keyboard_NUMPAD_6(const fn_call& fn);
    as_value keyboard_NUMPAD_7(const fn_call& fn);
    as_value keyboard_NUMPAD_8(const fn_call& fn);
    as_value keyboard_NUMPAD_9(const fn_call& fn);
    as_value keyboard_NUMPAD_ADD(const fn_call& fn);
    as_value keyboard_NUMPAD_DECIMAL(const fn_call& fn);
    as_value keyboard_NUMPAD_DIVIDE(const fn_call& fn);
    as_value keyboard_NUMPAD_ENTER(const fn_call& fn);
    as_value keyboard_NUMPAD_MULTIPLY(const fn_call& fn);
    as_value keyboard_NUMPAD_SUBTRACT(const fn_call& fn);
    as_value keyboard_PAGE_DOWN(const fn_call& fn);
    as_value keyboard_PAGE_UP(const fn_call& fn);
    as_value keyboard_RIGHT(const fn_call& fn);
    as_value keyboard_SHIFT(const fn_call& fn);
    as_value keyboard_SPACE(const fn_call& fn);
    as_value keyboard_TAB(const fn_call& fn);
    as_value keyboard_UP(const fn_call& fn);
    as_value keyboard_ctor(const fn_call& fn);
    void attachKeyboardInterface(as_object& o);
    as_object* getKeyboardInterface();
}

Keyboard_as::Keyboard_as()
    :
    as_object(getObjectInterface()),
    _unreleasedKeys(0),
    _lastKeyEvent(0)
{
    // Key is a broadcaster only in SWF6 and up (correct?)
    int swfversion = _vm.getSWFVersion();
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
void Keyboard_as::init(as_object& global)
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

    VM& vm = global.getVM();

    vm.registerNative(key_get_ascii, 800, 0);
    key_obj->init_member("getAscii", vm.getNative(800, 0), flags);

    vm.registerNative(key_get_code, 800, 1);
    key_obj->init_member("getCode", vm.getNative(800, 1), flags);

    vm.registerNative(key_is_down, 800, 2);
    key_obj->init_member("isDown", vm.getNative(800, 2), flags);

    vm.registerNative(key_is_toggled, 800, 3);
    key_obj->init_member("isToggled", vm.getNative(800, 3), flags);

    key_obj->init_member("isAccessible", 
            new builtin_function(key_is_accessible), flags);

    global.init_member("Key", key_obj);
}

namespace {

void
attachKeyboardInterface(as_object& o)
{
    o.init_member("BACKSPACE", new builtin_function(keyboard_BACKSPACE));
    o.init_member("C", new builtin_function(keyboard_C));
    o.init_member("CAPS_LOCK", new builtin_function(keyboard_CAPS_LOCK));
    o.init_member("CONTROL", new builtin_function(keyboard_CONTROL));
    o.init_member("DELETE", new builtin_function(keyboard_DELETE));
    o.init_member("DOWN", new builtin_function(keyboard_DOWN));
    o.init_member("END", new builtin_function(keyboard_END));
    o.init_member("ENTER", new builtin_function(keyboard_ENTER));
    o.init_member("ESCAPE", new builtin_function(keyboard_ESCAPE));
    o.init_member("F1", new builtin_function(keyboard_F1));
    o.init_member("F10", new builtin_function(keyboard_F10));
    o.init_member("F11", new builtin_function(keyboard_F11));
    o.init_member("F12", new builtin_function(keyboard_F12));
    o.init_member("F13", new builtin_function(keyboard_F13));
    o.init_member("F14", new builtin_function(keyboard_F14));
    o.init_member("F15", new builtin_function(keyboard_F15));
    o.init_member("F2", new builtin_function(keyboard_F2));
    o.init_member("F3", new builtin_function(keyboard_F3));
    o.init_member("F4", new builtin_function(keyboard_F4));
    o.init_member("F5", new builtin_function(keyboard_F5));
    o.init_member("F6", new builtin_function(keyboard_F6));
    o.init_member("F7", new builtin_function(keyboard_F7));
    o.init_member("F8", new builtin_function(keyboard_F8));
    o.init_member("F9", new builtin_function(keyboard_F9));
    o.init_member("G", new builtin_function(keyboard_G));
    o.init_member("HOME", new builtin_function(keyboard_HOME));
    o.init_member("INSERT", new builtin_function(keyboard_INSERT));
    o.init_member("LEFT", new builtin_function(keyboard_LEFT));
    o.init_member("NUMPAD_0", new builtin_function(keyboard_NUMPAD_0));
    o.init_member("NUMPAD_1", new builtin_function(keyboard_NUMPAD_1));
    o.init_member("NUMPAD_2", new builtin_function(keyboard_NUMPAD_2));
    o.init_member("NUMPAD_3", new builtin_function(keyboard_NUMPAD_3));
    o.init_member("NUMPAD_4", new builtin_function(keyboard_NUMPAD_4));
    o.init_member("NUMPAD_5", new builtin_function(keyboard_NUMPAD_5));
    o.init_member("NUMPAD_6", new builtin_function(keyboard_NUMPAD_6));
    o.init_member("NUMPAD_7", new builtin_function(keyboard_NUMPAD_7));
    o.init_member("NUMPAD_8", new builtin_function(keyboard_NUMPAD_8));
    o.init_member("NUMPAD_9", new builtin_function(keyboard_NUMPAD_9));
    o.init_member("NUMPAD_ADD", new builtin_function(keyboard_NUMPAD_ADD));
    o.init_member("NUMPAD_DECIMAL", new builtin_function(keyboard_NUMPAD_DECIMAL));
    o.init_member("NUMPAD_DIVIDE", new builtin_function(keyboard_NUMPAD_DIVIDE));
    o.init_member("NUMPAD_ENTER", new builtin_function(keyboard_NUMPAD_ENTER));
    o.init_member("NUMPAD_MULTIPLY", new builtin_function(keyboard_NUMPAD_MULTIPLY));
    o.init_member("NUMPAD_SUBTRACT", new builtin_function(keyboard_NUMPAD_SUBTRACT));
    o.init_member("PAGE_DOWN", new builtin_function(keyboard_PAGE_DOWN));
    o.init_member("PAGE_UP", new builtin_function(keyboard_PAGE_UP));
    o.init_member("RIGHT", new builtin_function(keyboard_RIGHT));
    o.init_member("SHIFT", new builtin_function(keyboard_SHIFT));
    o.init_member("SPACE", new builtin_function(keyboard_SPACE));
    o.init_member("TAB", new builtin_function(keyboard_TAB));
    o.init_member("UP", new builtin_function(keyboard_UP));
}


as_object*
getKeyboardInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachKeyboardInterface(*o);
    }
    return o.get();
}

as_value
keyboard_BACKSPACE(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_C(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_CAPS_LOCK(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_CONTROL(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_DELETE(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_DOWN(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_END(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_ENTER(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_ESCAPE(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F1(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F10(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F11(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F12(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F13(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F14(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F15(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F2(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F3(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F4(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F5(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F6(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F7(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F8(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_F9(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_G(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_HOME(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_INSERT(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_LEFT(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_0(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_1(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_2(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_3(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_4(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_5(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_6(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_7(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_8(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_9(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_ADD(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_DECIMAL(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_DIVIDE(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_ENTER(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_MULTIPLY(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_NUMPAD_SUBTRACT(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_PAGE_DOWN(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_PAGE_UP(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_RIGHT(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_SHIFT(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_SPACE(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_TAB(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_UP(const fn_call& fn)
{
    boost::intrusive_ptr<Keyboard_as> ptr =
        ensureType<Keyboard_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboard_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Keyboard_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

