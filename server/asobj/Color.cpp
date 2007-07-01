// Color.cpp:  ActionScript class for colors, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "Color.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

as_value color_getrgb(const fn_call& fn);
as_value color_gettransform(const fn_call& fn);
as_value color_setrgb(const fn_call& fn);
as_value color_settransform(const fn_call& fn);
as_value color_ctor(const fn_call& fn);

static void
attachColorInterface(as_object& o)
{
	o.init_member("getRGB", new builtin_function(color_getrgb));
	o.init_member("getTransform", new builtin_function(color_gettransform));
	o.init_member("setRGB", new builtin_function(color_setrgb));
	o.init_member("setTransform", new builtin_function(color_settransform));
}

static as_object*
getColorInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachColorInterface(*o);
	}
	return o.get();
}

class color_as_object: public as_object
{

public:

	color_as_object()
		:
		as_object(getColorInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "Color"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

as_value color_getrgb(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value color_gettransform(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value color_setrgb(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value color_settransform(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value
color_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new color_as_object;
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void color_class_init(as_object& global)
{
	// This is going to be the global Color "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&color_ctor, getColorInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachColorInterface(*cl);
		     
	}

	// Register _global.Color
	global.init_member("Color", cl.get());

}


} // end of gnash namespace
