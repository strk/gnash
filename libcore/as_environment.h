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

#ifndef GNASH_AS_ENVIRONMENT_H
#define GNASH_AS_ENVIRONMENT_H

#include <string> 
#include <vector>
#include <algorithm>

#include "as_value.h" 
#include "SafeStack.h"

// Forward declarations
namespace gnash {
    class DisplayObject;
    class VM;
    class Global_as;
    class movie_root;
    class string_table;
}

namespace gnash {


/// Provides information about timeline context.
//
/// This class stores information about the current and original "target"
/// of execution. A target is a MovieClip or other AS-referenceable 
/// DisplayObject. Non-timeline code has no target.
//
/// Why should a timeline context provide access to the VM stack? It shouldn't!
/// TODO: remove stack access proxy functions, SWF version, etc.
class as_environment
{
public:

    /// A stack of objects used for variables/members lookup
    typedef std::vector<as_object*> ScopeStack;

    as_environment(VM& vm);

    VM& getVM() const { return _vm; }

    DisplayObject* target() const { return _target; }

    /// Set default target for timeline opcodes
    //
    /// @param target   A DisplayObject to apply timeline opcodes on.
    ///                 Zero is a valid target, disabling timeline
    ///                 opcodes (would get ignored).
    void set_target(DisplayObject* target) {
        if (!_original_target) _original_target = target;
        _target = target;
    }

    void set_original_target(DisplayObject* target) {
        _original_target = target;
    }

    DisplayObject* get_original_target() const { return _original_target; }

    // Reset target to its original value
    void reset_target() { _target = _original_target; }

    /// Push a value on the stack
    void push(const as_value& val) {
        _stack.push(val);
    }

    /// Pops an as_value off the stack top and return it.
    as_value pop()
    try {
        return _stack.pop();
    }
    catch (const StackException&) {
        return as_value();
    }

    /// Get stack value at the given distance from top.
    //
    /// top(0) is actual stack top
    ///
    /// Throw StackException if index is out of range
    ///
    as_value& top(size_t dist) const 
    try {
        return _stack.top(dist);
    }
    catch (const StackException&) {
        return undefVal;
    }

    /// Drop 'count' values off the top of the stack.
    void drop(size_t count) {
        // in case count > stack size, just drop all, forget about
        // exceptions...
        _stack.drop(std::min(count, _stack.size()));
    }

    size_t stack_size() const { return _stack.size(); }

    /// Mark all reachable resources.
    //
    /// Only the targets are reachable.
    void markReachableResources() const;
    
    /// Return the SWF version we're running for.
    //
    /// This is merely a proxy for VM::getSWFVersion.
    int get_version() const;

private:

    VM& _vm;

    /// Stack of as_values in this environment
    SafeStack<as_value>& _stack;

    /// Movie target. 
    DisplayObject* _target;

    /// Movie target. 
    DisplayObject* _original_target;

    static as_value undefVal;
        
};

/// Return the (possibly undefined) value of the named var.
//
/// @param ctx         Timeline context to use for variable finding.
/// @param varname     Variable name. Can contain path elements.
/// @param scope       The Scope stack to use for lookups.
/// @param retTarget   If not null, the pointer will be set to the actual
///                    object containing the found variable (if found).
as_value getVariable(const as_environment& ctx, const std::string& varname,
    const as_environment::ScopeStack& scope, as_object** retTarget = 0);

/// Given a path to variable, set its value.
//
/// If no variable with that name is found, a new one is created.
///
/// For path-less variables, this would act as a proxy for
/// set_variable_raw.
//
/// @param ctx     Timeline context to use for variable finding.
/// @param path    Variable path. 
/// @param val     The value to assign to the variable.
/// @param scope   The Scope stack to use for lookups.
void setVariable(const as_environment& ctx, const std::string& path,
    const as_value& val, const as_environment::ScopeStack& scope);

/// Delete a variable, without support for the path, using a ScopeStack.
//
/// @param ctx      Timeline context to use for variable finding.
/// @param varname  Variable name. Must not contain path elements.
/// @param scope    The Scope stack to use for lookups.
bool delVariable(const as_environment& ctx, const std::string& varname,
        const as_environment::ScopeStack& scope);

/// See if the given variable name is actually a sprite path
/// followed by a variable name.  These come in the format:
///
/// /path/to/some/sprite/:varname
///
/// (or same thing, without the last '/')
///
/// or
/// path.to.some.var
///
/// If that's the format, puts the path part (no colon or
/// trailing slash) in *path, and the varname part (no colon, no dot)
/// in *var and returns true.
///
/// If no colon or dot, returns false and leaves *path & *var alone.
///
/// TODO: return an integer: 0 not a path, 1 a slash-based path, 2 a
/// dot-based path
DSOEXPORT bool parsePath(const std::string& var_path, std::string& path,
        std::string& var);

/// Find the object referenced by the given path.
//
/// This is exposed to allow AS-scoped lookup from C++.
//
/// Supports both /slash/syntax and dot.syntax
//
/// @param ctx     Timeline context to use for variable finding.
/// @param path    Variable path. 
/// @param scope   The Scope stack to use for lookups.
DSOEXPORT as_object* findObject(const as_environment& ctx, const std::string& path,
        const as_environment::ScopeStack* scope = 0);

/// Find the DisplayObject referenced by the given path.
//
/// Supports both /slash/syntax and dot.syntax. This is a wrapper round
/// findObject().
//
/// Note that only AS-referenceable DisplayObjects can be found by path,
/// so that the returned object (if it exists) will have an associated
/// as_object. This logic can't be reflected in the return type.
DisplayObject* findTarget(const as_environment& env, const std::string& path);

inline VM&
getVM(const as_environment& env)
{
    return env.getVM();
}

movie_root& getRoot(const as_environment& env);
string_table& getStringTable(const as_environment& env);
int getSWFVersion(const as_environment& env);
Global_as& getGlobal(const as_environment &env);

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
