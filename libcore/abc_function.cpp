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

#include "log.h"
#include "abc_function.h"
#include "asClass.h"
#include "fn_call.h"

namespace gnash{

abc_function::abc_function(asMethod *methodInfo, Machine* machine):as_function(){
		mMethodInfo = methodInfo;
		mMachine = machine;
}

// Dispatch.
as_value
abc_function::operator()(const fn_call& fn)
{
	log_debug("Calling an abc_function id=%u.",mMethodInfo->mMethodID);
	as_value val = mMachine->executeFunction(mMethodInfo->getBody(),mMethodInfo->getMaxRegisters(),fn);
	log_debug("Done calling abc_function id=%u value=%s",mMethodInfo->mMethodID,val.toDebugString());
	return val;

}


}