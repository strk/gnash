// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: as_environment.h,v 1.40 2007/02/06 17:46:25 rsavoye Exp $ */

#ifndef GNASH_AS_ENVIRONMENT_H
#define GNASH_AS_ENVIRONMENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_value.h" // for composition (vector + frame_slot)
#include "StringPredicates.h" // for Variables 

#include <map> // for composition (Variables)
#include <string> // for frame_slot name
#include <vector>
#include <iostream> // for dump_stack inline

namespace gnash {

// Forward declarations
class character;
class with_stack_entry;

/// ActionScript execution environment.
class as_environment
{
public:

	/// A stack of objects used for variables/members lookup
	typedef std::vector<with_stack_entry> ScopeStack;

	/// Stack of as_values in this environment
	std::vector<as_value>	m_stack;

	/// For local vars.  Use empty names to separate frames.
	class frame_slot
	{
	public:
		std::string	m_name;
		as_value	m_value;

		frame_slot()
		{
		}

		frame_slot(const std::string& name, const as_value& val)
			:
			m_name(name),
			m_value(val)
		{
		}
	};

	as_environment()
		:
		m_target(0)
	{
	}

	character* get_target() { return m_target; }
	void set_target(character* target) { m_target = target; }

	/// @{ Stack access/manipulation

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

	/// Insert 'count' undefined values before 'offset'.
	//
	/// An offset of 0 will prepend the values,
	/// An offset of size() [too far] will append the values.
	///
	void padStack(size_t offset, size_t count);

	/// Returns index of top stack element
	// FIXME: what if stack is empty ??
	// I'd obsolete this and convert calling code to use
	// stack_size() instead.
	int	get_top_index() const { return m_stack.size() - 1; }

	size_t stack_size() const { return m_stack.size(); }

	/// @}  stack access/manipulation
	///

	/// \brief
	/// Return the (possibly UNDEFINED) value of the named variable
	//
	/// @param varname 
	///	Variable name. Can contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///
	as_value get_variable(const std::string& varname) const;

	/// Return the (possibly UNDEFINED) value of the named variable.
	//
	/// @param varname 
	///	Variable name. Can not contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///
	as_value get_variable_raw(const std::string& varname) const;

	/// \brief
	/// Delete a variable, w/out support for the path, using
	/// a ScopeStack.
	//
	/// @param varname 
	///	Variable name. Can not contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///
	/// @param with_stack
	///	The Scope stack to use for lookups.
	///
	bool del_variable_raw(const std::string& varname,
			const ScopeStack& with_stack);

	/// Return the (possibly UNDEFINED) value of the named var.
	//
	/// @param varname 
	///	Variable name. Can contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///
	/// @param with_stack
	///	The Scope stack to use for lookups.
	///
	as_value get_variable(const std::string& varname,
		const ScopeStack& with_stack) const;

	/// \brief
	/// Given a path to variable, set its value.
	/// Variable name can contain path elements.
	//
	/// @param path 
	///	Variable path.
	///	TODO: should be case-insensitive up to SWF6.
	///
	/// @param val
	///	The value to assign to the variable, if found.
	///
	/// TODO: make this function return some info about the
	///       variable being found and set ?
	///
	void set_variable(const std::string& path, const as_value& val);

	/// Given a variable name, set its value (no support for path)
	//
	/// If no variable with that name is found, a new one
	/// will be created as a member of current target.
	///
	/// @param var
	///	Variable name. Can not contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///
	/// @param val
	///	The value to assign to the variable, if found.
	///
	void set_variable_raw(const std::string& var, const as_value& val);

	/// \brief
	/// Given a path to variable, set its value.
	//
	/// If no variable with that name is found, a new one is created.
	///
	/// For path-less variables, this would act as a proxy for
	/// set_variable_raw.
	///
	/// @param path
	///	Variable path. 
	///	TODO: should be case-insensitive up to SWF6.
	///
	/// @param val
	///	The value to assign to the variable.
	///
	/// @param with_stack
	///	The Scope stack to use for lookups.
	///
	void set_variable(const std::string& path, const as_value& val,
		const ScopeStack& with_stack);

	/// Set/initialize the value of the local variable.
	//
	/// If no *local* variable with that name is found, a new one
	/// will be created.
	///
	/// @param varname
	///	Variable name. Can not contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///
	/// @param val
	///	The value to assign to the variable. 
	///
	void	set_local(const std::string& varname, const as_value& val);

	/// \brief
	/// Add a local var with the given name and value to our
	/// current local frame. 
	///
	/// Use this when you know the var
	/// doesn't exist yet, since it's faster than set_local();
	/// e.g. when setting up args for a function.
	void	add_local(const std::string& varname, const as_value& val);

	/// Create the specified local var if it doesn't exist already.
	void	declare_local(const std::string& varname);

	/// Retrieve the named member (variable).
	//
	/// If no member is found under the given name
	/// 'val' is untouched and 'false' is returned.
	/// 
	/// TODO: rename to get_variable
	///
	bool	get_member(const std::string& varname, as_value* val) const;

	/// Set the named variable 
	//
	/// TODO: rename to set_variable, take a std::string
	///
	void	set_member(const std::string& varname, const as_value& val);

	// Parameter/local stack frame management.
	int	get_local_frame_top() const { return m_local_frames.size(); }
	void	set_local_frame_top(unsigned int t) {
		assert(t <= m_local_frames.size());
		m_local_frames.resize(t);
	}
	void	add_frame_barrier() { m_local_frames.push_back(frame_slot()); }

	/// Add 'count' local registers (add space to end)
	//
	/// Local registers are only meaningful within a function2 context.
	///
	void	add_local_registers(unsigned int register_count);

	/// Drop 'count' local registers (drop space from end)
	//
	/// Local registers are only meaningful within a function2 context.
	///
	void	drop_local_registers(unsigned int register_count);

	/// Return the number of local registers currently available
	//
	/// Local registers are only meaningful within a function2 context.
	///
	size_t num_local_registers() const {
		return m_local_register.size();
	}

	/// Set number of local registers.
	//
	/// Local registers are only meaningful within a function2 context.
	///
	void resize_local_registers(unsigned int register_count) {
		m_local_register.resize(register_count);
	}

	/// Return a reference to the Nth local register.
	//
	/// Local registers are only meaningful within a function2 context.
	///
	as_value& local_register(uint8_t n);

        /// Set the Nth local register to something
        void set_local_register(uint8_t n, as_value &val) {
	    if (n <= m_local_register.size()) {
		m_local_register[n] = val;
	    }
	}

	/// Return a reference to the Nth global register.
	as_value& global_register(unsigned int n)
	{
		assert(n<4);
		return m_global_register[n];
	}

        /// Set the Nth local register to something
        void set_global_register(uint8_t n, as_value &val) {
	    if (n <= 4) {
		m_global_register[n] = val;
	    }
	}

	/// Find the sprite/movie referenced by the given path.
	//
	/// Supports both /slash/syntax and dot.syntax
	///
	character* find_target(const std::string& path) const;

	/// \brief
	/// Find the sprite/movie represented by the given value.
	//
	/// The value might be a reference to the object itself, or a
	/// string giving a relative path name to the object.
	///
	/// @@ make private ? --strk;
	///
	character* find_target(const as_value& val) const;

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
	//
	/// NOTE that nothing will be written to the stream if NO local registers
	///      are set
	///
	void dump_local_registers(std::ostream& out=std::cerr) const;

	/// Dump the global registers to a std::ostream
	void dump_global_registers(std::ostream& out=std::cerr) const;

	/// Dump the local variables to a std::ostream
	void dump_local_variables(std::ostream& out=std::cerr) const;

	/// Return the SWF version we're running for.
	//
	/// NOTE: this is the version encoded in the first loaded
	///       movie, and cannot be changed during play even if
	///       replacing the root movie with an externally loaded one.
	///
	int get_version() const;

	// See if the given variable name is actually a sprite path
	// followed by a variable name.  These come in the format:
	//
	// 	/path/to/some/sprite/:varname
	//
	// (or same thing, without the last '/')
	//
	// or
	//	path.to.some.var
	//
	// If that's the format, puts the path part (no colon or
	// trailing slash) in *path, and the varname part (no colon, no dot)
	// in *var and returns true.
	//
	// If no colon or dot, returns false and leaves *path & *var alone.
	//
	/// @param is_slash_based
	///	If not null gets set to true if path is slash-based
	///	(path/to/:variable), and to false if path is dot-based
	///	(path.to.variable).
	///
	/// TODO: return an integer: 0 not a path, 1 a slash-based path, 2 a dot-based path
	static bool parse_path(const std::string& var_path, std::string& path,
		std::string& var, bool* is_slash_based=NULL);


	/// The variables container (case-insensitive)
	typedef std::map<std::string, as_value, StringNoCaseLessThen> Variables;

	/// The locals container (TODO: use a std::map here !)
	typedef std::vector<frame_slot>	LocalFrames;

	/// Local variables.
	//
	/// TODO: make private. currently an hack in timers.cpp prevents this.
	///
	LocalFrames m_local_frames;

private:

	/// Variables available in this environment
	Variables _variables;

	as_value m_global_register[4];

	/// function2 uses this (could move to swf_function2 class)
	std::vector<as_value>	m_local_register;

	/// Movie target. 
	character* m_target;

	/// Given a variable name, set its value (no support for path)
	void set_variable_raw(const std::string& path, const as_value& val,
		const ScopeStack& with_stack);

	/// Same of the above, but no support for path.
	as_value get_variable_raw(const std::string& varname,
		const ScopeStack& with_stack) const;


	/// \brief
	/// Return an iterator to the local variable with given name,
	/// or an iterator to it's end() iterator if none found
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @param descend
	///	If true the seek don't stop at local frame top, but
	///	descends in upper frames. By default it is false.
	///
	LocalFrames::iterator findLocal(const std::string& varname, bool descend=false);

	LocalFrames::const_iterator findLocal(const std::string& varname, bool descend=false) const
	{
		return const_cast<as_environment*>(this)->findLocal(varname, descend);
	}

	LocalFrames::iterator endLocal() {
		return m_local_frames.end();
	}

	LocalFrames::const_iterator endLocal() const {
		return m_local_frames.end();
	}

	LocalFrames::iterator beginLocal() {
		return m_local_frames.begin();
	}

	LocalFrames::const_iterator beginLocal() const {
		return m_local_frames.begin();
	}

	/// Find an object referenced by the given path (slash syntax).
	//
	/// Supports /slash/syntax 
	///
	/// Return NULL if path doesn't point to an object.
	///
	as_object* find_object_slashsyntax(const std::string& path) const;

	/// Find an object referenced by the given path (dot syntax).
	//
	/// Supports dot.syntax
	///
	/// Return NULL if path doesn't point to an object.
	///
	as_object* find_object_dotsyntax(const std::string& path) const;

};


} // end namespace gnash


#endif // GNASH_AS_ENVIRONMENT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
