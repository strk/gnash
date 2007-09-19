// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "swf_function.h"
#include "array.h"
#include "gnash.h"
#include "fn_call.h"
#include "sprite_instance.h"
#include "action_buffer.h"
#include "ActionExec.h" // for operator()
#include "VM.h" // for storing _global in a local register
#include "builtin_function.h" // for Function constructor
#include "Object.h" // for getObjectInterface

#include <typeinfo>
#include <iostream>
#include <string>

using namespace std;

namespace gnash {


swf_function::~swf_function()
{
#ifndef GNASH_USE_GC
	if ( _properties ) _properties->drop_ref();
#endif //ndef GNASH_USE_GC
}

swf_function::swf_function(const action_buffer* ab,
			as_environment* env,
			size_t start, const ScopeStack& scopeStack)
	:
	as_function(new as_object(getObjectInterface())),
	//ctor(0),
	m_action_buffer(ab),
	m_env(env),
	_scopeStack(scopeStack),
	m_start_pc(start),
	m_length(0),
	m_is_function2(false),
	m_local_register_count(0),
	m_function2_flags(0)
{
	assert(m_action_buffer);
	assert( m_start_pc < m_action_buffer->size() );

	init_member("constructor", as_value(as_function::getFunctionConstructor().get()));
}

/*private static*/
boost::intrusive_ptr<as_object>
swf_function::getSuper(as_object& obj)
{ 
//#define GNASH_DEBUG_GETSUPER
	// Super class prototype is : obj.__proto__.__constructor__.prototype 
	boost::intrusive_ptr<as_object> proto = obj.get_prototype();
	if ( ! proto )
	{
#ifdef GNASH_DEBUG_GETSUPER
		log_msg("Object %p doesn't have a __proto__", &obj);
#endif
		return NULL;
	}

	// TODO: add a getConstructor() method to as_object
	//       returning an as_function ?
	//
	as_value ctor;
	bool ret = proto->get_member(as_object::PROP_uuCONSTRUCTORuu, &ctor);
	if ( ! ret )
	{
#ifdef GNASH_DEBUG_GETSUPER
		log_msg("Object.__proto__ %p doesn't have a __constructor__", (void*)proto.get());
#endif
		return NULL;
	}

	// TODO: if we cast ctor to an as_function and call getPrototype on it,
	// 	 it is possible that the returned object is NOT the current
	// 	 'prototype' member, as as_function caches it ?
	//
	boost::intrusive_ptr<as_object> ctor_obj = ctor.to_object();
	if ( ! ctor_obj )
	{
#ifdef GNASH_DEBUG_GETSUPER
		log_msg("Object.__proto__.__constructor__ doesn't cast to an object");
#endif
		return NULL;
	}

#ifdef GNASH_DEBUG_GETSUPER
	log_msg("ctor_obj is %p", ctor_obj.get());
#endif

	as_value ctor_proto;
	ret = ctor_obj->get_member(as_object::PROP_PROTOTYPE, &ctor_proto);
	if ( ! ret )

	{
#ifdef GNASH_DEBUG_GETSUPER
		log_msg("Object.__proto__.constructor %p doesn't have a prototype", ctor_obj.get());
#endif
		return NULL;
	}

	boost::intrusive_ptr<as_object> super = ctor_proto.to_object();
	if ( ! super )
	{
#ifdef GNASH_DEBUG_GETSUPER
		log_msg("Object.__proto__.constructor.prototype doesn't cast to an object");
#endif
		return NULL;
	}

	return super; // FIXME: return the intrusive_ptr directly !!

}

/*private static*/
as_array_object* 
swf_function::getArguments(swf_function& callee, const fn_call& fn)
{ 
#ifndef GNASH_USE_GC
	// We'll be storing the callee as_object into an as_value
	// so you must make sure you have a reference on it before
	// callign this function.
	assert(callee.get_ref_count() > 0);
#endif // ndef GNASH_USE_GC

	// Super class prototype is : obj.__proto__.constructor.prototype 
	as_array_object* arguments = new as_array_object();
	for (unsigned int i=0; i<fn.nargs; ++i)
	{
		arguments->push(fn.arg(i));
	}
	arguments->set_member(as_object::PROP_CALLEE, &callee);

	return arguments;

}

// Dispatch.
as_value
swf_function::operator()(const fn_call& fn)
{

	as_environment*	our_env = m_env;
	assert(our_env);
	if (our_env == NULL)
	{
		our_env = &fn.env();
	}
	assert(our_env);

#if 0
	log_msg("swf_function() stack:\n"); fn.env().dump_stack();
	log_msg("  first_arg_bottom_index: %d\n", fn.first_arg_bottom_index);
#endif

	// Set up local stack frame, for parameters and locals.
	our_env->pushCallFrame(this);

	// Some features are version-dependant
	unsigned swfversion = VM::get().getSWFVersion();

	if (m_is_function2 == false)
	{
		// Conventional function.

		// Push the arguments onto the local frame.
		int	args_to_pass = imin(fn.nargs, m_args.size());
		for (int i = 0; i < args_to_pass; i++)
		{
			assert(m_args[i].m_register == 0);
			our_env->add_local(m_args[i].m_name, fn.arg(i));
		}

		// Add 'this'
		assert(fn.this_ptr);
		our_env->set_local("this", fn.this_ptr);

		// Add 'super' (SWF6+ only)
		if ( swfversion > 5 )
		{
			boost::intrusive_ptr<as_object> super = getSuper(*(fn.this_ptr));
			our_env->set_local("super", as_value(super));
		}

		// Add 'arguments'
		our_env->set_local("arguments", getArguments(*this, fn));
	}
	else
	{
		// function2: most args go in registers; any others get pushed.
		
		// Create local registers.
		our_env->add_local_registers(m_local_register_count);

		// Handle the explicit args.
		int	args_to_pass = imin(fn.nargs, m_args.size());
		for (int i = 0; i < args_to_pass; i++)
		{
			if (m_args[i].m_register == 0)
			{
				// Conventional arg passing: create a local var.
				our_env->add_local(m_args[i].m_name, fn.arg(i));
			}
			else
			{
				// Pass argument into a register.
				int	reg = m_args[i].m_register;
				our_env->local_register(reg) = fn.arg(i);
			}
		}

		// Handle the implicit args.
		// @@ why start at 1 ? Note that starting at 0 makes	
		// intro.swf movie fail to play correctly.
		uint8_t current_reg = 1;
		if (m_function2_flags & PRELOAD_THIS)
		{
			// preload 'this' into a register.
			our_env->local_register(current_reg).set_as_object(fn.this_ptr); 
			current_reg++;
		}

		if (m_function2_flags & SUPPRESS_THIS)
		{
			// Don't put 'this' into a local var.
		}
		else
		{
			// Put 'this' in a local var.
			our_env->add_local("this", as_value(fn.this_ptr));
		}

		// Init arguments array, if it's going to be needed.
		boost::intrusive_ptr<as_array_object>	arg_array;
		if ((m_function2_flags & PRELOAD_ARGUMENTS) || ! (m_function2_flags & SUPPRESS_ARGUMENTS))
		{
			arg_array = getArguments(*this, fn);
		}

		if (m_function2_flags & PRELOAD_ARGUMENTS)
		{
			// preload 'arguments' into a register.
			our_env->local_register(current_reg).set_as_object(arg_array.get());
			current_reg++;
		}

		if (m_function2_flags & SUPPRESS_ARGUMENTS)
		{
			// Don't put 'arguments' in a local var.
		}
		else
		{
			// Put 'arguments' in a local var.
			our_env->add_local("arguments", as_value(arg_array.get()));
		}

		if ( (m_function2_flags & PRELOAD_SUPER) && swfversion > 5)
		{
			// Put 'super' in a register (SWF6+ only).
			our_env->local_register(current_reg).set_as_object(getSuper(*(fn.this_ptr)));
			current_reg++;
		}

		if (m_function2_flags & SUPPRESS_SUPER)
		{
			// Don't put 'super' in a local var.
		}
		else if ( swfversion > 5 )
		{
			// Put 'super' in a local var (SWF6+ only)
			our_env->add_local("super", as_value(getSuper(*(fn.this_ptr))));
		}

		if (m_function2_flags & PRELOAD_ROOT) 
		{
			// Put '_root' in a register.
			our_env->local_register(current_reg).set_as_object(
				our_env->get_target()->get_root_movie());
			current_reg++;
		}

		if (m_function2_flags & PRELOAD_PARENT)
		{
			// Put '_parent' in a register.
			as_value parent = our_env->get_variable("_parent");
			our_env->local_register(current_reg) = parent;
			current_reg++;
		}

		if (m_function2_flags & PRELOAD_GLOBAL)
		{
			// Put '_global' in a register.
			as_object* global = VM::get().getGlobal();
			our_env->local_register(current_reg).set_as_object(global);
			current_reg++;
		}
	}

	// Execute the actions.
	//ActionExec exec(*m_action_buffer, *our_env, m_start_pc, m_length, fn.result, m_with_stack, m_is_function2);
        as_value result;
	ActionExec exec(*this, *our_env, &result, fn.this_ptr.get());
	exec();

	our_env->popCallFrame();
        return result;
}

void
swf_function::set_length(int len)
{
	assert(m_action_buffer);
	assert(len >= 0);
	assert(m_start_pc+len <= m_action_buffer->size());
	m_length = len;
}

#ifdef GNASH_USE_GC
void
swf_function::markReachableResources() const
{
	// Mark scope stack objects
	for (ScopeStack::const_iterator i=_scopeStack.begin(), e=_scopeStack.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

	if ( m_env ) m_env->markReachableResources();

	// Invoke parent class marker
	markAsFunctionReachable(); 
}
#endif // GNASH_USE_GC

} // end of gnash namespace

