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
// Test case for BitmapData ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: BitmapData.as,v 1.3 2008/06/20 13:28:56 bwy Exp $";


#include "check.as"

#if OUTPUT_VERSION < 8

check_equals(typeof(flash), 'undefined');

check_totals(1);

#else

Bitmap = flash.display.BitmapData;
check_equals(typeof(Bitmap), 'function');
check_equals(typeof(Bitmap.prototype), 'object');
check(Bitmap.prototype.hasOwnProperty('applyFilter'));
check(Bitmap.prototype.hasOwnProperty('clone'));
check(Bitmap.prototype.hasOwnProperty('colorTransform'));
check(Bitmap.prototype.hasOwnProperty('copyChannel'));
check(Bitmap.prototype.hasOwnProperty('copyPixels'));
check(Bitmap.prototype.hasOwnProperty('dispose'));
check(Bitmap.prototype.hasOwnProperty('draw'));
check(Bitmap.prototype.hasOwnProperty('fillRect'));
check(Bitmap.prototype.hasOwnProperty('floodFill'));
check(Bitmap.prototype.hasOwnProperty('generateFilterRect'));
check(Bitmap.prototype.hasOwnProperty('getColorBoundsRect'));
check(Bitmap.prototype.hasOwnProperty('getPixel'));
check(Bitmap.prototype.hasOwnProperty('getPixel32'));
check(Bitmap.prototype.hasOwnProperty('hitTest'));
check(Bitmap.prototype.hasOwnProperty('merge'));
check(Bitmap.prototype.hasOwnProperty('noise'));
check(Bitmap.prototype.hasOwnProperty('paletteMap'));
check(Bitmap.prototype.hasOwnProperty('perlinNoise'));
check(Bitmap.prototype.hasOwnProperty('pixelDissolve'));
check(Bitmap.prototype.hasOwnProperty('scroll'));
check(Bitmap.prototype.hasOwnProperty('setPixel'));
check(Bitmap.prototype.hasOwnProperty('setPixel32'));
check(Bitmap.prototype.hasOwnProperty('threshold'));
check(Bitmap.prototype.hasOwnProperty("height"));
check(Bitmap.prototype.hasOwnProperty("width"));
check(Bitmap.prototype.hasOwnProperty("rectangle"));
check(Bitmap.prototype.hasOwnProperty("transparent"));

check(Bitmap.hasOwnProperty("RED_CHANNEL"));
check(Bitmap.hasOwnProperty("GREEN_CHANNEL"));
check(Bitmap.hasOwnProperty("BLUE_CHANNEL"));
check(Bitmap.hasOwnProperty("ALPHA_CHANNEL"));

check(!Bitmap.prototype.hasOwnProperty('loadBitmap'));
check(Bitmap.hasOwnProperty('loadBitmap'));

Rectangle = flash.geom.Rectangle;

//-------------------------------------------------------------
// Test constructor
//-------------------------------------------------------------

bmp = new Bitmap();
check_equals(typeof(bmp), "undefined");

bmp = new Bitmap(10, 10);
check_equals(typeof(bmp), 'object');
check(bmp instanceof Bitmap);
check(!bmp.hasOwnProperty("height"));
check(!bmp.hasOwnProperty("width"));
check(!bmp.hasOwnProperty("rectangle"));
check(!bmp.hasOwnProperty("transparent"));
check_equals(bmp.height, 10);
check_equals(bmp.width, 10);
check_equals(bmp.transparent, true);
check_equals(bmp.rectangle.toString(), "(x=0, y=0, w=10, h=10)");
check(bmp.rectangle instanceOf flash.geom.Rectangle);
check_equals(bmp.getPixel(1, 1), 16777215);
check_equals(bmp.getPixel(9, 9), 16777215);
check_equals(bmp.getPixel32(1, 1), -1);

bmp = new Bitmap(10, 10, true);
check_equals(bmp.getPixel32(1, 1), -1);
bmp = new Bitmap(10, 10, false);
check_equals(bmp.getPixel32(1, 1), -1);


bmp = new Bitmap(20, 30, false, 0xeeddee);
check_equals(bmp.height, 30);
check_equals(bmp.width, 20);
check_equals(bmp.transparent, false);
check_equals(bmp.rectangle.toString(), "(x=0, y=0, w=20, h=30)");
check_equals(bmp.getPixel(1, 1), 0xeeddee);
check_equals(bmp.getPixel32(1, 1), -1122834);

// limits

check_equals(bmp.getPixel(50, 1), 0);
check_equals(bmp.getPixel(0, 0), 15654382);
check_equals(bmp.getPixel(-2, -5), 0);

// 0,0 is inside, 20, 30 outside a 20x30 bitmap.
check_equals(bmp.getPixel(20, 30), 0);


// 2880 is the maximum, 1 the minimum. Returns
// undefined if the dimensions are invalid.
bmp = new Bitmap(10000, 3);
check_equals(typeof(bmp), "undefined");
check_equals(bmp.height, undefined);

bmp = new Bitmap(0, 10000);
check_equals(bmp, undefined);
check_equals(bmp.height, undefined);

bmp = new Bitmap(2880, 2880);
check_equals(typeof(bmp), "object");
check_equals(bmp.height, 2880);

bmp = new Bitmap(2880, 2881);
check_equals(typeof(bmp), "undefined");
check_equals(bmp.height, undefined);

bmp = new Bitmap(0, 2880);
check_equals(bmp, undefined);
check_equals(bmp.height, undefined);

bmp = new Bitmap(2879, 2879);
check_equals(typeof(bmp), "object");
check_equals(bmp.height, 2879);

bmp = new Bitmap(0, 2879);
check_equals(bmp, undefined);
check_equals(bmp.height, undefined);

bmp = new Bitmap(-1, 10, false, 0xff);
check_equals(bmp, undefined);
check_equals(bmp.height, undefined);

// --------------------
// setPixel, setPixel32
// --------------------

tr = new Bitmap(30, 30, true);
ntr = new Bitmap(30, 30, false);

// Premultiplication?
tr.setPixel32(2, 2, 0x44);
xcheck_equals(tr.getPixel(2, 2), 0x00);
xcheck_equals(tr.getPixel32(2, 2), 0);

// Premultiplication?
tr.setPixel32(2, 2, 0x220000aa);
xcheck_equals(tr.getPixel(2, 2), 0xac);
xcheck_equals(tr.getPixel32(2, 2), 0x220000ac);

tr.setPixel32(2, 2, 0xff0000aa);
check_equals(tr.getPixel(2, 2), 0xaa);
check_equals(tr.getPixel32(2, 2), -16777046);

tr.setPixel(3, 3, 0xff);
check_equals(tr.getPixel(3, 3), 0xff);
check_equals(tr.getPixel32(3, 3), -16776961);

// Premultiplication?
tr.setPixel32(4, 4, 0x44444444);
xcheck_equals(tr.getPixel(4, 4), 0x434343);
xcheck_equals(tr.getPixel32(4, 4), 0x44434343);

tr.setPixel32(4, 4, 0x10101010);
check_equals(tr.getPixel(4, 4), 0x101010);
check_equals(tr.getPixel32(4, 4), 0x10101010);

// Premultiplication?
tr.setPixel32(4, 4, 0x43434343);
xcheck_equals(tr.getPixel(4, 4), 0x444444);
xcheck_equals(tr.getPixel32(4, 4), 0x43444444);

ntr.setPixel(5, 5, 0xff);
check_equals(ntr.getPixel(5, 5), 0xff);
check_equals(ntr.getPixel32(5, 5), -16776961);

ntr.setPixel32(6, 6, 0x44444444);
check_equals(ntr.getPixel(6, 6), 0x444444);
check_equals(ntr.getPixel32(6, 6), -12303292);

// ---------
// floodFill
// ---------

bmp = new Bitmap(20, 20, false);
bmp.floodFill(10, 10, 0x0000ff00);

check_equals(bmp.getPixel(10, 10), 0x0000ff00);
bmp.floodFill(5, 5, 0x000000ff);
check_equals(bmp.getPixel(10, 0), 0x000000ff);

mc = this.createEmptyMovieClip("mc", this.getNextHighestDepth());
mc.attachBitmap(bmp, this.getNextHighestDepth());

b = new Bitmap(200, 200, false, 0xffffff);
b.fillRect(new Rectangle(10, 10, 10, 10), 0x00ff00);
b.fillRect(new Rectangle(50, 20, 10, 10), 0x00ff00);
b.fillRect(new Rectangle(50, 70, 20, 20), 0x00ff00);
b.fillRect(new Rectangle(50, 70, 20, 20), 0x00ff00);

b.fillRect(new Rectangle(120, 100, 10, 10), 0x0000ff);
b.fillRect(new Rectangle(130, 90, 10, 10), 0xffff00);
b.fillRect(new Rectangle(140, 100, 10, 10), 0x00ffff);
b.fillRect(new Rectangle(130, 110, 10, 10), 0xff00ff);

check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(135, 105), 0xffffff);

// This is done twice deliberately to make sure Gnash doesn't hang!
b.floodFill(0, 0, 0);
b.floodFill(0, 0, 0);
check_equals(b.getPixel(1, 1), 0x0);
check_equals(b.getPixel(190, 190), 0x0);
check_equals(b.getPixel(135, 105), 0xffffff);

b.floodFill(135, 105, 0xee1111);
check_equals(b.getPixel(1, 1), 0x0);
check_equals(b.getPixel(190, 190), 0x0);
check_equals(b.getPixel(135, 105), 0xee1111);

mc2 = this.createEmptyMovieClip("mc2", this.getNextHighestDepth());
mc2.attachBitmap(b, this.getNextHighestDepth());
mc2._x = 300;
mc2._y = 300;

// fillRect

bmp = new Bitmap(20, 20, false);
r = new Rectangle(2, 2, 5, 5);
bmp.fillRect(r, 0xff1100);
check_equals(bmp.getPixel(1, 1), 0xffffff);
check_equals(bmp.getPixel(2, 2), 0xff1100);
check_equals(bmp.getPixel(2, 5), 0xff1100);
check_equals(bmp.getPixel(5, 2), 0xff1100);
check_equals(bmp.getPixel(2, 6), 0xff1100);
check_equals(bmp.getPixel(6, 6), 0xff1100);
check_equals(bmp.getPixel(6, 7), 0xffffff);
check_equals(bmp.getPixel(7, 6), 0xffffff);

r = new Rectangle(-2, -2, 8, 8);
bmp.fillRect(r, 0x00ff00);
check_equals(bmp.getPixel(1, 1), 0x00ff00);

// Fails.
r = new Rectangle(18, 18, -4, -4);
bmp.fillRect(r, 0x0000ff);
check_equals(bmp.getPixel(7, 6), 0xffffff);

r = new Rectangle(18, 18, 200, 200);
bmp.fillRect(r, 0x0000ff);
check_equals(bmp.getPixel(19,19), 0x0000ff);

// Doesn't have to be a rectangle
g = {x: 15, y: 15, width: 2, height: 2};
bmp.fillRect(g, 0xff00ff);
check_equals(bmp.getPixel(16, 16), 0xff00ff);

// Transparency (this bitmap is not transparent).
g = {x: 18, y: 2, width: 7, height: 7};
bmp.fillRect(g, 0xddff00ff);
check_equals(bmp.getPixel32(18, 2), -65281);

mc.attachBitmap(bmp, this.getNextHighestDepth());

// Transparency (transparent bitmap). Fill just obliterates
// what was there, even if it's transparent.
bmp = new Bitmap(20, 20, true);
r = new Rectangle(1, 1, 10, 10);
bmp.fillRect(r, 0xff00ff00);
r = new Rectangle(2, 2, 9, 9);
bmp.fillRect(r, 0x99ff1100);
check_equals(bmp.getPixel32(3, 3), -1711337216);

mc.attachBitmap(bmp, this.getNextHighestDepth());

bmp.dispose();
check_equals(bmp.height, -1);
check_equals(bmp.width, -1);
check_equals(bmp.transparent, -1);
check_equals(typeof(bmp.rectangle), "number");
check_equals(bmp.rectangle, -1);
check_equals(bmp.rectangle.toString(), "-1");

check(bmp instanceOf Bitmap);
bmp.height = 2;
check_equals(bmp.height, -1);

bmp = new Bitmap(20, 10, true);
backup = flash.geom.Rectangle;
flash.geom.Rectangle = 2;
check_equals(bmp.rectangle, -1);

flash.geom.Rectangle = function (x, y, w, h)
{
    this.y = x + 5;
    this.x = 10.5;
    this.width = h;
    this.height = w;
};
check_equals(bmp.rectangle.toString(), "[object Object]");

flash.geom.Rectangle = function (x, y, w, h)
{
};
check_equals(bmp.rectangle.toString(), "[object Object]");

flash.geom.Rectangle = function ()
{
};
check_equals(bmp.rectangle.toString(), "[object Object]");

flash.geom.Rectangle = backup;
check_equals(bmp.rectangle.toString(), "(x=0, y=0, w=20, h=10)");

////////////////////
// BitmapData.draw
////////////////////

// First we check that all user-defined transformations are ignored.

// Note: at the corners of these tests antialiasing makes a difference. The
// value can vary according to the transformation of the clip. We're not
// really interested in the exact values of anti-aliased pixels.

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


// 100x100, no transparency
b = new Bitmap(100, 100, false);
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// Hard ref
b.draw(_level0.tar);
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// User-defined translation makes no difference.
d._x = 500;
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// User defined transform makes no difference.
d._height = 30;
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// User defined transform makes no difference.
d._width = 30;
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// Color transform the old way (no difference).
c = new Color("_level0.tar");  
c.setRGB(0xff5500);
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// Color transform the new way.
var tr = d.transform;
tr.colorTransform = new flash.geom.ColorTransform(0.5, 0.5, 0.5, 0.5, 34, 34, 34, 34);
d.transform = tr;
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

dom = new flash.geom.Matrix();
dom.rotate(Math.PI / 4);
tr.matrix = dom;
d.transform = tr;
check_equals(b.getPixel(1, 1), 0xffffff);
check_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
check_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
check_equals(b.getPixel(55, 55), 0xff0000);

// Test behaviour of BitmapData.draw with masks.

// 1. The MovieClip is drawn with the custom transform
// 2. The mask is drawn with its current transform
near = function(bitmap, x, y, val) {
   tol = 2;
   col = bitmap.getPixel(x, y);
   col_r = (col & 0xff0000) >> 16;
   col_g = (col & 0xff00) >> 8;
   col_b = (col & 0xff);
   val_r = (val & 0xff0000) >> 16;
   val_g = (val & 0xff00) >> 8;
   val_b = (val & 0xff);
   if (Math.abs(col_r - val_r) > tol) return false;
   if (Math.abs(col_b - val_b) > tol) return false;
   if (Math.abs(col_g - val_g) > tol) return false;
   return true;
};

mc = _root.createEmptyMovieClip("mc", 1009);
a = mc.createEmptyMovieClip("a", 1090);
b = mc.createEmptyMovieClip("b", 1091);

mask = _root.createEmptyMovieClip("mask", 1150);

with(a) {
    beginFill(0xff0000);
    moveTo(0, 0);
    lineTo(10, 0);
    lineTo(10, 40);
    lineTo(0, 40);
    lineTo(0, 0);

    beginFill(0x00ff00);
    moveTo(10, 0);
    lineTo(20, 0);
    lineTo(20, 40);
    lineTo(10, 40);
    lineTo(10, 0);

    beginFill(0x0000ff);
    moveTo(20, 0);
    lineTo(30, 0);
    lineTo(30, 40);
    lineTo(20, 40);
    lineTo(20, 0);

};

with(mask) {
    beginFill(0x000000);
    moveTo(10, 10);
    lineTo(20, 10);
    lineTo(20, 20);
    lineTo(10, 20);
    lineTo(10, 10);
};

// Only for visual checking.
disp = _root.createEmptyMovieClip("disp", 1300);
disp._x = 200;
disp._y = 200;

mc.setMask(mask);

// Mask and MovieClip with neutral transform
bm = new flash.display.BitmapData(50, 50, false);
bm.draw(mc);

// A square of the green stripe is visible.
check(near(bm, 5, 5, 0xffffff));
check(near(bm, 5, 15, 0xffffff));
check(near(bm, 5, 25, 0xffffff));
check(near(bm, 15, 5, 0xffffff));
check(near(bm, 15, 15, 0x00ff00));
check(near(bm, 15, 25, 0xffffff));
check(near(bm, 25, 5, 0xffffff));
check(near(bm, 25, 15, 0xffffff));
check(near(bm, 25, 25, 0xffffff));

// Mask with neutral transform, MovieClip with different transform
mc._width = 30;
mc._height = 200;
mc._x = -39;
bm = new flash.display.BitmapData(50, 50, false);
bm.draw(mc);

// A square of the green stripe is visible.
check(near(bm, 5, 5, 0xffffff));
check(near(bm, 5, 15, 0xffffff));
check(near(bm, 5, 25, 0xffffff));
check(near(bm, 15, 5, 0xffffff));
xcheck(near(bm, 15, 15, 0x00ff00));
check(near(bm, 15, 25, 0xffffff));
check(near(bm, 25, 5, 0xffffff));
check(near(bm, 25, 15, 0xffffff));
check(near(bm, 25, 25, 0xffffff));

disp.attachBitmap(bm, 400);

// Mask with different transform, MovieClip with different transform
mask._x = 10;
bm = new flash.display.BitmapData(50, 50, false);
bm.draw(mc);

// A square of the blue stripe is visible.
check(near(bm, 5, 5, 0xffffff));
check(near(bm, 5, 15, 0xffffff));
check(near(bm, 5, 25, 0xffffff));
check(near(bm, 15, 5, 0xffffff));
check(near(bm, 15, 15, 0xffffff));
check(near(bm, 15, 25, 0xffffff));
check(near(bm, 25, 5, 0xffffff));
xcheck(near(bm, 25, 15, 0x0000ff));
check(near(bm, 25, 25, 0xffffff));

bm = new flash.display.BitmapData(50, 50, false);
bm.draw(mc, new flash.geom.Matrix(1, 0, 0, 1, 5, 5));

// A bit of the blue and green blue stripe is visible.
check(near(bm, 5, 5, 0xffffff));
check(near(bm, 5, 15, 0xffffff));
check(near(bm, 5, 25, 0xffffff));
check(near(bm, 15, 5, 0xffffff));
check(near(bm, 15, 15, 0xffffff));
check(near(bm, 15, 25, 0xffffff));
check(near(bm, 25, 5, 0xffffff));
xcheck(near(bm, 23, 15, 0x00ff00));
xcheck(near(bm, 25, 15, 0x0000ff));
check(near(bm, 25, 25, 0xffffff));

bm = new flash.display.BitmapData(10, 10, true, 0x5010eeff);
xcheck_equals(bm.getPixel32(5, 5), 0x5010efff);

bm = new flash.display.BitmapData(10, 10, true, 0xee11efff);
check_equals(bm.getPixel32(5, 5), -300814337);

bm = new flash.display.BitmapData(10, 10, true, 0x00011010);
check_equals(bm.getPixel32(5, 5), 0x00000000);

bm = new flash.display.BitmapData(10, 10, true, 0x0000ffff);
check_equals(bm.getPixel32(5, 5), 0x00000000);

bm = new flash.display.BitmapData(10, 10, true, 0x000000ff);
check_equals(bm.getPixel32(5, 5), 0x00000000);

bm = new flash.display.BitmapData(10, 10, true, 0x010000ff);
check_equals(bm.getPixel32(5, 5), 0x010000ff);

bm = new flash.display.BitmapData(10, 10, true, 0x300000ff);
check_equals(bm.getPixel32(5, 5), 0x300000ff);

bm = new flash.display.BitmapData(10, 10, true, 0x30ffffff);
check_equals(bm.getPixel32(5, 5), 0x30ffffff);

// copyPixels()

source = new flash.display.BitmapData(100, 100, false, 0x0000ff);

// $

Rect = flash.geom.Rectangle;
Point = flash.geom.Point;

// These are identical:
// i.e. any part of the rect that is above or left of the source
// is added as an offset to the destination point.

dest = new flash.display.BitmapData(100, 100, false, 0xff0000);
dest.copyPixels(source, new Rect(-50, -50, 100, 100), new Point(0, 0));
 check_equals(dest.getPixel(10, 10), 0xff0000);
 check_equals(dest.getPixel(52, 52), 0x0000ff);
 check_equals(dest.getPixel(52, 10), 0xff0000);
 check_equals(dest.getPixel(10, 52), 0xff0000);

dest = new flash.display.BitmapData(100, 100, false, 0xff0000);
dest.copyPixels(source, new Rect(0, 0, 100, 100), new Point(50, 50));
 check_equals(dest.getPixel(10, 10), 0xff0000);
 check_equals(dest.getPixel(52, 52), 0x0000ff);
 check_equals(dest.getPixel(52, 10), 0xff0000);
 check_equals(dest.getPixel(10, 52), 0xff0000);

// If the source rect is completely outside the source bitmap, nothing happens
dest = new flash.display.BitmapData(100, 100, false, 0xff0000);
dest.copyPixels(source, new Rect(101, 40, 100, 100), new Point(50, 50));
 check_equals(dest.getPixel(10, 10), 0xff0000);
 check_equals(dest.getPixel(52, 52), 0xff0000);
 check_equals(dest.getPixel(52, 10), 0xff0000);
 check_equals(dest.getPixel(10, 52), 0xff0000);
 check_equals(dest.getPixel(90, 90), 0xff0000);

// If the dest point is right or below the dest bitmap, nothing happens
dest = new flash.display.BitmapData(100, 100, false, 0xff0000);
dest.copyPixels(source, new Rect(0, 0, 100, 100), new Point(200, 50));
 check_equals(dest.getPixel(10, 10), 0xff0000);
 check_equals(dest.getPixel(52, 52), 0xff0000);
 check_equals(dest.getPixel(52, 10), 0xff0000);
 check_equals(dest.getPixel(10, 52), 0xff0000);
 check_equals(dest.getPixel(90, 90), 0xff0000);

// copyChannel

// This function seems to work as expected for single channel to single 
// channel.
// When the destination is a combination of channels, nothing happens. When
// it is a single channel, it is set to 0!

// Source:
//    ---------------------
//    |         |         |
//    |   R     |    RG   |
//    |         |         |
//    |         |         |
//    ---------------------
//    |         |         |
//    |   RB    | G(0x7f) |
//    |         |         |
//    |         |         |
//    ---------------------

// Copy one channel to another
src = new flash.display.BitmapData(100, 100, true);
src.fillRect(new Rect(0, 0, 50, 50), 0xffff0000); // Red channel
src.fillRect(new Rect(50, 0, 50, 50), 0xff00ffff); // Blue and green channels
src.fillRect(new Rect(0, 50, 50, 50), 0xffff00ff); // Red and Blue channels
src.fillRect(new Rect(50, 50, 50, 50), 0xff007f00); // Green channel

// Copy red channel to green channel
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 1, 2);
 xcheck_equals(dest.getPixel(25, 25), 0x00ff00); // Green
 check_equals(dest.getPixel(75, 25), 0x000000); // Nothing
 xcheck_equals(dest.getPixel(25, 75), 0x00ff00); // Was red, now green
 check_equals(dest.getPixel(75, 75), 0x000000); // Nothing

// Copy red channel to green and blue channels
// Doesn't work!
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 1, 6);
 check_equals(dest.getPixel(25, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(25, 75), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 75), 0x000000); // Nothing

// Copy red channel to green and blue channels
// Doesn't work!
dest = new flash.display.BitmapData(100, 100, true, 0xffffffff);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 1, 6);
 check_equals(dest.getPixel(25, 25), 0xffffff); // Nothing
 check_equals(dest.getPixel(75, 25), 0xffffff); // Nothing
 check_equals(dest.getPixel(25, 75), 0xffffff); // Nothing
 check_equals(dest.getPixel(75, 75), 0xffffff); // Nothing

// Copy red and green channels to blue channel
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 3, 4);
 check_equals(dest.getPixel(25, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(25, 75), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 75), 0x000000); // Nothing

// Copy green channel to green and blue channels
// Doesn't work!
dest = new flash.display.BitmapData(100, 100, true, 0xffffffff);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 2, 6);
 check_equals(dest.getPixel(25, 25), 0xffffff); // Nothing
 check_equals(dest.getPixel(75, 25), 0xffffff); // Nothing
 check_equals(dest.getPixel(25, 75), 0xffffff); // Nothing
 check_equals(dest.getPixel(75, 75), 0xffffff); // Nothing

// Copy red and green channels to blue and green channels
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 3, 6);
 check_equals(dest.getPixel(25, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(25, 75), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 75), 0x000000); // Nothing

// Copy red and green to red and green.
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 3, 3);
 check_equals(dest.getPixel(25, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(25, 75), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 75), 0x000000); // Nothing

// Copy green channel to red and green channels
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 2, 3);
 check_equals(dest.getPixel(25, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 25), 0x000000); // Nothing
 check_equals(dest.getPixel(25, 75), 0x000000); // Nothing
 check_equals(dest.getPixel(75, 75), 0x000000); // Nothing

// Copy green channel to blue channel
dest = new flash.display.BitmapData(100, 100, true, 0xff000000);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 2, 4);
 check_equals(dest.getPixel(25, 25), 0x000000); // Nothing
 xcheck_equals(dest.getPixel(75, 25), 0x0000ff); // Blue
 check_equals(dest.getPixel(25, 75), 0x000000); // Nothing
 xcheck_equals(dest.getPixel(75, 75), 0x00007f); // Half blue

// -------------------
// Without alpha
// -------------------

src = new flash.display.BitmapData(100, 100, false);
src.fillRect(new Rect(0, 0, 50, 50), 0xff0000); // Red channel
src.fillRect(new Rect(50, 0, 50, 50), 0x00ffff); // Blue and green channels
src.fillRect(new Rect(0, 50, 50, 50), 0xff00ff); // Red and Blue channels
src.fillRect(new Rect(50, 50, 50, 50), 0x007f00); // Green channel

// Copy red and green to red and green.
dest = new flash.display.BitmapData(100, 100, false);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 3, 3);
 check_equals(dest.getPixel(25, 25), 0xffffff); // Nothing
 check_equals(dest.getPixel(75, 25), 0xffffff); // Nothing
 check_equals(dest.getPixel(25, 75), 0xffffff); // Nothing
 check_equals(dest.getPixel(75, 75), 0xffffff); // Nothing

// Copy red and green to red
dest = new flash.display.BitmapData(100, 100, false);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 3, 1);
 xcheck_equals(dest.getPixel(25, 25), 0x00ffff); // Cyan
 xcheck_equals(dest.getPixel(75, 25), 0x00ffff); // Cyan
 xcheck_equals(dest.getPixel(25, 75), 0x00ffff); // Cyan
 xcheck_equals(dest.getPixel(75, 75), 0x00ffff); // Cyan

// Copy green to red and blue
dest = new flash.display.BitmapData(100, 100, false);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 2, 5);
 check_equals(dest.getPixel(25, 25), 0xffffff); // White
 check_equals(dest.getPixel(75, 25), 0xffffff); // White
 check_equals(dest.getPixel(25, 75), 0xffffff); // White
 check_equals(dest.getPixel(75, 75), 0xffffff); // White

// Copy red and blue to green
dest = new flash.display.BitmapData(100, 100, false);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 5, 2);
 xcheck_equals(dest.getPixel(25, 25), 0xff00ff); // White
 xcheck_equals(dest.getPixel(75, 25), 0xff00ff); // White
 xcheck_equals(dest.getPixel(25, 75), 0xff00ff); // White
 xcheck_equals(dest.getPixel(75, 75), 0xff00ff); // White

// Copy green and blue to blue
dest = new flash.display.BitmapData(100, 100, false);
dest.copyChannel(src, new Rect(0, 0, 100, 100), new Point(0, 0), 6, 4);
 xcheck_equals(dest.getPixel(25, 25), 0xffff00); // Yellow
 xcheck_equals(dest.getPixel(75, 25), 0xffff00); // Yellow
 xcheck_equals(dest.getPixel(25, 75), 0xffff00); // Yellow
 xcheck_equals(dest.getPixel(75, 75), 0xffff00); // Yellow

// clone();

orig = new flash.display.BitmapData(10, 10, false, 0x00ff10);
orig.a = 7;
orig.setPixel(5, 5, 0x0000ff);

// Cloning doesn't clone non-native properties.
cl = orig.clone();
check_equals(cl.a, undefined);
check_equals(cl.width, 10);
check_equals(cl.height, 10);
check_equals(cl.getPixel(2, 2), 0x00ff10);
check_equals(cl.getPixel(5, 5), 0x0000ff);
check_equals(typeof(cl.__proto__), "object");
check_equals(cl.__proto__, orig.__proto__);
check_equals(typeof(cl.constructor), "function");
check_equals(cl.constructor, orig.constructor);

// The constructor is irrelevant.
orig.constructor = 10;
check_equals(orig.constructor, 10);
cl = orig.clone();
check_equals(typeof(cl.__proto__), "object");
check_equals(typeof(cl.constructor), "function");

// The prototype is important.
orig.__proto__ = 8;
check_equals(orig.__proto__, 8);
cl = orig.clone();
check_equals(cl.__proto__, undefined);
check_equals(cl.constructor, undefined);

// What kind of prototype makes this work?
o = {};
o.constructor = 25;
o.clone = flash.display.BitmapData.prototype.clone;
orig.__proto__ = o;
o.width = 20;
o.height = 21;
o.getPixel = flash.display.BitmapData.prototype.getPixel;

cl = orig.clone();
check_equals(cl.__proto__, o);
check_equals(cl.constructor, 25);
check_equals(cl.width, 20);
check_equals(cl.height, 21);
cl.__proto__ = flash.display.BitmapData.prototype;
check_equals(cl.width, 10);
check_equals(cl.height, 10);

e = flash.display.BitmapData.prototype;
orig.__proto__ = e;
flash.display.BitmapData.prototype = 8;
check_equals(flash.display.BitmapData.prototype, 8);
cl = orig.clone();
check_equals(typeof(cl.__proto__), "object");
check_equals(typeof(cl.constructor), "function");

// The constructor property comes from the original __proto__.
cb = e.constructor;
e.constructor = 98;
cl = orig.clone();
check_equals(typeof(cl.__proto__), "object");
check_equals(cl.constructor, 98);

// Restore to original state!
e.constructor = cb;
flash.display.BitmapData.prototype = e;

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals(326);

#endif // OUTPUT_VERSION >= 8
