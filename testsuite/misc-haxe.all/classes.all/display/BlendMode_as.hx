// BlendMode_as.hx:  ActionScript 3 "BlendMode" class, for Gnash.
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
import flash.display.BlendMode;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class BlendMode_as {
    static function main() {
#if flash9

        // Make sure we actually get a valid class        
        if (BlendMode.ADD != null) {
            DejaGnu.pass("BlendMode.ADD class exists");
            if (Std.is(BlendMode.ADD, String)) {
            	DejaGnu.pass("BlendMode.ADD is a String");
            	if (Std.string(BlendMode.ADD) == "add") {
            		DejaGnu.pass("BlendMode.ADD is the correct string (add)");
            	} else {
            		DejaGnu.fail("BlendMode.ADD is not the correct string (Should be add, but is "+BlendMode.ADD+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.ADD is not a string. Instead, it is a "+Type.typeof(BlendMode.ADD));
            }
        } else {
            DejaGnu.fail("BlendMode.ADD class doesn't exist");
        }

        if (BlendMode.ALPHA != null) {
            DejaGnu.pass("BlendMode.ALPHA class exists");
            if (Std.is(BlendMode.ALPHA, String)) {
            	DejaGnu.pass("BlendMode.ALPHA is a String");
            	if (Std.string(BlendMode.ALPHA) == "alpha") {
            		DejaGnu.pass("BlendMode.ALPHA is the correct string (alpha)");
            	} else {
            		DejaGnu.fail("BlendMode.ALPHA is not the correct string (Should be alpha, but is "+BlendMode.ALPHA+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.ALPHA is not a string. Instead, it is a "+Type.typeof(BlendMode.ALPHA));
            }
        } else {
            DejaGnu.fail("BlendMode.ALPHA class doesn't exist");
        }

        if (BlendMode.DARKEN != null) {
            DejaGnu.pass("BlendMode.DARKEN class exists");
            if (Std.is(BlendMode.DARKEN, String)) {
            	DejaGnu.pass("BlendMode.DARKEN is a String");
            	if (Std.string(BlendMode.DARKEN) == "darken") {
            		DejaGnu.pass("BlendMode.DARKEN is the correct string (darken)");
            	} else {
            		DejaGnu.fail("BlendMode.DARKEN is not the correct string (Should be darken, but is "+BlendMode.DARKEN+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.DARKEN is not a string. Instead, it is a "+Type.typeof(BlendMode.DARKEN));
            }
        } else {
            DejaGnu.fail("BlendMode.DARKEN class doesn't exist");
        }

        if (BlendMode.DIFFERENCE != null) {
            DejaGnu.pass("BlendMode.DIFFERENCE class exists");
            if (Std.is(BlendMode.DIFFERENCE, String)) {
            	DejaGnu.pass("BlendMode.DIFFERENCE is a String");
            	if (Std.string(BlendMode.DIFFERENCE) == "difference") {
            		DejaGnu.pass("BlendMode.DIFFERENCE is the correct string (difference)");
            	} else {
            		DejaGnu.fail("BlendMode.DIFFERENCE is not the correct string (Should be difference, but is "+BlendMode.DIFFERENCE+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.DIFFERENCE is not a string. Instead, it is a "+Type.typeof(BlendMode.DIFFERENCE));
            }
        } else {
            DejaGnu.fail("BlendMode.DIFFERENCE class doesn't exist");
        }

        if (BlendMode.ERASE != null) {
            DejaGnu.pass("BlendMode.ERASE class exists");
            if (Std.is(BlendMode.ERASE, String)) {
            	DejaGnu.pass("BlendMode.ERASE is a String");
            	if (Std.string(BlendMode.ERASE) == "erase") {
           		DejaGnu.pass("BlendMode.ERASE is the correct string (erase)");
            	} else {
            		DejaGnu.fail("BlendMode.ERASE is not the correct string (Should be erase, but is "+BlendMode.ERASE+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.ERASE is not a string. Instead, it is a "+Type.typeof(BlendMode.ERASE));
            }
        } else {
            DejaGnu.fail("BlendMode.ERASE class doesn't exist");
        }

        if (BlendMode.HARDLIGHT != null) {
            DejaGnu.pass("BlendMode.HARDLIGHT class exists");
            if (Std.is(BlendMode.HARDLIGHT, String)) {
            	DejaGnu.pass("BlendMode.HARDLIGHT is a String");
            	if (Std.string(BlendMode.HARDLIGHT) == "hardlight") {
            		DejaGnu.pass("BlendMode.HARDLIGHT is the correct string (hardlight)");
            	} else {
            		DejaGnu.fail("BlendMode.HARDLIGHT is not the correct string (Should be hardlight, but is "+BlendMode.HARDLIGHT+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.HARDLIGHT is not a string. Instead, it is a "+Type.typeof(BlendMode.HARDLIGHT));
            }
        } else {
            DejaGnu.fail("BlendMode.HARDLIGHT class doesn't exist");
        }

        if (BlendMode.INVERT != null) {
            DejaGnu.pass("BlendMode.INVERT class exists");
            if (Std.is(BlendMode.INVERT, String)) {
            	DejaGnu.pass("BlendMode.INVERT is a String");
            	if (Std.string(BlendMode.INVERT) == "invert") {
            		DejaGnu.pass("BlendMode.INVERT is the correct string (invert)");
            	} else {
            		DejaGnu.fail("BlendMode.HARDLIGHT is not the correct string (Should be invert, but is "+BlendMode.INVERT+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.INVERT is not a string. Instead, it is a "+Type.typeof(BlendMode.INVERT));
            }
        } else {
            DejaGnu.fail("BlendMode.INVERT class doesn't exist");
        }

        if (BlendMode.LAYER != null) {
            DejaGnu.pass("BlendMode.LAYER class exists");
            if (Std.is(BlendMode.INVERT, String)) {
            	DejaGnu.pass("BlendMode.LAYER is a String");
            	if (Std.string(BlendMode.LAYER) == "layer") {
            		DejaGnu.pass("BlendMode.LAYER is the correct string (layer)");
            	} else {
            		DejaGnu.fail("BlendMode.LAYER is not the correct string (Should be layer, but is "+BlendMode.LAYER+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.LAYER is not a string. Instead, it is a "+Type.typeof(BlendMode.LAYER));
            }
        } else {
            DejaGnu.fail("BlendMode.LAYER class doesn't exist");
        }

        if (BlendMode.LIGHTEN != null) {
            DejaGnu.pass("BlendMode.LIGHTEN class exists");
            if (Std.is(BlendMode.LIGHTEN, String)) {
            	DejaGnu.pass("BlendMode.LIGHTEN is a String");
            	if (Std.string(BlendMode.LIGHTEN) == "lighten") {
            		DejaGnu.pass("BlendMode.LIGHTEN is the correct string (lighten)");
            	} else {
            		DejaGnu.fail("BlendMode.LIGHTEN is not the correct string (Should be lighten, but is "+BlendMode.LIGHTEN+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.LIGHTEN is not a string. Instead, it is a "+Type.typeof(BlendMode.LIGHTEN));
            }
        } else {
            DejaGnu.fail("BlendMode.LIGHTEN class doesn't exist");
        }

        if (BlendMode.MULTIPLY != null) {
            DejaGnu.pass("BlendMode.MULTIPLY class exists");
            if (Std.is(BlendMode.MULTIPLY, String)) {
            	DejaGnu.pass("BlendMode.MULTIPLY is a String");
            	if (Std.string(BlendMode.MULTIPLY) == "multiply") {
            		DejaGnu.pass("BlendMode.MULTIPLY is the correct string (multiply)");
            	} else {
            		DejaGnu.fail("BlendMode.MULTIPLY is not the correct string (Should be multiply, but is "+BlendMode.MULTIPLY+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.MULTIPLY is not a string. Instead, it is a "+Type.typeof(BlendMode.MULTIPLY));
            }
        } else {
            DejaGnu.fail("BlendMode.MULTIPLY class doesn't exist");
        }

        if (BlendMode.NORMAL != null) {
            DejaGnu.pass("BlendMode.NORMAL class exists");
            if (Std.is(BlendMode.NORMAL, String)) {
            	DejaGnu.pass("BlendMode.NORMAL is a String");
            	if (Std.string(BlendMode.NORMAL) == "normal") {
            		DejaGnu.pass("BlendMode.NORMAL is the correct string (normal)");
            	} else {
            		DejaGnu.fail("BlendMode.NORMAL is not the correct string (Should be normal, but is "+BlendMode.NORMAL+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.NORMAL is not a string. Instead, it is a "+Type.typeof(BlendMode.NORMAL));
            }
        } else {
            DejaGnu.fail("BlendMode.NORMAL class doesn't exist");
        }

        if (BlendMode.OVERLAY != null) {
            DejaGnu.pass("BlendMode.OVERLAY class exists");
            if (Std.is(BlendMode.OVERLAY, String)) {
            	DejaGnu.pass("BlendMode.OVERLAY is a String");
            	if (Std.string(BlendMode.OVERLAY) == "overlay") {
            		DejaGnu.pass("BlendMode.OVERLAY is the correct string (overlay)");
            	} else {
            		DejaGnu.fail("BlendMode.OVERLAY is not the correct string (Should be overlay, but is "+BlendMode.OVERLAY+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.OVERLAY is not a string. Instead, it is a "+Type.typeof(BlendMode.OVERLAY));
            }
        } else {
            DejaGnu.fail("BlendMode.OVERLAY class doesn't exist");
        }

        if (BlendMode.SCREEN != null) {
            DejaGnu.pass("BlendMode.SCREEN class exists");
            if (Std.is(BlendMode.SCREEN, String)) {
            	DejaGnu.pass("BlendMode.SCREEN is a String");
            	if (Std.string(BlendMode.SCREEN) == "screen") {
            		DejaGnu.pass("BlendMode.SCREEN is the correct string (screen)");
            	} else {
            		DejaGnu.fail("BlendMode.SCREEN is not the correct string (Should be screen, but is "+BlendMode.SCREEN+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.SCREEN is not a string. Instead, it is a "+Type.typeof(BlendMode.SCREEN));
            }
        } else {
            DejaGnu.fail("BlendMode.SCREEN class doesn't exist");
        }

        if (BlendMode.SUBTRACT != null) {
            DejaGnu.pass("BlendMode.SUBTRACT class exists");
            if (Std.is(BlendMode.SUBTRACT, String)) {
            	DejaGnu.pass("BlendMode.SUBTRACT is a String");
            	if (Std.string(BlendMode.SUBTRACT) == "subtract") {
            		DejaGnu.pass("BlendMode.SUBTRACT is the correct string (subtract)");
            	} else {
            		DejaGnu.fail("BlendMode.SUBTRACT is not the correct string (Should be subtract, but is "+BlendMode.SUBTRACT+")");
            	}
            } else {
            	DejaGnu.fail("BlendMode.SUBTRACT is not a string. Instead, it is a "+Type.typeof(BlendMode.SUBTRACT));
            }
        } else {
            DejaGnu.fail("BlendMode.SUBTRACT class doesn't exist");
        }

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (BlendMode) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

