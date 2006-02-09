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

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "check.as"

// Define a function returning 'this'.name and the given args
function getThisName(a,b,c) { return this.name+a+b+c; }
check (getThisName != undefined);
check ( typeof(getThisName) == "function" );

// Test Function.apply(this_ref)
var this_ref = {name:"extname"};
check ( getThisName.apply(this_ref) == "extname" );

// Test Function.apply(this_ref, args_array)
var ret=getThisName.apply(this_ref, [1,2,3]);
check ( ret == "extname123" );

// Define a class with its constructor
var TestClass = function() {
	this.name = "NONE";
};

// Test the Function constuctor
check (TestClass != undefined);
check ( typeof(TestClass) == "function" );

// test existance of the Function::apply method
check (TestClass.apply != undefined);

// test existance of the Function::call method
check (TestClass.call != undefined);

// test existance of the Function::prototype member
check (TestClass.prototype != undefined);

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

// Test the instanceof operator
check ( testInstance instanceof TestClass );

