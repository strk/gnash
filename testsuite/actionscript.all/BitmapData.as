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

check(!Bitmap.prototype.hasOwnProperty('loadBitmap'));
check(Bitmap.hasOwnProperty('loadBitmap'));

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

xcheck_equals(bmp.getPixel(10, 10), 0x0000ff00);
bmp.floodFill(5, 5, 0x000000ff);
xcheck_equals(bmp.getPixel(10, 0), 0x000000ff);

mc = this.createEmptyMovieClip("mc", this.getNextHighestDepth());
mc.attachBitmap(bmp, this.getNextHighestDepth());

Rectangle = flash.geom.Rectangle;

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
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

// Hard ref
b.draw(_level0.tar);
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

// User-defined translation makes no difference.
d._x = 500;
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

// User defined transform makes no difference.
d._height = 30;
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

// User defined transform makes no difference.
d._width = 30;
b.draw(d);
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

// Color transform the old way (no difference).
c = new Color("_level0.tar");  
c.setRGB(0xff5500);
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

// Color transform the new way.
var tr = d.transform;
tr.colorTransform = new flash.geom.ColorTransform(0.5, 0.5, 0.5, 0.5, 34, 34, 34, 34);
d.transform = tr;
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);

dom = new flash.geom.Matrix();
dom.rotate(Math.PI / 4);
tr.matrix = dom;
d.transform = tr;
check_equals(b.getPixel(1, 1), 0xffffff);
xcheck_equals(b.getPixel(21, 21), 0x00ff00);
check_equals(b.getPixel(19, 20), 0xffffff);
xcheck_equals(b.getPixel(79, 79), 0x00ff00);
check_equals(b.getPixel(50, 25), 0xffffff);
xcheck_equals(b.getPixel(55, 55), 0xff0000);


//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals(169);

#endif // OUTPUT_VERSION >= 8
