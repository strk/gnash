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

#include "Function.h"

#include <algorithm>

#include "log.h"
#include "fn_call.h"
#include "action_buffer.h"
#include "ActionExec.h" 
#include "VM.h" 
#include "NativeFunction.h" 
#include "Global_as.h" 
#include "namedStrings.h"
#include "CallStack.h"
#include "DisplayObject.h"

namespace gnash {

Function::Function(const action_buffer& ab, as_environment& env,
            size_t start, ScopeStack scopeStack)
    :
    UserFunction(getGlobal(env)),
    _env(env),
    _pool(getVM(env).getConstantPool()),
    _action_buffer(ab),
    _scopeStack(std::move(scopeStack)),
    _startPC(start),
    _length(0)
{
    assert( _startPC < _action_buffer.size() );
}

TargetGuard::TargetGuard(as_environment& e, DisplayObject* ch,
        DisplayObject* och)
    :
    env(e),
    from(env.target()),
    from_orig(env.get_original_target())
{
    env.set_target(ch);
    env.set_original_target(och);
}


TargetGuard::~TargetGuard() 
{
    env.set_target(from);
    env.set_original_target(from_orig);
}

// Dispatch.
as_value
Function::call(const fn_call& fn)
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
        DisplayObject* ch = get<DisplayObject>(fn.this_ptr);
        if (ch) {
            target = ch;
            orig_target = ch;
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

    // Push the arguments onto the local frame.
    for (size_t i = 0, n = _args.size(); i < n; ++i) {

        assert(_args[i].reg == 0);
        if (i < fn.nargs) {
            setLocal(cf, _args[i].name, fn.arg(i));
        }
        else {
            // Still declare named arguments, even if
            // they are not passed from caller
            // See bug #22203
            declareLocal(cf, _args[i].name);
        }
    }

    // Add 'this'
    setLocal(cf, NSV::PROP_THIS, fn.this_ptr ? fn.this_ptr : as_value());

    as_object* super = fn.super ? fn.super :
        fn.this_ptr ? fn.this_ptr->get_super() : nullptr;

    // Add 'super' (SWF6+ only)
    if (super && swfversion > 5) {
        setLocal(cf, NSV::PROP_SUPER, super);
    }

    // Add 'arguments'
    as_object* args = getGlobal(fn).createArray();

    // Put 'arguments' in a local var.
    setLocal(cf, NSV::PROP_ARGUMENTS, getArguments(*this, *args, fn, caller));

    // Execute the actions.
    as_value result;
    ActionExec(*this, _env, &result, fn.this_ptr)();
    return result;
}

void
Function::setLength(size_t len)
{
    assert(_startPC + len <= _action_buffer.size());
    _length = len;
}

void
Function::markReachableResources() const
{
    std::for_each(_scopeStack.begin(), _scopeStack.end(),
        std::mem_fun(&as_object::setReachable));

    _env.markReachableResources();

    // Invoke parent class marker
    as_object::markReachableResources(); 
}

as_object*
getArguments(Function& callee, as_object& args, const fn_call& fn,
        as_object* caller)
{ 

    for (size_t i = 0; i < fn.nargs; ++i) {
        callMethod(&args, NSV::PROP_PUSH, fn.arg(i));
    }

    args.init_member(NSV::PROP_CALLEE, &callee);
    args.init_member(NSV::PROP_CALLER, caller);
    return &args;

}

} // end of gnash namespace

