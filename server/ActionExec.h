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

#ifndef GNASH_ACTIONEXEC_H
#define GNASH_ACTIONEXEC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "with_stack_entry.h"

#include <vector>

// Forward declarations
namespace gnash {
	class action_buffer;
	class as_environment;
	class as_value;
}

namespace gnash {

/// Executor of an action_buffer 
class ActionExec {

	/// the 'with' stack associated with this execution thread
	std::vector<with_stack_entry> with_stack;

	/// Limit of with stack
	//
	/// This is 7 for SWF up to 5 and 15 for SWF 6 and up
	/// See: http://sswf.sourceforge.net/SWFalexref.html#action_with
	size_t _with_stack_limit;

	bool _function2_var;

public:

	/// The actual action buffer
	//
	/// TODO: provide a getter and make private
	///
	const action_buffer& code;

	/// Program counter (offset of current action tag)
	//
	/// TODO: provide mutator funx and make private
	///
	size_t pc;

	/// End of current function execution
	//
	/// TODO: make private
	///
	size_t stop_pc;

	/// Offset to next action tag
	size_t next_pc;

	/// TODO: provide a getter and make private ?
	as_environment& env;

	/// TODO: provide a setter and make private ?
	as_value* retval;

	/// Create an execution thread 
	ActionExec(const action_buffer& abuf, as_environment& newEnv);

	/// Create an execution thread (for a function call ?)
	//
	/// @param buf
	///	the action code
	///
	/// @param newEnv
	///	the execution environment (variables scope, stack etc.)
	///
	/// @param nStartPC
	///	where to start execution (offset from start of action code)
	///
	/// @param nExecBytes
	///	Number of bytes to run this is probably a redundant
	///	information, as an ActionEnd should tell us when to stop.
	///	We'll keep this parameter as an SWF integrity checker.
	///
	/// @param nRetval
	///	where to return a value, if this is a function call (??)
	///
	/// @param initial_with_stack
	///	the 'with' stack to use
	///
	/// @param nIsFunction2
	///	wheter the given action code is actually a Function2
	///	
	ActionExec(const action_buffer& abuf, as_environment& newEnv,
		size_t nStartPC, size_t nExecBytes, as_value* nRetval,  
		const std::vector<with_stack_entry>& initial_with_stack,
		bool nIsFunction2);

	/// Is this execution thread a function2 call ?
	bool isFunction2() { return _function2_var; }

	/// Returns 'with' stack associated with this execution thread
	// 
	/// If you need to modify it, use the pushWithEntry() function.
	///
	const std::vector<with_stack_entry>& getWithStack() const
	{
		return with_stack;
	}

	/// Return the maximum allowed 'with' stack limit.
	//
	/// See http://sswf.sourceforge.net/SWFalexref.html#action_with
	/// for more info.
	///
	size_t getWithStackLimit() const 
	{
		return _with_stack_limit;
	}

	/// Push an entry to the with stack
	//
	/// @return
	///	true if the entry was pushed,
	///	false otherwise (the stack is limited to 8 slots)
	///	
	bool pushWithEntry(const with_stack_entry& entry);

	/// Skip the specified number of action tags 
	//
	/// The offset is relative to next_pc
	///
	void skip_actions(size_t offset);

	/// Execute.
	void operator() ();
		
};

} // namespace gnash

#endif // GNASH_ACTIONEXEC_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
