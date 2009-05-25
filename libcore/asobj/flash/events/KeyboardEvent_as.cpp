// KeyboardEvent_as.cpp:  ActionScript "KeyboardEvent" class, for Gnash.
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

#include "events/KeyboardEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value keyboardevent_toString(const fn_call& fn);
    as_value keyboardevent_updateAfterEvent(const fn_call& fn);
    as_value keyboardevent_KEY_DOWN(const fn_call& fn);
    as_value keyboardevent_KEY_UP(const fn_call& fn);
    as_value keyboardevent_ctor(const fn_call& fn);
    void attachKeyboardEventInterface(as_object& o);
    void attachKeyboardEventStaticInterface(as_object& o);
    as_object* getKeyboardEventInterface();

}

// extern (used by Global.cpp)
void keyboardevent_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&keyboardevent_ctor, getKeyboardEventInterface());
        attachKeyboardEventStaticInterface(*cl);
    }

    // Register _global.KeyboardEvent
    global.init_member("KeyboardEvent", cl.get());
}

namespace {

void
attachKeyboardEventInterface(as_object& o)
{
    o.init_member("toString", new builtin_function(keyboardevent_toString));
    o.init_member("updateAfterEvent", new builtin_function(keyboardevent_updateAfterEvent));
    o.init_member("KEY_DOWN", new builtin_function(keyboardevent_KEY_DOWN));
    o.init_member("KEY_UP", new builtin_function(keyboardevent_KEY_UP));
}

void
attachKeyboardEventStaticInterface(as_object& o)
{

}

as_object*
getKeyboardEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachKeyboardEventInterface(*o);
    }
    return o.get();
}

as_value
keyboardevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<KeyboardEvent_as> ptr =
        ensureType<KeyboardEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_updateAfterEvent(const fn_call& fn)
{
    boost::intrusive_ptr<KeyboardEvent_as> ptr =
        ensureType<KeyboardEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_KEY_DOWN(const fn_call& fn)
{
    boost::intrusive_ptr<KeyboardEvent_as> ptr =
        ensureType<KeyboardEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_KEY_UP(const fn_call& fn)
{
    boost::intrusive_ptr<KeyboardEvent_as> ptr =
        ensureType<KeyboardEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new KeyboardEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

