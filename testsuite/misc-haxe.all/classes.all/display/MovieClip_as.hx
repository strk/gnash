// MovieClip_as.hx:  ActionScript 3 "MovieClip" class, for Gnash.
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
import flash.display.MovieClip;
import flash.display.Scene;
#else
#if flash8
import flash.geom.Rectangle;
import flash.geom.Transform;
#end
import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class MovieClip_as {
    static function main() {
#if flash9
        var x1:MovieClip = new MovieClip();
#else
		var x1:MovieClip = flash.Lib._root;
#end

        // Make sure we actually get a valid class        
        if (Std.is(x1,MovieClip)) {
            DejaGnu.pass("MovieClip class exists");
        } else {
            DejaGnu.fail("MovieClip lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Type.typeof(x1.currentFrame) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::currentFrame property exists");
	} else {
	    DejaGnu.fail("MovieClip::currentFrame property doesn't exist");
	}
//FIXME: This property exists only in the HaXe documentation
//	if (Type.typeof(x1.getCurrentFrame) == String) {
//	    DejaGnu.pass("MovieClip::currentFrame property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::currentFrame property doesn't exist");
//	}
//FIXME: currentLabel is shown as type TNull but it should be a String. haXe bug?
//	if (Std.is(x1.currentLabel, String)) {
//	    DejaGnu.pass("MovieClip::currentLabel property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::currentLabel property doesn't exist");
//	}
//	DejaGnu.note("Type of currentLabel is "+Type.typeof(x1.currentLabel));
	if (Std.is(x1.currentLabels,Array)) {
 	    DejaGnu.pass("MovieClip::currentLabels property exists");
 	} else {
 	    DejaGnu.fail("MovieClip::currentLabels property doesn't exist");
 	}
 	if (Std.is(x1.currentScene,Scene)) {
 	    DejaGnu.pass("MovieClip::currentScene property exists");
 	} else {
 	    DejaGnu.fail("MovieClip::currentScene property doesn't exist");
 	}
	if (Type.typeof(x1.framesLoaded) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::framesLoaded property exists");
	} else {
	    DejaGnu.fail("MovieClip::framesLoaded property doesn't exist");
	}
 	if (Std.is(x1.scenes, Array)) {
 	    DejaGnu.pass("MovieClip::scenes property exists");
 	} else {
 	    DejaGnu.fail("MovieClip::scenes property doesn't exist");
 	}
	if (Type.typeof(x1.totalFrames) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::totalFrames property exists");
	} else {
	    DejaGnu.fail("MovieClip::totalFrames property doesn't exist");
	}
	if (Type.typeof(x1.currentFrame) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::trackAsMenu property exists");
	} else {
	    DejaGnu.fail("MovieClip::trackAsMenu property doesn't exist");
	}
#else
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._alpha) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_alpha property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_alpha property should be float, returns type "+Type.typeof(x1._alpha));
	}
	if (Type.typeof(x1._currentframe) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::_currentframe property exists");
	} else {
	    DejaGnu.fail("MovieClip::_currentframe property should be int, returns type "+Type.typeof(x1._currentframe));
	}
	if (Std.is(x1._droptarget,String)) {
	    DejaGnu.pass("MovieClip::_droptarget property exists");
	} else {
	    DejaGnu.fail("MovieClip::_droptarget property doesn't exist");
	}
	if (Type.typeof(x1._focusrect) == ValueType.TBool) {
	    DejaGnu.pass("MovieClip::_focusrect property exists");
	} else {
	    DejaGnu.fail("MovieClip::_focusrect property doesn't exist");
	}
	if (Type.typeof(x1._framesloaded) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::_framesloaded property exists");
	} else {
	    DejaGnu.fail("MovieClip::_framesloaded property doesn't exist");
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._height) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_height property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_height property should be float, returns type "+Type.typeof(x1._height));
	}
	if (Type.typeof(x1._lockroot) == ValueType.TBool) {
	    DejaGnu.pass("MovieClip::_lockroot property exists");
	} else {
	    DejaGnu.fail("MovieClip::_lockroot property doesn't exist");
	}
	if (Std.is(x1._name, String)) {
	    DejaGnu.pass("MovieClip::_name property exists");
	} else {
	    DejaGnu.fail("MovieClip::_name property doesn't exist");
	}
//FIXME: This property is defined but not implemented	
//	if (Std.is(x1._parent, MovieClip)) {
//	    DejaGnu.pass("MovieClip::_parent property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::_parent property doesn't exist");
//	}
	if (Std.is(x1._quality, String)) {
	    DejaGnu.pass("MovieClip::_quality property exists");
	} else {
	    DejaGnu.fail("MovieClip::_quality property doesn't exist");
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._rotation) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_rotation property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_rotation property should be float, returns type "+Type.typeof(x1._rotation));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._soundbuftime) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_soundbuftime property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_soundbuftime property should be float, returns type "+Type.typeof(x1._soundbuftime));
	}
	if (Std.is(x1._target, String)) {
	    DejaGnu.pass("MovieClip::_target property exists");
	} else {
	    DejaGnu.fail("MovieClip::_target property doesn't exist");
	}
	if (Type.typeof(x1._totalframes) == ValueType.TInt) {
	    DejaGnu.pass("MovieClip::_totalframes property exists");
	} else {
	    DejaGnu.fail("MovieClip::_totalframes property doesn't exist");
	}
	if (Std.is(x1._url, String)) {
	    DejaGnu.pass("MovieClip::_url property exists");
	} else {
	    DejaGnu.fail("MovieClip::_url property doesn't exist");
	}
	if (Type.typeof(x1._visible) == ValueType.TBool) {
	    DejaGnu.pass("MovieClip::_visible property exists");
	} else {
	    DejaGnu.fail("MovieClip::_visible property doesn't exist");
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._width) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_width property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_width property should be float, returns type "+Type.typeof(x1._width));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._x) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_x property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_x property should be float, returns type "+Type.typeof(x1._x));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._xmouse) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_xmouse property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_xmouse property should be float, returns type "+Type.typeof(x1._xmouse));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._xscale) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_xscale property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_xscale property should be float, returns type "+Type.typeof(x1._xscale));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._y) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_y property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_y property should be float, returns type "+Type.typeof(x1._y));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._ymouse) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_ymouse property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_ymouse property should be float, returns type "+Type.typeof(x1._ymouse));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1._yscale) == ValueType.TFloat) {
	    DejaGnu.xpass("MovieClip::_yscale property exists");
	} else {
	    DejaGnu.xfail("MovieClip::_yscale property should be float, returns type "+Type.typeof(x1._yscale));
	}
	if (Std.is(x1.blendMode, Dynamic)) {
		DejaGnu.unresolved("This property has not been implemented");
	} else {
	    DejaGnu.xfail("MovieClip::blendMode property should be object, returns type "+Type.typeof(x1.blendMode));
	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.cacheAsBitmap, Bool)) {
//	    DejaGnu.pass("MovieClip::cacheAsBitmap property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::cacheAsBitmap property doesn't exist");
//	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.filters, Array)) {
//	    DejaGnu.pass("MovieClip::filters property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::filters property doesn't exist");
//	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.focusEnabled, Bool)) {
//	    DejaGnu.pass("MovieClip::focusEnabled property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::focusEnabled property doesn't exist");
//	}
//FIXME: Not sure about which version of Flash implements this property.
//	if (Std.is(x1.forceSmoothing, Bool)) {
//	    DejaGnu.pass("MovieClip::forceSmoothing property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::forceSmoothing property doesn't exist");
//	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.hitArea, MovieClip)) {
//	    DejaGnu.pass("MovieClip::hitArea property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::hitArea property doesn't exist");
//	}
	if (Type.typeof(x1.opaqueBackground) == ValueType.TInt) {
	    DejaGnu.xpass("MovieClip::opaqueBackground property exists");
	} else {
	    DejaGnu.xfail("MovieClip::opaqueBackground property doesn't exist");
	}
#if flash8
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.scale9Grid, Rectangle)) {
//	    DejaGnu.pass("MovieClip::scale9Grid property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::scale9Grid property doesn't exist");
//	}
#end
	if (Std.is(x1.scrollRect, Dynamic)) {
	    DejaGnu.unresolved("This property has not been implemented");
	} else {
	    DejaGnu.xfail("MovieClip::scrollRect property doesn't exist");
	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.tabChildren, Bool)) {
//	    DejaGnu.pass("MovieClip::tabChildren property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::tabChildren property doesn't exist");
//	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.tabEnabled, Bool)) {
//	    DejaGnu.pass("MovieClip::tabEnabled property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::tabEnabled property doesn't exist");
//	}
	if (Type.typeof(x1.tabIndex) == ValueType.TInt) {
	    DejaGnu.xpass("MovieClip::tabIndex property exists");
	} else {
	    DejaGnu.xfail("MovieClip::tabIndex property doesn't exist");
	}
//FIXME: This property is defined but not implemented
//	if (Std.is(x1.trackAsMenu, Bool)) {
//	    DejaGnu.pass("MovieClip::trackAsMenu property exists");
//	} else {
//	    DejaGnu.fail("MovieClip::trackAsMenu property doesn't exist");
//	}
#if flash8
	if (Std.is(x1.transform, Transform)) {
	    DejaGnu.pass("MovieClip::transform property exists");
	} else {
	    DejaGnu.fail("MovieClip::transform property doesn't exist");
	}
#end
	if (Type.typeof(x1.useHandCursor) == ValueType.TBool) {
	    DejaGnu.pass("MovieClip::useHandCursor property exists");
	} else {
	    DejaGnu.fail("MovieClip::useHandCursor property doesn't exist");
	}
#end
	//This method exists in both AS2 and AS3
	if (Type.typeof(x1.enabled) == ValueType.TBool) {
	    DejaGnu.pass("MovieClip::enabled property exists");
	} else {
	    DejaGnu.fail("MovieClip::enabled property doesn't exist");
	}
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Type.typeof(x1.addFrameScript) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::addFrameScript method exists");
	} else {
	    DejaGnu.fail("MovieClip::addFrameScript method doesn't exist");
	}
	if (Type.typeof(x1.gotoAndPlay) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::gotoAndPlay() method exists");
	} else {
	    DejaGnu.fail("MovieClip::gotoAndPlay() method doesn't exist");
	}
	if (Type.typeof(x1.gotoAndStop) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::gotoAndStop() method exists");
	} else {
	    DejaGnu.fail("MovieClip::gotoAndStop() method doesn't exist");
	}
	if (Type.typeof(x1.nextFrame) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::nextFrame() method exists");
	} else {
	    DejaGnu.fail("MovieClip::nextFrame() method doesn't exist");
	}
	if (Type.typeof(x1.nextScene) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::nextScene() method exists");
	} else {
	    DejaGnu.fail("MovieClip::nextScene() method doesn't exist");
	}
	if (Type.typeof(x1.play) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::play() method exists");
	} else {
	    DejaGnu.fail("MovieClip::play() method doesn't exist");
	}
	if (Type.typeof(x1.prevFrame) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::prevFrame() method exists");
	} else {
	    DejaGnu.fail("MovieClip::prevFrame() method doesn't exist");
	}
	if (Type.typeof(x1.prevScene) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::prevScene() method exists");
	} else {
	    DejaGnu.fail("MovieClip::prevScene() method doesn't exist");
	}
	if (Type.typeof(x1.stop) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::stop() method exists");
	} else {
	    DejaGnu.fail("MovieClip::stop() method doesn't exist");
	}
#else
	if (Type.typeof(x1.attachAudio) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::attachAudio method exists");
	} else {
	    DejaGnu.fail("MovieClip::attachAudio method doesn't exist");
	}
#if !(flash6 || flash7)
	if (Type.typeof(x1.attachBitmap) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::attachBitmap() method exists");
	} else {
	    DejaGnu.fail("MovieClip::attachBitmap() method doesn't exist");
	}
	if (Type.typeof(x1.beginBitmapFill) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::beginBitmapFill() method exists");
	} else {
	    DejaGnu.fail("MovieClip::beginBitmapFill() method doesn't exist");
	}
	if (Type.typeof(x1.getRect) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getRect() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getRect() method doesn't exist");
	}
	if (Type.typeof(x1.lineGradientStyle) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::lineGradientStyle method exists");
	} else {
	    DejaGnu.fail("MovieClip::lineGradientStyle method doesn't exist");
	}
#end
	if (Type.typeof(x1.attachMovie) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::attachMovie() method exists");
	} else {
	    DejaGnu.fail("MovieClip::attachMovie() method doesn't exist");
	}
	if (Type.typeof(x1.attachVideo) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::attachVideo() method exists");
	} else {
	    DejaGnu.fail("MovieClip::attachVideo() method doesn't exist");
	}

	if (Type.typeof(x1.beginFill) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::beginFill() method exists");
	} else {
	    DejaGnu.fail("MovieClip::beginFill() method doesn't exist");
	}
	if (Type.typeof(x1.beginGradientFill) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::beginGradientFill() method exists");
	} else {
	    DejaGnu.fail("MovieClip::beginGradientFill() method doesn't exist");
	}
	if (Type.typeof(x1.clear) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::clear() method exists");
	} else {
	    DejaGnu.fail("MovieClip::clear() method doesn't exist");
	}
	if (Type.typeof(x1.createEmptyMovieClip) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::createEmptyMovieClip() method exists");
	} else {
	    DejaGnu.fail("MovieClip::createEmptyMovieClip() method doesn't exist");
	}
	if (Type.typeof(x1.createTextField) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::createTextField method exists");
	} else {
	    DejaGnu.fail("MovieClip::createTextField method doesn't exist");
	}
	if (Type.typeof(x1.curveTo) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::curveTo() method exists");
	} else {
	    DejaGnu.fail("MovieClip::curveTo() method doesn't exist");
	}
	if (Type.typeof(x1.duplicateMovieClip) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::duplicateMovieClip() method exists");
	} else {
	    DejaGnu.fail("MovieClip::duplicateMovieClip() method doesn't exist");
	}
	if (Type.typeof(x1.endFill) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::endFill() method exists");
	} else {
	    DejaGnu.fail("MovieClip::endFill() method doesn't exist");
	}
	if (Type.typeof(x1.getBounds) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getBounds() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getBounds() method doesn't exist");
	}
	if (Type.typeof(x1.getBytesLoaded) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getBytesLoaded() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getBytesLoaded() method doesn't exist");
	}
	if (Type.typeof(x1.getBytesTotal) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getBytesTotal() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getBytesTotal() method doesn't exist");
	}
	if (Type.typeof(x1.getDepth) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getDepth() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getDepth() method doesn't exist");
	}
#if !flash6
	if (Type.typeof(x1.getInstanceAtDepth) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getInstanceAtDepth() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getInstanceAtDepth() method doesn't exist");
	}
	if (Type.typeof(x1.getNextHighestDepth) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getNextHighestDepth method exists");
	} else {
	    DejaGnu.fail("MovieClip::getNextHighestDepth method doesn't exist");
	}
#end
	if (Type.typeof(x1.getSWFVersion) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getSWFVersion() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getSWFVersion() method doesn't exist");
	}
	if (Type.typeof(x1.getTextSnapshot) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getTextSnapshot() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getTextSnapshot() method doesn't exist");
	}
	if (Type.typeof(x1.getURL) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::getURL() method exists");
	} else {
	    DejaGnu.fail("MovieClip::getURL() method doesn't exist");
	}
	if (Type.typeof(x1.globalToLocal) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::globalToLocal() method exists");
	} else {
	    DejaGnu.fail("MovieClip::globalToLocal() method doesn't exist");
	}
	if (Type.typeof(x1.gotoAndPlay) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::gotoAndPlay() method exists");
	} else {
	    DejaGnu.fail("MovieClip::gotoAndPlay() method doesn't exist");
	}
	if (Type.typeof(x1.gotoAndStop) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::gotoAndStop() method exists");
	} else {
	    DejaGnu.fail("MovieClip::gotoAndStop() method doesn't exist");
	}
	if (Type.typeof(x1.hitTest) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::hitTest() method exists");
	} else {
	    DejaGnu.fail("MovieClip::hitTest() method doesn't exist");
	}
	if (Type.typeof(x1.lineStyle) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::lineStyle() method exists");
	} else {
	    DejaGnu.fail("MovieClip::lineStyle() method doesn't exist");
	}
	if (Type.typeof(x1.lineTo) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::lineTo() method exists");
	} else {
	    DejaGnu.fail("MovieClip::lineTo() method doesn't exist");
	}
	if (Type.typeof(x1.loadMovie) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::loadMovie() method exists");
	} else {
	    DejaGnu.fail("MovieClip::loadMovie() method doesn't exist");
	}
	if (Type.typeof(x1.loadVariables) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::loadVariables() method exists");
	} else {
	    DejaGnu.fail("MovieClip::loadVariables() method doesn't exist");
	}
	if (Type.typeof(x1.localToGlobal) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::localToGlobal() method exists");
	} else {
	    DejaGnu.fail("MovieClip::localToGlobal() method doesn't exist");
	}
	if (Type.typeof(x1.moveTo) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::moveTo() method exists");
	} else {
	    DejaGnu.fail("MovieClip::moveTo() method doesn't exist");
	}
	if (Type.typeof(x1.nextFrame) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::nextFrame() method exists");
	} else {
	    DejaGnu.fail("MovieClip::nextFrame() method doesn't exist");
	}
//FIXME: These dynamic functions return a null type value.
//	if (Type.typeof(x1.onData) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onData() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onData() method doesn't exist");
//	}
//	if (Type.typeof(x1.onDragOut) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onDragOut() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onDragOut() method doesn't exist");
//	}
//	if (Type.typeof(x1.onDragOver) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onDragOver() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onDragOver() method doesn't exist");
//	}
//	if (Type.typeof(x1.onEnterFrame) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onEnterFrame() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onEnterFrame() method doesn't exist");
//	}
//	if (Type.typeof(x1.onKeyDown) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onKeyDown() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onKeyDown() method doesn't exist");
//	}
//	if (Type.typeof(x1.onKeyUp) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onKeyUp method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onKeyUp method doesn't exist");
//	}
//	if (Type.typeof(x1.onKillFocus) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onKillFocus() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onKillFocus() method doesn't exist");
//	}
//	if (Type.typeof(x1.onLoad) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onLoad() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onLoad() method doesn't exist");
//	}
//	if (Type.typeof(x1.onMouseDown) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onMouseDown() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onMouseDown() method doesn't exist");
//	}
//	if (Type.typeof(x1.onMouseMove) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onMouseMove() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onMouseMove() method doesn't exist");
//	}
//	if (Type.typeof(x1.onMouseUp) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onMouseUp() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onMouseUp() method doesn't exist");
//	}
//	if (Type.typeof(x1.onPress) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onPress() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onPress() method doesn't exist");
//	}
//	if (Type.typeof(x1.onRelease) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onRelease() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onRelease() method doesn't exist");
//	}
//	if (Type.typeof(x1.onReleaseOutside) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onReleaseOutside() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onReleaseOutside() method doesn't exist");
//	}
//	if (Type.typeof(x1.onRollOut) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onRollOut method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onRollOut method doesn't exist");
//	}
//	if (Type.typeof(x1.onRollOver) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onRollOver() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onRollOver() method doesn't exist");
//	}
//	if (Type.typeof(x1.onSetFocus) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onSetFocus() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onSetFocus() method doesn't exist");
//	}
//	if (Type.typeof(x1.onUnload) == ValueType.TFunction) {
//	    DejaGnu.pass("MovieClip::onUnload() method exists");
//	} else {
//	    DejaGnu.fail("MovieClip::onUnload() method doesn't exist");
//	}
	if (Type.typeof(x1.play) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::play() method exists");
	} else {
	    DejaGnu.fail("MovieClip::play() method doesn't exist");
	}
	if (Type.typeof(x1.prevFrame) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::prevFrame() method exists");
	} else {
	    DejaGnu.fail("MovieClip::prevFrame() method doesn't exist");
	}
	if (Type.typeof(x1.removeMovieClip) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::removeMovieClip() method exists");
	} else {
	    DejaGnu.fail("MovieClip::removeMovieClip() method doesn't exist");
	}
	if (Type.typeof(x1.setMask) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::setMask() method exists");
	} else {
	    DejaGnu.fail("MovieClip::setMask() method doesn't exist");
	}
	if (Type.typeof(x1.startDrag) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::startDrag() method exists");
	} else {
	    DejaGnu.fail("MovieClip::startDrag() method doesn't exist");
	}
	if (Type.typeof(x1.stop) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::stop() method exists");
	} else {
	    DejaGnu.fail("MovieClip::stop() method doesn't exist");
	}
	if (Type.typeof(x1.stopDrag) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::stopDrag() method exists");
	} else {
	    DejaGnu.fail("MovieClip::stopDrag() method doesn't exist");
	}
	if (Type.typeof(x1.swapDepths) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::swapDepths() method exists");
	} else {
	    DejaGnu.fail("MovieClip::swapDepths() method doesn't exist");
	}
	if (Type.typeof(x1.unloadMovie) == ValueType.TFunction) {
	    DejaGnu.pass("MovieClip::unloadMovie() method exists");
	} else {
	    DejaGnu.fail("MovieClip::unloadMovie() method doesn't exist");
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

