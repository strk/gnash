// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

// Test case for Instance construction
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Boolean.as";
#include "check.as"


n = 3;
o = new n;
check_equals(o, undefined);

// o = new 3;
asm {
    push "o"
    push 0
    push "3"
    new
    setvariable
};
check_equals(o, undefined);

o = new Math.cos(9);
check_equals(typeof(o), "object");
check_equals(typeof(o.__proto__), "undefined");
ASSetPropFlags(o, null, 6, 1);
#if OUTPUT_VERSION < 7
    check_equals(typeof(o.constructor), "function");
#else
    check_equals(typeof(o.constructor), "undefined");
#endif
#if OUTPUT_VERSION > 5
check_equals(typeof(o.__constructor__), "function");
#else
check_equals(typeof(o.__constructor__), "undefined");
#endif
check_equals(o.toString(), undefined);
check_equals(o.valueOf(), undefined);
check(!o instanceOf Object);
check(!o instanceOf Number);
check(!o instanceOf String);

o = new Math.cos();
check_equals(typeof(o), "object");
check_equals(typeof(o.__proto__), "undefined");
#if OUTPUT_VERSION < 7
    check_equals(typeof(o.constructor), "function");
#else
    check_equals(typeof(o.constructor), "undefined");
#endif
check_equals(o.toString(), undefined);
check_equals(o.valueOf(), undefined);
check(!o instanceOf Object);
check(!o instanceOf Number);
check(!o instanceOf String);

o = new Mouse.hide();
check_equals(typeof(o), "object");
check_equals(typeof(o.__proto__), "undefined");
#if OUTPUT_VERSION < 7
    check_equals(typeof(o.constructor), "function");
#else
    check_equals(typeof(o.constructor), "undefined");
#endif
check_equals(o.toString(), undefined);
check_equals(o.valueOf(), undefined);
check(!o instanceOf Object);
check(!o instanceOf Number);
check(!o instanceOf String);

o = new Stage.align();
check_equals(typeof(o), "undefined");

o = new Date.UTC();
check_equals(typeof(o), "object");
check_equals(o.toString(), undefined);
check_equals(o.valueOf(), undefined);
check(!o instanceOf Object);

// This should be undefined in SWF7 and below because BitmapData doesn't exist.
// It should be undefined in SWF8 because the object isn't constructed when the
// given values are incorrect.
o = new flash.display.BitmapData();
check_equals(typeof(o), "undefined");
check_equals(o, undefined);

// Check object.prototype
// It seems this can't be changed under any circumstances.
delete Object.prototype;
check_equals(typeof(Object.prototype), "object");
Object.prototype = 6;
check_equals(typeof(Object.prototype), "object");
check_equals(Object.prototype.toString(), "[object Object]");
ASSetPropFlags(Object, null, 0);
delete Object.prototype;
check_equals(typeof(Object.prototype), "object");

// String.prototype can be changed.
String.prototype = 8;
check_equals(typeof(String.prototype), "number");
check_equals(String.prototype, 8);
s = new String("hello");
#if OUTPUT_VERSION == 5
check_equals(s, undefined);
#else
check_equals(s, undefined);
#endif
check_equals(s.__proto__, 8);
check_equals(typeof(s), "object");
check(!s instanceOf String);

s = new Object("hello");
#if OUTPUT_VERSION == 5
check_equals(s, undefined);
#else
check_equals(s, undefined);
#endif

Cl = function() {};
Cl.prototype = 8;
c = new Cl();
check_equals(c.__proto__, 8);

check_totals(46);
