// Rectangle_as3.cpp:  ActionScript "Rectangle" class, for Gnash.
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

#include "geom/Rectangle_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value rectangle_contains(const fn_call& fn);
    as_value rectangle_containsPoint(const fn_call& fn);
    as_value rectangle_containsRect(const fn_call& fn);
    as_value rectangle_equals(const fn_call& fn);
    as_value rectangle_inflate(const fn_call& fn);
    as_value rectangle_inflatePoint(const fn_call& fn);
    as_value rectangle_intersection(const fn_call& fn);
    as_value rectangle_intersects(const fn_call& fn);
    as_value rectangle_isEmpty(const fn_call& fn);
    as_value rectangle_offset(const fn_call& fn);
    as_value rectangle_offsetPoint(const fn_call& fn);
    as_value rectangle_setEmpty(const fn_call& fn);
    as_value rectangle_toString(const fn_call& fn);
    as_value rectangle_union(const fn_call& fn);
    as_value rectangle_ctor(const fn_call& fn);
    void attachRectangleInterface(as_object& o);
    void attachRectangleStaticInterface(as_object& o);
    as_object* getRectangleInterface();

}

// extern (used by Global.cpp)
void rectangle_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&rectangle_ctor, getRectangleInterface());
        attachRectangleStaticInterface(*cl);
    }

    // Register _global.Rectangle
    global.init_member("Rectangle", cl.get());
}

namespace {

void
attachRectangleInterface(as_object& o)
{
    o.init_member("contains", new builtin_function(rectangle_contains));
    o.init_member("containsPoint", new builtin_function(rectangle_containsPoint));
    o.init_member("containsRect", new builtin_function(rectangle_containsRect));
    o.init_member("equals", new builtin_function(rectangle_equals));
    o.init_member("inflate", new builtin_function(rectangle_inflate));
    o.init_member("inflatePoint", new builtin_function(rectangle_inflatePoint));
    o.init_member("intersection", new builtin_function(rectangle_intersection));
    o.init_member("intersects", new builtin_function(rectangle_intersects));
    o.init_member("isEmpty", new builtin_function(rectangle_isEmpty));
    o.init_member("offset", new builtin_function(rectangle_offset));
    o.init_member("offsetPoint", new builtin_function(rectangle_offsetPoint));
    o.init_member("setEmpty", new builtin_function(rectangle_setEmpty));
    o.init_member("toString", new builtin_function(rectangle_toString));
    o.init_member("union", new builtin_function(rectangle_union));
}

void
attachRectangleStaticInterface(as_object& o)
{

}

as_object*
getRectangleInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachRectangleInterface(*o);
    }
    return o.get();
}

as_value
rectangle_contains(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_containsPoint(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_containsRect(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_equals(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_inflate(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_inflatePoint(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_intersection(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_intersects(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_isEmpty(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_offset(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_offsetPoint(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_setEmpty(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_toString(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_union(const fn_call& fn)
{
    boost::intrusive_ptr<Rectangle_as3> ptr =
        ensureType<Rectangle_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
rectangle_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Rectangle_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

