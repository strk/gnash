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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Function.as,v 1.11 2006/06/20 20:45:27 strk Exp $";

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

// Test Function.call(arg1, arg2, arg3)
check ( getThisName.call(this_ref, 1, 2, 3) == "extname123" );

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

// Test inheritance
check (testInstance.__proto__ != undefined);
check (testInstance.__proto__ == TestClass.prototype);
check (TestClass.prototype.constructor != undefined);
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
check (String.prototype.constructor != undefined);
check (String.prototype.constructor == String);
check (stringInstance.__proto__.constructor == String);

// Test the instanceof operator
check ( testInstance instanceof TestClass );
check ( stringInstance instanceof String );

