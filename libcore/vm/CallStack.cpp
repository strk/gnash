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

#include "CallStack.h"

#include <ostream>

#include "as_object.h"
#include "UserFunction.h" 
#include "Property.h"
#include "log.h"

namespace gnash {

CallFrame::CallFrame(UserFunction* f)
    :
    _locals(new as_object(getGlobal(*f))),
    _func(f),
    _registers(_func->registers())
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
    assert(_func);
    _func->setReachable();

    std::for_each(_registers.begin(), _registers.end(),
            std::mem_fun_ref(&as_value::setReachable));

    assert(_locals);
    _locals->setReachable();
}

void
CallFrame::setLocalRegister(size_t i, const as_value& val)
{
    if (i >= _registers.size()) return;

    _registers[i] = val;

    IF_VERBOSE_ACTION(
        log_action(_("-------------- local register[%d] = '%s'"),
            i, val);
    );

}

void
declareLocal(CallFrame& c, const ObjectURI& name)
{
    as_object& locals = c.locals();
    if (!hasOwnProperty(locals, name)) {
        locals.set_member(name, as_value());
    }
}

void
setLocal(CallFrame& c, const ObjectURI& name, const as_value& val)
{
    as_object& locals = c.locals();

    // This way avoids searching the prototype chain, though it seems
    // unlikely that it is an optimization.
    Property* prop = locals.getOwnProperty(name);
    if (prop) {
        prop->setValue(locals, val);
        return;
    }

    locals.set_member(name, val);
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
