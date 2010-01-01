// SharedObjectFlushStatus_as.hx:  ActionScript 3 "SharedObjectFlushStatus" class, for Gnash.
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
import flash.net.SharedObjectFlushStatus;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class SharedObjectFlushStatus_as {
    static function main() {
#if flash9
        // Make sure we actually get a valid class        

        if (SharedObjectFlushStatus.FLUSHED != null) {
            DejaGnu.pass("SharedObjectFlushStatus.FLUSHED constant exists");
            if (Std.is(SharedObjectFlushStatus.FLUSHED, String)) {
            	DejaGnu.pass("SharedObjectFlushStatus.FLUSHED is a String");
            	if (Std.string(SharedObjectFlushStatus.FLUSHED) == "flushed") {
            		DejaGnu.pass("SharedObjectFlushStatus.FLUSHED is the correct string (flushed)");
            	} else {
            		DejaGnu.fail(
            		"SharedObjectFlushStatus.FLUSHED is not the correct string (Should be flushed, but is "+SharedObjectFlushStatus.FLUSHED+")");
            	}
            } else {
            	DejaGnu.fail("SharedObjectFlushStatus.FLUSHED is not a string. Instead, it is a "+Type.typeof(SharedObjectFlushStatus.FLUSHED));
            }
        } else {
            DejaGnu.fail("SharedObjectFlushStatus.FLUSHED constant doesn't exist");
        }

        if (SharedObjectFlushStatus.PENDING != null) {
            DejaGnu.pass("SharedObjectFlushStatus.PENDING constant exists");
            if (Std.is(SharedObjectFlushStatus.PENDING, String)) {
            	DejaGnu.pass("SharedObjectFlushStatus.PENDING is a String");
            	if (Std.string(SharedObjectFlushStatus.PENDING) == "pending") {
            		DejaGnu.pass("SharedObjectFlushStatus.PENDING is the correct string (pending)");
            	} else {
            		DejaGnu.fail(
            		"SharedObjectFlushStatus.PENDING is not the correct string (Should be pending, but is "+SharedObjectFlushStatus.PENDING+")");
            	}
            } else {
            	DejaGnu.fail("SharedObjectFlushStatus.PENDING is not a string. Instead, it is a "+Type.typeof(SharedObjectFlushStatus.PENDING));
            }
        } else {
            DejaGnu.fail("SharedObjectFlushStatus.PENDING constant doesn't exist");
        }

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (SharedObjectFlushStatus) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

