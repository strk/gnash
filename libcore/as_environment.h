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

#ifndef GNASH_AS_ENVIRONMENT_H
#define GNASH_AS_ENVIRONMENT_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_value.h" // for composition (vector + frame_slot)
#include "SafeStack.h"

#include <string> // for frame_slot name
#include <vector>

namespace gnash {

// Forward declarations
class DisplayObject;
class VM;
class Global_as;
class movie_root;
class string_table;
class UserFunction;

/// ActionScript execution environment.
class as_environment
{

public:

    /// A stack of objects used for variables/members lookup
    typedef std::vector<as_object*> ScopeStack;

    as_environment(VM& vm);

    VM& getVM() const { return _vm; }

    DisplayObject* get_target() const { return m_target; }

    /// Set default target for timeline opcodes
    //
    /// @param target
    /// A DisplayObject to apply timeline opcodes on.
    /// Zero is a valid target, disabling timeline
    /// opcodes (would get ignored).
    ///
    void set_target(DisplayObject* target);

    void set_original_target(DisplayObject* target) {
        _original_target = target;
    }

    DisplayObject* get_original_target() { return _original_target; }

    // Reset target to its original value
    void reset_target() { m_target = _original_target; }

    /// Push a value on the stack
    void push(const as_value& val) {
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
    }

    /// Get stack value at the given distance from bottom.
    //
    /// bottom(stack_size()-1) is actual stack top
    ///
    /// Throw StackException if index is out of range
    ///
    as_value& bottom(size_t index) const
    {
        try {
            return _stack.value(index);
        } catch (StackException&) {
            return undefVal;
        }
    }

    /// Drop 'count' values off the top of the stack.
    void drop(size_t count)
    {
        // in case count > stack size, just drop all, forget about
        // exceptions...
        _stack.drop(std::min(count, _stack.size()));
    }

    size_t stack_size() const { return _stack.size(); }

    /// \brief
    /// Delete a variable, w/out support for the path, using
    /// a ScopeStack.
    //
    /// @param varname 
    /// Variable name. Must not contain path elements.
    /// TODO: should be case-insensitive up to SWF6.
    /// NOTE: no case conversion is performed currently,
    ///       so make sure you do it yourself. Note that
    ///       ActionExec performs the conversion
    ///       before calling this method.
    ///
    /// @param scopeStack
    /// The Scope stack to use for lookups.
    bool delVariableRaw(const std::string& varname,
            const ScopeStack& scopeStack);

    /// Return the (possibly UNDEFINED) value of the named var.
    //
    /// @param varname 
    /// Variable name. Can contain path elements.
    /// TODO: should be case-insensitive up to SWF6.
    /// NOTE: no case conversion is performed currently,
    ///       so make sure you do it yourself. Note that
    ///       ActionExec performs the conversion
    ///       before calling this method.
    ///
    /// @param scopeStack
    /// The Scope stack to use for lookups.
    ///
    /// @param retTarget
    /// If not NULL, the pointer will be set to the actual object containing the
    /// found variable (if found).
    ///
    as_value get_variable(const std::string& varname,
        const ScopeStack& scopeStack, as_object** retTarget=NULL) const;

    /// \brief
    /// Given a path to variable, set its value.
    //
    /// If no variable with that name is found, a new one is created.
    ///
    /// For path-less variables, this would act as a proxy for
    /// set_variable_raw.
    ///
    /// @param path
    /// Variable path. 
    /// TODO: should be case-insensitive up to SWF6.
    ///
    /// @param val
    /// The value to assign to the variable.
    ///
    /// @param scopeStack
    /// The Scope stack to use for lookups.
    ///
    void set_variable(const std::string& path, const as_value& val,
        const ScopeStack& scopeStack);

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
    DisplayObject* find_target(const std::string& path) const;

    /// Find the object referenced by the given path.
    //
    /// Supports both /slash/syntax and dot.syntax
    /// Case insensitive for SWF up to 6, sensitive from 7 up
    ///
    as_object* find_object(const std::string& path,
            const ScopeStack* scopeStack = 0) const;
    
    /// Return the SWF version we're running for.
    //
    /// NOTE: this is the version encoded in the first loaded
    ///       movie, and cannot be changed during play even if
    ///       replacing the root movie with an externally loaded one.
    ///
    int get_version() const;

private:

    VM& _vm;

    /// Stack of as_values in this environment
    SafeStack<as_value>& _stack;

    /// Movie target. 
    DisplayObject* m_target;

    /// Movie target. 
    DisplayObject* _original_target;

    /// Given a variable name, set its value (no support for path)
    //
    /// If no variable with that name is found, a new one
    /// will be created as a member of current target.
    ///
    /// @param var
    /// Variable name. Can not contain path elements.
    /// TODO: should be case-insensitive up to SWF6.
    ///
    /// @param val
    /// The value to assign to the variable, if found.
    void set_variable_raw(const std::string& path, const as_value& val,
        const ScopeStack& scopeStack);

    /// Same of the above, but no support for path.
    ///
    /// @param retTarget
    /// If not NULL, the pointer will be set to the actual object containing the
    /// found variable (if found).
    ///
    as_value get_variable_raw(const std::string& varname,
        const ScopeStack& scopeStack, as_object** retTarget=NULL) const;

    static as_value undefVal;
        
};

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
///
bool parsePath(const std::string& var_path, std::string& path,
        std::string& var);

inline VM&
getVM(const as_environment& env)
{
    return env.getVM();
}

movie_root& getRoot(const as_environment& env);
string_table& getStringTable(const as_environment& env);
int getSWFVersion(const as_environment& env);
Global_as& getGlobal(const as_environment &env);

} // end namespace gnash


#endif // GNASH_AS_ENVIRONMENT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
