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

#ifndef GNASH_VM_CALL_STACK_H
#define GNASH_VM_CALL_STACK_H

#include <vector>

#include "as_value.h"

// Forward declarations
namespace gnash {
    class as_object;
    struct ObjectURI;
    class UserFunction;
}

namespace gnash {

/// A CallFrame is an element of a CallStack.
//
/// A CallFrame exists for the duration of a UserFunction call. NativeFunctions
/// have no call frame.
//
/// The CallFrame provides space for local registers and local variables.
/// These values are discarded when the CallFrame is destroyed at the end of
/// a function call. It provides a scope for the function's execution.
class CallFrame
{
public:

    typedef std::vector<as_value> Registers;

    /// Construct a CallFrame for a specific UserFunction
    //
    /// @param func     The UserFunction to create the CallFrame for. This
    ///                 must provide information about the amount of registers
    ///                 to allocate.
    CallFrame(UserFunction* func);
    
    /// Copy constructor for containers
    CallFrame(const CallFrame& other)
        :
        _locals(other._locals),
        _func(other._func),
        _registers(other._registers)
    {}

    /// Assignment operator for containers.
    CallFrame& operator=(const CallFrame& other) {
        _locals = other._locals;
        _func = other._func;
        _registers = other._registers;
        return *this;
    }

    /// Access the local variables for this function call.
    as_object& locals() {
        return *_locals;
    }

    /// Get the function for which this CallFrame provides a scope.
    UserFunction& function() {
        return *_func;
    }

    /// Get a specific register in this CallFrame
    //
    /// @param i    The index of the register to return.
    /// @return     A pointer to the value in the register or 0 if no such
    ///             register exists.
    const as_value* getLocalRegister(size_t i) const {
        if (i >= _registers.size()) return 0;
        return &_registers[i];
    }

    /// Set a specific register in this CallFrame
    //
    /// If the register doesn't exist, nothing happens.
    //
    /// @param i    The index of the register to set.
    /// @param val  The value to set the register to.
    void setLocalRegister(size_t i, const as_value& val);

    /// Set the number of registers for this CallFrame.
    //
    /// The number of registers may only be set once! This is to ensure
    /// that pointers to the register values are always valid.
    bool hasRegisters() const {
        return !_registers.empty();
    }

    /// Mark all reachable resources
    //
    /// Reachable resources would be registers and
    /// locals (expected to be empty?) and function.
    void markReachableResources() const;


private:

    friend std::ostream& operator<<(std::ostream&, const CallFrame&);

    /// Local variables.
    as_object* _locals;

    UserFunction* _func;
    
    /// Local registers.
    Registers _registers;

};

/// Declare a local variable in this CallFrame
//
/// The variable is declared and set to undefined if it doesn't exist already.
//
/// @param c    The CallFrame to set the variable in.
/// @param name The name of the variable to declare.
void declareLocal(CallFrame& c, const ObjectURI& name);

/// Set a local variable in this CallFrame
//
/// If the variable does not already exist, it is created.
//
/// @param c    The CallFrame to set the variable in.
/// @param name The name of the variable to set.
/// @param val  The value to set the variable to.
void setLocal(CallFrame& c, const ObjectURI& name, const as_value& val);

typedef std::vector<CallFrame> CallStack;

std::ostream& operator<<(std::ostream& o, const CallFrame& fr);

} // namespace gnash

#endif // GNASH_VM_CALL_STACK_H
