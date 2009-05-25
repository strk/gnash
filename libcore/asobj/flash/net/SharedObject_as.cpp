// SharedObject_as.cpp:  ActionScript "SharedObject" class, for Gnash.
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

#include "net/SharedObject_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value sharedobject_close(const fn_call& fn);
    as_value sharedobject_connect(const fn_call& fn);
    as_value sharedobject_flush(const fn_call& fn);
    as_value sharedobject_getLocal(const fn_call& fn);
    as_value sharedobject_getRemote(const fn_call& fn);
    as_value sharedobject_send(const fn_call& fn);
    as_value sharedobject_setDirty(const fn_call& fn);
    as_value sharedobject_setProperty(const fn_call& fn);
    as_value sharedobject_asyncError(const fn_call& fn);
    as_value sharedobject_netStatus(const fn_call& fn);
    as_value sharedobject_sync(const fn_call& fn);
    as_value sharedobject_ctor(const fn_call& fn);
    void attachSharedObjectInterface(as_object& o);
    void attachSharedObjectStaticInterface(as_object& o);
    as_object* getSharedObjectInterface();

}

// extern (used by Global.cpp)
void sharedobject_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&sharedobject_ctor, getSharedObjectInterface());
        attachSharedObjectStaticInterface(*cl);
    }

    // Register _global.SharedObject
    global.init_member("SharedObject", cl.get());
}

namespace {

void
attachSharedObjectInterface(as_object& o)
{
    o.init_member("close", new builtin_function(sharedobject_close));
    o.init_member("connect", new builtin_function(sharedobject_connect));
    o.init_member("flush", new builtin_function(sharedobject_flush));
    o.init_member("getLocal", new builtin_function(sharedobject_getLocal));
    o.init_member("getRemote", new builtin_function(sharedobject_getRemote));
    o.init_member("send", new builtin_function(sharedobject_send));
    o.init_member("setDirty", new builtin_function(sharedobject_setDirty));
    o.init_member("setProperty", new builtin_function(sharedobject_setProperty));
    o.init_member("asyncError", new builtin_function(sharedobject_asyncError));
    o.init_member("netStatus", new builtin_function(sharedobject_netStatus));
    o.init_member("sync", new builtin_function(sharedobject_sync));
}

void
attachSharedObjectStaticInterface(as_object& o)
{

}

as_object*
getSharedObjectInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachSharedObjectInterface(*o);
    }
    return o.get();
}

as_value
sharedobject_close(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_connect(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_flush(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_getLocal(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_getRemote(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_send(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_setDirty(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_setProperty(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_asyncError(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_netStatus(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_sync(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> ptr =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sharedobject_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new SharedObject_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

