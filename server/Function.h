// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"

namespace gnash {

/// ActionScript Function.
class function_as_object : public as_object
{
private:
	// Common things to do, whatever constructor is used.
	void init();

public:
	action_buffer*	m_action_buffer;

	/// @@ might need some kind of ref count here, but beware cycles
	as_environment*	m_env;

	/// initial with-stack on function entry.
	array<with_stack_entry>	m_with_stack;

	int	m_start_pc;
	int	m_length;
	struct arg_spec
	{
		int	m_register;
		tu_string	m_name;
	};
	array<arg_spec>	m_args;
	bool	m_is_function2;
	uint8	m_local_register_count;

	/// used by function2 to control implicit
	/// arg register assignments
	uint16	m_function2_flags;

	/// The "prototype" member.
	//
	/// Used for class constructor and members
	/// to be inherited by instances of this
	/// "Function" (class)
	///
	as_object*	m_properties;

	~function_as_object();

	/// Default constructor
	function_as_object();

	/// NULL environment is allowed -- if so, then
	/// functions will be executed in the caller's
	/// environment, rather than the environment where they
	/// were defined.
	function_as_object(action_buffer* ab, as_environment* env,
			int start, const array<with_stack_entry>& with_stack);

	void	set_is_function2() { m_is_function2 = true; }
	void	set_local_register_count(uint8 ct) { assert(m_is_function2); m_local_register_count = ct; }
	void	set_function2_flags(uint16 flags) { assert(m_is_function2); m_function2_flags = flags; }

	void	add_arg(int arg_register, const char* name)
	{
		assert(arg_register == 0 || m_is_function2 == true);
		m_args.resize(m_args.size() + 1);
		m_args.back().m_register = arg_register;
		m_args.back().m_name = name;
	}

	void	set_length(int len) { assert(len >= 0); m_length = len; }

	/// Dispatch.
	void	operator()(const fn_call& fn);

	//void	lazy_create_properties();
};


void function_apply(const fn_call& fn);
void function_call(const fn_call& fn);

/// Initialize the global Function constructor
void function_init(as_object* global);

} // end of gnash namespace

// __FUNCTION_H__
#endif

