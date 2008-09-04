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

// Forward declarations
static as_value mouse_hide(const fn_call& fn);
static as_value mouse_show(const fn_call& fn);
static void attachMouseInterface(as_object& o);

void registerMouseNative(as_object& o)
{
	VM& vm = o.getVM();

	vm.registerNative(mouse_show, 5, 0);
	vm.registerNative(mouse_hide, 5, 1);
}

static void
attachMouseInterface(as_object& o)
{
	VM& vm = o.getVM();

	o.init_member("show", vm.getNative(5, 0));
	o.init_member("hide", vm.getNative(5, 1));
	
	if (vm.getSWFVersion() > 5)
	{
		AsBroadcaster::initialize(o);
	}
}


as_value
mouse_hide(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    int success = 0;

    movie_root& m = obj->getVM().getRoot();

    success = (m.callInterface("Mouse.hide", "") == "true") ? 1 : 0;

    // returns 1 if mouse was visible before call.
    return as_value(success);
}


as_value
mouse_show(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

    int success = 0;

    movie_root& m = obj->getVM().getRoot();

    success = (m.callInterface("Mouse.show", "") == "true") ? 1 : 0;

    // returns 1 if Mouse was visible before call.
    return as_value(success);
}


// extern (used by Global.cpp)
void
mouse_class_init(as_object& global)
{
	// This is going to be the global Mouse "class"/"function"
	boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
	attachMouseInterface(*obj);

	// Register _global.Mouse
	global.init_member("Mouse", obj.get());

}


} // end of gnash namespace
