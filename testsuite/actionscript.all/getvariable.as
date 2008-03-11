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

// Test case for getvariable Action tag class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: getvariable.as,v 1.25 2008/03/11 19:31:48 strk Exp $";
#include "check.as"

// see check.as
#ifdef MING_SUPPORTS_ASM

_global.globalvar = "gv1";
_global.globalobj = { m1: 1 };

mc1 = createEmptyMovieClip('mc', 1);
mc1.mem = 'mc1';
mc2 = mc1.createEmptyMovieClip('mc', 1);
mc2.mem = 'mc2';
mc2.o = new Object();

//---------------------------------------------------------------------
// Check access to root variable (simplest case)
//---------------------------------------------------------------------

var variable_in_root = 5;
asm {
        push 'checkpoint'
	push 'variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5);

//---------------------------------------------------------------------
// Check access to root variable trough '_root.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.1;
asm {
        push 'checkpoint'
	push '_root.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.1);

//---------------------------------------------------------------------
// Check access to root variable trough '_level0.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.2;
asm {
        push 'checkpoint'
	push '_level0.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.2);

//---------------------------------------------------------------------
// Check access to root variable trough 'this.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.3;
asm {
        push 'checkpoint'
	push 'this.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.3);

//---------------------------------------------------------------------
// Check access to root variable trough 'this._root._level0.varname'
// (insane)
//---------------------------------------------------------------------

var variable_in_root = 5.4;
asm {
        push 'checkpoint'
	push 'this._root._level0.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.4);

//---------------------------------------------------------------------
// Check access to root variable trough 'THIS.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.5;
asm {
        push 'checkpoint'
	push 'THIS.variable_in_root'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 6
check_equals(checkpoint, undefined);
#else
check_equals(checkpoint, 5.5);
#endif

//---------------------------------------------------------------------
// Check '/mc/mc' access 
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 5
 check_equals(typeof(checkpoint), 'movieclip');
 check_equals(checkpoint, _level0.mc.mc);
#else
 check_equals(typeof(checkpoint), 'undefined');
 check_equals(checkpoint, _level0.mc.mc);
#endif

//---------------------------------------------------------------------
// Check '/mc/mc/' access 
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc/'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 5
 check_equals(typeof(checkpoint), 'movieclip');
 check_equals(checkpoint, _level0.mc.mc);
#else
 check_equals(typeof(checkpoint), 'undefined');
 check_equals(checkpoint, _level0.mc.mc);
#endif

//---------------------------------------------------------------------
// Check '/mc/mc/:' access 
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc/:'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check '/mc/mc:' access 
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc:'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check '/mc/mc/mem' access 
// (expected to fail, but I'm not sure why)
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc/mem'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check '/mc/mc/o' access 
// (expected to fail, but I'm not sure why)
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc/o'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check '/mc/mc:mem' access 
//---------------------------------------------------------------------

asm {
        push 'checkpoint'
	push '/mc/mc:mem'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 5
 check_equals(checkpoint, 'mc2');
#else
 check_equals(typeof(checkpoint), 'undefined');
#endif


//---------------------------------------------------------------------
// Check '../:variable_in_root' access 
// (expected to fail, but I'm not sure why)
//---------------------------------------------------------------------

var variable_in_root = 6;
asm {
        push 'checkpoint'
	push '../:variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check '...:variable_in_root' access 
//---------------------------------------------------------------------

checkpoint = 1;
var variable_in_root = 6;
asm {
        push 'checkpoint'
	push '...:variable_in_root'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check '...:mc1' access 
//---------------------------------------------------------------------

checkpoint = 1;
asm {
        push 'checkpoint'
	push '...:mc1'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check 'obj:variable_in_object' access 
//---------------------------------------------------------------------

var obj = { variable_in_object: 'yes' };
check_equals(obj.variable_in_object, 'yes');
asm {
        push 'checkpoint'
	push 'obj:variable_in_object'
	getvariable
        setvariable
};
check_equals(checkpoint, 'yes');
asm {
	push 'obj:variable_in_object'
        push 'yes2'
        setvariable
};
check_equals(obj.variable_in_object, 'yes2');

//---------------------------------------------------------------------
// Check 'obj::::variable_in_object' access 
//---------------------------------------------------------------------

var obj = { variable_in_object: 'yes' };
check_equals(obj.variable_in_object, 'yes');
asm {
        push 'checkpoint'
	push 'obj::::variable_in_object'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);
asm {
	push 'obj::::variable_in_object'
        push 'yes2'
        setvariable
};
check_equals(obj.variable_in_object, 'yes');
check_equals(typeof(_root['obj::::variable_in_object']), 'undefined');

asm {
	push 'obj:variable_in_object'
        push 'yes3'
        setvariable
};
check_equals(obj.variable_in_object, 'yes3');

asm {
	push 'obj::variable_in_object'
        push 'yes4'
        setvariable
};
check_equals(obj.variable_in_object, 'yes4');

// NOTE: 'obj:::variable_in_object' is invalid, while
//       'obj::variable_in_object' is valid - AARagagag
asm {
	push 'obj:::variable_in_object'
        push 'yes5'
        setvariable
};
check_equals(obj.variable_in_object, 'yes4');

//---------------------------------------------------------------------
// Check 'mc1:variable_in_object' access 
//---------------------------------------------------------------------

#if OUTPUT_VERSION > 5

mc1.variable_in_object = 'yes';
asm {
        push 'checkpoint'
	push 'mc1:variable_in_object'
	getvariable
        setvariable
};
check_equals(checkpoint, 'yes');
asm {
	push 'mc1:variable_in_object'
        push 'yes2'
        setvariable
};
check_equals(mc1.variable_in_object, 'yes2');

#endif // OUTPUT_VERSION > 5

//---------------------------------------------------------------------
// Check 'mc1::::variable_in_object' access 
//---------------------------------------------------------------------

#if OUTPUT_VERSION > 5

mc1.variable_in_object = 'yes';
asm {
        push 'checkpoint'
	push 'mc1::::variable_in_object'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);
asm {
	push 'mc1::::variable_in_object'
        push 'yes2'
        setvariable
};
check_equals(mc1.variable_in_object, 'yes');
check_equals(typeof(_root['mc1::::variable_in_object']), 'undefined');

#endif // OUTPUT_VERSION > 5

//---------------------------------------------------------------------
// Check '../invalidname' access 
// (expected to fail)
//---------------------------------------------------------------------

asm {
	push '../invalidname'
	push '8'
	setvariable
        push 'checkpoint'
	push '../invalidname'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check '/invalidname2' access 
// (expected to fail)
//---------------------------------------------------------------------

asm {
	push '/invalidname2'
	push '18'
	setvariable
        push 'checkpoint'
	push '/invalidname'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj.member' access 
//---------------------------------------------------------------------

var obj = { memb:3 };
asm {
        push 'objmemb'
	push 'obj.memb'
	getvariable
        setvariable
};
check_equals(objmemb, 3);

//---------------------------------------------------------------------
// Check 'obj._root.variable_in_root' access 
// (expect to fail)
//---------------------------------------------------------------------

var variable_in_root = 10.0;
var o = { memb:4 };
asm {
        push 'checkpoint'
	push 'o._root.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj.this.memb' access 
// (expect to fail)
//---------------------------------------------------------------------

var o = { memb:10.1 };
asm {
        push 'checkpoint'
	push 'o.this.memb'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj1.obj2.member' access 
//---------------------------------------------------------------------

var o2 = { memb:4 };
var o = { obj2:o2 };
asm {
        push 'checkpoint'
	push 'o.obj2.memb'
	getvariable
        setvariable
};
check_equals(checkpoint, 4);

//---------------------------------------------------------------------
// Check 'obj1..obj2.member' access 
// (expect to fail)
//---------------------------------------------------------------------

var o2 = { memb:4 };
var o = { obj2:o2 };
asm {
        push 'checkpoint'
	push 'o..obj2.memb'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');

//---------------------------------------------------------------------
// Check 'obj1.globalvar'
// (expect to fail)
//---------------------------------------------------------------------

var o3 = { memb:4 };
asm {
        push 'checkpoint'
	push 'o3.globalvar'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'globalobj.m1'
//---------------------------------------------------------------------

var o3 = { memb:4 };
asm {
        push 'checkpoint'
	push 'globalobj.m1'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 5
check_equals(checkpoint, 1);
#else // OUTPUT_VERSION < 6
// _global was added in SWF6 !
check_equals(checkpoint, undefined);
#endif

//-----------------------------------------------------------------------
// Check 'obj/:member' access 
//-----------------------------------------------------------------------

var obj = { memb:3 };
asm {
        push 'objmemb'
	push 'obj/:memb'
	getvariable
        setvariable
};
check_equals(objmemb, 3);

//-----------------------------------------------------------------------
// Check '::obj:member' access 
//-----------------------------------------------------------------------

var obj = { memb:4 };
asm {
        push 'objmemb'
	push '::obj:memb'
	getvariable
        setvariable
};
check_equals(objmemb, 4);

//-----------------------------------------------------------------------
// Check '::obj::member' access 
//-----------------------------------------------------------------------

var obj = { memb:4.4 };
asm {
        push 'objmemb'
	push '::obj::memb'
	getvariable
        setvariable
};
check_equals(objmemb, 4.4);

//-----------------------------------------------------------------------
// Check '::obj.member' access 
//-----------------------------------------------------------------------

var obj = { memb:4 };
asm {
        push 'objmemb'
	push '::obj.memb'
	getvariable
        setvariable
};
check_equals(objmemb, 4);

//-----------------------------------------------------------------------
// Check 'this/:member' access  (and deletion)
//-----------------------------------------------------------------------

var memb = 6;
asm {
        push 'thismemb'
	push 'this/:memb'
	getvariable
        setvariable
};
check_equals(memb, 6);
check_equals(thismemb, 6);

//-----------------------------------------------------------------------
// Check 'invalid/name' access
// ('invalid/name' used as a variable name)
//-----------------------------------------------------------------------

asm {
	push 'invalid/name'
	push '7'
	setvariable
        push 'checkpoint'
	push 'invalid/name'
	getvariable
        setvariable
};
check_equals(checkpoint, 7);

//-----------------------------------------------------------------------
// Check 'invalid:Number'
//-----------------------------------------------------------------------

asm {
	push 'invalid:Number'
	push '8'
	setvariable
        push 'checkpoint'
	push 'invalid:Number'
	getvariable
        setvariable
};
check_equals(typeof(checkpoint), 'undefined');
check_equals(typeof(invalid), 'undefined');


//-----------------------------------------------------------------------
// Check 'path and var'
//-----------------------------------------------------------------------
function func_obj() 
{ 
	var xx = 1;
	asm {	
		push '/:xx'
		push '2'
		setvariable
	};
	check_equals(xx, 1);
	check_equals(_root.xx, 2);
};

func = new func_obj();
func();

//---------------------------------------------------------------------
// Check scope of function called trough a path
// nothing to do with GetVariable, but I didnt' feel like adding another
// file for testing this.. it's still related to tags and variable names
//---------------------------------------------------------------------

num = 7;
func = function() { return this.num; };
o = new Object;
o.func = func;
o.num = 5;
asm {
        push 'checkpoint'
	push 0
	push '_root.o.func'
	callfunction
        setvariable
};
check_equals(checkpoint, 5);

o2 = new Object;
o2.m = new Object;
o2.m.func = func;
o2.num = 3;
o2.m.num = 4;
with (o2.m) {
asm {
        push 'checkpoint'
	push 0
	push 'func'
	callfunction
        setvariable
};
};
check_equals(checkpoint, 4);

_global.func = func;
_global.num = 9;
asm {
        push 'checkpoint'
	push 0
	push 'func'
	callfunction
        setvariable
};
check_equals(checkpoint, 7);

this["o2.m.func"] = function() { return 19; };
asm {
        push 'checkpoint'
	push 0
	push 'o2.m.func'
	callfunction
        setvariable
};
check_equals(checkpoint, 4);

//-----------------------------------------------------------------------
// TODO: try use of 'with' stack
//-----------------------------------------------------------------------

#if OUTPUT_VERSION < 6
 xcheck_totals(52); // gnash runs +2 tests ?!
#else
 xcheck_totals(57); // gnash runs +2 tests ?!
#endif

#else // ndef MING_SUPPORT_ASM

check_totals(0);

#endif // MING_SUPPORT_ASM

