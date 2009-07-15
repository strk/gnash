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
import Reflect;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Stage_as {
    static function main() {
    	
#if !(flash8 || flash7 || flash6) // Basically checking #if flash5
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
//FIXME: This property doesn't exist in haXe
//	if (Type.typeof(x1.cacheAsBitmap)==ValueType.TBool) { 
//		DejaGnu.pass("Stage::cacheAsBitmap property exists");
//	} else {
//		DejaGnu.fail("Stage::cacheAsBitmap property doesn't exist");
//	}
	
// Set displayState first to standard state and then test for existence 
	untyped Stage.displayState = "normal";
	if (Std.is(untyped Stage.displayState,  String)) {
	    DejaGnu.pass("Stage::displayState property exists");
	} else {
	    DejaGnu.fail("Stage::displayState property doesn't exist");
	}
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
	var r1:Rectangle = new Rectangle(0,0,10,10);
	untyped Stage.fullScreenSourceRect = r1;
	if (Std.is(untyped Stage.fullScreenSourceRect, Rectangle)) {
	    DejaGnu.pass("Stage::fullScreenSourceRect property exists");
	} else {
	    DejaGnu.fail("Stage::fullScreenSourceRect property doesn't exist");
	}
//FIXME: These probably need to be set somehow before testing
//	DejaGnu.note("x1.fullScreenHeight type is "+Type.typeof(x1.fullScreenHeight));
//	if (Std.is(x1.fullScreenHeight, Int)) {
//	    DejaGnu.pass("Stage::fullScreenHeight property exists");
//	} else {
//	    DejaGnu.fail("Stage::fullScreenHeight property doesn't exist");
//	}
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
//	if (Std.is(x1.tabEnabled, Bool)) {
//		DejaGnu.pass("Stage::tabEnabled property exists");
//	} else {
//		DejaGnu.fail("Stage::tabEnabled property doesn't exist");
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

// START OF MING TESTS!

#if !flash9

	if (Type.typeof(untyped Stage.addListener) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::addChild() method exists");
 	} else {
 	    DejaGnu.fail("Stage::addChild() method doesn't exist");
 	}
 	if (Type.typeof(untyped Stage.removeListener) == ValueType.TFunction) {
 	    DejaGnu.pass("Stage::addChildAt() method exists");
 	} else {
 	    DejaGnu.fail("Stage::addChildAt() method doesn't exist");
 	}
 	if (Type.typeof(untyped Stage.broadcastMessage) == ValueType.TFunction) {
		DejaGnu.pass("Stage::broadcastMessage method exists");
	} else {
		DejaGnu.fail("Stage::broadcastMessage method doesn't exist");
	}
	

	if (untyped Stage.hasOwnProperty("_listeners")) {
		DejaGnu.pass("_listeners property exists");
	} else {
		DejaGnu.fail("_listeners property does not exist");
	}
	if (Reflect.isObject(untyped Stage._listeners)) {
		DejaGnu.pass("_listeners is an object");
	} else {
		DejaGnu.fail("_listeners is not an object");
	}
	if (Type.getClassName(Type.getClass(untyped Stage._listeners)) == "Array") {
    	DejaGnu.pass("_listeners is an array");
	} else {
		DejaGnu.fail("_listeners is not an array");
	}
	if (untyped Stage.hasOwnProperty("height")) {
		DejaGnu.pass("height property exists");
	} else {
		DejaGnu.fail("height property does not exist");
	}
	if (untyped Stage.hasOwnProperty("width")) {
		DejaGnu.pass("width property exists");
	} else {
		DejaGnu.fail("width property does not exist");
	}
	if (untyped Stage.hasOwnProperty("scaleMode")) {
		DejaGnu.pass("scaleMode property exists");
	} else {
		DejaGnu.fail("scaleMode property does not exist");
	}
	if (untyped Stage.hasOwnProperty("showMenu")) {
		DejaGnu.pass("showMenu property exists");
	} else {
		DejaGnu.fail("showMenu property does not exist");
	}
	if (untyped Stage.hasOwnProperty("align")) {
		DejaGnu.pass("align property exists");
	} else {
		DejaGnu.fail("align property does not exist");
	}
	if (untyped Stage.hasOwnProperty("displayState")) {
		DejaGnu.pass("displayState property exists");
	} else {
		DejaGnu.fail("displayState property does not exist");
	}

	// Checking for Stage.align
	if (Type.getClassName(Type.getClass(untyped Stage.align)) == "String") {
		DejaGnu.pass("Stage.align is of type string");
	} else {
		DejaGnu.fail("Stage.align is not of type string");
	}
	
	untyped Stage.align = "T";
	
	if (untyped Stage.align == "T") {
		DejaGnu.pass("Stage.align is initialized to 'T'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'T'");
	}
	
	untyped Stage.align = "B";
	
	if (untyped Stage.align == "B") {
		DejaGnu.pass("Stage.align is initialized to 'B'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'B'");
	}
	
	untyped Stage.align = "l";
	
	if (untyped Stage.align == "L") {
		DejaGnu.pass("Stage.align is initialized to 'L'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'L'");
	}
	
	untyped Stage.align = "R";
	
	if (untyped Stage.align == "R") {
		DejaGnu.pass("Stage.align is initialized to 'R'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'R'");
	}
	
	untyped Stage.align = "TL";
	
	if (untyped Stage.align == "LT") {
		DejaGnu.pass("Stage.align is initialized to 'LT'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LT'");
	}
	
	untyped Stage.align = "B        R";
	
	if (untyped Stage.align == "RB") {
		DejaGnu.pass("Stage.align is initialized to 'RB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'RB'");
	}
	
	untyped Stage.align = "LThhhhhhh";
	
	if (untyped Stage.align=="LT") {
		DejaGnu.pass("Stage.align is initialized to 'LT'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LT'");
	}
	
	untyped Stage.align = "B       rhhhh";
	
	if (untyped Stage.align=="RB") {
		DejaGnu.pass("Stage.align is initialized to 'RB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'RB'");
	}
	
	untyped Stage.align = "TR";
	
	if (untyped Stage.align=="TR") {
		DejaGnu.pass("Stage.align is initialized to 'TR'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'TR'");
	}
	
	untyped Stage.align = "RT";

	if (untyped Stage.align=="TR") {
		DejaGnu.pass("Stage.align is initialized to 'TR'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'TR'");
	}	
	
	untyped Stage.align = "lb";
	
	if (untyped Stage.align=="LB") {
		DejaGnu.pass("Stage.align is initialized to 'LB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LB'");
	}	
	
	untyped Stage.align = "BR";

	if (untyped Stage.align=="RB") {
		DejaGnu.pass("Stage.align is initialized to 'RB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'RB'");
	}	
	
	untyped Stage.align = "LT";

	if (untyped Stage.align=="LT") {
		DejaGnu.pass("Stage.align is initialized to 'LT'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LT'");
	}	
	
	untyped Stage.align = "LTR";

	if (untyped Stage.align=="LTR") {
		DejaGnu.pass("Stage.align is initialized to 'LTR'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LTR'");
	}		
	
	untyped Stage.align = "LTRB";

	if (untyped Stage.align=="LTRB") {
		DejaGnu.pass("Stage.align is initialized to 'LTRB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LTRB'");
	}	
	
	untyped Stage.align = "TBR";

	if (untyped Stage.align=="TRB") {
		DejaGnu.pass("Stage.align is initialized to 'TRB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'TRB'");
	}
	
	untyped Stage.align = "BT";

	if (untyped Stage.align=="TB") {
		DejaGnu.pass("Stage.align is initialized to 'TB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'TB'");
	}
	
	untyped Stage.align = "RL";

	if (untyped Stage.align=="LR") {
		DejaGnu.pass("Stage.align is initialized to 'LR'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'LR'");
	}
	
	untyped Stage.align = "R mdmdmdmdmdmdmsdcmbkjaskjhasd";

	if (untyped Stage.align=="RB") {
		DejaGnu.pass("Stage.align is initialized to 'RB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'RB'");
	}

	untyped Stage.align = "xR mdmdmdmdmdmdmsdcmbkjaskjhasd";

	if (untyped Stage.align=="RB") {
		DejaGnu.pass("Stage.align is initialized to 'RB'");
	} else {
		DejaGnu.fail("Stage.align is not initialized to 'RB'");
	}
	
	untyped Stage.align = "X";
	
	if (untyped Stage.align=="") {
		DejaGnu.pass("Stage.align is initialized to ''");
	} else {
		DejaGnu.fail("Stage.align is not initialized to ''");
	}
	
	// Checking for Stage.displayState
	if (Type.getClassName(Type.getClass(untyped Stage.displayState)) == "String") {
		DejaGnu.pass("Stage.displayState is of type string");
	} else {
		DejaGnu.fail("Stage.displayState is not of type string");
	}

	if (untyped Stage.displayState=="normal") {
		DejaGnu.pass("Stage.displayState is set to normal");
	} else {
		DejaGnu.fail("Stage.displayState is not set to normal");
	}
	
	untyped Stage.displayState = "fullScreen";
	
	if (untyped Stage.displayState=="fullScreen") {
		DejaGnu.pass("Stage.displayState is set to fullScreen");
	} else {
		DejaGnu.fail("Stage.displayState is not set to fullScreen");
	}
	
	untyped Stage.displayState = "X";
	
	if (untyped Stage.displayState=="fullScreen") {
		DejaGnu.pass("Stage.displayState is set to fullScreen");
	} else {
		DejaGnu.fail("Stage.displayState is not set to fullScreen");
	}
	
	untyped Stage.displayState = "NORMAL";
	
	if (untyped Stage.displayState=="normal") {
		DejaGnu.pass("Stage.displayState is set to normal");
	} else {
		DejaGnu.fail("Stage.displayState is not set to normal");
	}

	var stageheightcheck = 0;
	var rscount = 0;
	
	var listener : Dynamic = {};
	listener.onResize = function() {
		untyped Stage.height = 1;
		stageheightcheck = untyped Stage.height;
		rscount++;
	};

	var fscount = 0;
	var valtype = null;
	var fs = null;
	
	listener.onFullScreen = function(fs) {
    	DejaGnu.note("onFullScreen event received: value " + fs);
		valtype = untyped __typeof__(fs);
		fscount++;
	};	

	untyped Stage.addListener(listener);
	
	untyped Stage.scaleMode = 5;
	
	if (untyped Stage.scaleMode=="showAll") {
		DejaGnu.pass("Stage.scaleMode set to showAll");
	} else {
		DejaGnu.fail("Stage.scaleMode not set to showAll");
	}
	
	untyped Stage.scaleMode = "exactFit";
	
	if (untyped Stage.scaleMode=="exactFit") {
		DejaGnu.pass("Stage.scaleMode set to exactFit");
	} else {
		DejaGnu.fail("Stage.scaleMode not set to exactFit");
	}
	
	untyped Stage.scaleMode = "sHOwall";
	
	if (untyped Stage.scaleMode=="showAll") {
		DejaGnu.pass("Stage.scaleMode set to showAll");
	} else {
		DejaGnu.fail("Stage.scaleMode not set to showAll");
	}
	
	untyped Stage.scaleMode = "noBorder";
	
	if (untyped Stage.scaleMode=="noBorder") {
		DejaGnu.pass("Stage.scaleMode set to noBorder");
	} else {
		DejaGnu.fail("Stage.scaleMode not set to noBorder");
	}
	
	untyped Stage.scaleMode = "noScale";
	
	if (untyped Stage.scaleMode=="noScale") {
		DejaGnu.pass("Stage.scaleMode set to noScale");
	} else {
		DejaGnu.fail("Stage.scaleMode not set to noScale");
	}
	
	untyped Stage.displayState = "fullScreen";
	untyped Stage.displayState = "normal";
	
	if (fscount==2) {
		DejaGnu.pass("fscount = 2");
	} else {
		DejaGnu.fail("fscount = " + fscount);
	}

	DejaGnu.note("NOTE: Linux version of the proprietary player is known to fail a test (sending a bogus onResize event)");

	if (rscount==0) {
		DejaGnu.pass("fscount = 0");
	} else {
		DejaGnu.fail("fscount = " + rscount);
	}

	// valtype is null -- FAIL!
	if (valtype=="boolean") {
	//if (Type.typeof(valtype)==ValueType.TBool) {
		DejaGnu.pass("valtype returns a boolean");
	} else {
		DejaGnu.fail("valtype does not return a boolean");
	}
	
	if (stageheightcheck!=1) {
		DejaGnu.pass("stageheightcheck is not equal to 1");
	} else {
		DejaGnu.fail("stageheightcheck is equal to: " + stageheightcheck);
	}
	
	var o : Dynamic = {};
	
	o.onResize = function() {
		DejaGnu.note("Resize event received by deleted object");
	};
	
	untyped Stage.addListener(o);
	Reflect.deleteField;	
#else
	if (untyped Stage.addListener==null) {
		DejaGnu.pass("Stage.addListener is undefined");
	} else {
		DejaGnu.fail("Stage.addListener is defined");
	}
	if (untyped Stage.removeListener==null) {
		DejaGnu.pass("Stage.removeListener is undefined");
	} else { 
		DejaGnu.fail("Stage.removeListener is defined");
	}
#end
	
	//-------------------------------------------------
	//  Testing Stage.showMenu property
	//-------------------------------------------------
	DejaGnu.note("*** Begin testing Stage.showMenu property");
	
	DejaGnu.note("showMenu init: " + untyped Stage.showMenu);
	if (untyped Stage.showMenu == true ) {
		DejaGnu.pass("Stage.showMenu correctly initialized to 'true'");
	} else {
		DejaGnu.fail("Stage.showMenu is not initialized to 'true'");
	}
	
	untyped Stage.showMenu = false;
	//Reflect.setField( Stage, "showMenu", false );
	if (untyped Stage.showMenu == false) {
		DejaGnu.pass("Stage.showMenu changed to false");
	} else {
		DejaGnu.fail("Stage.showMenu was not correctly changed to false");
	}
	
	untyped Stage.showMenu = true;
	//Reflect.setField( Stage, "showMenu", true );
	if (untyped Stage.showMenu == true) {
		DejaGnu.pass("Stage.showMenu reset to true");
	} else {
		DejaGnu.fail("Stage.showMenu not correctly reset to true");
	}
	
	
    // Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

