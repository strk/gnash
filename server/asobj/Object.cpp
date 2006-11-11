// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

/* $Id: Object.cpp,v 1.8 2006/11/11 22:44:54 strk Exp $ */

// Implementation of ActionScript Object class.

#include "tu_config.h"
#include "Object.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

// Forward declarations
void object_addproperty(const fn_call&);


static void
attachObjectInterface(as_object& o)
{
	// FIXME: add Object interface here:
	o.set_member("addProperty", &object_addproperty);
	o.set_member_flags("addProperty", 1); // hidden
}

static as_object*
getObjectInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		attachObjectInterface(*o);
	}
	return o.get();
}

// FIXME: add some useful methods :)
class object_as_object : public as_object
{

public:

	object_as_object()
		:
		as_object(getObjectInterface())
	{
	}

};

static void
object_ctor(const fn_call& fn)
    // Constructor for ActionScript class Object.
{
	if ( fn.nargs == 1 ) // copy constructor
	{
		// just copy the reference
		//
		// WARNING: it is likely that fn.result and fn.arg(0)
		// are the same location... so we might skip
		// the set_as_object() call as a whole.
		fn.result->set_as_object(fn.arg(0).to_object());
		return;
	}

	boost::intrusive_ptr<as_object> new_obj;
	if ( fn.nargs == 0 )
	{
		new_obj = new object_as_object();
	}
	else
	{
		log_error("Too many args to Object constructor");
		new_obj = new object_as_object();
	}

	fn.result->set_as_object(new_obj.get()); // will keep alive
}

std::auto_ptr<as_object>
init_object_instance()
{
	return std::auto_ptr<as_object>(new object_as_object);
}


// extern (used by Global.cpp)
void object_class_init(as_object& global)
{
	// This is going to be the global Object "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl=NULL;

	if ( cl == NULL )
	{
		cl=new builtin_function(&object_ctor, getObjectInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachObjectInterface(*cl);
		     
	}

	// Register _global.Object
	global.set_member("Object", cl.get());

}

void
object_addproperty(const fn_call& fn)
{
	assert(fn.this_ptr);
	as_object* obj = fn.this_ptr;

	if ( fn.nargs != 3 )
	{
		log_warning("Invalid call to Object.addProperty() - "
			"wrong number of args: %d, expected 3 "
			"(property name, getter function, setter function)",
			fn.nargs);
		fn.result->set_bool(false);
		return;
	}

	std::string propname = fn.arg(0).to_string();
	if ( propname.empty() )
	{
		log_warning("Invalid call to Object.addProperty() - "
			"empty property name");
		fn.result->set_bool(false);
		return;
	}

	as_function* getter = fn.arg(1).to_as_function();
	if ( ! getter )
	{
		log_warning("Invalid call to Object.addProperty() - "
			"getter is not an AS function");
		fn.result->set_bool(false);
		return;
	}

	as_function* setter = fn.arg(2).to_as_function();
	if ( ! setter )
	{
		log_warning("Invalid call to Object.addProperty() - "
			"setter is not an AS function");
		fn.result->set_bool(false);
		return;
	}


	// Now that we checked everything, let's call the as_object
	// interface for getter/setter properties :)
	
	bool result = obj->add_property(propname, *getter, *setter);

	//log_warning("Object.addProperty(): testing");
	fn.result->set_bool(result);
}
  
} // namespace gnash
