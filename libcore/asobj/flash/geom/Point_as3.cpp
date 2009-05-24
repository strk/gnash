// Point_as3.cpp:  ActionScript "Point" class, for Gnash.
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

#include "geom/Point_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value point_clone(const fn_call& fn);
    as_value point_distance(const fn_call& fn);
    as_value point_equals(const fn_call& fn);
    as_value point_interpolate(const fn_call& fn);
    as_value point_normalize(const fn_call& fn);
    as_value point_offset(const fn_call& fn);
    as_value point_polar(const fn_call& fn);
    as_value point_subtract(const fn_call& fn);
    as_value point_toString(const fn_call& fn);
    as_value point_ctor(const fn_call& fn);
    void attachPointInterface(as_object& o);
    void attachPointStaticInterface(as_object& o);
    as_object* getPointInterface();

}

// extern (used by Global.cpp)
void point_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&point_ctor, getPointInterface());
        attachPointStaticInterface(*cl);
    }

    // Register _global.Point
    global.init_member("Point", cl.get());
}

namespace {

void
attachPointInterface(as_object& o)
{
    o.init_member("clone", new builtin_function(point_clone));
    o.init_member("distance", new builtin_function(point_distance));
    o.init_member("equals", new builtin_function(point_equals));
    o.init_member("interpolate", new builtin_function(point_interpolate));
    o.init_member("normalize", new builtin_function(point_normalize));
    o.init_member("offset", new builtin_function(point_offset));
    o.init_member("polar", new builtin_function(point_polar));
    o.init_member("subtract", new builtin_function(point_subtract));
    o.init_member("toString", new builtin_function(point_toString));
}

void
attachPointStaticInterface(as_object& o)
{

}

as_object*
getPointInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachPointInterface(*o);
    }
    return o.get();
}

as_value
point_clone(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_distance(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_equals(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_interpolate(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_normalize(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_offset(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_polar(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_subtract(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_toString(const fn_call& fn)
{
    boost::intrusive_ptr<Point_as3> ptr =
        ensureType<Point_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
point_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Point_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

