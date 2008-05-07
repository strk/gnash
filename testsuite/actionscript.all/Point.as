// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

rcsid="$Id: Point.as,v 1.1 2008/05/07 08:03:40 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 8

check_equals(typeof(flash), 'undefined');

check_totals(1);

#else

Point = flash.geom.Point;
check_equals(typeof(Point), 'function');
check_equals(typeof(Point.prototype), 'object');
check(Point.prototype.hasOwnProperty('length'));
check(!Point.prototype.hasOwnProperty('x'));
check(!Point.prototype.hasOwnProperty('y'));
check(Point.prototype.hasOwnProperty('add'));
check(Point.prototype.hasOwnProperty('clone'));
check(!Point.prototype.hasOwnProperty('distance'));
check(Point.hasOwnProperty('distance'));
check(Point.prototype.hasOwnProperty('equals'));
check(!Point.prototype.hasOwnProperty('interpolate'));
check(Point.hasOwnProperty('interpolate'));
check(Point.prototype.hasOwnProperty('normalize'));
check(Point.prototype.hasOwnProperty('offset'));
check(!Point.prototype.hasOwnProperty('polar'));
check(Point.hasOwnProperty('polar'));
check(Point.prototype.hasOwnProperty('subtract'));
check(Point.prototype.hasOwnProperty('toString'));

//-------------------------------------------------------------
// Test constructor (and x, y, length)
//-------------------------------------------------------------

p0 = new Point();
check_equals(typeof(p0), 'object');
check(p0 instanceof Point);
check(p0.hasOwnProperty('x'));
check(p0.hasOwnProperty('y'));
check_equals(''+p0, '(x=0, y=0)');
check_equals(typeof(p0.x), 'number');
check_equals(typeof(p0.y), 'number');
check_equals(typeof(p0.length), 'number');
check_equals(p0.length, 0);

a = []; for (var i in p0) a.push(i);
check_equals(a.length, 10); // most of them...

p0 = new Point('x', 'y');
check_equals(''+p0, '(x=x, y=y)');
check_equals(typeof(p0.x), 'string');
check_equals(typeof(p0.y), 'string');
check_equals(typeof(p0.length), 'number');
check(isNaN(p0.length));
p0.x = 1;
check(isNaN(p0.length));
p0.y = 0;
check_equals(p0.length, 1);

p0 = new Point(3, 4);
check_equals(p0.length, 5);

ASSetPropFlags(p0, "length", 0, 4); // clear read-only (if any)
p0.length = 10;
check_equals(p0.length, 5);

//-------------------------------------------------------------
// Test Point.add
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.clone
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.distance (static)
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.equals
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.interpolate (static)
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.normalize
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.offset
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.polar (static)
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test Point.subtract
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

check_totals(37);

#endif // OUTPUT_VERSION >= 8
