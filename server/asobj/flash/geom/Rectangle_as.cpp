// Rectangle_as.cpp:  ActionScript "Rectangle" class, for Gnash.
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

#include "Rectangle_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance

#include <sstream>

namespace gnash {

static as_value Rectangle_clone(const fn_call& fn);
static as_value Rectangle_contains(const fn_call& fn);
static as_value Rectangle_containsPoint(const fn_call& fn);
static as_value Rectangle_containsRectangle(const fn_call& fn);
static as_value Rectangle_equals(const fn_call& fn);
static as_value Rectangle_inflate(const fn_call& fn);
static as_value Rectangle_inflatePoint(const fn_call& fn);
static as_value Rectangle_intersection(const fn_call& fn);
static as_value Rectangle_intersects(const fn_call& fn);
static as_value Rectangle_isEmpty(const fn_call& fn);
static as_value Rectangle_offset(const fn_call& fn);
static as_value Rectangle_offsetPoint(const fn_call& fn);
static as_value Rectangle_setEmpty(const fn_call& fn);
static as_value Rectangle_toString(const fn_call& fn);
static as_value Rectangle_union(const fn_call& fn);
as_value Rectangle_ctor(const fn_call& fn);

static void
attachRectangleInterface(as_object& o)
{
    o.init_member("clone", new builtin_function(Rectangle_clone));
    o.init_member("contains", new builtin_function(Rectangle_contains));
    o.init_member("containsPoint", new builtin_function(Rectangle_containsPoint));
    o.init_member("containsRectangle", new builtin_function(Rectangle_containsRectangle));
    o.init_member("equals", new builtin_function(Rectangle_equals));
    o.init_member("inflate", new builtin_function(Rectangle_inflate));
    o.init_member("inflatePoint", new builtin_function(Rectangle_inflatePoint));
    o.init_member("intersection", new builtin_function(Rectangle_intersection));
    o.init_member("intersects", new builtin_function(Rectangle_intersects));
    o.init_member("isEmpty", new builtin_function(Rectangle_isEmpty));
    o.init_member("offset", new builtin_function(Rectangle_offset));
    o.init_member("offsetPoint", new builtin_function(Rectangle_offsetPoint));
    o.init_member("setEmpty", new builtin_function(Rectangle_setEmpty));
    o.init_member("toString", new builtin_function(Rectangle_toString));
    o.init_member("union", new builtin_function(Rectangle_union));
}

static as_object*
getRectangleInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		attachRectangleInterface(*o);
	}
	return o.get();
}

class Rectangle_as: public as_object
{

public:

	Rectangle_as()
		:
		as_object(getRectangleInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "Rectangle"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
Rectangle_clone(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_contains(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_containsPoint(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_containsRectangle(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_equals(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_inflate(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_inflatePoint(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_intersection(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_intersects(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_isEmpty(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_offset(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_offsetPoint(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_setEmpty(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_toString(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Rectangle_union(const fn_call& fn)
{
	boost::intrusive_ptr<Rectangle_as> ptr = ensureType<Rectangle_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
Rectangle_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new Rectangle_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("Rectangle(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void Rectangle_class_init(as_object& global)
{
	// This is going to be the global Rectangle "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&Rectangle_ctor, getRectangleInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachRectangleInterface(*cl);
	}

	// Register _global.Rectangle
	global.init_member("Rectangle", cl.get());
}

} // end of gnash namespace
