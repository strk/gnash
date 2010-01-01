// SoundLoaderContext_as.hx:  ActionScript 3 "SoundLoaderContext" class, for Gnash.
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

// NOTE: works for flash v.9 and greater only!

#if flash9
import flash.media.SoundLoaderContext;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

import DejaGnu;

// Class must be named with the PP prefix, as that's the name the
// file passed to haxe will have after the preprocessing step
class SoundLoaderContext_as {
    static function main() {
#if flash9
        var x1:SoundLoaderContext = new SoundLoaderContext();

        // Make sure we actually get a valid class        
        if (Std.is(x1, SoundLoaderContext)) {
            DejaGnu.pass("SoundLoaderContext class exists");
        } else {
            DejaGnu.fail("SoundLoaderContext lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Std.is(x1.bufferTime, Float)) {
	    DejaGnu.pass("SoundLoaderContext::bufferTime property exists");
	} else {
	    DejaGnu.fail("SoundLoaderContext::bufferTime property doesn't exist");
	}
	if (Std.is(x1.checkPolicyFile, Bool)) {
	    DejaGnu.pass("SoundLoaderContext::checkPolicyFile property exists");
	} else {
	    DejaGnu.fail("SoundLoaderContext::checkPolicyFile property doesn't exist");
	}

// Call this after finishing all tests. It prints out the totals.
	DejaGnu.done();
#else
	DejaGnu.note("This class (SoundLoaderContext) is only available in flash9");
#end
    }
}
