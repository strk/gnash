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

rcsid="$Id: Inheritance.as,v 1.13 2007/01/02 01:50:56 strk Exp $";

#include "check.as"

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

// Make 'userFunc' be a function (should inherit Function things)
var userFunc = function() {};
check (userFunc.__proto__ == Function.prototype);
check (userFunc.prototype.constructor == userFunc);
check (userFunc.prototype.apply == undefined);
check (userFunc.apply == Function.prototype.apply);

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
check(subInstance instanceOf Object);


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
check(Function instanceOf Object);
check(SubObj1 instanceOf Object);
#endif

// inheritance chain is NOT subobj1,SubObj1,Function,Object, as the
// above might suggest...
xcheck(!sobj1 instanceOf Function);

// but still, sobj1 is an instance of Object *and* SubObj1
check(sobj1 instanceOf Object);
check(sobj1 instanceOf SubObj1);

check(SubObj1.prototype != undefined);
check_equals(SubObj1.prototype.constructor, SubObj1);

check_equals(SubObj1.prototype.constructor.__proto__.constructor, Function);

