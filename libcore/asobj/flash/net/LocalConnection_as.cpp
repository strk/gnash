// LocalConnection_as.cpp:  ActionScript "LocalConnection" class, for Gnash.
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

#include "net/LocalConnection_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value localconnection_allowInsecureDomain(const fn_call& fn);
    as_value localconnection_close(const fn_call& fn);
    as_value localconnection_connect(const fn_call& fn);
    as_value localconnection_send(const fn_call& fn);
    as_value localconnection_asyncError(const fn_call& fn);
    as_value localconnection_securityError(const fn_call& fn);
    as_value localconnection_status(const fn_call& fn);
    as_value localconnection_ctor(const fn_call& fn);
    void attachLocalConnectionInterface(as_object& o);
    void attachLocalConnectionStaticInterface(as_object& o);
    as_object* getLocalConnectionInterface();

}

class LocalConnection_as : public as_object
{

public:

    LocalConnection_as()
        :
        as_object(getLocalConnectionInterface())
    {}
};

// extern (used by Global.cpp)
void localconnection_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&localconnection_ctor, getLocalConnectionInterface());
        attachLocalConnectionStaticInterface(*cl);
    }

    // Register _global.LocalConnection
    global.init_member("LocalConnection", cl.get());
}

namespace {

void
attachLocalConnectionInterface(as_object& o)
{
    o.init_member("allowInsecureDomain", new builtin_function(localconnection_allowInsecureDomain));
    o.init_member("close", new builtin_function(localconnection_close));
    o.init_member("connect", new builtin_function(localconnection_connect));
    o.init_member("send", new builtin_function(localconnection_send));
    o.init_member("asyncError", new builtin_function(localconnection_asyncError));
    o.init_member("securityError", new builtin_function(localconnection_securityError));
    o.init_member("status", new builtin_function(localconnection_status));
}

void
attachLocalConnectionStaticInterface(as_object& o)
{

}

as_object*
getLocalConnectionInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachLocalConnectionInterface(*o);
    }
    return o.get();
}

as_value
localconnection_allowInsecureDomain(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_close(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_connect(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_send(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_asyncError(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_status(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new LocalConnection_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

