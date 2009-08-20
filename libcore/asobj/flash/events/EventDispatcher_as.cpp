// EventDispatcher_as.cpp:  ActionScript "EventDispatcher" class, for Gnash.
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

#include "events/EventDispatcher_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value eventdispatcher_dispatchEvent(const fn_call& fn);
    as_value eventdispatcher_hasEventListener(const fn_call& fn);
    as_value eventdispatcher_removeEventListener(const fn_call& fn);
    as_value eventdispatcher_willTrigger(const fn_call& fn);
    as_value eventdispatcher_activate(const fn_call& fn);
    as_value eventdispatcher_deactivate(const fn_call& fn);
    as_value eventdispatcher_ctor(const fn_call& fn);
    void attachEventDispatcherInterface(as_object& o);
    void attachEventDispatcherStaticInterface(as_object& o);

}

// extern (used by Global.cpp)
void
eventdispatcher_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, eventdispatcher_ctor,
            attachEventDispatcherInterface,
            attachEventDispatcherStaticInterface, uri);
}

namespace {

void
attachEventDispatcherInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("dispatchEvent", gl->createFunction(eventdispatcher_dispatchEvent));
    o.init_member("hasEventListener", gl->createFunction(eventdispatcher_hasEventListener));
    o.init_member("removeEventListener", gl->createFunction(eventdispatcher_removeEventListener));
    o.init_member("willTrigger", gl->createFunction(eventdispatcher_willTrigger));
    o.init_member("activate", gl->createFunction(eventdispatcher_activate));
    o.init_member("deactivate", gl->createFunction(eventdispatcher_deactivate));
}

void
attachEventDispatcherStaticInterface(as_object& /*o*/)
{

}

as_value
eventdispatcher_dispatchEvent(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
eventdispatcher_hasEventListener(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
eventdispatcher_removeEventListener(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
eventdispatcher_willTrigger(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
eventdispatcher_activate(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
eventdispatcher_deactivate(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
eventdispatcher_ctor(const fn_call& /*fn*/)
{
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

