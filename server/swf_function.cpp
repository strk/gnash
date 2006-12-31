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

#include <typeinfo>
#include <iostream>
#include <string>

using namespace std;

namespace gnash {


swf_function::~swf_function()
{
	if ( _properties ) _properties->drop_ref();
}

swf_function::swf_function(const action_buffer* ab,
			as_environment* env,
			size_t start, const std::vector<with_stack_entry>& with_stack)
	:
	as_function(NULL),
	//ctor(0),
	m_action_buffer(ab),
	m_env(env),
	m_with_stack(with_stack),
	m_start_pc(start),
	m_length(0),
	m_is_function2(false),
	m_local_register_count(0),
	m_function2_flags(0)
{
	assert(m_action_buffer);
	assert( m_start_pc < m_action_buffer->size() );

	// Define the 'prototype' member as a new object with
	// only the 'constructor' element defined.
	//_properties = new as_object();
	//as_object* proto = _properties;
	//proto->add_ref();

	//proto->set_member("constructor", this); //as_value(func));
	//proto->set_member_flags("constructor", 1);

	//set_member("prototype", as_value(proto));
}

// Dispatch.
void
swf_function::operator()(const fn_call& fn)
{

	as_environment*	our_env = m_env;
	if (our_env == NULL)
	{
		our_env = fn.env;
	}
	assert(our_env);

#if 0
	log_msg("swf_function() stack:\n"); fn.env->dump_stack();
	log_msg("  first_arg_bottom_index: %d\n", fn.first_arg_bottom_index);
#endif

	// Set up local stack frame, for parameters and locals.
	int	local_stack_top = our_env->get_local_frame_top();
	our_env->add_frame_barrier();

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

		assert(fn.this_ptr);
		our_env->set_local("this", fn.this_ptr);
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
		if (m_function2_flags & 0x01)
		{
			// preload 'this' into a register.
			our_env->local_register(current_reg).set_as_object(our_env->get_target());
			current_reg++;
		}

		if (m_function2_flags & 0x02)
		{
			// Don't put 'this' into a local var.
		}
		else
		{
			// Put 'this' in a local var.
			our_env->add_local("this", as_value(our_env->get_target()));
		}

		// Init arguments array, if it's going to be needed.
		boost::intrusive_ptr<as_array_object>	arg_array;
		if ((m_function2_flags & 0x04) || ! (m_function2_flags & 0x08))
		{
			arg_array = new as_array_object;

			as_value	index_number;
			for (int i = 0; i < fn.nargs; i++)
			{
				index_number.set_int(i);
				arg_array->set_member(index_number.to_string(), fn.arg(i));
			}
		}

		if (m_function2_flags & 0x04)
		{
			// preload 'arguments' into a register.
			our_env->local_register(current_reg).set_as_object(arg_array.get());
			current_reg++;
		}

		if (m_function2_flags & 0x08)
		{
			// Don't put 'arguments' in a local var.
		}
		else
		{
			// Put 'arguments' in a local var.
			our_env->add_local("arguments", as_value(arg_array.get()));
		}

		if (m_function2_flags & 0x10)
		{
			// Put 'super' in a register.
			our_env->local_register(current_reg).set_as_object(m_prototype);
			log_warning("TESTING: implement 'super' in function2 dispatch (reg)\n");

			current_reg++;
		}

		if (m_function2_flags & 0x20)
		{
			// Don't put 'super' in a local var.
		}
		else
		{
			// Put 'super' in a local var.
			our_env->add_local("super", as_value(m_prototype));
			log_warning("TESTING: implement 'super' in function2 dispatch (var)\n");
		}

		if (m_function2_flags & 0x40)
		{
			// Put '_root' in a register.
			our_env->local_register(current_reg).set_as_object(
				our_env->get_target()->get_root_movie());
			current_reg++;
		}

		if (m_function2_flags & 0x80)
		{
			// Put '_parent' in a register.
			as_value parent = our_env->get_variable("_parent");
			our_env->local_register(current_reg) = parent;
			current_reg++;
		}

		if (m_function2_flags & 0x100)
		{
			// Put '_global' in a register.
			as_object* global = VM::get().getGlobal();
			our_env->local_register(current_reg).set_as_object(global);
			current_reg++;
		}
	}

	// Execute the actions.
	//ActionExec exec(*m_action_buffer, *our_env, m_start_pc, m_length, fn.result, m_with_stack, m_is_function2);
	ActionExec exec(*this, *our_env, fn.result);
	exec();

	// Clean up stack frame.
	our_env->set_local_frame_top(local_stack_top);

	if (m_is_function2)
	{
		// Clean up the local registers.
		our_env->drop_local_registers(m_local_register_count);
	}
}

void
swf_function::set_length(int len)
{
	assert(m_action_buffer);
	assert(len >= 0);
	assert(m_start_pc+len <= m_action_buffer->size());
	m_length = len;
}

} // end of gnash namespace

