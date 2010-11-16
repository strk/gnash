// DisplayObject_as.hx:  ActionScript 3 "DisplayObject" class, for Gnash.
//
// Generated on: 20090529 by "bnaugle". Remove this
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
        if (untyped __typeof__(DisplayObject) == 'object') {
          DejaGnu.pass("DisplayObject class exists");
        } else {
          DejaGnu.fail("DisplayObject class does not exist");
        }
        if (Std.is(x1, DisplayObject)) {
            DejaGnu.pass("Shape correctly inherits DisplayObject");
        } else {
            DejaGnu.fail("Shape does not correctly inherit DisplayObject");
        }
        
        //---------------------------------------------------------------------
        // Property Existence
        //---------------------------------------------------------------------
        DejaGnu.note("**** Property Existence testing ****");
        if (untyped x1.hasOwnProperty('accessibilityProperties')) {
            DejaGnu.pass("DisplayObject::accessibilityProperties property exists");
        } else {
            DejaGnu.fail(
                         "]DisplayObject::accessibilityProperties property"+
                         " doesn't exist");
        }
        if (untyped x1.hasOwnProperty('alpha')) {
            DejaGnu.pass("DisplayObject::alpha property exists");
        } else {
            DejaGnu.fail("DisplayObject::alpha"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('blendMode')) {
            DejaGnu.pass("DisplayObject::blendMode property exists");
        } else {
            DejaGnu.fail("DisplayObject::blendMode"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('cacheAsBitmap')) {
            DejaGnu.pass("DisplayObject::cacheAsBitmap property exists");
        } else {
            DejaGnu.fail("DisplayObject::cacheAsBitmap"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('filters')) {
            DejaGnu.pass("DisplayObject::filters property exists");
        } else {
            DejaGnu.fail("DisplayObject::filters"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('height')) {
            DejaGnu.pass("DisplayObject::height property exists");
        } else {
            DejaGnu.fail("DisplayObject::height"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('loaderInfo')) {
            DejaGnu.pass("DisplayObject::loaderInfo property exists");
        } else {
            DejaGnu.fail("DisplayObject::loaderInfo"+
                         " property does not exist");
        }
    //Determine if the mask property of x1 exists by setting it and testing it
        if (untyped x1.hasOwnProperty('mask')) {
            DejaGnu.pass("DisplayObject::mask property exists");
        } else {
            DejaGnu.fail("DisplayObject::mask property"+
                         " does not exist");
        }
        if (untyped x1.hasOwnProperty('mouseX')) {
            DejaGnu.pass("DisplayObject::mouseX property exists");
        } else {
            DejaGnu.fail("DisplayObject::mouseX"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('mouseY')) {
            DejaGnu.pass("DisplayObject::mouseY property exists");
        } else {
            DejaGnu.fail("DisplayObject::mouseY"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('name')) {
            DejaGnu.pass("DisplayObject::name property exists");
        } else {
            DejaGnu.fail("DisplayObject::name property"+
                         " doesn't exist");
        }
        if (untyped x1.hasOwnProperty('opaqueBackground')) {
            DejaGnu.pass("DisplayObject::opaqueBackground property exists");
        } else {
            DejaGnu.fail(
                         "]DisplayObject::opaqueBackground property doesn't"+
                         " exist");
        }
        if (untyped x1.hasOwnProperty('parent')) {
            DejaGnu.pass("DisplayObject::parent property exists");
        } else {
            DejaGnu.fail("DisplayObject::parent"+
                         " property does not exist");
        }
        if (untyped x1.hasOwnProperty('root')) {
            DejaGnu.pass("DisplayObject::root property exists");
        } else {
            DejaGnu.fail("DisplayObject::root property"+
                         " doesn't exist");
        }
        if (untyped x1.hasOwnProperty('rotation')) {
            DejaGnu.pass("DisplayObject::rotation property exists");
        } else {
            DejaGnu.fail("DisplayObject::rotation"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('scale9Grid')) {
            DejaGnu.pass("DisplayObject::scale9Grid property exists");
        } else {
            DejaGnu.fail("DisplayObject::scale9Grid"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('scaleX')) {
            DejaGnu.pass("DisplayObject::scaleX property exists");
        } else {
            DejaGnu.fail("DisplayObject::scaleX"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('scaleY')) {
            DejaGnu.pass("DisplayObject::scaleY property exists");
        } else {
            DejaGnu.fail("DisplayObject::scaleY"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('scrollRect')) {
            DejaGnu.pass("DisplayObject::scrollRect property exists");
        } else {
            DejaGnu.fail("DisplayObject::scrollRect"+
                         " property doesn't exist"); 
        }
        if (untyped x1.hasOwnProperty('stage')) {
            DejaGnu.pass("DisplayObject::stage property exists");
        } else {
            DejaGnu.fail("DisplayObject::stage"+
                         "property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('transform')) {
            DejaGnu.pass("DisplayObject::transform property exists");
        } else {
            DejaGnu.fail("DisplayObject::transform"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('visible')) {
            DejaGnu.pass("DisplayObject::visible property exists");
        } else {
            DejaGnu.fail("DisplayObject::visible"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('width')) {
            DejaGnu.pass("DisplayObject::width property exists");
        } else {
            DejaGnu.fail("DisplayObject::width"+
                         " property doesn't exist");
        }
        if (untyped x1.hasOwnProperty('x')) {
            DejaGnu.pass("DisplayObject::x property exists");
        } else {
            DejaGnu.fail("DisplayObject::x property"+
                         " doesn't exist");
        }
        if (untyped x1.hasOwnProperty('y')) {
            DejaGnu.pass("DisplayObject::y property exists");
        } else {
            DejaGnu.fail("DisplayObject::y property"+
                         " doesn't exist");
        }

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
        //----------------------------------------------------------------------
        // Method Existence
        //----------------------------------------------------------------------
        if (untyped x1.hasOwnProperty('getBounds')) {
            DejaGnu.pass("DisplayObject::getBounds() method exists");
        } else {
            DejaGnu.fail("DisplayObject::getBounds()"+
                         " method doesn't exist");
        }
        if (untyped __typeof__(x1.getBounds) == 'function') {
            DejaGnu.pass("getBounds() is a function");
        } else {
            DejaGnu.fail("getBounds() is not a function");
        }
        
        if (untyped x1.hasOwnProperty('getRect')) {
            DejaGnu.pass("DisplayObject::getRect() method exists");
        } else {
            DejaGnu.fail("DisplayObject::getRect()"+
                         " method doesn't exist");
        }
        if (untyped __typeof__(x1.getRect) == 'function') {
            DejaGnu.pass("getRect() is a function");
        } else {
            DejaGnu.fail("getRect() is not a function");
        }
        
        if (untyped x1.hasOwnProperty('globalToLocal')) {
            DejaGnu.pass("DisplayObject::globalToLocal() method exists");
        } else {
            DejaGnu.fail(
                        "]DisplayObject::globalToLocal() method doesn't exist");
        }
        if (untyped __typeof__(x1.globalToLocal) == 'function') {
            DejaGnu.pass("globalToLocal() is a function");
        } else {
            DejaGnu.fail("globalToLocal() is not a"+
                         " function");
        }

        if (untyped x1.hasOwnProperty('hitTestObject')) {
            DejaGnu.pass("DisplayObject::hitTestObject() method exists");
        } else {
            DejaGnu.fail(
                        "]DisplayObject::hitTestObject() method doesn't exist");
        }
        if (untyped __typeof__(x1.hitTestObject) == 'function') {
            DejaGnu.pass("hitTestObject() is a function");
        } else {
            DejaGnu.fail("hitTestObject() is not a"+
                         " function");
        }
        
        if (untyped x1.hasOwnProperty('hitTestPoint')) {
            DejaGnu.pass("DisplayObject::hitTestPoint() method exists");
        } else {
            DejaGnu.fail(
                         "]DisplayObject::hitTestPoint() method doesn't exist");
        }
        if (untyped __typeof__(x1.hitTestPoint) == 'function') {
            DejaGnu.pass("hitTestPoint() is a function");
        } else {
            DejaGnu.fail("hitTestPoint() is not a"+
                         " function");
        }
        
        if (untyped x1.hasOwnProperty('localToGlobal')) {
            DejaGnu.pass("DisplayObject::localToGlobal() method exists");
        } else {
            DejaGnu.fail(
                        "]DisplayObject::localToGlobal() method doesn't exist");
        }
        if (untyped __typeof__(x1.localToGlobal) == 'function') {
            DejaGnu.pass("localToGlobal() is a function");
        } else {
            DejaGnu.fail("localToGlobal() is not a"+
                         " function");
        }
#else
        DejaGnu.note("Display Object did not exist in versions prior to SWF 9");
        // Call this after finishing all tests. It prints out the totals.

#end
        DejaGnu.done();

    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

