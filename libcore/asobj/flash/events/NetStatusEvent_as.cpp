// NetStatusEvent_as.cpp:  ActionScript "NetStatusEvent" class, for Gnash.
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

#include "events/NetStatusEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value netstatusevent_toString(const fn_call& fn);
    as_value netstatusevent_NET_STATUS(const fn_call& fn);
    as_value netstatusevent_ctor(const fn_call& fn);
    void attachNetStatusEventInterface(as_object& o);
    void attachNetStatusEventStaticInterface(as_object& o);
    as_object* getNetStatusEventInterface();

}

// extern (used by Global.cpp)
void netstatusevent_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&netstatusevent_ctor, getNetStatusEventInterface());
        attachNetStatusEventStaticInterface(*cl);
    }

    // Register _global.NetStatusEvent
    global.init_member("NetStatusEvent", cl.get());
}

namespace {

void
attachNetStatusEventInterface(as_object& o)
{
    o.init_member("toString", new builtin_function(netstatusevent_toString));
    o.init_member("NET_STATUS", new builtin_function(netstatusevent_NET_STATUS));
}

void
attachNetStatusEventStaticInterface(as_object& o)
{

}

as_object*
getNetStatusEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachNetStatusEventInterface(*o);
    }
    return o.get();
}

as_value
netstatusevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<NetStatusEvent_as> ptr =
        ensureType<NetStatusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstatusevent_NET_STATUS(const fn_call& fn)
{
    boost::intrusive_ptr<NetStatusEvent_as> ptr =
        ensureType<NetStatusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstatusevent_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new NetStatusEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

