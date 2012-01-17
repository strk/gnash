// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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
#include "abc_function.h"
#include "fn_call.h"
#include "Machine.h"

namespace gnash {
namespace abc {

abc_function::abc_function(Method* methodInfo, Machine* machine)
    :
    as_function(*machine->global()),
    _methodInfo(methodInfo),
    _machine(machine)
{
}

// Dispatch.
as_value
abc_function::call(const fn_call& fn)
{

	log_abc("Calling an abc_function id=%u.", _methodInfo->methodID());
	as_value val = _machine->executeFunction(_methodInfo,fn);
	log_abc("Done calling abc_function id=%u value=%s",
            _methodInfo->methodID(), val);
	return val;

}

} // namespace abc
} // namespace gnash
