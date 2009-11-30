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

#include "CallStack.h"
#include "as_object.h"
#include "as_function.h" 
#include "Global_as.h" 

#include <ostream>

namespace gnash {

CallFrame::CallFrame(as_function* f)
    :
    _locals(new as_object(getGlobal(*f))),
    _func(f)
{
    assert(_func);
}

/// Mark all reachable resources
//
/// Reachable resources would be registers and
/// locals (expected to be empty?) and function.
void
CallFrame::markReachableResources() const
{
    // Func is always valid.
    assert(_func);
    _func->setReachable();

    std::for_each(_registers.begin(), _registers.end(),
            std::mem_fun_ref(&as_value::setReachable));

    if (_locals) _locals->setReachable();
}

std::ostream&
operator<<(std::ostream& o, const CallFrame& fr)
{
    CallFrame::Registers r = fr._registers;

    for (size_t i = 0; i < r.size(); ++i) {
        if (i) o << ", ";
        o << i << ':' << '"' << r[i] << '"';
    }
    return o;
    
}
} // namespace gnash
