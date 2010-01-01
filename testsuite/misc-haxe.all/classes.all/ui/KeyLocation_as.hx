// KeyLocation_as.hx:  ActionScript 3 "KeyLocation" class, for Gnash.
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
import flash.ui.KeyLocation;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;


// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class KeyLocation_as {
    static function main() {
#if flash9

// Tests to see if all the constants exist.
//	if (KeyLocation.STANDARD != null) {
//        DejaGnu.pass("KeyLocation.STANDARD constant exists");
        if (Type.typeof(KeyLocation.STANDARD) == ValueType.TInt) {
        	DejaGnu.pass("KeyLocation.STANDARD is an int");
        	if (Std.string(KeyLocation.STANDARD) == "0") {
        		DejaGnu.pass("KeyLocation.STANDARD is the correct int (0)");
        	} else {
        		DejaGnu.fail("KeyLocation.STANDARD is not the correct int (Should be 0, but is "+KeyLocation.STANDARD+")");
        	}
        } else {
        	DejaGnu.fail("KeyLocation.STANDARD is not an int. Instead, it is a "+Type.typeof(KeyLocation.STANDARD));
        }

//    } else {
//        DejaGnu.fail("KeyLocation.STANDARD constant doesn't exist");
//    }
    
//	if (KeyLocation.LEFT != null) {
//        DejaGnu.pass("KeyLocation.LEFT constant exists");
        if (Type.typeof(KeyLocation.LEFT) == ValueType.TInt) {
        	DejaGnu.pass("KeyLocation.LEFT is an int");
        	if (Std.string(KeyLocation.LEFT) == "1") {
        		DejaGnu.pass("KeyLocation.LEFT is the correct int (1)");
        	} else {
        		DejaGnu.fail("KeyLocation.LEFT is not the correct int (Should be 1, but is "+KeyLocation.LEFT+")");
        	}
        } else {
        	DejaGnu.fail("KeyLocation.LEFT is not an int. Instead, it is a "+Type.typeof(KeyLocation.LEFT));
        }
//    } else {
//        DejaGnu.fail("KeyLocation.LEFT constant doesn't exist");
//    }
    
//	if (KeyLocation.RIGHT != null) {
//        DejaGnu.pass("KeyLocation.RIGHT constant exists");
        if (Type.typeof(KeyLocation.RIGHT) == ValueType.TInt) {
        	DejaGnu.pass("KeyLocation.RIGHT is an int");
        	if (Std.string(KeyLocation.RIGHT) == "2") {
        		DejaGnu.pass("KeyLocation.RIGHT is the correct int (2)");
        	} else {
        		DejaGnu.fail("KeyLocation.RIGHT is not the correct int (Should be 2, but is "+KeyLocation.RIGHT+")");
        	}
        } else {
        	DejaGnu.fail("KeyLocation.RIGHT is not an int. Instead, it is a "+Type.typeof(KeyLocation.RIGHT));
        }
//    } else {
//        DejaGnu.fail("KeyLocation.RIGHT constant doesn't exist");
//    }

//	if (KeyLocation.NUM_PAD != null) {
//        DejaGnu.pass("KeyLocation.NUM_PAD constant exists");
        if (Type.typeof(KeyLocation.NUM_PAD) == ValueType.TInt) {
        	DejaGnu.pass("KeyLocation.NUM_PAD is an int");
        	if (Std.string(KeyLocation.NUM_PAD) == "3") {
        		DejaGnu.pass("KeyLocation.NUM_PAD is the correct int (3)");
        	} else {
        		DejaGnu.fail("KeyLocation.NUM_PAD is not the correct int (Should be 3, but is "+KeyLocation.NUM_PAD+")");
        	}
        } else {
        	DejaGnu.fail("KeyLocation.NUM_PAD is not an int. Instead, it is a "+Type.typeof(KeyLocation.NUM_PAD));
        }
//    } else {
//        DejaGnu.fail("KeyLocation.NUM_PAD constant doesn't exist");
//    }  
	
        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (KeyLocation) is only available in flash9");
#end
    }
}
    
// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

