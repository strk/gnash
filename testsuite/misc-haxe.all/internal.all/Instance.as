//FIXME: this needs to be either adapted for Flash 9 or ported to haxe. 

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


rcsid="$Id: Boolean.as,v 1.21 2008/04/02 09:39:59 strk Exp $";
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
    #if OUTPUT_VERSION == 6
    // SWF6 passes
    check_equals(typeof(o.constructor), "function");
    #else
    // SWF5 doesn't
    xcheck_equals(typeof(o.constructor), "function");
    #endif
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
    #if OUTPUT_VERSION == 6
    // SWF6 passes
    check_equals(typeof(o.constructor), "function");
    #else
    // SWF5 doesn't
    xcheck_equals(typeof(o.constructor), "function");
    #endif
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
    #if OUTPUT_VERSION == 6
    // SWF6 passes
    check_equals(typeof(o.constructor), "function");
    #else
    // SWF5 doesn't
    xcheck_equals(typeof(o.constructor), "function");
    #endif
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
#if OUTPUT_VERSION < 8
check_equals(typeof(o), "undefined");
check_equals(o, undefined);
#else
xcheck_equals(typeof(o), "undefined");
xcheck_equals(o, undefined);
#endif

check_totals(34);
