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
//
#include "CallStack.h"
#include "as_object.h"
#include "as_function.h" 
#include "Global_as.h" 

namespace gnash {

CallFrame::CallFrame(as_function* funcPtr)
	:
	locals(getGlobal(*funcPtr)->createObject()),
	func(funcPtr)
{
}

/// Mark all reachable resources
//
/// Reachable resources would be registers and
/// locals (expected to be empty?) and function.
void
CallFrame::markReachableResources() const
{
	if ( func ) func->setReachable();
	for (Registers::const_iterator i=registers.begin(), e=registers.end(); i!=e; ++i)
	{
		i->setReachable();
	}
	if (locals)
		locals->setReachable();
}

} // namespace gnash
