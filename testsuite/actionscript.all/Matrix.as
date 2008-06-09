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

rcsid="$Id: Matrix.as,v 1.5 2008/06/09 12:37:01 bwy Exp $";


// There are lots of floating point calculations here. Comparing them
// as a string means they are checked to the 15th significant digit, which
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
check(Matrix.prototype.hasOwnProperty('identity'));
check(Matrix.prototype.hasOwnProperty('rotate'));
check(Matrix.prototype.hasOwnProperty('concat'));
check(Matrix.prototype.hasOwnProperty('translate'));
check(Matrix.prototype.hasOwnProperty('scale'));
check(Matrix.prototype.hasOwnProperty('transformPoint'));
check(Matrix.prototype.hasOwnProperty('deltaTransformPoint'));
check(Matrix.prototype.hasOwnProperty('createBox'));
check(Matrix.prototype.hasOwnProperty('clone'));
check(Matrix.prototype.hasOwnProperty('createGradientBox'));
check(Matrix.prototype.hasOwnProperty('invert'));
check(Matrix.prototype.hasOwnProperty('toString'));
check(!Matrix.prototype.hasOwnProperty('a'));
check(!Matrix.prototype.hasOwnProperty('b'));
check(!Matrix.prototype.hasOwnProperty('c'));
check(!Matrix.prototype.hasOwnProperty('d'));
check(!Matrix.prototype.hasOwnProperty('tx'));
check(!Matrix.prototype.hasOwnProperty('ty'));

//-------------------------------------------------------------
// Test constructor
//-------------------------------------------------------------

m = new Matrix();
check_equals(typeof(m), 'object');
check(m instanceof Matrix);
check(m.hasOwnProperty('a'));
check(m.hasOwnProperty('b'));
check(m.hasOwnProperty('c'));
check(m.hasOwnProperty('d'));
check(m.hasOwnProperty("tx"));
check(m.hasOwnProperty("ty"));

check_equals(m.a, 1);
check_equals(m.b, 0);
check_equals(m.c, 0);
check_equals(m.d, 1);
check_equals(m.tx, 0);
check_equals(m.ty, 0);

check_equals (m.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

m.b = 5.4;
check_equals(m.b, "5.4");

m.c = 4.548759874;
check_equals(m.c, "4.548759874");

m.rotate(67);
// PP: (a=4.10203808667724, b=-3.65147689783865, c=-1.49969051027619, d=-4.40932475155777, tx=0, ty=0)
// Some of these may be accurate enough:
check_equals(m.toString(), "(a=4.10203808667724, b=-3.65147689783865, c=-1.49969051027619, d=-4.40932475155777, tx=0, ty=0)");
check (m.a < 4.1020381 && m.a > 4.1020379);
check (m.b < -3.6514768 && m.b > -3.6514769);
check (m.c < -1.49969051 && m.c > -1.49969052);
check (m.d < -4.40932475 && m.d > -4.40932476);
check (m.tx == 0);
check (m.ty == 0);

m.scale(-343, 0.33874983);
check_equals (m.toString(), "(a=-1406.99906373029, b=-1.23693717839177, c=514.393845024734, d=-1.49365801000499, tx=0, ty=0)");

m.translate(333,-283747.22);
check_equals (m.toString(), "(a=-1406.99906373029, b=-1.23693717839177, c=514.393845024734, d=-1.49365801000499, tx=333, ty=-283747.22)");

m.scale(4798747e+98, 0.33874983);
// PP: (a=-6.75183253607854e+107, b=-0.419012258900892, c=2.46844592063091e+107, d=-0.505976396967328, tx=1.597982751e+107, ty=-96119.3225379726)
// I get one discrepancy in 'a' here.
check(m.a < -6.7518325360784e+107 && m.a > -6.7518325360786e+107)
check_equals(m.b.toString(), "-0.419012258900892");
check_equals(m.c.toString(), "2.46844592063091e+107");
check_equals(m.d.toString(), "-0.505976396967328");
check_equals(m.tx.toString(), "1.597982751e+107");
check_equals(m.ty.toString(), "-96119.3225379726");

m.rotate(-1.2873874);
// PP: (a=-1.888016310255e+107, b=6.48248694618508e+107, c=6.9025203664787e+106, d=-2.36997413255563e+107, tx=4.46844242844096e+106, ty=-1.53423567131344e+107)
// tx is slightly different for me.
check_equals(m.a.toString(), "-1.888016310255e+107");
check_equals(m.b.toString(), "6.48248694618508e+107");
check_equals(m.c.toString(), "6.9025203664787e+106");
check_equals(m.d.toString(), "-2.36997413255563e+107");
check(m.tx < 4.46844242844097e+106 && m.tx > 4.46844242844095e+106)
check_equals(m.ty.toString(), "-1.53423567131344e+107");


m1 = new Matrix(8);
check_equals(m1.toString(), "(a=8, b=undefined, c=undefined, d=undefined, tx=undefined, ty=undefined)");

m1 = new Matrix(1, 2, 3, 4, 5, 6);
check_equals(m1.toString(), "(a=1, b=2, c=3, d=4, tx=5, ty=6)");

m1.scale(32, -3.4);
check_equals(m1.toString(), "(a=32, b=-6.8, c=96, d=-13.6, tx=160, ty=-20.4)");
// Do not change m1; it's used later to test concat!

m2 = new Matrix();
m2.rotate(Math.PI);

// PP: "(a=-1, b=1.22460635382238e-16, c=-1.22460635382238e-16, d=-1, tx=0, ty=0) ");
check_equals (m2.a, -1);
check (m2.b < 0.00000000001 && m2.b > -0.000000000001);
check (m2.c < 0.00000000001 && m2.c > -0.000000000001);
check (m2.d == -1);
check (m2.tx == 0);
check (m2.ty == 0);

m2 = new Matrix();
m2.rotate(1.3);

//PP: (a=0.267498828624587, b=0.963558185417193, c=-0.963558185417193, d=0.267498828624587, tx=0, ty=0)
check (m2.b < 0.96355819 && m2.b > 0.96355818);
check (m2.c < -0.96355818 && m2.c > -0.96355819);
check (m2.d < 0.26749883 && m2.d > 0.26749881);
check (m2.tx == 0);
check (m2.ty == 0);
// Do not change m2 after this; it's used later to test concat!

///
/// Test deltaTransform of Point
///
Point = flash.geom.Point;
p = new Point(34, -23);
newP = m2.deltaTransformPoint(p);
check_equals(typeof(newP), "object");
check_equals(newP.toString(), "(x=31.2567984378314, y=26.6085052458191)");

// Scale
m3 = new Matrix(2, 0, 0, 2, 100, 100);
m3.scale(3, 4);
check_equals(m3.toString(), "(a=6, b=0, c=0, d=8, tx=300, ty=400)");
// Do not change m3; it is used to test invert!

// Test clone
m4 = m3.clone();
check_equals(m4.toString(), "(a=6, b=0, c=0, d=8, tx=300, ty=400)");
// Do not change m4; it's used later to test concat!


// Test invert
m3.invert();
check_equals(m3.toString(), "(a=0.166666666666667, b=0, c=0, d=0.125, tx=-50, ty=-50)");

// Invalid inverse
m6 = new Matrix(4, 5, 44, 55, 2, 4);
check_equals(m6.toString(), "(a=4, b=5, c=44, d=55, tx=2, ty=4)");
m6.invert();
check_equals(m6.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

// Valid inverse.
m6 = new Matrix(4, 5, 0, 5, 2, 3);
check_equals(m6.toString(), "(a=4, b=5, c=0, d=5, tx=2, ty=3)");
m6.invert();
check_equals(m6.toString(), "(a=0.25, b=-0.25, c=0, d=0.2, tx=-0.5, ty=-0.1)");


// Rotation applies to translation
m3 = new Matrix(1, 0, 0, 1, 2, 2);
m3.rotate (Math.PI / 2);
// PP: "(a=6.12303176911189e-17, b=1, c=-1, d=6.12303176911189e-17, tx=-2, ty=2)"
check_equals(m3.b.toString(), "1");
check_equals(m3.c.toString(), "-1");
check_equals(m3.tx.toString(), "-2");
check_equals(m3.ty.toString(), "2");

m5 = m3.identity();
/// identity() returns void.
check_equals(m5, undefined);
check_equals(m3.a.toString(), "1");
check_equals(m3.b.toString(), "0");
check_equals(m3.c.toString(), "0");
check_equals(m3.d.toString(), "1");
check_equals(m3.tx.toString(), "0");
check_equals(m3.ty.toString(), "0");

// m4 is still interesting
m4.concat(m1);
check_equals(m4.toString(), "(a=192, b=-40.8, c=768, d=-108.8, tx=48160, ty=-7500.4)");
m4.concat(m2);
// Works for me.
check_equals(m4.toString(), "(a=90.6729490609422, b=174.089219392218, c=310.274230957074, d=710.908813846049, tx=20109.8154004632, ty=44398.6139954762)");

m7 = new Matrix ("A string", undefined, new Object, true, NaN, new Point);
check_equals("" + m7, "(a=A string, b=undefined, c=[object Object], d=true, tx=NaN, ty=(x=0, y=0))");

m7.rotate(2);
check_equals(m7.toString(), "(a=NaN, b=NaN, c=NaN, d=NaN, tx=NaN, ty=NaN)");

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals();

#endif // OUTPUT_VERSION >= 8
