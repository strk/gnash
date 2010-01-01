// SWFVersion_as.hx:  ActionScript 3 "SWFVersion" class, for Gnash.
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
import flash.display.SWFVersion;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class SWFVersion_as {
    static function main() {
#if flash9
        // Make sure we actually get a valid class        	

		if (Type.typeof(SWFVersion.FLASH1) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH1 is an int");
        	if (Std.string(SWFVersion.FLASH1) == "1") {
        		DejaGnu.pass("SWFVersion.FLASH1 is the correct int (1)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH1 is not the correct int (Should be 1, but is "+SWFVersion.FLASH1+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH1 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH1));
		}
		if (Type.typeof(SWFVersion.FLASH2) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH2 is an int");
        	if (Std.string(SWFVersion.FLASH2) == "2") {
        		DejaGnu.pass("SWFVersion.FLASH2 is the correct int (2)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH2 is not the correct int (Should be 2, but is "+SWFVersion.FLASH2+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH2 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH2));
		}
		if (Type.typeof(SWFVersion.FLASH3) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH3 is an int");
        	if (Std.string(SWFVersion.FLASH3) == "3") {
        		DejaGnu.pass("SWFVersion.FLASH3 is the correct int (3)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH3 is not the correct int (Should be 3, but is "+SWFVersion.FLASH3+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH3 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH3));
		}
		if (Type.typeof(SWFVersion.FLASH4) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH4 is an int");
        	if (Std.string(SWFVersion.FLASH4) == "4") {
        		DejaGnu.pass("SWFVersion.FLASH4 is the correct int (4)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH4 is not the correct int (Should be 4, but is "+SWFVersion.FLASH4+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH4 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH4));
		}
		if (Type.typeof(SWFVersion.FLASH5) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH5 is an int");
        	if (Std.string(SWFVersion.FLASH5) == "5") {
        		DejaGnu.pass("SWFVersion.FLASH5 is the correct int (5)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH5 is not the correct int (Should be 5, but is "+SWFVersion.FLASH5+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH5 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH5));
		}
		
		if (Type.typeof(SWFVersion.FLASH6) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH6 is an int");
        	if (Std.string(SWFVersion.FLASH6) == "6") {
        		DejaGnu.pass("SWFVersion.FLASH6 is the correct int (6)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH6 is not the correct int (Should be 6, but is "+SWFVersion.FLASH6+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH6 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH6));
		}
		
		if (Type.typeof(SWFVersion.FLASH7) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH7 is an int");
        	if (Std.string(SWFVersion.FLASH7) == "7") {
        		DejaGnu.pass("SWFVersion.FLASH7 is the correct int (7)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH7 is not the correct int (Should be 7, but is "+SWFVersion.FLASH7+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH7 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH7));
		}
		
		if (Type.typeof(SWFVersion.FLASH8) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH8 is an int");
        	if (Std.string(SWFVersion.FLASH8) == "8") {
        		DejaGnu.pass("SWFVersion.FLASH8 is the correct int (8)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH8 is not the correct int (Should be 8, but is "+SWFVersion.FLASH8+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH8 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH8));
		}
		
		if (Type.typeof(SWFVersion.FLASH9) == ValueType.TInt) {
        	DejaGnu.pass("SWFVersion.FLASH9 is an int");
        	if (Std.string(SWFVersion.FLASH9) == "9") {
        		DejaGnu.pass("SWFVersion.FLASH9 is the correct int (9)");
        	} else {
        		DejaGnu.fail("SWFVersion.FLASH9 is not the correct int (Should be 9, but is "+SWFVersion.FLASH9+")");
        	}
		} else {
       		DejaGnu.fail("SWFVersion.FLASH9 is not an int. Instead, it is a "+Type.typeof(SWFVersion.FLASH9));
		}


// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (SWFVersion) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

