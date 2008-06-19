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

rcsid="$Id: BitmapData.as,v 1.1 2008/06/19 17:40:15 bwy Exp $";


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
xcheck_equals(bmp.height, 10);
xcheck_equals(bmp.width, 10);
xcheck_equals(bmp.transparent, true);
xcheck_equals(bmp.rectangle.toString(), "(x=0, y=0, w=10, h=10)");
xcheck_equals(bmp.getPixel(1, 1), 16777215);
xcheck_equals(bmp.getPixel(9, 9), 16777215);
xcheck_equals(bmp.getPixel32(1, 1), -1);

bmp = new Bitmap(20, 30, false, 0xeeddee);
xcheck_equals(bmp.height, 30);
xcheck_equals(bmp.width, 20);
xcheck_equals(bmp.transparent, false);
xcheck_equals(bmp.rectangle.toString(), "(x=0, y=0, w=20, h=30)");
xcheck_equals(bmp.getPixel(1, 1), 0xeeddee);
xcheck_equals(bmp.getPixel32(1, 1), -1122834);

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

totals(51);

#endif // OUTPUT_VERSION >= 8
