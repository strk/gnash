// InterpolationMethod_as.hx:  ActionScript 3 "InterpolationMethod" class, for Gnash.
//
// Generated on: 20090601 by "bnaugle". Remove this
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
import flash.display.InterpolationMethod;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class InterpolationMethod_as {
    static function main() {
#if flash9
        // Make sure we actually get a valid class        
    if (InterpolationMethod.LINEAR_RGB != null) {
            DejaGnu.pass("InterpolationMethod.LINEAR_RGB constant exists");
            if (Std.is(InterpolationMethod.LINEAR_RGB, String)) {
            	DejaGnu.pass("InterpolationMethod.LINEAR_RGB is a String");
            	if (Std.string(InterpolationMethod.LINEAR_RGB) == "linearRGB") {
            		DejaGnu.pass("InterpolationMethod.LINEAR_RGB is the correct string (linearRGB)");
            	} else {
            		DejaGnu.fail(
            		"InterpolationMethod.LINEAR_RGB is not the correct string (Should be linearRGB, but is "+InterpolationMethod.LINEAR_RGB+")");
            	}
            } else {
            	DejaGnu.fail("InterpolationMethod.LINEAR_RGB is not a string. Instead, it is a "+Type.typeof(InterpolationMethod.LINEAR_RGB));
            }
        } else {
            DejaGnu.fail("InterpolationMethod.LINEAR_RGB constant doesn't exist");
        }

    if (InterpolationMethod.RGB != null) {
            DejaGnu.pass("InterpolationMethod.RGB constant exists");
            if (Std.is(InterpolationMethod.RGB, String)) {
            	DejaGnu.pass("InterpolationMethod.RGB is a String");
            	if (Std.string(InterpolationMethod.RGB) == "rgb") {
            		DejaGnu.pass("InterpolationMethod.RGB is the correct string (rgb)");
            	} else {
            		DejaGnu.fail(
            		"InterpolationMethod.RGB is not the correct string (Should be rgb, but is "+InterpolationMethod.RGB+")");
            	}
            } else {
            	DejaGnu.fail("InterpolationMethod.RGB is not a string. Instead, it is a "+Type.typeof(InterpolationMethod.RGB));
            }
        } else {
            DejaGnu.fail("InterpolationMethod.RGB constant doesn't exist");
        }

        DejaGnu.done();
#else
	DejaGnu.note("This class (InterpolationMethod) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

