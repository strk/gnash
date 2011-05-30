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
// Test case for Transform ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="Transform.as";

#include "check.as"

ASSetPropFlags (_global, "flash", 0, 5248);

#if OUTPUT_VERSION < 8
# if OUTPUT_VERSION < 6 
check_equals(typeOf(flash.geom.Transform), "undefined");
# else
check_equals(typeOf(flash.geom.Transform), "function");
# endif
totals(1);
#else

Transform = flash.geom.Transform;
Matrix = flash.geom.Matrix;
ColorTransform = flash.geom.ColorTransform;
Rectangle = flash.geom.Rectangle;

check(Transform.prototype.hasOwnProperty("matrix"));
check(Transform.prototype.hasOwnProperty("concatenatedMatrix"));
check(Transform.prototype.hasOwnProperty("colorTransform"));
check(Transform.prototype.hasOwnProperty("concatenatedColorTransform"));
check(Transform.prototype.hasOwnProperty("pixelBounds"));

// Cannot be instantiated without MovieClip argument.
t = new Transform;
check_equals(t, undefined);

t = Transform();
check_equals(t, undefined);

// Hooray!
t = new Transform(_root);
check_equals(typeOf(t), "object");
check(t instanceOf Transform);

t = _root.transform;
check_equals(typeOf(t), "object");
check(t instanceOf Transform);

check(t.matrix instanceOf Matrix);
check(t.concatenatedMatrix instanceOf Matrix);
check(t.colorTransform instanceOf ColorTransform);
check(t.concatenatedColorTransform instanceOf ColorTransform);
xcheck(t.pixelBounds instanceOf Rectangle);

check_equals(t.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");
check_equals(t.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
check_equals(t.concatenatedColorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
// These vary slightly with the pp
//check_equals(t.concatenatedMatrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");
//check_equals(t.pixelBounds.toString(), "(x=-2, y=48, w=804, h=804)");

/// transform.colorTransform

mcx = _root.createEmptyMovieClip("mcx", getNextHighestDepth());
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

mcx._alpha = 23;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=0.2265625, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

mcx._alpha = -203;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=-2.02734375, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

mcx._alpha = -99;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=-0.98828125, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

mcx._alpha = 13000;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=-128, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");

r = new ColorTransform(2, 3, 4, 5, 5, -5, 5, -5);
mcx.transform.colorTransform = r;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=2, greenMultiplier=3, blueMultiplier=4, alphaMultiplier=5, redOffset=5, greenOffset=-5, blueOffset=5, alphaOffset=-5)");

r = new ColorTransform(0, 1, 1, 1, 0, 0, 255, 0);
mcx.transform.colorTransform = r;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=0, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=255, alphaOffset=0)");

r = new ColorTransform(5e-67, 2342341, 11234112, -287394874978, 1222222, 2342343434, 255, 4e+5);
mcx.transform.colorTransform = r;
check_equals(mcx.transform.colorTransform.toString(), "(redMultiplier=0, greenMultiplier=-128, blueMultiplier=-128, alphaMultiplier=-128, redOffset=-32768, greenOffset=-32768, blueOffset=255, alphaOffset=-32768)");


// Tricks with the flash package:
flash.geom.Matrix = undefined;
check_equals(t.matrix, undefined);
check_equals(t.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
flash.geom.ColorTransform = undefined;
check_equals(t.colorTransform, undefined);
xcheck(t.pixelBounds instanceOf Rectangle);
flash.geom.Rectangle = undefined;
check_equals(t.pixelBounds, undefined);


flash.geom.Matrix = Matrix;
check_equals(t.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");
check_equals(t.colorTransform, undefined);
flash.geom.ColorTransform = ColorTransform;
check_equals(t.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
flash.geom.Rectangle = Rectangle;
xcheck(t.pixelBounds instanceOf Rectangle);

mc = _root.createEmptyMovieClip("mc", getNextHighestDepth());
check(mc.transform instanceOf Transform);
check(mc.transform.matrix instanceOf Matrix);
check_equals(mc.transform.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

// Store Matrix (copy)
mat = mc.transform.matrix;

// Store Transform (reference)
trans = mc.transform;

mc._x = "4";
check_equals(mc.transform.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=4, ty=0)");
check_equals(trans.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=4, ty=0)");
check_equals(mat.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");


mc._rotation = 1.5;
check_equals(mc.transform.matrix.toString(), trans.matrix.toString());
check_equals(Math.round(trans.matrix.b * 10000), 262);
check_equals(mat.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

mc2 = mc;
delete mc;

check_equals(mc.transform.matrix.toString(), trans.matrix.toString());
check_equals(Math.round(trans.matrix.a * 1000), 1000);
check_equals(mat.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");


_root.removeMovieClip(mc);
check_equals(Math.round(mc.transform.matrix.b * 10000), 262);

mc = undefined;

check_equals(Math.round(mc2.transform.matrix.b * 10000), 262);
check_equals(mc.transform.matrix.toString(), undefined);
check_equals(mc2.transform.matrix.toString(), trans.matrix.toString());
check_equals(mat.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

check_equals(mc2._xscale, 100);
check_equals(mc2._yscale, 100);
check_equals(mc2._rotation, 1.5);
trans.matrix = new Matrix(2, 0, 0, 2, 10, 11);

check_equals(trans.matrix.toString(), "(a=2, b=0, c=0, d=2, tx=10, ty=11)");
check_equals(mc2.transform.matrix.toString(), "(a=2, b=0, c=0, d=2, tx=10, ty=11)");

check_equals(mc2._xscale, 100);
check_equals(mc2._yscale, 100);
check_equals(mc2._rotation, 1.5);

delete mc2;

check_equals(mc2.transform.matrix.toString(), undefined);
check_equals(trans.matrix.toString(), "(a=2, b=0, c=0, d=2, tx=10, ty=11)");
check_equals(mat.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

mc2 = undefined;

check_equals(trans.matrix.toString(), "(a=2, b=0, c=0, d=2, tx=10, ty=11)");

// Identity;
trans.matrix = new Matrix;
check_equals(trans.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");


mc = _root.createEmptyMovieClip("mc", getNextHighestDepth());
trans = mc.transform;

mcOld = mc;
trans.matrix = new Matrix(3, 0.5, 0.5, 2, 0, 1);
check_equals(mc.transform.matrix.toString(), "(a=3, b=0.5, c=0.5, d=2, tx=0, ty=1)");
check_equals(mcOld.transform.matrix.toString(), "(a=3, b=0.5, c=0.5, d=2, tx=0, ty=1)");


mcOld = mc;
mc = _root.createEmptyMovieClip("mc", getNextHighestDepth());

trans.matrix = new Matrix(4, 0.3, 0.3, 1, 1, 0);
check_equals(mc.transform.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");

// Can we be this accurate? I think the AS matrix class is accurate enough.
xcheck_equals(mcOld.transform.matrix.toString(), "(a=4, b=0.300000011920929, c=0.300000011920929, d=1, tx=1, ty=0)");


// Concatenated transform. Does this make any sense?

// The following group of tests was updated on 2010-08-10 to match
// FP 9,0,0,115 results (Linux)

near = function(a, b) {
    return Math.abs(a - b) < 0.01;
};

nearly_equal = function(m, a, b, c, d, tx, ty) {
    return near(m.a, a) && near(m.b, b) &&
           near(m.c, c) && near(m.d, d) &&
           near(m.tx, tx) && near(m.ty, ty);
};

conc1 = _root.createEmptyMovieClip("conc1", getNextHighestDepth());
conc2 = conc1.createEmptyMovieClip("conc2", getNextHighestDepth());

conc1._x = 40;
conc2._x = 24;

xcheck(nearly_equal(conc1.transform.concatenatedMatrix, 1, 0, 0, 1, 39.75, 2));
xcheck(nearly_equal(conc2.transform.concatenatedMatrix, 1, 0, 0, 1, 63.5, 2));
conc2._width = 3;
conc1._height = 0.6;
xcheck(nearly_equal(conc1.transform.concatenatedMatrix, 0, 0, 0, 0, 39.75, 2));
xcheck(nearly_equal(conc1.transform.concatenatedMatrix, 0, 0, 0, 0, 39.75, 2));

d = _root.createEmptyMovieClip("tar", 600);
d.beginFill(0x00ff00);
d.moveTo(20, 20);
d.lineTo(20, 80);
d.lineTo(80, 80);
d.lineTo(80, 40);
d.lineTo(20, 20);

d.moveTo(50, 50);
d.beginFill(0xff0000);
d.lineTo(60, 50);
d.lineTo(60, 60);
d.lineTo(50, 60);
d.lineTo(50, 50);

ct8 = d.transform.colorTransform;

check_equals(ct8.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)")

c = new Color(tar);
c.setRGB(0xff0000);

btx = c.getTransform();
check_equals(btx.ra, 0);
check_equals(btx.ba, 0);
check_equals(btx.ga, 0);
check_equals(btx.aa, 100);
check_equals(btx.rb, 255);
check_equals(btx.gb, 0);
check_equals(btx.bb, 0);
check_equals(btx.ab, 0);

ct9 = d.transform.colorTransform;
check_equals(ct9.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=1, redOffset=255, greenOffset=0, blueOffset=0, alphaOffset=0)")

c.setRGB(0x0000ff);
ct10 = d.transform.colorTransform;
check_equals(ct10.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=255, alphaOffset=0)")

tr = d.transform;
tr.colorTransform = new ColorTransform(0, 0, 0, 1, 0, 127, 0, 0);
d.transform = tr;
ct11 = d.transform.colorTransform;
check_equals(ct11.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=1, redOffset=0, greenOffset=127, blueOffset=0, alphaOffset=0)");

// User transform is
btx = c.getTransform();
check_equals(btx.ra, 0);
check_equals(btx.ba, 0);
check_equals(btx.ga, 0);
check_equals(btx.aa, 100);
check_equals(btx.rb, 0);
check_equals(btx.gb, 127);
check_equals(btx.bb, 0);
check_equals(btx.ab, 0);

// A fake color transform doesn't work.
ct = {};
ct.redMultiplier = 1;
ct.blueMultiplier = 1;
ct.greenMultiplier = 1;
ct.alphaMultiplier = 1;
ct.redOffset = 255;
ct.blueOffet = 0;
ct.greenOffset = 255;
ct.alphaOffset = 0;

tr = d.transform;
tr.colorTransform = ct;
d.transform = tr;

ctfake = d.transform.colorTransform;
check_equals(ct11.toString(), "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=1, redOffset=0, greenOffset=127, blueOffset=0, alphaOffset=0)");

totals(95);
#endif
