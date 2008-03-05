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

// Test case for Video ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(Video, undefined);

#else

// test Video class an interface availability
check_equals(typeof(Video), 'function');
check_equals(typeof(Video.prototype), 'object');
check_equals(typeof(Video.prototype.__proto__), 'object');
check_equals(Video.prototype.__proto__, Object.prototype);
check_equals(typeof(Video.prototype.attachVideo), 'function');
check_equals(typeof(Video.prototype.clear), 'function');

// test Video instance
var videoObj = new Video;
check_equals (typeof(videoObj), 'object');
check_equals (typeof(videoObj.attachVideo), 'function');
check_equals (typeof(videoObj.clear), 'function');

// TODO: test other properties !

#endif
totals();
