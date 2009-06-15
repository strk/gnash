// Point_as.hx:  ActionScript 3 "Point" class, for Gnash.
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
#if !(flash6 || flash7)
import flash.geom.Point;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Point_as {
    static function main() {
#if !(flash6 || flash7)
#if flash9
	var x1:Point = new Point();
#else
	var x1:Point<Int> = new Point(0,0);
#end
	if (Std.is(x1, Point)) {
		DejaGnu.pass("Point class exists");
	} else {
		DejaGnu.fail("Point class doesn't exist");
	}
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.length) == ValueType.TInt) {
	    DejaGnu.pass("Point.length property exists");
	} else {
	    DejaGnu.fail("Point.length property doesn't exist");
	}
	if (Type.typeof(x1.x) == ValueType.TInt) {
	    DejaGnu.pass("Point.x property exists");
	} else {
	    DejaGnu.fail("Point.x property doesn't exist");
	}
	if (Type.typeof(x1.y) == ValueType.TInt) {
	    DejaGnu.pass("Point.y property exists");
	} else {
	    DejaGnu.fail("Point.y property doesn't exist");
	}
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.normalize) == ValueType.TFunction) {
	    DejaGnu.pass("Point::normalize() method exists");
	} else {
	    DejaGnu.fail("Point::normalize() method doesn't exist");
	}
	if (Type.typeof(x1.offset) == ValueType.TFunction) {
	    DejaGnu.pass("Point::offset() method exists");
	} else {
	    DejaGnu.fail("Point::offset() method doesn't exist");
	}
	if (Type.typeof(x1.toString) == ValueType.TFunction) {
	    DejaGnu.pass("Point::toString() method exists");
	} else {
	    DejaGnu.fail("Point::toString() method doesn't exist");
	}
 	if (Type.typeof(x1.add) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::add() method exists");
 	} else {
 	    DejaGnu.fail("Point::add() method doesn't exist");
 	}
 	if (Type.typeof(x1.clone) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::clone() method exists");
 	} else {
 	    DejaGnu.fail("Point::clone() method doesn't exist");
 	}
 	if (Type.typeof(x1.equals) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::equals() method exists");
 	} else {
 	    DejaGnu.fail("Point::equals() method doesn't exist");
 	}
 	if (Type.typeof(x1.subtract) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::subtract() method exists");
 	} else {
 	    DejaGnu.fail("Point::subtract() method doesn't exist");
 	}
 	if (Type.typeof(Point.polar) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::polar() method exists");
 	} else {
 	    DejaGnu.fail("Point::polar() method doesn't exist");
 	}
 	if (Type.typeof(Point.interpolate) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::interpolate() method exists");
 	} else {
 	    DejaGnu.fail("Point::interpolate() method doesn't exist");
 	}
 	if (Type.typeof(Point.distance) == ValueType.TFunction) {
 	    DejaGnu.pass("Point::distance() method exists");
 	} else {
 	    DejaGnu.fail("Point::distance() method doesn't exist");
 	}
        // Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
#else
    DejaGnu.note("This class (Matrix) is only available in flash8 and flash9");
#end
    }
}



// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

