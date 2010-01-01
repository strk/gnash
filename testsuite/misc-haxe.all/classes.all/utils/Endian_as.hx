// Endian_as.hx:  ActionScript 3 "Endian" class, for Gnash.
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
import flash.utils.Endian;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Endian_as {
    static function main() {
#if flash9

//FIXME: If test fails, remember Adobe spec says string should be bigEndian, but haXe may say big_endian
		if (Endian.BIG_ENDIAN != null) {
            DejaGnu.pass("Endian.BIG_ENDIAN class exists");
            if (Std.is(Endian.BIG_ENDIAN, String)) {
            	DejaGnu.pass("Endian.BIG_ENDIAN is a String");
            	if (Std.string(Endian.BIG_ENDIAN) == "bigEndian") {
            		DejaGnu.pass("Endian.BIG_ENDIAN is the correct string (bigEndian)");
            	} else {
            		DejaGnu.fail("Endian.BIG_ENDIAN is not the correct string (Should be bigEndian, but is "+Endian.BIG_ENDIAN+")");
            	}
            } else {
            	DejaGnu.fail("Endian.BIG_ENDIAN is not a string. Instead, it is a "+Type.typeof(Endian.BIG_ENDIAN));
            }
        } else {
            DejaGnu.fail("Endian.BIG_ENDIAN class doesn't exist");
        }
        
        if (Endian.LITTLE_ENDIAN != null) {
            DejaGnu.pass("Endian.LITTLE_ENDIAN class exists");
            if (Std.is(Endian.BIG_ENDIAN, String)) {
            	DejaGnu.pass("Endian.LITTLE_ENDIAN is a String");
            	if (Std.string(Endian.LITTLE_ENDIAN) == "littleEndian") {
            		DejaGnu.pass("Endian.LITTLE_ENDIAN is the correct string (littleEndian)");
            	} else {
            		DejaGnu.fail("Endian.LITTLE_ENDIAN is not the correct string (Should be littleEndian, but is "+Endian.LITTLE_ENDIAN+")");
            	}
            } else {
            	DejaGnu.fail("Endian.LITTLE_ENDIAN is not a string. Instead, it is a "+Type.typeof(Endian.LITTLE_ENDIAN));
            }
        } else {
            DejaGnu.fail("Endian.LITTLE_ENDIAN class doesn't exist");
        }

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (Endian) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

