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

#include "log.h"
#include "swf_function.h"
#include "fn_call.h"
#include "MovieClip.h"
#include "action_buffer.h"
#include "ActionExec.h" // for operator()
#include "VM.h" // for storing _global in a local register
#include "NativeFunction.h" // for Function constructor
#include "Global_as.h" 
#include "namedStrings.h"

#include <typeinfo>
#include <iostream>
#include <string>

namespace gnash {

namespace {

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
	as_object* getArguments(swf_function& callee, const fn_call& fn,
            as_object* caller);
}

swf_function::swf_function(const action_buffer& ab, as_environment& env,
			size_t start, const ScopeStack& scopeStack)
	:
	as_function(getGlobal(env)),
	m_action_buffer(ab),
	m_env(env),
	_scopeStack(scopeStack),
	m_start_pc(start),
	m_length(0),
	m_is_function2(false),
	m_local_register_count(0),
	m_function2_flags(0)
{
	assert( m_start_pc < m_action_buffer.size() );

    // We're stuck initializing our own prototype at the moment.
    as_object* proto = getGlobal(env).createObject();
    proto->init_member(NSV::PROP_CONSTRUCTOR, this); 
    init_member(NSV::PROP_PROTOTYPE, proto);
	init_member(NSV::PROP_CONSTRUCTOR, as_function::getFunctionConstructor());
}


/// Exception safe (scoped) as_environment's target changer
//
/// This class partially helps fixing an SWF5 scoping issue.
/// Completing the fix would likely reduce complexity of the
/// code too, in particular the concept of an 'original target'
/// in as_environment seems bogus, see setProperty.as, the 
/// failure of setTarget("") construct in SWF5.
///
struct TargetGuard {
	as_environment& env;
	DisplayObject* from;
	DisplayObject* from_orig;

	// @param ch : target to set temporarely
	// @param och : original target to set temporarely
	TargetGuard(as_environment& e, DisplayObject* ch, DisplayObject* och)
		:
		env(e),
        from(env.get_target()),
        from_orig(env.get_original_target())
	{
		env.set_target(ch);
		env.set_original_target(och);
	}


	~TargetGuard()
	{
		env.set_target(from);
		env.set_original_target(from_orig);
	}
};

// Dispatch.
as_value
swf_function::call(const fn_call& fn)
{
    // Extract caller before pushing ourself on the call stack
    as_object* caller = 0;
    VM& vm = getVM(fn); 
    CallStack& cs = vm.getCallStack();
    if ( ! cs.empty() ) caller = cs.back().func;

	// Set up local stack frame, for parameters and locals.
	as_environment::FrameGuard guard(m_env, this);

	DisplayObject* target = m_env.get_target();
	DisplayObject* orig_target = m_env.get_original_target();

	// Some features are version-dependant.
	const int swfversion = vm.getSWFVersion();
	as_object *super = NULL;
	if (swfversion > 5) {
		super = fn.super;
	}
	else {
		// In SWF5, when 'this' is a DisplayObject it becomes
		// the target for this function call.
		// See actionscript.all/setProperty.as
		// 
		if (fn.this_ptr) {
			DisplayObject* ch = get<DisplayObject>(fn.this_ptr);
			if (ch) {
				target = ch;
				orig_target = ch;
			}
		}
	}

	/// This is only needed for SWF5 (temp switch of target)
	/// We do always and base 'target' value on SWF version.
	/// TODO: simplify code by maybe using a custom as_environment
	///       instead, so to get an "original" target being 
	///       the one set now (rather then the really original one)
	/// TODO: test scope when calling functions defined in another timeline
	///       (target, in particular).
	///
	TargetGuard targetGuard(m_env, target, orig_target);

	if (!m_is_function2)
	{
		// Conventional function.

		// Push the arguments onto the local frame.
		for (size_t i=0, n=m_args.size(); i<n; ++i)
		{
			assert(m_args[i].m_register == 0);
			if (i < fn.nargs) {
				m_env.add_local(m_args[i].m_name, fn.arg(i));
			}
			else {
				// Still declare named arguments, even if
				// they are not passed from caller
				// See bug #22203
				m_env.declare_local(m_args[i].m_name);
			}
		}

		// Add 'this'
		assert(fn.this_ptr);
		m_env.set_local("this", fn.this_ptr);

		// Add 'super' (SWF6+ only)
		if (super && swfversion > 5) {
			m_env.set_local("super", as_value(super));
		}

		// Add 'arguments'
		m_env.set_local("arguments", getArguments(*this, fn, caller));
	}
	else
	{
		// function2: most args go in registers; any others get pushed.
		
		// Create local registers.
		m_env.add_local_registers(m_local_register_count);

		// Handle the implicit args.
		// @@ why start at 1 ? Note that starting at 0 makes	
		// intro.swf movie fail to play correctly.
		unsigned int current_reg = 1;
		if ((m_function2_flags & PRELOAD_THIS) &&
                !(m_function2_flags & SUPPRESS_THIS)) {
			// preload 'this' into a register.
			m_env.setRegister(current_reg, as_value(fn.this_ptr)); 
			current_reg++;
		}

		if (!(m_function2_flags & SUPPRESS_THIS)) {
			// Put 'this' in a local var.
			m_env.add_local("this", as_value(fn.this_ptr));
		}

		// Init arguments array, if it's going to be needed.
        as_object* arg_array = 0;

		if ((m_function2_flags & PRELOAD_ARGUMENTS) || 
                !(m_function2_flags & SUPPRESS_ARGUMENTS)) {
			arg_array = getArguments(*this, fn, caller);
		}

		if (m_function2_flags & PRELOAD_ARGUMENTS) {
			// preload 'arguments' into a register.
			m_env.setRegister(current_reg, as_value(arg_array));
			current_reg++;
		}

		if (!(m_function2_flags & SUPPRESS_ARGUMENTS)) {
			// Put 'arguments' in a local var.
			m_env.add_local("arguments", as_value(arg_array));
		}

		if ((m_function2_flags & PRELOAD_SUPER) && swfversion > 5) {
			// Put 'super' in a register (SWF6+ only).
			// TOCHECK: should we still set it if not available ?
			if ( super ) {
				m_env.setRegister(current_reg, as_value(super));
				current_reg++;
			}
		}

		if (!(m_function2_flags & SUPPRESS_SUPER)) {
		    if (super && swfversion > 5) {
                // TOCHECK: should we still set it if unavailable ?
                // Put 'super' in a local var (SWF6+ only)
                m_env.add_local("super", as_value(super));
            }
		}

		if (m_function2_flags & PRELOAD_ROOT) {
			// Put '_root' (if any) in a register.
			DisplayObject* tgtch = m_env.get_target();
			if (tgtch) {
				// NOTE: _lockroot will be handled by getAsRoot()
				as_object* r = getObject(tgtch->getAsRoot());
				m_env.setRegister(current_reg, as_value(r));
				++current_reg;
			}
		}

		if (m_function2_flags & PRELOAD_PARENT) {
			DisplayObject* tgtch = m_env.get_target();
            if (tgtch) {
                as_object* parent = getObject(tgtch->get_parent());
                m_env.setRegister(current_reg, parent);
                ++current_reg;
            }
		}

		if (m_function2_flags & PRELOAD_GLOBAL) {
			// Put '_global' in a register.
			as_object* global = vm.getGlobal();
			m_env.setRegister(current_reg, as_value(global));
			++current_reg;
		}

		// Handle the explicit args.
		// This must be done after implicit ones,
		// as the explicit override the implicits:
        // see swfdec/definefunction2-override
		for (size_t i = 0, n = m_args.size(); i < n; ++i) {
            // not a register, declare as local
			if (!m_args[i].m_register) {
				if (i < fn.nargs) {
					// Conventional arg passing: create a local var.
					m_env.add_local(m_args[i].m_name, fn.arg(i));
				}
				else {
					// Still declare named arguments, even if
					// they are not passed from caller
					// See bug #22203
					m_env.declare_local(m_args[i].m_name);
				}
			}
			else {
				if (i < fn.nargs) {
					// Pass argument into a register.
					const int reg = m_args[i].m_register;
					m_env.setRegister(reg, fn.arg(i));
				}
                // If no argument was passed, no need to setup a register
                // I guess.
			}
		}
	}

	as_value result;

	// Execute the actions.
	// Do this in a try block to proper drop the pushed call frame 
	// in case of problems (most interesting action limits)
	try 
	{
		ActionExec exec(*this, m_env, &result, fn.this_ptr);
		exec();
	}
	catch (ActionLimitException& ale) // expected and sane 
	{
		//log_debug("ActionLimitException got from swf_function execution: %s", ale.what());
		throw;
	}
	catch (std::exception& ex) // unexpected but we can tell what it is
	{
		log_debug("Unexpected exception from swf_function execution: %s",
                ex.what());
		throw;
	}
	catch (...) // unexpected, unknown, but why not cleaning up...
	{
		log_debug("Unknown exception got from swf_function execution");
		throw;
	}

        return result;
}

void
swf_function::set_length(int len)
{
	assert(len >= 0);
	assert(m_start_pc+len <= m_action_buffer.size());
	m_length = len;
}

#ifdef GNASH_USE_GC
void
swf_function::markReachableResources() const
{
	// Mark scope stack objects
	for (ScopeStack::const_iterator i = _scopeStack.begin(),
            e = _scopeStack.end(); i != e; ++i)
	{
		(*i)->setReachable();
	}

	m_env.markReachableResources();

	// Invoke parent class marker
	markAsFunctionReachable(); 
}
#endif // GNASH_USE_GC

namespace {

as_object* 
getArguments(swf_function& callee, const fn_call& fn,
        as_object* caller)
{ 

	as_object* arguments = getGlobal(fn).createArray();
	for (size_t i = 0; i < fn.nargs; ++i) {
		callMethod(arguments, NSV::PROP_PUSH, fn.arg(i));
	}

	arguments->init_member(NSV::PROP_CALLEE, &callee);

	arguments->init_member(NSV::PROP_CALLER, caller);

	return arguments;

}

}
} // end of gnash namespace

