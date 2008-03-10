// Mouse.cpp:  ActionScript "Mouse" input device class, for Gnash.
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

#include "Mouse.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "VM.h" // for registerNative
#include "Object.h" // for getObjectInterface
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "movie_root.h" // for GUI callback

namespace gnash {

as_value mouse_hide(const fn_call& fn);
as_value mouse_show(const fn_call& fn);
as_value mouse_ctor(const fn_call& fn);

static void
attachMouseInterface(as_object& o)
{
	VM& vm = o.getVM();

	// TODO: Mouse is an object, not a constructor ! Attach these interface to
	//       the singleton Mouse object then !
	vm.registerNative(mouse_show, 5, 0);
	o.init_member("show", vm.getNative(5, 0));

	vm.registerNative(mouse_hide, 5, 1);
	o.init_member("hide", vm.getNative(5, 1));
}

static as_object*
getMouseInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
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
	{
		int swfversion = _vm.getSWFVersion();
		if ( swfversion > 5 )
		{
			AsBroadcaster::initialize(*this);
		}
	}

	// override from as_object ?
	//std::string get_text_value() const { return "Mouse"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }

};

as_value mouse_hide(const fn_call& fn)
{

    boost::intrusive_ptr<mouse_as_object> obj=ensureType<mouse_as_object>(fn.this_ptr);
    UNUSED(obj);

	if (movie_root::interfaceHandle)
	{
		(*movie_root::interfaceHandle)("Mouse.hide", "");
	}
	else
	{
		log_error(_("No callback to handle Mouse.hide"));
	}

	/// Returns nothing
    return as_value();
}

as_value mouse_show(const fn_call& fn)
{
    boost::intrusive_ptr<mouse_as_object> obj=ensureType<mouse_as_object>(fn.this_ptr);
    UNUSED(obj);

	if (movie_root::interfaceHandle)
	{
		(*movie_root::interfaceHandle)("Mouse.show", "");
	}
	else
	{
		log_error(_("No callback to handle Mouse.show"));
	}
	/// Returns nothing
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
	static boost::intrusive_ptr<as_object> obj;

	if ( ! obj )
	{
		obj = new mouse_as_object();
		// we shouldn't keep the Mouse object
		// alive, I think.
		//VM::get().addStatic(obj.get());
	}

	// Register _global.Mouse
	global.init_member("Mouse", obj.get());

}


} // end of gnash namespace
