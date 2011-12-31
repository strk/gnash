// Point_as.cpp:  ActionScript "Point" class, for Gnash.
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

#include "Point_as.h"

#include <sstream>

#include "as_object.h" 
#include "as_function.h" 
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "GnashNumeric.h"
#include "namedStrings.h"

namespace gnash {

namespace {

    as_value point_add(const fn_call& fn);
    as_value point_clone(const fn_call& fn);
    as_value point_equals(const fn_call& fn);
    as_value point_normalize(const fn_call& fn);
    as_value point_offset(const fn_call& fn);
    as_value point_subtract(const fn_call& fn);
    as_value point_toString(const fn_call& fn);
    as_value point_length(const fn_call& fn);
    as_value point_distance(const fn_call& fn);
    as_value point_interpolate(const fn_call& fn);
    as_value point_polar(const fn_call& fn);
    as_value point_ctor(const fn_call& fn);

    as_value get_flash_geom_point_constructor(const fn_call& fn);

}

void
point_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_geom_point_constructor,
            flags);
}

namespace {

as_value
constructPoint(const fn_call& fn, const as_value& x, const as_value& y)
{
    as_function* ctor = getClassConstructor(fn, "flash.geom.Point");
    if (!ctor) return as_value();

    fn_call::Args args;
    args += x, y;

    return constructInstance(*ctor, fn.env(), args);
}

void
attachPointInterface(as_object& o)
{
    const int fl = 0;

    Global_as& gl = getGlobal(o);
    o.init_member("add", gl.createFunction(point_add), fl);
    o.init_member("clone", gl.createFunction(point_clone), fl);
    o.init_member("equals", gl.createFunction(point_equals), fl);
    o.init_member("normalize", gl.createFunction(point_normalize), fl);
    o.init_member("offset", gl.createFunction(point_offset), fl);
    o.init_member("subtract", gl.createFunction(point_subtract), fl);
    o.init_member("toString", gl.createFunction(point_toString), fl);
    o.init_property("length", point_length, point_length, fl);
}

void
attachPointStaticProperties(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("distance", gl.createFunction(point_distance), 0);
    o.init_member("interpolate", gl.createFunction(point_interpolate), 0);
    o.init_member("polar", gl.createFunction(point_polar), 0);
}


as_value
point_add(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x, y;
    ptr->get_member(NSV::PROP_X, &x);
    ptr->get_member(NSV::PROP_Y, &y);

    as_value x1, y1;

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s: missing arguments"), "Point.add()");
        );
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 1 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.add(%s): %s"), ss.str(),
                        _("arguments after first discarded"));
        }
        );
        const as_value& arg1 = fn.arg(0);
        as_object* o = toObject(arg1, getVM(fn));
        if ( ! o )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.add(%s): %s"), ss.str(),
                        _("first argument doesn't cast to object"));
            );
        }
        else
        {
            if ( ! o->get_member(NSV::PROP_X, &x1) )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("Point.add(%s): %s"), ss.str(),
                    _("first argument cast to object doesn't contain an 'x' member"));
                );
            }
            if ( ! o->get_member(NSV::PROP_Y, &y1) )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("Point.add(%s): %s"), ss.str(),
                    _("first argument cast to object doesn't contain an 'y' member"));
                );
            }
        }
    }

    VM& vm = getVM(fn);
    newAdd(x, x1, vm);
    newAdd(y, y1, vm);

    return constructPoint(fn, x, y);
}

as_value
point_clone(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x, y;
    ptr->get_member(NSV::PROP_X, &x);
    ptr->get_member(NSV::PROP_Y, &y);

    return constructPoint(fn, x, y);
}

as_value
point_equals(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s: missing arguments"), "Point.equals()");
        );
        return as_value(false);
    }

    const as_value& arg1 = fn.arg(0);
    if ( ! arg1.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.equals(%s): %s"), ss.str(),
                    _("First arg must be an object"));
        );
        return as_value(false);
    }
    as_object* o = toObject(arg1, getVM(fn));
    assert(o);
    if (!o->instanceOf(getClassConstructor(fn, "flash.geom.Point")))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.equals(%s): %s %s"), ss.str(),
            _("First arg must be an instance of"), "flash.geom.Point");
        );
        return as_value(false);
    }

    as_value x, y;
    ptr->get_member(NSV::PROP_X, &x);
    ptr->get_member(NSV::PROP_Y, &y);

    as_value x1, y1;
    o->get_member(NSV::PROP_X, &x1);
    o->get_member(NSV::PROP_Y, &y1);

    return as_value(equals(x, x1, getVM(fn)) && equals(y, y1, getVM(fn)));
}

as_value
point_normalize(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value argval;

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s: missing arguments"), "Point.normalize()");
        );
        return as_value();
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 1 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.normalize(%s): %s"), ss.str(),
                        _("arguments after first discarded"));
        }
        );

        argval = fn.arg(0);
    }

    // newlen may be NaN, and we'd still be updating x/y
    // see actionscript.all/Point.as
    double newlen = toNumber(argval, getVM(fn));

    as_value xval, yval;
    ptr->get_member(NSV::PROP_X, &xval);
    ptr->get_member(NSV::PROP_Y, &yval);

    double x = toNumber(xval, getVM(fn));
    if (!isFinite(x)) return as_value();
    double y = toNumber(yval, getVM(fn));
    if (!isFinite(y)) return as_value();

    if ( x == 0 && y == 0 ) return as_value();

    double curlen = std::sqrt(x*x+y*y);
    double fact = newlen/curlen;


    xval.set_double( toNumber(xval, getVM(fn)) * fact );
    yval.set_double( toNumber(yval, getVM(fn)) * fact );
    ptr->set_member(NSV::PROP_X, xval);
    ptr->set_member(NSV::PROP_Y, yval);

    return as_value();
}

as_value
point_offset(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x, y;
    ptr->get_member(NSV::PROP_X, &x);
    ptr->get_member(NSV::PROP_Y, &y);

    as_value xoff, yoff;

    if ( fn.nargs ) {
        xoff = fn.arg(0);
        if ( fn.nargs > 1 ) yoff = fn.arg(1);
    }

    VM& vm = getVM(fn);
    newAdd(x, xoff, vm);
    newAdd(y, yoff, vm);

    ptr->set_member(NSV::PROP_X, x);
    ptr->set_member(NSV::PROP_Y, y);

    return as_value();
}

as_value
point_subtract(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x, y;
    ptr->get_member(NSV::PROP_X, &x);
    ptr->get_member(NSV::PROP_Y, &y);

    as_value x1, y1;

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s: missing arguments"), "Point.add()");
        );
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 1 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.add(%s): %s"), ss.str(),
                        _("arguments after first discarded"));
        }
        );
        const as_value& arg1 = fn.arg(0);
        as_object* o = toObject(arg1, getVM(fn));
        if ( ! o )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.add(%s): %s"), ss.str(),
                        _("first argument doesn't cast to object"));
            );
        }
        else
        {
            if ( ! o->get_member(NSV::PROP_X, &x1) )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("Point.add(%s): %s"), ss.str(),
                              _("first argument casted to object doesn't contain an 'x' member"));
                );
            }
            if ( ! o->get_member(NSV::PROP_Y, &y1) )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("Point.add(%s): %s"), ss.str(),
                    _("first argument casted to object doesn't contain an 'y' member"));
                );
            }
        }
    }

    x.set_double(toNumber(x, getVM(fn)) - toNumber(x1, getVM(fn)));
    y.set_double(toNumber(y, getVM(fn)) - toNumber(y1, getVM(fn)));

    return constructPoint(fn, x, y);
}

as_value
point_toString(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    as_value x, y;
    ptr->get_member(NSV::PROP_X, &x);
    ptr->get_member(NSV::PROP_Y, &y);

    VM& vm = getVM(fn);

    as_value ret("(x=");
    newAdd(ret, x, vm);
    newAdd(ret, ", y=", vm);
    newAdd(ret, y, vm);
    newAdd(ret, ")", vm);

    return ret;
}

as_value
point_length(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    if ( ! fn.nargs ) // getter
    {
        as_value xval, yval;
        ptr->get_member(NSV::PROP_X, &xval);
        ptr->get_member(NSV::PROP_Y, &yval);
        double x = toNumber(xval, getVM(fn));
        double y = toNumber(yval, getVM(fn));

        double l = std::sqrt(x*x+y*y);
        return as_value(l);
    }

    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Attempt to set read-only property %s"), "Point.length");
    );
    return as_value();
}


/// Static member function.
as_value
point_distance(const fn_call& fn)
{

    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.distance(%s): %s"), ss.str(),
                    _("missing arguments"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 2 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.distance(%s): %s"), ss.str(),
                    _("arguments after first two discarded"));
    }
    );

    const as_value& arg1 = fn.arg(0);
    if ( ! arg1.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.distance(%s): %s"), ss.str(),
                    _("First arg must be an object"));
        );
        return as_value();
    }
    as_object* o1 = toObject(arg1, getVM(fn));
    assert(o1);
    if (!o1->instanceOf(getClassConstructor(fn, "flash.geom.Point")))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.equals(%s): %s %s"), ss.str(),
                    _("First arg must be an instance of"), "flash.geom.Point");
        );
        return as_value();
    }

    const as_value& arg2 = fn.arg(1);
    as_object* o2 = toObject(arg2, getVM(fn));
    assert(o2);
    // it seems there's no need to check arg2 (see actionscript.all/Point.as)

    as_value x1val;
    o1->get_member(NSV::PROP_X, &x1val);
    double x1 = toNumber(x1val, getVM(fn));
    //if ( ! isFinite(x1) ) return as_value(NaN);

    as_value y1val;
    o1->get_member(NSV::PROP_Y, &y1val);
    double y1 = toNumber(y1val, getVM(fn));
    //if ( ! isFinite(y1) ) return as_value(NaN);

    as_value x2val;
    o2->get_member(NSV::PROP_X, &x2val);
    double x2 = toNumber(x2val, getVM(fn));
    //if ( ! isFinite(x2) ) return as_value(NaN);

    as_value y2val;
    o2->get_member(NSV::PROP_Y, &y2val);
    double y2 = toNumber(y2val, getVM(fn));
    //if ( ! utility::isFinite(y2) ) return as_value(NaN);

    double hside = x2 - x1; // p1.x - p0.x;
    double vside = y2 - y1; // p1.y - p0.y;

    double sqdist = hside*hside + vside*vside;
    double dist = std::sqrt(sqdist);

    return as_value(dist);
}

as_value
point_interpolate(const fn_call& fn)
{
    as_value x0val;
    as_value y0val;
    as_value x1val;
    as_value y1val;
    as_value muval;

    if ( fn.nargs < 3 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.interpolate(%s): %s"), ss.str(),
                    _("missing arguments"));
        );
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 3 )
        {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.interpolate(%s): %s"), ss.str(),
                    _("arguments after first three discarded"));
        }
        );

        const as_value& p0val = fn.arg(0);
        as_object* p0 = toObject(p0val, getVM(fn));
        if ( ! p0 )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.interpolate(%s): %s"), ss.str(),
                        _("first argument doesn't cast to object"));
            );
        }
        else
        {
            p0->get_member(NSV::PROP_X, &x0val);
            p0->get_member(NSV::PROP_Y, &y0val);
        }

        const as_value& p1val = fn.arg(1);
        as_object* p1 = toObject(p1val, getVM(fn));
        if ( ! p1 )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.interpolate(%s): %s"), ss.str(),
                        _("second argument doesn't cast to object"));
            );
        }
        else
        {
            p1->get_member(NSV::PROP_X, &x1val);
            p1->get_member(NSV::PROP_Y, &y1val);
        }

        muval = fn.arg(2);
    }


    double x0 = toNumber(x0val, getVM(fn));
    double y0 = toNumber(y0val, getVM(fn));
    double x1 = toNumber(x1val, getVM(fn));
    double y1 = toNumber(y1val, getVM(fn));
    double mu = toNumber(muval, getVM(fn));

    as_value xoff = mu * (x0 - x1);
    as_value yoff = mu * (y0 - y1);

    VM& vm = getVM(fn);
    as_value x = x1val; // copy to avoid changing stack value
    newAdd(x, xoff, vm);
    as_value y = y1val; // copy to avoid changing stack value
    newAdd(y, yoff, vm);

    return constructPoint(fn, x, y);
}

/// Static member function.
as_value
point_polar(const fn_call& fn)
{

    as_value lval; // length
    as_value aval; // angle (radians)

    if ( fn.nargs )
    {
        lval=fn.arg(0);
        if ( fn.nargs > 1 ) aval=fn.arg(1);
        else
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Point.polar(%s): %s"), ss.str(),
                        _("missing arguments"));
            );
        }
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Point.polar(%s): %s"), ss.str(), _("missing arguments"));
        );
    }
    
    double len = toNumber(lval, getVM(fn));
    double angle = toNumber(aval, getVM(fn));

    double x = len * std::cos(angle);
    double y = len * std::sin(angle);

    as_value xval(x);
    as_value yval(y);
    return constructPoint(fn, x, y);
}


as_value
point_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        obj->set_member(NSV::PROP_X, 0.0);
        obj->set_member(NSV::PROP_Y, 0.0);
    }
    else {
        obj->set_member(NSV::PROP_X, fn.arg(0));
        obj->set_member(NSV::PROP_Y, (fn.nargs > 1) ? fn.arg(1) : as_value());
    }
    return as_value(); 
}


as_value
get_flash_geom_point_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.Point class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(&point_ctor, proto);
    attachPointInterface(*proto);
    attachPointStaticProperties(*cl);
    return cl;
}

}

} // end of gnash namespace
