// BitmapData_as.hx:  ActionScript 3 "BitmapData" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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
import flash.display.BitmapData;
import flash.display.Bitmap;
import flash.Error;
#end
#if flash8
import flash.MovieClip;
import flash.display.BitmapData;
#end
#if (flash8 || flash9)
import flash.geom.Rectangle;
#end
import flash.Lib;
import Type;
import Std;
import haxe.PosInfos;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class BitmapData_as {
    static function main() {
        
#if !(flash6 || flash7)

    #if flash8
        DejaGnu.note("*** Class/Method/Property existence tests ");
        //check_equals(typeof(Bitmap), 'function');
		if(untyped __typeof__(BitmapData) == 'function') {
			DejaGnu.pass("BitmapData is a function");
		} else {
            DejaGnu.fail("BitmapData is not a function");
		}
        //check_equals(typeof(Bitmap.prototype), 'object');
		if(untyped __typeof__(BitmapData.prototype) == 'object') {
			DejaGnu.pass("BitmapData prototype is an object");
		} else {
            DejaGnu.fail("BitmapData prototype is not an object");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('applyFilter')) {
			DejaGnu.pass("BitmapData prototype has 'applyFilter' property");
		} else {
            DejaGnu.fail("BitmapData prototype does not have 'applyFilter' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('clone')) {
			DejaGnu.pass("BitmapData prototype has 'clone' property");
		} else {
            DejaGnu.fail("BitmapData prototype does not have 'clone' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('colorTransform')) {
			DejaGnu.pass("BitmapData prototype has 'colorTransform' property");
		} else {
            DejaGnu.fail("BitmapData prototype does not have 'colorTransform' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('copyChannel')) {
			DejaGnu.pass("BitmapData prototype has 'copyChannel' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'copyChannel' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('copyPixels')) {
			DejaGnu.pass("BitmapData prototype has 'copyPixels' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'copyPixels' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('dispose')) {
			DejaGnu.pass("BitmapData prototype has 'dispose' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'dispose' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('draw')) {
			DejaGnu.pass("BitmapData prototype has 'draw' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'draw' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('fillRect')) {
			DejaGnu.pass("BitmapData prototype has 'fillRect' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'fillRect' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('floodFill')) {
			DejaGnu.pass("BitmapData prototype has 'floodFill' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'floodFill' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('generateFilterRect')) {
			DejaGnu.pass("BitmapData prototype has 'generateFilterRect' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'generateFilterRect' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('getColorBoundsRect')) {
			DejaGnu.pass("BitmapData prototype has 'getColorBoundsRect' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'getColorBoundsRect' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('getPixel')) {
			DejaGnu.pass("BitmapData prototype has 'getPixel' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'getPixel' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('getPixel32')) {
			DejaGnu.pass("BitmapData prototype has 'getPixel32' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'getPixel32' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('hitTest')) {
			DejaGnu.pass("BitmapData prototype has 'hitTest' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'hitTest' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('merge')) {
			DejaGnu.pass("BitmapData prototype has 'merge' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'merge' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('noise')) {
			DejaGnu.pass("BitmapData prototype has 'noise' property");
		} else {
			DejaGnu.fail(" BitmapData prototype does not have 'noise' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('paletteMap')) {
			DejaGnu.pass("BitmapData prototype has 'paletteMap' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'paletteMap' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('perlinNoise')) {
			DejaGnu.pass("BitmapData prototype has 'perlinNoise' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'perlinNoise' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('pixelDissolve')) {
			DejaGnu.pass("BitmapData prototype has 'pixelDissolve' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'pixelDissolve' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('scroll')) {
			DejaGnu.pass("BitmapData prototype has 'scroll' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'scroll' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('setPixel')) {
			DejaGnu.pass("BitmapData prototype has 'setPixel' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'setPixel' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('setPixel32')) {
			DejaGnu.pass("BitmapData prototype has 'setPixel32' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'setPixel32' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('threshold')) {
			DejaGnu.pass("BitmapData prototype has 'threshold' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'threshold' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('height')) {
			DejaGnu.pass("BitmapData prototype has 'height' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'height' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('width')) {
			DejaGnu.pass("BitmapData prototype has 'width' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'width' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('rectangle')) {
			DejaGnu.pass("BitmapData prototype has 'rectangle' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'rectangle' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty('transparent')) {
			DejaGnu.pass("BitmapData prototype has 'transparent' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'transparent' property");
		}

		if(!(untyped BitmapData.prototype.hasOwnProperty('loadBitmap'))) {
			DejaGnu.pass("BitmapData prototype does not have 'loadBitmap' property");
		} else {
			DejaGnu.fail("BitmapData prototype has 'loadBitmapData' property");
		}
		if(untyped BitmapData.hasOwnProperty('loadBitmap')) {
			DejaGnu.pass("BitmapData has 'loadBitmap' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'loadBitmap' property");
		}
    
    #else
        
        DejaGnu.note("*** Class/Method/Property existence tests ");
        // In flash 9 we seem to need an instance of the object to test these.
        //  This is probably because of the difference in the way inheritance 
        //  and class objects work
        var bdata = new BitmapData(200, 300);
        //check_equals(typeof(Bitmap), 'function');
		if(untyped __typeof__(BitmapData) == 'object') {
			DejaGnu.pass("BitmapData is a function");
		} else {
			DejaGnu.fail("BitmapData is not a function");
		}
        //check_equals(typeof(Bitmap.prototype), 'object');
		if(untyped __typeof__(BitmapData.prototype) == 'object') {
			DejaGnu.pass("BitmapData is an object");
		} else {
			DejaGnu.fail("BitmapData is not an object");
		}
        
		if(untyped bdata.hasOwnProperty('applyFilter')) {
			DejaGnu.pass("BitmapData has 'applyFilter' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'applyFilter' property");
		}
		if(untyped bdata.hasOwnProperty('clone')) {
			DejaGnu.pass("BitmapData has 'clone' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'clone' property");
		}
		if(untyped bdata.hasOwnProperty('colorTransform')) {
			DejaGnu.pass("BitmapData has 'colorTransform' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'colorTransform' property");
		}
		if(untyped bdata.hasOwnProperty('copyChannel')) {
			DejaGnu.pass("BitmapData has 'copyChannel' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'copyChannel' property");
		}
		if(untyped bdata.hasOwnProperty('copyPixels')) {
			DejaGnu.pass("BitmapData has 'copyPixels' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'copyPixels' property");
		}
		if(untyped bdata.hasOwnProperty('dispose')) {
			DejaGnu.pass("BitmapData has 'dispose' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'dispose' property");
		}
		if(untyped bdata.hasOwnProperty('draw')) {
			DejaGnu.pass("BitmapData has 'draw' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'draw' property");
		}
		if(untyped bdata.hasOwnProperty('fillRect')) {
			DejaGnu.pass("BitmapData has 'fillRect' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'fillRect' property");
		}
		if(untyped bdata.hasOwnProperty('floodFill')) {
			DejaGnu.pass("BitmapData has 'floodFill' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'floodFill' property");
		}
		if(untyped bdata.hasOwnProperty('generateFilterRect')) {
			DejaGnu.pass("BitmapData has 'generateFilterRect' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'generateFilterRect' property");
		}
		if(untyped bdata.hasOwnProperty('getColorBoundsRect')) {
			DejaGnu.pass("BitmapData has 'getColorBoundsRect' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'getColorBoundsRect' property");
		}
		if(untyped bdata.hasOwnProperty('getPixel')) {
			DejaGnu.pass("BitmapData has 'getPixel' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'getPixel' property");
		}
		if(untyped bdata.hasOwnProperty('getPixel32')) {
			DejaGnu.pass("BitmapData has 'getPixel32' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'getPixel32' property");
		}
		if(untyped bdata.hasOwnProperty('hitTest')) {
			DejaGnu.pass("BitmapData has 'hitTest' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'hitTest' property");
		}
		if(untyped bdata.hasOwnProperty('merge')) {
			DejaGnu.pass("BitmapData has 'merge' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'merge' property");
		}
		if(untyped bdata.hasOwnProperty('noise')) {
			DejaGnu.pass("BitmapData has 'noise' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'noise' property");
		}
		if(untyped bdata.hasOwnProperty('paletteMap')) {
			DejaGnu.pass("BitmapData has 'paletteMap' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'paletteMap' property");
		}
		if(untyped bdata.hasOwnProperty('perlinNoise')) {
			DejaGnu.pass("BitmapData has 'perlinNoise' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'perlinNoise' property");
		}
		if(untyped bdata.hasOwnProperty('pixelDissolve')) {
			DejaGnu.pass("BitmapData has 'pixelDissolve' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'pixelDissolve' property");
		}
		if(untyped bdata.hasOwnProperty('scroll')) {
			DejaGnu.pass("BitmapData has 'scroll' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'scroll' property");
		}
		if(untyped bdata.hasOwnProperty('setPixel')) {
			DejaGnu.pass("BitmapData has 'setPixel' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'setPixel' property");
		}
		if(untyped bdata.hasOwnProperty('setPixel32')) {
			DejaGnu.pass("BitmapData has 'setPixel32' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'setPixel32' property");
		}
		if(untyped bdata.hasOwnProperty('threshold')) {
			DejaGnu.pass("BitmapData has 'threshold' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'threshold' property");
		}
		if(untyped bdata.hasOwnProperty('height')) {
			DejaGnu.pass("BitmapData has 'height' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'height' property");
		}
		if(untyped bdata.hasOwnProperty('width')) {
			DejaGnu.pass("BitmapData has 'width' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'width' property");
		}
		if(untyped bdata.hasOwnProperty('rect')) {
			DejaGnu.pass("BitmapData has 'rectangle' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'rectangle' property");
		}
		if(untyped bdata.hasOwnProperty('transparent')) {
			DejaGnu.pass("BitmapData has 'transparent' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'transparent' property");
		}
        //NOTE: This property supposedly does not exist in flash9 (according to
        // the Adobe livedocs but this test seems to pass anyway
		if( !(untyped bdata.hasOwnProperty('loadBitmap')) ) {
			DejaGnu.pass("BitmapData 'loadBitmap' property exists even though livedocs claim it doesn't");
		} else {
			DejaGnu.fail("BitmapData does not have 'loadBitmap' property");
		}
        
    #end
        
        //Testing that functions are actually functions
        var x1 = new BitmapData(20, 30, false, 0xeeddee);
        // Change these, but keep them
		if (Type.typeof(x1.applyFilter) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::applyFilter() is a function");
		} else {
			DejaGnu.fail(" BitmapData::applyFilter() is not a function");
		}
	 	if (Type.typeof(x1.clone) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::clone() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::clone() is not a function");
	 	}
		if (Type.typeof(x1.colorTransform) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::colorTransform() is a function");
		} else {
			DejaGnu.fail(" BitmapData::colorTransform() is not a function");
		}
    #if flash9
	 	if (Type.typeof(x1.compare) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::compare() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::compare() is not a function");
	 	}
    #end
		if (Type.typeof(x1.copyChannel) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::copyChannel() is a function");
		} else {
			DejaGnu.fail(" BitmapData::copyChannel() is not a function");
		}
		if (Type.typeof(x1.copyPixels) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::copyPixels() is a function");
		} else {
			DejaGnu.fail(" BitmapData::copyPixels() is not a function");
		}
		if (Type.typeof(x1.dispose) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::dispose() is a function");
		} else {
			DejaGnu.fail(" BitmapData::dispose() is not a function");
		}
		if (Type.typeof(x1.draw) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::draw() is a function");
		} else {
			DejaGnu.fail(" BitmapData::draw() is not a function");
		}
	 	if (Type.typeof(x1.generateFilterRect) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::generateFilterRect() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::generateFilterRect() is not a function");
	 	}
	 	if (Type.typeof(x1.getColorBoundsRect) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::getColorBoundsRect() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::getColorBoundsRect() is not a function");
	 	}
    #if flash9
	 	if (Type.typeof(x1.getPixels) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::getPixels() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::getPixels() is not a function");
	 	}
    //FIXME: These method exists in HaXe API Documentation, but is not supported, and does not exist in Adobe documentation
    //	if (Type.typeof(x1.getVector) == ValueType.TFunction) {
    // 	    DejaGnu.pass("BitmapData::getVector() method exists");
    // 	} else {
    // 	    DejaGnu.fail("BitmapData::getVector() is not a function");
    // 	}
    //	if (Type.typeof(x1.histogram) == ValueType.TFunction) {
    // 	    DejaGnu.pass("BitmapData::histogram() method exists");
    // 	} else {
    // 	    DejaGnu.fail("BitmapData::histogram() is not a function");
    // 	}
    #end
	 	if (Type.typeof(x1.hitTest) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::hitTest() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::hitTest() is not a function");
	 	}
    #if flash9
		if (Type.typeof(x1.lock) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::lock() is a function");
		} else {
			DejaGnu.fail(" BitmapData::lock() is not a function");
		}
    #end
		if (Type.typeof(x1.merge) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::merge() is a function");
		} else {
			DejaGnu.fail(" BitmapData::merge() is not a function");
		}
		if (Type.typeof(x1.noise) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::noise() is a function");
		} else {
			DejaGnu.fail(" BitmapData::noise() is not a function");
		}
		if (Type.typeof(x1.paletteMap) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::paletteMap() is a function");
		} else {
			DejaGnu.fail(" BitmapData::paletteMap() is not a function");
		}
		if (Type.typeof(x1.perlinNoise) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::perlinNoise() is a function");
		} else {
			DejaGnu.fail(" BitmapData::perlinNoise() is not a function");
		}
	 	if (Type.typeof(x1.pixelDissolve) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::pixelDissolve() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::pixelDissolve() is not a function");
	 	}
		if (Type.typeof(x1.scroll) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::scroll() is a function");
		} else {
			DejaGnu.fail(" BitmapData::scroll() is not a function");
		}
    #if flash9
		if (Type.typeof(x1.setPixels) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::setPixels() is a function");
		} else {
			DejaGnu.fail(" BitmapData::setPixels() is not a function");
		}
    //FIXME: This method exists in HaXe API Documentation, but is not supported, and does not exist in Adobe documentation
    //	if (Type.typeof(x1.setVector) == ValueType.TFunction) {
    //	    DejaGnu.pass("BitmapData::setVector() method exists");
    //	} else {
    //	    DejaGnu.fail("BitmapData::setVector() is not a function");
    //	}
    #end
	 	if (Type.typeof(x1.threshold) == ValueType.TFunction) {
	 	    DejaGnu.pass("BitmapData::threshold() is a function");
	 	} else {
	 	    DejaGnu.fail(" BitmapData::threshold() is not a function");
	 	}
    #if flash9
		if (Type.typeof(x1.unlock) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::unlock() is a function");
		} else {
			DejaGnu.fail(" BitmapData::unlock() is not a function");
		}
    #else
		if (Type.typeof(BitmapData.loadBitmap) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::loadBitmap() is a function");
		} else {
			DejaGnu.fail(" BitmapData::loadBitmap() is not a function");
		}
    #end

		//-------------------------------------------------------------
		// Test constructor
		//-------------------------------------------------------------
        DejaGnu.note("*** BitmapData Constructor Tests ***");
    
    #if flash8
        DejaGnu.note("** Ctor BitmapData()");
		var bmpNoArg:BitmapData = untyped __new__(BitmapData);

        //FIXME: This test illustrates a problem in gnash that nobody has fixed
		if(untyped __typeof__(bmpNoArg) == 'undefined') {
			DejaGnu.pass("Bitmap constructor with no args returned undefined");
		} else {
			DejaGnu.fail("Bitmap constructor did not return undefined");
		}
    #else
        //NOTE: Should Gnash throw an error here too?
        //  finally figured out how to make a test out of this with try .. catch
        //  will probably have to individually test error codes to see what each
        //  errorID is
        DejaGnu.note("The PP throws an error if we call the constructor with no arguments");
        try {
            var bmpNoArg:BitmapData = untyped __new__(BitmapData);
            DejaGnu.fail("Ctor BitmapData() did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData() correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor BitmapData() correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError', error caught: " + e);
            }
        }
        
    #end
		
        DejaGnu.note("** Ctor BitmapData(10, 10)");
        var bmp = new BitmapData(10, 10);
        //check_equals(typeof(bmp), 'object');
        if (untyped __typeof__(bmp) == 'object') {
            DejaGnu.pass("Constructor BitmapData(10, 10) returns an object");
        } else {
            DejaGnu.fail("Constructor BitmapData(10, 10) did not return an object");
        }
        
        //NOTE: flash 9 does not have the instanceof function, however the haxe
        // function Std.is seems to work in SWF8 & 9. This probably needs more
        // investigation
        //check(bmp instanceof Bitmap);
        if (Std.is(bmp, BitmapData)) {
            DejaGnu.pass("bmp is an instance of a BitmapData");
        } else {
            DejaGnu.fail("bmp is not an instance of a BitmapData");
        }
        
        
    #if flash8
        //NOTE: The pp gives different behavior here between SWF8 and SWF9. This 
        //   has to do with the differences in property inheritance and the 
        //   hasOwnProperty function between the two versions
        
        // These are prototype properties in flash8 and don't exist on instances
        //check(!bmp.hasOwnProperty("height"));
        if ( !(untyped bmp.hasOwnProperty('height')) ) {
            DejaGnu.pass("Constructor did not set the 'height' property for bmp");
        } else {
            DejaGnu.fail("Constructor set the 'height' property for bmp");
        }
        //check(!bmp.hasOwnProperty("width"));
        if ( !(untyped bmp.hasOwnProperty("width")) ) {
            DejaGnu.pass("Constructor did not set the 'width' property for bmp");
        } else {
            DejaGnu.fail("Constructor set the 'width' property for bmp");
        }
        //check(!bmp.hasOwnProperty("rectangle"));
        if ( !(untyped bmp.hasOwnProperty('rectangle')) ) {
            DejaGnu.pass("Constructor did not set the 'rectangle' property for bmp");
        } else {
            DejaGnu.fail("Constructor set the 'rectangle' property for bmp");
        }
        //check(!bmp.hasOwnProperty("transparent"));
        if ( !(untyped bmp.hasOwnProperty('transparent')) ) {
            DejaGnu.pass("Constructor did not set the 'transparent' property for bmp");
        } else {
            DejaGnu.fail("Constructor set the 'transparent' property for bmp");
        }
    #else
        //These are object properties in flash9 that the instance should have
        if ( untyped bmp.hasOwnProperty('height') ) {
            DejaGnu.pass("Constructor set the 'height' property for bmp");
        } else {
            DejaGnu.fail("Constructor did not set the 'height' property for bmp");
        }
        if ( untyped bmp.hasOwnProperty("width") ) {
            DejaGnu.pass("Constructor set the 'width' property for bmp");
        } else {
            DejaGnu.fail("Constructor did not set the 'width' property for bmp");
        }
        //NOTE: need to check 'rect' for flash 9
        if ( untyped bmp.hasOwnProperty('rect') ) {
            DejaGnu.pass("Constructor set the 'rect' property for bmp");
        } else {
            DejaGnu.fail("Constructor did not set the 'rect' property for bmp");
        }
        if ( untyped bmp.hasOwnProperty('transparent') ) {
            DejaGnu.pass("Constructor set the 'transparent' property for bmp");
        } else {
            DejaGnu.fail("Constructor did not set the 'transparent' property for bmp");
        }
    #end
        //check_equals(bmp.height, 10);
        if (bmp.height == 10) {
            DejaGnu.pass("bmp.height == 10 after construction");
        } else {
            DejaGnu.fail("bmp.height != 10 after construction");
        }
        //check_equals(bmp.width, 10);
        if (bmp.width == 10) {
            DejaGnu.pass("bmp.width == 10 after construction");
        } else {
            DejaGnu.fail("bmp.width != 10 after construction");
        }
        //check_equals(bmp.transparent, true);
        if (bmp.transparent == true) {
            DejaGnu.pass("bmp.transparent == true after construction");
        } else {
            DejaGnu.fail("bmp.transparent != true after construction");
        }
    #if flash8
        //check_equals(bmp.rectangle.toString(), "(x=0, y=0, w=10, h=10)");
        if (bmp.rectangle.toString() == "(x=0, y=0, w=10, h=10)") {
            DejaGnu.pass("bmp.rectangle initialized correctly during construction");
        } else {
            DejaGnu.fail("bmp.rectangle not initialized correctly during construction");
        }
        //check(bmp.rectangle instanceOf flash.geom.Rectangle);
        if (Std.is(bmp.rectangle, flash.geom.Rectangle)) {
            DejaGnu.pass("bmp.rectangle is an instance of a rectangle");
        } else {
            DejaGnu.fail("bmp.rectangle is not an instance of a rectangle");
        }
    #else
        if (bmp.rect.toString() == "(x=0, y=0, w=10, h=10)") {
            DejaGnu.pass("bmp.rect initialized correctly during construction");
        } else {
            DejaGnu.fail("bmp.rect not initialized correctly during construction");
        }
        //check(bmp.rectangle instanceOf flash.geom.Rectangle);
        if (Std.is(bmp.rect, flash.geom.Rectangle)) {
            DejaGnu.pass("bmp.rect is an instance of a rectangle");
        } else {
            DejaGnu.fail("bmp.rect is not an instance of a rectangle");
        }
    #end
        //check_equals(bmp.getPixel(1, 1), 16777215);
        if (bmp.getPixel(1,1) == 16777215) {
            DejaGnu.pass("bmp pixel (1, 1,) initialized correctly");
        } else {
            DejaGnu.fail("bmp pixel (1, 1) was not initialized correctly");
        }
        //check_equals(bmp.getPixel(9, 9), 16777215);
        if (bmp.getPixel(9, 9) == 16777215) {
            DejaGnu.pass("bmp pixel (9, 9) initialized correctly");
        } else {
            DejaGnu.fail("bmp pixel (9, 9) was not initialized correctly");
        }
    #if flash8
        //NOTE: The pp returns different numbers here based on the version.
        // Different precission values are returned based on version
        
        // The livedocs claim that SWF8 returns a Number, which is a double-
        // precission IEEE-754 value

        //check_equals(bmp.getPixel32(1, 1), -1);
        if (bmp.getPixel32(1, 1) == -1) {
            DejaGnu.pass("bmp.getPixel32(1, 1) correctly returned -1");
        } else {
            DejaGnu.fail("bmp.getPixel32(1 1) did not return correctly");
        }
    #else
        // The livedocs claim that SWF9 returns a uint value, which is a 32 bit
        // unsigned integer
        if (bmp.getPixel32(1, 1) == 4294967295) {
            DejaGnu.pass("bmp.getPixel32(1, 1) correctly returns 4294967295");
        } else {
            DejaGnu.fail("bmp.getPixel32(1, 1) did not return correctly");
        }
    #end
        
        
        DejaGnu.note("** Ctor BitmapData(10, 10, true)");
        bmp = new BitmapData(10, 10, true);
        //see previous note about precission
    #if flash8
        //check_equals(bmp.getPixel32(1, 1), -1);
        if (bmp.getPixel32(1, 1) == -1) {
            DejaGnu.pass("Ctor BitmapData(10, 10, true) created correct (1, 1) pixel");
        } else {
            DejaGnu.fail("Constructor BitmapData(10, 10, true) did not operate correctly");
        }
    #else
        if (bmp.getPixel32(1, 1) == 4294967295) {
            DejaGnu.pass("Ctor BitmapData(10, 10, true) created correct (1, 1) pixel");
        } else {
            DejaGnu.fail("Constructor BitmapData(10, 10, true) did not operate correctly");
        }
    #end
        
        DejaGnu.note("** Ctor BitmapData(10, 10, false)");
        bmp = new BitmapData(10, 10, false);
        //see previous note about precission
    #if flash8
        //check_equals(bmp.getPixel32(1, 1), -1);
        if (bmp.getPixel32(1, 1) == -1) {
            DejaGnu.pass("Ctor BitmapData(10, 10, false) created correct (1, 1) pixel");
        } else {
            DejaGnu.fail("Ctor BitmapData(10, 10, false) did not operate correctly");
        }
    #else
        if (bmp.getPixel32(1, 1) == 4294967295) {
            DejaGnu.pass("Ctor BitmapData(10, 10, true) created correct (1, 1) pixel");
        } else {
            DejaGnu.fail("Constructor BitmapData(10, 10, true) did not operate correctly");
        }
    #end

		
        DejaGnu.note("** Ctor BitmapData(10, 30, false, 0xeeddee)");
        var x1 = new BitmapData(20, 30, false, 0xeeddee);
        // check(x1 instanceof Bitmap);      
        if (Std.is(x1,BitmapData)) {
            DejaGnu.pass("x1 is an instance of a BitmapData");
        } else {
            DejaGnu.fail("x1 is not an instance of a BitmapData");
        }

		//check_equals(x1.height, 10);
		if (Type.typeof(x1.height) == ValueType.TInt) {
			DejaGnu.pass("BitmapData::height property exists");
			if (x1.height == 30) {
				DejaGnu.pass("BitmapData.height property is correct int (30)");
			} else {
                DejaGnu.fail("BitmapData.height " + 
                  "property is incorrect int (should be 30, is "+x1.height+")");
			}
		} else {
			DejaGnu.fail("BitmapData::height property doesn't exist");
		}
		//check_equals(x1.width, 10);
		if (Type.typeof(x1.width) == ValueType.TInt) {
			DejaGnu.pass("BitmapData::width property exists");
			if (x1.width == 20) {
				DejaGnu.pass("BitmapData.width property is correct int (20)");
			} else {
                DejaGnu.fail("BitmapData.width property"
                          + "is incorrect int (should be 20, is "+x1.width+")");
			}
		} else {
            DejaGnu.fail("BitmapData::width property doesn't exist");
		}
		//check_equals(x1.transparent, true);
		if (Type.typeof(x1.transparent) == ValueType.TBool) {
			DejaGnu.pass("BitmapData::transparent property exists");
			if (x1.transparent == false) {
				DejaGnu.pass("BitmapData.transparent property is correct bool (false)");
			} else {
                DejaGnu.fail("BitmapData.transparent" +
                             "property is incorrect bool (should be false, is "+
                             x1.transparent+")");
			}
		} else {
            DejaGnu.fail("BitmapData::transparent property doesn't exist");
		}
    #if flash9
		//check_equals(x1.rect.toString(), "(x=0, y=0, w=10, h=10)");
		if (Std.is(x1.rect, Rectangle)) {
			DejaGnu.pass("BitmapData::rect property exists");
			if (x1.rect.toString() == "(x=0, y=0, w=20, h=30)") {
				DejaGnu.pass("BitmapData.rect property is correct rectangle (x=0, y=0, w=20, h=30)");
			} else {
                DejaGnu.fail("BitmapData.rect property is incorrect rectangle (should be (x=0, y=0, w=20, h=30), is "+x1.rect.toString()+")");
			}
		} else {
            DejaGnu.fail("BitmapData::rect property doesn't exist");
		}
    #else
		//check_equals(x1.rectangle.toString(), "(x=0, y=0, w=10, h=10)");
		if (Std.is(x1.rectangle, Rectangle)) {
			DejaGnu.pass("BitmapData::rectangle property exists");
			if (x1.rectangle.toString() == "(x=0, y=0, w=20, h=30)") {
				DejaGnu.pass("BitmapData.rectangle property is correct rectangle (x=0, y=0, w=20, h=30)");
			} else {
                DejaGnu.fail("BitmapData.rectangle property is incorrect rectangle (should be (x=0, y=0, w=20, h=30), is "+x1.rectangle.toString()+")");
			}
		} else {
			DejaGnu.fail("BitmapData::rectangle property doesn't exist");
		}
    #end

		//check_equals(bmp.getPixel(1, 1), 0xeeddee);
        if (x1.getPixel(1,1) == 0xeeddee) {
            DejaGnu.pass("BitmapData::getPixel(1,1) method returns correct number (0xeeddee)");
        } else {
            DejaGnu.fail("BitmapData::getPixel(1,1) method returns incorrect number (should be 0xeeddee, is "+x1.getPixel(1,1)+")");
        }
        //check_equals(bmp.getPixel(50, 1), 0);
        if (x1.getPixel(50,1) == 0) {
            DejaGnu.pass("BitmapData::getPixel(50,1) returns correct value (0)");
        } else {
            DejaGnu.fail("BitmapData::getPixel(50,1) returns incorrect value (should be 0, is "+x1.getPixel(50,1)+")");
        }
        //check_equals(bmp.getPixel(0, 0), 15654382);
        if (x1.getPixel(0,0) == 0xeeddee) {
            DejaGnu.pass("BitmapData::getPixel(0,0) returns correct value (0xeeddee)");
        } else {
            DejaGnu.fail("BitmapData::getPixel(0,0) returns incorrect value (should be 0xeeddee, is "+x1.getPixel(0,0)+")");
        }
        //check_equals(bmp.getPixel(-2, -5), 0);
        if (x1.getPixel(-2,-5) == 0) {
            DejaGnu.pass("BitmapData::getPixel(-2,-5) returns correct value (0)");
        } else {
            DejaGnu.fail("BitmapData::getPixel(-2,-5) returns incorrect value (should be 0, is "+x1.getPixel(-2,-5)+")");
        }
        //check_equals(bmp.getPixel(20, 30), 0);
        if (x1.getPixel(20,30) == 0) {
            DejaGnu.pass("BitmapData::getPixel(20,30) returns correct value (0)");
        } else {
            DejaGnu.fail("BitmapData::getPixel(20,30) returns incorrect value (should be 0, is "+x1.getPixel(20,30)+")");
        }
        
		
		//check_equals(bmp.getPixel32(1, 1), -1122834);
        //getPixel32() should return a float in AS2, and a uint in AS3.
#if !flash9
        if (untyped __typeof__(x1.getPixel32(1,1)) == 'number') {
            DejaGnu.pass("BitmapData::getPixel32() method returns correct type (number)");
            if (x1.getPixel32(1,1) == -1122834) {
                DejaGnu.pass("BitmapData::getPixel32() method returns correct number (-01122834)");
            } else {
                DejaGnu.fail("BitmapData::getPixel32() method returns incorrect number (should be -01122834, is "+x1.getPixel32(1,1)+")");
            }
        } else {
            DejaGnu.fail("BitmapData::getPixel32() method returns incorrect type (should be number, is "+untyped __typeof__(x1.getPixel32(1,1))+")");
        }
#else
        if ( untyped __typeof__(x1.getPixel32(1,1)) == 'number') {
            DejaGnu.pass("BitmapData::getPixel32() method returns correct type 'number'");			
            if (x1.getPixel32(1,1) == 4293844462) {
                DejaGnu.pass("BitmapData::getPixel32() method returns correct number (4293844462)");
            } else {
                DejaGnu.fail("BitmapData::getPixel32() method returns incorrect number (should be 4293844462, is "+x1.getPixel32(1,1)+")");
            }
        } else {
            DejaGnu.xfail("BitmapData::getPixel32() method returns incorrect type (should be number, is "+untyped __typeof__(x1.getPixel32(1,1))+")");
        }
#end
		
		// Testing limits on constructor values:
        // 2880 is the maximum dimension value, 1 the minimum. Returns
        // undefined if the dimensions are invalid.
        DejaGnu.note("*** Ctor limit tests ");
        
        
        //bmp = new Bitmap(2880, 2880);
        var bmp = new BitmapData(2880, 2880);
        //check_equals(typeof(bmp), "object");
        if (untyped __typeof__(bmp) == 'object') {
            DejaGnu.pass("Ctor BitmapData(2880, 2880) returned an 'object'");
        } else {
            DejaGnu.fail("Ctor BitmapData(2880, 2880) did not return an 'object'");
        }
        //check_equals(bmp.height, 2880);
        if ( bmp.height == 2880) {
            DejaGnu.pass("bmp.height value is 2880");
        } else {
            DejaGnu.fail("bmp.height value is not 2880");
        }
        
        //bmp = new Bitmap(2879, 2879);
        bmp = new BitmapData(2879, 2879);
        //check_equals(typeof(bmp), "object");
        if (untyped __typeof__(bmp) == 'object') {
            DejaGnu.pass("Ctor BitmapData(2879, 2879) returned 'object'");
        } else {
            DejaGnu.fail("Ctor BitmapData(2879, 2879) did not return an 'object'");
        }
        //check_equals(bmp.height, 2879);
        if ( bmp.height == 2879) {
            DejaGnu.pass("bmp.height value is 2879");
        } else {
            DejaGnu.fail("bmp.height value is not 2879");
        }
        
    #if flash8
        //bmp = new Bitmap(10000, 3);
        bmp = new BitmapData(10000, 3);
        //xcheck_equals(typeof(bmp), "undefined");
        if (untyped __typeof__(bmp) == 'undefined') {
            DejaGnu.pass("Ctor BitmapData(10000, 3) returned 'undefined'");
        } else {
            DejaGnu.fail("Ctor BitmapData(10000, 3) returned an object instead of 'undefined'");
        }
        //check_equals(bmp.height, undefined);
        if ( bmp.height == null) {
            DejaGnu.pass("bmp.height value is 'undefined'");
        } else {
            DejaGnu.fail("bmp.height value is not 'undefined'");
        }
        
        //bmp = new Bitmap(0, 10000)
        bmp = new BitmapData(0, 10000);
        //xcheck_equals(typeof(bmp), "undefined");
        if (untyped __typeof__(bmp) == 'undefined') {
            DejaGnu.pass("Ctor BitmapData(0, 10000) returned 'undefined'");
        } else {
            DejaGnu.fail("Ctor BitmapData(0, 10000) returned an object instead of 'undefined'");
        }
        //check_equals(bmp.height, undefined);
        if ( bmp.height == null) {
            DejaGnu.pass("bmp.height value is 'undefined'");
        } else {
            DejaGnu.fail("bmp.height value is not 'undefined'");
        }

        //bmp = new Bitmap(2880, 2881);
        bmp = new BitmapData(2880, 2881);
        //xcheck_equals(typeof(bmp), "undefined");
        if (untyped __typeof__(bmp) == 'undefined') {
            DejaGnu.pass("Ctor BitmapData(2880, 2881) returned 'undefined'");
        } else {
            DejaGnu.fail("Ctor BitmapData(2880, 2881) returned an object instead of 'undefined'");
        }
        //check_equals(bmp.height, undefined);
        if ( bmp.height == null) {
            DejaGnu.pass("bmp.height value is 'undefined'");
        } else {
            DejaGnu.fail("bmp.height value is not 'undefined'");
        }

        //bmp = new Bitmap(0, 2880);
        bmp = new BitmapData(0, 2880);
        //xcheck_equals(bmp, undefined);
        if (untyped __typeof__(bmp) == 'undefined') {
            DejaGnu.pass("Ctor BitmapData(0, 2880) returned 'undefined'");
        } else {
            DejaGnu.fail("Ctor BitmapData(0, 2880) returned an object instead of 'undefined'");
        }
        //check_equals(bmp.height, undefined);
        if ( bmp.height == null) {
            DejaGnu.pass("bmp.height value is 'undefined'");
        } else {
            DejaGnu.fail("bmp.height value is not 'undefined'");
        }

        //bmp = new Bitmap(0, 2879);
        bmp = new BitmapData(0, 2879);
        //xcheck_equals(bmp, undefined);
        if (untyped __typeof__(bmp) == 'undefined') {
            DejaGnu.pass("Ctor BitmapData(0, 2879) returned 'undefined'");
        } else {
            DejaGnu.fail("Ctor BitmapData(0, 2879) returned an object instead of 'undefined'");
        }
        //check_equals(bmp.height, undefined);
        if ( bmp.height == null) {
            DejaGnu.pass("bmp.height value is 'undefined'");
        } else {
            DejaGnu.fail("bmp.height value is not 'undefined'");
        }

        //bmp = new Bitmap(-1, 10, false, 0xff);
        bmp = new BitmapData(-1, 10, false, 0xff);
        //xcheck_equals(bmp, undefined);
        if (untyped __typeof__(bmp) == 'undefined') {
            DejaGnu.pass("Ctor BitmapData(-1, 10, false, 0xff) returned 'undefined'");
        } else {
            DejaGnu.fail("Ctor BitmapData(-1, 10, false, 0xff) returned an object instead of 'undefined'");
        }
        //check_equals(bmp.height, undefined)
        if ( bmp.height == null) {
            DejaGnu.pass("bmp.height value is 'undefined'");
        } else {
            DejaGnu.fail("bmp.height value is not 'undefined'");
        }
        
    #else
    
        //NOTE: Gnash should also throw SWF errors here
        DejaGnu.note("The pp throws an 'ArgumentError' in SWF9 if we try to call a constructor with out of bounds arguments");
        
        //bmp = new Bitmap(0, 10000
        try {
            bmp = new BitmapData(10000, 3);
            DejaGnu.fail("Ctor BitmapData(10000, 3) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData(10000, 3) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError'; error caught: " + e);
            }
        }
        
        //bmp = new Bitmap(0, 10000)
        try {
            bmp = new BitmapData(0, 10000);
            DejaGnu.fail("Ctor BitmapData(0, 10000) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData(10, 10000) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError'; error caught: " + e);
            }
        }

        //bmp = new Bitmap(2880, 2881);
        try {
            bmp = new BitmapData(2880, 2881);
            DejaGnu.fail("Ctor BitmapData(2880, 2881) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData(2880, 2881) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError'; error caught: " + e);
            }
        }
        
        //bmp = new Bitmap(0, 2880);
        try {
            bmp = new BitmapData(0, 2880);
            DejaGnu.fail("Ctor BitmapData(0, 2880) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData(0, 2880) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError'; error caught: " + e);
            }
        }

        //bmp = new Bitmap(0, 2879);
        try {
            bmp = new BitmapData(0, 2879);
            DejaGnu.fail("Ctor BitmapData(0, 2879) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData(0, 2879) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError'; error caught: " + e);
            }
        }
      
        //bmp = new Bitmap(-1, 10, false, 0xff);
        try {
            bmp = new BitmapData(-1, 10, false, 0xff);
            DejaGnu.fail("Ctor BitmapData(-1, 10, false, 0xff) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("Ctor BitmapData(-1, 10, false, 0xff) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("Ctor correctly threw 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("Ctor did not throw 'ArgumentError'; error caught: " + e);
            }
        }
      
        
    #end

        
		// --------------------
		// setPixel, setPixel32
		// --------------------
        DejaGnu.note("**** setPixel and setPixel32 testing ****");
		
        var tr:BitmapData = new BitmapData(30, 30, true);
        var ntr:BitmapData = new BitmapData(30, 30, false);
        
        if (untyped __typeof__(tr.setPixel) == 'function') {
			DejaGnu.pass("BitmapData::setPixel() is a function");
        } else {
			DejaGnu.fail(" BitmapData::setPixel() is not a function");
		}
        //FIXME: How to check return type void
        if (untyped __typeof__(tr.setPixel32) == 'function') {
			DejaGnu.pass("BitmapData::setPixel32() is a function");
        } else {
			DejaGnu.fail(" BitmapData::setPixel32() is not a function");
		}
        //FIXME: How to check return type void

        
        tr.setPixel(3, 3, 0xff);
        //check_equals(tr.getPixel(3, 3), 0xff);
        if (tr.getPixel(3,3) == 0xff) {
            DejaGnu.pass("BitmapData::setPixel(3,3,0xff) has set the correct value (0xff)");
        } else {
            DejaGnu.fail(" BitmapData::setPixel(3,3,0xff) has set an incorrect value (should be 0xff, is "+tr.getPixel(3,3)+")");
        }
        //check_equals(tr.getPixel32(3, 3), -16776961);
    #if flash8
        if ((tr.getPixel32(3, 3) == -16776961)) {
            DejaGnu.pass("BitmapData::setPixel(3,3,0xff) has set the correct value (-16776961)");
        } else {
            DejaGnu.fail(" BitmapData::setPixel(3,3,0xff) has set an incorrect value (should be -16776961, is "+tr.getPixel32(3,3)+")");
        }
    #else
        // compiler bug?
        if (tr.getPixel32(3, 3) == 4278190335) {
            DejaGnu.pass("BitmapData::setPixel(3,3,0xff) has set the correct value (4278190335)");
        } else {
            DejaGnu.fail(" BitmapData::setPixel(3,3,0xff) has set an incorrect value (should be 4278190335, is "+tr.getPixel32(3,3)+")");
        }
    #end
        
        ntr.setPixel(5, 5, 0xff);
        //check_equals(ntr.getPixel(5, 5), 0xff);
        if (ntr.getPixel(5, 5) == 0xff) {
            DejaGnu.pass("setPixel(3,3,0xff) has set the correct value - getPixel");
        } else {
            DejaGnu.fail("setPixel(5, 5, 0xff) set incorrect value: "+ntr.getPixel(5,5)+" should be 0xff");
        }
        //check_equals(ntr.getPixel32(5, 5), -16776961);
    #if flash8
        if (ntr.getPixel32(5,5) == -16776961) {
            DejaGnu.pass("setPixel(3,3,0xff) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel(5, 5, 0xff) set incorrect value: "+ntr.getPixel(5,5)+" should be -16776961");
        }
    #else
        if (ntr.getPixel32(5,5) == 4278190335) {
            DejaGnu.pass("setPixel(3,3,0xff) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel(5, 5, 0xff) set incorrect value: "+ntr.getPixel32(5,5)+" should be 4278190335");
        }
    #end
        
        // not sure what the following not from the ming test means
        //// Premultiplication?
        tr.setPixel32(2, 2, 0x220000aa);
        //xcheck_equals(tr.getPixel(2, 2), 0xac);
        if (tr.getPixel(2, 2) == 0xac) {
            DejaGnu.xpass("setPixel32(2, 2, 0x220000aa) has set the correct value - getPixel");
        } else {
            DejaGnu.xfail("setPixel32(2, 2, 0x220000aa) set incorrect value: "+tr.getPixel(2,2)+" should be 0xac");
        }
        //xcheck_equals(tr.getPixel32(2, 2), 0x220000ac);
        if (tr.getPixel32(2, 2) == 0x220000ac) {
            DejaGnu.xpass("setPixel32(2, 2, 0x220000aa) has set the correct value - getPixel32");
        } else {
            DejaGnu.xfail("setPixel32(2, 2, 0x220000aa) set incorrect value: "+tr.getPixel(2,2)+" should be 0x0x220000ac");
        }

        tr.setPixel32(2, 2, 0xff0000aa);
        //check_equals(tr.getPixel(2, 2), 0xaa);
        if (tr.getPixel(2, 2) == 0xaa) {
            DejaGnu.pass("setPixel32(2, 2, 0xff0000aa) has set the correct value - getPixel");
        } else {
            DejaGnu.fail("setPixel32(2, 2, 0xff0000aa) set incorrect value: "+tr.getPixel(2,2)+" should be 0xaa");
        }
        //check_equals(tr.getPixel32(2, 2), -16777046);
    #if flash8
        if (tr.getPixel32(2, 2) == -16777046) {
            DejaGnu.pass("setPixel32(2, 2, 0xff0000aa) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel32(2, 2, 0xff0000aa) set incorrect value: "+tr.getPixel32(2,2)+" should be -16777046");
        }
    #else
        if (tr.getPixel32(2, 2) == 4278190250) {
            DejaGnu.pass("setPixel32(2, 2, 0xff0000aa) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel32(2, 2, 0xff0000aa) set incorrect value: "+tr.getPixel32(2,2)+" should be 4278190250");
        }
    #end

        //// Premultiplication?
        tr.setPixel32(4, 4, 0x44444444);
        //xcheck_equals(tr.getPixel(4, 4), 0x434343);
        if (tr.getPixel(4, 4) == 0x434343) {
            DejaGnu.xpass("setPixel32(4, 4, 0x44444444) has set the correct value - getPixel");
        } else {
            DejaGnu.xfail("setPixel32(4, 4, 0x44444444) set incorrect value: "+tr.getPixel(4,4)+" should be 0x434343");
        }
        //xcheck_equals(tr.getPixel32(4, 4), 0x44434343);
        if (tr.getPixel32(4, 4) == 0x44434343) {
            DejaGnu.xpass("setPixel32(4, 4, 0x44444444) has set the correct value - getPixel32");
        } else {
            DejaGnu.xfail("setPixel32(4, 4, 0x44444444) set incorrect value: "+tr.getPixel32(4,4)+" should be 0x44434343");
        }

        tr.setPixel32(4, 4, 0x10101010);
        //check_equals(tr.getPixel(4, 4), 0x101010);
        if (tr.getPixel(4, 4) == 0x101010) {
            DejaGnu.pass("setPixel32(4, 4, 0x10101010) has set the correct value - getPixel");
        } else {
            DejaGnu.fail("setPixel32(4, 4, 0x10101010) set incorrect value: "+tr.getPixel(4,4)+" should be 0x101010");
        }
        //check_equals(tr.getPixel32(4, 4), 0x10101010);
        if (tr.getPixel32(4, 4) == 0x10101010) {
            DejaGnu.pass("setPixel32(2, 2, 0xff0000aa) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel32(2, 2, 0xff0000aa) set incorrect value: "+tr.getPixel32(2,2)+" should be -16777046");
        }

        //// Premultiplication?
        tr.setPixel32(4, 4, 0x43434343);
        //xcheck_equals(tr.getPixel(4, 4), 0x444444);
        if (tr.getPixel(4, 4) == 0x444444) {
            DejaGnu.xpass("setPixel32(4, 4, 0x43434343) has set the correct value - getPixel");
        } else {
            DejaGnu.xfail("setPixel32(4, 4, 0x43434343) set incorrect value: "+tr.getPixel(4,4)+" should be 0x444444");
        }
        //xcheck_equals(tr.getPixel32(4, 4), 0x43444444);
        if (tr.getPixel32(4, 4) == 0x43444444) {
            DejaGnu.xpass("setPixel32(4, 4, 0x43434343) has set the correct value - getPixel32");
        } else {
            DejaGnu.xfail("setPixel32(4, 4, 0x43434343) set incorrect value: "+tr.getPixel32(4,4)+" should be 0x43444444");
        }
        
        //// Premultiplication?
        tr.setPixel32(2, 2, 0x44);
        //xcheck_equals(tr.getPixel(2, 2), 0x00);
        if (tr.getPixel(2, 2) == 0x00) {
            DejaGnu.xpass("setPixel32(2, 2, 0x44) has set the correct value - getPixel");
        } else {
            DejaGnu.xfail("setPixel32(2, 2, 0x44) set incorrect value: "+tr.getPixel(2,2)+" should be 0x00");
        }
        //xcheck_equals(tr.getPixel32(2, 2), 0);
        if (tr.getPixel32(2, 2) == 0) {
            DejaGnu.xpass("setPixel32(2, 2, 0x44) has set the correct value - getPixel32");
        } else {
            DejaGnu.xfail("setPixel32(2, 2, 0x44) set incorrect value: "+tr.getPixel32(2,2)+" should be 0");
        }
        
        ntr.setPixel32(6, 6, 0x44444444);
        //check_equals(ntr.getPixel(6, 6), 0x444444);
        if (ntr.getPixel(6, 6) == 0x444444) {
            DejaGnu.pass("setPixel32(6, 6, 0x44444444) has set the correct value - getPixel");
        } else {
            DejaGnu.fail("setPixel32(6, 6, 0x44444444) set incorrect value: "+ntr.getPixel(6, 6)+" should be 0x444444");
        }
        //check_equals(ntr.getPixel32(6, 6), -12303292);
    #if flash8
        if (ntr.getPixel32(6, 6) == -12303292) {
            DejaGnu.pass("setPixel32(6, 6, 0x44444444) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel32(6, 6, 0x44444444) set incorrect value: "+ntr.getPixel32(6, 6)+" should be -12303292");
        }
    #else
        if (ntr.getPixel32(6, 6) == 4282664004) {
            DejaGnu.pass("setPixel32(6, 6, 0x44444444) has set the correct value - getPixel32");
        } else {
            DejaGnu.fail("setPixel32(6, 6, 0x44444444) set incorrect value: "+ntr.getPixel32(6, 6)+" should be 4282664004");
        }
    #end
		
		
		// --------------------------------------------------------------------
		// floodFill
		// --------------------------------------------------------------------
        DejaGnu.note("**** floodFill() function testing ****");
        
        
        var bmp = new BitmapData(20, 20, false);
        
        if (untyped __typeof__(bmp.floodFill) == 'function') {
            DejaGnu.pass("BitmapData.floodFill is a function");
        } else {
            DejaGnu.fail("BitmapData.floodfill is not a function");
        }
        
        //NOTE: as of 8/13/09 floodFill is unimplemented in gnash
        bmp.floodFill(10, 10, 0x0000ff00);
        //xcheck_equals(bmp.getPixel(10, 10), 0x0000ff00);
        if (bmp.getPixel(10,10) == 0x0000ff00) {
            DejaGnu.pass("floodFill correctly set the pixel (10, 10) to 0x0000ff00");
        } else {
            DejaGnu.fail("floodFill did not correctly set pixel (10, 10); value is: "+bmp.getPixel(10,10));
        }
        
        bmp.floodFill(5, 5, 0x000000ff);
        //xcheck_equals(bmp.getPixel(10, 0), 0x000000ff);
        if (bmp.getPixel(10,0) == 0x000000ff) {
            DejaGnu.pass("flodFill correctly set the pixel (10, 0) to 0x000000ff");
        } else {
            DejaGnu.fail("floodFill did not correctly set pixel (10, 0); value is: "+bmp.getPixel(10,0));
        }
        
    
    #if flash8
        var depth:Int = flash.Lib.current.getNextHighestDepth();
        //mc = this.createEmptyMovieClip("mc", this.getNextHighestDepth());
        var mc:MovieClip = flash.Lib.current.createEmptyMovieClip("mc_" + depth, depth);
        //mc.attachBitmap(bmp, this.getNextHighestDepth());
        mc.attachBitmap(bmp, depth);
    #else
        var mc:MovieClip = new MovieClip();
        var dispbmp:Bitmap = new Bitmap(bmp);
        mc.addChild(dispbmp);
    #end
        
        // not needed?
        //Rectangle = flash.geom.Rectangle;

        //---------------------------------------------------------------------
        // fillRect()
        //---------------------------------------------------------------------
        DejaGnu.note("**** fillRect() function testing *****");
        
        bmp = new BitmapData(20, 20, false);
        //r = new Rectangle(2, 2, 5, 5);
    #if flash9
		var r:Rectangle = new Rectangle(2, 2, 5, 5);
    #else
		var r:Rectangle<Int> = new Rectangle(2, 2, 5, 5);
    #end
    
        bmp.fillRect(r, 0xff1100);
        //check_equals(bmp.getPixel(1, 1), 0xffffff);
        if (bmp.getPixel(1, 1) == 0xffffff) {
            DejaGnu.pass("fillRect correctly set pixel (1,1) to 0xffffff");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (1,1); value is: "+bmp.getPixel(1,1));
        }
        //check_equals(bmp.getPixel(2, 2), 0xff1100);
        if (bmp.getPixel(2, 2) == 0xff1100) {
            DejaGnu.pass("fillRect correctly set pixel (2,2) to 0xff1100");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (2,2); value is: "+bmp.getPixel(2,2));
        }
        //check_equals(bmp.getPixel(2, 5), 0xff1100);
        if (bmp.getPixel(2, 5) == 0xff1100) {
            DejaGnu.pass("fillRect correctly set pixel (2,5) to 0xff1100");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (2,5); value is: "+bmp.getPixel(2,5));
        }
        //check_equals(bmp.getPixel(5, 2), 0xff1100);
        if (bmp.getPixel(5, 2) == 0xff1100) {
            DejaGnu.pass("fillRect correctly set pixel (5,2) to 0xff1100");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (5,2); value is: "+bmp.getPixel(5,2));
        }
        //check_equals(bmp.getPixel(2, 6), 0xff1100);
        if (bmp.getPixel(2, 6) == 0xff1100) {
            DejaGnu.pass("fillRect correctly set pixel (2,6) to 0xff1100");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (2,6); value is: "+bmp.getPixel(2,6));
        }
        //check_equals(bmp.getPixel(6, 6), 0xff1100);
        if (bmp.getPixel(6, 6) == 0xff1100) {
            DejaGnu.pass("fillRect correctly set pixel (6,6) to 0xff1100");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (6,6); value is: "+bmp.getPixel(6,6));
        }
        //check_equals(bmp.getPixel(6, 7), 0xffffff);
        if (bmp.getPixel(6, 7) == 0xffffff) {
            DejaGnu.pass("fillRect correctly set pixel (6,7) to 0xffffff");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (6,7); value is: "+bmp.getPixel(6,7));
        }
        //check_equals(bmp.getPixel(7, 6), 0xffffff);
        if (bmp.getPixel(7, 6) == 0xffffff) {
            DejaGnu.pass("fillRect correctly set pixel (7,6) to 0xffffff");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (7,6); value is: "+bmp.getPixel(7,6));
        }

        r = new Rectangle(-2, -2, 8, 8);
        bmp.fillRect(r, 0x00ff00);
        //check_equals(bmp.getPixel(1, 1), 0x00ff00);
        if (bmp.getPixel(1, 1) == 0x00ff00) {
            DejaGnu.pass("fillRect correctly set pixel (1,1) to 0x00ff00");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (1,1); value is: "+bmp.getPixel(1,1));
        }

        //// Fails.
        r = new Rectangle(18, 18, -4, -4);
        bmp.fillRect(r, 0x0000ff);
        //check_equals(bmp.getPixel(7, 6), 0xffffff);
        if (bmp.getPixel(7, 6) == 0xffffff) {
            DejaGnu.pass("fillRect correctly set pixel (7,6) to 0xffffff");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (7,6); value is: "+bmp.getPixel(7,6));
        }

        r = new Rectangle(18, 18, 200, 200);
        bmp.fillRect(r, 0x0000ff);
        //check_equals(bmp.getPixel(19,19), 0x0000ff);
        if (bmp.getPixel(19, 19) == 0x0000ff) {
            DejaGnu.pass("fillRect correctly set pixel (19,19) to 0x0000ff");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (19,19); value is: "+bmp.getPixel(19,19));
        }

        //// Doesn't have to be a rectangle
        // This doesn't seem to be true in flash 9 - throws an error
        //g = {x: 15, y: 15, width: 2, height: 2};
        var g = {x: 15, y: 15, width: 2, height: 2};
    #if flash8
        bmp.fillRect(untyped g, 0xff00ff);
        //check_equals(bmp.getPixel(16, 16), 0xff00ff);
        if (bmp.getPixel(16, 16) == 0xff00ff) {
            DejaGnu.pass("fillRect correctly set pixel (16,16) to 0xff00ff");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (16,16); value is: "+bmp.getPixel(16,16));
        }
    #else
        try {
            bmp.fillRect(untyped g, 0xff00ff);
            DejaGnu.fail("fillRect(g, 0xff0ff) did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("fillRect(g, 0xff0ff) correctly threw an error, id: " + e.errorID);
            if ( e.name == 'TypeError') {
                DejaGnu.pass("fillRect correctly threw 'TypeError' id: " + e.message);
            } else {
                DejaGnu.fail("fillRect did not throw 'TypeError'; error caught: " + e);
            }
        }
    #end

    #if flash8
        //// Transparency (this bitmap is not transparent).
        //g = {x: 18, y: 2, width: 7, height: 7};
        g = {x: 18, y: 2, width: 7, height: 7};
        bmp.fillRect(untyped g, 0xddff00ff);
        //check_equals(bmp.getPixel32(18, 2), -65281);
        if (bmp.getPixel32(18, 2) == -65281) {
            DejaGnu.pass("fillRect correctly set pixel (18,2) to -65281");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (18,2); value is: "+bmp.getPixel32(18,2));
        }
    #end
        
    #if flash8
        mc.attachBitmap(bmp, depth);
    #else
        dispbmp = new Bitmap(bmp);
        mc.addChild(dispbmp);
    #end

        //// Transparency (transparent bitmap). Fill just obliterates
        //// what was there, even if it's transparent.
        bmp = new BitmapData(20, 20, true);
        r = new Rectangle(1, 1, 10, 10);
        bmp.fillRect(r, 0xff00ff00);
        r = new Rectangle(2, 2, 9, 9);
        bmp.fillRect(r, 0x99ff1100);
    #if flash8
        //check_equals(bmp.getPixel32(3, 3), -1711337216);
        if (bmp.getPixel32(3, 3) == -1711337216) {
            DejaGnu.pass("fillRect correctly set pixel (3,3) to -1711337216");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (3,3); value is: "+bmp.getPixel32(3,3));
        }
    #else
        //precission difference
        if (bmp.getPixel32(3, 3) == 2583630080) {
            DejaGnu.pass("fillRect correctly set pixel (3,3) to 2583630080");
        } else {
            DejaGnu.fail("fillRect incorrectly set pixel (3,3); value is: "+bmp.getPixel32(3,3));
        }
    #end

    #if flash8
        mc.attachBitmap(bmp, depth);
    #else
        dispbmp = new Bitmap(bmp);
        mc.addChild(dispbmp);
    #end

        //----------------------------------------------------------------------
        // dispose()
        //----------------------------------------------------------------------
        DejaGnu.note("**** dispose() function testing ****");
        
        bmp.dispose();
        
        //check(bmp instanceOf Bitmap);
        if (Std.is(bmp, BitmapData) ) {
            DejaGnu.pass("dispose() correctly did not change the type of bmp");
        } else {
            DejaGnu.fail("dispose() incorrectly changed"+
                         " the type of bmp to: "+ untyped __typeof__(bmp) );
        }
    #if flash8
        //check_equals(bmp.height, -1);
        if (bmp.height == -1) {
            DejaGnu.pass("BitmapData.dispose correctly set bmp.height to -1");
        } else {
            DejaGnu.fail("dispose() did not correctly"+
                         " set bmp.height; value is: "+bmp.height);
        }
        //check_equals(bmp.width, -1);
        if (bmp.width == -1) {
            DejaGnu.pass("BitmapData.dispose correctly set bmp.width to -1");
        } else {
            DejaGnu.fail("dispose() did not correctly"+
                         " set bmp.width; value is: "+bmp.width);
        }
        //check_equals(bmp.transparent, -1);
        if (bmp.transparent == untyped -1) {
            DejaGnu.pass("BitmapData.dispose correctly set bmp.transparent to -1");
        } else {
            DejaGnu.fail("dispose() did not correctly"+
                         " set bmp.transparent; value is: "+bmp.transparent);
        }
        //check_equals(typeof(bmp.rectangle), "number");
        if (untyped __typeof__(bmp.rectangle) == 'number') {
            DejaGnu.pass("BitmapData.dispose correctly changed bmp.rectangle"+
                        " to a number");
        } else {
            DejaGnu.fail("dispose() did not correctly"+
                         "change bmp.rectangle; type is: "+
                         untyped __typeof__(bmp.rectangle) );
        }
        //check_equals(bmp.rectangle, -1);
        if (bmp.rectangle == untyped -1) {
            DejaGnu.pass("BitmapData.dispose correctly set bmp.rectangle to -1");
        } else {
            DejaGnu.fail("dispose() did not correctly"+
                         " set bmp.rectangle; value is: "+bmp.rectangle);
        }
        //check_equals(bmp.rectangle.toString(), "-1");
        if ( bmp.rectangle.toString() == "-1") {
            DejaGnu.pass("dispose() correctly set bmp.rectangle.toString()"+
                         " to '-1'");
        } else {
            DejaGnu.fail("dispose() did not correctly"+
                         " set bmp.rectangle.toString(); value is: "
                         + bmp.rectangle.toString() );
        }
        
        bmp.height = 2;
        //check_equals(bmp.height, -1);
        if (bmp.height == -1) {
            DejaGnu.pass("After calling dispose() we cannot change bmp.height");
        } else {
            DejaGnu.fail("After calling dispose() we"+
                        " were still able to change bmp.height which is not"+
                        " allowed");
        }
    #else
        var x1;
        //check_equals(bmp.height, -1);
        try {
            x1 = bmp.height;
            DejaGnu.fail("After dispose() getting"+
                         " bmp.height did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("After dispose() getting bmp.height correctly threw"+
                         " an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("getting bmp.height correctly threw"+
                             " 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("getting bmp.height did"+
                             " not throw 'ArgumentError'; error caught: " + e);
            }
        }
        //check_equals(bmp.width, -1);
        try {
            x1 = bmp.width;
            DejaGnu.fail("After dispose() getting"+
                         " bmp.width did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("After dispose() getting bmp.width correctly threw"+
                         " an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("getting bmp.width correctly threw"+
                             " 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("getting bmp.width did"+
                             " not throw 'ArgumentError'; error caught: " + e);
            }
        }
        //check_equals(bmp.transparent, -1);
        var x2;
        try {
            x2 = bmp.transparent;
            DejaGnu.fail("After dispose() getting"+
                         " bmp.transparent did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("After dispose() getting bmp.transparent correctly"+
                         " threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("getting bmp.transparent correctly threw"+
                             " 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("getting bmp.transparent"+
                             " did not throw 'ArgumentError'; error caught: "
                             + e );
            }
        }
        //check_equals(bmp.rectangle, -1);
        var x3;
        try {
            x3 = bmp.rect;
            DejaGnu.fail("After dispose() getting"+
                         " bmp.rect did not throw an error, it should");
        } catch( e : Error ) {
            DejaGnu.pass("After dispose() getting bmp.rect correctly"+
                         " threw an error, id: " + e.errorID);
            if ( e.name == 'ArgumentError') {
                DejaGnu.pass("getting bmp.rect correctly threw"+
                             " 'ArgumentError' id: " + e.message);
            } else {
                DejaGnu.fail("getting bmp.rect"+
                             " did not throw 'ArgumentError'; error caught: "
                             + e );
            }
        }

    #end
        
        DejaGnu.unresolved("Skipping some ming tests");
        DejaGnu.note("Skipping some very strange tests from the ming test cases");
        DejaGnu.note("Haxe will not allow assignment to flash.geom.Rectangle");
        DejaGnu.note("Does flash allow this sort of thing and should we"+ 
                     " allow this in Gnash?");
        
        //bmp = new BitmapData(20, 10, true);
        //var backup = flash.geom.Rectangle;
        //flash.geom.Rectangle = 2;
        //check_equals(bmp.rectangle, -1);

        //flash.geom.Rectangle = function (x, y, w, h)
        //{
            //this.y = x + 5;
            //this.x = 10.5;
            //this.width = h;
            //this.height = w;
        //};
        //check_equals(bmp.rectangle.toString(), "[object Object]");

        //flash.geom.Rectangle = function (x, y, w, h)
        //{
        //};
        //check_equals(bmp.rectangle.toString(), "[object Object]");

        //flash.geom.Rectangle = function ()
        //{
        //};
        //check_equals(bmp.rectangle.toString(), "[object Object]");

        //flash.geom.Rectangle = backup;
        //check_equals(bmp.rectangle.toString(), "(x=0, y=0, w=20, h=10)");
        
        
        
        

		//-------------------------------------------------------------
		// END OF TEST
		//-------------------------------------------------------------

#else
		DejaGnu.note("This class (BitmapData) is only available in flash8 and flash9");
#end
		// Call this after finishing all tests. It prints out the totals.
		DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

