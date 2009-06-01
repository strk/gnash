// BitmapData_as.hx:  ActionScript 3 "BitmapData" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.utils.ByteArray;
import flash.display.DisplayObject;
import flash.display.MovieClip;
#end
#if flash8
import flash.MovieClip;
#end
#if !flash6
#if !flash7
import flash.geom.Rectangle;
import flash.display.BitmapData;
#end
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class BitmapData_as {
    static function main() {
#if !flash6
#if !flash7
        var x1:BitmapData = new BitmapData(20, 30, false, 0xeeddee);
        // Make sure we actually get a valid class        
        if (Std.is(x1,BitmapData)) {
            DejaGnu.pass("BitmapData class exists");
        } else {
            DejaGnu.fail("BitmapData lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Std.is(x1.height, Int)) {
	    DejaGnu.pass("BitmapData::height property exists");
	} else {
	    DejaGnu.fail("BitmapData::height property doesn't exist");
	}
#if flash9
	if (Std.is(x1.rect, Rectangle)) {
	    DejaGnu.pass("BitmapData::rect property exists");
	} else {
	    DejaGnu.fail("BitmapData::rect property doesn't exist");
	}
#end
#if flash8
	if (Std.is(x1.rectangle, Rectangle)) {
	    DejaGnu.pass("BitmapData::rect property exists");
	} else {
	    DejaGnu.fail("BitmapData::rect property doesn't exist");
	}
#end
	if (Std.is(x1.transparent, Bool)) {
	    DejaGnu.pass("BitmapData::transparent property exists");
	} else {
	    DejaGnu.fail("BitmapData::transparent property doesn't exist");
	}
	if (Std.is(x1.width, Int)) {
	    DejaGnu.pass("BitmapData::width property exists");
	} else {
	    DejaGnu.fail("BitmapData::width property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.applyFilter) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::applyFilter() method exists");
	} else {
	    DejaGnu.fail("BitmapData::applyFilter() method doesn't exist");
	}
 	if (Type.typeof(x1.clone) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::clone() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::clone() method doesn't exist");
 	}
	if (Type.typeof(x1.colorTransform) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::colorTransform() method exists");
	} else {
	    DejaGnu.fail("BitmapData::colorTransform() method doesn't exist");
	}
#if flash9
 	if (Type.typeof(x1.compare) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::compare() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::compare() method doesn't exist");
 	}
#end
	if (Type.typeof(x1.copyChannel) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::copyChannel() method exists");
	} else {
	    DejaGnu.fail("BitmapData::copyChannel() method doesn't exist");
	}
	if (Type.typeof(x1.copyPixels) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::copyPixels() method exists");
	} else {
	    DejaGnu.fail("BitmapData::copyPixels() method doesn't exist");
	}
	if (Type.typeof(x1.dispose) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::dispose() method exists");
	} else {
	    DejaGnu.fail("BitmapData::dispose() method doesn't exist");
	}
	if (Type.typeof(x1.draw) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::draw() method exists");
	} else {
	    DejaGnu.fail("BitmapData::draw() method doesn't exist");
	}
	if (Type.typeof(x1.fillRect) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::fillRect() method exists");
	} else {
	    DejaGnu.fail("BitmapData::fillRect() method doesn't exist");
	}
	if (Type.typeof(x1.floodFill) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::floodFill() method exists");
	} else {
	    DejaGnu.fail("BitmapData::floodFill() method doesn't exist");
	}
 	if (Type.typeof(x1.generateFilterRect) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::generateFilterRect() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::generateFilterRect() method doesn't exist");
 	}
 	if (Type.typeof(x1.getColorBoundsRect) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::getColorBoundsRect() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::getColorBoundsRect() method doesn't exist");
 	}
	if (Type.typeof(x1.getPixel) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::getPixel() method exists");
	} else {
	    DejaGnu.fail("BitmapData::getPixel() method doesn't exist");
	}
	if (Type.typeof(x1.getPixel32) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::getPixel32() method exists");
	} else {
	    DejaGnu.fail("BitmapData::getPixel32() method doesn't exist");
	}
#if flash9
 	if (Type.typeof(x1.getPixels) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::getPixels() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::getPixels() method doesn't exist");
 	}
//FIXME: These method exists in HaXe API Documentation, but is not supported, and does not exist in Adobe documentation
//	if (Type.typeof(x1.getVector) == ValueType.TFunction) {
// 	    DejaGnu.pass("BitmapData::getVector() method exists");
// 	} else {
// 	    DejaGnu.fail("BitmapData::getVector() method doesn't exist");
// 	}
//	if (Type.typeof(x1.histogram) == ValueType.TFunction) {
// 	    DejaGnu.pass("BitmapData::histogram() method exists");
// 	} else {
// 	    DejaGnu.fail("BitmapData::histogram() method doesn't exist");
// 	}
#end
 	if (Type.typeof(x1.hitTest) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::hitTest() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::hitTest() method doesn't exist");
 	}
#if flash9
	if (Type.typeof(x1.lock) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::lock() method exists");
	} else {
	    DejaGnu.fail("BitmapData::lock() method doesn't exist");
	}
#end
	if (Type.typeof(x1.merge) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::merge() method exists");
	} else {
	    DejaGnu.fail("BitmapData::merge() method doesn't exist");
	}
	if (Type.typeof(x1.noise) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::noise() method exists");
	} else {
	    DejaGnu.fail("BitmapData::noise() method doesn't exist");
	}
	if (Type.typeof(x1.paletteMap) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::paletteMap() method exists");
	} else {
	    DejaGnu.fail("BitmapData::paletteMap() method doesn't exist");
	}
	if (Type.typeof(x1.perlinNoise) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::perlinNoise() method exists");
	} else {
	    DejaGnu.fail("BitmapData::perlinNoise() method doesn't exist");
	}
 	if (Type.typeof(x1.pixelDissolve) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::pixelDissolve() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::pixelDissolve() method doesn't exist");
 	}
	if (Type.typeof(x1.scroll) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::scroll() method exists");
	} else {
	    DejaGnu.fail("BitmapData::scroll() method doesn't exist");
	}
	if (Type.typeof(x1.setPixel) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::setPixel() method exists");
	} else {
	    DejaGnu.fail("BitmapData::setPixel() method doesn't exist");
	}
	if (Type.typeof(x1.setPixel32) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::setPixel32() method exists");
	} else {
	    DejaGnu.fail("BitmapData::setPixel32() method doesn't exist");
	}
#if flash9
	if (Type.typeof(x1.setPixels) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::setPixels() method exists");
	} else {
	    DejaGnu.fail("BitmapData::setPixels() method doesn't exist");
	}
//FIXME: This method exists in HaXe API Documentation, but is not supported, and does not exist in Adobe documentation
//	if (Type.typeof(x1.setVector) == ValueType.TFunction) {
//	    DejaGnu.pass("BitmapData::setVector() method exists");
//	} else {
//	    DejaGnu.fail("BitmapData::setVector() method doesn't exist");
//	}
#end
 	if (Type.typeof(x1.threshold) == ValueType.TFunction) {
 	    DejaGnu.pass("BitmapData::threshold() method exists");
 	} else {
 	    DejaGnu.fail("BitmapData::threshold() method doesn't exist");
 	}
#if flash9
	if (Type.typeof(x1.unlock) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::unlock() method exists");
	} else {
	    DejaGnu.fail("BitmapData::unlock() method doesn't exist");
	}
#else
	if (Type.typeof(BitmapData.loadBitmap) == ValueType.TFunction) {
	    DejaGnu.pass("BitmapData::loadBitmap() method exists");
	} else {
	    DejaGnu.fail("BitmapData::loadBitmap() method doesn't exist");
	}
#end

    // Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
#end
#end
#if flash6
	DejaGnu.note("This class (BitmapData) is not available in Flash6");
#end
#if flash7
	DejaGnu.note("This class (BitmapData) is not available in Flash7");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

