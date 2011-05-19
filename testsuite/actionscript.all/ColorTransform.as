// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for TextFormat ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="ColorTransform.as";

#include "check.as"

#if OUTPUT_VERSION < 8

check_equals(typeof(flash), 'undefined');

check_totals(1);

#else

ColorTransform = flash.geom.ColorTransform;
check_equals(typeof(ColorTransform), 'function');
check_equals(typeof(ColorTransform.prototype), 'object');
check(ColorTransform.prototype.hasOwnProperty('rgb'));
check(ColorTransform.prototype.hasOwnProperty('toString'));
check(ColorTransform.prototype.hasOwnProperty('concat'));
check(ColorTransform.prototype.hasOwnProperty('redMultiplier'));
check(ColorTransform.prototype.hasOwnProperty('blueMultiplier'));
check(ColorTransform.prototype.hasOwnProperty('greenMultiplier'));
check(ColorTransform.prototype.hasOwnProperty('alphaMultiplier'));
check(ColorTransform.prototype.hasOwnProperty('redOffset'));
check(ColorTransform.prototype.hasOwnProperty('blueOffset'));
check(ColorTransform.prototype.hasOwnProperty('greenOffset'));
check(ColorTransform.prototype.hasOwnProperty('alphaOffset'));

//-------------------------------------------------------------
// Test constructor
//-------------------------------------------------------------

c = new ColorTransform;
check_equals(typeof(c), 'object');
check(c instanceof ColorTransform);

check_equals(c.redMultiplier, 1);
check_equals(c.blueMultiplier, 1);
check_equals(c.greenMultiplier, 1);
check_equals(c.alphaMultiplier, 1);
check_equals(c.redOffset, 0);
check_equals(c.blueOffset, 0);
check_equals(c.greenOffset, 0);
check_equals(c.alphaOffset, 0);

check_equals (c.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

c.redMultiplier = 5.4;
check_equals(c.redMultiplier.toString(), "5.4");

c.alphaMultiplier = -0.3;
check_equals(c.alphaMultiplier.toString(), "-0.3");

c.redOffset = 123;
check_equals(c.redOffset.toString(), "123");

c.greenOffset = 287;
check_equals(c.greenOffset.toString(), "287");

o = {};
o.valueOf = function() { return 456; };

c = new ColorTransform(new Object, 3, "string", true, ASnative(100,9), new Error("custom error"), undefined, o);

check_equals (c.toString(), "(redMultiplier=NaN, greenMultiplier=3, blueMultiplier=NaN, alphaMultiplier=1, redOffset=NaN, greenOffset=NaN, blueOffset=NaN, alphaOffset=456)");

// Only 8 or more arguments are valid.
c = new ColorTransform(0, 2, 3);
check_equals (c.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

c = new ColorTransform(0, 2, 3, 4);
check_equals (c.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

c = new ColorTransform(0, 2, 3, 4, 5, 6, 7);
check_equals (c.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

c = new ColorTransform(0, 2, 3, 4, 5, 6, 7, 8);
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)");

c = new ColorTransform(0, 2, 3, 4, 5, 6, -8.334, 9.7);
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=-8.334, alphaOffset=9.7)");

c = new ColorTransform(0, 2, 3, 4, 5, 6, 7, 8, 9);
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)");


// ColorTransform.rgb
c.rgb = 0xFFFF00;
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=0, alphaOffset=8)");
check_equals(c.rgb.toString(), "16776960");

c = new ColorTransform(1, 1, -0.5, 4, 2, 2, 2, 8);
c.rgb = 0xFF34FF;
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=52, blueOffset=255, alphaOffset=8)");
check_equals(c.rgb.toString(), "16725247");

c.rgb = 0x000000;
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=8)");
check_equals(c.rgb.toString(), "0");

c.rgb = -4534;
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=238, blueOffset=74, alphaOffset=8)");
check_equals(c.rgb.toString(), "16772682");

c.rgb = 0xFFFFFFFF;
check_equals (c.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=255, alphaOffset=8)");
check_equals(c.rgb.toString(), "16777215");

// It's just bitshifting...
c = new ColorTransform(1, 1, 1, 1, 1000, 1000, 1000, 0);
check_equals(c.rgb.toString(), "65793000");

c = new ColorTransform(1, 1, 1, 1, 1000000, 1000, 1000, 0);
check_equals(c.rgb.toString(), "1111747560");

c = new ColorTransform(1, 1, 1, 1, 100000000000000000, 10000, 1000, 0);
check_equals(c.rgb.toString(), "2561000");

// Check object type strictness.

o = { redMultiplier:2, greenMultiplier:3, blueMultiplier:3, alphaMultiplier:0, redOffset:3, greenOffset:4, blueOffset:3, alphaOffset:3 };
o.toString = ColorTransform.toString;
check_equals(o.toString(), "[object Object]");



//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals(49);

#endif // OUTPUT_VERSION >= 8
