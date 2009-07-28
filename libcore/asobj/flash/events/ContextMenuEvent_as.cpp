// ContextMenuEvent_as.cpp:  ActionScript "ContextMenuEvent" class, for Gnash.
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

#include "events/ContextMenuEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value contextmenuevent_toString(const fn_call& fn);
    as_value contextmenuevent_MENU_ITEM_SELECT(const fn_call& fn);
    as_value contextmenuevent_MENU_SELECT(const fn_call& fn);
    as_value contextmenuevent_ctor(const fn_call& fn);
    void attachContextMenuEventInterface(as_object& o);
    void attachContextMenuEventStaticInterface(as_object& o);
    as_object* getContextMenuEventInterface();

}

class ContextMenuEvent_as : public as_object
{

public:

    ContextMenuEvent_as()
        :
        as_object(getContextMenuEventInterface())
    {}
};

// extern (used by Global.cpp)
void contextmenuevent_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&contextmenuevent_ctor, getContextMenuEventInterface());
        attachContextMenuEventStaticInterface(*cl);
    }

    // Register _global.ContextMenuEvent
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachContextMenuEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(contextmenuevent_toString));
    o.init_member("MENU_ITEM_SELECT", gl->createFunction(contextmenuevent_MENU_ITEM_SELECT));
    o.init_member("MENU_SELECT", gl->createFunction(contextmenuevent_MENU_SELECT));
}

void
attachContextMenuEventStaticInterface(as_object& /*o*/)
{

}

as_object*
getContextMenuEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachContextMenuEventInterface(*o);
    }
    return o.get();
}

as_value
contextmenuevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<ContextMenuEvent_as> ptr =
        ensureType<ContextMenuEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
contextmenuevent_MENU_ITEM_SELECT(const fn_call& fn)
{
    boost::intrusive_ptr<ContextMenuEvent_as> ptr =
        ensureType<ContextMenuEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
contextmenuevent_MENU_SELECT(const fn_call& fn)
{
    boost::intrusive_ptr<ContextMenuEvent_as> ptr =
        ensureType<ContextMenuEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
contextmenuevent_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new ContextMenuEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

