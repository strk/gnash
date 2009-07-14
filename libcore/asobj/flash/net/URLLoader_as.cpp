// URLLoader_as.cpp:  ActionScript "URLLoader" class, for Gnash.
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

#include "net/URLLoader_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value urlloader_close(const fn_call& fn);
    as_value urlloader_load(const fn_call& fn);
    as_value urlloader_complete(const fn_call& fn);
    as_value urlloader_httpStatus(const fn_call& fn);
    as_value urlloader_ioError(const fn_call& fn);
    as_value urlloader_open(const fn_call& fn);
    as_value urlloader_progress(const fn_call& fn);
    as_value urlloader_securityError(const fn_call& fn);
    as_value urlloader_ctor(const fn_call& fn);
    void attachURLLoaderInterface(as_object& o);
    void attachURLLoaderStaticInterface(as_object& o);
    as_object* getURLLoaderInterface();

}

class URLLoader_as : public as_object
{

public:

    URLLoader_as()
        :
        as_object(getURLLoaderInterface())
    {}
};

// extern (used by Global.cpp)
void urlloader_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&urlloader_ctor, getURLLoaderInterface());
        attachURLLoaderStaticInterface(*cl);
    }

    // Register _global.URLLoader
    global.init_member("URLLoader", cl.get());
}

namespace {

void
attachURLLoaderInterface(as_object& o)
{
    o.init_member("close", gl->createFunction(urlloader_close));
    o.init_member("load", gl->createFunction(urlloader_load));
    o.init_member("complete", gl->createFunction(urlloader_complete));
    o.init_member("httpStatus", gl->createFunction(urlloader_httpStatus));
    o.init_member("ioError", gl->createFunction(urlloader_ioError));
    o.init_member("open", gl->createFunction(urlloader_open));
    o.init_member("progress", gl->createFunction(urlloader_progress));
    o.init_member("securityError", gl->createFunction(urlloader_securityError));
}

void
attachURLLoaderStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);

}

as_object*
getURLLoaderInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachURLLoaderInterface(*o);
    }
    return o.get();
}

as_value
urlloader_close(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_load(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_complete(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_httpStatus(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_open(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_progress(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<URLLoader_as> ptr =
        ensureType<URLLoader_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlloader_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new URLLoader_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

