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
// Test case for Transform ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Transform.as,v 1.3 2008/06/20 13:28:56 bwy Exp $";

#include "check.as"

#if OUTPUT_VERSION < 8
check_equals(flash.geom.Transform, undefined);
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
xcheck_equals(t, undefined);

t = Transform();
xcheck_equals(t, undefined);

// Hooray!
t = new Transform(_root);
xcheck_equals(typeOf(t), "object");
xcheck(t instanceOf Transform);

t = _root.transform;
xcheck_equals(typeOf(t), "object");
xcheck(t instanceOf Transform);

xcheck(t.matrix instanceOf Matrix);
xcheck(t.concatenatedMatrix instanceOf Matrix);
xcheck(t.colorTransform instanceOf ColorTransform);
xcheck(t.concatenatedColorTransform instanceOf ColorTransform);
xcheck(t.pixelBounds instanceOf Rectangle);

xcheck_equals(t.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");
xcheck_equals(t.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
xcheck_equals(t.concatenatedColorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
// These vary slightly with the pp
//check_equals(t.concatenatedMatrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");
//check_equals(t.pixelBounds.toString(), "(x=-2, y=48, w=804, h=804)");


// Tricks with the flash package:
flash.geom.Matrix = undefined;
check_equals(t.matrix, undefined);
xcheck_equals(t.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
flash.geom.ColorTransform = undefined;
check_equals(t.colorTransform, undefined);
xcheck(t.pixelBounds instanceOf Rectangle);
flash.geom.Rectangle = undefined;
check_equals(t.pixelBounds, undefined);


flash.geom.Matrix = Matrix;
xcheck_equals(t.matrix.toString(), "(a=1, b=0, c=0, d=1, tx=0, ty=0)");
check_equals(t.colorTransform, undefined);
flash.geom.ColorTransform = ColorTransform;
xcheck_equals(t.colorTransform.toString(), "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)");
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

trans.matrix = new Matrix(2, 0, 0, 2, 10, 11);

check_equals(trans.matrix.toString(), "(a=2, b=0, c=0, d=2, tx=10, ty=11)");
check_equals(mc2.transform.matrix.toString(), "(a=2, b=0, c=0, d=2, tx=10, ty=11)");

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


totals(56);
#endif
