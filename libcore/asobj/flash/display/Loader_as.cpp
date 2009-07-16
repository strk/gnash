// Loader_as.cpp:  ActionScript "Loader" class, for Gnash.
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

#include "display/Loader_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value loader_load(const fn_call& fn);
    as_value loader_loadBytes(const fn_call& fn);
    as_value loader_unload(const fn_call& fn);
    as_value loader_ctor(const fn_call& fn);
    void attachLoaderInterface(as_object& o);
    void attachLoaderStaticInterface(as_object& o);
    as_object* getLoaderInterface();

}

class Loader_as : public as_object
{

public:

    Loader_as()
        :
        as_object(getLoaderInterface())
    {}
};

// extern (used by Global.cpp)
void loader_class_init(as_object& global, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&loader_ctor, getLoaderInterface());
        attachLoaderStaticInterface(*cl);
    }

    // Register _global.Loader
    global.init_member("Loader", cl.get());
}

namespace {

void
attachLoaderInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("load", gl->createFunction(loader_load));
    o.init_member("loadBytes", gl->createFunction(loader_loadBytes));
    o.init_member("unload", gl->createFunction(loader_unload));
}

void
attachLoaderStaticInterface(as_object& /*o*/)
{

}

as_object*
getLoaderInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachLoaderInterface(*o);
    }
    return o.get();
}

as_value
loader_load(const fn_call& fn)
{
    boost::intrusive_ptr<Loader_as> ptr =
        ensureType<Loader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loader_loadBytes(const fn_call& fn)
{
    boost::intrusive_ptr<Loader_as> ptr =
        ensureType<Loader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loader_unload(const fn_call& fn)
{
    boost::intrusive_ptr<Loader_as> ptr =
        ensureType<Loader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loader_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new Loader_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

