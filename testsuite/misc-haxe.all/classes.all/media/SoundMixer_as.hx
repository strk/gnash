// SoundMixer_as.hx:  ActionScript 3 "SoundMixer" class, for Gnash.
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

// Note: only works with flash v9 and above

#if flash9
import flash.media.SoundMixer;
import flash.media.SoundTransform;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// Class must be named with the PP prefix, as that's the name the
// file passed to haxe will have after the preprocessing step
class SoundMixer_as {
    static function main() {
#if flash9

    // SoundMixer doesn't contain a constructor, all it's methods are static
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Std.is(SoundMixer.bufferTime, Int)) {
	    DejaGnu.pass("SoundMixer::bufferTime property exists");
	} else {
	    DejaGnu.fail("SoundMixer::bufferTime property doesn't exist");
	}
	if (Std.is(SoundMixer.soundTransform, SoundTransform)) {
	    DejaGnu.pass("SoundMixer::soundTransform property exists");
	} else {
	    DejaGnu.fail("SoundMixer::soundTransform property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(SoundMixer.areSoundsInaccessible) == ValueType.TFunction) {
	    DejaGnu.pass("SoundMixer::areSoundsInaccessible() method exists");
	} else {
	    DejaGnu.fail("SoundMixer::areSoundsInaccessible() method doesn't exist");
	}
	if (Type.typeof(SoundMixer.computeSpectrum) == ValueType.TFunction) {
	    DejaGnu.pass("SoundMixer::computeSpectrum() method exists");
	} else {
	    DejaGnu.fail("SoundMixer::computeSpectrum() method doesn't exist");
	}
	if (Type.typeof(SoundMixer.stopAll) == ValueType.TFunction) {
	    DejaGnu.pass("SoundMixer::stopAll() method exists");
	} else {
	    DejaGnu.fail("SoundMixer::stopAll() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (SoundMixer) is only available in flash9");
#end
    }
}
