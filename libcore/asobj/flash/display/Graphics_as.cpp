// Graphics_as.cpp:  ActionScript "Graphics" class, for Gnash.
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

#include "display/Graphics_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value graphics_beginFill(const fn_call& fn);
    as_value graphics_beginGradientFill(const fn_call& fn);
    as_value graphics_clear(const fn_call& fn);
    as_value graphics_curveTo(const fn_call& fn);
    as_value graphics_drawCircle(const fn_call& fn);
    as_value graphics_drawEllipse(const fn_call& fn);
    as_value graphics_drawRect(const fn_call& fn);
    as_value graphics_drawRoundRect(const fn_call& fn);
    as_value graphics_endFill(const fn_call& fn);
    as_value graphics_lineGradientStyle(const fn_call& fn);
    as_value graphics_lineStyle(const fn_call& fn);
    as_value graphics_lineTo(const fn_call& fn);
    as_value graphics_moveTo(const fn_call& fn);
    as_value graphics_ctor(const fn_call& fn);
    void attachGraphicsInterface(as_object& o);
    void attachGraphicsStaticInterface(as_object& o);
    as_object* getGraphicsInterface();

}

class Graphics_as : public as_object
{

public:

    Graphics_as()
        :
        as_object(getGraphicsInterface())
    {}
};

// extern (used by Global.cpp)
void graphics_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&graphics_ctor, getGraphicsInterface());
        attachGraphicsStaticInterface(*cl);
    }

    // Register _global.Graphics
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachGraphicsInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("beginFill", gl->createFunction(graphics_beginFill));
    o.init_member("beginGradientFill", gl->createFunction(graphics_beginGradientFill));
    o.init_member("clear", gl->createFunction(graphics_clear));
    o.init_member("curveTo", gl->createFunction(graphics_curveTo));
    o.init_member("drawCircle", gl->createFunction(graphics_drawCircle));
    o.init_member("drawEllipse", gl->createFunction(graphics_drawEllipse));
    o.init_member("drawRect", gl->createFunction(graphics_drawRect));
    o.init_member("drawRoundRect", gl->createFunction(graphics_drawRoundRect));
    o.init_member("endFill", gl->createFunction(graphics_endFill));
    o.init_member("lineGradientStyle", gl->createFunction(graphics_lineGradientStyle));
    o.init_member("lineStyle", gl->createFunction(graphics_lineStyle));
    o.init_member("lineTo", gl->createFunction(graphics_lineTo));
    o.init_member("moveTo", gl->createFunction(graphics_moveTo));
}

void
attachGraphicsStaticInterface(as_object& /*o*/)
{
}

as_object*
getGraphicsInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachGraphicsInterface(*o);
    }
    return o.get();
}

as_value
graphics_beginFill(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_beginGradientFill(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_clear(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_curveTo(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_drawCircle(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_drawEllipse(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_drawRect(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_drawRoundRect(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_endFill(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_lineGradientStyle(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_lineStyle(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_lineTo(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_moveTo(const fn_call& fn)
{
    boost::intrusive_ptr<Graphics_as> ptr =
        ensureType<Graphics_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
graphics_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new Graphics_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

