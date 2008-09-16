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

#ifndef GNASH_VM_CALL_STACK_H
#define GNASH_VM_CALL_STACK_H

#include <vector>

// Forward declarations
namespace gnash {
    class as_object;
    class as_function;
    class as_value;
}

namespace gnash {

/// An element of a CallStack
struct CallFrame
{
    typedef std::vector<as_value> Registers;


    CallFrame(as_function* funcPtr);

    CallFrame(const CallFrame& other)
        :
        locals(other.locals),
        registers(other.registers),
        func(other.func)
    {/**/}

    /// function use this 
    as_object* locals;

    /// function2 also use this
    Registers registers;

    as_function* func;

    /// Mark all reachable resources
    //
    /// Reachable resources would be registers and
    /// locals (expected to be empty?) and function.
    void markReachableResources() const;
};

typedef std::vector<CallFrame> CallStack;


} // namespace gnash

#endif // GNASH_VM_CALL_STACK_H
