// ExternalInterface_as.cpp:  ActionScript "ExternalInterface" class, for Gnash.
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

#include "external/ExternalInterface_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value externalinterface_call(const fn_call& fn);
    as_value externalinterface_ctor(const fn_call& fn);
    void attachExternalInterfaceInterface(as_object& o);
    void attachExternalInterfaceStaticInterface(as_object& o);
    as_object* getExternalInterfaceInterface();

}

// extern (used by Global.cpp)
void externalinterface_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&externalinterface_ctor, getExternalInterfaceInterface());
        attachExternalInterfaceStaticInterface(*cl);
    }

    // Register _global.ExternalInterface
    global.init_member("ExternalInterface", cl.get());
}

namespace {

void
attachExternalInterfaceInterface(as_object& o)
{
    o.init_member("call", new builtin_function(externalinterface_call));
}

void
attachExternalInterfaceStaticInterface(as_object& o)
{

}

as_object*
getExternalInterfaceInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachExternalInterfaceInterface(*o);
    }
    return o.get();
}

as_value
externalinterface_call(const fn_call& fn)
{
    boost::intrusive_ptr<ExternalInterface_as> ptr =
        ensureType<ExternalInterface_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
externalinterface_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new ExternalInterface_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

