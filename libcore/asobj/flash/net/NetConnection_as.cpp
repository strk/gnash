// NetConnection_as.cpp:  ActionScript "NetConnection" class, for Gnash.
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

#include "net/NetConnection_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value netconnection_call(const fn_call& fn);
    as_value netconnection_close(const fn_call& fn);
    as_value netconnection_connect(const fn_call& fn);
    as_value netconnection_asyncError(const fn_call& fn);
    as_value netconnection_ioError(const fn_call& fn);
    as_value netconnection_netStatus(const fn_call& fn);
    as_value netconnection_securityError(const fn_call& fn);
    as_value netconnection_ctor(const fn_call& fn);
    void attachNetConnectionInterface(as_object& o);
    void attachNetConnectionStaticInterface(as_object& o);
    as_object* getNetConnectionInterface();

}

class NetConnection_as : public as_object
{

public:

    NetConnection_as()
        :
        as_object(getNetConnectionInterface())
    {}
};

// extern (used by Global.cpp)
void netconnection_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&netconnection_ctor, getNetConnectionInterface());
        attachNetConnectionStaticInterface(*cl);
    }

    // Register _global.NetConnection
    global.init_member("NetConnection", cl.get());
}

namespace {

void
attachNetConnectionInterface(as_object& o)
{
    o.init_member("call", new builtin_function(netconnection_call));
    o.init_member("close", new builtin_function(netconnection_close));
    o.init_member("connect", new builtin_function(netconnection_connect));
    o.init_member("asyncError", new builtin_function(netconnection_asyncError));
    o.init_member("ioError", new builtin_function(netconnection_ioError));
    o.init_member("netStatus", new builtin_function(netconnection_netStatus));
    o.init_member("securityError", new builtin_function(netconnection_securityError));
}

void
attachNetConnectionStaticInterface(as_object& o)
{

}

as_object*
getNetConnectionInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachNetConnectionInterface(*o);
    }
    return o.get();
}

as_value
netconnection_call(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_close(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_connect(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_asyncError(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_netStatus(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netconnection_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new NetConnection_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

