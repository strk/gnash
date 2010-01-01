// Transform_as.hx:  ActionScript 3 "Transform" class, for Gnash.
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
//import flash.geom.Matrix3D;
//import flash.geom.PerspectiveProjection;
#end
#if flash8
import flash.MovieClip;
#end
#if !flash6
#if !flash7
import flash.geom.ColorTransform;
import flash.geom.Rectangle;
import flash.geom.Transform;
import flash.geom.Matrix;
#end
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Transform_as {
    static function main() {
#if !flash6
#if !flash7
#if flash9
    	var m1:MovieClip = new MovieClip();
        var x1:Transform = new Transform(m1);
#end
#if flash8
		var m1:MovieClip = flash.Lib._root;
		var x1:Transform = new Transform(m1);
#end

         // Make sure we actually get a valid class        
         if (Std.is(x1, Transform)) {
             DejaGnu.pass("Transform class exists");
         } else {
             DejaGnu.fail("Transform class doesn't exist");
         }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
 	var c1:ColorTransform = new ColorTransform();
 	x1.colorTransform = c1;
 	if (Std.is(x1.colorTransform, ColorTransform)) {
 	    DejaGnu.pass("Transform.colorTransform property exists");
 	} else {
 	    DejaGnu.fail("Transform.colorTransform property doesn't exist");
 	}
//FIXME: This property is unimplemented for flash8
 	if (Std.is(x1.concatenatedColorTransform, ColorTransform)) {
 	    DejaGnu.pass("Transform.concatenatedColorTransform property exists");
 	} else {
 	    DejaGnu.xfail("Transform.concatenatedColorTransform property doesn't exist");
 	}
//FIXME: This property is unimplemented for flash8
 	if (Std.is(x1.concatenatedMatrix, Matrix)) {
 	    DejaGnu.pass("Transform.concatenatedMatrix property exists");
 	} else {
 	    DejaGnu.xfail("Transform.concatenatedMatrix property doesn't exist");
 	}
#if flash10
//	if (Std.is(x1.matrix3D, Matrix3D)) {
// 	    DejaGnu.pass("Transform.matrix3D property exists");
// 	} else {
// 	    DejaGnu.fail("Transform.matrix3D property doesn't exist");
// 	}
// 	if (Std.is(x1.perspectiveProjection, PerspectiveProjection)) {
// 	    DejaGnu.pass("Transform.perspectiveProjection property exists");
// 	} else {
// 	    DejaGnu.fail("Transform.perspectiveProjection property doesn't exist");
// 	}
// 	if (Type.typeof(x1.getRelativeMatrix3D) == ValueType.TFunction) {
// 	    DejaGnu.pass("Transform.getRelativeMatrix3D method exists");
// 	} else {
// 	    DejaGnu.fail("Transform.getRelativeMatrix3D method doesn't exist");
// 	}
#end
 	if (Std.is(x1.matrix, Matrix)) {
 	    DejaGnu.pass("Transform.matrix property exists");
 	} else {
 	    DejaGnu.fail("Transform.matrix property doesn't exist");
 	}
//FIXME: This property is unimplemented for flash8
 	if (Std.is(x1.pixelBounds, Rectangle)) {
 	    DejaGnu.pass("Transform.pixelBounds property exists");
 	} else {
 	    DejaGnu.xfail("Transform.pixelBounds property doesn't exist");
 	}
 	DejaGnu.note("pixelBounds... is "+Type.typeof(x1.pixelBounds));
#if flash9
//FIXME: This class is not implemented in Haxe or ActionScript, but exists in Haxe API documentation
//	if (Type.typeof(x1.getRelativeMatrix3D) == ValueType.TFunction) {
// 	    DejaGnu.pass("Transform.getRelativeMatrix3D() method exists");
// 	} else {
// 	    DejaGnu.fail("Transform.getRelativeMatrix3D() method doesn't exist");
// 	}
#end
        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#end
#end
#if flash6
	DejaGnu.note("This class (Transform) is not available in Flash6");
#end
#if flash7
	DejaGnu.note("This class (Transform) is not available in Flash7");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

