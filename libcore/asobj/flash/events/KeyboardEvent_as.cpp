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
#include "Global_as.h"
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
}

// extern (used by Global.cpp)
void
keyboardevent_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, keyboardevent_ctor,
            attachKeyboardEventInterface,
            attachKeyboardEventStaticInterface, uri);
}

namespace {

void
attachKeyboardEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(keyboardevent_toString));
    o.init_member("updateAfterEvent", gl->createFunction(keyboardevent_updateAfterEvent));
    o.init_member("KEY_DOWN", gl->createFunction(keyboardevent_KEY_DOWN));
    o.init_member("KEY_UP", gl->createFunction(keyboardevent_KEY_UP));
}

void
attachKeyboardEventStaticInterface(as_object& /*o*/)
{
}

as_value
keyboardevent_toString(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_updateAfterEvent(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_KEY_DOWN(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_KEY_UP(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
keyboardevent_ctor(const fn_call& /*fn*/)
{
    return as_value(); 
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

