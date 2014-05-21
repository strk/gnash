// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "Function2.h"

#include "log.h"
#include "fn_call.h"
#include "action_buffer.h"
#include "ActionExec.h"
#include "VM.h" 
#include "NativeFunction.h" 
#include "Global_as.h" 
#include "namedStrings.h"
#include "CallStack.h"
#include "MovieClip.h"
#include "DisplayObject.h"

namespace gnash {

Function2::Function2(const action_buffer& ab, as_environment& env,
			size_t start, const ScopeStack& scopeStack)
	:
	Function(ab, env, start, scopeStack),
	_registerCount(0),
	_function2Flags(0)
{
}

// Dispatch.
as_value
Function2::call(const fn_call& fn)
{
    // Extract caller before pushing ourself on the call stack
    VM& vm = getVM(fn); 

    as_object* caller = vm.calling() ? &vm.currentCall().function() : nullptr;

	// Set up local stack frame, for parameters and locals.
	FrameGuard guard(getVM(fn), *this);
    CallFrame& cf = guard.callFrame();

	DisplayObject* target = _env.target();
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

    // Temporarely restore the ConstantPool which was
    // in effect at the time of function definition
    PoolGuard poolGuard(getVM(_env), _pool);

    // function2: most args go in registers; any others get pushed.

    // Handle the implicit args.
    // @@ why start at 1 ? Note that starting at 0 makes	
    // intro.swf movie fail to play correctly.
    size_t current_reg(1);

    // This is us. TODO: why do we have to query the VM to get
    // what are effectively our own resources?

    // If this is not suppressed it is either placed in a register
    // or set as a local variable, but not both.
    if (!(_function2Flags & SUPPRESS_THIS)) {
        if (_function2Flags & PRELOAD_THIS) {
            // preload 'this' into a register.
            // TODO: check whether it should be undefined or null
            // if this_ptr is null.
            cf.setLocalRegister(current_reg, fn.this_ptr); 
            ++current_reg;
        }
        else {
            // Put 'this' in a local var.
            setLocal(cf, NSV::PROP_THIS,
                    fn.this_ptr ? fn.this_ptr : as_value());
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
            cf.setLocalRegister(current_reg, args);
            ++current_reg;
        }
        else {
            // Put 'arguments' in a local var.
            setLocal(cf, NSV::PROP_ARGUMENTS, args);
        }

    }

    // If super is not suppressed it is either placed in a register
    // or set as a local variable, but not both.
    if (swfversion > 5 && !(_function2Flags & SUPPRESS_SUPER)) {
        
        // Put 'super' in a register (SWF6+ only).
        // TOCHECK: should we still set it if not available ?
        as_object* super = fn.super ? fn.super :
            fn.this_ptr ? fn.this_ptr->get_super() : nullptr;

        if (super && (_function2Flags & PRELOAD_SUPER)) {
            cf.setLocalRegister(current_reg, super);
            current_reg++;
        }
        else if (super) {
            setLocal(cf, NSV::PROP_SUPER, super);
        }
    }

    if (_function2Flags & PRELOAD_ROOT) {
        // Put '_root' (if any) in a register.
        DisplayObject* tgtch = _env.target();
        if (tgtch) {
            // NOTE: _lockroot will be handled by getAsRoot()
            as_object* r = getObject(tgtch->getAsRoot());
            cf.setLocalRegister(current_reg, r);
            ++current_reg;
        }
    }

    if (_function2Flags & PRELOAD_PARENT) {
        DisplayObject* tgtch = _env.target();
        if (tgtch) {
            as_object* p = getObject(tgtch->parent());
            cf.setLocalRegister(current_reg, p);
            ++current_reg;
        }
    }

    if (_function2Flags & PRELOAD_GLOBAL) {
        // Put '_global' in a register.
        as_object* global = vm.getGlobal();
        cf.setLocalRegister(current_reg, global);
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
                setLocal(cf, _args[i].name, fn.arg(i));
            }
            else {
                // Still declare named arguments, even if
                // they are not passed from caller
                // See bug #22203
                declareLocal(cf, _args[i].name);
            }
        }
        else {
            if (i < fn.nargs) {
                // Pass argument into a register.
                const int reg = _args[i].reg;
                cf.setLocalRegister(reg, fn.arg(i));
            }
            // If no argument was passed, no need to setup a register
            // I guess.
        }
    }

	// Execute the actions.
    as_value result;
    ActionExec(*this, _env, &result, fn.this_ptr)();
    return result;
}

} // end of gnash namespace

