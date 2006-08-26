// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef __GNASH_BUILTIN_FUNCTION_H__
#define __GNASH_BUILTIN_FUNCTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_function.h" // for inheritance

#include <cassert>

namespace gnash {

typedef void (*as_c_function_ptr)(const fn_call& fn);


/// Any built-in function/class should be of this type
class builtin_function : public as_function
{

public:

	/// If 'func' parameter is NULL the function is not
	builtin_function(as_c_function_ptr func, as_object* iface)
		:
		as_function(iface),
		_func(func)
	{
	}

	/// Dispatch.
	virtual void operator()(const fn_call& fn)
	{
		assert(_func);
		_func(fn);
	}

	bool isBuiltin()  { return true; }

private:

	as_c_function_ptr _func;
};

} // end of gnash namespace

// __GNASH_BUILTIN_FUNCTION_H__
#endif

