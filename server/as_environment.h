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

#ifndef GNASH_AS_ENVIRONMENT_H
#define GNASH_AS_ENVIRONMENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


//#include "gnash.h"
//#include "as_object.h"
//#include "types.h"
//#include <wchar.h>

#include "container.h" // for composition (stringi_hash, tu_string)
#include "as_value.h" // for composition (vector + frame_slot)
#include "log.h" // for inlines (dump_*)

#include <vector>

namespace gnash {

// Forward declarations
class sprite_instance;
struct with_stack_entry;

/// ActionScript "environment", essentially VM state?
struct as_environment
{

	/// Stack of as_values in this environment
	std::vector<as_value>	m_stack;

	/// Variables available in this environment
	stringi_hash<as_value>	m_variables;

	/// For local vars.  Use empty names to separate frames.
	struct frame_slot
	{
		tu_string	m_name;
		as_value	m_value;

		frame_slot()
		{
		}

		frame_slot(const tu_string& name, const as_value& val)
			:
			m_name(name),
			m_value(val)
		{
		}
	};

	std::vector<frame_slot>	m_local_frames;


	as_environment()
		:
		m_target(0)
	{
	}

	sprite_instance*	get_target() { return m_target; }
	void	set_target(sprite_instance* target) { m_target = target; }

	// stack access/manipulation
	// @@ TODO do more checking on these
	template<class T>
	// stack access/manipulation
	void	push(T val) { push_val(as_value(val)); }
	void	push_val(const as_value& val) { m_stack.push_back(val); }


	/// Pops an as_value off the stack top and return it.
	as_value pop()
	{
		assert( m_stack.size() > 0 );
		as_value result = m_stack.back();
		m_stack.pop_back();
		return result;
	}

	/// Get stack value at the given distance from top.
	//
	/// top(0) is actual stack top
	///
	as_value& top(size_t dist)
	{
		assert ( m_stack.size() > dist );
		return m_stack[m_stack.size() - 1 - dist];
	}

	/// Get stack value at the given distance from bottom.
	//
	/// bottom(0) is actual stack top
	///
	as_value& bottom(size_t index)
	{
		assert ( m_stack.size() > index );
		return m_stack[index];
	}

	/// Drop 'count' values off the top of the stack.
	void drop(size_t count)
	{
		assert ( m_stack.size() >= count );
		m_stack.resize(m_stack.size() - count);
	}

	/// Returns index of top stack element
	// FIXME: what if stack is empty ??
	// I'd obsolete this and convert calling code to use
	// stack_size() instead.
	int	get_top_index() const { return m_stack.size() - 1; }

	size_t stack_size() const { return m_stack.size(); }

	/// \brief
	/// Return the (possibly UNDEFINED) value of the named var.
	/// Variable name can contain path elements.
	///
	as_value get_variable(const tu_string& varname) const;

	/// Same of the above, but no support for path.
	as_value get_variable_raw(const tu_string& varname) const;

	/// Given a path to variable, set its value.
	void	set_variable(const tu_string& path, const as_value& val);

	/// Same of the above, but no "with" stack to search for
	void	set_variable_raw(const tu_string& path, const as_value& val);

	/// Set/initialize the value of the local variable.
	void	set_local(const tu_string& varname, const as_value& val);

	/// \brief
	/// Add a local var with the given name and value to our
	/// current local frame. 
	///
	/// Use this when you know the var
	/// doesn't exist yet, since it's faster than set_local();
	/// e.g. when setting up args for a function.
	void	add_local(const tu_string& varname, const as_value& val);

	/// Create the specified local var if it doesn't exist already.
	void	declare_local(const tu_string& varname);

	/// Retrieve the named member (variable).
	//
	/// If no member is found under the given name
	/// 'val' is untouched and 'false' is returned.
	/// 
	bool	get_member(const tu_stringi& varname, as_value* val) const;
	void	set_member(const tu_stringi& varname, const as_value& val);

	// Parameter/local stack frame management.
	int	get_local_frame_top() const { return m_local_frames.size(); }
	void	set_local_frame_top(unsigned int t) {
		assert(t <= m_local_frames.size());
		m_local_frames.resize(t);
	}
	void	add_frame_barrier() { m_local_frames.push_back(frame_slot()); }

	// Local registers.
	void	add_local_registers(unsigned int register_count)
	{
		m_local_register.resize(m_local_register.size() + register_count);
	}
	void	drop_local_registers(unsigned int register_count)
	{
		m_local_register.resize(m_local_register.size() - register_count);
	}

	/// \brief
	/// Return a pointer to the specified local register.
	/// Local registers are numbered starting with 1.
	//
	/// Return value will never be NULL.  If reg is out of bounds,
	/// we log an error, but still return a valid pointer (to
	/// global reg[0]).  So the behavior is a bit undefined, but
	/// not dangerous.
	//as_value* local_register_ptr(unsigned int reg);

	/// Return a reference to the Nth local register.
	as_value& local_register(uint8_t n)
	{
		assert( n < m_local_register.size() );
		return m_local_register[n];
	}

	/// Return a reference to the Nth global register.
	as_value& global_register(unsigned int n)
	{
		assert(n<4);
		return m_global_register[n];
	}

	/// Find the sprite/movie referenced by the given path.
	sprite_instance*	find_target(const tu_string& path) const;

	/// \brief
	/// Find the sprite/movie represented by the given value.
	//
	/// The value might be a reference to the object itself, or a
	/// string giving a relative path name to the object.
	sprite_instance*	find_target(const as_value& val) const;

	/// Dump content of the stack to a std::ostream
	void dump_stack(std::ostream& out=std::cerr)
	{
		out << "Stack: ";
		for (unsigned int i=0, n=m_stack.size(); i<n; i++)
		{
			if (i) out << " | ";
			out << '"' << m_stack[i].to_string() << '"';
		}
		out << std::endl;
	}

	/// Dump the local registers to a std::ostream
	void dump_local_registers(std::ostream& out=std::cerr)
	{
		out << "Local registers: ";
		for (unsigned int i=0, n=m_local_register.size(); i<n; i++)
		{
			if (i) out << " | ";
			out << '"' << m_local_register[i].to_string() << '"';
		}
		out << std::endl;
	}

	/// Dump the global registers to a std::ostream
	void dump_global_registers(std::ostream& out=std::cerr)
	{
		out << "Global registers: ";
		for (unsigned int i=0; i<4; ++i)
		{
			if (i) out << " | ";
			out << '"' << m_global_register[i].to_string() << '"';
		}
		out << std::endl;
	}

	/// Return the SWF version we're running for.
	//
	/// TODO: check what to return when playing multiple
	/// movies with different versions: should we always
	/// return version of the topmost (first-loaded) movie
	/// or of the target (as I think we're doing now) ?
	///
	int get_version() const;

private:

	as_value m_global_register[4];

	/// function2 uses this (could move to swf_function2 class)
	std::vector<as_value>	m_local_register;

	/// Movie target. 
	sprite_instance*	m_target;

	int find_local(const tu_string& varname) const;

	bool parse_path(const tu_string& var_path, tu_string* path,
		tu_string* var) const;

	/// Given a variable name, set its value (no support for path)
	void set_variable_raw(const tu_string& path, const as_value& val,
		const std::vector<with_stack_entry>& with_stack);

	/// Given a path to variable, set its value.
	void set_variable(const tu_string& path, const as_value& val,
		const std::vector<with_stack_entry>& with_stack);

	/// \brief
	/// Return the (possibly UNDEFINED) value of the named var.
	/// Variable name can contain path elements.
	///
	as_value get_variable(const tu_string& varname,
		const std::vector<with_stack_entry>& with_stack) const;

	/// Same of the above, but no support for path.
	as_value get_variable_raw(const tu_string& varname,
		const std::vector<with_stack_entry>& with_stack) const;


};


} // end namespace gnash


#endif // GNASH_AS_ENVIRONMENT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
