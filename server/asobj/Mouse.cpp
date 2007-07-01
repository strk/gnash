// Mouse.cpp:  ActionScript "Mouse" input device class, for Gnash.
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

#include "Mouse.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

as_value mouse_addlistener(const fn_call& fn);
as_value mouse_hide(const fn_call& fn);
as_value mouse_removelistener(const fn_call& fn);
as_value mouse_show(const fn_call& fn);
as_value mouse_ctor(const fn_call& fn);

static void
attachMouseInterface(as_object& o)
{
	o.init_member("addlistener", new builtin_function(mouse_addlistener));
	o.init_member("hide", new builtin_function(mouse_hide));
	o.init_member("removelistener", new builtin_function(mouse_removelistener));
	o.init_member("show", new builtin_function(mouse_show));
}

static as_object*
getMouseInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachMouseInterface(*o);
	}
	return o.get();
}

class mouse_as_object: public as_object
{

public:

	mouse_as_object()
		:
		as_object(getMouseInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "Mouse"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }

};

as_value mouse_addlistener(const fn_call& fn)
{
    boost::intrusive_ptr<mouse_as_object> obj=ensureType<mouse_as_object>(fn.this_ptr);
    UNUSED(obj);

    static bool warned=false;
    if ( ! warned )
    {
        log_unimpl (__FUNCTION__);
        warned=true;
    }
    return as_value();
}

as_value mouse_hide(const fn_call& fn)
{
    boost::intrusive_ptr<mouse_as_object> obj=ensureType<mouse_as_object>(fn.this_ptr);
    UNUSED(obj);

    static bool warned=false;
    if ( ! warned )
    {
        log_unimpl (__FUNCTION__);
        warned=true;
    }
    return as_value();
}

as_value mouse_removelistener(const fn_call& fn)
{
    boost::intrusive_ptr<mouse_as_object> obj=ensureType<mouse_as_object>(fn.this_ptr);
    UNUSED(obj);

    static bool warned=false;
    if ( ! warned )
    {
        log_unimpl (__FUNCTION__);
        warned=true;
    }
    return as_value();
}

as_value mouse_show(const fn_call& fn)
{
    boost::intrusive_ptr<mouse_as_object> obj=ensureType<mouse_as_object>(fn.this_ptr);
    UNUSED(obj);

    static bool warned=false;
    if ( ! warned )
    {
        log_unimpl (__FUNCTION__);
        warned=true;
    }
    return as_value();
}

as_value
mouse_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new mouse_as_object;
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void mouse_class_init(as_object& global)
{
	// This is going to be the global Mouse "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&mouse_ctor, getMouseInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachMouseInterface(*cl);
		     
	}

	// Register _global.Mouse
	global.init_member("Mouse", cl.get());

}


} // end of gnash namespace
