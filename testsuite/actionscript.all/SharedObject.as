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

// Test case for SharedObject ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: SharedObject.as,v 1.10 2007/07/26 03:41:19 strk Exp $";

#include "check.as"

var sharedobjectObj = new SharedObject;

#if OUTPUT_VERSION < 6

// test the SharedObject constuctor
xcheck_equals (typeof(sharedobjectObj), 'object');

// test the SharedObject::getlocal method
check_equals (typeof(sharedobjectObj.getLocal), 'undefined');
xcheck_equals (typeof(SharedObject.getLocal), 'function');

#else // OUTPUT_VERSION >= 6

// test the SharedObject constuctor
check_equals (typeof(sharedobjectObj), 'object');

// test the SharedObject::clear method
check_equals (typeof(sharedobjectObj.clear), 'function');
// test the SharedObject::flush method
check_equals (typeof(sharedobjectObj.flush), 'function');

// test the SharedObject::getlocal method
check_equals (typeof(sharedobjectObj.getLocal), 'undefined');
check_equals (typeof(SharedObject.getLocal), 'function');

// test the SharedObject::getsize method
check_equals (typeof(sharedobjectObj.getSize), 'function');

#endif // OUTPUT_VERSION >= 6
