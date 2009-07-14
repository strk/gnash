// IEventDispatcher_as.cpp:  ActionScript "IEventDispatcher" class, for Gnash.
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

#include "events/IEventDispatcher_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value ieventdispatcher_dispatchEvent(const fn_call& fn);
    as_value ieventdispatcher_hasEventListener(const fn_call& fn);
    as_value ieventdispatcher_removeEventListener(const fn_call& fn);
    as_value ieventdispatcher_willTrigger(const fn_call& fn);
    as_value ieventdispatcher_ctor(const fn_call& fn);
    void attachIEventDispatcherInterface(as_object& o);
    void attachIEventDispatcherStaticInterface(as_object& o);
    as_object* getIEventDispatcherInterface();

}

class IEventDispatcher_as : public as_object
{

public:

    IEventDispatcher_as()
        :
        as_object(getIEventDispatcherInterface())
    {}
};

// extern (used by Global.cpp)
void ieventdispatcher_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&ieventdispatcher_ctor, getIEventDispatcherInterface());
        attachIEventDispatcherStaticInterface(*cl);
    }

    // Register _global.IEventDispatcher
    global.init_member("IEventDispatcher", cl.get());
}

namespace {

void
attachIEventDispatcherInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("dispatchEvent", gl->createFunction(ieventdispatcher_dispatchEvent));
    o.init_member("hasEventListener", gl->createFunction(ieventdispatcher_hasEventListener));
    o.init_member("removeEventListener", gl->createFunction(ieventdispatcher_removeEventListener));
    o.init_member("willTrigger", gl->createFunction(ieventdispatcher_willTrigger));
}

void
attachIEventDispatcherStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);

}

as_object*
getIEventDispatcherInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachIEventDispatcherInterface(*o);
    }
    return o.get();
}

as_value
ieventdispatcher_dispatchEvent(const fn_call& fn)
{
    boost::intrusive_ptr<IEventDispatcher_as> ptr =
        ensureType<IEventDispatcher_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
ieventdispatcher_hasEventListener(const fn_call& fn)
{
    boost::intrusive_ptr<IEventDispatcher_as> ptr =
        ensureType<IEventDispatcher_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
ieventdispatcher_removeEventListener(const fn_call& fn)
{
    boost::intrusive_ptr<IEventDispatcher_as> ptr =
        ensureType<IEventDispatcher_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
ieventdispatcher_willTrigger(const fn_call& fn)
{
    boost::intrusive_ptr<IEventDispatcher_as> ptr =
        ensureType<IEventDispatcher_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
ieventdispatcher_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new IEventDispatcher_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

