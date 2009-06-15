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
import flash.display.Bitmap;
#end
#if flash8
import flash.MovieClip;
#end
#if (flash8 || flash9)
import flash.geom.Rectangle;
import flash.display.BitmapData;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class BitmapData_as {
    static function main() {
#if !(flash6 || flash7)
		if(Type.typeof(untyped BitmapData) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData is a function");
		} else {
			DejaGnu.fail("BitmapData is not a function");
		}
		if(Type.typeof(untyped BitmapData.prototype) == ValueType.TObject) {
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
			DejaGnu.fail("BitmapData prototype does not have 'noise' property");
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
		if(untyped BitmapData.prototype.hasOwnProperty("height")) {
			DejaGnu.pass("BitmapData prototype has 'height' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'height' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty("width")) {
			DejaGnu.pass("BitmapData prototype has 'width' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'width' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty("rectangle")) {
			DejaGnu.pass("BitmapData prototype has 'rectangle' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'rectangle' property");
		}
		if(untyped BitmapData.prototype.hasOwnProperty("transparent")) {
			DejaGnu.pass("BitmapData prototype has 'transparent' property");
		} else {
			DejaGnu.fail("BitmapData prototype does not have 'transparent' property");
		}

		if(!(untyped BitmapData.prototype.hasOwnProperty('loadBitmap'))) {
			DejaGnu.pass("BitmapData prototype does not have 'loadBitmap' property");
		} else {
			DejaGnu.fail("BitmapData prototype has 'loadBitmap' property");
		}
		if(untyped BitmapData.hasOwnProperty("loadBitmap")) {
			DejaGnu.pass("BitmapData has 'loadBitmap' property");
		} else {
			DejaGnu.fail("BitmapData does not have 'loadBitmap' property");
		}

		//-------------------------------------------------------------
		// Test constructor
		//-------------------------------------------------------------

		var x1:BitmapData = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), []);
		if(Type.typeof(x1) == ValueType.TNull) {
			DejaGnu.pass("Type of x1 is undefined");
		} else {
			DejaGnu.fail("Type of x1 should be undefined, is "+Type.typeof(x1));
		}
		
		//FIXME: I simply don't understand this test series. If someone would enlighten me
		//as to the function of the hasOwnProperty method, I would gladly listen.
		
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [10, 10]);
		//if(Type.typeof(x1) == ValueType.TObject) {
			//DejaGnu.pass("x1 is an object");
		//} else {
			//DejaGnu.fail("x1 should be an object, is "+Type.typeof(x1));
		//}
		//if(Std.is(x1, BitmapData)) {
			//DejaGnu.pass("x1 is a BitmapData");
		//} else {
			//DejaGnu.fail("x1 should be a BitmapData, is "+Type.getClassName(Type.getClass(x1)));
		//}
		//if(!untyped x1.hasOwnProperty("height")) {
			//DejaGnu.pass("x1 does not have property 'height'");
		//} else {
			//DejaGnu.fail("x1 has property 'height'");
		//}
		//if(!untyped x1.hasOwnProperty("width")) {
			//DejaGnu.pass("x1 does not have property 'width'");
		//} else {
			//DejaGnu.fail("x1 has property 'width'");
		//}
		//if(!untyped x1.hasOwnProperty("rectangle")) {
			//DejaGnu.pass("x1 does not have property 'rectangle'");
		//} else {
			//DejaGnu.fail("x1 has property 'rectangle'");
		//}
		//if(!untyped x1.hasOwnProperty("transparent")) {
			//DejaGnu.pass("x1 does not have property 'transparent'");
		//} else {
			//DejaGnu.fail("x1 has property 'transparent'");
		//}
		//if(x1.height == 10) {
			//DejaGnu.pass("x1.height == 10");
		//} else {
			//DejaGnu.fail("x1.height != 10");
		//}
		//if(x1.width == 10) {
			//DejaGnu.pass("x1.width == 10");
		//} else {
			//DejaGnu.fail("x1.width != 10");
		//}
		//if(x1.transparent) {
			//DejaGnu.pass("x1.transparent is true");
		//} else {
			//DejaGnu.fail("x1.transparent is false");
		//}
//#if flash9
		//if(x1.rect.toString() == "(x=0, y=0, w=10, h=10)") {
			//DejaGnu.pass("x1.rectangle is correct rectangle (x=0, y=0, w=10, h=10)");
		//} else {
			//DejaGnu.fail("x1.rectangle should be (x=0, y=0, w=10, h=10), is "+x1.rect.toString());
		//}
		//if(Std.is(x1.rect, Rectangle)) {
			//DejaGnu.pass("x1.rectangle is a Rectangle");
		//} else {
			//DejaGnu.fail("x1.rectangle is not a Rectangle, it is a "+Type.typeof(x1.rect));
		//}
//#else
		//if(x1.rectangle.toString() == "(x=0, y=0, w=10, h=10)") {
			//DejaGnu.pass("x1.rectangle is correct rectangle (x=0, y=0, w=10, h=10)");
		//} else {
			//DejaGnu.fail("x1.rectangle should be (x=0, y=0, w=10, h=10), is "+x1.rectangle.toString());
		//}
		//if(Std.is(x1.rectangle, Rectangle)) {
			//DejaGnu.pass("x1.rectangle is a Rectangle");
		//} else {
			//DejaGnu.fail("x1.rectangle is not a Rectangle, it is a "+Type.typeof(x1.rectangle));
		//}
//#end
		//if(x1.getPixel(1,1) == 16777215) {
			//DejaGnu.pass("x1.getPixel(1,1) returns correct number (16777215)");
		//} else {
			//DejaGnu.fail("x1.getPixel(1,1) should return 16777215, returns "+x1.getPixel(1,1));
		//}
		//if(x1.getPixel(9,9) == 16777215) {
			//DejaGnu.pass("x1.getPixel(9,9) returns correct number (16777215)");
		//} else {
			//DejaGnu.fail("x1.getPixel(9,9) should return 16777215, returns "+x1.getPixel(9,9));
		//}
		//if(x1.getPixel32(1,1) == -1) {
			//DejaGnu.pass("x1.getPixel32(1,1) returns correct number (-1)");
		//} else {
			//DejaGnu.fail("x1.getPixel32(1,1) should return -1, returns "+x1.getPixel32(1,1));
		//}

		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [10, 10, true]);
		//if(x1.getPixel32(1,1) == -1) {
			//DejaGnu.pass("x1.getPixel32(1,1) returns correct number (-1)");
		//} else {
			//DejaGnu.fail("x1.getPixel32(1,1) should return -1, returns "+x1.getPixel32(1,1));
		//}
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [10, 10, false]);
		//if(x1.getPixel32(1,1) == -1) {
			//DejaGnu.pass("x1.getPixel32(1,1) returns correct number (-1)");
		//} else {
			//DejaGnu.fail("x1.getPixel32(1,1) should return -1, returns "+x1.getPixel32(1,1));
		//}
		
        x1 = new BitmapData(20, 30, false, 0xeeddee);
        
        // check(x1 instanceof Bitmap);      
        if (Std.is(x1,BitmapData)) {
            DejaGnu.pass("BitmapData class exists");
        } else {
            DejaGnu.fail("BitmapData lass doesn't exist");
        }

		//check_equals(x1.height, 10);
		if (Type.typeof(x1.height) == ValueType.TInt) {
			DejaGnu.pass("BitmapData::height property exists");
			if (x1.height == 30) {
				DejaGnu.pass("BitmapData.height property is correct int (30)");
			} else {
				DejaGnu.fail("BitmapData.height property is incorrect int (should be 30, is "+x1.height+")");
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
				DejaGnu.fail("BitmapData.width property is incorrect int (should be 20, is "+x1.width+")");
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
				DejaGnu.fail("BitmapData.transparent property is incorrect bool (should be false, is "+x1.transparent+")");
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
				DejaGnu.fail(
				"BitmapData.rectangle property is incorrect rectangle (should be (x=0, y=0, w=20, h=30), is "+x1.rectangle.toString()+")");
			}
		} else {
			DejaGnu.fail("BitmapData::rectangle property doesn't exist");
		}
#end

		//check_equals(bmp.getPixel(1, 1), 0xeeddee);
		if (Type.typeof(x1.getPixel) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::getPixel() method exists");
			if (Type.typeof(x1.getPixel(1,1)) == ValueType.TInt) {
				DejaGnu.pass("BitmapData::getPixel() method returns correct type (number)");
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
			} else {
				DejaGnu.fail("BitmapData::getPixel() method returns incorrect type (should be number, is "+Type.typeof(x1.getPixel(1,1))+")");
			}
		} else {
			DejaGnu.fail("BitmapData::getPixel() method doesn't exist");
		}
		//check_equals(bmp.getPixel32(1, 1), -1122834);
		if (Type.typeof(x1.getPixel32) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::getPixel32() method exists");
			//getPixel32() should return a float in AS2, and a uint in AS3.
#if !flash9
			if (Type.typeof(x1.getPixel32(1,1)) == ValueType.TFloat) {
				DejaGnu.xpass("BitmapData::getPixel32() method returns correct type (number)");
#else
			if (Type.typeof(x1.getPixel32(1,1)) == ValueType.TFloat) {
				DejaGnu.pass("BitmapData::getPixel32() method returns correct type (int)");
#end			
				if (x1.getPixel32(1,1) == -1122834) {
					DejaGnu.pass("BitmapData::getPixel32() method returns correct number (-01122834)");
				} else {
					DejaGnu.fail("BitmapData::getPixel32() method returns incorrect number (should be -01122834, is "+x1.getPixel32(1,1)+")");
				}
			} else {
				DejaGnu.xfail("BitmapData::getPixel32() method returns incorrect type (should be number, is "+Type.typeof(x1.getPixel32(1,1))+")");
			}
		} else {
			DejaGnu.fail("BitmapData::getPixel32() method doesn't exist");
		}
		
		//RESUME HERE***********************************************************************************************************
		////bmp = new Bitmap(10000, 3);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [10000, 3]);
		////xcheck_equals(typeof(bmp), "undefined");
		//if(Type.typeof(x1) == ValueType.TNull) {
			//DejaGnu.xpass("Type of x1 is undefined");
		//} else {
			//DejaGnu.xfail("Type of x1 should be undefined, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, undefined);
		//if(x1.height == null) {
			//DejaGnu.pass("x1.height is undefined");
		//} else {
			//DejaGnu.fail("x1.height should be undefined, is "+x1.height);
		//}

		////bmp = new Bitmap(0, 10000);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [0, 10000]);
		////xcheck_equals(typeof(bmp), "undefined");
		//if(Type.typeof(x1) == ValueType.TNull) {
			//DejaGnu.xpass("Type of x1 is undefined");
		//} else {
			//DejaGnu.xfail("Type of x1 should be undefined, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, undefined);
		//if(x1.height == null) {
			//DejaGnu.pass("x1.height is undefined");
		//} else {
			//DejaGnu.fail("x1.height should be undefined, is "+x1.height);
		//}

		////bmp = new Bitmap(2880, 2880);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [2880, 2880]);
		////check_equals(typeof(bmp), "object");
		//if(Type.typeof(x1) == ValueType.TObject) {
			//DejaGnu.xpass("Type of x1 is object");
		//} else {
			//DejaGnu.xfail("Type of x1 should be object, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, 2880);
		//if(x1.height == 2880) {
			//DejaGnu.pass("x1.height is 2880");
		//} else {
			//DejaGnu.fail("x1.height should be 2880, is "+x1.height);
		//}

		////bmp = new Bitmap(2880, 2881);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [2880, 2881]);
		////xcheck_equals(typeof(bmp), "undefined");
		//if(Type.typeof(x1) == ValueType.TNull) {
			//DejaGnu.xpass("Type of x1 is undefined");
		//} else {
			//DejaGnu.xfail("Type of x1 should be undefined, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, undefined);
		//if(x1.height == null) {
			//DejaGnu.pass("x1.height is undefined");
		//} else {
			//DejaGnu.fail("x1.height should be undefined, is "+x1.height);
		//}

		////bmp = new Bitmap(0, 2880);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [0, 2880]);
		////xcheck_equals(typeof(bmp), "undefined");
		//if(Type.typeof(x1) == ValueType.TNull) {
			//DejaGnu.xpass("Type of x1 is undefined");
		//} else {
			//DejaGnu.xfail("Type of x1 should be undefined, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, undefined);
		//if(x1.height == null) {
			//DejaGnu.pass("x1.height is undefined");
		//} else {
			//DejaGnu.fail("x1.height should be undefined, is "+x1.height);
		//}

		////bmp = new Bitmap(2879, 2879);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [2879, 2879]);
		////check_equals(typeof(bmp), "object");
		//if(Type.typeof(x1) == ValueType.TObject) {
			//DejaGnu.xpass("Type of x1 is object");
		//} else {
			//DejaGnu.xfail("Type of x1 should be object, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, 2879);
		//if(x1.height == 2879) {
			//DejaGnu.pass("x1.height is 2879");
		//} else {
			//DejaGnu.fail("x1.height should be 2879, is "+x1.height);
		//}

		////bmp = new Bitmap(0, 2879);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [0, 2879]);
		////xcheck_equals(typeof(bmp), "undefined");
		//if(Type.typeof(x1) == ValueType.TNull) {
			//DejaGnu.xpass("Type of x1 is undefined");
		//} else {
			//DejaGnu.xfail("Type of x1 should be undefined, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, undefined);
		//if(x1.height == null) {
			//DejaGnu.pass("x1.height is undefined");
		//} else {
			//DejaGnu.fail("x1.height should be undefined, is "+x1.height);
		//}

		////bmp = new Bitmap(-1, 10, false, 0xff);
		//x1 = Reflect.callMethod(BitmapData, Reflect.field(BitmapData, 'new'), [-1, 10, false, 0xff]);
		////xcheck_equals(typeof(bmp), "undefined");
		//if(Type.typeof(x1) == ValueType.TNull) {
			//DejaGnu.xpass("Type of x1 is undefined");
		//} else {
			//DejaGnu.xfail("Type of x1 should be undefined, is "+Type.typeof(x1));
		//}
		////check_equals(bmp.height, undefined);
		//if(x1.height == null) {
			//DejaGnu.pass("x1.height is undefined");
		//} else {
			//DejaGnu.fail("x1.height should be undefined, is "+x1.height);
		//}
		
		// --------------------
		// setPixel, setPixel32
		// --------------------
		
		if (Type.typeof(x1.setPixel) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::setPixel() method exists");
			//x1.setPixel32(2, 2, 0x44);
			if (Type.typeof(x1.setPixel(2,2,0x44)) == ValueType.TNull) {
				DejaGnu.pass("BitmapData::getPixel() method returns correct type (Void)");
				//xcheck_equals(x1.getPixel(2, 2), 0x00);
				if (!(x1.getPixel(2,2) == 0x00)) {
					DejaGnu.pass("BitmapData::setPixel(2,2,0x44) has not set an incorrect value (should not be 0x00, is "+x1.getPixel(2,2)+")");
				} else {
					DejaGnu.fail("BitmapData::setPixel(2,2,0x44) has set an incorrect value (0x00)");
				}
				//xcheck_equals(x1.getPixel32(2, 2), 0);
				if (!(x1.getPixel32(2,2) == 0)) {
					DejaGnu.pass("BitmapData::setPixel(2,2,0x44) has not set an incorrect value (should not be 0, is "+x1.getPixel32(2,2)+")");
				} else {
					DejaGnu.fail("BitmapData::setPixel(2,2,0x44) has set an incorrect value (0)");
				}
				x1.setPixel(3, 3, 0xff);
				//check_equals(tr.getPixel(3, 3), 0xff);
				if (x1.getPixel(3,3) == 0xff) {
					DejaGnu.pass("BitmapData::setPixel(3,3,0xff) has set the correct value (0xff)");
				} else {
					DejaGnu.fail("BitmapData::setPixel(3,3,0xff) has set an incorrect value (should be 0xff, is "+x1.getPixel(2,2)+")");
				}
				//check_equals(tr.getPixel32(3, 3), -16776961);
				if ((x1.getPixel32(3,3) == -16776961)) {
					DejaGnu.pass("BitmapData::setPixel(3,3,0xff) has set the correct value (-16776961)");
				} else {
					DejaGnu.fail("BitmapData::setPixel(3,3,0xff) has set an incorrect value (should be -16776961, is "+x1.getPixel32(2,2)+")");
				}
			} else {
				DejaGnu.fail("BitmapData::setPixel() method returns incorrect type (should be Void, is "+Type.typeof(x1.setPixel(2,2,0x44))+")");
			}
		} else {
			DejaGnu.fail("BitmapData::setPixel() method doesn't exist");
		}
		if (Type.typeof(x1.setPixel32) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::setPixel32() method exists");
			//x1.setPixel32(6, 6, 0x44444444);
			if (Type.typeof(x1.setPixel32(6,6,0x44444444)) == ValueType.TNull) {
				DejaGnu.pass("BitmapData::setPixel32() method returns correct type (Void)");
				//check_equals(ntr.getPixel(6, 6), 0x444444);
				if ((x1.getPixel(6,6) == 0x444444)) {
					DejaGnu.pass("BitmapData::setPixel32(6,6,0x44444444) has set a correct value (0x444444)");
				} else {
					DejaGnu.fail(
					"BitmapData::setPixel32(6,6,0x44444444) has set an incorrect value (should be 0x444444, is "+x1.getPixel(6,6)+")");
				}
				//check_equals(ntr.getPixel32(6, 6), -12303292);
				if ((x1.getPixel32(6,6) == -12303292)) {
					DejaGnu.pass("BitmapData::setPixel32(6,6,0x44444444) has set a correct value (-12303292)");
				} else {
					DejaGnu.fail(
					"BitmapData::setPixel32(6,6,0x44444444) has set an incorrect value (should be -12303292, is "+x1.getPixel32(6,6)+")");
				}
			} else {
				DejaGnu.fail("BitmapData::setPixel32() method returns incorrect type (should be Void, is "+Type.typeof(x1.setPixel(2,2,0x44))+")");
			}
		} else {
			DejaGnu.fail("BitmapData::setPixel32() method doesn't exist");
		}
		
		// ---------
		// floodFill
		// ---------
		
		//bmp.floodFill(10, 10, 0x0000ff00);
		if (Type.typeof(x1.floodFill) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::floodFill() method exists");
			if (Type.typeof(x1.floodFill(10,10,0x0000ff00)) == ValueType.TNull) {
				DejaGnu.pass("BitmapData::floodFill() method returns correct type (Void)");
				//xcheck_equals(bmp.getPixel(10, 10), 0x0000ff00);
				if (!(x1.getPixel(10,10) == 0x0000ff00)) {
					DejaGnu.pass(
					"BitmapData::floodFill(10,10,0x0000ff00) has not set an incorrect value (should not be 0x0000ff00, is "+x1.getPixel(10,10)+")");
				} else {
					DejaGnu.fail(
					"BitmapData::floodFill(10,10,0x0000ff00) has set an incorrect value (0x0000ff00)");
				}
				x1.floodFill(5,5,0x000000ff);
				//xcheck_equals(bmp.getPixel(10, 0), 0x000000ff);
				if (!(x1.getPixel(10,10) == 0x000000ff)) {
					DejaGnu.pass(
					"BitmapData::floodFill(5,5,0x000000ff) has not set an incorrect value (should not be 0x000000ff, is "+x1.getPixel(6,6)+")");
				} else {
					DejaGnu.fail(
					"BitmapData::floodFill(5,5,0x000000ff) has set an incorrect value (0x000000ff)");
				}
			} else {
				DejaGnu.fail(
				"BitmapData::floodFill() method returns incorrect type (should be Void, is "+Type.typeof(x1.floodFill(10,10,0x0000ff00))+")");
			}
		} else {
			DejaGnu.fail("BitmapData::floodFill() method doesn't exist");
		}
		#if flash9
		var r:Rectangle = new Rectangle(2, 2, 5, 5);
#else
		var r:Rectangle<Int> = new Rectangle(2, 2, 5, 5);
#end
		if (Type.typeof(x1.fillRect) == ValueType.TFunction) {
			DejaGnu.pass("BitmapData::fillRect() method exists");
			if (Type.typeof(x1.fillRect(r, 0xff1100)) == ValueType.TNull) {
				DejaGnu.pass("BitmapData::fillRect() method returns correct type (Void)");
				//check_equals(bmp.getPixel(1, 1), 0xffffff);
				if (x1.getPixel(1,1) == 0xeeddee) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has not set an incorrect value @ (1,1) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (1,1) (should be 0xeeddee, is "+x1.getPixel(1,1)+")");
				}
				//check_equals(bmp.getPixel(2, 2), 0xff1100);
				if (x1.getPixel(2,2) == 0xff1100) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has set a correct value @ (2,2) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (2,2) (should be 0xff1100, is "+x1.getPixel(2,2)+")");
				}
				//check_equals(bmp.getPixel(2, 5), 0xff1100);
				if (x1.getPixel(2,5) == 0xff1100) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has set a correct value @ (2,5) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (2,5) (should be 0xff1100, is "+x1.getPixel(2,5)+")");
				}
				//check_equals(bmp.getPixel(5, 2), 0xff1100);
				if (x1.getPixel(5,2) == 0xff1100) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has set a correct value @ (5,2) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (5,2) (should be 0xff1100, is "+x1.getPixel(5,2)+")");
				}
				//check_equals(bmp.getPixel(2, 6), 0xff1100);
				if (x1.getPixel(2,6) == 0xff1100) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has set a correct value @ (2,6) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (2,6) (should be 0xff1100, is "+x1.getPixel(2,6)+")");
				}
				//check_equals(bmp.getPixel(6, 6), 0xff1100);
				if (x1.getPixel(6,6) == 0xff1100) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has set a correct value @ (6,6) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (6,6) (should be 0xff1100, is "+x1.getPixel(6,6)+")");
				}
				//check_equals(bmp.getPixel(6, 7), 0xffffff);
				if (x1.getPixel(6,7) == 0xeeddee) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has not set an incorrect value @ (6,7) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (6,7) (should be 0xeeddee, is "+x1.getPixel(1,1)+")");
				}
				//check_equals(bmp.getPixel(7, 6), 0xffffff);
				if (x1.getPixel(7,6) == 0xeeddee) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0xff1100) has not set an incorrect value @ (7,6) (0xff1100)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0xff1100) has set an incorrect value @ (7,6) (should be 0xeeddee, is "+x1.getPixel(1,1)+")");
				}
				r = new Rectangle(-2, -2, 8, 8);
				x1.fillRect(r, 0x00ff00);
				//check_equals(bmp.getPixel(1, 1), 0x00ff00);
				if (x1.getPixel(1,1) == 0x00ff00) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0x00ff00) has set a correct value @ (1,1) (0x00ff00)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0x00ff00) has set an incorrect value @ (1,1) (should be 0x00ff00, is "+x1.getPixel(1,1)+")");
				}
				r = new Rectangle(18, 18, -4, -4);
				x1.fillRect(r, 0x0000ff);
				//check_equals(bmp.getPixel(7, 6), 0xeeddee);
				if (x1.getPixel(7,6) == 0xeeddee) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0x0000ff) has not set an incorrect value @ (7,6) (0x0000ff)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0x0000ff) has set an incorrect value @ (7,6) (should be 0xeeddee, is "+x1.getPixel(7,6)+")");
				}
				r = new Rectangle(18, 18, 200, 200);
				x1.fillRect(r, 0x0000ff);
				//check_equals(bmp.getPixel(19,19), 0x0000ff);
				if (x1.getPixel(19,19) == 0x0000ff) {
					DejaGnu.pass(
					"BitmapData::fillRect(r, 0x0000ff) has set a correct value @ (19,19) (0x0000ff)");
				} else {
					DejaGnu.fail(
					"BitmapData::fillRect(r, 0x0000ff) has set an incorrect value @ (19,19) (should be 0x0000ff, is "+x1.getPixel(19,19)+")");
				}
				// Doesn't have to be a rectangle
				//var g = {x: 15, y: 15, width: 2, height: 2};
				//bmp.fillRect(g, 0xff00ff);
				//check_equals(bmp.getPixel(16, 16), 0xff00ff);
			} else {
				DejaGnu.fail(
				"BitmapData::fillRect() method returns incorrect type (should be Void, is "+Type.typeof(x1.floodFill(10,10,0x0000ff00))+")");
			}
		} else {
			DejaGnu.fail("BitmapData::fillRect() method doesn't exist");
		}
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

		//-------------------------------------------------------------
		// END OF TEST
		//-------------------------------------------------------------

		// Call this after finishing all tests. It prints out the totals.
		DejaGnu.done();
#else
		DejaGnu.note("This class (BitmapData) is only available in flash8 and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

