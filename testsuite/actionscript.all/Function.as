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

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Function.as,v 1.24 2007/02/28 23:58:26 strk Exp $";

#include "check.as"

// Define a function returning 'this'.name and the given args
function getThisName(a,b,c) { return this.name+a+b+c; }

#if OUTPUT_VERSION >=6 
 check (getThisName != undefined);
#else
 // this might be due to forced numerical comparison
 xcheck_equals (getThisName, undefined);
#endif
check_equals ( typeof(getThisName), "function" );

//----------------------------------------------------------
//
// Test Function.apply
//
//----------------------------------------------------------

#if OUTPUT_VERSION >= 6

check_equals(typeof(getThisName.apply), 'function');

var this_ref = {name:"extname"};

// Test Function.apply(this_ref, args_array)
#if OUTPUT_VERSION >= 7
  check_equals ( getThisName.apply(this_ref), "extnameundefinedundefinedundefined" );
#else
  check_equals ( getThisName.apply(this_ref), "extname" );
#endif

// Test Function.apply(this_ref, args_array)
var ret=getThisName.apply(this_ref, [1,2,3]);
check ( ret == "extname123" );

// Test invalid Function.apply calls

var ret=getThisName.apply();
#if OUTPUT_VERSION > 6
  xcheck ( isNaN(ret) ); // result of the *numerical* sum of all undefined
#else
  check_equals ( ret , 0 ); // result of the *numerical* sum of all undefined
#endif

var ret=getThisName.apply(this_ref, [4,5,6], 4);
check_equals ( ret , "extname456" );
var ret=getThisName.apply(this_ref, "8");
#if OUTPUT_VERSION >= 7
  check_equals ( ret , "extnameundefinedundefinedundefined" );
#else
  check_equals ( ret , "extname" );
#endif

var ret=getThisName.apply(this_ref, 9);
#if OUTPUT_VERSION >= 7
  check_equals ( ret , "extnameundefinedundefinedundefined" );
#else
  check_equals ( ret , "extname" );
#endif

var ret=getThisName.apply(undefined, [4,5,6], 4);
#if OUTPUT_VERSION >= 7
  xcheck ( isNaN(ret) ); // the sum will be considered numerical
#else
  check_equals ( ret , 15 ); // the sum will be considered numerical
#endif

var ret=getThisName.apply(undefined, 7);
#if OUTPUT_VERSION >= 7
  xcheck ( isNaN(ret) ); 
#else
  check_equals ( ret , 0 );
#endif

var ret=getThisName.apply(undefined, "7");
#if OUTPUT_VERSION >= 7
  xcheck ( isNaN(ret) ); 
#else
  check_equals ( ret , 0 );
#endif

#else // OUTPUT_VERSION < 6

// No Function.apply... for SWF up to 5
check_equals(typeOf(getThisName.apply), 'undefined');

#endif

//----------------------------------------------------------
//
// Test Function.call
//
//----------------------------------------------------------

#if OUTPUT_VERSION >= 6

// Test Function.call(arg1, arg2, arg3)
check_equals ( getThisName.call(this_ref, 1, 2, 3), "extname123" );

#else // OUTPUT_VERSION < 6

check_equals ( typeOf(getThisName.call), 'undefined' );

#endif

//----------------------------------------------------------
//
// Test Function definition
//
//----------------------------------------------------------

// Define a class with its constructor
var TestClass = function() {
	this.name = "NONE";
};

// Test the Function constuctor
check_equals ( typeOf(TestClass), 'function' );

#if OUTPUT_VERSION >= 6

  // Test existance of the Function::apply method
  check_equals ( typeOf(TestClass.apply), 'function' );

  // Test existance of the Function::call method
  check_equals ( typeOf(TestClass.call), 'function' );

#endif

// test existance of the Function::prototype member
check_equals ( typeOf(TestClass.prototype), 'object' );

// Define methods 
TestClass.prototype.setname = function(name) {
	this.name = name;
};

// Test instanciation of Function
var testInstance = new TestClass;
check (testInstance != undefined);
check (testInstance.name != undefined);
check (testInstance.name == "NONE");
check (typeof(testInstance.setname) == "function");

// Test methods call
testInstance.setname("Test");
check (testInstance.name == "Test");

// Test inheritance
check (testInstance.__proto__ != undefined);
check (testInstance.__proto__ == TestClass.prototype);
check (testInstance instanceOf TestClass);
check (testInstance instanceOf Object);
#if OUTPUT_VERSION > 5
// Function was added in version 5
check_equals (typeOf(Function.prototype.addProperty), 'function');
check_equals (testInstance.__proto__, TestClass.prototype)
check_equals (TestClass.__proto__, Function.prototype)
check_equals (testInstance.addProperty, Object.prototype.addProperty)
Object.prototype.addProperty = function() { return 7; };
var t = testInstance.addProperty();
check_equals(t, 7);
check (Function instanceOf Object);
#endif

check_equals (typeOf(TestClass.prototype.constructor), 'function');

check (TestClass.prototype.constructor == TestClass);
check (testInstance.__proto__.constructor == TestClass);

// An instance .prototype is NOT a reference to 
// the superclass's prototype. That would be __proto__.
testInstance.prototype.additional = "not-in-super";
check (TestClass.prototype.additional == undefined);

// Test inheritance with built-in functions
var stringInstance = new String();
check (stringInstance.__proto__ != undefined);
check (stringInstance.__proto__ == String.prototype);
check_equals ( typeof(String.prototype.constructor), 'function' );
check (String.prototype.constructor == String);
check (stringInstance.__proto__.constructor == String);

// Test the instanceof operator
check ( testInstance instanceof TestClass );
check ( stringInstance instanceof String );

//----------------------------------------------------------
//
// Test access of a timeline locals from within a function
//
//----------------------------------------------------------

// These are timeline "locals" (note the 'var' prefix).
// They should produce DEFINELOCAL tags
var tl_local = "tl_local";
var todelete = "deleteme";
var tooverride = "tooverride";
a_func = function() {

	// get a "local" var of this function
	var localvar = "lv";
	check_equals(localvar, "lv");

	// get a "local" var of the timeline
	check_equals(tl_local, "tl_local");

	// A "local" var of this function with
	// hides a "local" var of the timeline
	// with the same name, but just within
	// this context (there's another check
	// outside the function to verify the
	// original value is preserved there)
	var tooverride = "overridden";
	check_equals(tooverride, "overridden");

	// set a "local" var of the timeline
	tl_local = "tl_local2";

	// delete a "local" var of the timeline
	delete todelete;

	// create a new variable of the timeline
	tl_new = "tl_new";

	// create a new function "local" for this function.
	check(! delete fl_func); // make sure there's no other
	var fl_func = function() { };

	// create a new function on the timeline
	// (this only works with SWF6 or up)
	check(! delete f_func); // make sure there's no other
	f_func = function() { };

};
check_equals(f_func, undefined); // will be created by a_func() call
check_equals(tl_new, undefined); // will be created by a_func() call
a_func(); // create tl_new and f_func
check_equals(tl_local, "tl_local2");
check_equals(todelete, undefined);
check_equals(tooverride, "tooverride");
check_equals(tl_new, "tl_new");
check_equals(fl_func, undefined);
check_equals(typeof(f_func), 'function'); // created by a_func() call

//----------------------------------------------------------
//
// Test nested functions
//
//----------------------------------------------------------

var result1 = "initial_result1_value";
var result2 = "initial_result2_value";
// just to make sure that eval works (more of a Ming test this one..)
check_equals(eval("result1"), "initial_result1_value");

outer_func = function() 
{
	var a = "hello";

	// inner_func should be created on the timeline,
	// see previous tests block
	inner_func = function(var_ref) 
	{
		return(eval(var_ref));
	};
  
	result1 = inner_func("a");  // should return "hello"
};

//call outer_func to set result1
check_equals(typeof(outer_func), 'function');
outer_func(); 

// call inner_func to set result2
check_equals(typeof(inner_func), 'function');
result2 = inner_func("a");  // should return "hello"

#if OUTPUT_VERSION >= 6

  check_equals ( result1, "hello" );

  // Gnash fails here, we want this fixed!
  xcheck_equals ( result2, "hello" );

#else // SWF5 or lower seems unable to work with nested functions

  xcheck_equals ( result1, undefined );

  // Gnash succeeds here, but that's for the same reason why it
  // fails in the SWF6+ section above...
  check_equals ( result2, undefined );

#endif

//----------------------------------------------------------
//
//  Test case for "this"  in Object's context
//  by Zou Lunkai, zoulunkai@gmail.com
//
//----------------------------------------------------------

var obj = new Object();
obj.a = "a_in_obj";

var a = "a_in_root";

obj.func = function ()
{
        check_equals(this.a, "a_in_obj");
};
obj.func();

func = function ()
{
   check_equals(this.a, "a_in_root");
};
func();

check_equals(this.a, "a_in_root");

//----------------------------------------------------------
//  Test the 'arguments' object
//----------------------------------------------------------

// this is to be called argsChecker(1,2,3)
function argsChecker()
{
	check_equals(typeof(arguments), 'object');
	check(arguments instanceOf Array);
	check(arguments instanceOf Object);
	check_equals(typeof(arguments.callee), 'function');
	check_equals(arguments.callee, argsChecker); 
	check_equals(arguments.length, 3);
	check_equals(arguments[0], 1);
	check_equals(arguments[1], 2);
	check_equals(arguments[2], 3);
	check_equals(arguments[3], undefined);
	arguments[3] = 3;
	check_equals(arguments[3], 3);
	arguments[0] = 'zero';
	check_equals(arguments[0], 'zero');
	arguments.length = 10;
	check_equals(arguments.length, 10);
	arguments.pop();
	check_equals(arguments.length, 9);
	arguments.somethingelse = "can extend";
	check_equals(arguments.somethingelse, "can extend");
}
argsChecker(1, 2, 3);

function argsCounter() {
	return arguments.length;
}
check_equals(argsCounter(1,2,3), 3);
check_equals(argsCounter(a,b,c,d), 4);
check_equals(argsCounter([a,b]), 1);

function factorial(n) {
	return n <= 1 ? n : n*factorial(n-1);
}
check_equals(factorial(3), 6);
check_equals(factorial(4), 24);
