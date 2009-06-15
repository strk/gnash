// DisplayObject_as.hx:  ActionScript 3 "DisplayObject" class, for Gnash.
//
// Generated on: 20090529 by "bnaugle". Remove this
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
import flash.accessibility.AccessibilityProperties;
import flash.display.Loader;
import flash.display.DisplayObjectContainer;
import flash.display.DisplayObject;
import flash.display.MovieClip;
import flash.display.Shape;
import flash.display.LoaderInfo;
import flash.display.Sprite;
import flash.display.Stage;
import flash.geom.Rectangle;
import flash.geom.Transform;
import flash.net.URLRequest;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class DisplayObject_as {
    static function main() {
#if flash9
        var x1:Shape = new Shape();
        var l1:DisplayObject = new Loader();
        var r1:Rectangle = new Rectangle(1,1,1,1);

        // Make sure we actually get a valid class        
        if (Std.is(x1, DisplayObject)) {
            DejaGnu.pass("DisplayObject class exists");
        } else {
            DejaGnu.fail("DisplayObject lass doesn't exist");
        }
        
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
//Determine if the accessibilityProperties property of x1 exists by setting it and testing it
        var accessProps:AccessibilityProperties = new AccessibilityProperties();
		accessProps.name = "Test";
		x1.accessibilityProperties = accessProps;
 	if (Std.is(x1.accessibilityProperties, AccessibilityProperties)) {
 	    DejaGnu.pass("DisplayObject::accessibilityProperties property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::accessibilityProperties property doesn't exist");
 	}
	if (Type.typeof(x1.alpha) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::alpha property exists");
	} else {
	    DejaGnu.fail("DisplayObject::alpha property doesn't exist");
	}
	if (Std.is(x1.blendMode, String)) {
	    DejaGnu.pass("DisplayObject::blendMode property exists");
	} else {
	    DejaGnu.fail("DisplayObject::blendMode property doesn't exist");
	}
	if (Type.typeof(x1.cacheAsBitmap) == ValueType.TBool) {
	    DejaGnu.pass("DisplayObject::cacheAsBitmap property exists");
	} else {
	    DejaGnu.fail("DisplayObject::cacheAsBitmap property doesn't exist");
	}
 	if (Std.is(x1.filters, Array)) {
 	    DejaGnu.pass("DisplayObject::filters property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::filters property doesn't exist");
 	}
	if (Type.typeof(x1.height) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::height property exists");
	} else {
	    DejaGnu.fail("DisplayObject::height property doesn't exist");
	}
//Determine if the mask property of x1 exists by setting it and testing it
		var m1:Sprite = new Sprite();
		x1.mask = m1;
 	if (Std.is(x1.mask, DisplayObject)) {
 	    DejaGnu.pass("DisplayObject::mask property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::mask property doesn't exist");
 	}
//Determine if the loaderInfo property of x1 exists by setting it and testing it
//	var urlRequest:URLRequest = new URLRequest("../classes.all/TEST.jpg");
//	loader.load(urlRequest);
// 	if (Std.is(x1.loaderInfo, LoaderInfo)) {
// 	    DejaGnu.pass("DisplayObject::loaderInfo property exists");
// 	} else {
// 	    DejaGnu.fail("DisplayObject::loaderInfo property doesn't exist");
// 	}
	if (Type.typeof(x1.mouseX) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::mouseX property exists");
	} else {
	    DejaGnu.fail("DisplayObject::mouseX property doesn't exist");
	}
	if (Type.typeof(x1.mouseY) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::mouseY property exists");
	} else {
	    DejaGnu.fail("DisplayObject::mouseY property doesn't exist");
	}
	if (Std.is(x1.name, String)) {
	    DejaGnu.pass("DisplayObject::name property exists");
	} else {
	    DejaGnu.fail("DisplayObject::name property doesn't exist");
	}
 	if (Type.typeof(x1.opaqueBackground) == ValueType.TNull) {
 	    DejaGnu.pass("DisplayObject::opaqueBackground property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::opaqueBackground property doesn't exist");
 	}
//Determine if the parent property of x1 exists by setting it and testing it
 	m1.addChild(x1);
 	if (Std.is(x1.parent, DisplayObjectContainer)) {
 	    DejaGnu.pass("DisplayObject::parent property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::parent property doesn't exist");
 	}
 	m1.addChild(l1);
//Determine if the stage property of x1 exists by setting it and testing it
 	flash.Lib.current.stage.addChild(x1);
 	if (Std.is(x1.stage, Stage)) {
 	    DejaGnu.pass("DisplayObject::stage property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::stage property doesn't exist");
 	}
 	if (Std.is(l1.root, DisplayObject)) {
 	    DejaGnu.pass("DisplayObject::root property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::root property doesn't exist");
 	}
 	DejaGnu.note("Type of l1.root is "+Type.typeof(l1.root));
	if (Type.typeof(x1.rotation) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::rotation property exists");
	} else {
	    DejaGnu.fail("DisplayObject::rotation property doesn't exist");
	}
//Determine if the scale9Grid property of x1 exists by setting it and testing it
	x1.graphics.drawRect(0,0,10,10);
	x1.scale9Grid = r1;
 	if (Std.is(x1.scale9Grid, Rectangle)) {
 	    DejaGnu.pass("DisplayObject::scale9Grid property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::scale9Grid property doesn't exist");
 	}
	if (Type.typeof(x1.scaleX) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::scaleX property exists");
	} else {
	    DejaGnu.fail("DisplayObject::scaleX property doesn't exist");
	}
	if (Type.typeof(x1.scaleY) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::scaleY property exists");
	} else {
	    DejaGnu.fail("DisplayObject::scaleY property doesn't exist");
	}
//Determine if the scrollRect property of x1 exists by setting it and testing it
	x1.scrollRect = r1;
 	if (Std.is(x1.scrollRect, Rectangle)) {
 	    DejaGnu.pass("DisplayObject::scrollRect property exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::scrollRect property doesn't exist"); 
 	}
 	if (Std.is(x1.transform, Transform)) {
 	    DejaGnu.pass("DisplayObject::transform property exists");
  	} else {
 	    DejaGnu.fail("DisplayObject::transform property doesn't exist");
 	}
	if (Type.typeof(x1.visible) == ValueType.TBool) {
	    DejaGnu.pass("DisplayObject::visible property exists");
	} else {
	    DejaGnu.fail("DisplayObject::visible property doesn't exist");
	}
	if (Type.typeof(x1.width) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::width property exists");
	} else {
	    DejaGnu.fail("DisplayObject::width property doesn't exist");
	}
	if (Type.typeof(x1.x) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::x property exists");
	} else {
	    DejaGnu.fail("DisplayObject::x property doesn't exist");
	}
	if (Type.typeof(x1.y) == ValueType.TFloat) {
	    DejaGnu.pass("DisplayObject::y property exists");
	} else {
	    DejaGnu.fail("DisplayObject::y property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
 	if (Type.typeof(x1.getBounds) == ValueType.TFunction) {
 	    DejaGnu.pass("DisplayObject::getBounds() method exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::getBounds() method doesn't exist");
 	}
 	if (Type.typeof(x1.getRect) == ValueType.TFunction) {
 	    DejaGnu.pass("DisplayObject::getRect() method exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::getRect() method doesn't exist");
 	}
 	if (Type.typeof(x1.globalToLocal) == ValueType.TFunction) {
 	    DejaGnu.pass("DisplayObject::globalToLocal() method exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::globalToLocal() method doesn't exist");
 	}
 	if (Type.typeof(x1.hitTestObject) == ValueType.TFunction) {
 	    DejaGnu.pass("DisplayObject::hitTestObject() method exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::hitTestObject() method doesn't exist");
 	}
 	if (Type.typeof(x1.hitTestPoint) == ValueType.TFunction) {
 	    DejaGnu.pass("DisplayObject::hitTestPoint() method exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::hitTestPoint() method doesn't exist");
 	}
 	if (Type.typeof(x1.localToGlobal) == ValueType.TFunction) {
 	    DejaGnu.pass("DisplayObject::localToGlobal() method exists");
 	} else {
 	    DejaGnu.fail("DisplayObject::localToGlobal() method doesn't exist");
 	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

