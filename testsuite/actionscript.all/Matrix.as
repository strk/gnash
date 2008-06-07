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

rcsid="$Id: Matrix.as,v 1.1 2008/06/07 09:36:33 bwy Exp $";


// There are lots of floating point calculations here. Comparing them
// as a string means they are xchecked to the 15th significant digit, which
// isn't always unreasonable. I've handled the ones that failed for me with
// an epsilon; others may fail for other people - so please edit as necessary
// - but it's good to be as accurate as possible.
#include "check.as"

#if OUTPUT_VERSION < 8

check_equals(typeof(flash), 'undefined');

check_totals(1);

#else

Matrix = flash.geom.Matrix;
check_equals(typeof(Matrix), 'function');
check_equals(typeof(Matrix.prototype), 'object');
// TODO: xcheck prototypes properties
//xcheck(Point.prototype.hasOwnProperty('length'));


//-------------------------------------------------------------
// Test constructor
//-------------------------------------------------------------

m = new Matrix();
check_equals(typeof(m), 'object');
check(m instanceof Matrix);
xcheck(m.hasOwnProperty('a'));
xcheck(m.hasOwnProperty('b'));
xcheck(m.hasOwnProperty('c'));
xcheck(m.hasOwnProperty('d'));
xcheck(m.hasOwnProperty("tx"));
xcheck(m.hasOwnProperty("ty"));

xcheck_equals(m.a, 1);
xcheck_equals(m.b, 0);
xcheck_equals(m.c, 0);
xcheck_equals(m.d, 1);
xcheck_equals(m.tx, 0);
xcheck_equals(m.ty, 0);

xcheck_equals (m.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

m.b = 5.4;
xcheck_equals(m.b, "5.4");

m.c = 4.548759874;
xcheck_equals(m.c, "4.548759874");

m.rotate(67);
// PP: (a=4.10203808667724, b=-3.65147689783865, c=-1.49969051027619, d=-4.40932475155777, tx=0, ty=0)
// Some of these may be accurate enough:
xcheck_equals(m.toString(), "(a=4.10203808667724, b=-3.65147689783865, c=-1.49969051027619, d=-4.40932475155777, tx=0, ty=0)");
xcheck (m.a < 4.1020381 && m.a > 4.1020379);
xcheck (m.b < -3.6514768 && m.b > -3.6514769);
xcheck (m.c < -1.49969051 && m.c > -1.49969052);
xcheck (m.d < -4.40932475 && m.d > -4.40932476);
xcheck (m.tx == 0);
xcheck (m.ty == 0);

m.scale(-343, 0.33874983);
xcheck_equals (m.toString(), "(a=-1406.99906373029, b=-1.23693717839177, c=514.393845024734, d=-1.49365801000499, tx=0, ty=0)");

m.translate(333,-283747.22);
xcheck_equals (m.toString(), "(a=-1406.99906373029, b=-1.23693717839177, c=514.393845024734, d=-1.49365801000499, tx=333, ty=-283747.22)");

m.scale(4798747e+98, 0.33874983);
// PP: (a=-6.75183253607854e+107, b=-0.419012258900892, c=2.46844592063091e+107, d=-0.505976396967328, tx=1.597982751e+107, ty=-96119.3225379726)
// I get one discrepancy in 'a' here.
xcheck(m.a < -6.7518325360784e+107 && m.a > -6.7518325360786e+107)
xcheck_equals(m.b.toString(), "-0.419012258900892");
xcheck_equals(m.c.toString(), "2.46844592063091e+107");
xcheck_equals(m.d.toString(), "-0.505976396967328");
xcheck_equals(m.tx.toString(), "1.597982751e+107");
xcheck_equals(m.ty.toString(), "-96119.3225379726");

m.rotate(-1.2873874);
// PP: (a=-1.888016310255e+107, b=6.48248694618508e+107, c=6.9025203664787e+106, d=-2.36997413255563e+107, tx=4.46844242844096e+106, ty=-1.53423567131344e+107)
// tx is slightly different for me.
xcheck_equals(m.a.toString(), "-1.888016310255e+107");
xcheck_equals(m.b.toString(), "6.48248694618508e+107");
xcheck_equals(m.c.toString(), "6.9025203664787e+106");
xcheck_equals(m.d.toString(), "-2.36997413255563e+107");
xcheck(m.tx < 4.46844242844097e+106 && m.tx > 4.46844242844095e+106)
xcheck_equals(m.ty.toString(), "-1.53423567131344e+107");


m1 = new Matrix(8);
xcheck_equals(m1.toString(), "(a=8, b=undefined, c=undefined, d=undefined, tx=undefined, ty=undefined)");

m1 = new Matrix(1, 2, 3, 4, 5, 6);
xcheck_equals(m1.toString(), "(a=1, b=2, c=3, d=4, tx=5, ty=6)");

m1.scale(32, -3.4);
xcheck_equals(m1.toString(), "(a=32, b=-6.8, c=96, d=-13.6, tx=160, ty=-20.4)");


m2 = new Matrix();
m2.rotate(Math.PI);

// PP: "(a=-1, b=1.22460635382238e-16, c=-1.22460635382238e-16, d=-1, tx=0, ty=0) ");
// Can we risk it?
xcheck_equals (m2.a, -1);
xcheck (m2.b < 0.00000000001 && m2.b > -0.000000000001);
xcheck (m2.c < 0.00000000001 && m2.c > -0.000000000001);
xcheck (m2.d == -1);
// Probably okay:
xcheck (m2.tx == 0);
xcheck (m2.ty == 0);

m2.rotate();

m2 = new Matrix();
m2.rotate(1.3);

//PP: (a=0.267498828624587, b=0.963558185417193, c=-0.963558185417193, d=0.267498828624587, tx=0, ty=0)
note(m2.toString());
xcheck (m2.b < 0.96355819 && m2.b > 0.96355818);
xcheck (m2.c < -0.96355818 && m2.c > -0.96355819);
xcheck (m2.d < 0.26749883 && m2.d > 0.26749881);
xcheck (m2.tx == 0);
xcheck (m2.ty == 0);


m3 = new Matrix(2, 0, 0, 2, 100, 100);
m3.scale(3, 4);
xcheck_equals(m3.toString(), "(a=6, b=0, c=0, d=8, tx=300, ty=400)");

m3.invert();
xcheck_equals(m3.toString(), "(a=0.166666666666667, b=0, c=0, d=0.125, tx=-50, ty=-50)");

// Rotation applies to translation
m3 = new Matrix(1, 0, 0, 1, 2, 2);
m3.rotate (Math.PI / 2);
// PP: "(a=6.12303176911189e-17, b=1, c=-1, d=6.12303176911189e-17, tx=-2, ty=2)"
xcheck_equals(m3.b.toString(), "1");
xcheck_equals(m3.c.toString(), "-1");
xcheck_equals(m3.tx.toString(), "-2");
xcheck_equals(m3.ty.toString(), "2");


/*xcheck(p0.hasOwnProperty('y'));*/
/*xcheck_equals(''+p0, '(x=0, y=0)');*/
/*xcheck_equals(typeof(p0.x), 'number');*/
/*xcheck_equals(typeof(p0.y), 'number');*/
/*xcheck_equals(typeof(p0.length), 'number');*/
/*xcheck_equals(p0.length, 0);*/

/*a = []; for (var i in p0) a.push(i);*/
/*xcheck_equals(a.length, 10); // most of them...*/

/*p0 = new Point('x', 'y');*/
/*xcheck_equals(''+p0, '(x=x, y=y)');*/
/*xcheck_equals(typeof(p0.x), 'string');*/
/*xcheck_equals(typeof(p0.y), 'string');*/
/*xcheck_equals(typeof(p0.length), 'number');*/
/*xcheck(isNaN(p0.length));*/
/*p0.x = 1;*/
/*xcheck(isNaN(p0.length));*/
/*p0.y = 0;*/
/*xcheck_equals(p0.length, 1);*/

/*p0 = new Point(3, 4);*/
/*xcheck_equals(p0.length, 5);*/

/*ASSetPropFlags(p0, "length", 0, 4); // clear read-only (if any)*/
/*p0.length = 10;*/
/*xcheck_equals(p0.length, 5);*/

/*p0 = new Point(50, -Infinity);*/
/*xcheck_equals(p0.length, Infinity);*/

/*p0 = new Point(0, 0);*/
/*xcheck_equals(p0.length, 0);*/

/*//-------------------------------------------------------------*/
/*// Test Point.add*/
/*//-------------------------------------------------------------*/

/*p0 = new Point('x', 'y');*/
/*ret = p0.add();*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/
/*xcheck_equals(ret.toString(), '(x=xundefined, y=yundefined)');*/
/*String.prototype.x = 3; // to test it's used*/
/*ret = p0.add('1');*/
/*delete String.prototype.x;*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=x3, y=yundefined)');*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/
/*ret = p0.add(1, '2');*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=xundefined, y=yundefined)');*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/

/*p0 = new Point('x', 'y');*/
/*p1 = new Point('x1', 'y1');*/
/*ret = p0.add(p1);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=xx1, y=yy1)');*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/
/*xcheck_equals(p1.toString(), '(x=x1, y=y1)');*/

/*p0 = new Point(2, 3);*/
/*p1 = { x:1, y:1 };*/
/*ret = p0.add(p1);*/
/*xcheck_equals(ret.toString(), '(x=3, y=4)');*/

/*ret = p0.add(p1, 4, 5, 6);*/
/*xcheck_equals(ret.toString(), '(x=3, y=4)');*/

/*//-------------------------------------------------------------*/
/*// Test Point.clone*/
/*//-------------------------------------------------------------*/

/*p0 = new Point(3, 4);*/
/*p0.z = 5;*/
/*p2 = p0.clone();*/
/*xcheck(p2 instanceof Point);*/
/*xcheck_equals(p2.toString(), "(x=3, y=4)");*/
/*xcheck_equals(typeof(p2.z), 'undefined');*/
/*p2 = p0.clone(1, 2, 3);*/
/*xcheck(p2 instanceof Point);*/
/*xcheck_equals(p2.toString(), "(x=3, y=4)");*/

/*//-------------------------------------------------------------*/
/*// Test Point.distance (static)*/
/*//-------------------------------------------------------------*/

/*dist = Point.distance();*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*dist = Point.distance(undefined);*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*o0 = {x:10, y:1};*/
/*o1 = {x:21, y:1};*/
/*dist = Point.distance(o0, o1);*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*p0 = new Point('x', 'y');*/
/*p1 = new Point('a', 'b');*/
/*dist = Point.distance(p0, p1);*/
/*xcheck_equals(typeof(dist), 'number');*/
/*xcheck(isNaN(dist));*/
/*dist = p0.distance(p1);*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*p0 = new Point('10', '20');*/
/*p1 = new Point('10', 'y');*/
/*dist = Point.distance(p0, p1);*/
/*xcheck_equals(typeof(dist), 'number');*/
/*xcheck(isNaN(dist));*/
/*dist = p0.distance(p1);*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*p0 = new Point('10', 'y');*/
/*p1 = new Point('10', '20');*/
/*dist = Point.distance(p0, p1);*/
/*xcheck_equals(typeof(dist), 'number');*/
/*xcheck(isNaN(dist));*/
/*dist = p0.distance(p1);*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*p0 = new Point('5', '4');*/
/*p1 = new Point('4', '7');*/
/*dist = Point.distance(p0, p1);*/
/*xcheck_equals(typeof(dist), 'number');*/
/*xcheck_equals(Math.round(dist*100), 316);*/
/*dist = p0.distance(p1);*/
/*xcheck_equals(typeof(dist), 'undefined');*/

/*p0 = new Point('1', '1');*/
/*p1 = new Point('10', '1');*/
/*dist = Point.distance(p0, p1);*/
/*xcheck_equals(typeof(dist), 'number');*/
/*xcheck_equals(dist, 9);*/

/*// Doesn't matter if second arg is an instanceof Point*/
/*dist = Point.distance(p0, o1);*/
/*xcheck_equals(typeof(dist), 'number');*/
/*xcheck_equals(dist, 20);*/

/*// But first arg *must* be instanceof point !*/
/*dist = Point.distance(o1, p0);*/
/*xcheck_equals(typeof(dist), 'undefined');*/
/*o1.__proto__ = Point.prototype;*/
/*dist = Point.distance(o1, p0);*/
/*xcheck_equals(dist, 20);*/

/*//-------------------------------------------------------------*/
/*// Test Point.equals*/
/*//-------------------------------------------------------------*/

/*o0 = {};*/
/*o0.valueOf = function() { return 4; };*/
/*o1 = {};*/
/*o1.valueOf = function() { return 4; };*/

/*p0 = new Point(3, o0);*/
/*xcheck(p0.equals(p0));*/

/*p1 = new Point(3, o1);*/
/*xcheck(p1.equals(p1));*/

/*xcheck(p0 != p1);*/
/*xcheck_equals(p0.toString(), p1.toString());*/

/*xcheck(!p0.equals(p1));*/
/*xcheck(!p1.equals(p0));*/

/*ret = p0.equals();*/
/*xcheck_equals(typeof(ret), 'boolean');*/
/*xcheck(!ret);*/

/*p2 = new Point(3, o1);*/
/*xcheck(p1.equals(p2));*/
/*// Equals doesn't return true if p2 isn't an point*/
/*p2 = {x:3, y:o1};*/
/*ret = p1.equals(p2);*/
/*xcheck_equals(typeof(ret), 'boolean');*/
/*xcheck(!ret);*/
/*// But we can cheat ...*/
/*p2.__proto__ = Point.prototype;*/
/*xcheck(p1.equals(p2));*/
/*// ... even with double jump to get there ...*/
/*o3 = {}; o3.prototype = {}; o3.prototype.__proto__ = Point.prototype;*/
/*p2.__proto__ = o3.prototype;*/
/*xcheck(p1.equals(p2));*/
/*// ... but not with syntetized objects ?*/
/*String.prototype.x = 3;*/
/*String.prototype.y = o1;*/
/*String.prototype.__proto__ = Point.prototype;*/
/*xcheck(!p1.equals('string'));*/


/*//-------------------------------------------------------------*/
/*// Test Point.interpolate (static)*/
/*//-------------------------------------------------------------*/

/*ret = Point.interpolate();*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/

/*ret = Point.interpolate(1, 2, 3);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/

/*p0 = new Point('x0', 'y0');*/
/*p1 = new Point('x1', 'y1');*/
/*ret = Point.interpolate(p0, p1, 3);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=x1NaN, y=y1NaN)');*/

/*p0 = new Point('0', '0');*/
/*p1 = new Point('10', '0');*/
/*ret = Point.interpolate(p0, p1, 3);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=10-30, y=00)');*/
/*ret = Point.interpolate(p0, p1, 0);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=100, y=00)');*/
/*ret = Point.interpolate(p0, p1, 0.5);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=10-5, y=00)');*/

/*// second arg drives newAdd*/
/*p0 = new Point(0, 0);*/
/*p1 = new Point('10', '0');*/
/*ret = Point.interpolate(p0, p1, 3);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=10-30, y=00)');*/

/*// second arg drives newAdd*/
/*p0 = new Point('0', '0');*/
/*p1 = new Point(10, 0);*/
/*ret = Point.interpolate(p0, p1, 3);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=-20, y=0)');*/

/*p0 = new Point(0, 0);*/
/*p1 = new Point(10, 0);*/
/*ret = Point.interpolate(p0, p1, 0.5);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=5, y=0)');*/

/*p0 = new Point(0, 0);*/
/*p1 = new Point(10, 0);*/
/*ret = Point.interpolate(p0, p1, 1, 'discarder arg');*/
/*xcheck(ret.equals(p0));*/
/*ret = Point.interpolate(p0, p1, 0);*/
/*xcheck(ret.equals(p1));*/
/*ret = Point.interpolate(p0, p1);*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/

/*o0 = {x:0, y:10};*/
/*o1 = {x:10, y:0};*/
/*ret = Point.interpolate(o0, o1, 1);*/
/*xcheck_equals(ret.toString(), '(x=0, y=10)');*/
/*ret = Point.interpolate(o0, o1, 0);*/
/*xcheck_equals(ret.toString(), '(x=10, y=0)');*/
/*ret = Point.interpolate(o0, o1, 0.5);*/
/*xcheck_equals(ret.toString(), '(x=5, y=5)');*/


/*//-------------------------------------------------------------*/
/*// Test Point.normalize*/
/*//-------------------------------------------------------------*/

/*p0 = new Point(0, 0);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize();*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck(p1.equals(p0));*/

/*p0 = new Point(0, 0);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize(10);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck(p1.equals(p0));*/

/*p0 = new Point(10, 0);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize(5);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p1.toString(), '(x=5, y=0)');*/

/*p0 = new Point(0, 10);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize(-5);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p1.toString(), '(x=0, y=-5)');*/

/*p0 = new Point(3, -4);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize(-10);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p1.toString(), '(x=-6, y=8)');*/

/*p0 = new Point(-10, 0);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize(5);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p1.toString(), '(x=-5, y=0)');*/

/*p0 = new Point(-10, 0);*/
/*p1 = p0.clone();*/
/*ret = p1.normalize('r');*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p1.toString(), '(x=NaN, y=NaN)');*/

/*p0 = new Point('x', 'y');*/
/*p1 = p0.clone();*/
/*ret = p1.normalize(5);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p1.toString(), '(x=x, y=y)');*/

/*//-------------------------------------------------------------*/
/*// Test Point.offset*/
/*//-------------------------------------------------------------*/

/*p0 = new Point('x', 'y');*/
/*ret = p0.offset();*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p0.toString(), '(x=xundefined, y=yundefined)');*/

/*p0 = new Point('x', 'y');*/
/*ret = p0.offset('a');*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p0.toString(), '(x=xa, y=yundefined)');*/

/*p0 = new Point('x', 'y');*/
/*ret = p0.offset('a', 'b', 3);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p0.toString(), '(x=xa, y=yb)');*/

/*p0 = new Point(4, 5);*/
/*ret = p0.offset('-6', -8);*/
/*xcheck_equals(typeof(ret), 'undefined');*/
/*xcheck_equals(p0.toString(), '(x=4-6, y=-3)');*/

/*//-------------------------------------------------------------*/
/*// Test Point.polar (static)*/
/*//-------------------------------------------------------------*/

/*p0 = Point.polar();*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(p0.toString(), '(x=NaN, y=NaN)');*/

/*p0 = Point.polar(1);*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(p0.toString(), '(x=NaN, y=NaN)');*/

/*p0 = Point.polar(1, 0);*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(p0.toString(), '(x=1, y=0)');*/

/*p0 = Point.polar(1, Math.PI);*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(p0.x, -1);*/
/*xcheck_equals(Math.round(p0.y*100), 0);*/

/*p0 = Point.polar(1, Math.PI/2);*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(Math.round(p0.x*100), 0);*/
/*xcheck_equals(p0.y, 1);*/

/*p0 = Point.polar(1, Math.PI*2);*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(p0.x, 1);*/
/*xcheck_equals(Math.round(p0.y*100), 0);*/

/*p0 = Point.polar(1, Math.PI*1.5);*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(Math.round(p0.x*100), 0);*/
/*xcheck_equals(p0.y, -1);*/

/*p0 = Point.polar('5', '0');*/
/*xcheck(p0 instanceof Point);*/
/*xcheck_equals(p0.x, 5);*/
/*xcheck_equals(p0.y, 0);*/


/*//-------------------------------------------------------------*/
/*// Test Point.subtract*/
/*//-------------------------------------------------------------*/

/*p0 = new Point('x', 'y');*/
/*ret = p0.subtract();*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/
/*String.prototype.x = 3; // to test it's used*/
/*ret = p0.subtract('1');*/
/*delete String.prototype.x;*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/
/*ret = p0.subtract(1, '2');*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/

/*p0 = new Point('x', 'y');*/
/*p1 = new Point('x1', 'y1');*/
/*ret = p0.subtract(p1);*/
/*xcheck(ret instanceof Point);*/
/*xcheck_equals(ret.toString(), '(x=NaN, y=NaN)');*/
/*xcheck_equals(p0.toString(), '(x=x, y=y)');*/
/*xcheck_equals(p1.toString(), '(x=x1, y=y1)');*/

/*p0 = new Point(2, 3);*/
/*p1 = { x:1, y:1 };*/
/*ret = p0.subtract(p1);*/
/*xcheck_equals(ret.toString(), '(x=1, y=2)');*/

/*ret = p0.subtract(p1, 4, 5, 6);*/
/*xcheck_equals(ret.toString(), '(x=1, y=2)');*/

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals();

#endif // OUTPUT_VERSION >= 8
