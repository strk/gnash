// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_FUNCTION_H
#define GNASH_SWF_FUNCTION_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // GNASH_USE_GC
#endif

#include "impl.h"
#include "as_function.h" // for inheritance
#include "as_object.h" // for composition (vector element)
#include "VM.h" //fow SWF version

#include <boost/algorithm/string/case_conv.hpp> 
#include <cassert>
#include <string>

// Forward declarations
namespace gnash {
	class action_buffer;
	class as_environmnet;
	class Array_as;
}

namespace gnash {

/// SWF-defined Function 
class swf_function : public as_function
{

private:

	/// Action buffer containing the function definition
	const action_buffer* m_action_buffer;

	/// @@ might need some kind of ref count here, but beware cycles
	as_environment*	m_env;

	typedef std::vector< boost::intrusive_ptr<as_object> > ScopeStack;

	/// Scope stack on function definition.
	ScopeStack _scopeStack;

	/// \brief
	/// Offset within the action_buffer where
	/// start of the function is found.
	size_t	m_start_pc;

	/// Length of the function within the action_buffer
	//
	/// This is currently expressed in bytes as the
	/// action_buffer is just a blog of memory corresponding
	/// to a DoAction block
	size_t m_length;

	struct arg_spec
	{
		int m_register;
		std::string m_name;
	};
	std::vector<arg_spec>	m_args;
	bool	m_is_function2;
	boost::uint8_t	m_local_register_count;

	/// used by function2 to control implicit arg register assignments
	// 
	/// See http://sswf.sourceforge.net/SWFalexref.html#action_declare_function2
	boost::uint16_t	m_function2_flags;

	/// Return an 'arguments' object.
	//
	/// The 'arguments' variable is an array with an additional
	/// 'callee' member, set to the function being called.
	///
	/// NOTE: the callee as_object will be stored in an as_value, thus
	///       getting wrapped into an intrusive_ptr. Make sure you have
	///	  a reference on it!
	///	  
	///
	static Array_as* getArguments(swf_function& callee, const fn_call& fn, as_object* caller);

public:

	enum SWFDefineFunction2Flags
	{
		/// Bind one register to "this" 
		PRELOAD_THIS = 0x01, // 1

		/// No "this" variable accessible by-name 
		SUPPRESS_THIS = 0x02, // 2

		/// Bind one register to "arguments" 
		PRELOAD_ARGUMENTS = 0x04, // 4

		/// No "argument" variable accessible by-name 
		SUPPRESS_ARGUMENTS = 0x08, // 8

		/// Bind one register to "super" 
		PRELOAD_SUPER = 0x10, // 16

		/// No "super" variable accessible by-name 
		SUPPRESS_SUPER = 0x20, // 32

		/// Bind one register to "_root" 
		PRELOAD_ROOT = 0x40, // 64

		/// Bind one register to "_parent" 
		PRELOAD_PARENT = 0x80, // 128

		/// Bind one register to "_global" 
		//
		/// TODO: check this. See http://sswf.sourceforge.net/SWFalexref.html#action_declare_function2
		///       Looks like flags would look swapped
		PRELOAD_GLOBAL = 256 // 0x100

	};


	~swf_function();

	/// Default constructor
	//
	/// Creates a Function object inheriting
	/// the Function interface (apply,call)
	///
	//swf_function();

	/// Construct a Built-in ActionScript class 
	//
	/// The provided export_iface as_object is what will end
	/// up being the class's 'prototype' member, caller must
	/// make sure to provide it with a 'constructor' member
	/// pointing to the function that creates an instance of
	/// that class.
	/// All built-in classes derive from the Function
	/// built-in class, whose exported interface will be 
	/// accessible trought their __proto__ member.
	///
	/// @param export_iface the exported interface
	///
	//swf_function(as_object* export_iface);
	// USE THE builtin_function instead!

	/// \brief
	/// Create an ActionScript function as defined in an
	/// action_buffer starting at offset 'start'
	//
	/// NULL environment is allowed -- if so, then
	/// functions will be executed in the caller's
	/// environment, rather than the environment where they
	/// were defined.
	///
	swf_function(const action_buffer* ab,
		as_environment* env,
		size_t start,
		const ScopeStack& with_stack);

	const ScopeStack& getScopeStack() const
	{
		return _scopeStack;
	}

	const action_buffer& getActionBuffer() const
	{
		assert(m_action_buffer);
		return *m_action_buffer;
	}

	size_t getStartPC() const
	{
		return m_start_pc;
	}

	size_t getLength() const
	{
		return m_length;
	}

	bool isFunction2() const
	{
		return m_is_function2;
	}

	void	set_is_function2() { m_is_function2 = true; }

	void	set_local_register_count(boost::uint8_t ct) { assert(m_is_function2); m_local_register_count = ct; }

	void	set_function2_flags(boost::uint16_t flags) { assert(m_is_function2); m_function2_flags = flags; }

	void	add_arg(int arg_register, const char* name)
	{
		assert(arg_register == 0 || m_is_function2 == true);
		m_args.resize(m_args.size() + 1);
		m_args.back().m_register = arg_register;
		m_args.back().m_name = PROPNAME(name);
	}

	void	set_length(int len);

	/// Dispatch.
	as_value	operator()(const fn_call& fn);

	//void	lazy_create_properties();

#ifdef GNASH_USE_GC
	/// Mark reachable resources. Override from as_function.
	//
	/// Reachable resources from this object is it's scope stack
	/// and the prototype.
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC
};


} // end of gnash namespace

// __GNASH_SWF_FUNCTION_H__
#endif

