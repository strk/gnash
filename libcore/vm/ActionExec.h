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

#ifndef GNASH_ACTIONEXEC_H
#define GNASH_ACTIONEXEC_H

#include "with_stack_entry.h"
#include "as_environment.h" // for ScopeStack
#include "smart_ptr.h"
#include "swf.h"
#include "action_buffer.h"

#include <vector>

// Forward declarations
namespace gnash {
	class as_value;
	class swf_function;
	class ActionExec;
}

namespace gnash {

class TryBlock
{
public:
	friend class ActionExec;

	enum tryState
	{
		TRY_TRY, // In a try block.
		TRY_CATCH, // In a catch block.
		TRY_FINALLY, // In a finally block.
		TRY_END // Finished with finally
	};

    TryBlock(size_t cur_off, size_t try_size, size_t catch_size,
		size_t finally_size, std::string catchName)
		:
		_catchOffset(cur_off + try_size),
		_finallyOffset(cur_off + try_size + catch_size),
		_afterTriedOffset(cur_off + try_size + catch_size + finally_size),
		_hasName(true),
		_name(catchName),
		_registerIndex(0),
		_tryState(TryBlock::TRY_TRY),
		_lastThrow()
	{}

	TryBlock(size_t cur_off, size_t try_size, size_t catch_size,
		size_t finally_size, boost::uint8_t register_index)
		:
		_catchOffset(cur_off + try_size),
		_finallyOffset(cur_off + try_size + catch_size),
		_afterTriedOffset(cur_off + try_size + catch_size + finally_size),
		_hasName(false),
		_name(),
		_registerIndex(register_index),
		_tryState(TryBlock::TRY_TRY),
		_lastThrow()
	{}

private:
	size_t _catchOffset;
	size_t _finallyOffset;
	size_t _afterTriedOffset;
	size_t _savedEndOffset;
	bool _hasName;
	std::string _name;
	unsigned int _registerIndex;
	tryState _tryState;
	as_value _lastThrow;
};

/// Executor of an action_buffer 
class ActionExec {

private: 

	/// \brief
	/// Debugging function:
	/// print opcodes from start (included) to end (not-included) PCs.
	//
	/// @param start
	///	First opcode to dump
	///
	/// @param end
	///	One-past last opcode to dump
	///
	/// @param os
	///	Output stream to dump to
	///
	void dumpActions(size_t start, size_t end, std::ostream& os);

    /// Processes the current try - catch - finally block
    //
    /// This function is called after each stage of a
    /// try/catch/finally block. It ensures it is called
    /// after each stage by setting stop_pc to the appropriate
    /// number. If an exception is on the stack at any stage,
    /// it takes the appropriate action (catch, set register
    /// values, return, or leave it on the stack). Return
    /// false means that the action processing loop should be
    /// interrupted.
    //
    /// @return whether to continue executing the buffer
    //
    /// @param t the try block to process.
    bool processExceptions(TryBlock& t);

	/// Run after a complete run, or after an run interrupted by 
	/// a bail-out exception (ActionLimitException, for example)
	//
	/// The method restores original target of the as_environment,
	/// checks for stack smashing (stack contains less entries
	/// then it had at time of execution start) or leftovers
	/// (stack contains more entries then it had at time of execution
	/// start) and finally gives movie_root a chance to execute
	/// actions queued in higher priority action queues.
	///
	/// The higher priority action queue flush is needed to allow
	/// initialize/construct/initactions queued by effect of gotoFrame
	/// calls in DOACTION block before frame actions queued by the same
	/// cause (the latter would be pushed in the same level gotoFrame is
	/// found)
	///
	/// @param expectInconsistencies
	///	If true, don't print an error if stack is bigger or smaller
	///	then we expect. The parameter is used when calling the
	///	cleanup function due to a thrown ActionLimitException.
	///
	void cleanupAfterRun(bool expectInconsistencies=false);

	/// the 'with' stack associated with this execution thread
	std::vector<with_stack_entry> _withStack;

	typedef as_environment::ScopeStack ScopeStack;

	/// the scope stack associated with this execution thread
	ScopeStack _scopeStack;

	/// Limit of with stack
	//
	/// This is 7 for SWF up to 5 and 15 for SWF 6 and up
	/// See: http://sswf.sourceforge.net/SWFalexref.html#action_with
	///
	/// Actually, there's likely NO point in using the limit.
	/// The spec say that a player6 is ensured to provide at least 15 elements
	/// in a 'with' stack, while player5 can support at most 7.
	/// There is no provision of a limit though.
	///
	/// Gnash will use this information only to generate useful
	/// warnings for coders (if ActionScript errors verbosity is
	/// enabled).
	///
	size_t _withStackLimit;

	/// A pointer to the function being executed, or NULL
	/// for non-function execution
	///
	/// TODO: 
	/// This should likely be put in a larger
	/// structure including return address 
	/// and maintained in a stack (the call stack)
	///
	const swf_function* _func;

	/// The 'this' pointer, if this is a function call
	boost::intrusive_ptr<as_object> _this_ptr;

	/// Stack size at start of execution
	size_t _initialStackSize;

	/// Call stack depth at start of execution
	size_t _initialCallStackDepth;

	DisplayObject* _originalTarget;

	int _origExecSWFVersion;

	std::list<TryBlock> _tryList;

	bool _returning;

	/// Warn about a stack underrun and fix it 
	//
	/// The fix is padding the stack with undefined
	/// values for the missing slots.
	/// 
	/// @param required
	///	Number of items required.
	///
	void fixStackUnderrun(size_t required);

	bool _abortOnUnload;

    /// Program counter (offset of current action tag)
	size_t pc;

	/// Offset to next action tag
	size_t next_pc;

	/// End of current function execution
	/// Used for try/throw/catch blocks.
	size_t stop_pc;

public:

	/// \brief
	/// Use this to push a try block.
	/// t will be copied
	void pushTryBlock(TryBlock& t);

	/// \brief
	/// Set the return value.
	void pushReturn(const as_value& t);

	/// The actual action buffer
	//
	/// TODO: provide a getter and make private
	///
	const action_buffer& code;

	/// TODO: provide a getter and make private ?
	as_environment& env;

	/// TODO: provide a setter and make private ?
	as_value* retval;

	/// Create an execution thread 
	//
	/// @param abuf
	///	the action code
	///
	/// @param newEnv
	///	the execution environment (variables scope, stack etc.)
	///
	/// @param abortOnUnloaded
	///	If true (the default) execution aborts as soon as the target sprite is unloaded.
	///	NOTE: original target is fetched from the environment.
	///
	ActionExec(const action_buffer& abuf, as_environment& newEnv, bool abortOnUnloaded=true);

	/// Create an execution thread for a function call.
	//
	/// @param func
	///	The function 
	///
	/// @param newEnv
	///	The execution environment (variables scope, stack etc.)
	///
	/// @param nRetval
	///	Where to return a value. If NULL any return will be discarded.
	///
	ActionExec(const swf_function& func, as_environment& newEnv, as_value* nRetVal, as_object* this_ptr);

	/// Is this execution thread a function2 call ?
	bool isFunction2() const;

	/// Is this execution thread a function call ?
	bool isFunction() const { return _func != 0; }

    /// Get a pointer of the function being executed (if any)
    const swf_function* getThisFunction() const { return _func; }

	/// Get the current 'this' pointer, for use in function calls
	as_object* getThisPointer();

	/// Returns the scope stack associated with this execution thread
	//
	/// TODO: return by const ref instead
	///
	const ScopeStack& getScopeStack() const
	{
		return _scopeStack;
	}

	/// Return the maximum allowed 'with' stack limit.
	//
	/// See http://sswf.sourceforge.net/SWFalexref.html#action_with
	/// for more info.
	///
	/// Note that Gnash have NO limit on the number of 'with'
	/// stack entries, this information will only be used to
	/// generate useful warnings for coders (if ActionScript errors
	/// verbosity is enabled).
	///
	size_t getWithStackLimit() const 
	{
		return _withStackLimit;
	}

	/// Push an entry to the with stack
	//
	/// @return
	///	true if the entry was pushed, false otherwise.
	///	Note that false will *never* be returned as Gnash
	///	removed the 'with' stack limit. If the documented
	///	limit for stack (SWF# bound) is reached, and ActionScript
	///	errors verbosity is enabled, a warning will be raised,
	///	but the call will still succeed.
	///	
	bool pushWithEntry(const with_stack_entry& entry);

	/// Skip the specified number of action tags 
	//
	/// The offset is relative to next_pc
	///
	void skip_actions(size_t offset);

	/// Delete named variable, seeking for it in the with stack if any
	//
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	bool delVariable(const std::string& name);

	/// Delete a named object member.
	//
	/// @param obj
	///	The object to remove the member/property from.
	///
	/// @param name
	///	Name of the member. 
	///	Name is converted to lowercase if SWF version is < 7.
	///
	/// @return true if the member was successfully removed, false otherwise.
	///
	bool delObjectMember(as_object& obj, const std::string& name);

	/// Set a named variable, seeking for it in the with stack if any.
	//
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	void setVariable(const std::string& name, const as_value& val);

	/// \brief
	/// If in a function context set a local variable,
	/// otherwise, set a normal variable.
	//
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	void setLocalVariable(const std::string& name, const as_value& val);

	/// Get a named variable, seeking for it in the with stack if any.
	//
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	as_value getVariable(const std::string& name);

	/// Get a named variable, seeking for it in the with stack if any.
	//
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	/// @param target
	///	An output parameter, will be set to point to the object
	///	containing any found variable.
	///	The pointer may be set to NULL if the variable was not 
	///	found or it belongs to no object (absolute references, for instance).
	///
	as_value getVariable(const std::string& name, as_object** target);

	/// Set an object's member.
	//
	/// @param obj
	///	The object we want to set the member of.
	///
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	/// @param val
	///	The value to assign to the object's member.
	///
	void setObjectMember(as_object& obj, const std::string& name, const as_value& val);

	/// Get an object's member.
	//
	/// @param obj
	///	The object we want to set the member of.
	///
	/// @param name
	///	Name of the variable. Supports slash and dot syntax.
	///	Name is converted to lowercase if SWF version is < 7.
	///
	/// @param val
	///	The as_value to write member value into.
	///
	/// @returns
	///	True if the member was found, false otherwise.
	///
	bool getObjectMember(as_object& obj, const std::string& name, as_value& val);

	/// Get current target.
	//
	/// This function returns top 'with' stack entry, if any.
	/// Main use for this function is for calling methods and
	/// properly setting the "this" pointer. 
	///
	/// TODO:
	/// A better, cleaner and less error-prone approach
	/// would be providing a callFunction() method in
	/// ActionExec. This will likely help debugger too
	/// 
	///
	as_object* getTarget();

	/// Execute.
	void operator() ();


    // TODO: cut down these accessors.
    bool atActionTag(SWF::ActionType t) { return code[pc] == t; }
	
	size_t getCurrentPC() const { return pc; }
	
	void skipRemainingBuffer() { next_pc = stop_pc; }
	
	void adjustNextPC(int offset);
	
	size_t getNextPC() const { return next_pc; }
	
	void setNextPC(size_t pc) { next_pc = pc; }
	
	size_t getStopPC() const { return stop_pc; }
	
};

} // namespace gnash

#endif // GNASH_ACTIONEXEC_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
