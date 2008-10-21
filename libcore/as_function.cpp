// as_function.cpp:  ActionScript Functions, for Gnash.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "log.h"
#include "as_function.h"
#include "builtin_function.h" // for _global.Function
#include "as_value.h"
#include "array.h"
#include "fn_call.h"
#include "GnashException.h"
#include "VM.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"

#include <typeinfo>
#include <iostream>

namespace gnash {

// should be static, probably
as_value function_apply(const fn_call& fn);
as_value function_call(const fn_call& fn);
static as_object* getFunctionPrototype();
static as_value function_ctor(const fn_call& fn);

/* 
 * This function returns the singleton
 * instance of the ActionScript Function object
 * prototype, which is what the AS Function class
 * exports, thus what each AS function instance inherit.
 *
 * The returned object can be accessed by ActionScript
 * code through Function.__proto__.prototype.
 * User AS code can add or modify members of this object
 * to modify behaviour of all Function AS instances.
 *
 * FIXME: do not use a static specifier for the proto
 * object, as multiple runs of a single movie should
 * each use a 'clean', unmodified, version of the
 * prototype. What should really happen is that this
 * prototype gets initializated by initialization of
 * the Function class itself, which would be a member
 * of the _global object for each movie instance.
 * 
 */
static as_object* getFunctionPrototype()
{
	// Make sure the prototype is always
	// alive (static boost::intrusive_ptr<> should ensure this)
	static boost::intrusive_ptr<as_object> proto;

	if ( proto.get() == NULL ) {

		// Initialize Function prototype
		proto = new as_object();

		// We initialize the __proto__ member separately, as getObjectInterface
		// will end up calling getFunctionPrototype again and we want that
		// call to return the still-not-completely-constructed prototype rather
		// then create a new one. 
		proto->set_prototype(getObjectInterface());

		VM::get().addStatic(proto.get());

		if ( VM::get().getSWFVersion() >= 6 )
		{
			proto->init_member("apply", new builtin_function(function_apply));
			proto->init_member("call", new builtin_function(function_call));
		}
	}

	return proto.get();

}

static as_value
function_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> func = new as_object(getFunctionPrototype());
	//log_debug(_("User tried to invoke new Function()"));
	return as_value(func.get());
}

as_function::as_function()
	:
	// all functions inherit from global Function class
	//as_object(getFunctionPrototype())
	as_object()
{
	int flags = as_prop_flags::dontDelete |
	            as_prop_flags::dontEnum | 
	            as_prop_flags::onlySWF6Up;
	init_member(NSV::PROP_uuPROTOuu, as_value(getFunctionPrototype()), flags);
}


// What if we want a function to inherit from Object instead ?
as_function::as_function(as_object* iface)
	:
	// all functions inherit from global Function class
	//as_object(getFunctionPrototype())
	as_object()
{
	int flags = as_prop_flags::dontDelete |
	            as_prop_flags::dontEnum |
	            as_prop_flags::onlySWF6Up;
	init_member(NSV::PROP_uuPROTOuu, as_value(getFunctionPrototype()), flags);

	if ( iface )
	{
		iface->init_member(NSV::PROP_CONSTRUCTOR, this); 
		init_member(NSV::PROP_PROTOTYPE, as_value(iface));
	}
}

void
as_function::setPrototype(as_object* proto)
{
	//_properties = proto;
	init_member(NSV::PROP_PROTOTYPE, as_value(proto));
}

void
as_function::extends(as_function& superclass)
{
	as_object* newproto = new as_object(superclass.getPrototype().get());
	newproto->init_member(NSV::PROP_uuPROTOuu, superclass.getPrototype().get());

	if ( VM::get().getSWFVersion() > 5 )
	{
		newproto->init_member(NSV::PROP_uuCONSTRUCTORuu, &superclass); 
	}

	init_member(NSV::PROP_PROTOTYPE, as_value(newproto));
}

boost::intrusive_ptr<as_object>
as_function::getPrototype()
{
	// TODO: create if not available ?
	// TODO WARNING: what if user overwrites the 'prototype' member ?!
	//               this function should likely return the *new*
	//               prototype, not the old !!
	as_value proto;
	get_member(NSV::PROP_PROTOTYPE, &proto);
	return proto.to_object();
}

/* static public */
boost::intrusive_ptr<builtin_function>
as_function::getFunctionConstructor()
{
	static boost::intrusive_ptr<builtin_function> func = NULL;
	if ( ! func )
	{
		func = new builtin_function(
			function_ctor, // function constructor doesn't do anything
			getFunctionPrototype(), // exported interface
			true // use "this" as constructor
			);
		VM::get().addStatic(func.get());
	}
	return func;
}

/*
 * Initialize the "Function" member of a _global object.
 */
void
function_class_init(as_object& global)
{
	boost::intrusive_ptr<builtin_function> func=as_function::getFunctionConstructor();

	// Register _global.Function
	global.init_member("Function", func.get());

}

as_value
function_apply(const fn_call& fn)
{
	//int pushed=0; // new values we push on the stack

	// Get function body 
	boost::intrusive_ptr<as_function> function_obj = ensureType<as_function>(fn.this_ptr);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);
	new_fn_call.resetArgs();

	if ( ! fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror (_("Function.apply() called with no args"));
		);
	}
	else
	{
		// Get the object to use as 'this' reference
		new_fn_call.this_ptr = fn.arg(0).to_object();
		if (!new_fn_call.this_ptr )
		{
			// ... or recycle this function's call 'this' pointer
			// (most likely the Function instance)
			new_fn_call.this_ptr = fn.this_ptr;
		}

		if ( fn.nargs > 1 )
		// we have an 'arguments' array
		{
			IF_VERBOSE_ASCODING_ERRORS(
				if ( fn.nargs > 2 )
				{
					log_aserror(_("Function.apply() got %d"
						" args, expected at most 2"
						" -- discarding the ones in"
						" excess"),
						fn.nargs);
				}
			);

			boost::intrusive_ptr<as_object> arg1 = fn.arg(1).to_object();
			if ( ! arg1 )
			{
				IF_VERBOSE_ASCODING_ERRORS(
					log_aserror(_("Second arg of Function.apply"
						" is %s (expected array)"
						" - considering as call with no args"),
						fn.arg(1));
				);
				goto call_it;
			}

			boost::intrusive_ptr<Array_as> arg_array = \
					boost::dynamic_pointer_cast<Array_as>(arg1);

			if ( ! arg_array )
			{
				IF_VERBOSE_ASCODING_ERRORS(
					log_aserror(_("Second arg of Function.apply"
						" is of type %s, with value %s"
						" (expected array)"
						" - considering as call with no args"),
						fn.arg(1).typeOf(),
						fn.arg(1).to_string().c_str());
				);
				goto call_it;
			}

			unsigned int nelems = arg_array->size();

			//log_debug(_("Function.apply(this_ref, array[%d])"), nelems);
			for (unsigned int i=0; i<nelems; ++i)
			{
				new_fn_call.pushArg(arg_array->at(i));
				//pushed++;
			}

			//new_fn_call.set_offset(fn.env().get_top_index());
			//new_fn_call.nargs=nelems;
		}
	}

	call_it:

	// Call the function 
	as_value rv = function_obj->call(new_fn_call);

	// Drop additional values we pushed on the stack 
	//fn.env().drop(pushed);

        return rv;
}

as_value
function_call(const fn_call& fn)
{

	// Get function body 
	boost::intrusive_ptr<as_function> function_obj = ensureType<as_function>(fn.this_ptr);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);

	if ( ! fn.nargs )
	{
                log_debug (_("Function.call() with no args"));
		new_fn_call.nargs=0;
	}
	else
	{
		// Get the object to use as 'this' reference
		as_value this_val = fn.arg(0);
		boost::intrusive_ptr<as_object> this_ptr = this_val.to_object();

		if ( ! this_ptr )
		{
			// If the first argument is not an object, we should
			// not pass an object to the function, which I belive
			// should be allowed (but gnash code is not ready).
			// Anyway, the 'this' label inside the function should 
			// then be a strange object in that typeof() would return
			// 'object' but when compared to undefined matches !!
			// See actionscript.all/Function.as
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("First argument to Function.call(%s) doesn't cast to object. "
				"Gnash will keep the current 'this' pointer as it is, "
				"but this is known to not be the correct way to handle "
				"such a malformed call."), this_val);
			);
		}
		else
		{
			new_fn_call.this_ptr = this_ptr;
			as_object* proto = this_ptr->get_prototype().get();
                        if ( proto )
                        {
                                new_fn_call.super = this_ptr->get_super();
                        }
                        else
                        {
                                // TODO: check this !
                                log_debug("No prototype in 'this' pointer passed to Function.call");
                                new_fn_call.super = function_obj->get_super();
			}
		}
		new_fn_call.drop_bottom();
	}

	// Call the function 
	return (*function_obj)(new_fn_call);

	//log_debug(_("at function_call exit, stack: \n")); fn.env->dump_stack();

	//log_debug(_("%s: tocheck \n"), __FUNCTION__);
}

boost::intrusive_ptr<as_object>
as_function::constructInstance( as_environment& env,
	std::auto_ptr< std::vector<as_value> > args)
{
//	GNASH_REPORT_FUNCTION;

#ifndef GNASH_USE_GC
	assert(get_ref_count() > 0);
#endif // GNASH_USE_GC

	int swfversion = env.getVM().getSWFVersion();

	boost::intrusive_ptr<as_object> newobj;

	as_value us;
	bool has_proto = false;
	get_member(NSV::PROP_PROTOTYPE, &us);
	if (!us.is_undefined())
	{
		has_proto = true;
	}

        // a built-in class takes care of assigning a prototype
	// TODO: change this
        if ( isBuiltin() )
	{

		IF_VERBOSE_ACTION (
		log_action(_("it's a built-in class"));
		);

		fn_call fn(NULL, &env, args);
		as_value ret;
		try {
			ret = call(fn);
			//newobj = ret.to_object();
		} catch (std::exception& ex) {
			log_debug("Native function called as constructor threw exception: %s", ex.what());
			//newobj = new as_object();
		}

		if ( ret.is_object() )
		{
			newobj = ret.to_object();
		}
		else 
		{
			log_debug("Native function called as constructor returned %s", ret);
			newobj = new as_object();
		}
		assert(newobj); // we assume builtin functions do return objects !!

		// Add a __constructor__ member to the new object, but only for SWF6 up
		// (to be checked). NOTE that we assume the builtin constructors
		// won't set __constructor__ to some other value...
		int flags = as_prop_flags::dontEnum|as_prop_flags::onlySWF6Up; // can delete, hidden in swf5
		newobj->init_member(NSV::PROP_uuCONSTRUCTORuu, as_value(this), flags);

		if ( swfversion < 7 ) // && swfversion > 5 ?
		{
			newobj->init_member(NSV::PROP_CONSTRUCTOR, as_value(this), flags);
		}

        }
	else
	{
		// Set up the prototype.
		as_value	proto;

		// We can safely call as_object::get_member here as member name is 
		// a literal string in lowercase. (we should likely avoid calling
		// get_member as a whole actually, and use a getProto() or similar
		// method directly instead) TODO
		/*bool func_has_prototype=*/ get_member(NSV::PROP_PROTOTYPE, &proto);

		// user could have dropped the prototype..
		// see construct-properties-#.swf from swfdec testsuite
		//assert(func_has_prototype);

		IF_VERBOSE_ACTION (
		log_action(_("constructor prototype is %s"), proto);
		);

		// Create an empty object, with a ref to the constructor's prototype.
		newobj = new as_object(proto.to_object());

		// Add a __constructor__ member to the new object, but only for SWF6 up
		// (to be checked)
		int flags = as_prop_flags::dontEnum|as_prop_flags::onlySWF6Up; // can delete, hidden in swf5
		newobj->init_member(NSV::PROP_uuCONSTRUCTORuu, as_value(this), flags);

		if ( swfversion < 7 )
		{
			newobj->init_member(NSV::PROP_CONSTRUCTOR, as_value(this), flags);
		}

		// Super is computed from the object we're constructing,
		// It will work as long as we did set it's __proto__ and __constructor__
		// properties already.
		as_object* super = newobj->get_super();

		// Call the actual constructor function; new_obj is its 'this'.

		// We don't need the function result.
		fn_call fn(newobj.get(), &env, args, super);
		call(fn);
	}

	if (!has_proto)
		set_member(NSV::PROP_PROTOTYPE, as_value(newobj));
    
	return newobj;
}

} // end of gnash namespace

