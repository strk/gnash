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


rcsid="$Id: with.as,v 1.41 2008/03/11 19:31:49 strk Exp $";
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
//  test 'this' in 'with' context
//
objA = new Object();
objB = new Object();
objA.func = function () {
   check_equals(this, objA);
   with(_root.objB){
    // 'with' won't change this context
   	check_equals(this, objA);
   }
};
objA.func();


// 
// test ActionSetTarget and ActionSetTarget2 
//
#if OUTPUT_VERSION > 5

// create _root.clip1.clip2 
_root.createEmptyMovieClip("clip1", 101);
clip1.createEmptyMovieClip("clip2", 102);  
_root.createEmptyMovieClip("clip3", 103);
check_equals(typeof(clip1.clip2),'movieclip');

clip1.testvar = 'clip1_var'; 
clip1.clip2.testvar = 'clip2_var';  
clip3.testvar = 'clip3_var';  
testvar = '_root_timeline_var'; 
_global.testvar = 'global_var';

testvar2 = '_root_timeline_var'; 
_global.testvar2  = 'global_var';

testvar3 = '_root_timeline_var';


// Quick Dejagnu hack for setTarget
// After setTarget, those function are hidden.
// Calling _root.check is also not appropriate at the moment.
// TODO: find a better way to fix Dejagnu
_global.pass_check = pass_check;
_global.fail_check = fail_check;
_global.xpass_check = xpass_check;
_global.xfail_check = xfail_check;

setTarget('/clip1'); //tag 0x8B 
    check_equals(this, _level0);
    check_equals(testvar, 'clip1_var');     
    // won't search timeline properties, different with ActionWith     
    check_equals(testvar2, 'global_var');
    check_equals(testvar3, undefined);  
setTarget("");

setTarget('/clip1/clip2'); //tag 0x8B 
    check_equals(this, _level0);
    check_equals(testvar, 'clip2_var');     
    check_equals(testvar2, 'global_var');  
    check_equals(testvar3, undefined);  
setTarget("");

setTarget('/clip1/../clip3'); //tag 0x8B 
    check_equals(this, _level0);
    check_equals(testvar, 'clip3_var');     
    check_equals(testvar2, 'global_var');  
    check_equals(testvar3, undefined);  
setTarget("");

// first understand getproperty 
asm{   
    push  'checkpoint'   
    push  ''   
    push  11 //_target   
    getproperty  
    setvariable        
};  
check_equals(checkpoint, '/');

// then see how _target is affected. 
setTarget("/clip1"); 
  asm{
    push 'checkpoint'         
    push ''         
    push 11 //_target        
    getproperty  
    setvariable             
  };      
  check_equals(checkpoint, '/clip1'); 
setTarget("");

setTarget("/clip1/clip2");  
  asm{
    push 'checkpoint'         
    push ''
    push 11         
    getproperty  //_target         
    setvariable             
  };
  check_equals(checkpoint, '/clip1/clip2'); 
setTarget("");

setTarget("/clip1"); 
  setTarget("/clip1/clip2");  
    asm{
      push 'checkpoint'         
      push ''
      push 11         
      getproperty  //_target         
      setvariable             
    };
    check_equals(checkpoint, '/clip1/clip2'); 
  setTarget("");
  asm{
    push 'checkpoint'         
    push ''
    push 11         
    getproperty  //_target         
    setvariable             
  };
  // _target don't need to be stacked.
  check_equals(checkpoint, '/'); 
setTarget("");

var o = {}; o.t = _root.clip1;
setTarget('o.t');
check_equals(_target, "/clip1");
setTarget("");

setTarget('o:t');
check_equals(_target, "/clip1");
setTarget("");

setTarget('../o:t'); // invalid ?
check_equals(_target, "/");
setTarget("");

var o2 = {};
o2.o = o;
setTarget('o2.o'); // not a movie
check_equals(_target, "/");
setTarget("");

setTarget('o2.o.t'); 
check_equals(_target, "/clip1");
setTarget("");

o2['o.p'] = o;
setTarget('o2.o.p.t');  // member p of object o2.o doesn't exist
check_equals(_target, "/");
setTarget("");

o2.__proto__.inh = o;
setTarget('o2.inh.t');  // inherited members are scanned 
check_equals(_target, "/clip1");
setTarget("");

with (o2)
{
	// NOTE: AS setTarget() used SETTARGET opcode while
	// ASM settargetexpr uses the setTArgetExpression one.
	// Problem is that Ming 0.4.0.beta6 correctly compiles
	// a setTarget() call inside a 'with' block as a function
	// call rather then as the SETTARGET opcode, so we
	// *need* to use ASM here, but 'settarget' is not available yet.
	asm { push 'o:t' settargetexpr }; //setTarget("o:t");
	check_equals(_target, "/clip1");
	asm { push '' settargetexpr }; //setTarget("");
}

// 
// TODO: add tests for setTarget
//

#endif  //OUTPUT_VERSION > 5

//---------------------------------------------------------
// Test with() inside user-defined function context
//---------------------------------------------------------

function testWith()
{
	var a = 1;
	var b = 6;
	var c = "empty";
	var f = "empty";

	with (o)
	{
		//// GETTING VARIABLES

		// with stack takes precedence over locals
		// (this is o.a)
		check_equals(a, 4);

		// locals take precedence over scope stack
		// (o.b does not exist)
		check_equals(b, 6);

		// Members named 'x.y' won't be seeked for in with stack elements 
		asm {
			push  'checkpoint'   
			push  'g.h'   
			getvariable  
			setvariable
		};
		check_equals(typeof(checkpoint), "undefined");

		// Path like 'x.y' will be looked for in with stack elements 
		asm {
			push  'checkpoint'   
			push  'i.j'   
			getvariable  
			setvariable
		};
		check_equals(checkpoint, "o.i.j");

		//// SETTING VARIABLES

		// with stack takes precedence over locals:
		// locals are only set if with stack don't contain the variable being set
		c = "with o"; // c exists in  locals:YES  with:NO 
		d = "with o"; // d exists in  locals:NO   with:NO 
		e = "with o"; // e exists in  locals:NO   with:YES
		f = "with o"; // f exists in  locals:YES  with:YES
	}

	check_equals(c, "with o");
	check_equals(d, "with o");
	check_equals(typeof(e), "undefined");
	check_equals(f, "empty"); 

	_root.newFunc = function()
	{
		var b = 7;

#if OUTPUT_VERSION >= 6
		// scope stack includes activation object
		// which is locals of testWith
		check_equals(a, 1); 
#else
		// scope stack doesn't include activation object
		// of testWith
		check_equals(a, 120); 
#endif

		// locals take precedence over scope stack
		check_equals(b, 7);

	};
}

o = new Object();
o.a = 4;
o.e = "o.e";
o.f = "o.f";
o['g.h'] = "o.g.h";
o.i = new Object;
o.i.j = "o.i.j";
a = 120;
b = 5;
testWith();
check_equals(typeof(o.c), 'undefined');
check_equals(typeof(o.d), 'undefined');
check_equals(o.e, 'with o');
check_equals(o.f, 'with o'); 

asm {
	push  'checkpoint'   
	push  'o.g.h'   
	getvariable  
	setvariable
};
check_equals(typeof(checkpoint), "undefined");

newFunc();

//---------------------------------------------------------
// Test with(movieclip) 
//---------------------------------------------------------

#if OUTPUT_VERSION > 5
createEmptyMovieClip("mc", 1);
mc.createEmptyMovieClip("child", 1);
mc.mem = "mcMember";
mc.nooverride = "nooverride";
ASSetPropFlags(mc, "nooverride", 4, 1); 
MovieClip.prototype.inheritedMem = "McProto";
with (mc)
{
	child = "rootChild"; // non-proper in mc, will be set in root
	mem = "mcMemberUpdated"; // overridable in mc, will be updated
	nooverride = "nooverrideUpdated"; // protected from override in mc, but existing. Will NOT be set in _root
	nonexistent = "nonExistent"; // non-existing in mc, will be set in root
	inheritedMem = "McProtoOverridden"; // non own-property of mc, will be set in root

	check_equals(inheritedMem, "McProto");
	check_equals(nonexistent, "nonExistent");
	check_equals(nooverride, "nooverride");
	check_equals(mem, "mcMemberUpdated");
	check_equals(child._target, "/mc/child");
}
check_equals(inheritedMem, "McProtoOverridden");
check_equals(mc.inheritedMem, "McProto");
check_equals(nonexistent, 'nonExistent');
check_equals(typeof(nooverride), 'undefined');
check_equals(mc.nooverride, 'nooverride');
check_equals(typeof(mem), 'undefined');
check_equals(mc.mem, "mcMemberUpdated");
check_equals(typeof(mc.child), 'movieclip');
check_equals(child, "rootChild");
#endif // OUTPUT_VERSION > 5

//---------------------------------------------------------
// END OF TESTS
//---------------------------------------------------------

#if OUTPUT_VERSION < 6
 check_totals(41);
#else
 check_totals(99);
#endif
