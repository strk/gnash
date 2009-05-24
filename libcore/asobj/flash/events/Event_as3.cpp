// Event_as3.cpp:  ActionScript "Event" class, for Gnash.
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

#include "events/Event_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value event_formatToString(const fn_call& fn);
    as_value event_isDefaultPrevented(const fn_call& fn);
    as_value event_preventDefault(const fn_call& fn);
    as_value event_stopImmediatePropagation(const fn_call& fn);
    as_value event_stopPropagation(const fn_call& fn);
    as_value event_toString(const fn_call& fn);
    as_value event_ACTIVATE(const fn_call& fn);
    as_value event_ADDED(const fn_call& fn);
    as_value event_ADDED_TO_STAGE(const fn_call& fn);
    as_value event_CANCEL(const fn_call& fn);
    as_value event_CHANGE(const fn_call& fn);
    as_value event_CLOSE(const fn_call& fn);
    as_value event_COMPLETE(const fn_call& fn);
    as_value event_CONNECT(const fn_call& fn);
    as_value event_DEACTIVATE(const fn_call& fn);
    as_value event_DISPLAYING(const fn_call& fn);
    as_value event_ENTER_FRAME(const fn_call& fn);
    as_value event_FULLSCREEN(const fn_call& fn);
    as_value event_ID3(const fn_call& fn);
    as_value event_INIT(const fn_call& fn);
    as_value event_MOUSE_LEAVE(const fn_call& fn);
    as_value event_OPEN(const fn_call& fn);
    as_value event_REMOVED(const fn_call& fn);
    as_value event_REMOVED_FROM_STAGE(const fn_call& fn);
    as_value event_RENDER(const fn_call& fn);
    as_value event_RESIZE(const fn_call& fn);
    as_value event_SCROLL(const fn_call& fn);
    as_value event_SELECT(const fn_call& fn);
    as_value event_SOUND_COMPLETE(const fn_call& fn);
    as_value event_TAB_CHILDREN_CHANGE(const fn_call& fn);
    as_value event_TAB_ENABLED_CHANGE(const fn_call& fn);
    as_value event_TAB_INDEX_CHANGE(const fn_call& fn);
    as_value event_UNLOAD(const fn_call& fn);
    as_value event_ctor(const fn_call& fn);
    void attachEventInterface(as_object& o);
    void attachEventStaticInterface(as_object& o);
    as_object* getEventInterface();

}

// extern (used by Global.cpp)
void event_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&event_ctor, getEventInterface());
        attachEventStaticInterface(*cl);
    }

    // Register _global.Event
    global.init_member("Event", cl.get());
}

namespace {

void
attachEventInterface(as_object& o)
{
    o.init_member("formatToString", new builtin_function(event_formatToString));
    o.init_member("isDefaultPrevented", new builtin_function(event_isDefaultPrevented));
    o.init_member("preventDefault", new builtin_function(event_preventDefault));
    o.init_member("stopImmediatePropagation", new builtin_function(event_stopImmediatePropagation));
    o.init_member("stopPropagation", new builtin_function(event_stopPropagation));
    o.init_member("toString", new builtin_function(event_toString));
    o.init_member("ACTIVATE", new builtin_function(event_ACTIVATE));
    o.init_member("ADDED", new builtin_function(event_ADDED));
    o.init_member("ADDED_TO_STAGE", new builtin_function(event_ADDED_TO_STAGE));
    o.init_member("CANCEL", new builtin_function(event_CANCEL));
    o.init_member("CHANGE", new builtin_function(event_CHANGE));
    o.init_member("CLOSE", new builtin_function(event_CLOSE));
    o.init_member("COMPLETE", new builtin_function(event_COMPLETE));
    o.init_member("CONNECT", new builtin_function(event_CONNECT));
    o.init_member("DEACTIVATE", new builtin_function(event_DEACTIVATE));
    o.init_member("DISPLAYING", new builtin_function(event_DISPLAYING));
    o.init_member("ENTER_FRAME", new builtin_function(event_ENTER_FRAME));
    o.init_member("FULLSCREEN", new builtin_function(event_FULLSCREEN));
    o.init_member("ID3", new builtin_function(event_ID3));
    o.init_member("INIT", new builtin_function(event_INIT));
    o.init_member("MOUSE_LEAVE", new builtin_function(event_MOUSE_LEAVE));
    o.init_member("OPEN", new builtin_function(event_OPEN));
    o.init_member("REMOVED", new builtin_function(event_REMOVED));
    o.init_member("REMOVED_FROM_STAGE", new builtin_function(event_REMOVED_FROM_STAGE));
    o.init_member("RENDER", new builtin_function(event_RENDER));
    o.init_member("RESIZE", new builtin_function(event_RESIZE));
    o.init_member("SCROLL", new builtin_function(event_SCROLL));
    o.init_member("SELECT", new builtin_function(event_SELECT));
    o.init_member("SOUND_COMPLETE", new builtin_function(event_SOUND_COMPLETE));
    o.init_member("TAB_CHILDREN_CHANGE", new builtin_function(event_TAB_CHILDREN_CHANGE));
    o.init_member("TAB_ENABLED_CHANGE", new builtin_function(event_TAB_ENABLED_CHANGE));
    o.init_member("TAB_INDEX_CHANGE", new builtin_function(event_TAB_INDEX_CHANGE));
    o.init_member("UNLOAD", new builtin_function(event_UNLOAD));
}

void
attachEventStaticInterface(as_object& o)
{

}

as_object*
getEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachEventInterface(*o);
    }
    return o.get();
}

as_value
event_formatToString(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_isDefaultPrevented(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_preventDefault(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_stopImmediatePropagation(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_stopPropagation(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_toString(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ACTIVATE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ADDED(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ADDED_TO_STAGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CANCEL(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CLOSE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_COMPLETE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CONNECT(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_DEACTIVATE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_DISPLAYING(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ENTER_FRAME(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_FULLSCREEN(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ID3(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_INIT(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_MOUSE_LEAVE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_OPEN(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_REMOVED(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_REMOVED_FROM_STAGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_RENDER(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_RESIZE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_SCROLL(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_SELECT(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_SOUND_COMPLETE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_TAB_CHILDREN_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_TAB_ENABLED_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_TAB_INDEX_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_UNLOAD(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as3> ptr =
        ensureType<Event_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Event_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

