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

rcsid="$Id: Point.as,v 1.3 2008/05/19 11:51:41 strk Exp $";

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

p0 = new Point('x', 'y');
ret = p0.add();
check(ret instanceof Point);
check_equals(p0.toString(), '(x=x, y=y)');
check_equals(ret.toString(), '(x=xundefined, y=yundefined)');
String.prototype.x = 3; // to test it's used
ret = p0.add('1');
delete String.prototype.x;
check(ret instanceof Point);
check_equals(ret.toString(), '(x=x3, y=yundefined)');
check_equals(p0.toString(), '(x=x, y=y)');
ret = p0.add(1, '2');
check(ret instanceof Point);
check_equals(ret.toString(), '(x=xundefined, y=yundefined)');
check_equals(p0.toString(), '(x=x, y=y)');

p0 = new Point('x', 'y');
p1 = new Point('x1', 'y1');
ret = p0.add(p1);
check(ret instanceof Point);
check_equals(ret.toString(), '(x=xx1, y=yy1)');
check_equals(p0.toString(), '(x=x, y=y)');
check_equals(p1.toString(), '(x=x1, y=y1)');

p0 = new Point(2, 3);
p1 = { x:1, y:1 };
ret = p0.add(p1);
check_equals(ret.toString(), '(x=3, y=4)');

ret = p0.add(p1, 4, 5, 6);
check_equals(ret.toString(), '(x=3, y=4)');

//-------------------------------------------------------------
// Test Point.clone
//-------------------------------------------------------------

p0 = new Point(3, 4);
p0.z = 5;
p2 = p0.clone();
check(p2 instanceof Point);
check_equals(p2.toString(), "(x=3, y=4)");
check_equals(typeof(p2.z), 'undefined');
p2 = p0.clone(1, 2, 3);
check(p2 instanceof Point);
check_equals(p2.toString(), "(x=3, y=4)");

//-------------------------------------------------------------
// Test Point.distance (static)
//-------------------------------------------------------------

dist = Point.distance();
check_equals(typeof(dist), 'undefined');

dist = Point.distance(undefined);
check_equals(typeof(dist), 'undefined');

o0 = {x:10, y:1};
o1 = {x:21, y:1};
dist = Point.distance(o0, o1);
check_equals(typeof(dist), 'undefined');

p0 = new Point('x', 'y');
p1 = new Point('a', 'b');
dist = Point.distance(p0, p1);
check_equals(typeof(dist), 'number');
check(isNaN(dist));
dist = p0.distance(p1);
check_equals(typeof(dist), 'undefined');

p0 = new Point('10', '20');
p1 = new Point('10', 'y');
dist = Point.distance(p0, p1);
check_equals(typeof(dist), 'number');
check(isNaN(dist));
dist = p0.distance(p1);
check_equals(typeof(dist), 'undefined');

p0 = new Point('10', 'y');
p1 = new Point('10', '20');
dist = Point.distance(p0, p1);
check_equals(typeof(dist), 'number');
check(isNaN(dist));
dist = p0.distance(p1);
check_equals(typeof(dist), 'undefined');

p0 = new Point('5', '4');
p1 = new Point('4', '7');
dist = Point.distance(p0, p1);
check_equals(typeof(dist), 'number');
check_equals(Math.round(dist*100), 316);
dist = p0.distance(p1);
check_equals(typeof(dist), 'undefined');

p0 = new Point('1', '1');
p1 = new Point('10', '1');
dist = Point.distance(p0, p1);
check_equals(typeof(dist), 'number');
check_equals(dist, 9);

// Doesn't matter if second arg is an instanceof Point
dist = Point.distance(p0, o1);
check_equals(typeof(dist), 'number');
check_equals(dist, 20);

// But first arg *must* be instanceof point !
dist = Point.distance(o1, p0);
check_equals(typeof(dist), 'undefined');
o1.__proto__ = Point.prototype;
dist = Point.distance(o1, p0);
check_equals(dist, 20);

//-------------------------------------------------------------
// Test Point.equals
//-------------------------------------------------------------

o0 = {};
o0.valueOf = function() { return 4; };
o1 = {};
o1.valueOf = function() { return 4; };

p0 = new Point(3, o0);
check(p0.equals(p0));

p1 = new Point(3, o1);
check(p1.equals(p1));

check(p0 != p1);
check_equals(p0.toString(), p1.toString());

check(!p0.equals(p1));
check(!p1.equals(p0));

ret = p0.equals();
check_equals(typeof(ret), 'boolean');
check(!ret);

p2 = new Point(3, o1);
check(p1.equals(p2));
// Equals doesn't return true if p2 isn't an point
p2 = {x:3, y:o1};
ret = p1.equals(p2);
check_equals(typeof(ret), 'boolean');
check(!ret);
// But we can cheat ...
p2.__proto__ = Point.prototype;
check(p1.equals(p2));
// ... even with double jump to get there ...
o3 = {}; o3.prototype = {}; o3.prototype.__proto__ = Point.prototype;
p2.__proto__ = o3.prototype;
check(p1.equals(p2));
// ... but not with syntetized objects ?
String.prototype.x = 3;
String.prototype.y = o1;
String.prototype.__proto__ = Point.prototype;
check(!p1.equals('string'));


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

check_totals(92);

#endif // OUTPUT_VERSION >= 8
