// EventDispatcher.cpp:  Implementation of ActionScript DisplayObjectContainer class, for Gnash.
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

#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "flash/display/InteractiveObject_as.h"
#include "flash/display/DisplayObjectContainer_as.h"

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

static as_value DisplayObjectContainer_addChild(const fn_call& fn);

static void
attachDisplayObjectContainerInterface(as_object& o)
{
	 o.init_member("addChild", new builtin_function(DisplayObjectContainer_addChild));
}


display_object_container_as_object::display_object_container_as_object()
	:
	as_object()
{
}

as_value
display_object_container_as_object::addChild(as_value child){

		return child;
}

static as_value DisplayObjectContainer_addChild(const fn_call& fn){
	
	boost::intrusive_ptr<display_object_container_as_object> ptr = ensureType<display_object_container_as_object>(fn.this_ptr);

	return ptr->addChild(fn.arg(0));

}

static as_value
display_object_container_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new display_object_container_as_object();
	
	return as_value(obj.get()); // will keep alive
}

as_object*
getDisplayObjectContainerInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getInteractiveObjectInterface());
	}
	attachDisplayObjectContainerInterface(*o);
	o.get()->dump_members();
	return o.get();
}

// extern
void display_object_container_class_init(as_object& where)
{
    static boost::intrusive_ptr<builtin_function> cl;

	cl=new builtin_function(&display_object_container_ctor, getDisplayObjectContainerInterface());

	where.init_member("DisplayObjectContainer", cl.get());
}

std::auto_ptr<as_object>
init_display_object_container_instance()
{
	return std::auto_ptr<as_object>(new display_object_container_as_object);
}


}
