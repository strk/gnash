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

// Test case for Object ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

// uncomment the following to trace using xtrace()
#ifdef USE_XTRACE
# include "xtrace.as"
# define trace xtrace
#endif

// Test Object creation using 'new'
var obj = new Object;
if (obj != undefined) {
	if ( typeof(obj) == "object" ) {
		trace("PASSED: 'new Object'");
	} else {
		trace("FAILED: 'new Object' is a "+typeof(TestClass));
	}
} else {
	trace("FAILED: 'new Object' is undefined");
}

// Test Object creation using literal initialization
var obj = { member:1 };
if (obj != undefined) {
	if ( typeof(obj) == "object" ) {
		trace("PASSED: 'obj = { ... }'");
	} else {
		trace("FAILED: 'obj = { ... }' is a "+typeof(obj));
	}
} else {
	trace("FAILED: 'obj = { ... }' is undefined");
}

// Test initialized object members
if ( obj.member == 1 ) {
	trace("PASSED: initialization-provided obj.member is correctly set");
} else {
	if ( obj.member == undefined ) {
		trace("FAILED: initialization-provided obj.member is undefined");
	} else {
		trace("FAILED: initialization-provided obj.member is "+obj.member+" (should be 1)");
	}
}

// Test Object creation using initializing constructor
var obj = new Object({ member:1 });
if (obj != undefined) {
	if ( typeof(obj) == "object" ) {
		trace("PASSED: 'new Object({ ... })'");
	} else {
		trace("FAILED: 'new Object({ ... })' is a "+typeof(obj));
	}
} else {
	trace("FAILED: 'new Object({...})' is undefined");
}

// Test initialized object members
if ( obj.member == 1 ) {
	trace("PASSED: initialization-provided.member is correctly set");
} else {
	if ( obj.member == undefined ) {
		trace("FAILED: initialization-provided obj.member is undefined");
	} else {
		trace("FAILED: initialization-provided obj.member is "+obj.member+" (should be 1)");
	}
}


// Test after-initialization members set/get
obj.member2 = 3;
if ( obj.member2 == 3 ) {
	trace("PASSED: explicitly set obj.member2 is correctly set");
} else {
	if ( obj.member == undefined ) {
		trace("FAILED: explicitly set obj.member2 is undefined");
	} else {
		trace("FAILED: explicitly set obj.member2 is "+obj.member2+" (should be 1)");
	}
}
