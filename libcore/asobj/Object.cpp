// Object.cpp:  Implementation of ActionScript Object class, for Gnash.
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

#include "Object.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "movie_definition.h" // for Object.registerClass (get_exported_resource)
#include "sprite_definition.h" // for Object.registerClass  (get_movie_definition)
#include "VM.h" // for SWF version (attachObjectInterface)
#include "namedStrings.h" // for NSV::PROP_TO_STRING

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

// Forward declarations
static as_value object_addproperty(const fn_call&);
static as_value object_registerClass(const fn_call& fn);
static as_value object_hasOwnProperty(const fn_call&);
static as_value object_isPropertyEnumerable(const fn_call&);
static as_value object_isPrototypeOf(const fn_call&);
static as_value object_watch(const fn_call&);
static as_value object_unwatch(const fn_call&);
static as_value object_toLocaleString(const fn_call&);


static void
attachObjectInterface(as_object& o)
{
	VM& vm = o.getVM();

	// We register natives despite swf version,

	vm.registerNative(object_watch, 101, 0); 
	vm.registerNative(object_unwatch, 101, 1); 
	vm.registerNative(object_addproperty, 101, 2); 
	vm.registerNative(as_object::valueof_method, 101, 3); 
	vm.registerNative(as_object::tostring_method, 101, 4); 
	vm.registerNative(object_hasOwnProperty, 101, 5); 
	vm.registerNative(object_isPrototypeOf, 101, 6); 
	vm.registerNative(object_isPropertyEnumerable, 101, 7); 

	// Then will attach to the prototype based on version

	//int target_version = vm.getSWFVersion();

	// Object.valueOf()
	o.init_member("valueOf", vm.getNative(101, 3));

	// Object.toString()
	o.init_member("toString", vm.getNative(101, 4));

	// Object.toLocaleString()
	o.init_member("toLocaleString", new builtin_function(object_toLocaleString));

	int swf6flags = as_prop_flags::dontEnum|as_prop_flags::dontDelete|as_prop_flags::onlySWF6Up;
	//if ( target_version  < 6 ) return;

	// Object.addProperty()
	o.init_member("addProperty", vm.getNative(101, 2), swf6flags);

	// Object.hasOwnProperty()
	o.init_member("hasOwnProperty", vm.getNative(101, 5), swf6flags);

	// Object.isPropertyEnumerable()
	o.init_member("isPropertyEnumerable", vm.getNative(101, 7), swf6flags);

	// Object.isPrototypeOf()
	o.init_member("isPrototypeOf", vm.getNative(101, 6), swf6flags);

	// Object.watch()
	o.init_member("watch", vm.getNative(101, 0), swf6flags);

	// Object.unwatch()
	o.init_member("unwatch", vm.getNative(101, 1), swf6flags);
}

as_object*
getObjectInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(); // end of the inheritance chain
		attachObjectInterface(*o);
		//o->set_prototype(o.get()); // proto is self ?
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

static as_value
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
		return as_value(fn.arg(0).to_object());
	}

	boost::intrusive_ptr<as_object> new_obj;
	if ( fn.nargs == 0 )
	{
		new_obj = new object_as_object();
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS (
		log_aserror(_("Too many args to Object constructor"));
		)
		new_obj = new object_as_object();
	}

	return as_value(new_obj.get()); // will keep alive
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

	VM& vm = global.getVM();

	if ( cl == NULL )
	{
		cl=new builtin_function(&object_ctor, getObjectInterface());

		// TODO: is this needed ?
		//cl->init_member("prototype", as_value(getObjectInterface()));

		// Object.registerClass() -- TODO: should this only be in SWF6 or higher ?
		vm.registerNative(object_registerClass, 101, 8);
		cl->init_member("registerClass", vm.getNative(101, 8));

		     
	}

	// Register _global.Object
	global.init_member("Object", cl.get());

}

static as_value
object_addproperty(const fn_call& fn)
{
	assert(fn.this_ptr);
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	if ( fn.nargs != 3 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.addProperty(%s) - "
			"expected 3 arguments (<name>, <getter>, <setter>)"),
		       	ss.str());
		);

		// if we've been given more args then needed there's
		// no need to abort here
		if ( fn.nargs < 3 )
		{
			return as_value(false);
		}
	}

	const std::string& propname = fn.arg(0).to_string();
	if ( propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.addProperty() - "
			"empty property name"));
		);
		return as_value(false);
	}

	as_function* getter = fn.arg(1).to_as_function();
	if ( ! getter )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.addProperty() - "
			"getter is not an AS function"));
		);
		return as_value(false);
	}

	as_function* setter = NULL;
	const as_value& setterval = fn.arg(2);
	if ( ! setterval.is_null() )
	{
		setter = setterval.to_as_function();
		if ( ! setter )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Invalid call to Object.addProperty() - "
				"setter is not null and not an AS function (%s)"),
				setterval);
			);
			return as_value(false);
		}
	}


	// Now that we checked everything, let's call the as_object
	// interface for getter/setter properties :)
	
	bool result = obj->add_property(propname, *getter, setter);

	//log_debug("Object.addProperty(): testing");
	return as_value(result);
}

static as_value
object_registerClass(const fn_call& fn)
{
	assert(fn.this_ptr);

	if ( fn.nargs != 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.registerClass(%s) - "
			"expected 2 arguments (<symbol>, <constructor>)"),
			ss.str());
		);

		// if we've been given more args then needed there's
		// no need to abort here
		if ( fn.nargs < 2 )
		{
			return as_value(false);
		}
	}

	const std::string& symbolid = fn.arg(0).to_string();
	if ( symbolid.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.registerClass(%s) - "
			"first argument (symbol id) evaluates to empty string"), ss.str());
		);
		return as_value(false);
	}

	as_function* theclass = fn.arg(1).to_as_function();
	if ( ! theclass )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.registerClass(%s) - "
			"second argument (class) is not a function)"), ss.str());
		);
		return as_value(false);
	}

	// Find the exported resource

	// TODO: check to *which* definition should we ask the export
	//       this code uses the *relative* root of current environment's target
	//       and don't use VM::get() if this code is ever reactivated.
#if 0
	movie_definition* def = VM::get().getRoot().get_movie_definition();
#else
	// Using definition of current target fixes the youtube beta case
	// https://savannah.gnu.org/bugs/index.php?23130
	character* tgt = fn.env().get_target();
	if ( ! tgt ) {
		log_error("current environment has no target, wouldn't know where to look for symbol required for registerClass"); 
		return as_value(false);
	}
	movie_instance* relRoot = tgt->get_root();
	assert(relRoot);
	movie_definition* def = relRoot->get_movie_definition();
#endif
	boost::intrusive_ptr<resource> exp_res = def->get_exported_resource(symbolid);
	if ( ! exp_res )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.registerClass(%s, %s): "
			"can't find exported symbol"),
			symbolid, 
			typeid(theclass).name());
		);
		return as_value(false);
	}

	// Check that the exported resource is a sprite_definition
	// (we're looking for a MovieClip symbol)

	boost::intrusive_ptr<sprite_definition> exp_clipdef = 
		boost::intrusive_ptr<sprite_definition>(dynamic_cast<sprite_definition*>(exp_res.get()));


	if ( ! exp_clipdef )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.registerClass(%s, %s): "
			"exported symbol is not a MovieClip symbol "
			"(sprite_definition), but a %s"),
			symbolid, 
			typeid(theclass).name(),
			typeid(*exp_res).name());
		);
		return as_value(false);
	}

	exp_clipdef->registerClass(theclass);
	return as_value(true);
}

as_value
object_hasOwnProperty(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

	//assert(fn.result->is_undefined());
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.hasOwnProperty() requires one arg"));
		);
		return as_value(false);
	}
	const as_value& arg = fn.arg(0);
	const std::string& propname = arg.to_string();
	if ( arg.is_undefined() || propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.hasOwnProperty('%s')"), arg);
		);
		return as_value(false);
	}
	//log_debug("%p.hasOwnProperty", fn.this_ptr);
	return as_value(fn.this_ptr->hasOwnProperty(obj->getVM().getStringTable().find(propname)));
}

as_value
object_isPropertyEnumerable(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

	//assert(fn.result->is_undefined());
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.isPropertyEnumerable() requires one arg"));
		);
		return as_value();
	}
	const as_value& arg = fn.arg(0);
	const std::string& propname = arg.to_string();
	if ( arg.is_undefined() || propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.isPropertyEnumerable('%s')"), arg);
		);
		return as_value();
	}

	Property* prop = fn.this_ptr->getOwnProperty(obj->getVM().getStringTable().find(propname));
	if ( ! prop )
	{
		return as_value(false);
	}

	return as_value( ! prop->getFlags().get_dont_enum() );
}

as_value
object_isPrototypeOf(const fn_call& fn)
{
	//assert(fn.result->is_undefined());
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.isPrototypeOf() requires one arg"));
		);
		return as_value(false); 
	}

	boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
	if ( ! obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("First arg to Object.isPrototypeOf(%s) is not an object"), fn.arg(0));
		);
		return as_value(false);
	}

	return as_value(fn.this_ptr->prototypeOf(*obj));

}

as_value
object_watch(const fn_call& fn)
{
	as_object* obj = fn.this_ptr.get();

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Object.watch(%s): missing arguments"));
		);
		return as_value(false);
	}

	const as_value& propval = fn.arg(0);
	const as_value& funcval = fn.arg(1);

	if ( ! funcval.is_function() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Object.watch(%s): second argument is not a function"));
		);
		return as_value(false);
	}

	VM& vm = obj->getVM();
	string_table& st = vm.getStringTable();

	std::string propname = propval.to_string();
	string_table::key propkey = st.find(propname);
	as_function* trig = funcval.to_as_function();
	as_value cust; if ( fn.nargs > 2 ) cust = fn.arg(2);

	return as_value(obj->watch(propkey, *trig, cust));
}

as_value
object_unwatch(const fn_call& fn)
{
	as_object* obj = fn.this_ptr.get();

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Object.unwatch(%s): missing argument"));
		);
		return as_value(false);
	}

	const as_value& propval = fn.arg(0);

	VM& vm = obj->getVM();
	string_table& st = vm.getStringTable();

	std::string propname = propval.to_string();
	string_table::key propkey = st.find(propname);

	return as_value(obj->unwatch(propkey));
}

as_value
object_toLocaleString(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;
	return obj->callMethod(NSV::PROP_TO_STRING);
}
  
} // namespace gnash
