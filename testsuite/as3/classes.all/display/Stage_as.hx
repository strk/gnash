// Stage_as.hx:  ActionScript 3 "Stage" class, for Gnash.
//
// Generated on: 20090601 by "bnaugle". Remove this
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
import flash.display.Stage;
import flash.display.StageDisplayState;
import flash.display.MovieClip;
import flash.display.InteractiveObject;
import flash.geom.Rectangle;
//import flash.display.NativeWindow; FIXME: This class does not exist in haXe
import flash.text.TextSnapshot;
import flash.text.TextField;
#else
#if flash8
import flash.geom.Rectangle;
#end
import flash.Stage;
import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Stage_as {
    static function main() {
#if !(flash8 || flash7 || flash6)
        var x1:Stage = flash.Lib.current.stage;

        // Make sure we actually get a valid class        
        if (Std.is(x1, Stage)) {
            DejaGnu.pass("Stage class exists");
        } else {
            DejaGnu.fail("Stage class doesn't exist");
        }
#else
		if(Type.typeof(Stage) == ValueType.TObject) {
		    DejaGnu.pass("Stage class exists");
        } else {
            DejaGnu.fail("Stage class doesn't exist");
        }
#end
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash10
	if (Std.is(x1.enableColorCorrection, Bool)) {
	    DejaGnu.pass("Stage::enableColorCorrection() method exists");
	} else {
	    DejaGnu.fail("Stage::enableColorCorrection() method doesn't exist");
	}
#end
#if flash9
	if (Std.is(x1.align, String)) {
	    DejaGnu.pass("Stage::align property exists");
	} else {
	    DejaGnu.fail("Stage::align property doesn't exist");
	}
//FIXME: I can't get this to work. displayState type is always TNull
	x1.displayState = StageDisplayState.NORMAL;
	if (Std.is(x1.displayState, String)) {
	    DejaGnu.pass("Stage::displayState property exists");
	} else {
	    DejaGnu.fail("Stage::displayState property doesn't exist");
	}
	DejaGnu.note("x1.displaystate type is "+Type.typeof(x1.displayState));
//Determine if the focus property exists by setting and testing it
	var t1:TextField = new TextField();
	x1.focus = t1;
 	if (Std.is(x1.focus, InteractiveObject)) {
 	    DejaGnu.pass("Stage::focus property exists");
 	} else {
 	    DejaGnu.fail("Stage::focus property doesn't exist");
 	}
	if (Std.is(x1.frameRate, Int)) {
	    DejaGnu.pass("Stage::frameRate property exists");
	} else {
	    DejaGnu.fail("Stage::frameRate property doesn't exist");
	}
//FIXME: These probably need to be set somehow before testing
//	DejaGnu.note("x1.fullScreenHeight type is "+Type.typeof(x1.fullScreenHeight));
//	if (Std.is(x1.fullScreenHeight, Int)) {
//	    DejaGnu.pass("Stage::fullScreenHeight property exists");
//	} else {
//	    DejaGnu.fail("Stage::fullScreenHeight property doesn't exist");
//	}
// 	if (Std.is(x1.fullScreenSourceRect, Rectangle)) {
// 	    DejaGnu.pass("Stage::fullScreenSourceRect property exists");
// 	} else {
// 	    DejaGnu.fail("Stage::fullScreenSourceRect property doesn't exist");
// 	}
//	if (Std.is(x1.fullScreenWidth, Int)) {
//	    DejaGnu.pass("Stage::fullScreenWidth property exists");
//	} else {
//	    DejaGnu.fail("Stage::fullScreenWidth property doesn't exist");
//	}
//FIXME: This property does not exist in haXe
//	if (Std.is(x1.height, Int)) {
//	    DejaGnu.pass("Stage::height property exists");
//	} else {
//	    DejaGnu.fail("Stage::height property doesn't exist");
//	}
//FIXME: This property does not exist in haXe
//	if (Std.is(x1.mouseChildren, Bool)) {
//	    DejaGnu.pass("Stage::mouseChildren property exists");
//	} else {
//	    DejaGnu.fail("Stage::mouseChildren property doesn't exist");
//	}
//FIXME: This property does not exist in haXe
// 	if (Std.is(x1.nativeWindow, NativeWindow)) {
// 	    DejaGnu.pass("Stage::nativeWindow property exists");
// 	} else {
// 	    DejaGnu.fail("Stage::nativeWindow property doesn't exist");
// 	}
//FIXME: This property does not exist in haXe
//	if (Std.is(x1.numChildren, Int)) {
//	    DejaGnu.pass("Stage::numChildren property exists");
//	} else {
//	    DejaGnu.fail("Stage::numChildren property doesn't exist");
//	}
	if (Std.is(x1.quality, String)) {
	    DejaGnu.pass("Stage::quality property exists");
	} else {
	    DejaGnu.fail("Stage::quality property doesn't exist");
	}
	if (Std.is(x1.scaleMode, String)) {
	    DejaGnu.pass("Stage::scaleMode property exists");
	} else {
	    DejaGnu.fail("Stage::scaleMode property doesn't exist");
	}
	if (Std.is(x1.showDefaultContextMenu, Bool)) {
	    DejaGnu.pass("Stage::showDefaultContextMenu property exists");
	} else {
	    DejaGnu.fail("Stage::showDefaultContextMenu property doesn't exist");
	}
	if (Std.is(x1.stageFocusRect, Bool)) {
	    DejaGnu.pass("Stage::stageFocusRect property exists");
	} else {
	    DejaGnu.fail("Stage::stageFocusRect property doesn't exist");
	}
	if (Std.is(x1.stageHeight, Int)) {
	    DejaGnu.pass("Stage::stageHeight property exists");
	} else {
	    DejaGnu.fail("Stage::stageHeight property doesn't exist");
	}
	if (Std.is(x1.stageWidth, Int)) {
	    DejaGnu.pass("Stage::stageWidth property exists");
	} else {
	    DejaGnu.fail("Stage::stageWidth property doesn't exist");
	}
//FIXME: This property does not exist in haXe
//	if (Std.is(x1.tabChildren, Bool)) {
//	    DejaGnu.pass("Stage::tabChildren property exists");
//	} else {
//	    DejaGnu.fail("Stage::tabChildren property doesn't exist");
//	}
//FIXME: This property does not exist in haXe
// 	if (Std.is(x1.textSnapshot, TextSnapshot)) {
// 	    DejaGnu.pass("Stage::textSnapshot property exists");
// 	} else {
// 	    DejaGnu.fail("Stage::textSnapshot property doesn't exist");
// 	}
//FIXME: This property does not exist in haXe
//	if (Std.is(x1.width, Int)) {
//	    DejaGnu.pass("Stage::width property exists");
//	} else {
//	    DejaGnu.fail("Stage::width property doesn't exist");
//	}
#else
	if (Std.is(Stage.width, Float)) {
	    DejaGnu.pass("Stage::width property exists");
	} else {
	    DejaGnu.fail("Stage::width property doesn't exist");
	}
	if (Std.is(Stage.height, Float)) {
	    DejaGnu.pass("Stage::height property exists");
	} else {
	    DejaGnu.fail("Stage::height property doesn't exist");
	}
	if (Std.is(Stage.scaleMode, String)) {
	    DejaGnu.pass("Stage::scaleMode property exists");
	} else {
	    DejaGnu.fail("Stage::scaleMode property doesn't exist");
	}
	if (Std.is(Stage.align, String)) {
	    DejaGnu.pass("Stage::align property exists");
	} else {
	    DejaGnu.fail("Stage::align property doesn't exist");
	}
//FIXME: This property has not been implemented yet
	if (Std.is(Stage.showMenu, Bool)) {
	    DejaGnu.pass("Stage::showMenu property exists");
	} else {
	    DejaGnu.fail("Stage::showMenu property doesn't exist");
	}
#if flash8
	if (Std.is(Stage.displayState, String)) {
	    DejaGnu.pass("Stage::displayState property exists");
	} else {
	    DejaGnu.fail("Stage::displayState property doesn't exist");
	}
//Test to see if the Stage.fullScreenSourceRect property exists by setting and testing it
	var r1:Rectangle<Int> = new Rectangle(0,0,10,10);
	Stage.fullScreenSourceRect = r1;
	if (Std.is(Stage.fullScreenSourceRect, Rectangle)) {
	    DejaGnu.pass("Stage::fullScreenSourceRect property exists");
	} else {
	    DejaGnu.fail("Stage::fullScreenSourceRect property doesn't exist");
	}
#end
#end
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if !flash
 	if (Type.typeof(x1.assignFocus) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::assignFocus() method exists");
 	} else {
 	    DejaGnu.fail("Stage::assignFocus() method doesn't exist");
 	}
#end
#if !flash9
	if (Type.typeof(Stage.addListener) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::addChild() method exists");
 	} else {
 	    DejaGnu.fail("Stage::addChild() method doesn't exist");
 	}
 	if (Type.typeof(Stage.removeListener) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::addChildAt() method exists");
 	} else {
 	    DejaGnu.fail("Stage::addChildAt() method doesn't exist");
 	}
#end
#if flash9
 	if (Type.typeof(x1.addChild) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::addChild() method exists");
 	} else {
 	    DejaGnu.fail("Stage::addChild() method doesn't exist");
 	}
 	if (Type.typeof(x1.addChildAt) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::addChildAt() method exists");
 	} else {
 	    DejaGnu.fail("Stage::addChildAt() method doesn't exist");
 	}
	if (Type.typeof(x1.addEventListener) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::addEventListener() method exists");
	} else {
	    DejaGnu.fail("Stage::addEventListener() method doesn't exist");
	}
 	if (Type.typeof(x1.dispatchEvent) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::dispatchEvent() method exists");
 	} else {
 	    DejaGnu.fail("Stage::dispatchEvent() method doesn't exist");
 	}
	if (Type.typeof(x1.hasEventListener) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::hasEventListener() method exists");
	} else {
	    DejaGnu.fail("Stage::hasEventListener() method doesn't exist");
	}
	if (Type.typeof(x1.invalidate) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::invalidate() method exists");
	} else {
	    DejaGnu.fail("Stage::invalidate() method doesn't exist");
	}
	if (Type.typeof(x1.isFocusInaccessible) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::isFocusInaccessible() method exists");
	} else {
	    DejaGnu.fail("Stage::isFocusInaccessible() method doesn't exist");
	}
 	if (Type.typeof(x1.removeChild) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::removeChildAt() method exists");
 	} else {
 	    DejaGnu.fail("Stage::removeChildAt() method doesn't exist");
 	}
	if (Type.typeof(x1.setChildIndex) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::setChildIndex() method exists");
	} else {
	    DejaGnu.fail("Stage::setChildIndex() method doesn't exist");
	}
	if (Type.typeof(x1.swapChildrenAt) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::swapChildrenAt() method exists");
	} else {
	    DejaGnu.fail("Stage::swapChildrenAt() method doesn't exist");
	}
	if (Type.typeof(x1.willTrigger) == ValueType.TFunction) {
	    DejaGnu.pass("Stage::willTrigger() method exists");
	} else {
	    DejaGnu.fail("Stage::willTrigger() method doesn't exist");
	}
#end

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

