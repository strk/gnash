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


// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Function.as,v 1.73 2008/04/28 16:10:00 bwy Exp $";
#include "check.as"

#if OUTPUT_VERSION >= 6
check_equals(Function.prototype.__proto__, Object.prototype);
check_equals(Function.constructor, Function);
check( ! Function.hasOwnProperty('__constructor__') );
#endif

// Define a function returning 'this'.name and the given args
function getThisName(a,b,c) { return this.name+a+b+c; }

// Every newly created function's super class is Object
check_equals(getThisName.prototype.__proto__, Object.prototype);

#if OUTPUT_VERSION >=6 
 check (getThisName != undefined);
#else
 // this might be due to forced string comparison ?
 check_equals (getThisName, undefined);
 check_equals (getThisName, null);
 check (getThisName != 0);
 check (getThisName != 1);
 check (! isNaN(getThisName) );
 check (getThisName != "");
 check (getThisName != "[type Function]");
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
  check ( isNaN(ret) ); // result of the *numerical* sum of all undefined
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
  check ( isNaN(ret) ); // the sum will be considered numerical
#else
  check_equals ( ret , 15 ); // the sum will be considered numerical
#endif

var ret=getThisName.apply(undefined, 7);
#if OUTPUT_VERSION >= 7
  check ( isNaN(ret) ); 
#else
  check_equals ( ret , 0 );
#endif

var ret=getThisName.apply(undefined, "7");
#if OUTPUT_VERSION >= 7
  check ( isNaN(ret) ); 
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

// Test Function.call(this, arg1, arg2, arg3)
check_equals ( getThisName.call(this_ref, 1, 2, 3), "extname123" );

// Test Function.call(null, arg1, arg2, arg3)
nullcall = getThisName.call(null, 1, 2, 3);
#if OUTPUT_VERSION > 6
 check_equals ( typeof(nullcall), 'number' );
 check ( isNaN(nullcall) );
#else
 check_equals ( nullcall, 6 );
#endif

function getThis () { ++c; return this; }
o={};
c=0;
ret = getThis.call(o);
check_equals(ret, o);
check_equals(c, 1);
ret = getThis.call(null);
check_equals(c, 2);
xcheck_equals(typeof(ret), 'object');
xcheck_equals(ret, undefined); // an object type which returns 'undefined' as primitive value ?
check( ! (ret === undefined) ); // an object type which returns 'undefined' as primitive value ?
check( ! (ret === null) ); // an object type which returns 'undefined' as primitive value ?

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
check(TestClass.hasOwnProperty('constructor'));
#endif

check_equals (TestClass.constructor, Function);
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
  check_equals ( result2, "hello" );

#else // SWF5 or lower doesn't use a scope chain

  check_equals ( result1, undefined );
  check_equals ( result2, undefined );

#endif

function bla (num)
{
	bla = function ()
	{
		return num;
	};
	return num;
}

check_equals(typeof(bla), 'function');
check_equals(bla(42), 42);
check_equals(typeof(bla), 'function');
#if OUTPUT_VERSION < 6
check_equals(typeof(bla(43)), 'undefined');
#else
check_equals(bla(43), 42);
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

function f()
{
    check_equals(typeof(arguments), 'object');
    check(arguments instanceOf Array);
    check(arguments instanceOf Object);
    check_equals(arguments.length, 0);
    
    check_equals(typeof(arguments.callee), 'function');
    // callee: the function being called
    check_equals(arguments.callee, _root.f);
    // caller: the caller function
    xcheck_equals(typeof(arguments.caller), 'null'); //? typeof return 'null', seems new!
    check_equals(arguments.caller, null);
    
    var a = arguments;
    var propRecorder = new Array();
    for(var props in a)
    {
        propRecorder.push(props.toString());
    }
    // no enumerable properties in default mode.
    check_equals(propRecorder.length, 0); 
    
    ASSetPropFlags(a, null, 6, 1 );
    for(var props in a)
    {
        propRecorder.push(props.toString());
    }
    propRecorder.sort();
    xcheck_equals(propRecorder.length, 5);
    check_equals(propRecorder[0], '__proto__');
    check_equals(propRecorder[1], 'callee');
    xcheck_equals(propRecorder[2], 'caller');
    xcheck_equals(propRecorder[3], 'constructor');
    xcheck_equals(propRecorder[4], 'length');
}
f();

// test argument.caller
function child_func()
{
#if OUTPUT_VERSION == 5
    check_equals(arguments.caller, parent_func);
#else
    //? passed on swf5, but failed on swf6,7,8
    xcheck_equals(arguments.caller, parent_func);
#endif
}
function parent_func()
{
    child_func();
}
parent_func();


// this is to be called argsChecker(1,2,3)
function argsChecker(a, b, c, d, e, f, g)
{
	check_equals(arguments.length, 3);
	check_equals(arguments[0], 1);
	check_equals(arguments[1], 2);
	check_equals(arguments[2], 3);
	check_equals(arguments[3], undefined);
	check_equals(d, undefined);
	arguments[3] = 3;
	check_equals(arguments[3], 3);
	// Changing a member of the 'arguments' object doesn't change
	// the corresponding named parameter, contrary to ECMA262 specs.
	check(d != 3);
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
	return n <= 1 ? n : n*arguments.callee(n-1);
}
check_equals(factorial(3), 6);
check_equals(factorial(4), 24);

//------------------------------------------------------
// Test using 'this' as a constructor
//------------------------------------------------------

#if OUTPUT_VERSION >= 6

Function.prototype['new'] = function()
{
	return new this;
};

function Foo() {};

var fooInstance = Foo['new']();
check_equals(typeof(fooInstance), 'object');
check(fooInstance instanceOf Foo);

#endif // OUTPUT_VERSION >= 6 

//----------------------------------------------------------
//
// Test conversion to string
//
//----------------------------------------------------------

function textOutFunc() {};
#if OUTPUT_VERSION >= 6
check_equals(typeof(textOutFunc.toString()), 'string');
check_equals(textOutFunc.toString(), '[type Function]');
// the toString method is inherited from Object class
check(!textOutFunc.hasOwnProperty('toString'));
check(!Function.prototype.hasOwnProperty('toString'));
check(Object.prototype.hasOwnProperty('toString'));
check_equals(textOutFunc.toString, Object.prototype.toString);
#else
check_equals(typeof(textOutFunc.toString), 'undefined');
#endif
textOutFunc.toString = function() { return "custom text rep"; };
#if OUTPUT_VERSION >= 6
check(textOutFunc.hasOwnProperty('toString'));
#endif
check_equals(textOutFunc.toString(), 'custom text rep');
check_equals(typeof(textOutFunc.toString()), 'string');

// expect 'custom text rep', not '[type Function]' in output.
// No way to check in this framework, but it's known to be bogus.
note(textOutFunc);

textOutFunc.toString = 4;
check_equals(typeof(textOutFunc.toString), 'number');

// expect '[type Function]', not 'custom text rep' in output (no way to check this!!)
note(textOutFunc);


//-----------------------------------------------------
// Test constructor and __constructor__ properties
//-----------------------------------------------------

a = 4; // number primitive to Number object
check_equals(typeof(a.constructor), 'function');
#if OUTPUT_VERSION > 5
check_equals(typeof(a.__constructor__), 'function');
#if OUTPUT_VERSION == 6
check(a.hasOwnProperty('constructor'));
#else // OUTPUT_VERSION > 6
check(!a.hasOwnProperty('constructor'));
#endif
check(a.hasOwnProperty('__constructor__'));
check_equals(a.constructor, Number);
check_equals(a.__constructor__, Number);
check(! a instanceof Number);
check(a.constructor != Object);
#endif

a = "string"; // string primitive to String object
check_equals(typeof(a.constructor), 'function');
#if OUTPUT_VERSION > 5
check_equals(typeof(a.__constructor__), 'function');
#if OUTPUT_VERSION == 6
check(a.hasOwnProperty('constructor'));
#else // OUTPUT_VERSION > 6
check(!a.hasOwnProperty('constructor'));
#endif
check(a.hasOwnProperty('__constructor__'));
check_equals(a.constructor, String);
check_equals(a.__constructor__, String);
check(! a instanceof String);
check(a.constructor != Object);
#endif

a = true; // boolean primitive to Boolean object
check_equals(typeof(a.constructor), 'function');
#if OUTPUT_VERSION > 5
check_equals(typeof(a.__constructor__), 'function');
#if OUTPUT_VERSION == 6
check(a.hasOwnProperty('constructor'));
#else // OUTPUT_VERSION > 6
check(!a.hasOwnProperty('constructor'));
#endif
check(a.hasOwnProperty('__constructor__'));
check_equals(a.constructor, Boolean);
check_equals(a.__constructor__, Boolean);
check(! a instanceof String);
check(a.constructor != Object);
#endif

//-----------------------------------------------------
// Test use of 'super'
//-----------------------------------------------------

function Mail(recipient, message)
{
	this.to = recipient;
	this.message = message;
}

function Email(subject, recipient, message)
{
	this.subject = subject;

#if OUTPUT_VERSION > 5
	check_equals(typeof(super), 'object');
#else // OUTPUT_VERSION <= 5
	check_equals(typeof(super), 'undefined');
#endif
	super(recipient, message);
}

check_equals(typeof(Email.prototype.__constructor__), 'undefined');

// Email is a Function instance, and it's "constructor" property
// tells us so
#if OUTPUT_VERSION == 5
// Function is supported in SWF6 and above
check_equals(Email.constructor.toString(), undefined);
check_equals(Function, undefined);
#endif
check_equals(typeof(Email.constructor), 'function');
check_equals(typeof(Email.constructor.constructor), 'function');
check_equals(typeof(Email.constructor.constructor.constructor), 'function');
#if OUTPUT_VERSION > 5
check_equals(Email.constructor, Function);
check_equals(Email.constructor.constructor, Function);
check_equals(Email.constructor.constructor.constructor, Function);

check(Email.hasOwnProperty('constructor'));
check(Email.constructor.hasOwnProperty('constructor'));
check(Email.constructor.constructor.hasOwnProperty('constructor'));
#endif // OUTPUT_VERSION > 5

// Anyway, Email was not created using 'new', so it does
// not have a __constructor__ property
check_equals(typeof(Email.__constructor__), 'undefined');
check( ! Email.hasOwnProperty('__constructor__') );

Email.prototype = new Mail;

check_equals(typeof(Email.prototype.constructor), 'function');
check_equals(Email.prototype.constructor, Mail);

#if OUTPUT_VERSION > 5 
check_equals(typeof(Email.prototype.__constructor__), 'function');
check_equals(Email.prototype.__constructor__, Email.prototype.constructor);
#else
check_equals(typeof(Email.prototype.__constructor__), 'undefined');
#endif

myMail = new Email('greetings', "you", "hello");
check_equals(myMail.subject, 'greetings');

#if OUTPUT_VERSION > 5
check_equals(myMail.to, 'you');
check_equals(myMail.message, 'hello');
#else // OUTPUT_VERSION <= 5
// no 'super' defined for SWF5 and below, so don't expect it to be called
check_equals(typeof(myMail.to), 'undefined');
check_equals(typeof(myMail.message), 'undefined');
#endif

function Spam()
{
	this.to = 'everyone';
	this.message = 'enlarge yourself';
}

Email.prototype.__constructor__ = Spam;

myMail = new Email('greetings', "you", "hello");
check_equals(myMail.subject, 'greetings');
#if OUTPUT_VERSION > 5
check_equals(myMail.to, 'everyone');
check_equals(myMail.message, 'enlarge yourself');
#else
check_equals(myMail.to, undefined);
check_equals(myMail.message, undefined);
#endif

//-----------------------------------------------------
// Test the 'this' reference
//-----------------------------------------------------

getThis = function() { return this; };

check_equals(getThis(), this);

o = new Object;
o.getThis = getThis;
check_equals(o.getThis(), o);

o.sub = new Object;
o.sub.getThis = getThis;

ret = o.sub.getThis();
check_equals(typeof(ret), 'object');
check_equals(ret, o.sub);


with(o) {
	with(sub) {
		ret = getThis();
		check_equals(typeof(ret), 'object');
		check_equals(ret, o.sub);
	}

	ret = getThis();
	check_equals(typeof(ret), 'object');
	check_equals(ret, o);

	ret = sub.getThis();
	check_equals(typeof(ret), 'object');
	check_equals(ret, sub);
}

check(delete o.sub.getThis);

with(o) {
	with (sub) {
		ret = getThis();
		check_equals(typeof(ret), 'object');
		check_equals(getThis(), o);
	}
}

function testInFunctionContext(o)
{
	var localGetThis = function() { return this; };
	ret = localGetThis();
	check_equals(typeof(ret), 'object');
#if OUTPUT_VERSION < 6
	check(ret == testInFunctionContext);
#else
	check(ret != testInFunctionContext);
#endif
	check(ret != this);

	var num = 4;
	with(o) {
		// see bug #19704
		ret = getThis();
		check_equals(typeof(ret), 'object');
		check_equals(ret, o);

		// 'with' stack takes precedence over locals
		check_equals(num, 5);
	}
	check_equals(num, 4);
}

o.num = 5;
testInFunctionContext(o);

//-----------------------------------------------------------------------------
// Test local vars scope of outer function to be kept alive by inner functions
//-----------------------------------------------------------------------------

foo = function () {
	var x = 42;
	return function () { return x; }; 
};

f = foo();
delete foo;
#if OUTPUT_VERSION > 5
check_equals(f(), 42);
#else
check_equals(typeof(f()), 'undefined');
#endif

#ifdef MING_SUPPORTS_ASM
//
// --case1--
//
testvar1 = 0;
testvar2 = 0;
testvar3 = 0;
asm{
    push 'testvar1'
    push 1
    push 'testvar2'
    push 2
    push 'testvar3'
    push 3
};
function stack_test1()
{
    asm{
        setvariable
        setvariable
        setvariable
    };
}

stack_test1();

xcheck_equals(testvar1, 1);
xcheck_equals(testvar2, 2);
xcheck_equals(testvar3, 3);

//
// --case2--
//
testvar1 = 0;
testvar2 = 0;
testvar3 = 0;
asm{
    push 'testvar1'
    push 4
    push 'testvar2'
    push 5
    push 'testvar3'
    push 6
};
_root.createEmptyMovieClip("clip1", '9');
clip1.stack_test2 = function () {
    asm{
        setvariable
        setvariable
        setvariable
    };
};

clip1.stack_test2();

#if OUTPUT_VERSION > 5
    xcheck_equals(testvar1, 4);
    xcheck_equals(testvar2, 5);
    xcheck_equals(testvar3, 6);
#endif

//
// --case3--
//

testvar1 = 0;
testvar2 = 0;
testvar3 = 0;

function stack_test3 () {
    asm{
        // Please check the produced swf file to see if the 
        // structure of opcodes are that you expect.
        push 7, 8, 9, 'pad to make Ming work as I expect'
        // pop out the pad stuff, I just want to push 7,8,9 actually. 
        pop 
    };
}

outer_func1 = function () {
    stack_test3();
};

outer_func2 = function () {
    outer_func1();
};

outer_func2();

//stack content after calling outer_func2:
//    7, 8, 9

asm{
    push 'testvar1'
    swap
    setvariable
};

//stack: 7, 8
asm{
    push 'testvar2'
    swap
    setvariable
};

//stack: 7
asm{
    push 'testvar3'
    swap
    setvariable
};
xcheck_equals(testvar1, 9);
xcheck_equals(testvar2, 8);
xcheck_equals(testvar3, 7);

#endif //MING_SUPPORTS_ASM

//-----------------------------------------------------------------------------
// Test that local var names are still declared, even if not passed by caller
//-----------------------------------------------------------------------------

function inc(a,b)
{
	a.count++;
	b.count++;
}
a={count:1}; b={count:1};
inc(a);
check_equals(a.count, 2);
check_equals(b.count, 1); // See bug #22203

#if OUTPUT_VERSION == 5
 check_totals(147); // SWF5
#endif
#if OUTPUT_VERSION == 6
 check_totals(207); // SWF6
#endif
#if OUTPUT_VERSION >= 7
 check_totals(208); // SWF7,SWF8
#endif
