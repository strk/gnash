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
xcheck_equals(typeof(bmp), "undefined");

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
xcheck_equals(bmp.rectangle.toString(), "(x=0, y=0, w=10, h=10)");
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
xcheck_equals(bmp.rectangle.toString(), "(x=0, y=0, w=20, h=30)");
check_equals(bmp.getPixel(1, 1), 0xeeddee);
check_equals(bmp.getPixel32(1, 1), -1122834);

// limits

check_equals(bmp.getPixel(50, 1), 0);
check_equals(bmp.getPixel(0, 0), 15654382);
check_equals(bmp.getPixel(-2, -5), 0);

// 0,0 is inside, 20, 30 outside a 20x30 bitmap.
check_equals(bmp.getPixel(20, 30), 0);

bmp = new Bitmap(10000, 0);
xcheck_equals(bmp, undefined);
bmp = new Bitmap(0, 10000);
xcheck_equals(bmp, undefined);

bmp = new Bitmap(2881, 0);
xcheck_equals(typeof(bmp), "undefined");
bmp = new Bitmap(0, 2881);
xcheck_equals(bmp, undefined);


// floodFill
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

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals(77);

#endif // OUTPUT_VERSION >= 8
