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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <log.h>
#include <as_function.h>
#include <builtin_function.h> // for _global.Function
#include "as_value.h"
#include <array.h>
#include <gnash.h>
#include <fn_call.h>
#include <sprite_instance.h>

#include <typeinfo>
#include <iostream>

using namespace std;

namespace gnash {

// should be static, probably
void function_apply(const fn_call& fn);
void function_call(const fn_call& fn);
static as_object* getFunctionPrototype();
static void do_nothing(const fn_call& fn);

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
	static as_object* proto = NULL;

	if ( proto == NULL ) {
		// Initialize Function prototype
		proto = new as_object();
		proto->set_member("apply", &function_apply);
		proto->set_member("call", &function_call);
	}

	return proto;

}

static void
do_nothing(const fn_call& fn)
{
	log_msg("User tried to invoke new Function()");
	if ( fn.result )
	{
		fn.result->set_undefined();
	}
}



// What if we want a function to inherit from Object instead ?
as_function::as_function(as_object* iface)
	:
	// all functions inherit from global Function class
	as_object(getFunctionPrototype()),
	_properties(iface)
{
	/// TODO: create properties lazily, on getPrototype() call
	if ( ! _properties )
	{
		_properties = new as_object();
	}

	_properties->set_member("constructor", this); 
	_properties->set_member_flags("constructor", 1);
	set_member("prototype", as_value(_properties));
}

as_object*
as_function::getPrototype()
{
	// TODO: create if not available ?
	return _properties;
}

/*
 * Initialize the "Function" member of a _global object.
 */
void function_init(as_object* global)
{
	// This is going to be the global Function "class"/"function"
	// TODO: use Function() instead (where Function derives from as_function, being a class)
	static as_function *func=new builtin_function(
		do_nothing, // function constructor doesn't do anything
		getFunctionPrototype() // exported interface
	);

	// We make the 'prototype' element be a reference to
	// the __proto__ element
	//as_object* proto = func->m_prototype;
	//proto->add_ref();

	//proto->set_member("constructor", func); //as_value(func));
	//proto->set_member_flags("constructor", 1);

	//func->set_member("prototype", as_value(proto));

	// Register _global.Function
	global->set_member("Function", func);

}

void
function_apply(const fn_call& fn)
{
	int pushed=0; // new values we push on the stack

	// Get function body 
	as_function* function_obj = fn.env->top(1).to_as_function();
	assert(function_obj);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);
	new_fn_call.nargs=0;

	if ( ! fn.nargs )
	{
            dbglogfile << "Function.apply() with no args" << endl;
	}
	else
	{
		// Get the object to use as 'this' reference
		as_object *this_ptr = fn.arg(0).to_object();
		new_fn_call.this_ptr = this_ptr;

		if ( fn.nargs > 1 )
		// we have an 'arguments' array
		{
			if ( fn.nargs > 2 )
			{
                            dbglogfile << "Function.apply() with more then 2 args" << endl;
			}

			as_object *arg1 = fn.arg(1).to_object();
			assert(arg1);

			as_array_object *arg_array = \
					dynamic_cast<as_array_object*>(arg1);

			if ( ! arg_array )
			{
                            dbglogfile << "Second argument to Function.apply() is not an array" << endl;
			}
			else
			{

				unsigned int nelems = arg_array->size();

				//log_error("Function.apply(this_ref, array[%d])\n", nelems);
				as_value index, value;
				for (unsigned int i=nelems; i; i--)
				{
					value=arg_array->at(i-1);
					//log_msg("value: %s\n", value.to_string());
					fn.env->push_val(value);
					pushed++;
				}

				new_fn_call.first_arg_bottom_index=fn.env->get_top_index();
				new_fn_call.nargs=nelems;
			}
		}
	}

	// Call the function 
	(*function_obj)(new_fn_call);

	// Drop additional values we pushed on the stack 
	fn.env->drop(pushed);

	//log_msg("at function_apply exit, stack: \n"); fn.env->dump_stack();
}

void
function_call(const fn_call& fn)
{

	// Get function body 
	as_function* function_obj = fn.env->top(1).to_as_function();
	assert(function_obj);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);

	if ( ! fn.nargs )
	{
                dbglogfile << "Function.call() with no args" << endl;
		new_fn_call.nargs=0;
	}
	else
	{
		// Get the object to use as 'this' reference
		as_object *this_ptr = fn.arg(0).to_object();
		new_fn_call.this_ptr = this_ptr;
		new_fn_call.nargs--;
		new_fn_call.first_arg_bottom_index--;
	}

	// Call the function 
	(*function_obj)(new_fn_call);

	//log_msg("at function_call exit, stack: \n"); fn.env->dump_stack();

	//log_msg("%s: tocheck \n", __FUNCTION__);
}



} // end of gnash namespace

