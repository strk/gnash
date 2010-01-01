// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
    class as_function;
}

namespace gnash {

/// An element of a CallStack
class CallFrame 
{
public:

    typedef std::vector<as_value> Registers;

    CallFrame(as_function* func);

    CallFrame(const CallFrame& other)
        :
        _locals(other._locals),
        _registers(other._registers),
        _func(other._func)
    {}

    as_object& locals() {
        return *_locals;
    }

    as_function& function() {
        return *_func;
    }

    bool getRegister(size_t i, as_value& val) const {
        if (i >= _registers.size()) return false;
        val = _registers[i];
        return true;
    }

    bool setRegister(size_t i, const as_value& val) {
        if (i >= _registers.size()) return false;
        _registers[i] = val;
        return true;
    }

    void resizeRegisters(size_t i) {
        _registers.resize(i);
    }

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

    /// function use this 
    as_object* _locals;

    /// function2 also use this
    Registers _registers;

    as_function* _func;

};

typedef std::vector<CallFrame> CallStack;

std::ostream& operator<<(std::ostream& o, const CallFrame& fr);

} // namespace gnash

#endif // GNASH_VM_CALL_STACK_H
