// Rectangle_as.hx:  ActionScript 3 "Rectangle" class, for Gnash.
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
//  visiting Rectangle for AS3 support.

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
import Reflect;

import DejaGnu;

class Rectangle_as {
    static function main() {
#if (flash8 || flash9)
		if(Type.typeof(untyped Rectangle) == ValueType.TFunction) {
			DejaGnu.pass("Rectangle is a function");
		} else {
			DejaGnu.fail("Rectangle is not a function");
		}
		if(Type.typeof(untyped Rectangle.prototype) == ValueType.TObject) {
			DejaGnu.pass("Rectangle prototype is an object");
		} else {
			DejaGnu.fail("Rectangle prototype is not an object");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('bottom')) {
			DejaGnu.pass("Rectangle prototype has 'bottom' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'bottom' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('bottomRight')) {
			DejaGnu.pass("Rectangle prototype has 'bottomRight' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'bottomRight' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('left')) {
			DejaGnu.pass("Rectangle prototype has 'left' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'left' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('right')) {
			DejaGnu.pass("Rectangle prototype has 'right' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'right' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('size')) {
			DejaGnu.pass("Rectangle prototype has 'size' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'size' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('top')) {
			DejaGnu.pass("Rectangle prototype has 'top' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'top' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('topLeft')) {
			DejaGnu.pass("Rectangle prototype has 'topLeft' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'topLeft' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('clone')) {
			DejaGnu.pass("Rectangle prototype has 'clone' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'clone' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('contains')) {
			DejaGnu.pass("Rectangle prototype has 'contains' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'contains' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('containsPoint')) {
			DejaGnu.pass("Rectangle prototype has 'containsPoint' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'containsPoint' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('containsRectangle')) {
			DejaGnu.pass("Rectangle prototype has 'containsRectangle' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'containsRectangle' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('equals')) {
			DejaGnu.pass("Rectangle prototype has 'equals' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'equals' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('inflate')) {
			DejaGnu.pass("Rectangle prototype has 'inflate' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'inflate' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('inflatePoint')) {
			DejaGnu.pass("Rectangle prototype has 'inflatePoint' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'inflatePoint' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('intersection')) {
			DejaGnu.pass("Rectangle prototype has 'intersection' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'intersection' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('intersects')) {
			DejaGnu.pass("Rectangle prototype has 'intersects' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'intersects' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('isEmpty')) {
			DejaGnu.pass("Rectangle prototype has 'isEmpty' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'isEmpty' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('offset')) {
			DejaGnu.pass("Rectangle prototype has 'offset' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'offset' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('offsetPoint')) {
			DejaGnu.pass("Rectangle prototype has 'offsetPoint' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'offsetPoint' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('setEmpty')) {
			DejaGnu.pass("Rectangle prototype has 'setEmpty' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'setEmpty' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('toString')) {
			DejaGnu.pass("Rectangle prototype has 'toString' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'toString' property");
		}
		if(untyped Rectangle.prototype.hasOwnProperty('union')) {
			DejaGnu.pass("Rectangle prototype has 'union' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'union' property");
		}
		if(!(untyped Rectangle.prototype.hasOwnProperty('height'))) {
			DejaGnu.pass("Rectangle prototype has 'height' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'height' property");
		}
		if(!(untyped Rectangle.prototype.hasOwnProperty('width'))) {
			DejaGnu.pass("Rectangle prototype has 'width' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'width' property");
		}
		if(!(untyped Rectangle.prototype.hasOwnProperty('x'))) {
			DejaGnu.pass("Rectangle prototype has 'x' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'x' property");
		}
		if(!(untyped Rectangle.prototype.hasOwnProperty('y'))) {
			DejaGnu.pass("Rectangle prototype has 'y' property");
		} else {
			DejaGnu.fail("Rectangle prototype does not have 'y' property");
		}

		//-------------------------------------------------------------
		// Test constructor (and width, height, x, y)
		//-------------------------------------------------------------

#if flash9
        var x1:Rectangle = untyped __new__(Rectangle);
        DejaGnu.note("var x1:Rectangle = new Rectangle();");
#else
		var x1:Rectangle<Int> = untyped __new__(Rectangle);
		DejaGnu.note("var x1:Rectangle<Int> = new Rectangle();");
#end
		if (Type.typeof(x1) == ValueType.TObject) {
			DejaGnu.pass("new Rectangle() returns an object");
		} else {
			DejaGnu.fail("new Rectangle() does not return an object");
		}
		if (Std.is(x1, Rectangle)) {
            DejaGnu.pass("new Rectangle() returns a Rectangle object");
        } else {
            DejaGnu.fail("new Rectangle() does not return a Rectangle object");
        }
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
		if (x1.toString() == "(x=0, y=0, w=0, h=0)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=0, y=0, w=0, h=0)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=0, y=0, w=0, h=0), is "+x1.toString()+")");
		}
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
		x1 = untyped __new__(Rectangle, 1);
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
		x1 = new Rectangle(1, 1, 1, untyped "string");
		DejaGnu.note("x1 = new Rectangle(1, 1, 1, untyped \"string\");");
		if (x1.toString() == "(x=1, y=1, w=1, h=string)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=1, y=1, w=1, h=string)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=1, y=1, w=1, h=string), is "+x1.toString()+")");
		}
		if (x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		x1 = new Rectangle(untyped ['a',3], 1, -30, untyped 'string');
		DejaGnu.note("x1 = new Rectangle(['a',3], 1, -30, 'string');");
		if (x1.toString() == "(x=a,3, y=1, w=-30, h=string)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=a,3, y=1, w=-30, h=string)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=a,3, y=1, w=-30, h=string), is "+x1.toString()+")");
		}
		if (x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		if(Std.is(x1.width, Int)) {
			DejaGnu.pass("Rectangle object .width is a number");
		} else {
			DejaGnu.fail("Rectangle object .width is not a number");
		}
		if(Std.is(x1.height, String)) {
			DejaGnu.pass("Rectangle object .height is a string");
		} else {
			DejaGnu.fail("Rectangle object .height is not a string");
		}
		//o1 = {}; o1.toString = function() { return '2'; };
		var o1 = {};
		Reflect.setField(o1, 'toString', function() { return '2'; });
		x1 = new Rectangle(0, 0, untyped o1, null);
		DejaGnu.note("x1 = new Rectangle(0,0,untyped o1,null);");
		if (x1.toString() == "(x=0, y=0, w=2, h=null)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=0, y=0, w=2, h=null)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=0, y=0, w=2, h=null), is "+x1.toString()+")");
		}
		if (x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		if(Type.typeof(x1.width) == ValueType.TObject) {
			DejaGnu.pass("Rectangle object .width is an object");
		} else {
			DejaGnu.fail("Rectangle object .width is not an object");
		}
		if(Type.typeof(x1.height) == ValueType.TNull) {
			DejaGnu.pass("Rectangle object .height is null");
		} else {
			DejaGnu.fail("Rectangle object .height is not null");
		}
		
		x1 = new Rectangle(0, 0, untyped o1, untyped o1);
		DejaGnu.note("x1 = new Rectangle(0, 0, untyped o1, untyped o1);");
		if (x1.toString() == "(x=0, y=0, w=2, h=2)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=0, y=0, w=2, h=2)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=0, y=0, w=2, h=2), is "+x1.toString()+")");
		}
		if (x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		if(Type.typeof(x1.width) == ValueType.TObject) {
			DejaGnu.pass("Rectangle object .width is an object");
		} else {
			DejaGnu.fail("Rectangle object .width is not an object");
		}
		if(Type.typeof(x1.height) == ValueType.TObject) {
			DejaGnu.pass("Rectangle object .height is an object");
		} else {
			DejaGnu.fail("Rectangle object .height is not an object");
		}

		x1 = new Rectangle(untyped 'string', 0, 2, 2);
		DejaGnu.note("x1 = new Rectangle(untyped 'string', 0, 2, 2);");
		if (x1.toString() == "(x=string, y=0, w=2, h=2)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=string, y=0, w=2, h=2)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=string, y=0, w=2, h=2), is "+x1.toString()+")");
		}
		if (!x1.isEmpty()) {
			DejaGnu.pass("Rectangle object is empty");
		} else {
			DejaGnu.fail("Rectangle object is not empty");
		}
		if(x1.x == untyped 'string') {
			DejaGnu.pass("x1.x == 'string'");
		} else {
			DejaGnu.fail("x1.x != 'string'");
		}
		if(x1.y == 0) {
			DejaGnu.pass("x1.y == 0");
		} else {
			DejaGnu.fail("x1.y != 0");
		}
		if(x1.width == 2) {
			DejaGnu.pass("x1.width == 2");
		} else {
			DejaGnu.fail("x1.width != 2");
		}
		if(x1.height == 2) {
			DejaGnu.pass("x1.height == 2");
		} else {
			DejaGnu.fail("x1.height != 2");
		}

		x1.width = 40;
		DejaGnu.note("x1.width = 40;");
		x1.height = untyped 'string2';
		DejaGnu.note("x1.height = 'string2';");
		x1.x = 32;
		DejaGnu.note("x1.x = 32;");
		x1.y = -30;
		DejaGnu.note("x1.y = -30;");
		if (x1.toString() == "(x=32, y=-30, w=40, h=string2)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=32, y=-30, w=40, h=string2)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=32, y=-30, w=40, h=string2), is "+x1.toString()+")");
		}
		Reflect.deleteField(x1, 'x');
		if (x1.toString() == "(x=undefined, y=-30, w=40, h=string2)") {
			DejaGnu.pass("Rectangle object is correct rectangle (x=undefined, y=-30, w=40, h=string2)");
		} else {
			DejaGnu.fail(
			"Rectangle property is incorrect rectangle (should be (x=undefined, y=-30, w=40, h=string2), is "+x1.toString()+")");
		}
        
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
		
		x1 = new Rectangle(untyped 'x', untyped 'y', untyped 'w', untyped 'h');
		DejaGnu.note("x1 = new Rectangle(untyped 'x', untyped 'y', untyped 'w', untyped 'h');");
		if(Std.is(x1.bottomRight, Point)) {
			DejaGnu.pass("x1.bottomRight is a Point");
		} else {
			DejaGnu.fail("x1.bottomRight should be a Point, is "+Type.typeof(x1.bottomRight));
		}
		if(Std.is(x1.topLeft, Point)) {
			DejaGnu.pass("x1.topLeft is a Point");
		} else {
			DejaGnu.fail("x1.topLeft should be a Point, is "+Type.typeof(x1.topLeft));
		}
		if(x1.bottomRight.toString() == '(x=xw, y=yh)') {
			DejaGnu.pass("x1.bottomRight is the correct point (x=xw, y=yh)");
		} else {
			DejaGnu.fail("x1.bottomRight should be (x=xw, y=yh), is "+x1.bottomRight.toString());
		}
		if(x1.topLeft.toString() == '(x=x, y=y)') {
			DejaGnu.pass("x1.topLeft is the correct point (x=x, y=y)");
		} else {
			DejaGnu.fail("x1.topLeft should be (x=x, y=y), is "+x1.topLeft.toString());
		}

		//ASSetPropFlags(r0, "bottomRight", 0, 4); // clear read-only (if any)
		//r0.bottomRight = 4;
		//check_equals(typeof(r0.bottomRight), 'object');

		//ASSetPropFlags(r0, "topLeft", 0, 4); // clear read-only (if any)
		//r0.topLeft = 4;
		//check_equals(typeof(r0.topLeft), 'object');
	 	
	 	//-------------------------------------------------------------
		// Test size
		//-------------------------------------------------------------
	 	
	 	x1 = new Rectangle(untyped 'x',untyped 'y',untyped 'w',untyped 'h');
		DejaGnu.note("x1 = new Rectangle(untyped 'x',untyped 'y',untyped 'w',untyped 'h');");
		if(Std.is(x1.size, Point)) {
			DejaGnu.pass("x1.size is a Point");
		} else {
			DejaGnu.fail("x1.size is not a Point");
		}
		if(x1.size.toString() == '(x=w, y=h)') {
			DejaGnu.pass("x1.size is the correct Point (x=w, y=h)");
		} else {
			DejaGnu.fail("x1.size should be (x=w, y=h), is "+x1.size.toString());
		}
		//ASSetPropFlags(r0, "size", 0, 4); // clear read-only (if any)
		//r0.size = 4;
		//check_equals(typeof(r0.topLeft), 'object');
	 	
	 	//-------------------------------------------------------------
		// Test clone
		//-------------------------------------------------------------
	 	
	 	x1 = new Rectangle(untyped 'x',untyped 'y',untyped 'w',untyped 'h');
		DejaGnu.note("x1 = new Rectangle(untyped 'x',untyped 'y',untyped 'w',untyped 'h');");
		//Reflect.setField(x1, 'custom', 4);
		untyped x1.custom = 4;
#if flash9
		var x2:Rectangle = x1.clone();
#else
		var x2:Rectangle<Int> = x1.clone();
#end
		DejaGnu.note("x2 = x1.clone();");
		//check_equals(r2.toString(), '(x=x, y=y, w=w, h=h)');
		if(x2.toString() == '(x=x, y=y, w=w, h=h)') {
			DejaGnu.pass("x2 is the correct Rectangle (x=x, y=y, w=w, h=h)");
		} else {
			DejaGnu.fail("x2 should be (x=x, y=y, w=w, h=h), is "+x2.toString());
		}
		//check_equals(r2.custom, undefined);
		if(Std.string(untyped x2.custom) == 'undefined') {
			DejaGnu.pass("no nonsense fields were cloned");
		} else {
			DejaGnu.fail("a nonsense field was cloned! x2.custom = "+Std.string(untyped x2.custom));
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
		if (Type.typeof(Reflect.callMethod(x1, Reflect.field(x1, 'contains'), [])) == ValueType.TNull) {
			DejaGnu.pass("Rectangle.contains() returns undefined");
		} else {
			DejaGnu.fail("Rectangle.contains() did not return undefined");
		}

		if (Type.typeof(Reflect.callMethod(x1, Reflect.field(x1, 'contains'), [0])) == ValueType.TNull) {
			DejaGnu.pass("Rectangle.contains(0) returns undefined");
		} else {
			DejaGnu.fail("Rectangle.contains(0) did not return undefined");
		}

		if (Type.typeof(Reflect.callMethod(x1, Reflect.field(x1, 'contains'), [0,'undefined'])) == ValueType.TNull) {
			DejaGnu.pass("Rectangle.contains(0,'undefined') returns undefined");
		} else {
			DejaGnu.fail("Rectangle.contains(0,'undefined') did not return undefined");
		}

		if (Type.typeof(Reflect.callMethod(x1, Reflect.field(x1, 'contains'), [0,null])) == ValueType.TNull) {
			DejaGnu.pass("Rectangle.contains(0,null) returns undefined");
		} else {
			DejaGnu.fail("Rectangle.contains(0,null) did not return undefined");
		}

		if (Reflect.callMethod(x1, Reflect.field(x1, 'contains'), ['1','1'])) {
			DejaGnu.pass("Rectangle.contains('1','1') returns true");
		} else {
			DejaGnu.fail("Rectangle.contains('1','1') returns false");
		}
		
		var o1 = { valueOfCalls : 0 };
		var o2 = { valueOfCalls : 0 };
		Reflect.setField(o1, 'valueOf', function() { o1.valueOfCalls++; return 3; });
		Reflect.setField(o2, 'valueOf', function() { o2.valueOfCalls++; return 2; });
		var ret = x1.contains(untyped o1, untyped o2);
		if(o1.valueOfCalls == 2) { // if ( *o1* < r0.x || *o1* >= r0.x+r0.width ) return false
			DejaGnu.pass("o1.valueOfCalls == 2");
		} else {
			DejaGnu.fail("o1.valueOfCalls != 2");
		}
		if(o2.valueOfCalls == 2) { // if ( *o2* < r0.y || *o2* >= r0.y+r0.height ) return false
			DejaGnu.pass("o2.valueOfCalls == 2");
		} else {
			DejaGnu.fail("o2.valueOfCalls != 2");
		}
		if(ret) {
			DejaGnu.pass("x1.contains(o1, o2) returns true");
		} else {
			DejaGnu.fail("x1.contains(o1, o2) returns false");
		}

		Reflect.setField(o1, 'valueOf', function() { o1.valueOfCalls++; return -1; });
		Reflect.setField(o2, 'valueOf', function() { o2.valueOfCalls++; return 2; });
		Reflect.setField(o1, 'valueOfCalls', 0);
		Reflect.setField(o2, 'valueOfCalls', 0);
		ret = x1.contains(untyped o1, untyped o2);
		if(o1.valueOfCalls == 1) { // if ( *o1* < r0.x || *o1* >= r0.x+r0.width ) return false
			DejaGnu.pass("o1.valueOfCalls == 1");
		} else {
			DejaGnu.fail("o1.valueOfCalls != 1");
		}
		if(o2.valueOfCalls == 0) { // ... (false returned above)
			DejaGnu.pass("o2.valueOfCalls == 0");
		} else {
			DejaGnu.fail("o2.valueOfCalls != 0");
		}
		if(Std.is(ret, Bool)) {
			DejaGnu.pass("Type of ret is boolean");
		} else {
			DejaGnu.fail("Type of ret should be boolean, is "+Type.typeof(ret));
		}
		if(!ret) {
			DejaGnu.pass("and ret = false");
		} else {
			DejaGnu.fail("and ret = true");
		}
		
		Reflect.setField(o1, 'valueOf', function() { o1.valueOfCalls++; return 'undefined'; });
		Reflect.setField(o2, 'valueOf', function() { o2.valueOfCalls++; return 2; });
		Reflect.setField(o1, 'valueOfCalls', 0);
		Reflect.setField(o2, 'valueOfCalls', 0);
		ret = x1.contains(untyped o1, untyped o2);
		if(o1.valueOfCalls == 2) { // if ( *o1* < r0.x || *o1* >= r0.x+r0.width ) return xxx 
			DejaGnu.xpass("o1.valueOfCalls == 2");
		} else {
			DejaGnu.xfail("o1.valueOfCalls != 2");
		}
		// Test for Y is skipped, likely because
		// the test for X evaluated to undefined anyway
		if(o2.valueOfCalls == 0) {
			DejaGnu.pass("o2.valueOfCalls == 0");
		} else {
			DejaGnu.fail("o2.valueOfCalls != 0");
		}
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of ret is undefined");
		} else {
			DejaGnu.fail("Type of ret should be undefined, is "+Type.typeof(ret));
		}
		
		Reflect.setField(o1, 'valueOf', function() { return null; });
		Reflect.setField(o2, 'valueOf', function() { return 2; });
		ret = x1.contains(untyped o1, untyped o2);
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of ret is undefined");
		} else {
			DejaGnu.fail("Type of ret should be undefined, is "+Type.typeof(ret));
		}
		
		ret = x1.contains(0/0, 2);
		if(Type.typeof(ret) == ValueType.TNull) {
			DejaGnu.pass("Type of ret is undefined");
		} else {
			DejaGnu.fail("Type of ret should be undefined, is "+Type.typeof(ret));
		}

		x1 = new Rectangle(untyped 'd',untyped 'd',untyped '10',untyped '10');
		ret = x1.contains(untyped 'e',untyped 'e');
		if(!ret) {
			DejaGnu.pass("x1.contains('e', 'e') returns false");
		} else {
			DejaGnu.fail("x1.contains('e', 'e') returns true");
		}
		
		x1 = new Rectangle(untyped 'a',untyped 'a',untyped 'b',untyped 'b');
		ret = x1.contains(untyped 'a',untyped 'a'); // 'a' >= 'a' && 'a' < 'ab'
		if(ret) {
			DejaGnu.pass("x1.contains('a', 'a') returns true");
		} else {
			DejaGnu.fail("x1.contains('a', 'a') returns false");
		}

		x1 = new Rectangle(untyped 'a',untyped 'a',untyped 'c',untyped 'c');
		ret = x1.contains(untyped 'ab',untyped 'ab'); // 'ab' >= 'ac' && 'ab' < 'ac'
		if(ret) {
			DejaGnu.pass("x1.contains('ab', 'ab') returns true");
		} else {
			DejaGnu.fail("x1.contains('ab', 'ab') returns false");
		}

		x1 = new Rectangle(untyped '2',untyped '2',untyped '10',untyped '10');
		ret = x1.contains(untyped '3',untyped '3');
		if(!ret) { // string-wise, '3' > '210' ('2'+'10')
			DejaGnu.pass("x1.contains('3', '3') returns false");
		} else {
			DejaGnu.fail("x1.contains('3', '3') returns true");
		}

		x1 = new Rectangle(untyped '2',untyped '2',untyped '10',untyped '10');
		ret = x1.contains(3, 3);
		if(ret) { // number-wise, 3 > 2 and < 210 ('2'+'10')
			DejaGnu.pass("x1.contains(3, 3) returns true");
		} else {
			DejaGnu.fail("x1.contains(3, 3) returns false");
		}

		x1 = new Rectangle(2, 2, 10, 10);
		ret = x1.contains(untyped '3',untyped '3');
		if(ret) { // number-wise, 3 > 2 && 3 < 10
			DejaGnu.pass("x1.contains('3', '3') returns true");
		} else {
			DejaGnu.fail("x1.contains('3', '3') returns false");
		}

		x1 = new Rectangle(2, 2, untyped '0',untyped '0'); // becomes 2,2,'20','20'
		ret = x1.contains(untyped '3',untyped '3');
		if(!ret) { // '3' > 2 but '3' > '20'
			DejaGnu.pass("x1.contains('3', '3') returns false");
		} else {
			DejaGnu.fail("x1.contains('3', '3') returns true");
		}

		x1 = new Rectangle(2, 2,untyped '0',untyped '0'); // becomes 2,2,'20','20'
		ret = x1.contains(3, 3);
		if(ret) { // 3 > 2 && 3 > '20'
			DejaGnu.pass("x1.contains(3, 3) returns true");
		} else {
			DejaGnu.fail("x1.contains(3, 3) returns false");
		}
	
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

