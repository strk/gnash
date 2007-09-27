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

// Test case for 'with' call
// See http://sswf.sourceforge.net/SWFalexref.html#action_with
//
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: with.as,v 1.14 2007/09/27 09:09:43 zoulunkai Exp $";

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

#if OUTPUT_VERSION > 5
// Try with a depth of 8, should be supported by Players from version 6 up
// but supported by later target (alexis sais)
with(o8) {
with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) {
	check_equals(obj.a, 1); // scan back to the root
	found8 = a;
}}}}}}}} // depth 8 (should be unsupported by SWF5)
check_equals(found8, 1); 
#else
// Don't assume that SWF version drives this: it only depends on the player.
//check_equals(found8, undefined); 
#endif

#if 0 // Don't assume that SWF version drives this: it only depends on the player!
// Try with a depth of 17, should be unsupported with all targets
// target
with(o17) {
with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(0) {
with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(o) { with(0) {
	found17 = a; // this should never execute !
}}}}}}}}}}}}}}}}}
check_equals(found17, undefined); 
#endif

//---------------------------------------------------------
// Test function calls in 'with' context
//---------------------------------------------------------

var name = "timeline";
get_name = function() { return this.name; }; 
var obj1 = { name: "obj1", get_name: get_name }; 
check_equals(get_name(), "timeline");
with(obj1)
{
	check_equals(get_name(), "obj1");
}

//---------------------------------------------------------
// Test empty 'with' block
//---------------------------------------------------------

with(_root) { ; }
// not aborting is enough to pass this test

//---------------------------------------------------------
// Test with(undefined) with(null) with(3) with("string")
//---------------------------------------------------------

checkpoint = 1;

with(null) { _root.checkpoint = 2; }
check_equals(checkpoint, 1);

with(undefined) { _root.checkpoint = 3; }
check_equals(checkpoint, 1);

with(4) { _root.checkpoint = 3; __proto__.checkpoint = 'three'; }
check_equals(checkpoint, 3);
check_equals(Number.prototype.checkpoint, 'three');

with('string') { _root.checkpoint = 4; __proto__.checkpoint = 'four'; }
check_equals(checkpoint, 4);
check_equals(String.prototype.checkpoint, 'four');


//---------------------------------------------------------
// tests how "with" affect the serch path(ScopeChain).
//---------------------------------------------------------

a = 100;
b = 100;
_root.createEmptyMovieClip("mc1", 3);
mc1.a = 1;
mc1.b = 1;
mc1.x = 3;
_root.createEmptyMovieClip("mc2", 4);
mc2.a = 2;
mc2.b = 2;
mc2.y = 3;

with(mc1)
{
	check_equals(a, 1);
}
with(mc2)
{
	check_equals(a, 2);
}

with(mc1)
{
	with(mc2)
	{
		check_equals(a, 2);
		check_equals(b, 2);
		check_equals(x, 3);
		check_equals(y, 3);
	}
}

with(mc2)
{
	check_equals(this, _root);
	with(mc1)
	{
	  check_equals(this, _level0);
		check_equals(a, 1);
		check_equals(b, 1);
		check_equals(x, 3);
		check_equals(y, 3);
	}
}

function f_a()
{
	return a;
}

function f_b()
{
	return b;
}

function f_x()
{
	return x;
}

function f_y()
{
	return y;
}

with(mc1)
{
	with(mc2)
	{
		check_equals(f_a(), 100); 
		check_equals(f_b(), 100); 
		check_equals(typeof(f_x()), 'undefined'); 
		check_equals(typeof(f_y()), 'undefined'); 
	}
}


//
// tests of setTarget
//
tellTarget("mc1");
// setTarget and With won't change this context.
// but the deduction might be incorrect!!!
check_equals(this, _root);
tellTarget("");
// Ming thinks tellTarget is a Function, but it is not.
// So all related tests are probably bogus.
check_equals(typeof(tellTarget), 'undefined');
check_equals(this, _root);
