// SoundTransform_as.hx:  ActionScript 3 "SoundTransform" class, for Gnash.
//
// Generated on: 20090603 by "bnaugle". Remove this
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


//NOTE: this is a new class for as3 (flash9 and >)


#if flash9
import flash.media.SoundTransform;
import flash.media.SoundChannel;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;

import DejaGnu;

// Class must be named with the PP prefix, as that's the name the
// file passed to haxe will have after the preprocessing step
class SoundTransform_as {
    static function main() {
#if flash9
        var x1:SoundTransform = new SoundTransform();

        // Make sure we actually get a valid class        
        if (Std.is(x1, SoundTransform)) {
            DejaGnu.pass("SoundTransform class exists");
        } else {
            DejaGnu.fail("SoundTransform lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Std.is(x1.leftToLeft, Float)) {
	    DejaGnu.pass("SoundTransform::leftToLeft property exists");
	} else {
	    DejaGnu.fail("SoundTransform::leftToLeft property doesn't exist");
	}
	if (Std.is(x1.leftToRight, Float)) {
	    DejaGnu.pass("SoundTransform::leftToRight property exists");
	} else {
	    DejaGnu.fail("SoundTransform::leftToRight property doesn't exist");
	}
	if (Std.is(x1.pan, Float)) {
	    DejaGnu.pass("SoundTransform::pan property exists");
	} else {
	    DejaGnu.fail("SoundTransform::pan property doesn't exist");
	}
	if (Std.is(x1.rightToLeft, Float)) {
	    DejaGnu.pass("SoundTransform::rightToLeft property exists");
	} else {
	    DejaGnu.fail("SoundTransform::rightToLeft property doesn't exist");
	}
	if (Std.is(x1.rightToRight, Float)) {
	    DejaGnu.pass("SoundTransform::rightToRight property exists");
	} else {
	    DejaGnu.fail("SoundTransform::rightToRight property doesn't exist");
	}
	if (Std.is(x1.volume, Float)) {
	    DejaGnu.pass("SoundTransform::volume property exists");
	} else {
	    DejaGnu.fail("SoundTransform::volume property doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (SoundTransform) is only available in flash9");
#end
    }
}
