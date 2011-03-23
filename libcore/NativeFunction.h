// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_NATIVE_FUNCTION_H
#define GNASH_NATIVE_FUNCTION_H

#include "as_function.h" // for inheritance

#include <cassert>

namespace gnash {
    class fn_call;
}

namespace gnash {

/// This class implements functions native to the player.
//
/// They are not implemented in ActionScript in the reference player, but
/// rather have access to internal functions.
//
/// Native functions include methods for handling SharedObjects, NetConnections
/// and MovieClips, but also constructors for types such as String, Number,
/// Array, Boolean, and MovieClip.
class NativeFunction : public as_function
{
    typedef as_value (*ASFunction)(const fn_call& fn);

public:

	/// Construct a builtin function/class with a default interface
	//
	/// The default interface will have a constructor member set as 'this'
	///
	/// @param func
	///	The C function to call when this as_function is invoked.
	/// 	For classes, the function pointer is the constructor.
	///
    NativeFunction(Global_as& gl, ASFunction func)
		:
		as_function(gl),
		_func(func)
	{
	}

	/// Invoke this function
	virtual as_value call(const fn_call& fn)
	{
		assert(_func);
		return _func(fn);
	}

	bool isBuiltin()  { return true; }

private:

	ASFunction _func;
};

} // end of gnash namespace

#endif

