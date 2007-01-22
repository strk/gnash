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

// Test case for ActionScript inheritance
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Inheritance.as,v 1.18 2007/01/22 22:12:18 strk Exp $";

#include "check.as"

check_equals(typeof(Object.prototype.constructor), 'function');
check_equals(Object.prototype.constructor, Object);

// Function.apply is Function.__proto__.apply
#if OUTPUT_VERSION > 5
check (typeof(Function.apply) != undefined);
#endif
check (Function.apply == Function.__proto__.apply);

// Confirm '__proto__' and 'prototype' members
// for Function to be the same thing
Function.prototype.fake = function(){};
check(Function.fake == Function.__proto__.fake);
check(Function.fake == Function.prototype.fake);

// Make 'functionObject' be an instance of Function (an object)
var functionObject = new Function();

#if OUTPUT_VERSION > 5
check_equals(typeof(functionObject), 'object');
#else
xcheck_equals(typeof(functionObject), 'undefined');
#endif

// functionObject '__proto__' is a reference to
// it's constructor's 'prototype' member.
check_equals (functionObject.__proto__, Function.prototype);
check_equals (functionObject.__proto__.constructor, Function);

// functionObject.apply should be functionObject.__proto__.apply
#if OUTPUT_VERSION > 5
check (functionObject.apply != undefined);
check (functionObject.apply == Function.prototype.apply);
check (functionObject.apply == functionObject.__proto__.apply);
#endif

// functionObject is an Object, not functionObject Function,
// so it doesn't have functionObject 'prototype' member
// itself.
check (functionObject.prototype == undefined);

#if OUTPUT_VERSION > 5
functionObject.prototype = {a:1};
check_equals(functionObject.prototype.a, 1);
#endif

// Make 'userFunc' be a function (should inherit Function things)
var userFunc = function() {};
check_equals (userFunc.__proto__, Function.prototype);
check_equals (userFunc.prototype.constructor, userFunc);
check_equals (userFunc.prototype.apply, undefined);
check_equals (userFunc.apply, Function.prototype.apply);

// Override the inherited apply() method
#if OUTPUT_VERSION > 5 
// Function.apply was introduced in SWF6
userFunc.apply = function() {};
check (userFunc.apply != Function.prototype.apply);
#endif


// Define the Ball constructor
function Ball (radius) {
	this.radius = radius;
}
check (Ball.prototype.constructor == Ball);
Ball.prototype.gravity = 9.8;

var myBall = new Ball(3);
check (myBall.gravity == 9.8);
check(myBall.radius == 3);

function sayHello() { return "Hello"; }
Ball.prototype.sayHello = sayHello;
check(myBall.sayHello() == "Hello");
check(myBall.gravity == 9.8);
myBall.gravity = 5;
check(myBall.gravity == 5);
check(myBall.__proto__ == Ball.prototype);

// Define a superclass
function SuperClass() {
	this.sayHello = function() { return "hello from SuperClass"; };
}

// Define a class derived from SuperClass
function SubClass () {}
SubClass.prototype = new SuperClass();
subInstance = new SubClass();
check_equals(subInstance.sayHello(), "hello from SuperClass" );
SubClass.prototype.sayHello = function() { return "hello from SubClass"; };
check_equals(subInstance.sayHello(), "hello from SubClass" );
subInstance.sayHello = function() { return "hello from subInstance"; };
check_equals(subInstance.sayHello(), "hello from subInstance" );

// Test the instanceOf operator
check(subInstance instanceOf SubClass);
check(subInstance instanceOf SuperClass);
xcheck(subInstance instanceOf Object);


//------------------------------------------------
// Test the 'super' keyword
//------------------------------------------------

#if OUTPUT_VERSION > 5
function BaseClass() {}
BaseClass.prototype.sayHello = function () {
  return "Hello from BaseClass"; 
};
function DerivedClass() {}
DerivedClass.prototype = new BaseClass();
DerivedClass.prototype.sayHello = function () {
  return "Hello from DerivedClass"; 
};
DerivedClass.prototype.saySuperHello = function () {
  return super.sayHello();
};
var derived = new DerivedClass();
var greeting = derived.saySuperHello();
check_equals(greeting, "Hello from BaseClass");
#endif // OUTPUT_VERSION > 5
check_equals(super, undefined);

//------------------------------------------------
//
//------------------------------------------------


/// THese have been moved here from inheritance.as
var obj = new Object({a:1});

check_equals(obj.__proto__.constructor, Object);
check(obj instanceOf Object);

function SubObj1() {}
var sobj1 = new SubObj1();

check_equals(sobj1.__proto__.constructor, SubObj1);
#if OUTPUT_VERSION > 5
check(SubObj1 instanceOf Function);
xcheck(Function instanceOf Object);
xcheck(SubObj1 instanceOf Object);
#endif

// inheritance chain is NOT subobj1,SubObj1,Function,Object, as the
// above might suggest...
check(!sobj1 instanceOf Function);

// but still, sobj1 is an instance of Object *and* SubObj1
xcheck(sobj1 instanceOf Object);
check(sobj1 instanceOf SubObj1);

check(SubObj1.prototype != undefined);
check_equals(SubObj1.prototype.constructor, SubObj1);

check_equals(SubObj1.prototype.constructor.__proto__.constructor, Function);

//------------------------------------------------
// Test the 'extends' tag (require ming > 0.4.0.beta3)
//------------------------------------------------

// see check.as
#ifdef MING_SUPPORTS_ASM_EXTENDS

function BaseClass1() {}
BaseClass1.prototype.var1 = "var_in_Base_prototype";
function DerivedClass1() {}
asm {
	push "DerivedClass1"
	getvariable
	push "BaseClass1"
	getvariable
	extends
};
DerivedClass1.prototype.var2 = "var_in_Derived_prototype";
var obj = new DerivedClass1;
check(obj instanceOf DerivedClass1);
check(obj instanceOf BaseClass1);
check_equals(obj.__proto__, DerivedClass1.prototype);
check_equals(DerivedClass1.prototype.constructor, BaseClass1);
check_equals(obj.var1, "var_in_Base_prototype");
check_equals(obj.var2, "var_in_Derived_prototype");

function MyClass() {}
asm {
	push "MyClass"
	getvariable
	push "MovieClip"
	getvariable
	extends
};
var myInstance = new MyClass();
myInstance._x = 4;
check_equals(myInstance._x, 4);

#endif // MING_SUPPORTS_ASM_EXTENDS

