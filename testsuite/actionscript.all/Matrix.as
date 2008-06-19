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

rcsid="$Id: Matrix.as,v 1.11 2008/06/19 11:49:17 bwy Exp $";


// There are lots of floating point calculations here. Comparing them
// as a string means they are checked to the 15th significant digit, which
// isn't always unreasonable. I've handled the ones that failed for me with
// an epsilon; others may fail for other people - so please edit as necessary
// - but it's good to be as accurate as possible.
//
// A ming bug up to version 0.4 beta 5 makes very large numbers fail.
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

// A non-matrix with a matrix's rotate method (fails?).
fakematrix = {a:1, b:1, c: 1, d: 1, tx:5, ty: 5};
fakematrix.rotate = Matrix.prototype.rotate;
fakematrix.rotate(2);
check_equals(fakematrix.a.toString(), 1);
check_equals(fakematrix.b.toString(), 1);
check_equals(fakematrix.c.toString(), 1);
check_equals(fakematrix.d.toString(), 1);
check_equals(fakematrix.tx.toString(), 5);
check_equals(fakematrix.ty.toString(), 5);

m.scale(-343, 0.33874983);
check_equals (m.toString(), "(a=-1406.99906373029, b=-1.23693717839177, c=514.393845024734, d=-1.49365801000499, tx=0, ty=0)");

m.translate(333,-283747.22);
check_equals (m.toString(), "(a=-1406.99906373029, b=-1.23693717839177, c=514.393845024734, d=-1.49365801000499, tx=333, ty=-283747.22)");

// A non-matrix with a matrix's translate method (works).
fakematrix = {a:1, b:1, c: 1, d: 1, tx:5, ty: 5};
fakematrix.translate = Matrix.prototype.translate;
fakematrix.translate(200,400);
check_equals(fakematrix.tx.toString(), 205);
check_equals(fakematrix.ty.toString(), 405);

m.scale(4798747e+98, 0.33874983);
// PP: (a=-6.75183253607854e+107, b=-0.419012258900892, c=2.46844592063091e+107, d=-0.505976396967328, tx=1.597982751e+107, ty=-96119.3225379726)
// I get one discrepancy in 'a' here.
#if MING_VERSION_CODE > 00040005
check(m.a < -6.7518325360784e+107 && m.a > -6.7518325360786e+107)
check_equals(m.c.toString(), "2.46844592063091e+107");
check_equals(m.tx.toString(), "1.597982751e+107");
#endif
check_equals(m.b.toString(), "-0.419012258900892");
check_equals(m.d.toString(), "-0.505976396967328");
check_equals(m.ty.toString(), "-96119.3225379726");

m.rotate(-1.2873874);
// PP: (a=-1.888016310255e+107, b=6.48248694618508e+107, c=6.9025203664787e+106, d=-2.36997413255563e+107, tx=4.46844242844096e+106, ty=-1.53423567131344e+107)
// tx is slightly different for me.
#if MING_VERSION_CODE > 00040005
check_equals(m.a.toString(), "-1.888016310255e+107");
check_equals(m.b.toString(), "6.48248694618508e+107");
check_equals(m.c.toString(), "6.9025203664787e+106");
check_equals(m.d.toString(), "-2.36997413255563e+107");
check(m.tx < 4.46844242844097e+106 && m.tx > 4.46844242844095e+106)
check_equals(m.ty.toString(), "-1.53423567131344e+107");
#endif

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

fakepoint = {x: 34, y: -23};
newP2 = m2.deltaTransformPoint(fakepoint);
check_equals(typeof(newP2), "object");
check(newP2 instanceof Point);
check_equals(newP.toString(), newP2.toString());




// Scale
m3 = new Matrix(2, 0, 0, 2, 100, 100);
m3.scale(3, 4);
check_equals(m3.toString(), "(a=6, b=0, c=0, d=8, tx=300, ty=400)");
// Do not change m3; it is used to test invert!

// A non-matrix with a matrix's invert method (fails?).
fakematrix = {a:3, b:2, c: 5, d: 3, tx:5, ty: 5};
fakematrix.scale = Matrix.prototype.scale;
fakematrix.scale(4, 5);
check_equals(fakematrix.a.toString(), "3");
check_equals(fakematrix.b.toString(), "2");
check_equals(fakematrix.c.toString(), "5");
check_equals(fakematrix.d.toString(), "3");
check_equals(fakematrix.tx.toString(), "5");
check_equals(fakematrix.ty.toString(), "5");
fakematrix.toString = Matrix.prototype.toString;
check_equals(fakematrix.toString(), "(a=3, b=2, c=5, d=3, tx=5, ty=5)");

// Test clone
m4 = m3.clone();
check_equals(m4.toString(), "(a=6, b=0, c=0, d=8, tx=300, ty=400)");
// Do not change m4; it's used later to test concat!

// A non-matrix with a matrix's invert method (works).
fakematrix = {a:3, b:2, c: 5, d: 3, tx:5, ty: 5};
fakematrix.clone = Matrix.prototype.clone;
r = fakematrix.clone();
check_equals(r.toString(), "(a=3, b=2, c=5, d=3, tx=5, ty=5)");
check(r instanceof Matrix);


// Test invert
m3.invert();
check_equals(m3.toString(), "(a=0.166666666666667, b=0, c=0, d=0.125, tx=-50, ty=-50)");

// Invalid inverse
m6 = new Matrix(4, 5, 44, 55, 2, 4);
check_equals(m6.toString(), "(a=4, b=5, c=44, d=55, tx=2, ty=4)");
m6.invert();
check_equals(m6.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

// A non-matrix with a matrix's invert method (half works).
fakematrix = {a:3, b:2, c: 5, d: 3, tx:5, ty: 5};
fakematrix.invert = Matrix.prototype.invert;
fakematrix.invert();
xcheck_equals(fakematrix.tx.toString(), "NaN");
check_equals(fakematrix.a.toString(), -3);

// Valid inverse2.
m6 = new Matrix(4, 5, 0, 5, 2, 3);
check_equals(m6.toString(), "(a=4, b=5, c=0, d=5, tx=2, ty=3)");
m6.invert();
check_equals(m6.toString(), "(a=0.25, b=-0.25, c=0, d=0.2, tx=-0.5, ty=-0.1)");
m6.rotate(-0.5);
check_equals(m6.toString(), "(a=0.0995392558215424, b=-0.339252025123644, c=0.0958851077208406, d=0.175516512378075, tx=-0.486733834805607, ty=0.151954513113064)");
m6.invert();
check_equals(m6.toString(), "(a=3.51033024756149, b=6.78504050247288, c=-1.91770215441681, d=1.99078511643085, tx=2, ty=3)");


// Matrix.transformPoint (and deltaTransformPoint again)
p = new Point(23, 95);
p2 = m6.transformPoint(p);
check_equals(p2.toString(), "(x=-99.4441089756828, y=348.180517617807)");
p3 = m6.deltaTransformPoint(p);
check_equals(p3.toString(), "(x=-101.444108975683, y=345.180517617807)");
p2 = m6.transformPoint(p2);
check_equals(p2.toString(), "(x=-1014.78819244077, y=21.420285172384)");

// Transforming points with a fake matrix.
fakematrix = {a:3, b:2, c: 5, d: 3, tx:5, ty: 5};
fakematrix.deltaTransformPoint = Matrix.prototype.deltaTransformPoint;
f = fakematrix.deltaTransformPoint(p2);
check (f instanceof Point);
check_equals (f.toString(), "(x=-2937.26315146039, y=-1965.31552936439)");

fakematrix.transformPoint = Matrix.prototype.transformPoint;
f = fakematrix.transformPoint(p2);
check (f instanceof Point);
check_equals (f.toString(), "(x=-2932.26315146039, y=-1960.31552936439)");


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

t = new Object();
check_equals(t.a, undefined)
t.identity = Matrix.prototype.identity;
t.identity();
check_equals(t.a.toString(), "1");
check_equals(t.b.toString(), "0");
check_equals(t.c.toString(), "0");
check_equals(t.d.toString(), "1");
check_equals(t.tx.toString(), "0");
check_equals(t.ty.toString(), "0");

// m4 is still interesting
m4.concat(m1);
check_equals(m4.toString(), "(a=192, b=-40.8, c=768, d=-108.8, tx=48160, ty=-7500.4)");
m4.concat(m2);
// Works for me.
check_equals(m4.toString(), "(a=90.6729490609422, b=174.089219392218, c=310.274230957074, d=710.908813846049, tx=20109.8154004632, ty=44398.6139954762)");

// A non-matrix with a matrix's concat method (works).
fakematrix = {a:1, b:1, c: 1, d: 1, tx:5, ty: 5};
fakematrix.concat = Matrix.prototype.concat;
fakematrix.concat(new Matrix(4, 4, 4, 4, 4, 4));
check_equals(fakematrix.tx.toString(), 44);
check_equals(fakematrix.a.toString(), 8);

m7 = new Matrix ("A string", undefined, new Object, true, NaN, new Point);
check_equals("" + m7, "(a=A string, b=undefined, c=[object Object], d=true, tx=NaN, ty=(x=0, y=0))");

m7.rotate(2);
check_equals(m7.toString(), "(a=NaN, b=NaN, c=NaN, d=NaN, tx=NaN, ty=NaN)");

m8 = new Matrix(5, 4, 5, 3, 2, 1);
check_equals(m8.toString(), "(a=5, b=4, c=5, d=3, tx=2, ty=1)");
m8.createBox(4, 3, 4, 2, 3);
check_equals(m8.toString(), "(a=-2.61457448345445, b=-2.27040748592378, c=3.02720998123171, d=-1.96093086259084, tx=2, ty=3)");
m8.createBox(45, 444, -1.3874987, -47, -2999398);
check_equals(m8.toString(), "(a=8.2022824555003, b=-436.562099487155, c=44.2461587318062, d=80.9291868942697, tx=-47, ty=-2999398)");
m8.createBox(4, 3, new Object(), "a string");
check_equals(m8.toString(), "(a=NaN, b=NaN, c=NaN, d=NaN, tx=a string, ty=0)");
m8.createBox("a", "b");
check_equals(m8.toString(), "(a=NaN, b=NaN, c=NaN, d=NaN, tx=0, ty=0)");

// A non-matrix with a matrix's createBox method (half works).
delete fakematrix;
fakematrix = new Object();
fakematrix.createBox = Matrix.prototype.createBox;
fakematrix.createBox(4, 3, 4, 2, 3);
xcheck_equals(fakematrix.a.toString(), undefined);
xcheck_equals(fakematrix.b.toString(), undefined);
xcheck_equals(fakematrix.c.toString(), undefined);
xcheck_equals(fakematrix.d.toString(), undefined);
check_equals(fakematrix.tx.toString(), "2");
check_equals(fakematrix.ty.toString(), "3");


m8.createGradientBox(20, 30, 2 * Math.PI, 10, 25);

// The very small numbers aren't very 'accurate', of course.
check_equals(m8.a.toString(), "0.01220703125");
check_equals(m8.d.toString(), "0.018310546875");
check_equals(m8.tx.toString(), "20");
check_equals(m8.ty.toString(), "40");
check(Math.abs(m8.b) < 0.0000000000001);
check(Math.abs(m8.c) < 0.0000000000001);

m8.createGradientBox(40, 49, 0, "string", undefined);
// Half of the width is added to the translation - they take that quite literally...
check_equals(m8.toString(), "(a=0.0244140625, b=0, c=0, d=0.0299072265625, tx=string20, ty=NaN)");
m8.createGradientBox(5, 6, 0, 1, 1);
check_equals(m8.toString(), "(a=0.0030517578125, b=0, c=0, d=0.003662109375, tx=3.5, ty=4)");
m8.createGradientBox(5, 6, 2, 1, 1);
check_equals(m8.toString(), "(a=-0.0012699793595799, b=0.00332994663144171, c=-0.00277495552620142, d=-0.00152397523149588, tx=3.5, ty=4)");

// A non-matrix with a matrix's createGradientBox method (fails).
delete fakematrix;
fakematrix = new Object();
fakematrix.createGradientBox = Matrix.prototype.createGradientBox;
fakematrix.createGradientBox(20, 30, 2 * Math.PI, 10, 25);
check_equals(fakematrix.a.toString(), undefined);
check_equals(fakematrix.b.toString(), undefined);
check_equals(fakematrix.c.toString(), undefined);
check_equals(fakematrix.d.toString(), undefined);
check_equals(fakematrix.tx.toString(), undefined);
check_equals(fakematrix.ty.toString(), undefined);


//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------
#if MING_VERSION_CODE > 00040005
totals(162);
#else
totals(153);
#endif

#endif // OUTPUT_VERSION >= 8
