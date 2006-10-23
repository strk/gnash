// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

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

// Test case for 'with' call
// See http://sswf.sourceforge.net/SWFalexref.html#action_with
//
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: with.as,v 1.4 2006/10/23 16:09:15 strk Exp $";

#include "check.as"

var l0 = 5;
var obj = { a:1, b:2 };

check_equals(a, undefined);
check_equals(b, undefined);

with(obj)
{
	check_equals(a, 1);
	check_equals(b, 2);
	check_equals(l0, 5);
	c = 3; // see below
}
// make sure that the assignment above didn't affect the object
check_equals(obj.c, undefined); 

var obj2 = { o:obj };
with(obj2)
{
	with(o)
	{
		check_equals(l0, 5); // scan back to the root
		check_equals(obj.a, 1); // scan back to the root
		check_equals(a, 1);
		check_equals(b, 2);
	}
	check_equals(obj.a, 1); // scan back to the root
	with(obj) // scans back to the root...
	{
		check_equals(a, 1);
		check_equals(b, 2);
	}
}
with(obj2.o) // this seems more a Ming test...
{
	check_equals(a, 1);
	check_equals(b, 2);
}

// Now test the limits of the 'with' stack
// use 20 item, to make sure both 7/8 and 15/16 limit is reached

var o3 = { o:obj2 }; var o4 = { o:o3 }; var o5 = { o:o4 }; var o6 = { o:o5 };
var o7 = { o:o6 }; var o8 = { o:o7 }; var o9 = { o:o8 }; var o10 = { o:o9 };
var o11 = { o:o10 }; var o12 = { o:o11 }; var o13 = { o:o12 };
var o14 = { o:o13 }; var o15 = { o:o14 }; var o16 = { o:o15 };
var o17 = { o:o16 }; var o18 = { o:o17 };

// Try with a depth of 7, should be supported by SWF5 and up
with(o7) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { 
	found7 = a;
}}}}}}} // depth 7 (should be supported by SWF5)
check_equals(found7, 1); // this works if the above worked

// Try with a depth of 8, should be unsupported by SWF5
// but supported by later target (alexis sais)
with(o8) {
with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) {
	check_equals(obj.a, 1); // scan back to the root
	found8 = a;
}}}}}}}} // depth 8 (should be unsupported by SWF5)
#if OUTPUT_VERSION > 5
check_equals(found8, 1); 
#else
check_equals(found8, undefined); 
#endif

// Try with a depth of 17, should be unsupported with all targets
// target
with(o17) {
with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(0) {
with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(0) {
	found17 = a; // this should never execute !
}}}}}}}}}}}}}}}}}
check_equals(found17, undefined); 
