// Point_as.cpp:  ActionScript "Point" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics
#include "GnashNumeric.h"

#include <sstream>

namespace gnash {

static as_value Point_add(const fn_call& fn);
static as_value Point_clone(const fn_call& fn);
static as_value Point_equals(const fn_call& fn);
static as_value Point_normalize(const fn_call& fn);
static as_value Point_offset(const fn_call& fn);
static as_value Point_subtract(const fn_call& fn);
static as_value Point_toString(const fn_call& fn);
static as_value Point_length_getset(const fn_call& fn);

static as_value Point_distance(const fn_call& fn);
static as_value Point_interpolate(const fn_call& fn);
static as_value Point_polar(const fn_call& fn);

as_value Point_ctor(const fn_call& fn);

static void
attachPointInterface(as_object& o)
{
    int fl=0; // flags...

    Global_as* gl = getGlobal(o);
    o.init_member("add", gl->createFunction(Point_add), fl);
    o.init_member("clone", gl->createFunction(Point_clone), fl);
    o.init_member("equals", gl->createFunction(Point_equals), fl);
    o.init_member("normalize", gl->createFunction(Point_normalize), fl);
    o.init_member("offset", gl->createFunction(Point_offset), fl);
    o.init_member("subtract", gl->createFunction(Point_subtract), fl);
    o.init_member("toString", gl->createFunction(Point_toString), fl);
    o.init_property("length", Point_length_getset, Point_length_getset, fl);
}

static void
attachPointStaticProperties(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("distance", gl->createFunction(Point_distance), 0);
    o.init_member("interpolate", gl->createFunction(Point_interpolate), 0);
    o.init_member("polar", gl->createFunction(Point_polar), 0);
}

static as_object*
getPointInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachPointInterface(*o);

	}

	return o.get();
}


class Point_as: public as_object
{
public:
	Point_as()
	    :
	    as_object(getPointInterface())
	{}
	    
};


static as_value
Point_add(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

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
			log_aserror("Point.add(%s): %s", ss.str(), _("arguments after first discarded"));
		}
		);
		const as_value& arg1 = fn.arg(0);
		as_object* o = arg1.to_object(*getGlobal(fn)).get();
		if ( ! o )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Point.add(%s): %s", ss.str(), _("first argument doesn't cast to object"));
			);
		}
		else
		{
			if ( ! o->get_member(NSV::PROP_X, &x1) )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror("Point.add(%s): %s", ss.str(),
					_("first argument cast to object doesn't contain an 'x' member"));
				);
			}
			if ( ! o->get_member(NSV::PROP_Y, &y1) )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror("Point.add(%s): %s", ss.str(),
					_("first argument cast to object doesn't contain an 'y' member"));
				);
			}
		}
	}

	x.newAdd(x1);
	y.newAdd(y1);

	boost::intrusive_ptr<as_object> ret = new Point_as;
	ret->set_member(NSV::PROP_X, x);
	ret->set_member(NSV::PROP_Y, y);

	return as_value(ret.get());
}

static as_value
Point_clone(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

	as_value x, y;
	ptr->get_member(NSV::PROP_X, &x);
	ptr->get_member(NSV::PROP_Y, &y);

	boost::intrusive_ptr<as_object> ret = new Point_as;
	ret->set_member(NSV::PROP_X, x);
	ret->set_member(NSV::PROP_Y, y);

	return as_value(ret.get());
}

static as_value
Point_equals(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

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
		log_aserror("Point.equals(%s): %s", ss.str(),
            _("First arg must be an object"));
		);
		return as_value(false);
	}
	as_object* o = arg1.to_object(*getGlobal(fn)).get();
	assert(o);
	if ( ! o->instanceOf(getFlashGeomPointConstructor(fn)) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.equals(%s): %s %s", ss.str(),
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

	return as_value(x.equals(x1) && y.equals(y1));
}

static as_value
Point_normalize(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);

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
			log_aserror("Point.normalize(%s): %s", ss.str(), _("arguments after first discarded"));
		}
		);

		argval = fn.arg(0);
	}

	// newlen may be NaN, and we'd still be updating x/y
	// see actionscript.all/Point.as
	double newlen = argval.to_number();

	as_value xval, yval;
	ptr->get_member(NSV::PROP_X, &xval);
	ptr->get_member(NSV::PROP_Y, &yval);

	double x = xval.to_number();
	if (!isFinite(x)) return as_value();
	double y = yval.to_number();
	if (!isFinite(y)) return as_value();

	if ( x == 0 && y == 0 ) return as_value();

	double curlen = std::sqrt(x*x+y*y);
	double fact = newlen/curlen;


	xval.set_double( xval.to_number() * fact );
	yval.set_double( yval.to_number() * fact );
	ptr->set_member(NSV::PROP_X, xval);
	ptr->set_member(NSV::PROP_Y, yval);

	return as_value();
}

static as_value
Point_offset(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);

	as_value x, y;
	ptr->get_member(NSV::PROP_X, &x);
	ptr->get_member(NSV::PROP_Y, &y);

	as_value xoff, yoff;

	if ( fn.nargs ) {
		xoff = fn.arg(0);
		if ( fn.nargs > 1 ) yoff = fn.arg(1);
	}

	x.newAdd(xoff);
	y.newAdd(yoff);

	ptr->set_member(NSV::PROP_X, x);
	ptr->set_member(NSV::PROP_Y, y);

	return as_value();
}

static as_value
Point_subtract(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);

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
			log_aserror("Point.add(%s): %s", ss.str(), _("arguments after first discarded"));
		}
		);
		const as_value& arg1 = fn.arg(0);
		as_object* o = arg1.to_object(*getGlobal(fn)).get();
		if ( ! o )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Point.add(%s): %s", ss.str(), _("first argument doesn't cast to object"));
			);
		}
		else
		{
			if ( ! o->get_member(NSV::PROP_X, &x1) )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror("Point.add(%s): %s", ss.str(),
					_("first argument casted to object doesn't contain an 'x' member"));
				);
			}
			if ( ! o->get_member(NSV::PROP_Y, &y1) )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror("Point.add(%s): %s", ss.str(),
					_("first argument casted to object doesn't contain an 'y' member"));
				);
			}
		}
	}

	x.set_double(x.to_number() - x1.to_number());
	y.set_double(y.to_number() - y1.to_number());

	boost::intrusive_ptr<as_object> ret = new Point_as;
	ret->set_member(NSV::PROP_X, x);
	ret->set_member(NSV::PROP_Y, y);

	return as_value(ret.get());
}

static as_value
Point_toString(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);

	as_value x, y;
	ptr->get_member(NSV::PROP_X, &x);
	ptr->get_member(NSV::PROP_Y, &y);

    int version = getSWFVersion(fn);

	std::stringstream ss;
	ss << "(x=" << x.to_string_versioned(version)
		<< ", y=" << y.to_string_versioned(version)
		<< ")";

	return as_value(ss.str());
}

static as_value
Point_length_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);

	if ( ! fn.nargs ) // getter
	{
		as_value xval, yval;
		ptr->get_member(NSV::PROP_X, &xval);
		ptr->get_member(NSV::PROP_Y, &yval);
		double x = xval.to_number();
		double y = yval.to_number();

		double l = std::sqrt(x*x+y*y);
		return as_value(l);
	}
	else // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Attempt to set read-only property %s"), "Point.length");
		);
		return as_value();
	}
}

static as_value
Point_distance(const fn_call& fn)
{
	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.distance(%s): %s", ss.str(), _("missing arguments"));
		);
		return as_value();
	}

	IF_VERBOSE_ASCODING_ERRORS(
	if ( fn.nargs > 2 )
	{
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.distance(%s): %s", ss.str(), _("arguments after first two discarded"));
	}
	);

	const as_value& arg1 = fn.arg(0);
	if ( ! arg1.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.distance(%s): %s", ss.str(), _("First arg must be an object"));
		);
		return as_value();
	}
	as_object* o1 = arg1.to_object(*getGlobal(fn)).get();
	assert(o1);
	if ( ! o1->instanceOf(getFlashGeomPointConstructor(fn)) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.equals(%s): %s %s", ss.str(), _("First arg must be an instance of"), "flash.geom.Point");
		);
		return as_value();
	}

	const as_value& arg2 = fn.arg(1);
	as_object* o2 = arg2.to_object(*getGlobal(fn)).get();
	assert(o2);
	// it seems there's no need to check arg2 (see actionscript.all/Point.as)

	as_value x1val;
	o1->get_member(NSV::PROP_X, &x1val);
	double x1 = x1val.to_number();
	//if ( ! isFinite(x1) ) return as_value(NaN);

	as_value y1val;
	o1->get_member(NSV::PROP_Y, &y1val);
	double y1 = y1val.to_number();
	//if ( ! isFinite(y1) ) return as_value(NaN);

	as_value x2val;
	o2->get_member(NSV::PROP_X, &x2val);
	double x2 = x2val.to_number();
	//if ( ! isFinite(x2) ) return as_value(NaN);

	as_value y2val;
	o2->get_member(NSV::PROP_Y, &y2val);
	double y2 = y2val.to_number();
	//if ( ! utility::isFinite(y2) ) return as_value(NaN);

	double hside = x2 - x1; // p1.x - p0.x;
	double vside = y2 - y1; // p1.y - p0.y;

	double sqdist = hside*hside + vside*vside;
	double dist = std::sqrt(sqdist);

	return as_value(dist);
}

static as_value
Point_interpolate(const fn_call& fn)
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
		log_aserror("Point.interpolate(%s): %s", ss.str(), _("missing arguments"));
		);
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		if ( fn.nargs > 3 )
		{
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.interpolate(%s): %s", ss.str(), _("arguments after first three discarded"));
		}
		);

		const as_value& p0val = fn.arg(0);
		as_object* p0 = p0val.to_object(*getGlobal(fn)).get();
		if ( ! p0 )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Point.interpolate(%s): %s", ss.str(), _("first argument doesn't cast to object"));
			);
		}
		else
		{
			p0->get_member(NSV::PROP_X, &x0val);
			p0->get_member(NSV::PROP_Y, &y0val);
		}

		const as_value& p1val = fn.arg(1);
		as_object* p1 = p1val.to_object(*getGlobal(fn)).get();
		if ( ! p1 )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Point.interpolate(%s): %s", ss.str(), _("second argument doesn't cast to object"));
			);
		}
		else
		{
			p1->get_member(NSV::PROP_X, &x1val);
			p1->get_member(NSV::PROP_Y, &y1val);
		}

		muval = fn.arg(2);
	}


	double x0 = x0val.to_number();
	double y0 = y0val.to_number();
	double x1 = x1val.to_number();
	double y1 = y1val.to_number();
	double mu = muval.to_number();

	// newX = b.x + ( muval * (a.x - b.x) );
	// newY = b.y + ( muval * (a.y - b.y) );

	as_value xoff = mu * (x0 - x1);
	as_value yoff = mu * (y0 - y1);

	//log_debug("xoff:%s, yoff:%s, x1val:%s, y1val:%s", xoff, yoff, x1val, y1val);

	as_value x = x1val; // copy to avoid changing stack value
	x.newAdd(xoff);
	as_value y = y1val; // copy to avoid changing stack value
	y.newAdd(yoff);

	boost::intrusive_ptr<as_object> ret = new Point_as;
	ret->set_member(NSV::PROP_X, as_value(x));
	ret->set_member(NSV::PROP_Y, as_value(y));

	return as_value(ret.get());
}

static as_value
Point_polar(const fn_call& fn)
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
			log_aserror("Point.polar(%s): %s", ss.str(), _("missing arguments"));
			);
		}
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Point.polar(%s): %s", ss.str(), _("missing arguments"));
		);
	}
	
	double len = lval.to_number();
	double angle = aval.to_number();

	double x = len * std::cos(angle);
	double y = len * std::sin(angle);

	as_value xval(x);
	as_value yval(y);
	boost::intrusive_ptr<as_object> obj = new Point_as;

	obj->set_member(NSV::PROP_X, x);
	obj->set_member(NSV::PROP_Y, y);
	
	return as_value(obj.get());
}


as_value
Point_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new Point_as;

	as_value x;
	as_value y;

	if ( ! fn.nargs )
	{
		x.set_double(0);
		y.set_double(0);
	}
	else
	{
		do {
			x = fn.arg(0);
			if ( fn.nargs < 2 ) break;
			y = fn.arg(1);
			if ( fn.nargs < 3 ) break;
			IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss;
				fn.dump_args(ss);
				log_aserror("flash.geom.Point(%s): %s", ss.str(), _("arguments after the first two discarded"));
			);
		} while(0);
	}

	obj->set_member(NSV::PROP_X, x);
	obj->set_member(NSV::PROP_Y, y);

	return as_value(obj.get()); // will keep alive
}

// extern 
as_function*
getFlashGeomPointConstructor(const fn_call& fn)
{
    as_value point(fn.env().find_object("flash.geom.Point"));
    return point.to_as_function();
}

static
as_value get_flash_geom_point_constructor(const fn_call& fn)
{
	log_debug("Loading flash.geom.Point class");
    Global_as* gl = getGlobal(fn);
    as_object* cl = gl->createClass(&Point_ctor, getPointInterface());
    attachPointStaticProperties(*cl);
    return cl;
}

boost::intrusive_ptr<as_object> init_Point_instance()
{
    return boost::intrusive_ptr<as_object>(new Point_as);
}

// extern 
void
point_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
	where.init_destructive_property(getName(uri),
            get_flash_geom_point_constructor, flags, getNamespace(uri));
}

} // end of gnash namespace
