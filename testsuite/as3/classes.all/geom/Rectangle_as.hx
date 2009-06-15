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
#if (flash8 || flash9)
import flash.geom.Rectangle;
import flash.geom.Point;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Rectangle_as {
    static function main() {
#if (flash8 || flash9)
		//check_equals(typeof(Rectangle), 'function');
		if(Type.typeof(untyped Rectangle) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle is a function");
		} else {
			DejaGnu.fail("Rectangle is not a function");
		}
		//check_equals(typeof(Rectangle.prototype), 'object');
		if(Type.typeof(untyped Rectangle.prototype) == ValueType.TObject) {
			DejaGnu.pass("Rectangle prototype is an object");
		} else {
			DejaGnu.fail("Rectangle prototype is not an object");
		}
		//check(Rectangle.prototype.hasOwnProperty('bottom'));
		if(untyped Rectangle.prototype.hasOwnProperty('bottom')) {
			DejaGnu.pass("Rectangle prototype has 'bottom' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'bottom' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('bottomRight'));
		if(untyped Rectangle.prototype.hasOwnProperty('bottomRight')) {
			DejaGnu.pass("Rectangle prototype has 'bottomRight' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'bottomRight' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('left'));
		if(untyped Rectangle.prototype.hasOwnProperty('left')) {
			DejaGnu.pass("Rectangle prototype has 'left' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'left' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('right'));
		if(untyped Rectangle.prototype.hasOwnProperty('right')) {
			DejaGnu.pass("Rectangle prototype has 'right' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'right' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('size'));
		if(untyped Rectangle.prototype.hasOwnProperty('size')) {
			DejaGnu.pass("Rectangle prototype has 'size' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'size' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('top'));
		if(untyped Rectangle.prototype.hasOwnProperty('top')) {
			DejaGnu.pass("Rectangle prototype has 'top' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'top' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('topLeft'));
		if(untyped Rectangle.prototype.hasOwnProperty('topLeft')) {
			DejaGnu.pass("Rectangle prototype has 'topLeft' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'topLeft' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('clone'));
		if(untyped Rectangle.prototype.hasOwnProperty('clone')) {
			DejaGnu.pass("Rectangle prototype has 'clone' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'clone' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('contains'));
		if(untyped Rectangle.prototype.hasOwnProperty('contains')) {
			DejaGnu.pass("Rectangle prototype has 'contains' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'contains' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('containsPoint'));
		if(untyped Rectangle.prototype.hasOwnProperty('containsPoint')) {
			DejaGnu.pass("Rectangle prototype has 'containsPoint' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'containsPoint' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('containsRectangle'));
		if(untyped Rectangle.prototype.hasOwnProperty('containsRectangle')) {
			DejaGnu.pass("Rectangle prototype has 'containsRectangle' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'containsRectangle' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('equals'));
		if(untyped Rectangle.prototype.hasOwnProperty('equals')) {
			DejaGnu.pass("Rectangle prototype has 'equals' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'equals' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('inflate'));
		if(untyped Rectangle.prototype.hasOwnProperty('inflate')) {
			DejaGnu.pass("Rectangle prototype has 'inflate' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'inflate' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('inflatePoint'));
		if(untyped Rectangle.prototype.hasOwnProperty('inflatePoint')) {
			DejaGnu.pass("Rectangle prototype has 'inflatePoint' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'inflatePoint' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('intersection'));
		if(untyped Rectangle.prototype.hasOwnProperty('intersection')) {
			DejaGnu.pass("Rectangle prototype has 'intersection' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'intersection' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('intersects'));
		if(untyped Rectangle.prototype.hasOwnProperty('intersects')) {
			DejaGnu.pass("Rectangle prototype has 'intersects' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'intersects' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('isEmpty'));
		if(untyped Rectangle.prototype.hasOwnProperty('isEmpty')) {
			DejaGnu.pass("Rectangle prototype has 'isEmpty' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'isEmpty' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('offset'));
		if(untyped Rectangle.prototype.hasOwnProperty('offset')) {
			DejaGnu.pass("Rectangle prototype has 'offset' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'offset' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('offsetPoint'));
		if(untyped Rectangle.prototype.hasOwnProperty('offsetPoint')) {
			DejaGnu.pass("Rectangle prototype has 'offsetPoint' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'offsetPoint' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('setEmpty'));
		if(untyped Rectangle.prototype.hasOwnProperty('setEmpty')) {
			DejaGnu.pass("Rectangle prototype has 'setEmpty' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'setEmpty' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('toString'));
		if(untyped Rectangle.prototype.hasOwnProperty('toString')) {
			DejaGnu.pass("Rectangle prototype has 'toString' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'toString' property");
		}
		//check(Rectangle.prototype.hasOwnProperty('union'));
		if(untyped Rectangle.prototype.hasOwnProperty('union')) {
			DejaGnu.pass("Rectangle prototype has 'union' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'union' property");
		}
		//check(!Rectangle.prototype.hasOwnProperty('height'));
		if(!(untyped Rectangle.prototype.hasOwnProperty('height'))) {
			DejaGnu.pass("Rectangle prototype has 'height' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'height' property");
		}
		//check(!Rectangle.prototype.hasOwnProperty('width'));
		if(!(untyped Rectangle.prototype.hasOwnProperty('width'))) {
			DejaGnu.pass("Rectangle prototype has 'width' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'width' property");
		}
		//check(!Rectangle.prototype.hasOwnProperty('x'));
		if(!(untyped Rectangle.prototype.hasOwnProperty('x'))) {
			DejaGnu.pass("Rectangle prototype has 'x' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'x' property");
		}
		//check(!Rectangle.prototype.hasOwnProperty('y'));
		if(!(untyped Rectangle.prototype.hasOwnProperty('y'))) {
			DejaGnu.pass("Rectangle prototype has 'y' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'y' property");
		}

		//-------------------------------------------------------------
		// Test constructor (and width, height, x, y)
		//-------------------------------------------------------------

#if flash9
        var x1:Rectangle = new Rectangle();
        DejaGnu.note("var x1:Rectangle = new Rectangle();");
#else
		var x1:Rectangle<Int> = new Rectangle();
		DejaGnu.note("var x1:Rectangle<Int> = new Rectangle();");
#end
		//check_equals(typeof(x1), 'object');
		if (Type.typeof(x1) == ValueType.TObject) {
			DejaGnu.pass("new Rectangle() returns an object");
		} else {
			DejaGnu.fail("new Rectangle() does not return an object");
		}
		//check(x1 instanceof Rectangle);
		if (Std.is(x1, Rectangle)) {
            DejaGnu.pass("new Rectangle() returns a Rectangle object");
        } else {
            DejaGnu.fail("new Rectangle() does not return a Rectangle object");
        }
		//check(x1.hasOwnProperty('height'));
		if(untyped x1.hasOwnProperty('height')) {
			DejaGnu.pass("Rectangle object has 'height' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'height' property");
		}
		//check(x1.hasOwnProperty('width'));
		if(untyped x1.hasOwnProperty('width')) {
			DejaGnu.pass("Rectangle object has 'width' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'width' property");
		}
		//check(x1.hasOwnProperty('x'));
		if(untyped x1.hasOwnProperty('x')) {
			DejaGnu.pass("Rectangle object has 'x' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'x' property");
		}
		//check(x1.hasOwnProperty('y'));
		if(untyped x1.hasOwnProperty('y')) {
			DejaGnu.pass("Rectangle object has 'y' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'y' property");
		}
		//check_equals(''+x1, '(x=0, y=0, w=0, h=0)');
		if (x1.toString() == "(x=0, y=0, w=0, h=0)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=0, y=0, w=0, h=0)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=0, y=0, w=0, h=0), is "+x1.toString()+")");
		}
		//check(x1.isEmpty());
		if (x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		if(Type.typeof(x1.x) == ValueType.TInt) {
			DejaGnu.pass("Rectangle object .x is a number");
		} else {
			DejaGnu.fail("Rectangle object .x is not a number");
		}
		if(Type.typeof(x1.y) == ValueType.TInt) {
			DejaGnu.pass("Rectangle object .y is a number");
		} else {
			DejaGnu.fail("Rectangle object .y is not a number");
		}
		if(Type.typeof(x1.width) == ValueType.TInt) {
			DejaGnu.pass("Rectangle object .width is a number");
		} else {
			DejaGnu.fail("Rectangle object .width is not a number");
		}
		if(Type.typeof(x1.height) == ValueType.TInt) {
			DejaGnu.pass("Rectangle object .height is a number");
		} else {
			DejaGnu.fail("Rectangle object .height is not a number");
		}
		x1 = new Rectangle(1);
		DejaGnu.note("x1 = new Rectangle(1);");
		if(untyped x1.hasOwnProperty('height')) {
			DejaGnu.pass("Rectangle object has 'height' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'height' property");
		}
		if(untyped x1.hasOwnProperty('width')) {
			DejaGnu.pass("Rectangle object has 'width' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'width' property");
		}
		if(untyped x1.hasOwnProperty('x')) {
			DejaGnu.pass("Rectangle object has 'x' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'x' property");
		}
		if(untyped x1.hasOwnProperty('y')) {
			DejaGnu.pass("Rectangle object has 'y' property");
		} else {
			DejaGnu.fail("Rectangle object does not have 'y' property");
		}
		if (x1.toString() == "(x=1, y=undefined, w=undefined, h=undefined)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=1, y=undefined, w=undefined, h=undefined)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=1, y=undefined, w=undefined, h=undefined), is "+x1.toString()+")");
		}
		if (x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		//x1 = new Rectangle(1, 1, 1, "string");
		//DejaGnu.note("x1 = new Rectangle(1, 1, 1, "string");");
		//check_equals(''+x1, '(x=1, y=1, w=1, h=string)');
		//if (x1.toString() == "(x=1, y=1, w=1, h=string)") {
		//	DejaGnu.pass("Rectangle object is correct rectangle (x=1, y=1, w=1, h=string)");
		//} else {
		//	DejaGnu.fail(
		//	"Rectangle property is incorrect rectangle (should be (x=1, y=1, w=1, h=string), is "+x1.toString()+")");
		//}
		//check(x1.isEmpty());
		//if (x1.isEmpty()) {
		//	DejaGnu.pass("Rectangle object is empty");
		//} else {
		//	DejaGnu.fail("Rectangle object is not empty");
		//}
		//r0 = new Rectangle(['a',3], 1, -30, 'string');
		//check_equals(''+r0, '(x=a,3, y=1, w=-30, h=string)');
		//check(r0.isEmpty());
		//check_equals(typeof(r0.width), 'number');
		//check_equals(typeof(r0.height), 'string');
        
		//-------------------------------------------------------------
		// Test bottom, right, left top
		//-------------------------------------------------------------
		
		x1 = new Rectangle(untyped 'x', untyped 'y', untyped 'w', untyped 'h');
		DejaGnu.note("x1 = new Rectangle(untyped 'x', untyped 'y', untyped 'w', untyped 'h');");
		//check_equals(r0.left, 'x');
		//check_equals(r0.top, 'y');
		//check_equals(r0.right, 'xw');
		//check_equals(r0.bottom, 'yh');
		if(x1.left == untyped 'x') {
			DejaGnu.pass("x1.left == 'x'");
		} else {
			DejaGnu.fail("x1.left != 'x'");
		}
		if(x1.top == untyped 'y') {
			DejaGnu.pass("x1.top == 'y'");
		} else {
			DejaGnu.fail("x1.top != 'y'");
		}
		if(x1.right == untyped 'xw') {
			DejaGnu.pass("x1.right == 'xw'");
		} else {
			DejaGnu.fail("x1.right != 'xw'");
		}
		if(x1.bottom == untyped 'yh') {
			DejaGnu.pass("x1.bottom == 'yh'");
		} else {
			DejaGnu.fail("x1.bottom != 'yh'");
		}

		x1.left = 10;
		DejaGnu.note("x1.left = 10;");
		if(x1.x == 10) {
			DejaGnu.pass("x1.x == 10");
		} else {
			DejaGnu.fail("x1.x != 10");
		}
		if(x1.left == 10) {
			DejaGnu.pass("x1.left == 10");
		} else {
			DejaGnu.fail("x1.left != 10");
		}
		if(x1.width == untyped 'wNaN') { // w + old_x-10 ?
			DejaGnu.pass("x1.width == 'wNaN'");
		} else {
			DejaGnu.fail("x1.width != 'wNaN'");
		}
		if(x1.right == untyped '10wNaN') {
			DejaGnu.pass("x1.right == '10wNaN'");
		} else {
			DejaGnu.fail("x1.right != '10wNaN'");
		}

		x1.right = 20;
		DejaGnu.note("x1.right = 20;");
		if(x1.x == 10) {
			DejaGnu.pass("x1.x == 10");
		} else {
			DejaGnu.fail("x1.x != 10");
		}
		if(Std.is(x1.width, Int)) {
			DejaGnu.pass("Type of x1.width is number");
		} else {
			DejaGnu.fail("Type of x1.width should be number, is "+Type.typeof(x1.width));
		}
		if(x1.width == 10) { // right-left
			DejaGnu.pass("x1.width == 10");
		} else {
			DejaGnu.fail("x1.width != 10");
		}

		x1.top = 5;
		DejaGnu.note("x1.top = 5;");
		if(x1.y == 5) {
			DejaGnu.pass("x1.y == 5");
		} else {
			DejaGnu.fail("x1.y != 5");
		}
		if(x1.bottom == untyped '5hNaN') {
			DejaGnu.pass("x1.bottom == '5hNaN'");
		} else {
			DejaGnu.fail("x1.bottom != '5hNaN'");
		}
		x1.bottom = 10;
		DejaGnu.note("x1.bottom = 10;");
		if(x1.height == untyped '5') {
			DejaGnu.pass("x1.height == '5'");
		} else {
			DejaGnu.fail("x1.height != '5'");
		}

		x1 = new Rectangle(10, 10, 20, 20);
		DejaGnu.note("x1 = new Rectangle(10, 10, 20, 20);");
		x1.left = 15;
		DejaGnu.note("x1.left = 15;");
		if(x1.width == 15) { // old width (20) + ( old left (10) - new left (15) )
			DejaGnu.pass("x1.width == 15");
		} else {
			DejaGnu.fail("x1.width != 15");
		}
		
		//-------------------------------------------------------------
		// Test bottomRight, topLeft
		//-------------------------------------------------------------
		
		if (Std.is(x1.bottomRight, Point)) {
	 	    DejaGnu.pass("Rectangle.bottomRight property exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle.bottomRight property doesn't exist");
	 	}
	 	if (Std.is(x1.topLeft, Point)) {
	 	    DejaGnu.pass("Rectangle.topLeft property exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle.topLeft property doesn't exist");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test size
		//-------------------------------------------------------------
	 	
	 	if (Std.is(x1.size, Point)) {
	 	    DejaGnu.pass("Rectangle.size property exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle.size property doesn't exist");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test clone
		//-------------------------------------------------------------
	 	
	 	if (Type.typeof(x1.clone) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::clone() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::clone() method doesn't exist");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test contains
		//-------------------------------------------------------------
	 	
	 	x1 = new Rectangle(0, 0, 10, 10);
		DejaGnu.note("x1 = new Rectangle(0, 0, 10, 10);");
		//BORDERS

		//check_equals(typeof(ret), "boolean");
		if (Type.typeof(x1.contains(0,5)) == ValueType.TBool) {
	 	    DejaGnu.pass("Rectangle::contains() method returns boolean");
	 	} else {
	 	    DejaGnu.fail("Rectangle::contains() method does not return boolean");
	 	}
	 	//test left border
		if (x1.contains(0,5)) {
			DejaGnu.pass("Rectangle::contains(0,5) returns true");
		} else {
			DejaGnu.fail("Rectangle::contains(0,5) returns false");
		}

		//test top border
		if (x1.contains(5,0)) {
			DejaGnu.pass("Rectangle::contains(5,0) returns true");
		} else {
			DejaGnu.fail("Rectangle::contains(5,0) returns false");
		}

		//test right border
		if (!x1.contains(10,5)) {
			DejaGnu.pass("Rectangle::contains(10,5) returns false");
		} else {
			DejaGnu.fail("Rectangle::contains(10,5) returns true");
		}

		//test bottom border
		if (!x1.contains(5,10)) {
			DejaGnu.pass("Rectangle::contains(5,10) returns false");
		} else {
			DejaGnu.fail("Rectangle::contains(5,10) returns true");
		}

		//INTERIOR

		//test interior point
		if (x1.contains(0.1,0.1)) {
			DejaGnu.pass("Rectangle::contains(0.1,0.1) returns true");
		} else {
			DejaGnu.fail("Rectangle::contains(0.1,0.1) returns false");
		}

		//EXTERIOR

		//test exterior point, to the left
		if (!x1.contains(-5,5)) {
			DejaGnu.pass("Rectangle::contains(-5,5) returns false");
		} else {
			DejaGnu.fail("Rectangle::contains(-5,5) returns true");
		}

		//test exterior point, to the right
		if (!x1.contains(15,5)) {
			DejaGnu.pass("Rectangle::contains(15,5) returns false");
		} else {
			DejaGnu.fail("Rectangle::contains(15,5) returns true");
		}

		//test exterior point, above
		if (!x1.contains(5,-5)) {
			DejaGnu.pass("Rectangle::contains(5,-5) returns false");
		} else {
			DejaGnu.fail("Rectangle::contains(5,-5) returns true");
		}

		//test exterior point, below
		if (!x1.contains(5,15)) {
			DejaGnu.pass("Rectangle::contains(5,15) returns false");
		} else {
			DejaGnu.fail("Rectangle::contains(5,15) returns true");
		}

		//NONTRIVIAL CALLS
		//if (x1.contains().toString == "undefined") {
		//	DejaGnu.pass("Rectangle.contains(null,null) returns undefined");
		//} else {
		//	DejaGnu.fail("Rectangle.contains(null,null) did not return undefined");
		//}

		//if (x1.contains(0).toString == "undefined") {
		//	DejaGnu.pass("Rectangle.contains(null,null) returns undefined");
		//} else {
		//	DejaGnu.fail("Rectangle.contains(null,null) did not return undefined");
		//}

		//if (x1.contains(0, undefined).toString == "undefined") {
		//	DejaGnu.pass("Rectangle.contains(null,null) returns undefined");
		//} else {
		//	DejaGnu.fail("Rectangle.contains(null,null) did not return undefined");
		//}

		//if (x1.contains(0,null).toString == "undefined") {
		//	DejaGnu.pass("Rectangle.contains(null,null) returns undefined");
		//} else {
		//	DejaGnu.fail("Rectangle.contains(null,null) did not return undefined");
		//}

		//ret = r0.contains('1', '1');
		//check_equals(typeof(ret), 'boolean');
		//check_equals(ret, true);
	
		//-------------------------------------------------------------
		// Test containsPoint
		//-------------------------------------------------------------
		
		if (Type.typeof(x1.containsPoint) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::containsPoint() method exists");
		} else {
			DejaGnu.fail("Rectangle::containsPoint() method doesn't exist");
		}
		
		//-------------------------------------------------------------
		// Test containsRectangle
		//-------------------------------------------------------------
		
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

		//-------------------------------------------------------------
		// Test equals
		//-------------------------------------------------------------
	
	 	if (Type.typeof(x1.equals) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::equals() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::equals() method doesn't exist");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test inflate
		//-------------------------------------------------------------
	 	
		if (Type.typeof(x1.inflate) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::inflate() method exists");
		} else {
			DejaGnu.fail("Rectangle::inflate() method doesn't exist");
		}
		
		//-------------------------------------------------------------
		// Test inflatePoint
		//-------------------------------------------------------------
		
		if (Type.typeof(x1.inflatePoint) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::inflatePoint() method exists");
		} else {
			DejaGnu.fail("Rectangle::inflatePoint() method doesn't exist");
		}
		//-------------------------------------------------------------
		// Test intersection
		//-------------------------------------------------------------

	 	if (Type.typeof(x1.intersection) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::intersection() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::intersection() method doesn't exist");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test intersects
		//-------------------------------------------------------------
	 	
		if (Type.typeof(x1.intersects) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::intersects() method exists");
		} else {
			DejaGnu.fail("Rectangle::intersects() method doesn't exist");
		}
		
		//-------------------------------------------------------------
		// Test offset
		//-------------------------------------------------------------
		
		if (Type.typeof(x1.offset) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::offset() method exists");
		} else {
			DejaGnu.fail("Rectangle::offset() method doesn't exist");
		}
		
		//-------------------------------------------------------------
		// Test offsetPoint
		//-------------------------------------------------------------
		
		if (Type.typeof(x1.offsetPoint) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::offsetPoint() method exists");
		} else {
			DejaGnu.fail("Rectangle::offsetPoint() method doesn't exist");
		}
		
		//-------------------------------------------------------------
		// Test setEmpty
		//-------------------------------------------------------------
		
		if (Type.typeof(x1.setEmpty) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle::setEmpty() method exists");
		} else {
			DejaGnu.fail("Rectangle::setEmpty() method doesn't exist");
		}
		
		//-------------------------------------------------------------
		// Test union
		//-------------------------------------------------------------
		
	 	if (Type.typeof(x1.union) == ValueType.TFunction) {
	 	    DejaGnu.pass("Rectangle::union() method exists");
	 	} else {
	 	    DejaGnu.fail("Rectangle::union() method doesn't exist");
	 	}
	 	
	 	//-------------------------------------------------------------
		// END OF TEST
		//-------------------------------------------------------------

	    // Call this after finishing all tests. It prints out the totals.
	    DejaGnu.done();
#else
	DejaGnu.note("This class (Rectangle) is only available in flash8 and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

