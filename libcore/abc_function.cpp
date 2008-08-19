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
#include "fn_call.h"

namespace gnash{

//TODO: Should we create a new machine for each abc_function.
abc_function::abc_function(CodeStream* stream, Machine* machine):as_function(){
		mStream = stream;
		mMachine = machine;
}

// Dispatch.
as_value
abc_function::operator()(const fn_call& fn)
{
	log_debug("Calling an abc_function.");
	mMachine->executeFunction(mStream);
	return as_value();
}


}