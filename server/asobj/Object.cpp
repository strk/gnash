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

/* $Id: Object.cpp,v 1.1 2006/10/24 14:36:25 strk Exp $ */

// Implementation of ActionScript Object class.

#include "tu_config.h"
#include "Object.h"
//#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function

#include "log.h"

#include <sstream>

namespace gnash {

// Forward declarations

static void
attachObjectInterface(as_object& /*o*/)
{
	// FIXME: add Object interface here:
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
		new_obj = new as_object();
	}
	else
	{
		log_error("Too many args to Object constructor");
		new_obj = new as_object();
	}

	fn.result->set_as_object(new_obj);
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
  
} // namespace gnash
