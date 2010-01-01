// Timer_as.hx:  ActionScript 3 "Timer" class, for Gnash.
//
// Generated on: 20090602 by "bnaugle". Remove this
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
import flash.utils.Timer;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Timer_as {
    static function main() {
#if flash9
        var x1:Timer = new Timer(0,0);

        // Make sure we actually get a valid class        
        if (Std.is(x1, Timer)) {
            DejaGnu.pass("Timer class exists");
        } else {
            DejaGnu.fail("Timer class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.currentCount) == ValueType.TInt) {
	    DejaGnu.pass("Timer.currentCount property exists");
	} else {
	    DejaGnu.fail("Timer.currentCount property doesn't exist");
	}
	if (Type.typeof(x1.delay) == ValueType.TInt) {
	    DejaGnu.pass("Timer.delay property exists");
	} else {
	    DejaGnu.fail("Timer.delay property doesn't exist");
	}
	if (Type.typeof(x1.repeatCount) == ValueType.TInt) {
	    DejaGnu.pass("Timer.repeatCount property exists");
	} else {
	    DejaGnu.fail("Timer.repeatCount property doesn't exist");
	}
	if (Type.typeof(x1.running) == ValueType.TBool) {
	    DejaGnu.pass("Timer.running property exists");
	} else {
	    DejaGnu.fail("Timer.running property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.reset) == ValueType.TFunction) {
	    DejaGnu.pass("Timer::reset() method exists");
	} else {
	    DejaGnu.fail("Timer::reset() method doesn't exist");
	}
	if (Type.typeof(x1.start) == ValueType.TFunction) {
	    DejaGnu.pass("Timer::start() method exists");
	} else {
	    DejaGnu.fail("Timer::start() method doesn't exist");
	}
	if (Type.typeof(x1.stop) == ValueType.TFunction) {
	    DejaGnu.pass("Timer::stop() method exists");
	} else {
	    DejaGnu.fail("Timer::stop() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (Timer) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

