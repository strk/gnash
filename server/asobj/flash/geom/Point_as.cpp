// Point_as.cpp:  ActionScript "Point" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "Point_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics
#include "utility.h" // isFinite

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

    o.init_member("add", new builtin_function(Point_add), fl);
    o.init_member("clone", new builtin_function(Point_clone), fl);
    o.init_member("equals", new builtin_function(Point_equals), fl);
    o.init_member("normalize", new builtin_function(Point_normalize), fl);
    o.init_member("offset", new builtin_function(Point_offset), fl);
    o.init_member("subtract", new builtin_function(Point_subtract), fl);
    o.init_member("toString", new builtin_function(Point_toString), fl);
    o.init_property("length", Point_length_getset, Point_length_getset, fl);
}

static void
attachPointStaticProperties(as_object& o)
{
   
    o.init_member("distance", new builtin_function(Point_distance));
    o.init_member("interpolate", new builtin_function(Point_interpolate));
    o.init_member("polar", new builtin_function(Point_polar));
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

	// override from as_object ?
	//std::string get_text_value() const { return "Point"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
Point_add(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_clone(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_equals(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_normalize(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_offset(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_subtract(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_toString(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);

	as_value x, y;
	ptr->get_member(NSV::PROP_X, &x);
	ptr->get_member(NSV::PROP_Y, &y);

	std::stringstream ss;
	ss << "(x=" << x.to_string()
		<< ", y=" << y.to_string()
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
		if ( ! utility::isFinite(x) ) return as_value(NAN);
		double y = yval.to_number();
		if ( ! utility::isFinite(y) ) return as_value(NAN);

		double l = sqrt(x*x+y*y);
		return as_value(l);
	}
	else // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Attempt to set read-only property %s"), "Point.length");
		);
		return as_value();
		as_value y;
		ptr->get_member(NSV::PROP_Y, &y);

		as_value bottom = fn.arg(0);
		as_value newh = bottom.subtract(y);
		ptr->set_member(NSV::PROP_HEIGHT, newh);
	}
}

static as_value
Point_distance(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_interpolate(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_polar(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
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
as_function* getFlashGeomPointConstructor()
{
	static builtin_function* cl=NULL;
	if ( ! cl )
	{
		cl=new builtin_function(&Point_ctor, getPointInterface());
		VM::get().addStatic(cl);
		attachPointStaticProperties(*cl);
	}
	return cl;
}

static as_value get_flash_geom_point_constructor(const fn_call& /*fn*/)
{
	log_debug("Loading flash.geom.Point class");

	return getFlashGeomPointConstructor();
}

// extern 
void Point_class_init(as_object& where)
{
	// Register _global.Point
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("Point"), get_flash_geom_point_constructor);
}

} // end of gnash namespace
