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

/* $Id: Object.cpp,v 1.14 2007/02/07 13:31:26 strk Exp $ */

// Implementation of ActionScript Object class.

#include "tu_config.h"
#include "Object.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "movie_definition.h" // for Object.registerClass (get_exported_resource)
#include "character.h" // for Object.registerClass  (get_root_movie)
#include "sprite_instance.h" // for Object.registerClass  (get_movie_definition)
#include "sprite_definition.h" // for Object.registerClass  (get_movie_definition)
#include "VM.h" // for SWF version (attachObjectInterface)

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

// Forward declarations
static void object_addproperty(const fn_call&);
static void object_registerClass(const fn_call&);


static void
attachObjectInterface(as_object& o)
{
	int target_version = o.getVM().getSWFVersion();

	// FIXME: add Object interface here:
	o.init_member("registerClass", &object_registerClass);
	o.set_member_flags("registerClass", 1); // hidden

	// Object.valueOf()
	o.init_member("valueOf", &as_object::valueof_method);

	// Object.toString()
	o.init_member("toString", &as_object::tostring_method);

	if ( target_version  < 6 ) return;
	o.init_member("addProperty", &object_addproperty);
	o.set_member_flags("addProperty", 1); // hidden
}

as_object*
getObjectInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		attachObjectInterface(*o);
		o->set_prototype(o.get());
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
	global.init_member("Object", cl.get());

}

static void
object_addproperty(const fn_call& fn)
{
	assert(fn.this_ptr);
	as_object* obj = fn.this_ptr;

	if ( fn.nargs != 3 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror("Invalid call to Object.addProperty(%s) - "
			"expected 3 arguments (<name>, <getter>, <setter>).",
		       	ss.str().c_str());
		);

		// if we've been given more args then needed there's
		// no need to abort here
		if ( fn.nargs < 3 )
		{
			fn.result->set_bool(false);
			return;
		}
	}

	std::string propname = fn.arg(0).to_string();
	if ( propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Invalid call to Object.addProperty() - "
			"empty property name");
		);
		fn.result->set_bool(false);
		return;
	}

	as_function* getter = fn.arg(1).to_as_function();
	if ( ! getter )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Invalid call to Object.addProperty() - "
			"getter is not an AS function");
		);
		fn.result->set_bool(false);
		return;
	}

	as_function* setter = fn.arg(2).to_as_function();
	if ( ! setter )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Invalid call to Object.addProperty() - "
			"setter is not an AS function");
		);
		fn.result->set_bool(false);
		return;
	}


	// Now that we checked everything, let's call the as_object
	// interface for getter/setter properties :)
	
	bool result = obj->add_property(propname, *getter, *setter);

	//log_warning("Object.addProperty(): testing");
	fn.result->set_bool(result);
}

static void
object_registerClass(const fn_call& fn)
{
	assert(fn.this_ptr);
	//as_object* obj = fn.this_ptr;

	if ( fn.nargs != 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror("Invalid call to Object.registerClass(%s) - "
			"expected 2 arguments (<symbol>, <constructor>).",
			ss.str().c_str());
		);

		// if we've been given more args then needed there's
		// no need to abort here
		if ( fn.nargs < 2 )
		{
			fn.result->set_bool(false);
			return;
		}
	}

	std::string symbolid = fn.arg(0).to_std_string();
	if ( symbolid.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Invalid call to Object.registerClass() - "
			"empty symbol id");
		);
		fn.result->set_bool(false);
		return;
	}

	as_function* theclass = fn.arg(1).to_as_function();
	if ( ! theclass )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Invalid call to Object.registerClass() - "
			"class is not a function");
		);
		fn.result->set_bool(false);
		return;
	}

	// Find the exported resource

	// TODO: check to *which* definition should we ask the export
	//       this code uses the *relative* root of current environment's target
	movie_definition* def = fn.env->get_target()->get_root_movie()->get_movie_definition();
	boost::intrusive_ptr<resource> exp_res = def->get_exported_resource(symbolid.c_str());
	if ( ! exp_res )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Object.registerClass(%s, %s): "
			"can't find exported symbol",
			symbolid.c_str(), 
			typeid(theclass).name());
		);
		fn.result->set_bool(false);
		return;
	}

	// Check that the exported resource is a sprite_definition
	// (we're looking for a MovieClip symbol)

	boost::intrusive_ptr<sprite_definition> exp_clipdef = 
		boost::intrusive_ptr<sprite_definition>(dynamic_cast<sprite_definition*>(exp_res.get()));


	if ( ! exp_clipdef )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Object.registerClass(%s, %s): "
			"exported symbol is not a MovieClip symbol "
			"(sprite_definition), but a %s",
			symbolid.c_str(), 
			typeid(theclass).name(),
			typeid(*exp_res).name());
		);
		fn.result->set_bool(false);
		return;
	}

	exp_clipdef->registerClass(theclass);
	

	log_warning("Object.registerClass(%s, %s [%p]) TESTING)",
			symbolid.c_str(),
			typeid(theclass).name(),
			(void*)theclass);
	fn.result->set_bool(false);
}
  
} // namespace gnash
