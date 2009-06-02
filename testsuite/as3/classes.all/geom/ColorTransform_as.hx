// ColorTransform_as.hx:  ActionScript 3 "ColorTransform" class, for Gnash.
//
// Generated on: 20090602 by "bnaugle". Remove this
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

#if (flash9 || flash8)
import flash.geom.ColorTransform;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class ColorTransform_as {
    static function main() {
#if !(flash6 || flash7)
        var x1:ColorTransform = new ColorTransform();

        // Make sure we actually get a valid class        
	if (Std.is(x1, ColorTransform)) {
	    DejaGnu.pass("ColorTransform class exists");
        } else {
            DejaGnu.fail("ColorTransform class doesn't exist");
        }

	// Tests to see if all the properties exist. All these do is test for
	// existance of a property, and don't test the functionality at all. This
	// is primarily useful only to test completeness of the API implementation.
	if (Std.is(x1.alphaMultiplier, Float)) {
 	    DejaGnu.pass("ColorTransform.alphaMultiplier property exists");
	} else {
	    DejaGnu.fail("ColorTransform.alphaMultiplier property doesn't exist");
	}
	if (Std.is(x1.alphaOffset, Float)) {	
 	    DejaGnu.pass("ColorTransform.alphaOffset property exists");
	} else {
	    DejaGnu.fail("ColorTransform.alphaOffset property doesn't exist");
	}
	if (Std.is(x1.blueMultiplier, Float)) {
   	    DejaGnu.pass("ColorTransform.blueMultiplier property exists");
	} else {
	    DejaGnu.fail("ColorTransform.blueMultiplier property doesn't exist");
	}
	if (Std.is(x1.blueOffset, Float)) {
	    DejaGnu.pass("ColorTransform.blueOffset property exists");
	} else {
	    DejaGnu.fail("ColorTransform.blueOffset property doesn't exist");
	}
	#if flash9
	if (Std.is(x1.color, Int)) {
	    DejaGnu.pass("ColorTransform.color property exists");
	} else {
	    DejaGnu.fail("ColorTransform.color property doesn't exist");
	}
	#end
	if (Std.is(x1.greenMultiplier, Float)) {
	    DejaGnu.pass("ColorTransform.greenMultiplier property exists");
	} else {
	    DejaGnu.fail("ColorTransform.greenMultiplier property doesn't exist");
	}
	if (Std.is(x1.greenOffset, Float)) {
	    DejaGnu.pass("ColorTransform.greenOffset property exists");
	} else {
	    DejaGnu.fail("ColorTransform.greenOffset property doesn't exist");
	}
	if (Std.is(x1.redMultiplier, Float)) {
	    DejaGnu.pass("ColorTransform.redMultiplier property exists");
	} else {
	    DejaGnu.fail("ColorTransform.redMultiplier property doesn't exist");
	}
	if (Std.is(x1.redOffset, Float)) {
	    DejaGnu.pass("ColorTransform.redOffset property exists");
	} else {
	    DejaGnu.fail("ColorTransform.redOffset property doesn't exist");
	}

	// Tests to see if all the methods exist. All these do is test for
	// existance of a method, and don't test the functionality at all. This
	// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.concat)==ValueType.TFunction) {
  	    DejaGnu.pass("ColorTransform::concat() method exists");
	} else {
	    DejaGnu.fail("ColorTransform::concat() method doesn't exist");
	}
	if (Type.typeof(x1.toString)==ValueType.TFunction) {
	    DejaGnu.pass("ColorTransform::toString() method exists");
	} else {
	    DejaGnu.fail("ColorTransform::toString() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
	DejaGnu.done();
#else
	DejaGnu.note("This class (ColorTransform) is only available in flash8 and flash9");
#end
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

