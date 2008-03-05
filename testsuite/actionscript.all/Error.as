// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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


// Test case for Error ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


#include "check.as"

var errorObj = new Error;

#if OUTPUT_VERSION < 7

// test the Error constuctor
xcheck_equals (typeof(errorObj), 'object');

// test the Error::tostring method
xcheck_equals (typeof(errorObj.toString), 'function');

#else // OUTPUT_VERSION >= 7

// test the Error constuctor
check_equals (typeof(errorObj), 'object');

// test the Error::tostring method
check_equals (typeof(errorObj.toString), 'function');

#endif
totals();
