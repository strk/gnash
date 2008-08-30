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

#ifndef GNASH_AS_ENVIRONMENT_H
#define GNASH_AS_ENVIRONMENT_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "dsodefs.h" // DSOEXPORT
#include "as_value.h" // for composition (vector + frame_slot)
#include "StringPredicates.h" // for Variables 
#include "as_object.h"
#include "SafeStack.h"

#include <map> // for composition (Variables)
#include <string> // for frame_slot name
#include <vector>
#include <iostream> // for dump_stack inline

namespace gnash {

// Forward declarations
class character;
//class with_stack_entry;

/// ActionScript execution environment.
class as_environment
{
public:

	/// A stack of objects used for variables/members lookup
	//typedef std::vector<with_stack_entry> ScopeStack;
	typedef std::vector< boost::intrusive_ptr<as_object> > ScopeStack;

	/// Stack of as_values in this environment
	//std::vector<as_value>	m_stack;
	SafeStack<as_value>	_stack;

	as_environment()
		:
		_stack(),
		m_target(0),
		_original_target(0)
	{
	}

	character* get_target() { return m_target; }

	/// Set default target for timeline opcodes
	//
	/// @param target
	///	A character to apply timeline opcodes on.
	///	Zero is a valid target, disabling timeline
	///	opcodes (would get ignored).
	///
	void set_target(character* target);

	void set_original_target(character* target) { _original_target = target; }

	character* get_original_target() { return _original_target; }

	// Reset target to its original value
	void reset_target() { m_target = _original_target; }

	/// @{ Stack access/manipulation

	/// Push a value on the stack
	void	push(const as_value& val)
	{
		_stack.push(val);
	}


	/// Pops an as_value off the stack top and return it.
	as_value pop()
	{
		try {
			return _stack.pop();
		} catch (StackException&) {
			return undefVal;
		}
		//assert( ! _stack.empty() );
		//as_value result = m_stack.back();
		//m_stack.pop_back();
		//return result;
	}

	/// Get stack value at the given distance from top.
	//
	/// top(0) is actual stack top
	///
	/// Throw StackException if index is out of range
	///
	as_value& top(size_t dist)
	{
		try {
			return _stack.top(dist);
		} catch (StackException&) {
			return undefVal;
		}
		//size_t ssize = m_stack.size();
		//assert ( ssize > dist );
		//return m_stack[ssize - 1 - dist];
	}

	/// Get stack value at the given distance from bottom.
	//
	/// bottom(stack_size()-1) is actual stack top
	///
	/// Throw StackException if index is out of range
	///
	as_value& bottom(size_t index)
	{
		try {
			return _stack.value(index);
		} catch (StackException&) {
			return undefVal;
		}
		//assert ( m_stack.size() > index );
		//return m_stack[index];
	}

	/// Drop 'count' values off the top of the stack.
	//
	/// Throw StackException if there's not enough to drop
	///
	void drop(size_t count)
	{
		// in case count > stack size, just drop all, forget about
		// exceptions...
		_stack.drop(std::min(count, static_cast<size_t>(_stack.size())));
		//size_t ssize = m_stack.size();
		//assert ( ssize >= count );
		//m_stack.resize(ssize - count);
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
	int	get_top_index() const { return _stack.size() - 1; }

	size_t stack_size() const { return _stack.size(); }

	/// @}  stack access/manipulation
	///

	/// \brief
	/// Return the (possibly UNDEFINED) value of the named variable
	//
	/// @param varname 
	///	Variable name. Can contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///	NOTE: no case conversion is performed currently,
	///	      so make sure you do it yourself. Note that
	///	      ActionExec performs the conversion
	///	      before calling this method.
	///
	as_value get_variable(const std::string& varname) const;

	/// \brief
	/// Delete a variable, w/out support for the path, using
	/// a ScopeStack.
	//
	/// @param varname 
	///	Variable name. Can not contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///	NOTE: no case conversion is performed currently,
	///	      so make sure you do it yourself. Note that
	///	      ActionExec performs the conversion
	///	      before calling this method.
	///
	/// @param scopeStack
	///	The Scope stack to use for lookups.
	///
	bool del_variable_raw(const std::string& varname,
			const ScopeStack& scopeStack);

	/// Return the (possibly UNDEFINED) value of the named var.
	//
	/// @param varname 
	///	Variable name. Can contain path elements.
	///	TODO: should be case-insensitive up to SWF6.
	///	NOTE: no case conversion is performed currently,
	///	      so make sure you do it yourself. Note that
	///	      ActionExec performs the conversion
	///	      before calling this method.
	///
	/// @param scopeStack
	///	The Scope stack to use for lookups.
	///
	/// @param retTarget
	///	If not NULL, the pointer will be set to the actual object containing the
	///	found variable (if found).
	///
	as_value get_variable(const std::string& varname,
		const ScopeStack& scopeStack, as_object** retTarget=NULL) const;

	/// \brief
	/// Given a path to variable, set its value.
	/// Variable name can contain path elements.
	//
	/// @param path 
	///	Variable path.
	///	TODO: should be case-insensitive up to SWF6.
	///	NOTE: no case conversion is performed currently,
	///	      so make sure you do it yourself. Note that
	///	      ActionExec performs the conversion
	///	      before calling this method.
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
	/// @param scopeStack
	///	The Scope stack to use for lookups.
	///
	void set_variable(const std::string& path, const as_value& val,
		const ScopeStack& scopeStack);

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
	void set_local(const std::string& varname, const as_value& val);

	/// \brief
	/// Add a local var with the given name and value to our
	/// current local frame. 
	///
	/// Use this when you know the var
	/// doesn't exist yet, since it's faster than set_local();
	/// e.g. when setting up args for a function.
	///
	void add_local(const std::string& varname, const as_value& val);

	/// Create the specified local var if it doesn't exist already.
	void	declare_local(const std::string& varname);

	/// Add 'count' local registers (add space to end)
	//
	/// Local registers are only meaningful within a function2 context.
	///
	void add_local_registers(unsigned int register_count)
	{
		assert(!_localFrames.empty());
		return _localFrames.back().registers.resize(register_count);
	}

	/// Set value of a register (local or global).
	//
	/// When not in a function context the register will be
	/// global or none (if regnum is not in the valid range
	/// of global registers).
	///
	/// When in a function context defining NO registers, 
	/// we'll behave the same as for a non-function context.
	///
	/// When in a function context defining non-zero number
	/// of local registers, the register set will be local
	/// or none (if regnum is not in the valid range of local
	/// registers).
	///
	/// @param regnum
	///	Register number
	///
	/// @param v
	///	Value to assign to the register
	///
	/// @return 0 if register num is invalid
	///         1 if a global register was set
	///         2 if a local register was set
	///
	unsigned int setRegister(unsigned int regnum, const as_value& v);

	/// Get value of a register (local or global).
	//
	/// When not in a function context the register will be
	/// global or none (if regnum is not in the valid range
	/// of global registers).
	///
	/// When in a function context defining NO registers, 
	/// we'll behave the same as for a non-function context.
	///
	/// When in a function context defining non-zero number
	/// of local registers, the register set will be local
	/// or none (if regnum is not in the valid range of local
	/// registers).
	///
	/// @param regnum
	///	Register number
	///
	/// @param v
	///	Output parameter, will be set to register
	///     value or untouched if 0 is returned.
	///
	/// @return 0 if register num is invalid (v unmodified in this case)
	///         1 if a global register was set
	///         2 if a local register was set
	///
	unsigned int getRegister(unsigned int regnum, as_value& v);

	/// Return a reference to the Nth local register.
	//
	/// Local registers are only meaningful within a function2 context.
	///
	as_value& local_register(boost::uint8_t n)
	{
		assert(!_localFrames.empty());
		return _localFrames.back().registers[n];
	}

        /// Set the Nth local register to something
        void set_local_register(boost::uint8_t n, as_value &val)
	{
		if ( ! _localFrames.empty() )
		{
			Registers& registers = _localFrames.back().registers;
			if ( n < registers.size() )
			{
				registers[n] = val;
			}
		}
	}

	/// Return a reference to the Nth global register.
	as_value& global_register(unsigned int n)
	{
		assert(n<4);
		return m_global_register[n];
	}

        /// Set the Nth local register to something
        void set_global_register(boost::uint8_t n, as_value &val) {
	    if (n <= 4) {
		m_global_register[n] = val;
	    }
	}

#ifdef GNASH_USE_GC
	/// Mark all reachable resources.
	//
	/// Reachable resources from an as_environment
	/// would be global registers, stack (expected to be empty
	/// actually), stack frames and targets (original and current).
	///
	void markReachableResources() const;
#endif

	/// Find the sprite/movie referenced by the given path.
	//
	/// Supports both /slash/syntax and dot.syntax
	/// Case insensitive for SWF up to 6, sensitive from 7 up
	///
	character* find_target(const std::string& path) const;

	/// Find the object referenced by the given path.
	//
	/// Supports both /slash/syntax and dot.syntax
	/// Case insensitive for SWF up to 6, sensitive from 7 up
	///
	as_object* find_object(const std::string& path, const ScopeStack* scopeStack=NULL) const;

	/// Dump content of the stack to a std::ostream
	//
	/// @param out
	///	The output stream, standard error if omitted.
	///
	/// @param limit
	///	If > 0, limit number of printed item by the given amount (from the top).
	///	Unlimited by default;
	///
	void dump_stack(std::ostream& out=std::cerr, unsigned int limit=0) const;

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

	/// See if the given variable name is actually a sprite path
	/// followed by a variable name.  These come in the format:
	///
	///	/path/to/some/sprite/:varname
	///
	/// (or same thing, without the last '/')
	///
	/// or
	///	path.to.some.var
	///
	/// If that's the format, puts the path part (no colon or
	/// trailing slash) in *path, and the varname part (no colon, no dot)
	/// in *var and returns true.
	///
	/// If no colon or dot, returns false and leaves *path & *var alone.
	///
	/// TODO: return an integer: 0 not a path, 1 a slash-based path, 2 a dot-based path
	///
	static bool parse_path(const std::string& var_path, std::string& path,
			std::string& var);

	/// \brief
	/// Try to parse a string as a variable path
	//
	/// Variable paths come in the form:
	///
	/// 	/path/to/some/sprite/:varname
	///
	/// or
	///
	///     /path/to/some/sprite 
	///
	/// or
	///	path.to.some.var
	///
	/// If there's no dot nor colon, or if the 'path' part
	/// does not resolve to an object, this function returns false.
	/// Otherwise, true is returned and 'target' and 'val'
	/// parameters are appropriaterly set.
	///
	/// Note that if the parser variable name doesn't exist in the found
	/// target, the 'val' will be undefined, but no other way to tell whether
	/// the variable existed or not from the caller...
	///
	bool parse_path(const std::string& var_path, as_object** target, as_value& val);

	/// The variables container (case-insensitive)
	typedef std::map<std::string, as_value, StringNoCaseLessThen> Variables;

	/// The locals container 
	//typedef std::vector<frame_slot>	LocalVars;
	typedef boost::intrusive_ptr<as_object> LocalVars;

	typedef std::vector<as_value> Registers;

	struct CallFrame
	{
		CallFrame(as_function* funcPtr);

		CallFrame(const CallFrame& other) : locals(other.locals),
			registers(other.registers), func(other.func)
		{/**/}

		/// function use this 
		LocalVars locals;

		/// function2 also use this
		Registers registers;

		as_function* func;

#ifdef GNASH_USE_GC
		/// Mark all reachable resources
		//
		/// Reachable resources would be registers and
		/// locals (expected to be empty?) and function.
		void markReachableResources() const;
#endif // GNASH_USE_GC
	};

	/// A class to wrap frame access.  Stack allocating a frame guard
	/// will ensure that all CallFrame pushes have a corresponding
	/// CallFrame pop, even in the presence of extraordinary returns.
	class FrameGuard
	{
	public:
		FrameGuard(as_function* func)
		{ as_environment::pushCallFrame(func); }
		~FrameGuard() { as_environment::popCallFrame(); }
	};

	/// Get top element of the call stack
	//
	CallFrame& topCallFrame()
	{
		assert(!_localFrames.empty());
		return _localFrames.back();
	}

	/// Return the depth of call stack
	static size_t callStackDepth()
	{
		return _localFrames.size();
	}

private:

	static const short unsigned int numGlobalRegisters = 4;

	typedef std::vector<CallFrame> CallStack;
		
	static CallStack _localFrames;

	as_value m_global_register[numGlobalRegisters];

	/// Movie target. 
	character* m_target;

	/// Movie target. 
	character* _original_target;

	/// Push a frame on the calls stack.
	//
	/// This should happen right before calling an ActionScript
	/// function. Function local registers and variables
	/// must be set *after* pushCallFrame has been invoked
	///
	/// Call popCallFrame() at ActionScript function return.
	///
	/// @param func
	///	The function being called
	///
	static DSOEXPORT void pushCallFrame(as_function* func);

	/// Remove current call frame from the stack
	//
	/// This should happen when an ActionScript function returns.
	///
	static DSOEXPORT void popCallFrame();
	
	/// Return the (possibly UNDEFINED) value of the named variable.
	//
	/// @param varname 
	///	Variable name. Can not contain path elements.
	///	NOTE: no case conversion is performed currently,
	///	      so make sure you do it yourself. 
	///
	as_value get_variable_raw(const std::string& varname) const;

	/// Given a variable name, set its value (no support for path)
	void set_variable_raw(const std::string& path, const as_value& val,
		const ScopeStack& scopeStack);

	/// Same of the above, but no support for path.
	///
	/// @param retTarget
	///	If not NULL, the pointer will be set to the actual object containing the
	///	found variable (if found).
	///
	as_value get_variable_raw(const std::string& varname,
		const ScopeStack& scopeStack, as_object** retTarget=NULL) const;


	/// \brief
	/// Get a local variable given its name,
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @param ret
	///	If a variable is found it's assigned to this parameter.
	///	Untouched if the variable is not found.
	///
	/// @param retTarget
	///	If not NULL, the pointer will be set to the actual object containing the
	///	found variable (if found).
	///
	/// @return true if the variable was found, false otherwise
	///
	bool findLocal(const std::string& varname, as_value& ret, as_object** retTarget=NULL);

	bool findLocal(const std::string& varname, as_value& ret, as_object** retTarget=NULL) const
	{
		return const_cast<as_environment*>(this)->findLocal(varname, ret, retTarget);
	}

	/// Find a variable in the given LocalVars
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @param ret
	///	If a variable is found it's assigned to this parameter.
	///	Untouched if the variable is not found.
	///
	/// @return true if the variable was found, false otherwise
	///
	static bool findLocal(LocalVars& locals, const std::string& name, as_value& ret);

	/// Delete a local variable
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @return true if the variable was found and deleted, false otherwise
	///
	bool delLocal(const std::string& varname);

	/// Delete a variable from the given LocalVars
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @return true if the variable was found, false otherwise
	///
	static bool delLocal(LocalVars& locals, const std::string& varname);

	/// Set a local variable, if it exists.
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @param val
	///	Value to assign to the variable
	///
	/// @return true if the variable was found, false otherwise
	///
	bool setLocal(const std::string& varname, const as_value& val);

	/// Set a variable from the given LocalVars, if it exists.
	//
	/// @param varname
	///	Name of the local variable
	///
	/// @param val
	///	Value to assign to the variable
	///
	/// @return true if the variable was found, false otherwise
	///
	static bool setLocal(LocalVars& locals, const std::string& varname, const as_value& val);

	static as_value undefVal;
};


} // end namespace gnash


#endif // GNASH_AS_ENVIRONMENT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
