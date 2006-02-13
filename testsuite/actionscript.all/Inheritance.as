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
//

// Test case for ActionScript inheritance
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


#include "check.as"

// Function.apply is Function.__proto__.apply
check (Function.apply != undefined);
check (Function.apply === Function.__proto__.apply);

// Function.prototype is a reference to Function.__proto__ (why?)
//check (Function.prototype === Function.__proto__);
//check (Function.prototype.constructor === Function.__proto__.constructor);

// Confirm '__proto__' and 'prototype' members
// for Function to be the same thing
Function.prototype.fake = function(){};
check(Function.fake === Function.__proto__.fake);
check(Function.fake == Function.prototype.fake);

// Make 'functionObject' be an instance of Function (an object)
var functionObject = new Function();

// functionObject '__proto__' is a reference to
// it's constructor's 'prototype' member.
check (functionObject.__proto__ == Function.prototype);
check (functionObject.__proto__.constructor == Function);

// functionObject.apply should be functionObject.__proto__.apply
check (functionObject.apply != undefined);
check (functionObject.apply == Function.prototype.apply);
check (functionObject.apply == functionObject.__proto__.apply);

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
userFunc.apply = function() {};
check (userFunc.apply != Function.prototype.apply);


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
function Super() {
	this.sayHello = function() { return "hello from Super"; };
}

// Define a class derived from Super
function Sub () {}
Sub.prototype = new Super();
subInstance = new Sub();
check(subInstance.sayHello() == "hello from Super" );
Sub.prototype.sayHello = function() { return "hello from Sub"; };
check(subInstance.sayHello() == "hello from Sub" );
subInstance.sayHello = function() { return "hello from subInstance"; };
check(subInstance.sayHello() == "hello from subInstance" );
