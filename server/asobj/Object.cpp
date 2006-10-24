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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

/* $Id: Object.cpp,v 1.3 2006/10/24 15:51:47 strk Exp $ */

// Implementation of ActionScript Object class.

#include "tu_config.h"
#include "Object.h"
//#include "smart_ptr.h"
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
}

static as_object*
getObjectInterface()
{
	static as_object* o=NULL;
	if ( o == NULL )
	{
		o = new as_object();
		attachObjectInterface(*o);
	}
	return o;
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

	as_object* new_obj;
	if ( fn.nargs == 0 )
	{
		new_obj = new object_as_object();
	}
	else
	{
		log_error("Too many args to Object constructor");
		new_obj = new object_as_object();
	}

	fn.result->set_as_object(new_obj);
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
	static builtin_function* cl=NULL;

	if ( cl == NULL )
	{
		cl=new builtin_function(&object_ctor, getObjectInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachObjectInterface(*cl);
		     
	}

	// Register _global.Object
	global.set_member("Object", cl);

}

void
object_addproperty(const fn_call& fn)
{
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

	as_function* setter = fn.arg(1).to_as_function();
	if ( ! setter )
	{
		log_warning("Invalid call to Object.addProperty() - "
			"setter is not an AS function");
		fn.result->set_bool(false);
		return;
	}

	as_function* getter = fn.arg(2).to_as_function();
	if ( ! getter )
	{
		log_warning("Invalid call to Object.addProperty() - "
			"getter is not an AS function");
		fn.result->set_bool(false);
		return;
	}


	// Now, we'd need a new interface of as_object to
	// actually register the new property...
	// TODO: add it to as_object class

	log_error("Object.addProperty(): unimplemented");
	fn.result->set_bool(false);
}
  
} // namespace gnash
