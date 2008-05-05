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
static as_value Point_x_getset(const fn_call& fn);
static as_value Point_y_getset(const fn_call& fn);

static as_value Point_distance(const fn_call& fn);
static as_value Point_interpolate(const fn_call& fn);
static as_value Point_polar(const fn_call& fn);

as_value Point_ctor(const fn_call& fn);

static void
attachPointInterface(as_object& o)
{
    o.init_member("add", new builtin_function(Point_add));
    o.init_member("clone", new builtin_function(Point_clone));
    o.init_member("equals", new builtin_function(Point_equals));
    o.init_member("normalize", new builtin_function(Point_normalize));
    o.init_member("offset", new builtin_function(Point_offset));
    o.init_member("subtract", new builtin_function(Point_subtract));
    o.init_member("toString", new builtin_function(Point_toString));
    o.init_property("length", Point_length_getset, Point_length_getset);
    o.init_property("x", Point_x_getset, Point_x_getset);
    o.init_property("y", Point_y_getset, Point_y_getset);
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
	boost::intrusive_ptr<as_object> o;
	// TODO: check if this class should inherit from Object
	//       or from a different class
	o = new as_object(getObjectInterface());
	attachPointInterface(*o);
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
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_length_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_x_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Point_y_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Point_as> ptr = ensureType<Point_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
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

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("Point(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern 
void Point_class_init(as_object& where)
{
	// This is going to be the Point "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&Point_ctor, getPointInterface());
	attachPointStaticProperties(*cl);

	// Register _global.Point
	where.init_member("Point", cl.get());
}

} // end of gnash namespace
