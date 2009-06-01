// Rectangle_as.hx:  ActionScript 3 "Rectangle" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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
import flash.display.MovieClip;
#end
#if flash8
import flash.MovieClip;
#end
#if !flash6
#if !flash7
import flash.geom.Rectangle;
import flash.geom.Point;
#end
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Rectangle_as {
    static function main() {
#if !flash6
#if !flash7
#if flash9
        var x1:Rectangle = new Rectangle();
#else
		var x1:Rectangle<Int> = new Rectangle(1,1,1,1);
#end
        // Make sure we actually get a valid class        
        if (Std.is(x1, Rectangle)) {
            DejaGnu.pass("Rectangle class exists");
        } else {
            DejaGnu.fail("Rectangle class doesn't exist");
        }
	// Tests to see if all the properties exist. All these do is test for
	// existance of a property, and don't test the functionality at all. This
	// is primarily useful only to test completeness of the API implementation.
		if (Std.is(x1.bottom, Int)) {
			DejaGnu.pass("Rectangle.bottom property exists");
		} else {
			DejaGnu.fail("Rectangle.bottom property doesn't exist");
		}
	 	if (Std.is(x1.bottomRight, Point)) {
	 	    DejaGnu.pass("Rectangle.bottomRight property exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle.bottomRight property doesn't exist");
	 	}
		if (Std.is(x1.height, Int)) {
			DejaGnu.pass("Rectangle.height property exists");
		} else {
			DejaGnu.fail("Rectangle.height property doesn't exist");
		}
		if (Std.is(x1.left, Int)) {
			DejaGnu.pass("Rectangle.left property exists");
		} else {
			DejaGnu.fail("Rectangle.left property doesn't exist");
		}
		if (Std.is(x1.right, Int)) {
			DejaGnu.pass("Rectangle.right property exists");
		} else {
			DejaGnu.fail("Rectangle.right property doesn't exist");
		}
	 	if (Std.is(x1.size, Point)) {
	 	    DejaGnu.pass("Rectangle.size property exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle.size property doesn't exist");
	 	}
		if (Std.is(x1.top, Int)) {
			DejaGnu.pass("Rectangle.top property exists");
		} else {
			DejaGnu.fail("Rectangle.top property doesn't exist");
		}
	 	if (Std.is(x1.topLeft, Point)) {
	 	    DejaGnu.pass("Rectangle.topLeft property exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle.topLeft property doesn't exist");
	 	}
		if (Std.is(x1.width, Int)) {
			DejaGnu.pass("Rectangle.width property exists");
		} else {
			DejaGnu.fail("Rectangle.width property doesn't exist");
		}
		if (Std.is(x1.x, Int)) {
			DejaGnu.pass("Rectangle.x property exists");
		} else {
			DejaGnu.fail("Rectangle.x property doesn't exist");
		}
		if (Std.is(x1.y, Int)) {
			DejaGnu.pass("Rectangle.y property exists");
		} else {
			DejaGnu.fail("Rectangle.y property doesn't exist");
		}

	// Tests to see if all the methods exist. All these do is test for
	// existance of a method, and don't test the functionality at all. This
	// is primarily useful only to test completeness of the API implementation.
	 	if (Type.typeof(x1.clone) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::clone() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::clone() method doesn't exist");
	 	}
		if (Type.typeof(x1.contains) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::contains() method exists");
		} else {
			DejaGnu.fail("Rectangle::contains() method doesn't exist");
		}
	
		if (Type.typeof(x1.containsPoint) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::containsPoint() method exists");
		} else {
			DejaGnu.fail("Rectangle::containsPoint() method doesn't exist");
		}
#if flash9
		if (Type.typeof(x1.containsRect) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::containsRect() method exists");
		} else {
			DejaGnu.fail("Rectangle::containsRect() method doesn't exist");
		}
#else
		if (Type.typeof(x1.containsRectangle) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::containsRect() method exists");
		} else {
			DejaGnu.fail("Rectangle::containsRect() method doesn't exist");
		}
#end
	 	if (Type.typeof(x1.equals) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::equals() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::equals() method doesn't exist");
	 	}
		if (Type.typeof(x1.inflate) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::inflate() method exists");
		} else {
			DejaGnu.fail("Rectangle::inflate() method doesn't exist");
		}
		if (Type.typeof(x1.inflatePoint) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::inflatePoint() method exists");
		} else {
			DejaGnu.fail("Rectangle::inflatePoint() method doesn't exist");
		}
	 	if (Type.typeof(x1.intersection) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::intersection() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::intersection() method doesn't exist");
	 	}
		if (Type.typeof(x1.intersects) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::intersects() method exists");
		} else {
			DejaGnu.fail("Rectangle::intersects() method doesn't exist");
		}
		if (Type.typeof(x1.isEmpty) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::isEmpty() method exists");
		} else {
			DejaGnu.fail("Rectangle::isEmpty() method doesn't exist");
		}
		if (Type.typeof(x1.offset) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::offset() method exists");
		} else {
			DejaGnu.fail("Rectangle::offset() method doesn't exist");
		}
		if (Type.typeof(x1.offsetPoint) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::offsetPoint() method exists");
		} else {
			DejaGnu.fail("Rectangle::offsetPoint() method doesn't exist");
		}
		if (Type.typeof(x1.setEmpty) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::setEmpty() method exists");
		} else {
			DejaGnu.fail("Rectangle::setEmpty() method doesn't exist");
		}
		if (Type.typeof(x1.toString) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::toString() method exists");
		} else {
			DejaGnu.fail("Rectangle::toString() method doesn't exist");
		}
	 	if (Type.typeof(x1.union) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::union() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::union() method doesn't exist");
	 	}

	    // Call this after finishing all tests. It prints out the totals.
	    DejaGnu.done();
#end
#end
#if flash6
	DejaGnu.note("This class (Rectangle) is not available in Flash6");
#end
#if flash7
	DejaGnu.note("This class (Rectangle) is not available in Flash7");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

