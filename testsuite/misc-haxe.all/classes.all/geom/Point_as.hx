// Point_as.hx:  ActionScript 3 "Point" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
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

// This test is valid for Flash 8 and lower. Failures will occur if run in
//  the Adobe player, due to lack of implementation in Gnash, as well as
//  possible unexpected behavior in haXe. This will be re-evaluated when
//  visiting Point for AS3 support.

#if flash9
import flash.display.MovieClip;
#end
#if flash8
import flash.MovieClip;
#end
#if !(flash6 || flash7)
import flash.geom.Point;
import Std;
import Reflect;
import Math;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Point_as {
    static function main() {
#if !(flash6 || flash7)
		if(Reflect.isFunction(Point)) {
			DejaGnu.pass("Point is a function");
		} else {
#if !flash9
			DejaGnu.fail("Point is not a function");
#else
			DejaGnu.xfail("This doesn't pass in flash9");
#end
		}
		if(Type.typeof(untyped Point.prototype) == ValueType.TObject) {
			DejaGnu.pass("Point prototype is an object");
		} else {
			DejaGnu.fail("Point prototype is not an object");
		}
		if(untyped Point.prototype.hasOwnProperty('length')) {
			DejaGnu.pass("Point prototype has 'length' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'length' property");
		}
		if(!(untyped Point.prototype.hasOwnProperty('x'))) {
			DejaGnu.pass("Point prototype does not have 'x' property");
		} else {
			DejaGnu.fail("Point prototype has 'x' property");
		}
		if(!(untyped Point.prototype.hasOwnProperty('y'))) {
			DejaGnu.pass("Point prototype does not have 'y' property");
		} else {
			DejaGnu.fail("Point prototype has 'y' property");
		}
		if(untyped Point.prototype.hasOwnProperty('add')) {
			DejaGnu.pass("Point prototype has 'add' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'add' property");
		}
		if(untyped Point.prototype.hasOwnProperty('clone')) {
			DejaGnu.pass("Point prototype has 'clone' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'clone' property");
		}
		if(!(untyped Point.prototype.hasOwnProperty('distance'))) {
			DejaGnu.pass("Point prototype does not have 'distance' property");
		} else {
			DejaGnu.fail("Point prototype has 'distance' property");
		}
		if(untyped Point.hasOwnProperty('distance')) {
			DejaGnu.pass("Point has 'distance' property");
		} else {
			DejaGnu.fail("Point does not have 'distance' property");
		}
		if(untyped Point.prototype.hasOwnProperty('equals')) {
			DejaGnu.pass("Point prototype has 'equals' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'equals' property");
		}
		if(!(untyped Point.prototype.hasOwnProperty('interpolate'))) {
			DejaGnu.pass("Point prototype does not have 'interpolate' property");
		} else {
			DejaGnu.fail("Point prototype has 'interpolate' property");
		}
		if(untyped Point.hasOwnProperty('interpolate')) {
			DejaGnu.pass("Point has 'interpolate' property");
		} else {
			DejaGnu.fail("Point does not have 'interpolate' property");
		}
		if(untyped Point.prototype.hasOwnProperty('normalize')) {
			DejaGnu.pass("Point prototype has 'normalize' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'normalize' property");
		}
		if(untyped Point.prototype.hasOwnProperty('offset')) {
			DejaGnu.pass("Point prototype has 'offset' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'offset' property");
		}
		if(!(untyped Point.prototype.hasOwnProperty('polar'))) {
			DejaGnu.pass("Point prototype does not have 'polar' property");
		} else {
			DejaGnu.fail("Point prototype has 'polar' property");
		}
		if(untyped Point.hasOwnProperty('polar')) {
			DejaGnu.pass("Point prototype has 'polar' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'polar' property");
		}
		if(untyped Point.prototype.hasOwnProperty('subtract')) {
			DejaGnu.pass("Point prototype has 'subtract' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'subtract' property");
		}
		if(untyped Point.prototype.hasOwnProperty('toString')) {
			DejaGnu.pass("Point prototype has 'toString' property");
		} else {
			DejaGnu.fail("Point prototype does not have 'toString' property");
		}
		
		//-------------------------------------------------------------
		// Test constructor (and x, y, length)
		//-------------------------------------------------------------

#if flash9
		var x1:Point = new Point();
		DejaGnu.note("var x1:Point = new Point();");
#else
		var x1:Point<Int> = new Point(0,0);
		DejaGnu.note("var x1:Point<Int> = new Point(0,0);");
#end
		if (Type.typeof(x1) == ValueType.TObject) {
			DejaGnu.pass("new Point() returns an object");
		} else {
			DejaGnu.fail("new Point() does not return an object");
		}
		DejaGnu.note(""+Type.typeof(x1));
		if (Std.is(x1, Point)) {
			DejaGnu.pass("new Point() returns a Point object");
		} else {
			DejaGnu.fail("new Point() does not return a Point object");
		}
		if(untyped x1.hasOwnProperty('x')) {
			DejaGnu.pass("Point object has 'x' property");
		} else {
			DejaGnu.fail("Point object does not have 'x' property");
		}
		if(untyped x1.hasOwnProperty('y')) {
			DejaGnu.pass("Point object has 'y' property");
		} else {
			DejaGnu.fail("Point object does not have 'y' property");
		}
		if(x1.toString() == "(x=0, y=0)") {
			DejaGnu.pass("Point object is the correct point (x=0, y=0)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=0, y=0), is "+x1.toString());
		}
		if (Type.typeof(x1.x) == ValueType.TInt) {
			DejaGnu.pass("x1.x is a number");
		} else {
			DejaGnu.fail("x1.x is not a number");
		}
		if (Type.typeof(x1.y) == ValueType.TInt) {
			DejaGnu.pass("x1.y is a number");
		} else {
			DejaGnu.fail("x1.y is not a number");
		}
		if (Type.typeof(x1.length) == ValueType.TInt) {
			DejaGnu.pass("x1.length is a number");
		} else {
			DejaGnu.fail("x1.length is not a number");
		}
		if (x1.length == 0) {
			DejaGnu.pass("x1.length is correct number (0)");
		} else {
			DejaGnu.fail("x1.length is incorrect number (should be 0, is "+x1.length);
		}

		x1 = new Point(untyped 'x', untyped 'y');
		DejaGnu.note("x1 = new Point(untyped 'x', untyped 'y');");
		if(x1.toString() == "(x=x, y=y)") {
			DejaGnu.pass("Point object is the correct point (x=x, y=y)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x, y=y), is "+x1.toString());
		}
		if (Std.is(x1.x, String)) {
			DejaGnu.pass("x1.x is a string");
		} else {
			DejaGnu.fail("x1.x is not a string, it is a "+Type.typeof(x1.x));
		}
		if (Std.is(x1.y, String)) {
			DejaGnu.pass("x1.y is a string");
		} else {
			DejaGnu.fail("x1.y is not a string, it is a "+Type.typeof(x1.y));
		}
		if (Type.typeof(x1.length) == ValueType.TInt) {
			DejaGnu.pass("x1.length is a number");
		} else {
			DejaGnu.fail("x1.length is not a number");
		}
		if (Std.string(x1.length) == "NaN") {
			DejaGnu.pass("x1.length = NaN");
		} else {
			DejaGnu.fail("x1.length != NaN");
		}
		x1.x = 1;
		DejaGnu.note("x1.x = 1;");
		if (Std.string(x1.length) == "NaN") {
			DejaGnu.pass("x1.length = NaN");
		} else {
			DejaGnu.fail("x1.length != NaN");
		}
		x1.y = 0;
		DejaGnu.note("x1.y = 0;");
		if (x1.length == 1) {
			DejaGnu.pass("x1.length = 1");
		} else {
			DejaGnu.fail("x1.length is not 1, it is "+x1.length);
		}
		x1 = new Point(3, 4);
		DejaGnu.note("x1 = new Point(3, 4);");
		if (x1.length == 5) {
			DejaGnu.pass("x1.length is correct number (5)");
		} else {
			DejaGnu.fail("x1.length is incorrect number (should be 5, is "+x1.length);
		}
#if !flash9
		x1 = new Point(50, untyped -Infinity);
		//x1 = Reflect.callMethod(Point, Reflect.field(Point, 'new'), [50, -Infinity]);
		DejaGnu.note("x1 = new Point(50, untyped -Infinity);");
		if (Std.string(x1.length) == 'Infinity') {
			DejaGnu.pass("x1.length is Infinity");
		} else {
			DejaGnu.fail("x1.length is not Infinity, is "+Std.string(x1.length));
		}
#end
		x1 = new Point(0, 0);
		DejaGnu.note("x1 = new Point(0, 0);");
		if (x1.length == 0) {
			DejaGnu.pass("x1.length is correct number (0)");
		} else {
			DejaGnu.fail("x1.length is incorrect number (should be 0, is "+x1.length);
		}
#if !flash9
		x1 = new Point(untyped undef, untyped undef);
		DejaGnu.note("x1 = new Point(untyped undef, untyped undef);");
		if(x1.toString() == "(x=undefined, y=undefined)") {
			DejaGnu.pass("Point object is the correct point (x=undefined, y=undefined)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=undefined, y=undefined), is "+x1.toString());
		}
#end
		
		//-------------------------------------------------------------
		// Test Point.add
		//-------------------------------------------------------------
		
		x1 = new Point(untyped 'x', untyped 'y');
		DejaGnu.note("x1 = new Point(untyped 'x', untyped 'y');");
		//ret = p0.add();
#if flash9
		var x2:Point = Reflect.callMethod(x1, Reflect.field(x1, 'add'), []);
#else
		var x2:Point<Int> = Reflect.callMethod(x1, Reflect.field(x1, 'add'), []);
#end
		DejaGnu.note("x2 = x1.add()");
		if (Std.is(x2, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
		if(x1.toString() == "(x=x, y=y)") {
			DejaGnu.pass("Point object is the correct point (x=x, y=y)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x, y=y), is "+x1.toString());
		}
		if(x2.toString() == "(x=xundefined, y=yundefined)") {
			DejaGnu.pass("Point object is the correct point (x=xundefined, y=yundefined)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=xundefined, y=yundefined), is "+x2.toString());
		}
		untyped String.prototype.x = 3;
		x2 = Reflect.callMethod(x1, Reflect.field(x1, 'add'), ['1']);
		Reflect.deleteField(untyped String.prototype, 'x');
		if (Std.is(x2, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=x3, y=yundefined)');
		if(x2.toString() == "(x=x3, y=yundefined)") {
			DejaGnu.pass("Point object is the correct point (x=x3, y=yundefined)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x3, y=yundefined), is "+x2.toString());
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == "(x=x, y=y)") {
			DejaGnu.pass("Point object is the correct point (x=x, y=y)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x, y=y), is "+x1.toString());
		}
		//ret = p0.add(1, '2');
		x2 = Reflect.callMethod(x1, Reflect.field(x1, 'add'), [1, '2']);
		//check(ret instanceof Point);
		if (Std.is(x2, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=xundefined, y=yundefined)');
		if(x2.toString() == "(x=xundefined, y=yundefined)") {
			DejaGnu.pass("Point object is the correct point (x=xundefined, y=yundefined)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=xundefined, y=yundefined), is "+x2.toString());
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == "(x=x, y=y)") {
			DejaGnu.pass("Point object is the correct point (x=x, y=y)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x, y=y), is "+x1.toString());
		}
		
		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x',untyped 'y');
		//p1 = new Point('x1', 'y1');
		x2 = new Point(untyped 'x1',untyped 'y1');
		//ret = p0.add(p1);
#if flash9
		var x3:Point = new Point(0,0);
		DejaGnu.note("var x3:Point = new Point();");
#else
		var x3:Point<Int> = new Point(0,0);
		DejaGnu.note("var x3:Point<Int> = new Point();");
#end
		x3 = x1.add(x2);
		DejaGnu.note("x3 = x1.add(x2);");
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=xx1, y=yy1)');
		if(x3.toString() == "(x=xx1, y=yy1)") {
			DejaGnu.pass("Point object is the correct point (x=xx1, y=yy1)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=xx1, y=yy1), is "+x3.toString());
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == "(x=x, y=y)") {
			DejaGnu.pass("Point object is the correct point (x=x, y=y)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x, y=y), is "+x1.toString());
		}
		//check_equals(p1.toString(), '(x=x1, y=y1)');
		if(x2.toString() == "(x=x1, y=y1)") {
			DejaGnu.pass("Point object is the correct point (x=x1, y=y1)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x1, y=y1), is "+x2.toString());
		}

		//p0 = new Point(2, 3);
		x1 = new Point(2,3);
		//p1 = { x:1, y:1 };
		var o0 = { x:1, y:1 };
		//ret = p0.add(p1);
		x3 = Reflect.callMethod(x1, Reflect.field(x1, 'add'), [o0]);
		//check_equals(ret.toString(), '(x=3, y=4)');
		if(x3.toString() == "(x=3, y=4)") {
			DejaGnu.pass("Point object is the correct point (x=3, y=4)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=3, y=4), is "+x3.toString());
		}
		
		//ret = p0.add(p1, 4, 5, 6);
		x3 = Reflect.callMethod(x1, Reflect.field(x1, 'add'), [o0, 4, 5, 6]);
		//check_equals(ret.toString(), '(x=3, y=4)');
		if(x3.toString() == "(x=3, y=4)") {
			DejaGnu.pass("Point object is the correct point (x=3, y=4)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=3, y=4), is "+x3.toString());
		}

		// A non-point with a point's add method.
		//fakepoint = {x:20, y:30};
		var fakepoint = {x:20, y:30};
		//fakepoint.add = Point.prototype.add;
		untyped fakepoint.add = untyped Point.prototype.add;
		//ret = fakepoint.add(p0);
		x3 = untyped fakepoint.add(x1);
		//check_equals(ret.toString(), "(x=22, y=33)");
		if(x3.toString() == "(x=22, y=33)") {
			DejaGnu.pass("Point object is the correct point (x=22, y=33)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=22, y=33), is "+x3.toString());
		}
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test Point.clone
		//-------------------------------------------------------------
	 	
	 	//p0 = new Point(3, 4);
	 	x1 = new Point(3,4);
	 	DejaGnu.note("x1 = new Point(3,4);");
		//p0.z = 5;
		untyped x1.z = 5; //make sure no nonsense fields are cloned
		//p2 = p0.clone();
		x2 = x1.clone();
		//check(p2 instanceof Point);
		if (Std.is(x2, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
		//check_equals(p2.toString(), "(x=3, y=4)");
		if(x2.toString() == "(x=3, y=4)") {
			DejaGnu.pass("Point object is the correct point (x=3, y=4)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=3, y=4), is "+x2.toString());
		}
		//check_equals(typeof(p2.z), 'undefined');
		if (untyped x2.z != 5) {
			DejaGnu.pass("No nonsense fields were cloned");
		} else {
			DejaGnu.fail("A nonsense field (.z) was cloned, and is ("+untyped x2.z+")");
		}
		//p2 = p0.clone(1, 2, 3);
		x2 = Reflect.callMethod(x1, Reflect.field(x1, 'clone'), [1,2,3]); //make sure clone does not have or use, and instead ignores, parameters
		//check(p2 instanceof Point);
		if (Std.is(x2, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
		//check_equals(p2.toString(), "(x=3, y=4)");
		if(x2.toString() == "(x=3, y=4)") {
			DejaGnu.pass("Point object is the correct point (x=3, y=4)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=3, y=4), is "+x1.toString());
		}

		// A non-point with a point's clone method.
		//fakepoint = {x:20, y:30};
		fakepoint = {x:20, y:30};
		//fakepoint.clone = Point.prototype.clone;
		untyped fakepoint.clone = untyped Point.prototype.clone;
		//ret = fakepoint.clone(p0);
		x3 = untyped fakepoint.clone(x1);
		//check_equals(ret.toString(), "(x=20, y=30)");
		if(x3.toString() == "(x=20, y=30)") {
			DejaGnu.pass("Point object is the correct point (x=20, y=30)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=20, y=30), is "+x3.toString());
		}
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::clone() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::clone() method does not return Point object");
	 	}
	 	
	 	//-------------------------------------------------------------
		// Test Point.distance (static)
		//-------------------------------------------------------------
	 	
	 	if (Type.typeof(Point.distance) == ValueType.TFunction) {
	 	    DejaGnu.pass("Point::distance() method exists");
	 	} else {
	 	    DejaGnu.fail("Point::distance() method doesn't exist");
	 	}
		
		//dist = Point.distance();
		var dist = Reflect.callMethod(Point, Reflect.field(Point, 'distance'), []);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of Point.distance() is undefined");
		} else {
			DejaGnu.fail("Type of Point.distance() is not undefined, is "+dist);
		}

		//dist = Point.distance(undefined);
		dist = Reflect.callMethod(Point, Reflect.field(Point, 'distance'), [null]);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of Point.distance(undefined) is undefined");
		} else {
			DejaGnu.fail("Type of Point.distance(undefined) is not undefined, is "+dist);
		}

		//o0 = {x:10, y:1};
		var o0 = {x:10, y:1};
		//o1 = {x:21, y:1};
		var o1 = {x:21, y:1};
		//dist = Point.distance(o0, o1);
		dist = Reflect.callMethod(Point, Reflect.field(Point, 'distance'), [o0, o1]);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of Point.distance(FakePoint, FakePoint) is undefined");
		} else {
			DejaGnu.fail("Type of Point.distance(FakePoint, FakePoint) is not undefined, is "+dist);
		}

		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x',untyped 'y');
		//p1 = new Point('a', 'b');
		x2 = new Point(untyped 'a',untyped 'b');
		//dist = Point.distance(p0, p1);
		var dist = Point.distance(x1, x2);
		//check_equals(typeof(dist), 'number');
		if(Std.is(dist, Float)) {
			DejaGnu.pass("Type of Point.distance(x1, x2) is number");
		} else {
			DejaGnu.fail("Type of Point.distance(x1, x2) is not number, is "+dist);
		}
		//check(isNaN(dist));
		if (Std.string(dist) == "NaN") {
			DejaGnu.pass("dist = NaN");
		} else {
			DejaGnu.fail("dist != NaN");
		}
		//dist = p0.distance(p1);
		dist = Reflect.callMethod(x1, Reflect.field(x1, 'distance'), [x2]);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of x1.distance(x2) is undefined");
		} else {
			DejaGnu.fail("Type of x1.distance(x2) is not undefined, is "+dist);
		}

		//p0 = new Point('10', '20');
		x1 = new Point(untyped '10',untyped '20');
		//p1 = new Point('10', 'y');
		x2 = new Point(untyped '10',untyped 'y');
		//dist = Point.distance(p0, p1);
		dist = Point.distance(x1, x2);
		//check_equals(typeof(dist), 'number');
		if(Std.is(dist, Float)) {
			DejaGnu.pass("Type of Point.distance(x1, x2) is number");
		} else {
			DejaGnu.fail("Type of Point.distance(x1, x2) is not number, is "+dist);
		}
		//check(isNaN(dist));
		if (Std.string(dist) == "NaN") {
			DejaGnu.pass("dist = NaN");
		} else {
			DejaGnu.fail("dist != NaN");
		}
		//dist = p0.distance(p1);
		dist = Reflect.callMethod(x1, Reflect.field(x1, 'distance'), [x2]);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of x1.distance(x2) is undefined");
		} else {
			DejaGnu.fail("Type of x1.distance(x2) is not undefined, is "+dist);
		}

		//p0 = new Point('10', 'y');
		x1 = new Point(untyped '10',untyped 'y');
		//p1 = new Point('10', '20');
		x2 = new Point(untyped '10',untyped '20');
		//dist = Point.distance(p0, p1);
		dist = Point.distance(x1, x2);
		//check_equals(typeof(dist), 'number');
		if(Std.is(dist, Float)) {
			DejaGnu.pass("Type of Point.distance(x1, x2) is number");
		} else {
			DejaGnu.fail("Type of Point.distance(x1, x2) is not number, is "+dist);
		}
		//check(isNaN(dist));
		if (Std.string(dist) == "NaN") {
			DejaGnu.pass("dist = NaN");
		} else {
			DejaGnu.fail("dist != NaN");
		}
		//dist = p0.distance(p1);
		dist = Reflect.callMethod(x1, Reflect.field(x1, 'distance'), [x2]);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of x1.distance(x2) is undefined");
		} else {
			DejaGnu.fail("Type of x1.distance(x2) is not undefined, is "+dist);
		}

		//p0 = new Point('5', '4');
		x1 = new Point(untyped '5',untyped '4');
		//p1 = new Point('4', '7');
		x2 = new Point(untyped '4',untyped '7');
		//dist = Point.distance(p0, p1);
		dist = Point.distance(x1, x2);
		//check_equals(typeof(dist), 'number');
		if(Std.is(dist, Float)) {
			DejaGnu.pass("Type of Point.distance(x1, x2) is number");
		} else {
			DejaGnu.fail("Type of Point.distance(x1, x2) is not number, is "+dist);
		}
		//check_equals(Math.round(dist*100), 316);
		if(Math.round(dist*100) == 316) {
			DejaGnu.pass("dist is correct number (316)");
		} else {
			DejaGnu.fail("dist is not correct number (should be 316, is "+Math.round(dist*100)+")");
		}
		//dist = p0.distance(p1);
		dist = Reflect.callMethod(x1, Reflect.field(x1, 'distance'), [x2]);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of x1.distance(x2) is undefined");
		} else {
			DejaGnu.fail("Type of x1.distance(x2) is not undefined, is "+dist);
		}

		//p0 = new Point('1', '1');
		x1 = new Point(untyped '1', untyped '1');
		//p1 = new Point('10', '1');
		x2 = new Point(untyped '10', untyped '1');
		//dist = Point.distance(p0, p1);
		dist = Point.distance(x1,x2);
		//check_equals(typeof(dist), 'number');
		if(Std.is(dist, Float)) {
			DejaGnu.pass("Type of Point.distance(x1, x2) is number");
		} else {
			DejaGnu.fail("Type of Point.distance(x1, x2) is not number, is "+dist);
		}
		//check_equals(dist, 9);
		if(dist == 9) {
			DejaGnu.pass("dist is correct number (9)");
		} else {
			DejaGnu.fail("dist is not correct number (should be 9, is "+dist+")");
		}

		// Doesn't matter if second arg is an instanceof Point
		//dist = Point.distance(p0, o1);
		dist = Point.distance(x1, untyped o1);
		//check_equals(typeof(dist), 'number');
		if(Std.is(dist, Float)) {
			DejaGnu.pass("Type of Point.distance(Point, FakePoint) is number");
		} else {
			DejaGnu.fail("Type of Point.distance(Point, FakePoint) is not number, is "+dist);
		}
		//check_equals(dist, 20);
		if(dist == 20) {
			DejaGnu.pass("dist is correct number (20)");
		} else {
			DejaGnu.fail("dist is not correct number (should be 20, is "+dist+")");
		}

		// But first arg *must* be instanceof point !
		//dist = Point.distance(o1, p0);
		dist = Point.distance(untyped o1, x1);
		//check_equals(typeof(dist), 'undefined');
		if(Type.typeof(dist) == ValueType.TNull) {
			DejaGnu.pass("Type of Point.distance(FakePoint, Point) is undefined");
		} else {
			DejaGnu.fail("Type of Point.distance(FakePoint, Point) is not undefined, is "+dist);
		}
		//o1.__proto__ = Point.prototype;
		untyped o1.__proto__ = untyped Point.prototype;
		//dist = Point.distance(o1, p0);
		dist = Point.distance(untyped o1, x1);
		//check_equals(dist, 20);
		if(dist == 20) {
			DejaGnu.pass("dist is correct number (20)");
		} else {
			DejaGnu.fail("dist is not correct number (should be 20, is "+dist+")");
		}
	 	
	 	//-------------------------------------------------------------
		// Test Point.equals
		//-------------------------------------------------------------
	 	
	 	if (Type.typeof(x1.equals) == ValueType.TFunction) {
	 	    DejaGnu.pass("Point::equals() method exists");
	 	} else {
	 	    DejaGnu.fail("Point::equals() method doesn't exist");
	 	}
		
		//o0 = {};
		var o0 = {};
		//o0.valueOf = function() { return 4; };
		Reflect.setField(o0, 'valueOf', function() { return 4; });
		//o1 = {};
		var o1 = {};
		//o1.valueOf = function() { return 4; };
		Reflect.setField(o1, 'valueOf', function() { return 4; });

		//p0 = new Point(3, o0);
		x1 = new Point(3, untyped o0);
		//check(p0.equals(p0));
		if (x1 == x1) {
			DejaGnu.pass("This is an obscure test, but it passed!");
		} else {
			DejaGnu.fail("This is an obscure test, and it failed.");
		}

		//p1 = new Point(3, o1);
		x2 = new Point(3, untyped o1);
		//check(p1.equals(p1));
		if (x2 == x2) {
			DejaGnu.pass("This is an obscure test, but it passed!");
		} else {
			DejaGnu.fail("This is an obscure test, and it failed.");
		}

		//check(p0 != p1);
		if (x1 != x2) {
			DejaGnu.pass("This is an obscure test, but it passed!");
		} else {
			DejaGnu.fail("This is an obscure test, and it failed.");
		}
		//check_equals(p0.toString(), p1.toString());
		if (x1.toString() == x2.toString()) {
			DejaGnu.pass("x1 and x2 have the same string indentification");
		} else {
			DejaGnu.fail("x1 and x2 have different string identifications, x1="+x1.toString()+", x2="+x2.toString());
		}

		//check(!p0.equals(p1));
		if (x1 != x2) {
			DejaGnu.pass("This is an obscure test, but it passed!");
		} else {
			DejaGnu.fail("This is an obscure test, and it failed.");
		}
		//check(!p1.equals(p0));
		if (x2 != x1) {
			DejaGnu.pass("This is an obscure test, but it passed!");
		} else {
			DejaGnu.fail("This is an obscure test, and it failed.");
		}

		//ret = p0.equals();
		var ret = Reflect.callMethod(x1, Reflect.field(x1, 'equals'), []);
		//check_equals(typeof(ret), 'boolean');
		if(Std.is(ret, Bool)) {
			DejaGnu.pass("x1.equals() returns boolean");
		} else {
			DejaGnu.fail("x1.equals() does not return boolean, returns "+Type.typeof(ret));
		}
		//check(!ret);
		if(!ret) {
			DejaGnu.pass("and return value is false");
		} else {
			DejaGnu.fail("and return value is true");
		}

		//p2 = new Point(3, o1);
		x3 = new Point(3, untyped o1);
		//check(p1.equals(p2));
		if(x2.equals(x3)) {
			DejaGnu.pass("x2.equals(x3) returns true");
		} else {
			DejaGnu.fail("x2.equals(x3) returns false");
		}
		// Equals doesn't return true if p2 isn't an point
		//p2 = {x:3, y:o1};
		o0 = {x:3, y:o1};
		//ret = p1.equals(p2);
		ret = x2.equals(untyped o0);
		//check_equals(typeof(ret), 'boolean');
		if(Std.is(ret, Bool)) {
			DejaGnu.pass("x1.equals() returns boolean");
		} else {
			DejaGnu.fail("x1.equals() does not return boolean, returns "+Type.typeof(ret));
		}
		//check(!ret);
		if(!ret) {
			DejaGnu.pass("and return value is false");
		} else {
			DejaGnu.fail("and return value is true");
		}
		// But we can cheat ...
		//p2.__proto__ = Point.prototype;
		untyped o0.__proto__ = untyped Point.prototype;
		//check(p1.equals(p2));
		if(x2.equals(untyped o0)) {
			DejaGnu.pass("x2.equals(o0) is fooled with Point.prototype");
		} else {
			DejaGnu.fail("x2.equals(o0) is not fooled with Point.prototype");
		}
		// ... even with double jump to get there ...
		//o3 = {}; o3.prototype = {}; o3.prototype.__proto__ = Point.prototype;
		var o3 = {};
		untyped o3.prototype = {};
		untyped o3.prototype.__proto__ = untyped Point.prototype;
		//p2.__proto__ = o3.prototype;
		untyped o0.__proto__ = untyped Point.prototype;
		//check(p1.equals(p2));
		if(x2.equals(untyped o0)) {
			DejaGnu.pass("x2.equals(o0) is fooled with prototype of Point.prototype");
		} else {
			DejaGnu.fail("x2.equals(o0) is not fooled with prototype of Point.prototype");
		}
		// ... but not with syntetized objects ?
		//String.prototype.x = 3;
		untyped String.prototype.x = 3;
		//String.prototype.y = o1;
		untyped String.prototype.y = o1;
		//String.prototype.__proto__ = Point.prototype;
		untyped String.prototype.__proto__ = untyped Point.prototype;
		//check(!p1.equals('string'));
		if(!x2.equals(untyped 'string')) {
			DejaGnu.pass("x2.equals('string') is not fooled with Point.prototype");
		} else {
			DejaGnu.fail("x2.equals('string') is fooled with Point.prototype");
		}

		// A non-point with a point's equals method.
		//fakepoint = {x:20, y:30};
		fakepoint = {x:20, y:30};
		//fakepoint.equals = Point.prototype.equals;
		untyped fakepoint.equals = untyped Point.prototype.equals;
		//ret = fakepoint.equals(new Point(20,30));
		ret = untyped fakepoint.equals(new Point(20,30));
		//check_equals(ret.toString(), "true");
		if(Std.string(ret) == "true") {
			DejaGnu.pass("fakepoint.equals(new Point(20,30)) returns true");
		} else {
			DejaGnu.fail("fakepoint.equals(new Point(20,30)) returns false");
		}
		//check_equals(typeof(ret), "boolean");
		if(Std.is(ret, Bool)) {
			DejaGnu.pass("fakepoint.equals returns a boolean");
		} else {
			DejaGnu.fail("fakepoint.equals does not return a boolean");
		}
	 	
	 	//-------------------------------------------------------------
		// Test Point.interpolate (static)
		//-------------------------------------------------------------
	 	
	 	if (Type.typeof(Point.interpolate) == ValueType.TFunction) {
	 	    DejaGnu.pass("Point::interpolate() method exists");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method doesn't exist");
	 	}
		//ret = Point.interpolate();
		x1 = Reflect.callMethod(Point, Reflect.field(Point, 'interpolate'), []);
		//check(ret instanceof Point);
		if (Std.is(x1, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(x1.toString() == "(x=NaN, y=NaN)") {
			DejaGnu.pass("Point object is the correct point (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=NaN, y=NaN), is "+x1.toString());
		}
		//ret = Point.interpolate(1, 2, 3);
		x1 = Reflect.callMethod(Point, Reflect.field(Point, 'interpolate'), [1,2,3]);
		//check(ret instanceof Point);
		if (Std.is(x1, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(x1.toString() == "(x=NaN, y=NaN)") {
			DejaGnu.pass("Point object is the correct point (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=NaN, y=NaN), is "+x1.toString());
		}
	 	//p0 = new Point('x0', 'y0');
		x1 = new Point(untyped 'x0',untyped 'y0');
		//p1 = new Point('x1', 'y1');
		x2 = new Point(untyped 'x1',untyped 'y1');
		//ret = Point.interpolate(p0, p1, 3);
		x3 = Point.interpolate(x1,x2,3);
		DejaGnu.note("x3 = Point.interpolate(x1,x2,3);");
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=x1NaN, y=y1NaN)');
		if(x3.toString() == "(x=x1NaN, y=y1NaN)") {
			DejaGnu.pass("Point object is the correct point (x=x1NaN, y=y1NaN)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=x1NaN, y=y1NaN), is "+x3.toString());
		}
	 	//p0 = new Point('0', '0');
	 	x1 = new Point(untyped '0',untyped '0');
	 	DejaGnu.note("x1 = new Point(untyped '0',untyped '0');");
		//p1 = new Point('10', '0');
		x2 = new Point(untyped '10',untyped '0');
		DejaGnu.note("x2 = new Point(untyped '10',untyped '0');");
		//ret = Point.interpolate(p0, p1, 3);
		x3 = Point.interpolate(x1,x2,3);
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=10-30, y=00)');
		if(x3.toString() == "(x=10-30, y=00)") {
			DejaGnu.pass("Point object is the correct point (x=10-30, y=00)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=10-30, y=00), is "+x3.toString());
		}
		//ret = Point.interpolate(p0, p1, 0);
		x3 = Point.interpolate(x1,x2,0);
		DejaGnu.note("x3 = Point.interpolate(x1,x2,0);");
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=100, y=00)');
		if(x3.toString() == "(x=100, y=00)") {
			DejaGnu.pass("Point object is the correct point (x=100, y=00)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=100, y=00), is "+x3.toString());
		}
		//ret = Point.interpolate(p0, p1, 0.5);
		x3 = Point.interpolate(x1,x2,untyped .5); 
		DejaGnu.note("x3 = Point.interpolate(x1,x2,0.5);");
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=10-5, y=00)');
		if(x3.toString() == "(x=10-5, y=00)") {
			DejaGnu.pass("Point object is the correct point (x=10-5, y=00)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=10-5, y=00), is "+x3.toString());
		}
		//p0 = new Point(0, 0);
		x1 = new Point(0,0);
		DejaGnu.note("x1 = new Point(0,0);");
		//p1 = new Point('10', '0');
		x2 = new Point(untyped '10',untyped '0');
		DejaGnu.note("x2 = new Point(untyped '10',untyped '0');");
		//ret = Point.interpolate(p0, p1, 3);
		x3 = Point.interpolate(x1,x2,3);
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=10-30, y=00)');
	 	if(x3.toString() == "(x=10-30, y=00)") {
			DejaGnu.pass("Point object is the correct point (x=10-30, y=00)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=10-30, y=00), is "+x3.toString());
		}
		//p0 = new Point('0', '0');
		x1 = new Point(untyped '0', untyped '0');
		DejaGnu.note("x1 = new Point(untyped '0', untyped '0');");
		//p1 = new Point(10, 0);
		x2 = new Point(10,0);
		DejaGnu.note("x2 = new Point(10,0);");
		//ret = Point.interpolate(p0, p1, 3);
		x3 = Point.interpolate(x1,x2,3);
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=-20, y=0)');
		if(x3.toString() == "(x=-20, y=0)") {
			DejaGnu.pass("Point object is the correct point (x=-20, y=0)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=-20, y=0), is "+x3.toString());
		}
		//p0 = new Point(0, 0);
		x1 = new Point(0, 0);
		DejaGnu.note("x1 = new Point(0, 0);");
		//p1 = new Point(10, 0);
		x2 = new Point(10,0);
		DejaGnu.note("x2 = new Point(10,0);");
		//ret = Point.interpolate(p0, p1, 0.5);
		x3 = Point.interpolate(x1,x2,untyped .5);
		//check(ret instanceof Point);
		if (Std.is(x3, Point)) {
	 	    DejaGnu.pass("Point::interpolate() method returns Point object");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return Point object");
	 	}
		//check_equals(ret.toString(), '(x=5, y=0)');
		if(x3.toString() == "(x=5, y=0)") {
			DejaGnu.pass("Point object is the correct point (x=5, y=0)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=5, y=0), is "+x3.toString());
		}
		//p0 = new Point(0, 0);
		x1 = new Point(0,0);
		DejaGnu.note("x1 = new Point(0,0);");
		//p1 = new Point(10, 0);
		x2 = new Point(10,0);
		DejaGnu.note("x2 = new Point(10,0);");
		//ret = Point.interpolate(p0, p1, 1, 'discarder arg');
		x3 = Reflect.callMethod(Point, Reflect.field(Point,'interpolate'), [x1,x2,1,'discarder arg']);
		//check(ret.equals(p0));		
		if (x3.equals(x1)) {
	 	    DejaGnu.pass("Point::interpolate() method returns original point");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return original point");
	 	}
		//ret = Point.interpolate(p0, p1, 0);
		x3 = Reflect.callMethod(Point, Reflect.field(Point,'interpolate'), [x1,x2,0]);
		//check(ret.equals(p1));
		if (x3.equals(x2)) {
	 	    DejaGnu.pass("Point::interpolate() method returns original point");
	 	} else {
	 	    DejaGnu.fail("Point::interpolate() method does not return original point");
	 	}
		//ret = Point.interpolate(p0, p1);
		x3 = Reflect.callMethod(Point, Reflect.field(Point, 'interpolate'), [x1,x2]);
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(x3.toString() == "(x=NaN, y=NaN)") {
			DejaGnu.pass("Point object is the correct point (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=NaN, y=NaN), is "+x3.toString());
		}
		//o0 = {x:0, y:10};
		o0 = {x:0, y:10};
		//o1 = {x:10, y:0};
		o1 = {x:10, y:0};
		//ret = Point.interpolate(o0, o1, 1);
		x3 = Point.interpolate(untyped o0, untyped o1, 1);
		//check_equals(ret.toString(), '(x=0, y=10)');
		if(x3.toString() == "(x=0, y=10)") {
			DejaGnu.pass("Point object is the correct point (x=0, y=10)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=0, y=10), is "+x3.toString());
		}
		//ret = Point.interpolate(o0, o1, 0);
		x3 = Point.interpolate(untyped o0, untyped o1, 0);
		//check_equals(ret.toString(), '(x=10, y=0)');
		if(x3.toString() == "(x=10, y=0)") {
			DejaGnu.pass("Point object is the correct point (x=10, y=0)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=10, y=0), is "+x3.toString());
		}
		//ret = Point.interpolate(o0, o1, 0.5);
		x3 = Point.interpolate(untyped o0, untyped o1, untyped .5);
		//check_equals(ret.toString(), '(x=5, y=5)');
		if(x3.toString() == "(x=5, y=5)") {
			DejaGnu.pass("Point object is the correct point (x=5, y=5)");
		} else {
			DejaGnu.fail("Point object is not the correct point (should be (x=5, y=5), is "+x3.toString());
		}
		
	 	//-------------------------------------------------------------
		// Test Point.normalize
		//-------------------------------------------------------------
	 	
		if (Type.typeof(x1.normalize) == ValueType.TFunction) {
			DejaGnu.pass("Point::normalize() method exists");
		} else {
			DejaGnu.fail("Point::normalize() method doesn't exist");
		}
		
		//p0 = new Point(0, 0);
		x1 = new Point(0,0);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize();
		var ret = Reflect.callMethod(x2, Reflect.field(x2, 'normalize'), []);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check(p1.equals(p0));
		if(x2.equals(x1)) {
			DejaGnu.pass("x2 = x1");
		} else {
			DejaGnu.fail("x2 != x1");
		}

		//p0 = new Point(0, 0);
		x1 = new Point(0,0);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize(10);
		var ret = x2.normalize(10);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check(p1.equals(p0));
		if(x2.equals(x1)) {
			DejaGnu.pass("x2 = x1");
		} else {
			DejaGnu.fail("x2 != x1");
		}

		//p0 = new Point(10, 0);
		x1 = new Point(10,0);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize(5);
		var ret = x2.normalize(5);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check_equals(p1.toString(), '(x=5, y=0)');
		if(x2.toString() == '(x=5, y=0)') {
			DejaGnu.pass("x2 has been normalized to (x=5, y=0)");
		} else {
			DejaGnu.fail("x2 has been normalized to "+x2.toString());
		}

		//p0 = new Point(0, 10);
		x1 = new Point(0, 10);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize(-5);
		var ret = x2.normalize(-5);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check_equals(p1.toString(), '(x=0, y=-5)');
		if(x2.toString() == '(x=0, y=-5)') {
			DejaGnu.pass("x2 has been normalized to (x=0, y=-5)");
		} else {
			DejaGnu.fail("x2 has been normalized to "+x2.toString());
		}

		//p0 = new Point(3, -4);
		x1 = new Point(3,-4);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize(-10);
		var ret = x2.normalize(-10);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check_equals(p1.toString(), '(x=-6, y=8)');
		if(x2.toString() == '(x=-6, y=8)') {
			DejaGnu.pass("x2 has been normalized to (x=-6, y=8)");
		} else {
			DejaGnu.fail("x2 has been normalized to "+x2.toString());
		}

		//p0 = new Point(-10, 0);
		x1 = new Point(-10, 0);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize(5);
		var ret = x2.normalize(5);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check_equals(p1.toString(), '(x=-5, y=0)');
		if(x2.toString() == '(x=-5, y=0)') {
			DejaGnu.pass("x2 has been normalized to (x=-5, y=0)");
		} else {
			DejaGnu.fail("x2 has been normalized to "+x2.toString());
		}

		//p0 = new Point(-10, 0);
		x1 = new Point(-10, 0);
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize('r');
		var ret = x2.normalize(untyped 'r');
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check_equals(p1.toString(), '(x=NaN, y=NaN)');
		if(x2.toString() == '(x=NaN, y=NaN)') {
			DejaGnu.pass("x2 has been normalized to (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("x2 has been normalized to "+x2.toString());
		}

		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x', untyped 'y');
		//p1 = p0.clone();
		x2 = x1.clone();
		//ret = p1.normalize(5);
		var ret = x2.normalize(5);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.normalize() is undefined");
		} else {
			DejaGnu.fail("Type of x2.normalise() is not undefined, is "+ret);
		}
		//check_equals(p1.toString(), '(x=x, y=y)');
		if(x2.toString() == '(x=x, y=y)') {
			DejaGnu.pass("x2 has been normalized to (x=x, y=y)");
		} else {
			DejaGnu.fail("x2 has been normalized to "+x2.toString());
		}
		
		//-------------------------------------------------------------
		// Test Point.offset
		//-------------------------------------------------------------
		
		if (Type.typeof(x1.offset) == ValueType.TFunction) {
			DejaGnu.pass("Point::offset() method exists");
		} else {
			DejaGnu.fail("Point::offset() method doesn't exist");
		}
		
		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x', untyped 'y');
		//ret = p0.offset();
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'offset'), []);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.offset() is undefined");
		} else {
			DejaGnu.fail("Type of x2.offset() is not undefined, is "+ret);
		}
		//check_equals(p0.toString(), '(x=xundefined, y=yundefined)');
		if(x1.toString() == '(x=xundefined, y=yundefined)') {
			DejaGnu.pass("x2 has been offset to (x=xundefined, y=yundefined)");
		} else {
			DejaGnu.fail("x2 has been offset to "+x1.toString());
		}

		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x', untyped 'y');
		//ret = p0.offset('a');
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'offset'), ['a']);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.offset('a') is undefined");
		} else {
			DejaGnu.fail("Type of x2.offset('a') is not undefined, is "+ret);
		}
		//check_equals(p0.toString(), '(x=xa, y=yundefined)');
		if(x1.toString() == '(x=xa, y=yundefined)') {
			DejaGnu.pass("x2 has been offset to (x=xa, y=yundefined)");
		} else {
			DejaGnu.fail("x2 has been offset to "+x1.toString());
		}

		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x', untyped 'y');
		//ret = p0.offset('a', 'b', 3);
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'offset'), ['a', 'b', 3]);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.offset('a', 'b', 3) is undefined");
		} else {
			DejaGnu.fail("Type of x2.offset('a', 'b', 3) is not undefined, is "+ret);
		}
		//check_equals(p0.toString(), '(x=xa, y=yb)');
		if(x1.toString() == '(x=xa, y=yb)') {
			DejaGnu.pass("x2 has been offset to (x=xa, y=yb)");
		} else {
			DejaGnu.fail("x2 has been offset to "+x1.toString());
		}

		//p0 = new Point(4, 5);
		x1 = new Point(4, 5);
		//ret = p0.offset('-6', -8);
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'offset'), ['-6', -8]);
		//check_equals(typeof(ret), 'undefined');
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of x2.offset('-6', -8) is undefined");
		} else {
			DejaGnu.fail("Type of x2.offset('-6', -8) is not undefined, is "+ret);
		}
		//check_equals(p0.toString(), '(x=4-6, y=-3)');
		if(x1.toString() == '(x=4-6, y=-3)') {
			DejaGnu.pass("x2 has been offset to (x=4-6, y=-3)");
		} else {
			DejaGnu.fail("x2 has been offset to "+x1.toString());
		}

		// A non-point with a point's offset method (fails)
		//fakepoint = {x:20, y:30};
		fakepoint = {x:20, y:30};
		//fakepoint.offset = Point.prototype.offset;
		untyped fakepoint.offset = untyped Point.prototype.offset;
		//ret = fakepoint.offset(new Point(1, 3));
		ret = untyped fakepoint.offset(new Point(1,3));
		//check_equals(ret.toString(), undefined);
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of fakepoint.offset(new Point(1,3)) is undefined");
		} else {
			DejaGnu.fail("Type of fakepoint.offset(new Point(1,3)) is not undefined, is "+ret);
		}
		//check(! ret instanceof Point);
		if(!Std.is(ret, Point)) {
			DejaGnu.pass("and does not return a point");
		} else {
			DejaGnu.fail("but returns a point");
		}
		
		//-------------------------------------------------------------
		// Test Point.polar (static)
		//-------------------------------------------------------------
		
	 	if (Type.typeof(Point.polar) == ValueType.TFunction) {
	 	    DejaGnu.pass("Point::polar() method exists");
	 	} else {
	 	    DejaGnu.fail("Point::polar() method doesn't exist");
	 	}
		
		//p0 = Point.polar();
		x1 = Reflect.callMethod(Point, Reflect.field(Point, 'polar'), []);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(p0.toString(), '(x=NaN, y=NaN)');
		if(x1.toString() == '(x=NaN, y=NaN)') {
			DejaGnu.pass("x2 is not a real point yet (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("x2 is a real point at "+x1.toString()+", but it shouldn't be");
		}

		//p0 = Point.polar(1);
		x1 = Reflect.callMethod(Point, Reflect.field(Point, 'polar'), [1]);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(p0.toString(), '(x=NaN, y=NaN)');
		if(x1.toString() == '(x=NaN, y=NaN)') {
			DejaGnu.pass("x1 is not a real point yet (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("x1 is a real point at "+x1.toString()+", but it shouldn't be");
		}

		//p0 = Point.polar(1, 0);
		x1 = Point.polar(1,0);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(p0.toString(), '(x=1, y=0)');
		if(x1.toString() == '(x=1, y=0)') {
			DejaGnu.pass("x1 is not a real point yet (x=1, y=0)");
		} else {
			DejaGnu.fail("x1 is a real point at "+x1.toString()+", but it shouldn't be");
		}

		//p0 = Point.polar(1, Math.PI);
		x1 = Point.polar(1, untyped Math.PI);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(p0.x, -1);
		if(x1.x == -1) {
			DejaGnu.pass("x1.x = -1");
		} else {
			DejaGnu.fail("x1.x != -1");
		}
		//check_equals(Math.round(p0.y*100), 0);
		if(Math.round(x1.y*100) == 0) {
			DejaGnu.pass("Math.round(x1.y*100) = 0");
		} else {
			DejaGnu.fail("Math.round(x1.y*100) != 0");
		}

		//p0 = Point.polar(1, Math.PI/2);
		x1 = Point.polar(1, untyped Math.PI/2);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(Math.round(p0.x*100), 0);
		if(Math.round(x1.x*100) == 0) {
			DejaGnu.pass("Math.round(x1.x*100) = 0");
		} else {
			DejaGnu.fail("Math.round(x1.x*100) != 0");
		}
		//check_equals(p0.y, 1);
		if(x1.y == 1) {
			DejaGnu.pass("x1.y = 1");
		} else {
			DejaGnu.fail("x1.y != 1");
		}
		
		//p0 = Point.polar(1, Math.PI*2);
		x1 = Point.polar(1, untyped Math.PI*2);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(p0.x, 1);
		if(x1.x == 1) {
			DejaGnu.pass("x1.x = 1");
		} else {
			DejaGnu.fail("x1.x != 1");
		}
		//check_equals(Math.round(p0.y*100), 0);
		if(Math.round(x1.y*100) == 0) {
			DejaGnu.pass("Math.round(x1.y*100) = 0");
		} else {
			DejaGnu.fail("Math.round(x1.y*100) != 0");
		}

		//p0 = Point.polar(1, Math.PI*1.5);
		x1 = Point.polar(1, untyped Math.PI*1.5);
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(Math.round(p0.x*100), 0);
		if(Math.round(x1.x*100) == 0) {
			DejaGnu.pass("Math.round(x1.x*100) = 0");
		} else {
			DejaGnu.fail("Math.round(x1.x*100) != 0");
		}
		//check_equals(p0.y, -1);
		if(x1.y == -1) {
			DejaGnu.pass("x1.y = -1");
		} else {
			DejaGnu.fail("x1.y != -1");
		}

		//p0 = Point.polar('5', '0');
		x1 = Point.polar(untyped '5', untyped '0');
		//check(p0 instanceof Point);
		if(Std.is(x1, Point)) {
			DejaGnu.pass("x1 is a Point");
		} else {
			DejaGnu.fail("x1 is not a Point");
		}
		//check_equals(p0.x, 5);
		if(x1.x == 5) {
			DejaGnu.pass("x1.x = 5");
		} else {
			DejaGnu.fail("x1.x != 5");
		}
		//check_equals(p0.y, 0);
		if(x1.y == 0) {
			DejaGnu.pass("x1.y = 0");
		} else {
			DejaGnu.fail("x1.y != 0");
		}
	 	
		//-------------------------------------------------------------
		// Test Point.subtract
		//-------------------------------------------------------------	 	
	 	
	 	if (Type.typeof(x1.subtract) == ValueType.TFunction) {
	 	    DejaGnu.pass("Point::subtract() method exists");
	 	} else {
	 	    DejaGnu.fail("Point::subtract() method doesn't exist");
	 	}
		
		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x', untyped 'y');
		//ret = p0.subtract();
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'subtract'), []);
		//check(ret instanceof Point);
		if(Std.is(ret, Point)) {
			DejaGnu.pass("x1.subtract() returns a Point");
		} else {
			DejaGnu.fail("x1.subtract() does not return a Point");
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == '(x=x, y=y)') {
			DejaGnu.pass("x1 is still (x=x, y=y)");
		} else {
			DejaGnu.fail("x1 is now "+x1.toString());
		}
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(Std.string(ret) == '(x=NaN, y=NaN)') {
			DejaGnu.pass("ret is (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("ret should be (x=NaN, y=NaN), but it is "+Std.string(ret));
		}
		//String.prototype.x = 3; // to test it's used
		untyped String.prototype.x = 3;
		//ret = p0.subtract('1');
		var ret = x1.subtract(untyped '1');
		//delete String.prototype.x;
		Reflect.deleteField(untyped String.prototype, 'x');
		//check(ret instanceof Point);
		if(Std.is(ret, Point)) {
			DejaGnu.pass("x1.subtract('1') returns a Point");
		} else {
			DejaGnu.fail("x1.subtract('1') does not return a Point");
		}
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(Std.string(ret) == '(x=NaN, y=NaN)') {
			DejaGnu.pass("ret is (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("ret should be (x=NaN, y=NaN), but it is "+Std.string(ret));
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == '(x=x, y=y)') {
			DejaGnu.pass("x1 is still (x=x, y=y)");
		} else {
			DejaGnu.fail("x1 is now "+x1.toString());
		}
		//ret = p0.subtract(1, '2');
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'subtract'), [1, '2']);
		//check(ret instanceof Point);
		if(Std.is(ret, Point)) {
			DejaGnu.pass("x1.subtract(1, '2') returns a Point");
		} else {
			DejaGnu.fail("x1.subtract(1, '2') does not return a Point");
		}
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(ret.toString() == '(x=NaN, y=NaN)') {
			DejaGnu.pass("ret is (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("ret should be (x=NaN, y=NaN), but it is "+ret.toString());
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == '(x=x, y=y)') {
			DejaGnu.pass("x1 is still (x=x, y=y)");
		} else {
			DejaGnu.fail("x1 is now "+x1.toString());
		}

		//p0 = new Point('x', 'y');
		x1 = new Point(untyped 'x', untyped 'y');
		//p1 = new Point('x1', 'y1');
		x2 = new Point(untyped 'x1', untyped 'y1');
		//ret = p0.subtract(p1);
		ret = x1.subtract(x2);
		//check(ret instanceof Point);
		if(Std.is(ret, Point)) {
			DejaGnu.pass("x1.subtract(1, '2') returns a Point");
		} else {
			DejaGnu.fail("x1.subtract(1, '2') does not return a Point");
		}
		//check_equals(ret.toString(), '(x=NaN, y=NaN)');
		if(ret.toString() == '(x=NaN, y=NaN)') {
			DejaGnu.pass("ret is (x=NaN, y=NaN)");
		} else {
			DejaGnu.fail("ret should be (x=NaN, y=NaN), but it is "+ret.toString());
		}
		//check_equals(p0.toString(), '(x=x, y=y)');
		if(x1.toString() == '(x=x, y=y)') {
			DejaGnu.pass("x1 is still (x=x, y=y)");
		} else {
			DejaGnu.fail("x1 is now "+x1.toString());
		}
		//check_equals(p1.toString(), '(x=x1, y=y1)');
		if(x2.toString() == '(x=x1, y=y1)') {
			DejaGnu.pass("x2 is still (x=x1, y=y1)");
		} else {
			DejaGnu.fail("x2 is now "+x2.toString());
		}

		//p0 = new Point(2, 3);
		x1 = new Point(2, 3);
		//p1 = { x:1, y:1 };
		o1 = { x:1, y:1 };
		//ret = p0.subtract(p1);
		ret = x1.subtract(untyped o1);
		//check_equals(ret.toString(), '(x=1, y=2)');
		if(ret.toString() == '(x=1, y=2)') {
			DejaGnu.pass("ret is (x=1, y=2)");
		} else {
			DejaGnu.fail("ret should be (x=1, y=2), but it is "+ret.toString());
		}

		//ret = p0.subtract(p1, 4, 5, 6);
		ret = Reflect.callMethod(x1, Reflect.field(x1, 'subtract'), [o1, 4, 5, 6]);
		//check_equals(ret.toString(), '(x=1, y=2)');
		if(ret.toString() == '(x=1, y=2)') {
			DejaGnu.pass("ret is (x=1, y=2)");
		} else {
			DejaGnu.fail("ret should be (x=1, y=2), but it is "+ret.toString());
		}
	 	
		//-------------------------------------------------------------
		// END OF TEST
		//-------------------------------------------------------------
		    // Call this after finishing all tests. It prints out the totals.
		DejaGnu.done();
#else
    	DejaGnu.note("This class (Point) is only available in flash8 and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
