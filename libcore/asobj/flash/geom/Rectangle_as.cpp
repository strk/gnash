// Rectangle_as.cpp:  ActionScript "Rectangle" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "Rectangle_as.h"

#include <sstream>

#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "as_value.h"
#include "namedStrings.h"
#include "GnashNumeric.h" // isFinite
#include "as_function.h"

namespace gnash {

namespace {
    as_value Rectangle_clone(const fn_call& fn);
    as_value Rectangle_contains(const fn_call& fn);
    as_value Rectangle_containsPoint(const fn_call& fn);
    as_value Rectangle_containsRectangle(const fn_call& fn);
    as_value Rectangle_equals(const fn_call& fn);
    as_value Rectangle_inflate(const fn_call& fn);
    as_value Rectangle_inflatePoint(const fn_call& fn);
    as_value Rectangle_intersection(const fn_call& fn);
    as_value Rectangle_intersects(const fn_call& fn);
    as_value Rectangle_isEmpty(const fn_call& fn);
    as_value Rectangle_offset(const fn_call& fn);
    as_value Rectangle_offsetPoint(const fn_call& fn);
    as_value Rectangle_setEmpty(const fn_call& fn);
    as_value Rectangle_toString(const fn_call& fn);
    as_value Rectangle_union(const fn_call& fn);
    as_value Rectangle_bottom(const fn_call& fn);
    as_value Rectangle_bottomRight(const fn_call& fn);
    as_value Rectangle_left(const fn_call& fn);
    as_value Rectangle_right(const fn_call& fn);
    as_value Rectangle_size(const fn_call& fn);
    as_value Rectangle_top(const fn_call& fn);
    as_value Rectangle_topLeft(const fn_call& fn);
    as_value Rectangle_ctor(const fn_call& fn);
    as_value get_flash_geom_rectangle_constructor(const fn_call& fn);

}

void
rectangle_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_geom_rectangle_constructor,
            flags);
}


namespace {

void
attachRectangleInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("clone", gl.createFunction(Rectangle_clone), 0);
    o.init_member("contains", gl.createFunction(Rectangle_contains), 0);
    o.init_member("containsPoint",
            gl.createFunction(Rectangle_containsPoint), 0);
    o.init_member("containsRectangle",
            gl.createFunction(Rectangle_containsRectangle), 0);
    o.init_member("equals", gl.createFunction(Rectangle_equals), 0);
    o.init_member("inflate", gl.createFunction(Rectangle_inflate), 0);
    o.init_member("inflatePoint",
            gl.createFunction(Rectangle_inflatePoint), 0);
    o.init_member("intersection",
            gl.createFunction(Rectangle_intersection), 0);
    o.init_member("intersects", gl.createFunction(Rectangle_intersects), 0);
    o.init_member("isEmpty", gl.createFunction(Rectangle_isEmpty), 0);
    o.init_member("offset", gl.createFunction(Rectangle_offset), 0);
    o.init_member("offsetPoint", gl.createFunction(Rectangle_offsetPoint), 0);
    o.init_member("setEmpty", gl.createFunction(Rectangle_setEmpty), 0);
    o.init_member("toString", gl.createFunction(Rectangle_toString), 0);
    o.init_member("union", gl.createFunction(Rectangle_union), 0);
    o.init_property("bottom",
            Rectangle_bottom, Rectangle_bottom, 0);
    o.init_property("bottomRight", Rectangle_bottomRight,
            Rectangle_bottomRight, 0);
    o.init_property("left", Rectangle_left,
            Rectangle_left, 0);
    o.init_property("right", Rectangle_right,
            Rectangle_right, 0);
    o.init_property("size", Rectangle_size,
            Rectangle_size, 0);
    o.init_property("top", Rectangle_top,
            Rectangle_top, 0);
    o.init_property("topLeft", Rectangle_topLeft,
            Rectangle_topLeft, 0);
}


as_value
Rectangle_clone(const fn_call& fn)
{
    // The object will be interpreted as a rectangle. Any Rectangle
    // properties that the object has (width, height, x, y) are used.
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x = getMember(*ptr, NSV::PROP_X);
    as_value y = getMember(*ptr, NSV::PROP_Y);
    as_value w = getMember(*ptr, NSV::PROP_WIDTH);
    as_value h = getMember(*ptr, NSV::PROP_HEIGHT);

    as_function* ctor = getClassConstructor(fn, "flash.geom.Rectangle");
    if (!ctor) return as_value();

    fn_call::Args args;
    args += x, y, w, h;

    return constructInstance(*ctor, fn.env(), args);
}

as_value
Rectangle_contains(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value rect_x_as = getMember(*ptr, NSV::PROP_X);
    as_value rect_width_as = getMember(*ptr, NSV::PROP_WIDTH);
    as_value rect_y_as = getMember(*ptr, NSV::PROP_Y);
    as_value rect_height_as = getMember(*ptr, NSV::PROP_HEIGHT);

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror("flash.geom.Rectangle(%s): %s", ss.str(), 
                _("missing arguments"));
        );
        return as_value();
    }

    const as_value& x_as = fn.arg(0);
    const as_value& y_as = fn.arg(1);

    VM& vm = getVM(fn);
    
    as_value rect_x1_as = rect_x_as;
    newAdd(rect_x1_as, rect_width_as, vm);

    as_value rect_y1_as = rect_y_as;
    newAdd(rect_y1_as, rect_height_as, vm);

    // Points are contained within the Rectangle IFF they lie
    // on the top or left borders of the rectangle, but not the right or
    // bottom borders, or they are not on a border but between all.
    
    // NOTE: order of tests is important, see actionscript.all/Rectangle.as

    as_value ret = newLessThan(x_as, rect_x_as, vm);
    if (ret.is_undefined()) return as_value();
    if (toBool(ret, vm)) return as_value(false); 

    ret = newLessThan(x_as, rect_x1_as, vm);
    if (ret.is_undefined()) return as_value();
    if (!toBool(ret, vm)) return as_value(false); 

    ret = newLessThan(y_as, rect_y_as, vm);
    if (ret.is_undefined()) return as_value();
    if (toBool(ret, vm)) return as_value(false); 

    ret = newLessThan(y_as, rect_y1_as, vm);
    if (ret.is_undefined()) return as_value();
    if (!toBool(ret, vm)) return as_value(false); 

    return as_value(true);

}


// This is horrible ActionScript implemented in C++.
as_value
Rectangle_containsPoint(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_object* arg = (fn.nargs > 0) ? toObject(fn.arg(0), getVM(fn)) : 0;
    
    VM& vm = getVM(fn);

    as_value thisx = getMember(*ptr, NSV::PROP_X);
    as_value argx = arg ? getMember(*arg, NSV::PROP_X) : as_value();
    
    // argx >= thisx
    as_value ret = newLessThan(argx, thisx, vm);
    if (ret.is_undefined()) return as_value(); 
    if (toBool(ret, vm)) return as_value(false); 

    as_value thisw = getMember(*ptr, NSV::PROP_WIDTH);
    
    newAdd(thisx, thisw, vm);
    ret = newLessThan(argx, thisx, vm);
    if (ret.is_undefined()) return as_value(); 
    if (!toBool(ret, vm)) return as_value(false); 
 
    as_value thisy = getMember(*ptr, NSV::PROP_Y);
    as_value argy = arg ? getMember(*arg, NSV::PROP_Y) : as_value();
    
    // argy >= thisy
    ret = newLessThan(argy, thisy, vm);
    if (ret.is_undefined()) return as_value(); 
    if (toBool(ret, vm)) return as_value(false); 

    as_value thish = getMember(*ptr, NSV::PROP_HEIGHT);
    
    newAdd(thisy, thish, vm);
    ret = newLessThan(argy, thisy, vm);
    if (ret.is_undefined()) return as_value(); 
    if (!toBool(ret, vm)) return as_value(false); 

    return as_value(true);


}

as_value
Rectangle_containsRectangle(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Rectangle_equals(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) return as_value(false);
    as_object* comp = toObject(fn.arg(0), getVM(fn));

    if (!comp) return as_value(false);

    if (!comp->instanceOf(getClassConstructor(fn, "flash.geom.Rectangle"))) {
        return as_value(false);
    }

    if (!equals(getMember(*comp, NSV::PROP_X),
                getMember(*ptr, NSV::PROP_X), getVM(fn))) {
        return as_value(false);
    }

    if (!equals(getMember(*comp, NSV::PROP_Y),
                getMember(*ptr, NSV::PROP_Y), getVM(fn))) {
        return as_value(false);
    }

    if (!equals(getMember(*comp, NSV::PROP_WIDTH),
                getMember(*ptr, NSV::PROP_WIDTH), getVM(fn))) {
        return as_value(false);
    }

    if (!equals(getMember(*comp, NSV::PROP_HEIGHT),
                getMember(*ptr, NSV::PROP_HEIGHT), getVM(fn))) {
        return as_value(false);
    }

    return as_value(true);
}

as_value
Rectangle_inflate(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Rectangle_inflatePoint(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Rectangle_intersection(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Rectangle_intersects(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Rectangle_isEmpty(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value w = getMember(*ptr, NSV::PROP_WIDTH);
    if (w.is_undefined() || w.is_null()) return as_value(true);

    as_value h = getMember(*ptr, NSV::PROP_HEIGHT);
    if (h.is_undefined() || h.is_null()) return as_value(true);

    double wn = toNumber(w, getVM(fn));
    if (!isFinite(wn) || wn <= 0) return as_value(true);

    double hn = toNumber(h, getVM(fn));
    if (!isFinite(hn) || hn <= 0) return as_value(true);

    return as_value(false);
}

as_value
Rectangle_offset(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value xdelta = fn.nargs > 0 ? fn.arg(0) : as_value();
    as_value ydelta = fn.nargs > 1 ? fn.arg(1) : as_value();

    as_value x = getMember(*ptr, NSV::PROP_X);
    newAdd(x, xdelta, getVM(fn));
    ptr->set_member(NSV::PROP_X, x);

    as_value y = getMember(*ptr, NSV::PROP_Y);
    newAdd(y, ydelta, getVM(fn));
    ptr->set_member(NSV::PROP_Y, y);

    return as_value();
}

as_value
Rectangle_offsetPoint(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_object* arg = (fn.nargs > 0) ? toObject(fn.arg(0), getVM(fn)) : 0;
    if (!arg) return as_value();

    as_value xdelta = getMember(*arg, NSV::PROP_X);
    as_value ydelta = getMember(*arg, NSV::PROP_Y);

    as_value x = getMember(*ptr, NSV::PROP_X);
    newAdd(x, xdelta, getVM(fn));
    ptr->set_member(NSV::PROP_X, x);

    as_value y = getMember(*ptr, NSV::PROP_Y);
    newAdd(y, ydelta, getVM(fn));
    ptr->set_member(NSV::PROP_Y, y);

    return as_value();
}

as_value
Rectangle_setEmpty(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    ptr->set_member(NSV::PROP_X, 0.0);
    ptr->set_member(NSV::PROP_Y, 0.0);
    ptr->set_member(NSV::PROP_WIDTH, 0.0);
    ptr->set_member(NSV::PROP_HEIGHT, 0.0);
    return as_value();
}

as_value
Rectangle_toString(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x = getMember(*ptr, NSV::PROP_X);
    as_value y = getMember(*ptr, NSV::PROP_Y);
    as_value w = getMember(*ptr, NSV::PROP_WIDTH);
    as_value h = getMember(*ptr, NSV::PROP_HEIGHT);

    VM& vm = getVM(fn);

    as_value ret("(x=");
    newAdd(ret, x, vm);
    newAdd(ret, ", y=", vm);
    newAdd(ret, y, vm);
    newAdd(ret, ", w=", vm);
    newAdd(ret, w, vm);
    newAdd(ret, ", h=", vm);
    newAdd(ret, h, vm);
    newAdd(ret, ")", vm);

    return ret;
}

as_value
Rectangle_union(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Rectangle_bottom(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        as_value b = getMember(*ptr, NSV::PROP_Y);
        as_value height = getMember(*ptr, NSV::PROP_HEIGHT);
        newAdd(b, height, getVM(fn));
        return b;
    }
    as_value y = getMember(*ptr, NSV::PROP_Y);
    as_value height = fn.arg(0);
    subtract(height, y, getVM(fn));
    ptr->set_member(NSV::PROP_HEIGHT, height);

    return as_value();
}

as_value
Rectangle_bottomRight(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {

        as_value x = getMember(*ptr, NSV::PROP_X);
        as_value y = getMember(*ptr, NSV::PROP_Y);
        as_value w = getMember(*ptr, NSV::PROP_WIDTH);
        as_value h = getMember(*ptr, NSV::PROP_HEIGHT);

        VM& vm = getVM(fn);
        newAdd(x, w, vm);
        newAdd(y, h, vm);

        as_value point(findObject(fn.env(), "flash.geom.Point"));

        as_function* pointCtor = point.to_function();

        if (!pointCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror("Failed to construct flash.geom.Point!");
            );
            return as_value();
        }

        fn_call::Args args;
        args += x, y;

        return constructInstance(*pointCtor, fn.env(), args);
    }

    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Attempt to set read-only property %s"),
        "Rectangle.bottomRight");
    );
    return as_value();

}

as_value
Rectangle_left(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        return getMember(*ptr, NSV::PROP_X);
    }
    as_value oldx = getMember(*ptr, NSV::PROP_X);

    as_value newx = fn.arg(0);
    ptr->set_member(NSV::PROP_X, newx);

    as_value w = getMember(*ptr, NSV::PROP_WIDTH);

    VM& vm = getVM(fn);
    subtract(oldx, newx, vm);
    newAdd(w, oldx, vm);
    ptr->set_member(NSV::PROP_WIDTH, w);

    return as_value();
}

as_value
Rectangle_right(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        as_value r = getMember(*ptr, NSV::PROP_X);
        as_value width = getMember(*ptr, NSV::PROP_WIDTH);
        newAdd(r, width, getVM(fn));
        return r;
    }

    as_value x = getMember(*ptr, NSV::PROP_X);

    as_value width = fn.arg(0);
    subtract(width, x, getVM(fn));
    ptr->set_member(NSV::PROP_WIDTH, width);
    return as_value();
}

as_value
Rectangle_size(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        as_value w = getMember(*ptr, NSV::PROP_WIDTH);
        as_value h = getMember(*ptr, NSV::PROP_HEIGHT);

        as_function* pointCtor = getClassConstructor(fn, "flash.geom.Point");
        if (!pointCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror("Failed to construct flash.geom.Point!");
            );
            return as_value();
        }

        fn_call::Args args;
        args += w, h;

        return constructInstance(*pointCtor, fn.env(), args);
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set read-only property %s"),
            "Rectangle.size");
    );

    return as_value();
}

as_value
Rectangle_top(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        // getter
        return getMember(*ptr, NSV::PROP_Y);
    }

    // setter
    as_value oldy = getMember(*ptr, NSV::PROP_Y);

    as_value newy = fn.arg(0);
    ptr->set_member(NSV::PROP_Y, newy);

    as_value h = getMember(*ptr, NSV::PROP_HEIGHT);

    VM& vm = getVM(fn);
    subtract(oldy, newy, vm);
    newAdd(h, oldy, vm);
    ptr->set_member(NSV::PROP_HEIGHT, h);

    return as_value();
}

as_value
Rectangle_topLeft(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    // getter
    if (!fn.nargs) {
        as_value x = getMember(*ptr, NSV::PROP_X);
        as_value y = getMember(*ptr, NSV::PROP_Y);

        as_function* pointCtor = getClassConstructor(fn, "flash.geom.Point");
        if (!pointCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror("Failed to construct flash.geom.Point!");
            );
            return as_value();
        }

        fn_call::Args args;
        args += x, y;

        return constructInstance(*pointCtor, fn.env(), args);

    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set read-only property %s"),
            "Rectangle.topLeft");
    );

    return as_value();
}


as_value
Rectangle_ctor(const fn_call& fn)
{

    as_object* obj = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        // TODO: use NSV !
        const ObjectURI& setEmpty = getURI(getVM(fn), "setEmpty");
        callMethod(obj, setEmpty);
        return as_value();
    }

    // At least one arg
    obj->set_member(NSV::PROP_X, fn.arg(0));
    obj->set_member(NSV::PROP_Y, fn.nargs > 1 ? fn.arg(1) : as_value());
    obj->set_member(NSV::PROP_WIDTH, fn.nargs > 2 ? fn.arg(2) : as_value());
    obj->set_member(NSV::PROP_HEIGHT, fn.nargs > 3 ? fn.arg(3) : as_value());

    return as_value();
}

as_value
get_flash_geom_rectangle_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.Rectangle class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
    attachRectangleInterface(*proto);
    return gl.createClass(&Rectangle_ctor, proto);
}

} // anonymous namespace
} // end of gnash namespace
