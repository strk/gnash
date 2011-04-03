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

#ifndef GNASH_BUILTIN_FUNCTION_H
#define GNASH_BUILTIN_FUNCTION_H

#include "UserFunction.h" 
#include "fn_call.h"

#include <cassert>

namespace gnash {


/// This is a special type of function implementing AS-code in C++
//
/// Many functions (including classes) are implemented in ActionScript in
/// the reference player. Gnash implements them in C++, but they must
/// be treated like swf-defined functions.
//
/// They are distinct from NativeFunctions, which are part of the player and
/// do not go through the ActionScript interpreter.
class builtin_function : public UserFunction
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
	builtin_function(Global_as& gl, ASFunction func)
		:
		UserFunction(gl),
		_func(func)
	{
	}

    /// Return the number of registers required for function execution
    //
    /// Gnash's C++ implementations of AS functions don't need any registers!
    virtual boost::uint8_t registers() const {
        return 0;
    }

	/// Invoke this function or this Class constructor
	virtual as_value call(const fn_call& fn)
	{
		FrameGuard guard(getVM(fn), *this);

		assert(_func);
		return _func(fn);
	}

	bool isBuiltin()  { return true; }

private:

	ASFunction _func;
};

} // end of gnash namespace

#endif

