// XMLSocket_as3.cpp:  ActionScript "XMLSocket" class, for Gnash.
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

#include "net/XMLSocket_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value xmlsocket_connect(const fn_call& fn);
    as_value xmlsocket_send(const fn_call& fn);
    as_value xmlsocket_close(const fn_call& fn);
    as_value xmlsocket_data(const fn_call& fn);
    as_value xmlsocket_ioError(const fn_call& fn);
    as_value xmlsocket_securityError(const fn_call& fn);
    as_value xmlsocket_ctor(const fn_call& fn);
    void attachXMLSocketInterface(as_object& o);
    void attachXMLSocketStaticInterface(as_object& o);
    as_object* getXMLSocketInterface();

}

// extern (used by Global.cpp)
void xmlsocket_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&xmlsocket_ctor, getXMLSocketInterface());
        attachXMLSocketStaticInterface(*cl);
    }

    // Register _global.XMLSocket
    global.init_member("XMLSocket", cl.get());
}

namespace {

void
attachXMLSocketInterface(as_object& o)
{
    o.init_member("connect", new builtin_function(xmlsocket_connect));
    o.init_member("send", new builtin_function(xmlsocket_send));
    o.init_member("close", new builtin_function(xmlsocket_close));
    o.init_member("connect", new builtin_function(xmlsocket_connect));
    o.init_member("data", new builtin_function(xmlsocket_data));
    o.init_member("ioError", new builtin_function(xmlsocket_ioError));
    o.init_member("securityError", new builtin_function(xmlsocket_securityError));
}

void
attachXMLSocketStaticInterface(as_object& o)
{

}

as_object*
getXMLSocketInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachXMLSocketInterface(*o);
    }
    return o.get();
}

as_value
xmlsocket_connect(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as3> ptr =
        ensureType<XMLSocket_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlsocket_send(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as3> ptr =
        ensureType<XMLSocket_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlsocket_close(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as3> ptr =
        ensureType<XMLSocket_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlsocket_data(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as3> ptr =
        ensureType<XMLSocket_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlsocket_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as3> ptr =
        ensureType<XMLSocket_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlsocket_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as3> ptr =
        ensureType<XMLSocket_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlsocket_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new XMLSocket_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

