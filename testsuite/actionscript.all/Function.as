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

// Define USE_XTRACE to use "visual" trace
#ifdef USE_XTRACE
# include "xtrace.as"
# define trace xtrace
#endif

// Define a class with its constructor
var TestClass = function() {
	this.name = "NONE";
};

// Test the Function constuctor
if (TestClass != undefined) {
	if ( typeof(TestClass) == "function" ) {
		trace("PASSED: Function constructor");
	} else {
		trace("FAILED: Function constructor (is a "+typeof(TestClass)+", not a function)");
	}
} else {
	trace("FAILED: Function constructor");
}

// test existance of the Function::apply method
if (TestClass.apply != undefined) {
	trace("PASSED: Function::apply() is defined");
} else {
	trace("FAILED: Function::apply() is undefined");
}

// test existance of the Function::call method
if (TestClass.call != undefined) {
	trace("PASSED: Function::call() is defined");
} else {
	trace("FAILED: Function::call() is undefined");
}

// test existance of the Function::prototype member
if (TestClass.prototype != undefined) {
	trace("PASSED: Function.prototype is defined");
} else {
	trace("FAILED: Function.prototype is undefined");
}

// Define methods 
TestClass.prototype.setname = function(name) {
	this.name = name;
};

// Test instanciation of Function
var testInstance = new TestClass;
if (testInstance != undefined) {
	trace("PASSED: Function instance is defined");
} else {
	trace("FAILED: Function instance is undefined");
}
if (testInstance.name != undefined) {
	trace("PASSED: Function instance member is defined");
} else {
	trace("FAILED: Function instance member is undefined");
}
if (testInstance.name == "NONE") {
	trace("PASSED: Function instance member is initialized");
} else {
	trace("FAILED: Function instance member is not initialized (class ctor failed)");
}
if (typeof(testInstance.setname) == "function") {
	trace("PASSED: Function instance method is function");
} else {
	trace("FAILED: Function instance method is "+typeof(testInstance.setname));
}

// Test methods call
testInstance.setname("Test");
if (testInstance.name == "Test") {
	trace("PASSED: Function instance method works");
} else {
	trace("FAILED: Function instance method doesn't work");
}

// Test the instanceof operator
if ( testInstance instanceof TestClass ) {
	trace("PASSED: testInstance instance of TestClass");
} else {
	trace("FAILED: testInstance not instance of TestClass");
}
