// LoaderInfo_as.cpp:  ActionScript "LoaderInfo" class, for Gnash.
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

#include "display/LoaderInfo_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value loaderinfo_complete(const fn_call& fn);
    as_value loaderinfo_httpStatus(const fn_call& fn);
    as_value loaderinfo_init(const fn_call& fn);
    as_value loaderinfo_ioError(const fn_call& fn);
    as_value loaderinfo_open(const fn_call& fn);
    as_value loaderinfo_progress(const fn_call& fn);
    as_value loaderinfo_unload(const fn_call& fn);
    as_value loaderinfo_ctor(const fn_call& fn);
    void attachLoaderInfoInterface(as_object& o);
    void attachLoaderInfoStaticInterface(as_object& o);
    as_object* getLoaderInfoInterface();

}

class LoaderInfo_as : public as_object
{

public:

    LoaderInfo_as()
        :
        as_object(getLoaderInfoInterface())
    {}
};

// extern (used by Global.cpp)
void loaderinfo_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&loaderinfo_ctor, getLoaderInfoInterface());
        attachLoaderInfoStaticInterface(*cl);
    }

    // Register _global.LoaderInfo
    global.init_member("LoaderInfo", cl.get());
}

namespace {

void
attachLoaderInfoInterface(as_object& o)
{
    o.init_member("complete", new builtin_function(loaderinfo_complete));
    o.init_member("httpStatus", new builtin_function(loaderinfo_httpStatus));
    o.init_member("init", new builtin_function(loaderinfo_init));
    o.init_member("ioError", new builtin_function(loaderinfo_ioError));
    o.init_member("open", new builtin_function(loaderinfo_open));
    o.init_member("progress", new builtin_function(loaderinfo_progress));
    o.init_member("unload", new builtin_function(loaderinfo_unload));
}

void
attachLoaderInfoStaticInterface(as_object& o)
{

}

as_object*
getLoaderInfoInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachLoaderInfoInterface(*o);
    }
    return o.get();
}

as_value
loaderinfo_complete(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_httpStatus(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_init(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_open(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_progress(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_unload(const fn_call& fn)
{
    boost::intrusive_ptr<LoaderInfo_as> ptr =
        ensureType<LoaderInfo_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new LoaderInfo_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

