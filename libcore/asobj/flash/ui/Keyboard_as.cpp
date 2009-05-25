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

#include "ui/Keyboard_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
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
    void attachKeyboardStaticInterface(as_object& o);
    as_object* getKeyboardInterface();

}

// extern (used by Global.cpp)
void keyboard_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&keyboard_ctor, getKeyboardInterface());
        attachKeyboardStaticInterface(*cl);
    }

    // Register _global.Keyboard
    global.init_member("Keyboard", cl.get());
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

void
attachKeyboardStaticInterface(as_object& o)
{

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

