// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

	/// Add properties to an 'arguments' object.
	//
	/// The 'arguments' variable is an array with an additional
	/// 'callee' member, set to the function being called.
	as_object* getArguments(swf_function& callee, as_object& args, 
            const fn_call& fn, as_object* caller);
}

swf_function::swf_function(const action_buffer& ab, as_environment& env,
			size_t start, const ScopeStack& scopeStack)
	:
	as_function(getGlobal(env)),
	m_action_buffer(ab),
	_env(env),
	_scopeStack(scopeStack),
	_startPC(start),
	_length(0),
	_isFunction2(false),
	_registerCount(0),
	_function2Flags(0)
{
	assert( _startPC < m_action_buffer.size() );

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
    VM& vm = getVM(fn); 
    CallStack& cs = vm.getCallStack();

    as_object* caller = cs.empty() ? 0 : &cs.back().function();

	// Set up local stack frame, for parameters and locals.
	as_environment::FrameGuard guard(_env, *this);

	DisplayObject* target = _env.get_target();
	DisplayObject* orig_target = _env.get_original_target();

	// Some features are version-dependant.
	const int swfversion = getSWFVersion(fn);

	if (swfversion < 6) {
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
	TargetGuard targetGuard(_env, target, orig_target);

	if (!_isFunction2) {

		// Conventional function.

		// Push the arguments onto the local frame.
		for (size_t i=0, n=_args.size(); i<n; ++i)
		{
			assert(_args[i].reg == 0);
			if (i < fn.nargs) {
				_env.add_local(_args[i].name, fn.arg(i));
			}
			else {
				// Still declare named arguments, even if
				// they are not passed from caller
				// See bug #22203
				_env.declare_local(_args[i].name);
			}
		}

		// Add 'this'
		_env.set_local("this", fn.this_ptr ? fn.this_ptr : as_value());

        as_object* super = fn.super ? fn.super :
            fn.this_ptr ? fn.this_ptr->get_super() : 0;

		// Add 'super' (SWF6+ only)
		if (super && swfversion > 5) {
			_env.set_local("super", super);
		}

		// Add 'arguments'
        as_object* args = getGlobal(fn).createArray();
		_env.set_local("arguments", getArguments(*this, *args, fn, caller));
	}
	else
	{
		// function2: most args go in registers; any others get pushed.
		
		// Create local registers.
		_env.add_local_registers(_registerCount);

		// Handle the implicit args.
		// @@ why start at 1 ? Note that starting at 0 makes	
		// intro.swf movie fail to play correctly.
        size_t current_reg(1);

        // If this is not suppressed it is either placed in a register
        // or set as a local variable, but not both.
		if (!(_function2Flags & SUPPRESS_THIS)) {
            if (_function2Flags & PRELOAD_THIS) {
                // preload 'this' into a register.
                // TODO: check whether it should be undefined or null
                // if this_ptr is null.
                _env.setRegister(current_reg, fn.this_ptr); 
                ++current_reg;
            }
            else {
                // Put 'this' in a local var.
                _env.add_local("this", fn.this_ptr ? fn.this_ptr : as_value());
            }
        }

        // This works slightly differently from 'super' and 'this'. The
        // arguments are only ever either placed in the register or a
        // local variable, but if both preload and suppress arguments flags
        // are set, an empty array is still placed to the register.
        // This seems like a bug in the reference player.
        if (!(_function2Flags & SUPPRESS_ARGUMENTS) ||
                (_function2Flags & PRELOAD_ARGUMENTS)) {
            
            as_object* args = getGlobal(fn).createArray();

            if (!(_function2Flags & SUPPRESS_ARGUMENTS)) {
                getArguments(*this, *args, fn, caller);
            }

            if (_function2Flags & PRELOAD_ARGUMENTS) {
                // preload 'arguments' into a register.
                _env.setRegister(current_reg, args);
                ++current_reg;
            }
            else {
                // Put 'arguments' in a local var.
                _env.add_local("arguments", args);
            }

        }

        // If super is not suppressed it is either placed in a register
        // or set as a local variable, but not both.
        if (swfversion > 5 && !(_function2Flags & SUPPRESS_SUPER)) {
            
            // Put 'super' in a register (SWF6+ only).
            // TOCHECK: should we still set it if not available ?
            as_object* super = fn.super ? fn.super :
                fn.this_ptr ? fn.this_ptr->get_super() : 0;

            if (super && (_function2Flags & PRELOAD_SUPER)) {
				_env.setRegister(current_reg, super);
				current_reg++;
			}
            else if (super) {
                _env.add_local("super", super);
            }
		}

		if (_function2Flags & PRELOAD_ROOT) {
			// Put '_root' (if any) in a register.
			DisplayObject* tgtch = _env.get_target();
			if (tgtch) {
				// NOTE: _lockroot will be handled by getAsRoot()
				as_object* r = getObject(tgtch->getAsRoot());
				_env.setRegister(current_reg, r);
				++current_reg;
			}
		}

		if (_function2Flags & PRELOAD_PARENT) {
			DisplayObject* tgtch = _env.get_target();
            if (tgtch) {
                as_object* parent = getObject(tgtch->get_parent());
                _env.setRegister(current_reg, parent);
                ++current_reg;
            }
		}

		if (_function2Flags & PRELOAD_GLOBAL) {
			// Put '_global' in a register.
			as_object* global = vm.getGlobal();
			_env.setRegister(current_reg, global);
			++current_reg;
		}

		// Handle the explicit args.
		// This must be done after implicit ones,
		// as the explicit override the implicits:
        // see swfdec/definefunction2-override
		for (size_t i = 0, n = _args.size(); i < n; ++i) {
            // not a register, declare as local
			if (!_args[i].reg) {
				if (i < fn.nargs) {
					// Conventional arg passing: create a local var.
					_env.add_local(_args[i].name, fn.arg(i));
				}
				else {
					// Still declare named arguments, even if
					// they are not passed from caller
					// See bug #22203
					_env.declare_local(_args[i].name);
				}
			}
			else {
				if (i < fn.nargs) {
					// Pass argument into a register.
					const int reg = _args[i].reg;
					_env.setRegister(reg, fn.arg(i));
				}
                // If no argument was passed, no need to setup a register
                // I guess.
			}
		}
	}

	// Execute the actions.
	// Do this in a try block to proper drop the pushed call frame 
	// in case of problems (most interesting action limits)
	try 
	{
        as_value result;
		ActionExec exec(*this, _env, &result, fn.this_ptr);
		exec();
        return result;
	}
	catch (ActionLimitException& ale) // expected and sane 
	{
		throw;
	}
}

void
swf_function::set_length(int len)
{
	assert(len >= 0);
	assert(_startPC+len <= m_action_buffer.size());
	_length = len;
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

	_env.markReachableResources();

	// Invoke parent class marker
	markAsObjectReachable(); 
}
#endif // GNASH_USE_GC

namespace {

as_object*
getArguments(swf_function& callee, as_object& args, const fn_call& fn,
        as_object* caller)
{ 

	for (size_t i = 0; i < fn.nargs; ++i) {
		callMethod(&args, NSV::PROP_PUSH, fn.arg(i));
	}

	args.init_member(NSV::PROP_CALLEE, &callee);
	args.init_member(NSV::PROP_CALLER, caller);
    return &args;

}

}
} // end of gnash namespace

