// Event_as.cpp:  ActionScript "Event" class, for Gnash.
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

#include "events/Event_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
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

class Event_as : public as_object
{

public:

    Event_as()
        :
        as_object(getEventInterface())
    {}
};

// extern (used by Global.cpp)
void event_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&event_ctor, getEventInterface());
        attachEventStaticInterface(*cl);
    }

    // Register _global.Event
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("formatToString", gl->createFunction(event_formatToString));
    o.init_member("isDefaultPrevented", gl->createFunction(event_isDefaultPrevented));
    o.init_member("preventDefault", gl->createFunction(event_preventDefault));
    o.init_member("stopImmediatePropagation", gl->createFunction(event_stopImmediatePropagation));
    o.init_member("stopPropagation", gl->createFunction(event_stopPropagation));
    o.init_member("toString", gl->createFunction(event_toString));
    o.init_member("ACTIVATE", gl->createFunction(event_ACTIVATE));
    o.init_member("ADDED", gl->createFunction(event_ADDED));
    o.init_member("ADDED_TO_STAGE", gl->createFunction(event_ADDED_TO_STAGE));
    o.init_member("CANCEL", gl->createFunction(event_CANCEL));
    o.init_member("CHANGE", gl->createFunction(event_CHANGE));
    o.init_member("CLOSE", gl->createFunction(event_CLOSE));
    o.init_member("COMPLETE", gl->createFunction(event_COMPLETE));
    o.init_member("CONNECT", gl->createFunction(event_CONNECT));
    o.init_member("DEACTIVATE", gl->createFunction(event_DEACTIVATE));
    o.init_member("DISPLAYING", gl->createFunction(event_DISPLAYING));
    o.init_member("ENTER_FRAME", gl->createFunction(event_ENTER_FRAME));
    o.init_member("FULLSCREEN", gl->createFunction(event_FULLSCREEN));
    o.init_member("ID3", gl->createFunction(event_ID3));
    o.init_member("INIT", gl->createFunction(event_INIT));
    o.init_member("MOUSE_LEAVE", gl->createFunction(event_MOUSE_LEAVE));
    o.init_member("OPEN", gl->createFunction(event_OPEN));
    o.init_member("REMOVED", gl->createFunction(event_REMOVED));
    o.init_member("REMOVED_FROM_STAGE", gl->createFunction(event_REMOVED_FROM_STAGE));
    o.init_member("RENDER", gl->createFunction(event_RENDER));
    o.init_member("RESIZE", gl->createFunction(event_RESIZE));
    o.init_member("SCROLL", gl->createFunction(event_SCROLL));
    o.init_member("SELECT", gl->createFunction(event_SELECT));
    o.init_member("SOUND_COMPLETE", gl->createFunction(event_SOUND_COMPLETE));
    o.init_member("TAB_CHILDREN_CHANGE", gl->createFunction(event_TAB_CHILDREN_CHANGE));
    o.init_member("TAB_ENABLED_CHANGE", gl->createFunction(event_TAB_ENABLED_CHANGE));
    o.init_member("TAB_INDEX_CHANGE", gl->createFunction(event_TAB_INDEX_CHANGE));
    o.init_member("UNLOAD", gl->createFunction(event_UNLOAD));
}

void
attachEventStaticInterface(as_object& /*o*/)
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
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_isDefaultPrevented(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_preventDefault(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_stopImmediatePropagation(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_stopPropagation(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_toString(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ACTIVATE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ADDED(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ADDED_TO_STAGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CANCEL(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CLOSE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_COMPLETE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_CONNECT(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_DEACTIVATE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_DISPLAYING(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ENTER_FRAME(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_FULLSCREEN(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ID3(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_INIT(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_MOUSE_LEAVE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_OPEN(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_REMOVED(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_REMOVED_FROM_STAGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_RENDER(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_RESIZE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_SCROLL(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_SELECT(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_SOUND_COMPLETE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_TAB_CHILDREN_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_TAB_ENABLED_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_TAB_INDEX_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_UNLOAD(const fn_call& fn)
{
    boost::intrusive_ptr<Event_as> ptr =
        ensureType<Event_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
event_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new Event_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

