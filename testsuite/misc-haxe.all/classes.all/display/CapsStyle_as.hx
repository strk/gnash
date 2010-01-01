// CapsStyle_as.hx:  ActionScript 3 "CapsStyle" class, for Gnash.
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
import flash.display.CapsStyle;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class CapsStyle_as {
    static function main() {
#if flash9
	if (CapsStyle.NONE != null) {
            DejaGnu.pass("CapsStyle.NONE constant exists");
            if (Std.is(CapsStyle.NONE, String)) {
            	DejaGnu.pass("CapsStyle.NONE is a String");
            	if (Std.string(CapsStyle.NONE) == "none") {
            		DejaGnu.pass("CapsStyle.NONE is the correct string (none)");
            	} else {
            		DejaGnu.fail("CapsStyle.NONE is not the correct string (Should be none, but is "+CapsStyle.NONE+")");
            	}
            } else {
            	DejaGnu.fail("CapsStyle.NONE is not a string. Instead, it is a "+Type.typeof(CapsStyle.NONE));
            }
        } else {
            DejaGnu.fail("CapsStyle.NONE constant doesn't exist");
        }

	if (CapsStyle.ROUND != null) {
            DejaGnu.pass("CapsStyle.ROUND constant exists");
            if (Std.is(CapsStyle.ROUND, String)) {
            	DejaGnu.pass("CapsStyle.ROUND is a String");
            	if (Std.string(CapsStyle.ROUND) == "round") {
            		DejaGnu.pass("CapsStyle.ROUND is the correct string (round)");
            	} else {
            		DejaGnu.fail("CapsStyle.ROUND is not the correct string (Should be round, but is "+CapsStyle.ROUND+")");
            	}
            } else {
            	DejaGnu.fail("CapsStyle.ROUND is not a string. Instead, it is a "+Type.typeof(CapsStyle.ROUND));
            }
        } else {
            DejaGnu.fail("CapsStyle.ROUND constant doesn't exist");
        }

	if (CapsStyle.SQUARE != null) {
            DejaGnu.pass("CapsStyle.SQUARE constant exists");
            if (Std.is(CapsStyle.SQUARE, String)) {
            	DejaGnu.pass("CapsStyle.SQUARE is a String");
            	if (Std.string(CapsStyle.SQUARE) == "square") {
            		DejaGnu.pass("CapsStyle.SQUARE is the correct string (square)");
            	} else {
            		DejaGnu.fail("CapsStyle.SQUARE is not the correct string (Should be square, but is "+CapsStyle.SQUARE+")");
            	}
            } else {
            	DejaGnu.fail("CapsStyle.SQUARE is not a string. Instead, it is a "+Type.typeof(CapsStyle.SQUARE));
            }
        } else {
            DejaGnu.fail("CapsStyle.SQUARE constant doesn't exist");
        }

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (CapsStyle) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

