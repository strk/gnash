// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

// Initial test written by Mike Carlson
// Tests updated for haxe by Jonathan Crider 5/28/09

#if flash9
//not sure what needs to be imported here
//import flash.array;
import flash.display.MovieClip;
#else
//not sure what needs to be imported here
//import flash.array;
import flash.MovieClip;
#end
import flash.Lib;
import Array;
import Type;
import Reflect;
import Std;
import haxe.PosInfos;

// importing our testing api
import DejaGnu;

class Array_as {
	
	static var undef = untyped __global__[ "undefined" ];
	
	
    static function main() {

// Based on the test cases in the array.as file there seem to be some changes
// in array in versions earlier than 6. Since Haxe does not support flash
// versions prior to 6, these will be commented out until a later date
    

    DejaGnu.note("*** Testing Array class property and method existence");
    // Test to see if class exists
    //DejaGnu.note("array = " + Type.typeof(Array));
    if (Type.typeof(Array) == ValueType.TObject) {
        DejaGnu.pass("Array type exists [" + here.lineNumber + "]");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array type does not exist");
    }
	
	
    //DejaGnu.note("typeof: Array = " + Type.typeof(Array));
    //DejaGnu.note("class fields: " + Type.getClassFields(Array));
    //DejaGnu.note("Instance fields: " + Type.getInstanceFields(Array));
    //DejaGnu.note("CASINSENITIVE = " + Type.typeof(untyped Array.CASEINSENSITIVE));
    
    //Testing Class fields
    if ( Reflect.isObject(untyped Array.prototype)) {
        DejaGnu.pass("Array.prototype field exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype field does not exist");
    }
    
    // In order to test the array constants we must use the 'untyped' keyword.
    // This is because of the way that haxe has overrided the Array class
    if ( (Type.typeof(untyped Array.CASEINSENSITIVE) == ValueType.TInt) && 
         (untyped Array.CASEINSENSITIVE == 1) ) {
        DejaGnu.pass("Array.CASEINSENSITIVE constant exists and == 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.CASEINSENSITIVE constant does not exist or != 1");
    }
    if ( (Type.typeof(untyped Array.DESCENDING) == ValueType.TInt) && 
         (untyped Array.DESCENDING == 2) ) {
        DejaGnu.pass("Array.DESCENDING constant exists and == 2");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.DESCENDING constant does not exist or != 2");
    }
    if ( (Type.typeof(untyped Array.UNIQUESORT) == ValueType.TInt) && 
         (untyped Array.UNIQUESORT == 4) ) {
        DejaGnu.pass("Array.UNIQUESORT constant exists and == 2");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.UNIQUESORT constant does not exist or != 2");
    }
    if ( (Type.typeof(untyped Array.RETURNINDEXEDARRAY) == ValueType.TInt) && 
         (untyped Array.RETURNINDEXEDARRAY == 8) ) {
        DejaGnu.pass("Array.RETURNINDEXEDARRAY constant exists and == 8");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.RETURNINDEXEDARRAY constant does not exist or != 8");
    }
    if ( (Type.typeof(untyped Array.NUMERIC) == ValueType.TInt) && 
         (untyped Array.NUMERIC == 16) ) {
        DejaGnu.pass("Array.NUMERIC constant exists and == 16");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.NUMERIC constant does not exist or != 16");
    }
    
    // Not sure if these tests are equivalent
    // check_equals ( typeof(Array.prototype.CASEINSENSITIVE), 'undefined' );
    // check_equals ( typeof(Array.prototype.DESCENDING), 'undefined' );
    // check_equals ( typeof(Array.prototype.UNIQUESORT), 'undefined' );
    // check_equals ( typeof(Array.prototype.RETURNINDEXEDARRAY), 'undefined' );
    // check_equals ( typeof(Array.prototype.NUMERIC), 'undefined' );
    if (untyped Array.prototype.CASEINSENSITIVE == null) {
        DejaGnu.pass("Array.prototype.CASEINSENSITIVE does not exist");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.CASEINSENSITIVE exists");
    }
    if (untyped Array.prototype.DESCENDING == null) {
        DejaGnu.pass("Array.prototype.DESCENDING does not exist");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.DESCENDING exists");
    }
    if (untyped Array.prototype.UNIQUESORT == null) {
        DejaGnu.pass("Array.prototype.UNIQUESORT does not exist");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.UNIQUESORT exists");
    }
    if (untyped Array.prototype.RETURNINDEXARRAY == null) {
        DejaGnu.pass("Array.prototype.RETURNINDEXARRAY does not exist");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.RETURNINDEXARRAY exists");
    }
    if (untyped Array.prototype.NUMERIC == null) {
        DejaGnu.pass("Array.prototype.NUMERIC does not exist");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.NUMERIC exists");
    }
    
    var x1:Array<String> = new Array();
    //test for existence of common methods
    if (Type.typeof(x1.concat) == ValueType.TFunction) {
        DejaGnu.pass("Array.concat method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.concat method does not exist");
    }
    if (Type.typeof(x1.join) == ValueType.TFunction) {
        DejaGnu.pass("Array.join method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.join method does not exist");
    }
    if (Type.typeof(x1.pop) == ValueType.TFunction) {
        DejaGnu.pass("Array.pop method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.pop method does not exist");
    }
    if (Type.typeof(x1.push) == ValueType.TFunction) {
        DejaGnu.pass("Array.push method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.push method does not exist");
    }
    if (Type.typeof(x1.reverse) == ValueType.TFunction) {
        DejaGnu.pass("Array.reverse method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.reverse method does not exist");
    }
    if (Type.typeof(x1.shift) == ValueType.TFunction) {
        DejaGnu.pass("Array.shift method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.shift method does not exist");
    }
    if (Type.typeof(x1.slice) == ValueType.TFunction) {
        DejaGnu.pass("Array.slice method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.slice method does not exist");
    }
    if (Type.typeof(x1.sort) == ValueType.TFunction) {
        DejaGnu.pass("Array.sort method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.sort method does not exist");
    }
    if (Type.typeof(untyped x1.sortOn) == ValueType.TFunction) {
        DejaGnu.pass("Array.sortOn method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.sortOn method does not exist");
    }
    if (Type.typeof(x1.splice) == ValueType.TFunction) {
        DejaGnu.pass("Array.splice method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.splice method does not exist");
    }
    if (Type.typeof(x1.unshift) == ValueType.TFunction) {
        DejaGnu.pass("Array.unshift method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.unshift method does not exist");
    }
    DejaGnu.note("Note: " + Reflect.field(x1, "toString"));
    if (Type.typeof(Reflect.field(x1, "toString")) == ValueType.TFunction) {
        DejaGnu.pass("Array.toString method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.toString method does not exist");
    }
    
    // Test constructors
    // basic constructor
    var a = new Array();
    if ( Std.is(a, Array) ) {
        DejaGnu.pass("Array base constructor works");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array base constructor not working");
    }
    // haxe has a 'Magic' operator that allows us to use new in this way
    // this is equivalent to the AS3: var arr:Array = new Array(5);
    var b:Array<Int> = untyped __new__(Array, 5);
    //DejaGnu.note("b length: " + b.length);
    if ( Std.is(b, Array) && b.length == 5) {
        DejaGnu.pass("Array alt constructor: Array(int) is working");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array alt constructor: Array(int) is not working");
    }
    
#if flash9
    
    // Test existence of methods exclusive to flash9
    // these methods are not defined is haxe
    if (Type.typeof(untyped a.every) == ValueType.TFunction) {
        DejaGnu.pass("Array.every method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.every method does not exist");
    }
    
    if (Type.typeof(untyped a.filter) == ValueType.TFunction) {
        DejaGnu.pass("Array.filter method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.every method does not exist");
    }
    
    if (Type.typeof(untyped a.forEach) == ValueType.TFunction) {
        DejaGnu.pass("Array.forEach method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.forEach method does not exist");
    }
    
    if (Type.typeof(untyped a.indexOf) == ValueType.TFunction) {
        DejaGnu.pass("Array.indexOf method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.indexOf method does not exist");
    }
    
    if (Type.typeof(untyped a.lastIndexOf) == ValueType.TFunction) {
        DejaGnu.pass("Array.lastIndexOf method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.lastIndexOf method does not exist");
    }
    
    if (Type.typeof(untyped a.map) == ValueType.TFunction) {
        DejaGnu.pass("Array.map method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.map method does not exist");
    }
    
    if (Type.typeof(untyped a.some) == ValueType.TFunction) {
        DejaGnu.pass("Array.some method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.some method does not exist");
    }
    
    if (Type.typeof(Reflect.field(x1, "toLocaleString")) == ValueType.TFunction) {
        DejaGnu.pass("Array.toLocaleString method exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.toLocaleString method does not exist");
    }
    
    // equivalent to AS3: var arr:Array = new Array("one", "two", "three")
    var c:Array<String> = untyped __new__(Array, "one", "two", "three");
    //DejaGnu.note("c[0]: " + c[0] + " c[1]: " + c[1] + " c[2]: " + c[2]);
    if ( Std.is(c, Array) && c[0] == "one" && 
         c[1] == "two" && c[2] == "three") {
        DejaGnu.pass("Array alt constructor: Array(value, value) is working");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array alt constructor: Array(value, value) is not working");
    }
    
    
#else

    //  Inheritance check, AS2
    //  __proto__ field does not exist in AS3
    DejaGnu.note("__proto__: " + Type.typeof(untyped Array.prototype.__proto__));
    if ( Reflect.isObject(untyped Array.prototype.__proto__)) {
        DejaGnu.pass("Array.prototype.__proto__ field exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.__proto__ field does not exist");
    }
    if ( untyped Array.prototype.__proto__ == untyped Object.prototype) {
        DejaGnu.pass("Array.prototype.__proto__ references Object.prototype");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype.__proto__ does not reference Object.prototype");
    }
    
    // equivalent to AS3: var arr:Array = new Array("one", "two", "three")
    // note: haxe seems to pass the arguments in reverse order, but only in AS2
    var c:Array<String> = untyped __new__(Array, "three", "two", "one");
    //DejaGnu.note("c[0]: " + c[0] + " c[1]: " + c[1] + " c[2]: " + c[2]);
    if ( Std.is(c, Array) && c[0] == "one" && 
         c[1] == "two" && c[2] == "three") {
        DejaGnu.pass("Array alt constructor: Array(value, value) is working");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array alt constructor: Array(value, value) is not working");
    }

    
#end

    //check ( Array.hasOwnProperty('CASEINSENSITIVE') );
    //check ( Array.hasOwnProperty('DESCENDING') );
    //check ( Array.hasOwnProperty('UNIQUESORT') );
    //check ( Array.hasOwnProperty('RETURNINDEXEDARRAY') );
    //check ( Array.hasOwnProperty('NUMERIC') );
    //Testing hasOwnProperty for array constants
    if ( untyped Array.hasOwnProperty('CASEINSENSITIVE') ) {
        DejaGnu.pass("Array has property CASEINSENSITIVE");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array does not have property CASEINSENSITIVE");
    }
    if ( untyped Array.hasOwnProperty('DESCENDING') ) {
        DejaGnu.pass("Array has property DESCENDING");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array does not have property DESCENDING");
    }
    if ( untyped Array.hasOwnProperty('UNIQUESORT') ) {
        DejaGnu.pass("Array has property UNIQUESORT");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array does not have property UNIQUESORT");
    }
    if ( untyped Array.hasOwnProperty('RETURNINDEXEDARRAY') ) {
        DejaGnu.pass("Array has property RETURNINDEXEDARRAY");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array does not have property RETURNINDEXEDARRAY");
    }
    if ( untyped Array.hasOwnProperty('NUMERIC') ) {
        DejaGnu.pass("Array has property NUMERIC");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array does not have property NUMERIC");
    }
    
    //check(Array.prototype.hasOwnProperty('concat'));
    if (untyped Array.prototype.hasOwnProperty('concat')) {
        DejaGnu.pass("Array.prototype 'concat' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'concat' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('join'));
    if (untyped Array.prototype.hasOwnProperty('join')) {
        DejaGnu.pass("Array.prototype 'join' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'join' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('pop'));
    if (untyped Array.prototype.hasOwnProperty('pop')) {
        DejaGnu.pass("Array.prototype 'pop' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'pop' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('push'));
    if (untyped Array.prototype.hasOwnProperty('push')) {
        DejaGnu.pass("Array.prototype 'push' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'push' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('reverse'));
    if (untyped Array.prototype.hasOwnProperty('reverse')) {
        DejaGnu.pass("Array.prototype 'reverse' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'reverse' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('shift'));
    if (untyped Array.prototype.hasOwnProperty('shift')) {
        DejaGnu.pass("Array.prototype 'shift' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'shift' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('slice'));
    if (untyped Array.prototype.hasOwnProperty('slice')) {
        DejaGnu.pass("Array.prototype 'slice' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'slice' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('sort'));
    if (untyped Array.prototype.hasOwnProperty('sort')) {
        DejaGnu.pass("Array.prototype 'sort' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'sort' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('sortOn'));
    if (untyped Array.prototype.hasOwnProperty('sortOn')) {
        DejaGnu.pass("Array.prototype 'sortOn' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'sortOn' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('splice'));
    if (untyped Array.prototype.hasOwnProperty('splice')) {
        DejaGnu.pass("Array.prototype 'splice' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'splice' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('unshift'));
    if (untyped Array.prototype.hasOwnProperty('unshift')) {
        DejaGnu.pass("Array.prototype 'unshift' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'unshift' property does not exist");
    }
    //check(Array.prototype.hasOwnProperty('toString'));
    if (untyped Array.prototype.hasOwnProperty('toString')) {
        DejaGnu.pass("Array.prototype 'toString' property exists");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.prototype 'toString' property does not exist");
    }
    //check(!Array.prototype.hasOwnProperty('length'));
    if (untyped !Array.prototype.hasOwnProperty('length')) {
        DejaGnu.pass("Array.prototype 'length' property exists");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] Array.prototype 'length' property does not exist");
    }
    //check(!Array.prototype.hasOwnProperty('valueOf'));
    if (untyped !Array.prototype.hasOwnProperty('valueOf')) {
        DejaGnu.pass("Array.prototype 'valueOf' property exists");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] Array.prototype 'valueOf' property does not exist");
    }
    //check(!Array.prototype.hasOwnProperty('size'));
    if (untyped !Array.prototype.hasOwnProperty('size')) {
        DejaGnu.pass("Array.prototype 'size' property exists");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] Array.prototype 'size' property does not exist");
    }
    if (Reflect.isObject(untyped Array())) {
        DejaGnu.pass("Array() function returns an object");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array() function does not reaturn an object");
    }
    if (Reflect.isObject(new Array())) {
        DejaGnu.pass("new Array() constructor returns an object");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] new Array() constructor does not return and object");
    }
    
    //===============================================================
    // Array functionality testing
    //===============================================================
    
    DejaGnu.note("***  Begin testing Array functionality  ***");
    #if !flash9
    //  Testing ASnative function pointers
    //  ASnative does not exist in flash 9 or later
    DejaGnu.note("Functionality testing with arrays a and b, and some others");
    var f = untyped ASnative(252, 0);
    if (Type.typeof(f) == ValueType.TFunction) {
        DejaGnu.pass("f = ASnative(252, 0) returns a function");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] f = ASnative(252, 0) does not return a function");
    }
    var a = f();
    if (Reflect.isObject(a)) {
        DejaGnu.pass("Assignment a = f(); properly returns an array object");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Assignment a = f(); does not return an array object");
    }
    if (Type.typeof(a.pop) == ValueType.TFunction) {
        DejaGnu.pass("a.pop array method properly initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.pop array method not initialized properly");
    }
    #end

    
    var a;
    var popped;
    a = [551, "asdf", 12];
    // DejaGnu.note("a size field = " + Reflect.field(a, "size"));
    if (Reflect.field(a, "size") == null) {
        DejaGnu.pass("a has no 'size' property which is correct");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a incorrectly initialize with a 'size' property");
    }
    //DejaGnu.note("a type: " + Type.typeof(a));
    //DejaGnu.note("a classtype: " + Type.getClass(a));
    //DejaGnu.note("a classtype: " + Type.getClassName(Type.getClass(a)));
    if (Type.getClassName(Type.getClass(a)) == "Array") {
        DejaGnu.pass("a properly constructed as an Array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a not properly constructed as an Array");
    }
    if ( untyped a.hasOwnProperty('length') ) {
        DejaGnu.pass("a correctly initialized with length property");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.length property missing");
    }
    if ( a.length == 3 ) {
        DejaGnu.pass("a.length property initalized properly to 3");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.length property returns incorrect value");
    }
    
    var primitiveArrayValue = Reflect.callMethod(a, Reflect.field(a,"valueOf"),[]);
    if (Reflect.isObject(primitiveArrayValue)) {
        DejaGnu.pass("a.valueOf returns an object");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.valueOf does not return an object");
    }
    if (primitiveArrayValue == a) {
        DejaGnu.pass("a.valueOf returns itself");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.valueOf does not return itself");
    }
    
    //Not sure how to implement this
    //#if OUTPUT_VERSION > 5
    //check( primitiveArrayValue === a );
    //#endif
    
    //create array b with same elements as a
    var b:Array<Dynamic> = new Array();
    Reflect.callMethod(b, Reflect.field(b,"push"), [551,"asdf",12]);
    //b.push(551);
    //b.push("asdf");
    //b.push(12);
    
    
    if ( Reflect.isObject(a)) {
        DejaGnu.pass("Array a is still initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array a is no longer an object");
    }
    if (a != b) {
        DejaGnu.pass("a != b as expected");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a == b which should not happen");
    }
    
    var tmp = untyped __new__(Array, 2);
    if (tmp.length == 2) {
        DejaGnu.pass("tmp Array properly initialized to length 2");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] tmp Array not properly initialized");
    }
    
    #if (flash6 || flash9)
    //DejaGnu.note("tmp.toString() = " + tmp.toString());
    if ( tmp.toString() == ",") {
        DejaGnu.pass("tmp Array elements correctly initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] tmp Array elements not initialized correctly");
    }
    #else
    if ( tmp.toString() == "undefined,undefined") {
        DejaGnu.pass("tmp Array elements correctly initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] tmp Array elements not initialized correctly");
    }
    #end 
    tmp = untyped __new__(Array, "two");
    if ( tmp.length == 1 ) {
        DejaGnu.pass("tmp Array properly re-initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] tmp Array was not properly re-initialized");
    }
    
    // Testing pop and push functions
    DejaGnu.note("*** Testing array pop, push, join, and reverse");
    if (a.length == 3) {
        DejaGnu.pass("a.length still == 3");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.length does not == 3");
    }   
    if (a[2] == 12) {
        DejaGnu.pass("a[2] element == 12");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a[2] element not assigned value 12");
    }
    popped = a.pop();
    if (popped == 12) {
        DejaGnu.pass("a.pop properly returned the value 12");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.pop did no return the correct value");
    }
    // DejaGnu.note("a[2] = " + a[2]);
    if (a[2] == null) {
        DejaGnu.pass("a.pop properly removes last element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.pop does not remove the last element");
    }
    if (a[1] == "asdf") {
        DejaGnu.pass("a[1] element still == 'asdf'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a[1] no longer contains correct value");
    }
    a[1] = a[0];
    if (a[1] == 551) {
        DejaGnu.pass("array element assignment works correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array element assignment does not work");
    }
    a[0] = 200;
    if (a[0] == 200) {
        DejaGnu.pass("array integer assignment works");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array integer assignment does not work");
    }
    if (a.toString() == "200,551") {
        DejaGnu.pass("a.toString method now returns new correct elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.toString method not returning new elements");
    }
    a.push(7);
    a.push(8);
    a.push(9);
    if (a.length == 5) {
        DejaGnu.pass("a.push method correctly added 3 elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.push method not working properly");
    }
    if (a[100] == null) {
        DejaGnu.pass("a.push working correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.push added too many elements");
    }
    if (a[5] == null) {
        DejaGnu.pass("a.push adds the correct number of elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.push added an extra element");
    }
    if (a[4] == 9) {
        DejaGnu.pass("a.push added correct element to the end of the array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.push did not add the correct element at the end");
    }
    if (a.join(",") == "200,551,7,8,9") {
        DejaGnu.pass("a.join returns correct string");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.join does not return correct string");
    }
    
    a.reverse();
    if (a.join(",") == "9,8,7,551,200") {
        DejaGnu.pass("a.reverse correctly reverses the array order");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.reverse does not correctly reverse the array order");
    }
    
    //  These two tests do not work for swf version < 5, but haxe does not 
    //  compile to those versions anyway
    if (untyped Array.prototype.join.apply(a) == "9,8,7,551,200") {
        DejaGnu.pass("apply function properly called from Array.prototype");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] apply function does not work from Array.prototype");
    }
    if (untyped a.join.apply(a) == "9,8,7,551,200") {
        DejaGnu.pass("apply function properly called from a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] apply function does not work from a");
    }
    
    if (a.join("test") == "9test8test7test551test200") {
        DejaGnu.pass("a.join seperator argument works");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.join seperator argument does not work");
    }
    
    // Test one of our sorting type members
    if (untyped Array.UNIQUE == null) {
        DejaGnu.pass("Array.UNIQUE is undefined as expected");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array.UNIQUE is not undefined which is unexpected");
    }
    
    // bitwise operator tests
    // the following tests do not belong here, but
    // better somewhere then nowhere (are here due to
    // a typo in this testcase triggering this bug)
    //
    DejaGnu.note("*** Testing bitwise operators (don't seem to be tested elsewhere)");
    if ( (untyped "undefined" | 1) == 1) {
        DejaGnu.pass("undefined | 1 == 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] undefined | 1 != 1");
    }
    if ( (1 | untyped "undefined") == 1) {
        DejaGnu.pass("1 | undefined == 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] 1 | undefined != 1");
    }
    if ( (untyped "undefined" & 1)  == 0) {
        DejaGnu.pass("undefined & 1 == 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] undefined & 1 != 0");
    }
    if ( (1 & untyped "undefined") == 0) {
        DejaGnu.pass("1 & undefined == 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] 1 & undefined != 0");
    }
    if ( (untyped "undefined"^1) == 1) {
        DejaGnu.pass("undefined^1 == 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] undefined^1 != 1");
    }
    if ( (1^untyped "undefined") == 1) {
        DejaGnu.pass("1^undefined == 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] 1^undefined != 1");
    }   
    if ( (untyped Array.UNIQUE | untyped Array.CASEINSENSITIVE | 
          untyped Array.RETURNINDEXEDARRAY) == 9) {
        DejaGnu.pass("Bitwise-or | operator works properly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Bitwise-or | is not working");
    }
    

    // NOTE: The Reflect.callMethod() function ended up being very important
    //       here. haXe will not let us call sort() with no arguments.
    //       The only test I haven't been able to figure out is the
    //       following one. 
    
    //a.sort( Array.UNIQUESORT | Array.DESCENDING | Array.NUMERIC);
    //check_equals (a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );
    
    // Testing sort functions
    DejaGnu.note("*** Testing sort functions");
    // a.sort();
    Reflect.callMethod( a, Reflect.field(a, "sort"), []);
    if (a.toString() == "200,551,7,8,9") {
        DejaGnu.pass("a.sort() method correctly sorted a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.sort() method did not correctly sort a");
    }
    
    // a.push(200,7,200,7,200,8,8,551,7,7);
    Reflect.callMethod(a, Reflect.field(a, "push"), [200,7,200,7,200,8,8,551,7,7]);

    //a.sort(Array.NUMERIC);
    Reflect.callMethod( a, Reflect.field(a, "sort"), [untyped Array.NUMERIC]);
    
    // check_equals ( a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );
    if (a.toString() == "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551") {
        DejaGnu.pass("a.sort(NUMERIC) correctly sorts the array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.sort(NUMERIC) does not correctly sort the array");
    }
    
    //FIXED
    Reflect.callMethod( a, Reflect.field(a, "sort"), [untyped Array.UNIQUESORT  |
                        untyped Array.DESCENDING | untyped Array.NUMERIC]);
    // check_equals (a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" )
    if (a.toString() == "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551") {
        DejaGnu.pass("a.sort() with | operator correctly sorts the array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.sort() with | operator does not correctly sort the array");
    }
    
    // Test multi-parameter constructor, and keep testing sort cases
    var trysortarray = untyped __new__(Array, "But", "alphabet", "Different",
                                       "capitalization");
    // trysortarray.sort( Array.CASEINSENSITIVE );
    Reflect.callMethod( trysortarray, Reflect.field(trysortarray, "sort"),
                       [untyped Array.CASEINSENSITIVE]);
    if (trysortarray.toString() == "alphabet,But,capitalization,Different") {
        DejaGnu.pass("sort(CASEINSENSITIVE) correctly sorts the array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(CASEINSENSITIVE) does not correctly sort the array");
    }
    // trysortarray.sort();
    Reflect.callMethod( trysortarray, Reflect.field(trysortarray, "sort"), []);
    if (trysortarray.toString() == "But,Different,alphabet,capitalization") {
        DejaGnu.pass("sort() correctly sorts string array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() does not correctly sort string array");
    }
    
    
    // testing array with unassigned indexes
    DejaGnu.note("*** Testing sparse array");
    var gaparray = [];
    gaparray [4] = '4';
    gaparray [16] = '16';
    if (gaparray.length == 17) {
        DejaGnu.pass("Array with unfilled indexes initialized with correct length");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Array with unfilled indexes not initialized correctly");
    }
    if (gaparray[4] == '4') {
        DejaGnu.pass("gaparray[4] correctly assigned");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray[4] not correctly assigned");
    }
    if (gaparray[16] == '16') {
        DejaGnu.pass("gaparray[16] correctly assigned");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray[16] not correctly assigned");
    }
    if (untyped gaparray.hasOwnProperty('4')) {
        DejaGnu.pass("gaparray property '4' correctly initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray property '4' not correctly initialized");
    }
    if (untyped gaparray.hasOwnProperty('16')) {
        DejaGnu.pass("gaparray property '16' correctly initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray property '16' not correctly initialized");
    }
    if ( !(untyped gaparray.hasOwnProperty('0'))) {
        DejaGnu.pass("gaparray property '0' correctly not found");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] extra gaparray property '0' not found");
    }
    if ( !(untyped gaparray.hasOwnProperty('1'))) {
        DejaGnu.pass("gaparray property '1' correctly not found");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] extra gaparray property '1' not found");
    }
    // gaparray.sort();
    Reflect.callMethod( gaparray, Reflect.field(gaparray, "sort"), []);
    if (gaparray.length == 17) {
        DejaGnu.pass("After sort gaparry retains correct length");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] After sort gaparray has incorrect length");
    }
    
    //not sure if these should be ported since gnash seems to give wrong values
    #if flash6
    //xcheck_equals(gaparray[0], undefined); // this is 16 with gnash
    if (gaparray[0] == null) {
        DejaGnu.xpass("gaparray[0] contains correct value");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray[0] does not contain correct value");
    }
    //xcheck_equals(gaparray[1], undefined); // this is 4 with gnash
    if (gaparray[1] == null) {
        DejaGnu.xpass("gaparray[1] contains correct value");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray[1] does not contain correct value");
    }
    #else
    if (gaparray[0] == '16') {
        DejaGnu.pass("gaparray[0] contains correct value");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray[0] does not contain correct value");
    }
    if (gaparray[1] == '4') {
        DejaGnu.pass("gaparray[1] contains correct value");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray[1] does not contain correct value");
    }
    #end
    
    if (gaparray[2] == null) {
        DejaGnu.pass("empty gaparray index 2 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 2 is no longer empty");
    }
    if (gaparray[3] == null) {
        DejaGnu.pass("empty gaparray index 3 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 3 is no longer empty");
    }
    if (gaparray[4] == null) {
        DejaGnu.pass("empty gaparray index 4 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 4 is no longer empty");
    }
    if (gaparray[5] == null) {
        DejaGnu.pass("empty gaparray index 5 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 5 is no longer empty");
    }
    if (gaparray[6] == null) {
        DejaGnu.pass("empty gaparray index 6 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 6 is no longer empty");
    }
    if (gaparray[7] == null) {
        DejaGnu.pass("empty gaparray index 7 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 7 is no longer empty");
    }
    if (gaparray[8] == null) {
        DejaGnu.pass("empty gaparray index 8 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 8 is no longer empty");
    }
    if (gaparray[9] == null ) {
        DejaGnu.pass("empty gaparray index 9 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 9 is no longer empty");
    }
    if (gaparray[10] == null ) {
        DejaGnu.pass("empty gaparray index 10 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 10 is no longer empty");
    }
    if (gaparray[11] == null ) {
        DejaGnu.pass("empty gaparray index 11 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 11 is no longer empty");
    }
    if (gaparray[12] == null) {
        DejaGnu.pass("empty gaparray index 12 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 12 is no longer empty");
    }
    if (gaparray[13] == null) {
        DejaGnu.pass("empty gaparray index 13 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 13 is no longer empty");
    }
    if (gaparray[14] == null) {
        DejaGnu.pass("empty gaparray index 14 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 14 is no longer empty");
    }
    if (gaparray[15] == null) {
        DejaGnu.pass("empty gaparray index 15 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 15 is no longer empty");
    }
    if (gaparray[16] == null) {
        DejaGnu.pass("empty gaparray index 16 still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty gaparray index 16 is no longer empty");
    }
    
    #if flash6
    //xcheck(gaparray.hasOwnProperty('15'));
    if (untyped gaparray.hasOwnProperty('15')) {
        DejaGnu.xpass("gaparray property '15' has something in it for some reason");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray property '15' is now undefined");
    }
    //xcheck(gaparray.hasOwnProperty('16'));
    if (untyped gaparray.hasOwnProperty('16')) {
        DejaGnu.xpass("gaparray property '16' has something in it for some reason");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray property '16' is now undefined");
    }
    //xcheck(gaparray.hasOwnProperty('4')); // a-ha!
    if (untyped gaparray.hasOwnProperty('4')) {
        DejaGnu.xpass("gaparray property '4' has something in it for some reason");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray property '4' is now undefined");
    }
    //xcheck(!gaparray.hasOwnProperty('0'));
    if ( !(untyped gaparray.hasOwnProperty('0'))) {
        DejaGnu.xpass("gaparray property '0' has something in it for some reason");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray property '0' is now undefined");
    }
    #else
    if (untyped gaparray.hasOwnProperty('16')) {
        DejaGnu.xpass("gaparray property '16' has something in it for some reason");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray property '16' is now undefined");
    }
    if (untyped gaparray.hasOwnProperty('4')) {
        DejaGnu.xpass("gaparray property '4' has something in it for some reason");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] gaparray property '4' is now undefined");
    }
    if (untyped gaparray.hasOwnProperty('1')) {
        DejaGnu.pass("gaparray property '1' has something in it");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] gaparray property '1' is now undefined");
    }
    #end
    
    //NOTE: Based on some results from these tests, it looks like there could be
    //      a bug in the gnash array sort or push function
    //DejaGnu.note("gaparray = " + gaparray.toString());
    //var tmp2:Array<String> = new Array();
    var tmp2 = [];
    //DejaGnu.note("tmp2 = " + tmp2.toString());
    for (v in gaparray) {
        untyped tmp2.push(untyped v);
        //DejaGnu.note("tmp2 = " + tmp2.toString());
    }
    //DejaGnu.note("tmp2 = " + tmp2.toString());
    // tmp2.sort();
    Reflect.callMethod( tmp2, Reflect.field(tmp2, "sort"), []);
    #if flash6
    // 4, 15 and 16
    if (tmp2.length == 3) {
        DejaGnu.xpass("tmp2 now has length 3");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2 does not have length 3");
    }
    if (tmp2[0] == '15') {
        DejaGnu.xpass("tmp2[0] contains '15'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[0] does not contain '15'");
    }
    if (tmp2[1] == '16') {
        DejaGnu.xpass("tmp2[1] contains '16'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[1] does not contain '16'");
    }
    if (tmp2[2] == '4') {
        DejaGnu.xpass("tmp2[2] contains '4'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[2] does not contain '4'");
    }
    #else
    // 0, 1, 2, 4, 16 
    //DejaGnu.note("tmp2 = " + tmp2.toString());
    if (tmp2.length == 5) {
        DejaGnu.xpass("tmp2 now has length 5");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2 does not have length 5");
    }
    DejaGnu.note("***These next two cases pass in ming which seems very bogus");
    if (tmp2[0] == '0') {
        DejaGnu.xpass("tmp2[0] now contains '0'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[0] does not contain'0'");
    }
    if (tmp2[1] == '1') {
        DejaGnu.xpass("tmp2[1] now contains '1'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[1] does not contain'1'");
    }
    if (tmp2[2] == '16') {
        DejaGnu.xpass("tmp2[2] contains '16'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[2] does not contain '16'");
    }
    if (tmp2[3] == '2') {
        DejaGnu.xpass("tmp2[3] contains '2'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[3] does not contain '2'");
    }
    if (tmp2[4] == '4') {
        DejaGnu.xpass("tmp2[4] contains '4'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] tmp2[4] does not contain '4'");
    }
    #end

	/*
	 * 
	tmp = []; for (v in gaparray) tmp.push(v);
	tmp.sort();
	#if OUTPUT_VERSION < 7
	 xcheck_equals(tmp.length, '3'); // 4, 15 and 16
	 xcheck_equals(tmp[0], '15');
	 xcheck_equals(tmp[1], '16');
	 xcheck_equals(tmp[2], '4');
	#else
	 xcheck_equals(tmp.length, '5'); // 0, 1, 2, 4, 16 
	 check_equals(tmp[0], '0');
	 check_equals(tmp[1], '1');
	 xcheck_equals(tmp[2], '16');
	 xcheck_equals(tmp[3], '2');
	 xcheck_equals(tmp[4], '4');
	#endif
	*/


    //-----------------------------------------------------
    // Test sorting using a custom comparison function
    //-----------------------------------------------------

    var testCmpCalls = 0;
    var testCmpThis = "not set";
    var testCmp = function (x,y) {
        // Gnash fails here by *requiring* a not-null 'this_ptr' in fn_call
        // NOTE: we can't rely on the number of calls to this function,
        //       which is implementation-defined
        // HaXe NOTE: we cannot access 'this' from a local function
        //if ( testCmpCalls++ ) testCmpThis=this;
        
        if (x.length < y.length) { return -1; }
        if (x.length > y.length) { return 1; }
        return 0;
    }
    DejaGnu.note("*** Testing Custom Comparison function");
    if (trysortarray.toString() == "But,Different,alphabet,capitalization") {
        DejaGnu.pass("trysortarray still in initial sort order");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] trysortarray is not in initial sort order");
    }
    trysortarray.sort( testCmp );
    if (trysortarray.toString() == "But,alphabet,Different,capitalization") {
        DejaGnu.pass("trysortarray was correctly sorted with custom function");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] trysortarray was not correctly sorted with custom function");
    }
    //DejaGnu.note("testCmpThis = " + Type.typeof(testCmpThis));
    if (Type.typeof( testCmpThis ) == null) {
        DejaGnu.xpass("testCmpThis == null");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] testCmpThis != null");
    }
    // original ming test writer didn't think this mattered much
    // probably based on the note in the compare function
    if (testCmpCalls == 7) {
        DejaGnu.xpass("testCmpCalls == 7");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] testCmpCalls != 7");
    }
    
    //DejaGnu.note("array = " + trysortarray.toString());
    var testCmpBogus1 = function (x,y) {return -1;}
    trysortarray.sort( testCmpBogus1 );
    // this sort fails in gflashplayer. does as3 iterate or sort differently?
#if !flash9
    if (trysortarray.toString() == "But,alphabet,Different,capitalization") {
        DejaGnu.pass("custom sort returned correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] custom sort did not return"+
                     " correct array; trysortarray = "+trysortarray.toString());
    }
#else
    // flash9 returns this instead, but I'm not sure why, maybe it compares the
    // elements in a different order (or Haxe does)
    if (trysortarray.toString() == "capitalization,alphabet,But,Different") {
        DejaGnu.pass("custom sort returned correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] custom sort did not return"+
                     " correct array; trysortarray = "+trysortarray.toString());
    }
#end
    
    var testCmpBogus2 = function (x,y) {return 1;}
    trysortarray.sort( testCmpBogus2 );
    if (trysortarray.toString() == "alphabet,Different,capitalization,But") {
        DejaGnu.xpass("custom sort returned correct array");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] custom sort did not return correct array");
    }
    
    var testCmpBogus3 = function (x,y) {return 0;}
    trysortarray.sort( testCmpBogus3 );
    if (trysortarray.toString() == "alphabet,Different,capitalization,But") {
        DejaGnu.xpass("custom sort returned correct array");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] custom sort did not return correct array");
    }
    
    #if !flash9
    var testCmpBogus4 = function (x,y) {return untyped tmp2++%2;}
    trysortarray.sort( testCmpBogus4 );
    if (trysortarray.toString() == "alphabet,Different,capitalization,But") {
        DejaGnu.xpass("custom sort returned correct array");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] custom sort did not return correct array");
    }
    #else
    // The testCmpBogus4 function can not be used in as3 because null may not
    // be used as an int
    #end
    
    var testCmpBogus5 = function (x,y) { trysortarray.pop(); return -1;}
    trysortarray.sort( testCmpBogus5 );
    if (trysortarray.length == 0) {
        DejaGnu.xpass("custom sort returned length 0 array");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] custom sort did not return length 0 array");
    }
    
    // what is this testing?
    var trysortarray2 = [1,2,3,4];
    var testCmpBogus6 = function (x,y) { trysortarray2.pop(); return 1;}
    if (trysortarray2.toString() == "1,2,3,4") {
        DejaGnu.pass("new array correctly initialized: sanity checked");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] new array not correctly initialized");
    }
    if (trysortarray2.length == 4) {
        DejaGnu.pass("new array has correct length: sanity checked");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] new array does not have correct length");
    }
    trysortarray2.sort( testCmpBogus6 );
    if (trysortarray2.length == 4) {
        DejaGnu.pass("array still has correct length");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array does not have correct length");
    }
    if (trysortarray2.toString() == "2,3,4,1") {
        DejaGnu.xpass("custom sort returned '2,3,4,1'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] custom sort did not return '2,3,4,1'");
    }
    
    
    //-----------------------------------------------------
    // Test non-integer and insane indices.
    //-----------------------------------------------------
    // FIXME: This next group of tests does not work very well because of
    //        incompatibilities between haxe/gnash/actionscript. I have written
    //        some of the test but will leave the rest in the comments until we
    //        can figure out a way to write them reliably. I'm not even sure all
    //        the ming test cases are legitimate

    DejaGnu.note("*** Testing non-integer and insane indices");

    var c = ["zero", "one", "two", "three"];


    if (Reflect.isObject(c)) {
        DejaGnu.pass("new array c is an object: sanity checked");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] new array c is not an object");
    }
    
    c[untyped 1.1] = "one point one";
    c[-3] = "minus three";
    
    if (c[0] == "zero") {
        DejaGnu.pass("c[0] == 'zero'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0] != 'zero'");
    }
    if (c[1] == "one") {
        DejaGnu.pass("c[1] == 'one'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1] != 'one'");
    }
    if (c[untyped 1.1] == "one point one") {
        DejaGnu.pass("c[1.1] == 'one point one'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1.1] != 'one point one'");
    }
    if (c[untyped 1.9] == null) {
        DejaGnu.pass("c[1.9] == 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1.9] != 'undefined'");
    }
    if (c[-3] == "minus three") {
        DejaGnu.pass("c[-3] == 'minus three'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[-3] == 'minus three'");
    }
    if (c[untyped -3.7] == null) {
        DejaGnu.pass("c[-3.7] == 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[-3.7] != 'undefined'");
    }
    
    untyped c[-2147483649] = "too low";
    if (c[0] == null) {
        DejaGnu.xpass("c[0] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[0] is not 'undefined'");
    }
    if (c[1] == null) {
        DejaGnu.xpass("c[1] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[1] is not 'undefined'");
    }
    if (c[2] == null) {
        DejaGnu.xpass("c[2] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[2] is not 'undefined'");
    }
    if (c[3] == null) {
        DejaGnu.xpass("c[3] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[3] is not 'undefined'");
    }
    if (c[untyped 1.1] == "one point one") {
        DejaGnu.pass("c[1.1] == 'one point one'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1.1] != 'one point one'");
    }
    if (c[untyped -2147483649] == "too low") {
        DejaGnu.pass("c[-2147483649] == 'too low'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[-2147483649] != 'too low'");
    }
    // not sure what original test comment means here
    // doesn't set the int(-2147483649) element:
    #if !(flash9 || flash6)
    if (c[untyped int(-2147483649)] == null) {
        DejaGnu.pass("c[int(-2147483649)] == 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[int(-2147483649)] != 'undefined'");
    }
    #end
    #if flash9
    // FIXME: gflashplayer crashes on this test. Not sure what this is testing.
    //        is this supposed to be a type cast?
    // check_equals (c[int(-2147483649)], undefined); 
    //if (c[untyped 'int'(-2147483649)] == null) {
        //DejaGnu.pass("c[int(-2147483649)] == 'undefined'");
    //} else {
        //DejaGnu.fail("c[int(-2147483649)] != 'undefined'");
    //}
    #end
    
    c[untyped 2147483649] = "too high";
    if (c[untyped -2147483649] == "too low") {
        DejaGnu.pass("c[-2147483649] == 'too low'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[-2147483649] != 'too low'");
    }
    if (c[untyped 2147483649] == "too high") {
        DejaGnu.pass("c[2147483649] == 'too high'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[2147483649] != 'too high'");
    }
    if (c[1] == null) {
        DejaGnu.xpass("c[1] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[1] is not 'undefined'");
    }
    if (c[2] == null) {
        DejaGnu.xpass("c[2] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[2] is not 'undefined'");
    }
    if (c[3] == null) {
        DejaGnu.xpass("c[3] == 'undefined'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[3] is not 'undefined'");
    }
    // unexpected behavior here in gflashplayer. Is this sequence of testing
    // legitimate
    if (c.length == untyped -2147483646) {
        DejaGnu.xpass("c.length == -2147483646");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c.length != -2147483646");
    }
    
    
    // More checking of crazy stuff
    // do iterators work differently in ming/haxe/actionscript?
    // iterator doesn't do the same thing in haxe. Changing this to xfail until
    // a solution is found.
    var str:String = new String("");
    for (i in c) {
        str += i + ": " + untyped c[i] + "; ";
    }
    //DejaGnu.note("str = " + str);
    if (str == "2147483649: too high; -2147483649: too low; -2147483648: lowest int; -3: minus three; 1.1: one point one; ") {
        DejaGnu.xpass("str now contains correct value");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] str does not contain correct value");
    }
    
    c = ["zero", "one", "two", "three"];
    c[untyped 1.1] = "one point one";
    c[-3] = "minus three";
    // DejaGnu.note("c = " + c.toString());
    if (c[0] == "zero") {
        DejaGnu.pass("c[0] == 'zero'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0] != 'zero'");
    }
    if (c[1] == "one") {
        DejaGnu.pass("c[1] == 'one'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1] != 'one'");
    }
    
    c[0xffffffff + 1] = "too high";
    // DejaGnu.note("c[0] = " + c[0]);
    if (c[0] == "zero") {
        DejaGnu.xpass("c[0] == 'zero'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[0] != 'zero'");
    }
    if (c[1] == "one") {
        DejaGnu.pass("c[1] == 'one'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1] != 'one'");
    }
    if (c[0xffffffff] == null) {
        DejaGnu.pass("c[0xffffffff] is 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0xffffffff] is not 'undefined'");
    }
    if (c[0xffffffff + 1] == "too high") {
        DejaGnu.pass("c[0xffffffff + 1] == 'too high'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0xffffffff + 1] != 'too high'");
    }



    untyped c[ untyped 0xfffffffffffffffff] = "much too high";
    //check_equals (c[0xfffffffffffffffff], "much too high");
    if (c[ untyped 0xfffffffffffffffff] == "much too high") {
        DejaGnu.pass("c[0xfffffffffffffffff] == 'much too high'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0xfffffffffffffffff] != 'much too high'");
    }
    
    // Also no problem. Looks like a fairly crappy bug to me.
    c[untyped -2147483650] = "still lower";
    // DejaGnu.note("ln1402 c[0] = " + c[0]);
    if (c[0] == "zero") {
        DejaGnu.xpass("c[0] == 'zero'");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c[0] != 'zero'");
    }
    if (c[1] == "one") {
        DejaGnu.pass("c[1] == 'one'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1] != 'one'");
    }
    if (c.length == 2147483647) {
        DejaGnu.xpass("c.length == 2147483647");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c.length != 2147483647");
    }
    
    // NOTE: These are the tests we were trying to duplicate in the tests above.
    //       I don't know if these are legitimate things to try to test in haxe.
    //       It's not clear whether the problem is with haxe or gnash. 
//c = ["zero", "one", "two", "three"];
//check_equals(typeof(c), "object");

//c[1.1] = "one point one";
//c[-3] = "minus three";

//check_equals (c[0], "zero");
//check_equals (c[1], "one");
//check_equals (c[1.1], "one point one");
//check_equals (c[1.9], undefined);
//check_equals (c[-3], "minus three");
//check_equals (c[-3.7], undefined);

//c[-2147483648] = "lowest int";
//check_equals (c[0], "zero");
//check_equals (c[1], "one");

//// This appears to invalidate integer indices, but
//// not non-integer ones.
//c[-2147483649] = "too low";
//xcheck_equals (c[0], undefined);
//xcheck_equals (c[1], undefined);
//xcheck_equals (c[2], undefined);
//xcheck_equals (c[3], undefined);
//check_equals (c[1.1], "one point one");
//check_equals (c[-2147483649], "too low");
//// doesn't set the int(-2147483649) element:
//check_equals (c[int(-2147483649)], undefined); 

//c[2147483649] = "too high";
//check_equals (c[-2147483649], "too low");
//check_equals (c[2147483649], "too high");
//xcheck_equals (c[1], undefined);
//xcheck_equals (c[2], undefined);
//xcheck_equals (c[3], undefined);

//xcheck_equals (c.length, -2147483646);

//str = "";

//for (i in c)
//{
    //str += i + ": " + c[i] + "; ";
//}
//xcheck_equals(str, "2147483649: too high; -2147483649: too low; -2147483648: lowest int; -3: minus three; 1.1: one point one; ");

//c = ["zero", "one", "two", "three"];
//c[1.1] = "one point one";
//c[-3] = "minus three";

//check_equals (c[0], "zero");
//check_equals (c[1], "one");

//// No problem...
//c[0xffffffff + 1] = "too high";
//check_equals (c[0], "zero");
//check_equals (c[1], "one");
//check_equals (c[0xffffffff], undefined);
//check_equals (c[0xffffffff + 1], "too high");

//c[0xfffffffffffffffff] = "much too high";
//check_equals (c[0xfffffffffffffffff], "much too high");

//// Also no problem. Looks like a fairly crappy bug to me.
//c[-2147483650] = "still lower";
//check_equals (c[0], "zero");
//check_equals (c[1], "one");

////xcheck_equals (c.length, 2147483647);

//DejaGnu.note("c.length" + c.length);
//var str= "";

//for (i in 0...c.length)
//{
    //str += i + ": " + c[i] + "; ";
//}
//DejaGnu.note("str" + str);
//check_equals(str, "-2147483650: still lower; 2.95147905179353e+20: much too high; 4294967296: too high; -3: minus three; 1.1: one point one; 3: three; 2: two; 1: one; 0: zero; ");

//// Getting 'holes' crawls the inheritance chain !
//Array.prototype[3] = 3;
//sparse = new Array();
//sparse[2] = 2;
//check_equals(sparse[3], 3); // crawl inheritance chain !
//sparse[4] = 4;
//check_equals(sparse[3], 3); // crawl inheritance chain !
//delete Array.prototype[3];

//c = []; c[''] = 2;
//check_equals(c.length, 0);
//check_equals(typeof(c['']), 'undefined');

//c = []; c[2.2] = 2;
//#if OUTPUT_VERSION < 7
  //xcheck_equals(c.length, 3);
//#else
  //check_equals(c.length, 0);
//#endif
//check_equals(c[2.2], 2);


    //-----------------------------------------------------
    // Test Array.pop()
    //-----------------------------------------------------
    DejaGnu.note("*** Testing Array.pop()");

    popped = b.pop();
    //check ( popped == 12 );
    if (popped == 12) {
        DejaGnu.pass("b.pop() == 12: correct");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.pop() != 12");
    }
    popped=b.pop();
    //check ( popped == "asdf" );
    if (popped == "asdf") {
        DejaGnu.pass("b.pop() == 'asdf: correct");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.pop() != 'asdf'");
    }
    popped=b.pop();
    //check ( popped == 551 );
    if (popped == 551) {
        DejaGnu.pass("b.pop() == 551: correct");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.pop() != 551");
    }
    //// make sure pops on an empty array don't cause problems
    popped=b.pop();
    //check ( popped == undefined );
    if (popped == null) {
        DejaGnu.pass("b.pop() == 'undefined': correct");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.pop() != 'undefined'");
    }
    b.pop(); b.pop();
    //check_equals ( b.length, 0 );
    if (b.length == 0) {
        DejaGnu.pass("b.length now equals 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.length != 0");
    }
    Reflect.callMethod(b, Reflect.field(b,"unshift"), [8,2]);
    Reflect.callMethod(b, Reflect.field(b,"push"), [4,3]);
    b.pop();
    b.shift();
    //check_equals ( b.toString() , "2,4" );
    if (b.toString() == "2,4") {
        DejaGnu.pass("b.toString == '2,4' after several operations");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.toString != '2,4' b contains incorrect values");
    }
    b.shift();
    b.pop();
    //check_equals ( b.toString() , "" );
    if (b.toString() == "") {
        DejaGnu.pass("b array is now empty as expected");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b array is not empty");
    }
    
    //-----------------------------------------------------
    // Test Array.shift()
    //-----------------------------------------------------
    DejaGnu.note("*** Testing Array.shift()");

    a = untyped __new__(Array, 1);
    //check_equals(a.length, 1);
    if (a.length == 1) {
        DejaGnu.pass("a.length == 1 correct");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 1");
    }
    var ret = a.shift();
    //check_equals(typeof(ret), 'undefined');
    if (ret == null) {
        DejaGnu.pass("shift on empty array returns 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift on empty array does not return 'undefined'");
    }
    //check_equals(a.length, 0);
    if (a.length == 0) {
        DejaGnu.pass("shift on empty array reduces length by 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift on empty array does not reduce length by 1");
    }
    a[1] = 'a';
    //check_equals(a.length, 2);
    if (a.length == 2) {
        DejaGnu.pass("a[1] = 'a' increases length to 2 on length 0 array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a[1] = 'a' does not correctly increase length to 2");
    }
    ret = a.shift();
    //check_equals(typeof(ret), 'undefined');
    if (ret == null) {
        DejaGnu.pass("shift on empty first index returns 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift on empty first index does not return 'undefined'");
    }
    //check_equals(a.length, 1);
    if (a.length == 1) {
        DejaGnu.pass("shift reduces length by 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift does not reduce length by 1");
    }
    ret = a.shift();
    //check_equals(typeof(ret), 'string');
    if (Std.is(ret, String)) {
        DejaGnu.pass("shift on string element returns correct type");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift on string element does not return correct type");
    }
    //check_equals(ret, 'a');
    if (ret == "a") {
        DejaGnu.pass("shift on single element array returns correct value");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift on single element array returns incorrect value");
    }
    //check_equals(a.length, 0);
    if (a.length == 0) {
        DejaGnu.pass("shift on single element array reduces length by 1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] shift on single element array does not reduce length by 1");
    }
    
    
    //------------------------------------------------------
    // Test Array.reverse
    //------------------------------------------------------
    DejaGnu.note("*** Testing Array.reverse");
    
    // check reverse for empty case
    b.reverse();
    //check_equals ( b.toString() , "" );
    if (b.toString() == "") {
        DejaGnu.pass("reverse on empty array returns empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] reverse on empty array does not return empty array");
    }

    // check reverse for sparse array
    var sparse = new Array();
    sparse[5] = 5;
    //count=0; for (var i in sparse) count++;
    //FIXED: needed to use a while loop instead of for...in loop
    var count=0;
    var i:Int = 0;
    while (i < sparse.length) {
        if( untyped sparse.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 1); // a single element exists
    if (count == 1) {
        DejaGnu.pass("Sparse array contains 1 element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Sparse array contains more or less than 1 element");
    }
    //check(!sparse.hasOwnProperty(0));
    if ( !(untyped sparse.hasOwnProperty(0))) {
        DejaGnu.pass("first element of sparse array is still empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] first element of sparse array is not empty");
    }
    //check(sparse.hasOwnProperty(5));
    if (untyped sparse.hasOwnProperty(5)) {
        DejaGnu.pass("5th element of sparse array has been initialized");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] 5th element of sparse array has not been initialized");
    }
    #if (flash6 || flash9)
    // flash9 seems to output sparse arrays in this way also
    // flash10 too?
    //check_equals(sparse.toString(), ",,,,,5");
    if (sparse.toString() == ",,,,,5") {
        DejaGnu.pass("sparse array contains correct values");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sparse array does not contain correct values");
    }
    #else
    //check_equals(sparse.toString(), "undefined,undefined,undefined,undefined,undefined,5");
    if (sparse.toString() == "undefined,undefined,undefined,undefined,undefined,5") {
         DejaGnu.pass("sparse array constains correct values");
     } else {
         DejaGnu.fail("[ln:"+here.lineNumber+"] sparse array does not contain correct values");
     } 
    #end
    sparse.reverse();
    //count=0; for (i in sparse) count++;
    //loop needs to be done in this way
    count=0;
    i = 0;
    while (i < sparse.length) {
        if( untyped sparse.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 6); // no more holes
    if (count == 6) {
        DejaGnu.pass("After reverse sparse array contains 6 elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] After reverse sparse array contains more or less than 6 elements");
    }
    //check(sparse.hasOwnProperty(0));
    if (untyped sparse.hasOwnProperty(0)) {
        DejaGnu.pass("After reverse sparse[0] contains a value");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] After reverse sparse[0] does not contain a value");
    }
    //check(sparse.hasOwnProperty(5));
    if (untyped sparse.hasOwnProperty(5)) {
        DejaGnu.pass("After reverse sparse[5] contains a value");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] After reverse sparse[5] does not contain a value");
    }
    #if (flash6 || flash9)
    //check_equals(sparse.toString(), "5,,,,,");
    if (sparse.toString() == "5,,,,,") {
        DejaGnu.pass("sparse array contains correct values");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sparse array does not contain correct values");
    }
    #else
    //check_equals(sparse.toString(), "5,undefined,undefined,undefined,undefined,undefined");
    if (sparse.toString() == 
        "5,undefined,undefined,undefined,undefined,undefined") {
         DejaGnu.pass("sparse array constains correct values");
     } else {
         DejaGnu.fail("[ln:"+here.lineNumber+"] sparse array does not contain correct values");
     } 
    #end
    
	
    //------------------------------------------------------
    // Test Array.join
    //------------------------------------------------------
    DejaGnu.note("*** Testing Array.join");
    // join a sparse array
    var j = new Array();
    j[1] = 1;
    j[3] = 3;
    var s = j.join("^");
    #if (flash6 || flash9)
    //~ check_equals(s, "^1^^3");
    if (s == "^1^^3") {
        DejaGnu.pass("array.join works correctly on sparse array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.join does not work correctly on sparse array");
    }
    #else
    //~ check_equals(s, "undefined^1^undefined^3");
    if (s == "undefined^1^undefined^3") {
        DejaGnu.pass("array.join works correctly on sparse array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.join does not work correctly on sparse array");
    }
    #end
    
    
    //------------------------------------------------------
    // Test Array.concat and Array.slice (TODO: split)
    //------------------------------------------------------
    DejaGnu.note("*** Testing Array.concat and Array.slice");
    // check concat, slice
    
    var bclone = new Array();
    bclone.concat(b);
    //check_equals ( bclone.length, 0 );
    if (bclone.length == 0) {
        DejaGnu.pass("concat on empty array returns empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat on empty array does not return empty array");
    }
    //check_equals ( b.length, 0 );
    if (b.length == 0) {
        DejaGnu.pass("b array is still empty: sanity checked");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b array is no longer empty");
    }
    var basic = new Array();
    basic = Reflect.callMethod(b, Reflect.field(b,"concat"), [0,1,2]);
    var concatted = new Array();
    concatted = Reflect.callMethod(basic, Reflect.field(basic, "concat"), 
                                   [3,4,5,6]);
    //check_equals ( concatted.join() , "0,1,2,3,4,5,6" );
    if (concatted.join(",") == "0,1,2,3,4,5,6") {
        DejaGnu.pass("concat works using direct input of values");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat does not work using direct input of values");
    }
    //check_equals ( concatted[4] , 4 );
    if (concatted[4] == 4) {
        DejaGnu.pass("concat places correct value at correct index");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat does not place correct value at correct index");
    }
    //check_equals ( basic.toString() , "0,1,2" );
    if (basic.toString() == "0,1,2") {
        DejaGnu.pass("basic array still contains correct values");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] basic array does not contain correct values");
    }
    var portion = concatted.slice( 2,-2 );
    //check_equals ( portion.toString() , "2,3,4" );
    if (portion.toString() == "2,3,4") {
        DejaGnu.pass("array.slice(2,-2) returns correct array values");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.slice(2, -2) does not return correct array values");
    }
    portion = portion.slice(1);
    //check_equals ( portion.toString() , "3,4" );
    if (portion.toString() == "3,4") {
        DejaGnu.pass("array.slice(1) returns the correct array vlaues");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.slice(1) does not return the correct array values");
    }
    portion = portion.slice(1, 2);
    //check_equals ( portion.toString() , "4" );
    if (portion.toString() == "4") {
        DejaGnu.pass("array.slice(1, 2) works correctly on 2 element array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.slice(1, 2) does not work correctly on 2 element array");
    }
    //check_equals ( portion.length, 1);
    if (portion.length == 1) {
        DejaGnu.pass("slice operations resulted in correct array length");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] slice operations did not result in correct array length");
    }
    portion = concatted.slice(-2, -1);
    //check_equals ( portion.toString(), "5");
    if (portion.toString() == "5") {
        DejaGnu.pass("array.slice(-2,-1) works correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.slice(-2, -1) does not work correctly");
    }
    portion = concatted.slice(-2);
    //check_equals ( portion.toString(), "5,6");
    if (portion.toString() == "5,6") {
        DejaGnu.pass("array.slice(-2) works correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array.slice(-2) does not work correctly");
    }
    var mixed = portion.concat([7,8,9]);
    //check_equals ( mixed.toString(), "5,6,7,8,9");
    if (mixed.toString() == "5,6,7,8,9") {
        DejaGnu.pass("concat after slice returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat after slice does not return correct array");
    }
    mixed = Reflect.callMethod(mixed, Reflect.field(mixed, "concat"),
                               [[10,11],12,[13]]);
    //check_equals ( mixed.toString(), "5,6,7,8,9,10,11,12,13");
    if (mixed.toString() == "5,6,7,8,9,10,11,12,13") {
        DejaGnu.pass("concat with arrays within array works");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat with arrays within array does not work");
    }

    // invalid calls
    portion = Reflect.callMethod(concatted, Reflect.field(concatted, "slice"),
                                 [0, -8]);
    //check_equals ( portion.toString(), "");
    if (portion.toString() == "") {
        DejaGnu.pass("invalid call to slice returns empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] invalid call to slice does not return empty array");
    }
    //portion = concatted.slice(-18);
    portion = Reflect.callMethod(concatted, Reflect.field(concatted, "slice"),
                                 [-18]);
    //check_equals ( portion.toString(), "0,1,2,3,4,5,6");
    if (portion.toString() == "0,1,2,3,4,5,6") {
        DejaGnu.pass("invalid call to slice returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] invalid call to slice does not return correct array");
    }
    //portion = concatted.slice(-18, 3);
    portion = Reflect.callMethod(concatted, Reflect.field(concatted, "slice"),
                                 [-18, 3]);
    //check_equals ( portion.toString(), "0,1,2");
    if (portion.toString() == "0,1,2") {
        DejaGnu.pass("invalid call to slice returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] invalid call to slice does not return correct array");
    }
    //portion = concatted.slice(18);
    portion = Reflect.callMethod(concatted, Reflect.field(concatted, "slice"),
                                 [18]);
    //check_equals ( portion.toString(), "");
    if (portion.toString() == "") {
        DejaGnu.pass("invalid call to slice returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] invalid call to slice does not return correct array");
    }
    
    // using objects that implement valueOf as index positions
    // FIXME: Currently returns an empty array. Is this a haxe problem or not?
    //        setting to xfail in the mean time. Not sure haxe knows how to deal
    //        with valueOf.
    //portion = concatted.slice(zero, two);
    portion = Reflect.callMethod(concatted, Reflect.field(concatted, "slice"),
                                 ['zero', 'two']);
    //check_equals ( portion.toString(), "0,1");
    //DejaGnu.note("portion = " + portion.toString());
    if (portion.toString() == "0,1") {
        DejaGnu.pass("invalid call to slice returns correct array");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] invalid call to slice does not return correct array");
    }

    concat_Test();
	
	splice_Test();
	
	constructorSingle_Test();
	
	sort_Test();
    
	sortOn_Test();
	
	enumeration_Test();
	
	lengthProperty_Test();
	
	propertyGetSet_Test();
	
	// The following two tests are internal to gnash
	popProtected_Test();
	
    popReadOnly_Test();
    
	
	// NOTE: Completed elswhere?
	// TODO: test ASnative-returned functions:
	//
	// ASnative(252, 1) - [Array.prototype] push
	// ASnative(252, 2) - [Array.prototype] pop
	// ASnative(252, 3) - [Array.prototype] concat
	// ASnative(252, 4) - [Array.prototype] shift
	// ASnative(252, 5) - [Array.prototype] unshift
	// ASnative(252, 6) - [Array.prototype] slice
	// ASnative(252, 7) - [Array.prototype] join
	// ASnative(252, 8) - [Array.prototype] splice
	// ASnative(252, 9) - [Array.prototype] toString
	// ASnative(252, 10) - [Array.prototype] sort
	// ASnative(252, 11) - [Array.prototype] reverse
	// ASnative(252, 12) - [Array.prototype] sortOn 
	//


    DejaGnu.done();
    }//end Main
	
	
	//------------------------------------------------------
    // Test Array.concat 
    //------------------------------------------------------
	static public function concat_Test() {
    DejaGnu.note("*** Testing Array.concat");

    var sparse1 = new Array();
    sparse1[3] = 'a3';

    var sparse2 = new Array();
    sparse2[2] = 'b2';

    //csp = sparse1.concat(sparse2);
    var csp = sparse1.concat(sparse2);

    //count=0; for (var i in sparse1) count++;
    var count = 0;
    var i = 0;
    while (i < sparse1.length) {
        if( untyped sparse1.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 1);
    if (count == 1) {
        DejaGnu.pass("sparse1 contains only 1 element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sparse1 contains more or less than 1 element");
    }
    
    
    //count=0; for (var i in sparse2) count++;
    count=0;
    i = 0;
    while (i < sparse2.length) {
        if( untyped sparse2.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 1);
    if (count == 1) {
        DejaGnu.pass("sparse2 contains only 1 element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sparse2 contains more or less than 1 element");
    }

    //count=0; for (var i in csp) count++;
    count = 0;
    i = 0;
    while (i < csp.length) {
        if( untyped csp.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    
    //check_equals(count, 7); // concat filled any holes
    if (count == 7) {
        DejaGnu.pass("concat filled holes in csp");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat did not fill holes in csp");
    }

    csp = sparse1.concat(untyped 'onemore');
    //count=0; for (var i in csp) count++;
    count = 0;
    i = 0;
    while (i < csp.length) {
        if( untyped csp.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 5); // concat filled any holes
    if (count == 5) {
        DejaGnu.pass("concat filled holes in sparse1");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] concat did not fill holes in sparse1");
    }
    
	}//end concat_Test
	
	
	//-------------------------------
    // Test Array.splice
    //-------------------------------
	static public function splice_Test() {
    DejaGnu.note("***  Begin testing Array.splice");

    var ary = [0,1,2,3,4,5];
    //check_equals ( ary.toString(), "0,1,2,3,4,5" );
    if (ary.toString() == "0,1,2,3,4,5") {
        DejaGnu.pass("ary constructed properly: sanity checked");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary not constructed proplerly");
    }

    // No args is invalid
    //spliced = ary.splice();
    var spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), []);
    //check_equals ( ary.toString(), "0,1,2,3,4,5" );
    if (ary.toString() == "0,1,2,3,4,5") {
        DejaGnu.pass("splice() with no args returns same array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice() with no args does not return same array");
    }
    //check_equals ( typeof(spliced), "undefined" );
    if (untyped __typeof__(spliced) == "undefined") {
        DejaGnu.pass("splice() returns undefined");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice() does not return undefined");
    }

    // Zero and positive offset starts from the end (-1 is last)
    var spliced = ary.splice(0, 1);
    //check_equals ( ary.toString(), "1,2,3,4,5" );
    if (ary.toString() == "1,2,3,4,5") {
        DejaGnu.pass("ary[0] element removed");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary[0] element was not remeoved");
    }
    //check_equals ( spliced.toString(), "0" );
    if (spliced.toString() == "0") {
        DejaGnu.pass("'0' element moved into spliced");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] '0' element was not moved into spliced");
    }
    spliced = ary.splice(1, 1);
    //check_equals ( ary.toString(), "1,3,4,5" );
    if (ary.toString() == "1,3,4,5") {
        DejaGnu.pass("ary[1] element removed");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary[1] element was not removed");
    }
    //check_equals ( spliced.toString(), "2" );
    if (spliced.toString() == "2") {
        DejaGnu.pass("'2' element moved into spliced");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] '2' element was not moved into spliced");
    }

    //// Negative offset starts from the end (-1 is last)
    spliced = ary.splice(-1, 1);
    //check_equals ( ary.toString(), "1,3,4" );
    if (ary.toString() == "1,3,4") {
        DejaGnu.pass("ary[3] element removed from end");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary[3] element not removed from end");
    }
    //check_equals ( spliced.toString(), "5" );
    if (spliced.toString() == "5") {
        DejaGnu.pass("'5' element moved into spliced");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] '5' element was not moved into spliced");
    }
    spliced = ary.splice(-2, 1);
    //check_equals ( ary.toString(), "1,4" );
    if (ary.toString() == "1,4") {
        DejaGnu.pass("ary[1] removed with negative argument");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary[1] was not removed with negative argument");
    }
    //check_equals ( spliced.toString(), "3" );
    if (spliced.toString() == "3") {
        DejaGnu.pass("'3' correctly moved into spliced");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] '3' was not correctly moved into spliced");
    }

    // Out-of bound zero or positive offset are taken as one-past the end
    spliced = ary.splice(2, 1);
    //check_equals ( ary.toString(), "1,4" );
    if (ary.toString() == "1,4") {
        DejaGnu.pass("out of bounds index did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] out of bounds index changed ary");
    }
    //check_equals ( spliced.toString(), "" );
    if (spliced.toString() == "") {
        DejaGnu.pass("out of bounds index produced empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] out of bounds index did not produce empty array");
    }
    spliced = ary.splice(2, 10);
    //check_equals ( ary.toString(), "1,4" );
    if (ary.toString() == "1,4") {
        DejaGnu.pass("out of bounds index did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] out of bounds index changed ary");
    }
    //check_equals ( spliced.toString(), "" );
    if (spliced.toString() == "") {
        DejaGnu.pass("out of bounds index produced empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] out of bounds index did not produce empty array");
    }

    // Out-of bound negative offset are taken as zero
    spliced = ary.splice(-20, 1);
    //check_equals ( ary.toString(), "4" );
    if (ary.toString() == "4") {
        DejaGnu.pass("out of bounds negative index taken as 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] out of bounds negative index not taken as 0");
    }
    //check_equals ( spliced.toString(), "1" );
    if (spliced.toString() == "1") {
        DejaGnu.pass("out of bounds negative index removed correct element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] out of bounds negative index did not remove correct element");
    }

    // rebuild the array
    ary = [0,1,2,3,4,5,6,7,8];

    // Zero length doesn't change anything, and return an empty array
    spliced = ary.splice(2, 0);
    //check_equals ( ary.toString(), "0,1,2,3,4,5,6,7,8" );
    if (ary.toString() == "0,1,2,3,4,5,6,7,8") {
        DejaGnu.pass("0 length arg did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] 0 length arg changed ary");
    }
    //check_equals ( spliced.toString(), "" );
    if (spliced.toString() == "") {
        DejaGnu.pass("0 length arg returned empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] 0 length arg did not return empty array");
    }

    // Out of bound positive length consumes up to the end
    spliced = ary.splice(2, 100);
    //check_equals ( ary.toString(), "0,1" );
    if (ary.toString() == "0,1") {
        DejaGnu.pass("too long positive length removed to end of ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] too long positive length did not remove to end");
    }
    //check_equals ( spliced.toString(), "2,3,4,5,6,7,8" );
    if (spliced.toString() == "2,3,4,5,6,7,8") {
        DejaGnu.pass("splice contains ary from [2] to the end");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice does not contain ary from [2] to the end");
    }
    ary=spliced; // reset array
    spliced = ary.splice(-2, 100);
    //check_equals ( ary.toString(), "2,3,4,5,6" );
    if (ary.toString() == "2,3,4,5,6") {
        DejaGnu.pass("neg index large length removes end of ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] neg index large length does not remove end of ary");
    }
    //check_equals ( spliced.toString(), "7,8" );
    if (spliced.toString() == "7,8") {
        DejaGnu.pass("spliced now contains end of ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] spliced does not contain correct end of ary");
    }

    // Negative length are invalid
    var spliced = ary.splice(0, -1);
    //check_equals ( typeof(spliced), 'undefined' );
#if !flash9
    if (untyped __typeof__(spliced) == "undefined") {
        DejaGnu.pass("splice(0,-1) returns undefined");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(0,-1) does not return undefined");
    }
#else
    DejaGnu.unresolved("It is possible that for the following splice tests for"+
            " flash9, flash is in fact returning an empty array, but it is"+
            " also possible that this is something HaXe is doing on its own");
    if ( Std.is(spliced, Array) && spliced.toString() == "" 
         && spliced.length == 0) {
        DejaGnu.pass("splice(0,-1) (negative length) returned empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"]splice(0,-1), (negative length)"+
                     " did not return empty array; spliced = " + spliced);
    }
#end
    //check_equals ( ary.toString(), "2,3,4,5,6" );
    if (ary.toString() == "2,3,4,5,6") {
        DejaGnu.pass("negative length did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] negative length changed ary");
    }
    
    var spliced = ary.splice(3, -1);
#if !flash9
    //check_equals ( typeof(spliced), 'undefined' );
    if (untyped __typeof__(spliced) == "undefined") {
        DejaGnu.pass("splice(-3,-1,) returns undefined");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(-3,-1,) does not return undefined");
    }
#else
    if ( Std.is(spliced, Array) && spliced.toString() == "" 
         && spliced.length == 0) {
        DejaGnu.pass("splice(3,-1) (negative length) returned empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"]splice(3,-1), (negative length)"+
                     " did not return empty array; spliced = " + spliced);
    }
#end
    //check_equals ( ary.toString(), "2,3,4,5,6" );
    if (ary.toString() == "2,3,4,5,6") {
        DejaGnu.pass("negative length did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] negative length changed ary");
    }
    
    var spliced = ary.splice(-1, -1);
#if !flash9
    //check_equals ( typeof(spliced), 'undefined' );
    if (Std.string(untyped __typeof__(spliced)) == "undefined") {
        DejaGnu.pass("splice(-1,-1) returns undefined");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(-1,-1) does not return undefined");
    }
#else
    if ( Std.is(spliced, Array) && spliced.toString() == "" 
         && spliced.length == 0) {
        DejaGnu.pass("splice(-1,-1) (negative length) returned empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"]splice(-1,-1), (negative length)"+
                     " did not return empty array; spliced = " + spliced);
    }
#end
    //check_equals ( ary.toString(), "2,3,4,5,6" );
    if (ary.toString() == "2,3,4,5,6") {
        DejaGnu.pass("negative length did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] negative length changed ary");
    }
    
    //spliced = ary.splice(-1, -1, "a", "b", "c");
    var spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"),
                                  [-1,-1,"a","b","c"]);
#if !flash9
    //check_equals ( typeof(spliced), 'undefined' );
    if (Std.string(untyped __typeof__(spliced)) == "undefined") {
        DejaGnu.pass("splice(-1,-1,'a','b','c') returns undefined");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(-1,-1,'a','b','c') does not return undefined");
    }
#else
    if ( Std.is(spliced, Array) && spliced.toString() == "" 
         && spliced.length == 0) {
        DejaGnu.pass("splice(-1,-1) negative length returned empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"]splice(-1,-1), negative length"+
                     " did not return empty array; spliced = " + spliced);
    }
#end

#if !flash9
    //check_equals ( ary.toString(), "2,3,4,5,6" );
    if (ary.toString() == "2,3,4,5,6") {
        DejaGnu.pass("negative length did not change ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] negative length changed ary");
    }
#else
    //NOTE: flash9 seems to insert elements into the array even thought the call
    //  to splice was invalid. Is this a flash9 behavior or a HaXe problem?
    if (ary.toString() == "2,3,4,5,a,b,c,6") {
        DejaGnu.pass("In flash9, invalid call to splice(-1,-1,'a','b','c')"+
                     " inserts elements into ary anyway");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"]ary changed. Invalid call to"+
                     " splice(-1,-1,'a','b','c') did not insert elements into"+
                     " the array");
    }
#end
    
    // NOTE: resetting ary because flash 9 makes changes here which invalidate 
    //       further tests.
    ary = new Array();
    ary = [2,3,4,5,6];

    // Provide substitutions now
    //spliced = ary.splice(1, 1, "a", "b", "c");
    var spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"),
                                  [1,1,"a","b","c"]);
    //check_equals ( ary.toString(), "2,a,b,c,4,5,6" );
    if (ary.toString() == "2,a,b,c,4,5,6") {
        DejaGnu.pass("splice with subst. changed ary correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice with subst. did not change ary correctly");
    }
    //check_equals ( spliced.toString(), '3' );
    if (spliced.toString() == "3") {
        DejaGnu.pass("splice with subst. removed correct element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice with subst. did not remove correct element");
    }
    //spliced = ary.splice(-4, 2, 8);
    spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), [-4,2,8]);
    //check_equals ( ary.toString(), "2,a,b,8,5,6" );
    if (ary.toString() == "2,a,b,8,5,6") {
        DejaGnu.pass("splice with subst. changed ary correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice with subst. did not change ary correctly");
    }
    //check_equals ( spliced.toString(), 'c,4' );
    if ( spliced.toString() == "c,4" ) {
        DejaGnu.pass("splice with subst. removed correct elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice with subst. removed correct elements");
    }

    // Insert w/out deleting anything
    //spliced = ary.splice(3, 0, 10, 11, 12);
    spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), 
                                 [3, 0, 10, 11, 12]);
    //check_equals ( ary.toString(), "2,a,b,10,11,12,8,5,6" );
    if (ary.toString() == "2,a,b,10,11,12,8,5,6") {
        DejaGnu.pass("splice correctly inserted elements into ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice did not correctly insert elements into ary");
    }
    //check_equals ( spliced.toString(), '' );
    if (spliced.toString() == "") {
        DejaGnu.pass("splice did not remove any elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice removed elements when it shouldn't");
    }

    // Use arrays as replacement
    //spliced = ary.splice(0, 7, [1,2], [3,4]);
    spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"),
                                 [0, 7, [1,2], [3,4]]);
    //check_equals ( ary.toString(), "1,2,3,4,5,6" );
    if (ary.toString() == "1,2,3,4,5,6") {
        DejaGnu.pass("array arguments passed as subst. correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array arguments not passed correctly");
    }
    //check_equals ( ary.length, 4 ); // don't be fooled by toString output !
    if (ary.length == 4) {
        DejaGnu.pass("ary.length == 4 because of internal arrays");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary.length != 4");
    }
    //check_equals ( spliced.toString(), '2,a,b,10,11,12,8' );
    if (spliced.toString() == "2,a,b,10,11,12,8") {
        DejaGnu.pass("splice removed correct elements from ary");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice did not remove correct elements from ary");
    }

    // Ensure the simplest usage cases are correct!
    //spliced = ary.splice(1);
    spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), [1]);
    //check_equals ( spliced.toString(), "3,4,5,6");
    if (spliced.toString() == "3,4,5,6") {
        DejaGnu.pass("splice(1) returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(1) does not return correct array");
    }
    //spliced = ary.splice(0);
    spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), [0]);
    //check_equals ( spliced.toString(), "1,2");
    if (spliced.toString() == "1,2") {
        DejaGnu.pass("splice(0) returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(0) does not return correct arry");
    }

    // Splice a sparse array
    ary = new Array(); ary[2] = 2; ary[7] = 7;

    //check_equals(ary.length, 8);
    if (ary.length == 8) {
        DejaGnu.pass("sparse array has correct length");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sparse array does not have correct length");
    }
    //count=0; for (var i in ary) count++;
    var count = 0;
    var i = 0;
    while (i < ary.length) {
        if( untyped ary.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 2);
    if (count == 2) {
        DejaGnu.pass("sparse array has only 2 elements");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sparse array has more or less than 2 elements");
    }

    var spliced = ary.splice(3, 0); // no op ?
    //check_equals(ary.length, 8); // no change in length
    if (ary.length == 8) {
        DejaGnu.pass("splice(3,0) does not change length of sparse array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice(3,0) changed length of sparse array");
    }
    //count=0; for (var i in ary) count++;
    count = 0;
    i = 0;
    while (i < ary.length) {
        if( untyped ary.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 8); // but fills the gaps !
    //NOTE: is this correct behavior? works differently in flash 9
    if (count == 8) {
        DejaGnu.pass("splice fills holes in sparse array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice does not fill holes in"+
                     " sparse array; count = "+ count);
    }

    ary = new Array(); ary[2] = 2; ary[7] = 7;
    //spliced = ary.splice(3, 0, 3); // add 3 at index 3
    spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), [3,0,3]);
    //check_equals(ary.length, 9);
    if (ary.length == 9) {
        DejaGnu.pass("splice inserted element into sparse array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice did not insert element into sparse array");
    }
    //count=0; for (var i in ary) count++;
    count = 0;
    i = 0;
    while (i < ary.length) {
        if( untyped ary.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 9); // fills the gaps !
    //correct or not?
    if (count == 9) {
        DejaGnu.pass("splice fills holes in sparse array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice does not fill holes in"+
                     " sparse array; count = "+ count);
    }
    //check_equals(ary[3], 3);
    if (ary[3] == 3) {
        DejaGnu.pass("ary[3] == 3");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary[3] != 3");
    }
    //check_equals(ary[2], 2);
    if (ary[3] == 3) {
        DejaGnu.pass("ary[3] == 3");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] ary[3] != 3");
    }

    ary = new Array(); ary[2] = 2; ary[7] = 7;
    //spliced = ary.splice(3, 1, 3); // replace index 3 (an hole) with a 3 value
    var spliced = Reflect.callMethod( ary, Reflect.field(ary, "splice"), [3,1,3]);
    //count=0; for (var i in ary) count++;
    count = 0;
    i = 0;
    while (i < ary.length) {
        if( untyped ary.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 8); // fills the gaps
    if (count == 8) {
        DejaGnu.pass("empty ary indexes have been filled");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] empty ary indexes have not been filled");
    }
    //count=0; for (var i in spliced) count++;
    count = 0;
    i = 0;
    while (i < spliced.length) {
        if( untyped spliced.hasOwnProperty(i) ) {
            count++;
        }
        i++;
    }
    //check_equals(count, 1); // the returned array contains an actual value, not an hole
    if (count == 1) {
        DejaGnu.pass("splice on empty index returned a value");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] splice on empty index did not return a value");
    }
    
	}//end splice_Test
	
	
	//-------------------------------
    // Test single parameter constructor, and implicitly expanding array
    //-------------------------------
	static public function constructorSingle_Test() {
    DejaGnu.note("*** Begin testing single parameter constructor");

    //var c = new Array(10);
    var c = untyped __new__(Array, 10);
    //check_equals(c.constructor, Array);
    if (untyped c.constructor == Array) {
        DejaGnu.pass("array c has an Array constructor property");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] array c does not have a constructor property");
    }
    //check (a instanceOf Array);
    // I'm pretty sure this is supposed to be check (c instanceOf Array)
    if (Type.getClassName(Type.getClass(c)) == "Array") {
        DejaGnu.pass("c is an Array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c is not an Array");
    }
    //check_equals ( typeof(c), "object" );
    if (Std.string(untyped __typeof__(c)) == "object") {
        DejaGnu.pass("c is an object");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c is not an object");
    }
    //check_equals ( c.length, 10 );
    if (c.length == 10) {
        DejaGnu.pass("c.length == 10");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c.length != 10");
    }
    //check_equals ( c[5] , undefined );
    if (untyped c[5] == null) {
        DejaGnu.pass("c[5] is currently 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[5] is not 'undefined'");
    }
    untyped c[1000] = 283;
    //check_equals ( c[1000] , 283 );
    if (untyped c[1000] == 283) {
        DejaGnu.pass("c[1000] correctly assigned 283");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1000] not correctly assigned 283");
    }
    //check_equals ( c[1001] , undefined );
    if (untyped c[1001] == null) {
        DejaGnu.pass("c[1001] == 'undefined' one beyond array end");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1001] != 'undefined' one beyond array end");
    }
    //check_equals ( c[999] , undefined );
    if (untyped c[999] == null) {
        DejaGnu.pass("c[999] == 'undefined' still");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[999] != 'undefined'");
    }
    //check_equals ( c.length, 1001 );
    if (c.length == 1001) {
        DejaGnu.pass("length of c correctly extended to 1001");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] length of c not correctly extended");
    }

    // Test that the 'length' property is overridable
    untyped c[8] = 'eight';
    untyped c[0] = 'zero';
    //check_equals(c[8], 'eight');
    if (untyped c[8] == "eight") {
        DejaGnu.pass("c[8] == 'eight'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[8] != 'eight'");
    }
    //c.length = 2;
    Reflect.setField(c, "length", 2);
    //check_equals(c.length, 2);
    if (c.length == 2) {
        DejaGnu.pass("length of c reset to 2");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] length of c not reset to 2");
    }
    //check_equals(c[8], undefined);
    if (untyped c[8] == null) {
        DejaGnu.pass("c[8] changed to 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[8] not correctly changed to 'undefined'");
    }
    //check_equals(c[0], 'zero');
    if (untyped c[0] == "zero") {
        DejaGnu.pass("c[0] == 'zero' still");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0] != 'zero' anymore");
    }
    
    // Probable flash9 bug in gflashplayer here or Haxe bug, not sure which
    //c.length = -1;
    Reflect.setField(c, "length", -1);
    // it seems Gnash needs to store the 'length' property as a normal property
    //xcheck_equals(c.length, -1);
    if (c.length == -1) {
        DejaGnu.xpass("c.length now == -1");
    } else {
        DejaGnu.xfail("[ln:"+here.lineNumber+"] c.length != -1");
    }
    //check_equals(c[0], undefined);
    if (untyped c[0] == null) {
        DejaGnu.pass("c[0] == 'undefined'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0] != 'undefined'");
    }
    
	}//end constructorSingle_Test
	
	
	//-------------------------------
    // Test deleting an array element
    //-------------------------------
	static public function deletElement_Test() {
    //var c = new Array(10,20,30);
    //var c = untyped __new__(Array, 10, 20, 30);
    var c = [10,20,30];
    //check_equals ( c.length, 3 );
    if (c.length == 3) {
        DejaGnu.pass("c constructed with correct length");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c not constructed with correct length");
    }
    //check_equals(c[0], 10);
    if (untyped c[0] == 10) {
        DejaGnu.pass("c[0] == 10");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[0] != 10");
    }
    //check_equals(c[1], 20);
    if (untyped c[1] == 20) {
        DejaGnu.pass("c[1] == 20");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[1] != 20");
    }
    //check_equals(c[2], 30);
    if (untyped c[2] == 30) {
        DejaGnu.pass("c[2] == 30");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c[2] != 30");
    }
    
    //check(c.hasOwnProperty('0'));
    if (untyped c.hasOwnProperty('0')) {
        DejaGnu.pass("c has property '0'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c does not have property '0'");
    }
    //check(c.hasOwnProperty('1'));
    if (untyped c.hasOwnProperty('1')) {
        DejaGnu.pass("c has property '1'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c does not have property '1'");
    }
    //check(c.hasOwnProperty('2'));
    if (untyped c.hasOwnProperty('2')) {
        DejaGnu.pass("c has property '2'");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] c does not have property '2'");
    }
    
    // NOTE: the following tests do not seem to have an equivalent haxe test.
    //       Is the delete keyword specific to ming?
    //check(delete c[1]);
    //check_equals ( c.length, 3 );
    //check_equals(c[0], 10);
    //check_equals(typeof(c[1]), 'undefined');
    //check_equals(c[2], 30);
    //#if OUTPUT_VERSION > 5
    //check(c.hasOwnProperty('0'));
    //check(!c.hasOwnProperty('1'));
    //check(c.hasOwnProperty('2'));
    //#endif

    //c[10] = 'ten';
    //check_equals(c.length, 11);
    //ASSetPropFlags(c, "2", 7, 0); // protect from deletion
    //xcheck( ! delete c[2] ); // gnash doesn't store prop flags here..
    //xcheck_equals(c[2], 30); // so won't respect delete-protection
    //c.length = 2;
    //xcheck_equals(c[2], 30); // was protected !
    //check_equals(typeof(c[10]), 'undefined'); // was not protected..
    //c.length = 11;
    //check_equals(typeof(c[10]), 'undefined'); // and won't come back
	
	}//end deleteElement_Test


	//-------------------------------
    // Test sort
    //-------------------------------
	static public function sort_Test() {
    DejaGnu.note("*** Begin Array sort testing");
    
    //NOTE: Gnash seems to allow these types of objects to be used as comparison
    //      functions. However, gflashplayer, flash9 do not allow this. I will
    //      write some equivalent functions so that these test can be run in 
    //      flash9.
    //used later
    //neg = new Object();
    //neg.valueOf = function () { return -1; };
    var neg = { valueOf : function() {return -1;} };
    var fneg = function(x,y) {return -1;};
    //zero = new Object();
    //zero.valueOf = function () { return 0; };
    var zero = { valueOf : function() {return 0;} };
    var fzero = function(x,y) {return 0;};
    //pos = new Object();
    //pos.valueOf = function () { return 1; };
    var pos = { valueOf : function() {return 1;} };
    var fpos = function(x,y) {return 1;};
    //two = new Object();
    //two.valueOf = function () { return 2; };
    var two = { valueOf : function() {return 2;} };
    var ftwo = function(x,y) {return 2;};
    //numeric = new Object();
    //numeric.valueOf = function () { return Array.NUMERIC; };
    var numeric = { valueOf : function() {return untyped Array.NUMERIC;} };
    var fnumeric = function(x,y) {return untyped Array.NUMERIC;};
    //numericRev = new Object();
    //numericRev.valueOf = function () { return (Array.NUMERIC | Array.DESCENDING); };
    var numericRev = { valueOf : function() {return (untyped Array.NUMERIC |
                      untyped Array.DESCENDING);} };
    var fnumericRev = function(x,y) {return (untyped Array.NUMERIC |
                                     untyped Array.DESCENDING);};

    
    var cmp_fn = function(x,y) {
        if (x.length < y.length) { return -1; }
        if (x.length > y.length) { return 1; }
        return 0;
    };

    var cmp_fn_obj =  function(x,y) {
        if (x.length < y.length) { return neg; }
        if (x.length > y.length) { return pos; }
        return zero;
    };

    var tolen = function(x) {
        var i = 0;
        var str = "[";
        while( i < x.length ) {
            str += untyped x[i].length;
            if ( i != x.length - 1) str += ", ";
            i++;
        }
        str += "]";
        return str;
    };
    

    //id = new Object();
    //id.toString = function () { return "Name"; };
    var id = { toString : function() {return "Name";} };
    //yr = new Object();
    //yr.toString = function () { return "Year"; };
    var yr = { toString : function() {return "Year";} };

    var a = ["ed", "emacs", "", "vi", "nano", "Jedit"];
    var b = [8, 1, -2, 5, -7, -9, 3, 0];
    var c = [7.2, 2.0, -0.5, 3/0, 0.0, 8.35, 0.001, -3.7];
    var d = [];
    var e = ["singleton"];
    var f = [id, yr, id];

    //trace(" -- Basic Sort Tests -- ");
    DejaGnu.note("** Basic sort tests");
    
    
    //a.sort();
    Reflect.callMethod( a, Reflect.field(a, "sort"), []);
    //check_equals( a.toString(), ",Jedit,ed,emacs,nano,vi" );
    if (a.toString() == ",Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("a.sort() correctly sorts a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] a.sorty() does not correctly sort a");
    }
    //NOTE: flash9/gflashplayer does not allow NUMERIC sorting of strings
    #if !flash9
    //r = a.sort( Array.NUMERIC );
    var r = Reflect.callMethod( a, Reflect.field(a, "sort"),
                               [untyped Array.NUMERIC]);
    //check_equals( r.toString(), ",Jedit,ed,emacs,nano,vi" );
    if (r.toString() == ",Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("Numeric sort on a returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Numeric sort on a does not return correct array");
    }
    //check_equals( a.toString(), ",Jedit,ed,emacs,nano,vi" );
    if (a.toString() == ",Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("Numeric sort on a correctly sorts a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Numeric sort on a does not correctly sort a");
    }
    //a.sort( Array.NUMERIC | Array.CASEINSENSITIVE );
    Reflect.callMethod( a, Reflect.field(a, "sort"), [untyped Array.NUMERIC |
                       untyped Array.CASEINSENSITIVE]);
    //check_equals( a.toString(), ",ed,emacs,Jedit,nano,vi" );
    if (a.toString() == ",ed,emacs,Jedit,nano,vi") {
        DejaGnu.pass("Numeric | Caseinsensitive sort on a works correclty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Numeric | Caseinsensitive sort on a does not work");
    }
    #end
	

    //a.sort( Array.CASEINSENSITIVE );
    Reflect.callMethod( a, Reflect.field(a, "sort"), 
                       [untyped Array.CASEINSENSITIVE]);
    //check_equals( a.toString(), ",ed,emacs,Jedit,nano,vi" );
    if (a.toString() == ",ed,emacs,Jedit,nano,vi") {
        DejaGnu.pass("Caseinsensitive sort on a works correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Caseinsensitive sort on a does not work");
    }
    //a.sort( Array.UNIQUESORT );
    Reflect.callMethod( a, Reflect.field(a, "sort"), 
                       [untyped Array.UNIQUESORT]);
    //check_equals( a.toString(), ",Jedit,ed,emacs,nano,vi" );
    if (a.toString() == ",Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("Uniquesort sort on a works correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Uniquesort sort on a does not work");
    }
    //r = a.sort( Array.DESCENDING );
    var r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.DESCENDING]);
    //check_equals( r.toString(), "vi,nano,emacs,ed,Jedit," );
    if (r.toString() == "vi,nano,emacs,ed,Jedit,") {
        DejaGnu.pass("Descending sort on a returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Descending sort on a does not return correct array");
    }
    //check_equals( a.toString(), "vi,nano,emacs,ed,Jedit," );
    if (a.toString() == "vi,nano,emacs,ed,Jedit,") {
        DejaGnu.pass("Descending sort on a works correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] Descending sort on a does not work");
    }

    //r = b.sort();
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), []);
    //check_equals( r.toString(), "-2,-7,-9,0,1,3,5,8" );
    if (r.toString() == "-2,-7,-9,0,1,3,5,8") {
        DejaGnu.pass("b.sort() returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.sort() does not return correct array");
    }
    //check_equals( b.toString(), "-2,-7,-9,0,1,3,5,8" );
    if (b.toString() == "-2,-7,-9,0,1,3,5,8") {
        DejaGnu.pass("b.sort() correctly sorted b");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] b.sort() did not correctly sort b");
    }
    //b.sort( Array.NUMERIC );
    Reflect.callMethod( b, Reflect.field(b, "sort"), [untyped Array.NUMERIC]);
    //check_equals( b.toString(), "-9,-7,-2,0,1,3,5,8" );
    if (b.toString() == "-9,-7,-2,0,1,3,5,8") {
        DejaGnu.pass("NUMERIC sort on b correctly sorted b");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] NUMERIC sort on b did not work");
    }
    //b.sort( Array.UNIQUESORT );
    Reflect.callMethod( b, Reflect.field(b, "sort"), 
                       [untyped Array.UNIQUESORT]);
    //check_equals( b.toString(), "-2,-7,-9,0,1,3,5,8" );
    if (b.toString() == "-2,-7,-9,0,1,3,5,8") {
        DejaGnu.pass("UNIQUESORT on b correctly sorted b");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNIQUESORT on b did not work");
    }
    //b.sort( Array.DESCENDING );
    Reflect.callMethod( b, Reflect.field(b, "sort"), 
                       [untyped Array.DESCENDING]);
    //check_equals( b.toString(), "8,5,3,1,0,-9,-7,-2" );
    if (b.toString() == "8,5,3,1,0,-9,-7,-2") {
        DejaGnu.pass("DESCENDING sort on b correctly sorted b");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] DESCENDING sort on b did not work");
    }
    //r = b.sort( Array.DESCENDING | Array.NUMERIC );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.DESCENDING | untyped Array.NUMERIC]);
    //check_equals( r.toString(), "8,5,3,1,0,-2,-7,-9" );
    if (r.toString() == "8,5,3,1,0,-2,-7,-9") {
        DejaGnu.pass("DESCENDING | NUMERIC sort on b returned correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] DESCENDING | NUMERIC sort on b did not return correctly");
    }
    //check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
    if (b.toString() == "8,5,3,1,0,-2,-7,-9") {
        DejaGnu.pass("DESCENDING | NUMERIC sort correctly sorted b");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] DESCENDING | NUMERIC sort on b did not work");
    }
    
    //NOTE: These may not be doing anything in Gnash. It could be that these
    //      test cases were written so that they pass and not written for
    //      correctness. It seems that the intention here is to pass a custom
    //      sort function to array, but what's being passed are objects with
    //      function properties.
    //      It also could be that these tests are just checking to make sure an 
    //      object cannot be accidentally interpreted as a function.
    
    //r = b.sort( zero );
    #if !flash9
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), [zero]);
    #else
    // gflashplyaer crashes if the arguement to sort is an object and not a 
    // function. These tests may need to be changed once gnash is capable of
    // running flash9
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), [fzero]);
    #end
    //check_equals( r.toString(), "8,5,3,1,0,-2,-7,-9" );
    if (r.toString() == "8,5,3,1,0,-2,-7,-9") {
        DejaGnu.pass("custom sort 'zero' returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] custom sort 'zero' does not"+
                     " return correctly; r = "+ r.toString());
    }
    //check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
    if (b.toString() == "8,5,3,1,0,-2,-7,-9") {
        DejaGnu.pass("custom sort 'zero' sorted b correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] custom sort 'zero' did not sort"+
                     " b correctly; b = "+ b.toString());
    }
    //b.sort( numeric );
    #if !flash9
    Reflect.callMethod( b, Reflect.field(b, "sort"), [numeric]);
    #else
    Reflect.callMethod( b, Reflect.field(b, "sort"), [fnumeric]);
    #end
    //check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
    if (b.toString() == "8,5,3,1,0,-2,-7,-9") {
        DejaGnu.pass("custom sort 'numeric' sorted b correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] custom sort 'numeric' did not"+
                     " sort b correctly; b = "+ b.toString());
    }
    //b.sort( numericRev );
    #if !flash9
    Reflect.callMethod( b, Reflect.field(b, "sort"), [numericRev]);
    #else
    Reflect.callMethod( b, Reflect.field(b, "sort"), [fnumericRev]);
    #end
    //check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
    if (b.toString() == "8,5,3,1,0,-2,-7,-9") {
        DejaGnu.pass("custom sort 'numericRev' sorted b correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] custom sort 'numericRev' did"+
                     " not sort b correctly; b = "+ b.toString());
    }
    
    // resetting order of b so that it is in the expected order for the
    // RETURNINDEXEDARRAY tests. Might be able to remove this once gnash can run
    // the as3 tests
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.DESCENDING | untyped Array.NUMERIC]);
    

    //r = c.sort();
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), []);
    //check_equals( r.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
    if (r.toString() == "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity") {
        DejaGnu.pass("sort() on c returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on c does not return correct array");
    }
    //check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
    if (c.toString() == "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity") {
        DejaGnu.pass("sort() on c correctly sorted c");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on c did not correctly sort c");
    }
    //c.sort( Array.CASEINSENSITIVE );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.CASEINSENSITIVE]);
    //check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
    if (c.toString() == "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity") {
        DejaGnu.pass("sort(CASEINSENSITIVE) on c correctly sorted c");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(CASEINSENSITIVE) on c did not correctly sort c");
    }
    //c.sort( Array.NUMERIC );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.NUMERIC]);
    //check_equals( c.toString(), "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity" );
    if (c.toString() == "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity") {
        DejaGnu.pass("sort(NUMERIC) on c correctly sorted c");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(NUMERIC) on c did not correctly sort c");
    }
    //r = c.sort( Array.UNIQUESORT );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.UNIQUESORT]);
    //check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
    if (c.toString() == "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity") {
        DejaGnu.pass("sort(UNIQUESORT) on c correctly sorted c");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(UNIQUESORT) on c did not correctly sort c");
    }
    //r = c.sort( Array.DESCENDING | Array.NUMERIC );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.DESCENDING | untyped Array.NUMERIC]);
    //check_equals( c.toString(), "Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
    if (c.toString() == "Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7") {
        DejaGnu.pass("sort(DESCENDING | NUMERIC) on c works");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(DESCENDING | NUMERIC) on c does not work");
    }

    //r = d.sort();
    r = Reflect.callMethod( d, Reflect.field(d, "sort"), []);
    //check_equals( r.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("sort() on empty d returns empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on empty d does not return empty array");
    }
    //check_equals( d.toString(), "" );
    if (d.toString() == "") {
        DejaGnu.pass("sort() on d leaves d empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on d does not leave d empty");
    }
    //d.sort( Array.UNIQUESORT );
    Reflect.callMethod( d, Reflect.field(d, "sort"), 
                       [untyped Array.UNIQUESORT]);
    //check_equals( d.toString(), "" );
    if (d.toString() == "") {
        DejaGnu.pass("sort(UNIQUESORT) on d leaves d empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(UNIQUESORT) on d does not leave d empty");
    }
    //d.sort( Array.DESCENDING | Array.NUMERIC );
    Reflect.callMethod( d, Reflect.field(d, "sort"), 
                       [untyped Array.DESCENDING | untyped Array.NUMERIC]);
    //check_equals( d.toString(), "" );
    if (d.toString() == "") {
        DejaGnu.pass("sort(DESCENDING | NUMERIC) on d leaves d empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(DESCENDING | NUMERIC) on d does not leave d empty");
    }

    //r = e.sort();
    r = Reflect.callMethod( e, Reflect.field(e, "sort"), []);
    //check_equals( r.toString(), "singleton" );
    if (r.toString() == "singleton") {
        DejaGnu.pass("sort() on e array returns correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on e array does not return correctly");
    }
    //check_equals( e.toString(), "singleton" );
    if (e.toString() == "singleton") {
        DejaGnu.pass("sort() on e array correctly sorts e");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on e array does not correctly sort e");
    }
    //e.sort( Array.UNIQUESORT );
    Reflect.callMethod( e, Reflect.field(e, "sort"), 
                       [untyped Array.UNIQUESORT]);
    //check_equals( e.toString(), "singleton" );
    if (e.toString() == "singleton") {
        DejaGnu.pass("sort(UNIQUESORT) on e array correctly sorts e");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(UNIQUESORT) on e array does not correctly sort e");
    }
    //e.sort( Array.DESCENDING | Array.CASEINSENSITIVE );
    Reflect.callMethod( e, Reflect.field(e, "sort"),[untyped Array.DESCENDING |
                       untyped Array.CASEINSENSITIVE]);
    //check_equals( e.toString(), "singleton" );
    if (e.toString() == "singleton") {
        DejaGnu.pass("sort(DESCENDING | CASEINSENSITIVE) on e array correctly sorts e");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(DESCENDING | CASEINSENSITIVE) on e array does not correctly sort e");
    }

    //r = f.sort();
    r = Reflect.callMethod( f, Reflect.field(f, "sort"), []);
    //check_equals( r.toString(), "Name,Name,Year" );
    if (r.toString() == "Name,Name,Year") {
        DejaGnu.pass("sort() on f array returns correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on f array does not return correctly");
    }
    //check_equals( f.toString(), "Name,Name,Year" );
    if (f.toString() == "Name,Name,Year") {
        DejaGnu.pass("sort() on f array sorts f correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on f array did not sort f correctly");
    }
    //r = f.sort( Array.UNIQUESORT );
    r = Reflect.callMethod( f, Reflect.field(f, "sort"), 
                           [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("sort(UNIQUESORT) on f returns correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(UNIQUESORT) on f does not return correctly");
    }
    //f.sort( Array.DESCENDING | Array.CASEINSENSITIVE );
    Reflect.callMethod( f, Reflect.field(f, "sort"), [untyped Array.DESCENDING |
                       untyped Array.CASEINSENSITIVE]);
    //check_equals( f.toString(), "Year,Name,Name" );
    if (f.toString() == "Year,Name,Name") {
        DejaGnu.pass("sort(DESCENDING | CASEINSENSITIVE) on f correctly sorts f");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] sort(DESCENDING | CASEINSENSITIVE) on f does not correctly sort f");
    }

    ////trace(" -- Return Indexed Array Tests -- ");
    DejaGnu.note("** Testing RETURNINDEXEDARRAY sorts");


    //r = a.sort( Array.RETURNINDEXEDARRAY );
    var r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "5,4,3,2,1,0" );
    if (r.toString() == "5,4,3,2,1,0") {
        DejaGnu.pass("RIA sort on a returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on a does not return correct indexes");
    }
    //check_equals( a.toString(), "vi,nano,emacs,ed,Jedit," );
    if (a.toString() == "vi,nano,emacs,ed,Jedit,") {
        DejaGnu.pass("RIA sort on  a does not change a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on a changed a");
    }
    //r = a.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY |
                            untyped Array.DESCENDING |
                            untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "0,1,4,2,3,5" );
    if (r.toString() == "0,1,4,2,3,5") {
        DejaGnu.pass("RIA sort on a returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA on a does not return correct indexes");
    }
    //check_equals( a.toString(), "vi,nano,emacs,ed,Jedit," );
    if (a.toString() == "vi,nano,emacs,ed,Jedit,") {
        DejaGnu.pass("RIA sort on  a does not change a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA on a changed a");
    }
    //r = b.sort( Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "5,6,7,4,3,2,1,0" );
    if (r.toString() == "5,6,7,4,3,2,1,0") {
        DejaGnu.pass("RIA sort on b returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on b does not return correct indexes");
    }
    //r = b.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY | 
                            untyped Array.NUMERIC]);
    //check_equals( r.toString(), "7,6,5,4,3,2,1,0" );
    if (r.toString() == "7,6,5,4,3,2,1,0") {
        DejaGnu.pass("RIA sort on b returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on b does not return correct indexes");
    }
    //r = b.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY | 
                            untyped Array.DESCENDING |
                            untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "0,1,2,3,4,7,6,5" );
    if (r.toString() == "0,1,2,3,4,7,6,5") {
        DejaGnu.pass("RIA sort on b returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on b does not return correct indexes");
    }
    //r = c.sort( Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "6,7,5,4,3,2,1,0" );
    if (r.toString() == "6,7,5,4,3,2,1,0") {
        DejaGnu.pass("RIA sort on c returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on c does not return correct indexes");
    }
    //r = c.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY |
                            untyped Array.NUMERIC]);
    //check_equals( r.toString(), "7,6,5,4,3,2,1,0" );
    if (r.toString() == "7,6,5,4,3,2,1,0") {
        DejaGnu.pass("RIA sort on c returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on c does not return correct indexes");
    }
    //r = c.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY |
                            untyped Array.DESCENDING |
                            untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "0,1,2,3,4,5,7,6" );
    if (r.toString() == "0,1,2,3,4,5,7,6") {
        DejaGnu.pass("RIA sort on c returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on c does not return correct indexes");
    }
    //r = d.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING );
    r = Reflect.callMethod( d, Reflect.field(d, "sort"), 
                           [untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("RIA sort on d returns empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on d does not return empty");
    }
    //check_equals( d.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("RIA sort on d leaves d empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on d does not leave d empty");
    }
    //r = d.sort( Array.NUMERIC | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( d, Reflect.field(d, "sort"), 
                           [untyped Array.NUMERIC |
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("RIA sort on d returns empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on d does not return empty");
    }
    //check_equals( d.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("RIA sort on d leaves d empty");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on d does not leave d empty");
    }
    //r = e.sort( Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( e, Reflect.field(e, "sort"), 
                           [untyped Array.CASEINSENSITIVE |
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("RIA sort on e returns single index");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on e does not return single index");
    }
    //check_equals( e.toString(), "singleton" );
    if (e.toString() == "singleton") {
        DejaGnu.pass("RIA sort on e did not change e");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on e changed e");
    }
    #if !flash9
    // flash9/gflashplayer can't do NUMERIC sorts on strings
    //r = e.sort( Array.NUMERIC | Array.RETURNINDEXEDARRAY | Array.DESCENDING );
    r = Reflect.callMethod( e, Reflect.field(e, "sort"), 
                           [untyped Array.NUMERIC |
                            untyped Array.RETURNINDEXEDARRAY |
                            untyped Array.DESCENDING]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("RIA sort on e returns single index");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] RIA sort on e does not return single index");
    }
    #end
	
    //trace(" -- Custom AS function tests -- ");
    DejaGnu.note("** Custom AS functon sort tests");
    
    //r = a.sort( cmp_fn, Array.UNIQUESORT );
    var r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.UNIQUESORT | 0]);
    //check_equals( r.toString(), ",vi,ed,nano,emacs,Jedit" );
    if (r.toString() == ",vi,ed,nano,emacs,Jedit") {
        DejaGnu.pass("cmp_fn sort returned correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn sort did not return correct array");
    }
    //check_equals( a.toString(), ",vi,ed,nano,emacs,Jedit" );
    if (a.toString() == ",vi,ed,nano,emacs,Jedit") {
        DejaGnu.pass("cmp_fn sort correctly sorted a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn sort did not correctly sort a");
    }
    #if !flash9
    // flash9/gflashplayer crashes on this
    //r = a.sort( something_undefined );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped something_undefined]);
    //check_equals(typeof(r), 'undefined');
    if (Std.string(untyped __typeof__(r)) == "undefined") {
        DejaGnu.pass("undefined sort returns 'undefined' array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] undefined sort does not return 'undefined' array");
    }
    #end
    //r = a.sort( cmp_fn, Array.DESCENDING );
    // The following is not working for some reason. Probable compiler bug
    // DejaGnu.note("DESCENDING = " + Std.string(untyped Array.DESCENDING));
    //r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           //[cmp_fn, untyped Array.DESCENDING]);
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.DESCENDING | 0]);
    //check_equals( tolen(r), "[5, 5, 4, 2, 2, 0]" );
    if (tolen(r) == "[5, 5, 4, 2, 2, 0]") {
        DejaGnu.pass("cmp_fn and DESCENDING sort returned correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn and DESCENDING sort did not return correctly");
    }
    //check_equals( tolen(a), "[5, 5, 4, 2, 2, 0]" );
    if (tolen(a) == "[5, 5, 4, 2, 2, 0]") {
        DejaGnu.pass("cmp_fn and DESCENDING sorted correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn and DESCENDING did not sort correctly");
    }
    //a.sort( cmp_fn, Array.CASEINSENSITIVE | Array.NUMERIC );
    Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.CASEINSENSITIVE |
                            untyped Array.NUMERIC]);
    //check_equals( tolen(a), "[0, 2, 2, 4, 5, 5]" );
    if (tolen(a) == "[0, 2, 2, 4, 5, 5]") {
        DejaGnu.pass("cmp_fn, CSE, NUM sorted correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, CSE, NUM did not sort correctly");
    }
    //r = a.sort( cmp_fn, Array.RETURNINDEXEDARRAY );
    //r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           //[cmp_fn, untyped Array.RETURNINDEXEDARRAY]);
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.RETURNINDEXEDARRAY | 0]);

    //check_equals( r.toString(), "0,1,2,3,4,5" );
    if (r.toString() == "0,1,2,3,4,5") {
        DejaGnu.pass("cmp_fn, RIA returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, RIA does not return correct indexes");
    }

    //r = a.sort( cmp_fn, Array.RETURNINDEXEDARRAY | Array.DESCENDING );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.RETURNINDEXEDARRAY |
                            untyped Array.DESCENDING]);

    //check_equals( r.toString(), "5,4,3,2,1,0" );
    if (r.toString() == "5,4,3,2,1,0") {
        DejaGnu.pass("cmp_fn, RIA, DESCENDING returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, RIA, DESCENDING not returning crrect indexes");
    }
    //r = d.sort( cmp_fn );
    r = Reflect.callMethod( d, Reflect.field(d, "sort"), 
                           [cmp_fn]);
    //check_equals( r.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("cmp_fn on empty array returns nothing");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn on empty array does not return nothing");
    }
    //check_equals( d.toString(), "" );
    if (d.toString() == "") {
        DejaGnu.pass("cmp_fn on empty array did not alter array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn on empty array aleterd the array");
    }
    //r = d.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( d, Reflect.field(d, "sort"), 
                           [cmp_fn, untyped Array.UNIQUESORT |
                            untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "" );
    if (r.toString() == "") {
        DejaGnu.pass("cmp_fn, + ARGS on d returns empty array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, + ARGS on d did not return empty array");
    }
    //check_equals( d.toString(), "" );
    if (d.toString() == "") {
        DejaGnu.pass("cmp_fn, + ARGS on d did not alter d");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, + ARGS on d altered d");
    }
    //r = e.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( e, Reflect.field(e, "sort"), 
                           [cmp_fn, untyped Array.UNIQUESORT |
                            untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "singleton" );
    if (r.toString() == "singleton") {
        DejaGnu.pass("cmp_fn, + ARGS on e returns correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, + ARGS on e did not return correct array");
    }
    //check_equals( e.toString(), "singleton" );
    if (e.toString() == "singleton") {
        DejaGnu.pass("cmp_fn, + ARGS on e correctly sorted e");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, + ARGS on e did not correctly sort e");
    }

    //trace(" -- Custom AS function tests using an AS comparator that returns objects -- ");
    //r = a.sort( cmp_fn_obj, Array.DESCENDING );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn_obj, untyped Array.DESCENDING | 0]);
    //check_equals( tolen(r), "[5, 5, 4, 2, 2, 0]" );
    if (tolen(r) == "[5, 5, 4, 2, 2, 0]") {
        DejaGnu.pass("cmp_fn_obj, DES sort returned correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn_obj, DES sort did not return correct array");
    }
    //check_equals( tolen(a), "[5, 5, 4, 2, 2, 0]" );
    if (tolen(a) == "[5, 5, 4, 2, 2, 0]") {
        DejaGnu.pass("cmp_fn_obj, DES on a sorted a correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn_obj, DES on a did not sort a correctly");
    }
    //a.sort( cmp_fn_obj, Array.CASEINSENSITIVE | Array.NUMERIC );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn_obj, untyped Array.CASEINSENSITIVE |
                            Array.NUMERIC]);
    //check_equals( tolen(a), "[0, 2, 2, 4, 5, 5]" );
    if (tolen(a) == "[0, 2, 2, 4, 5, 5]") {
        DejaGnu.pass("cmp_fn_obj, CAS, NUM, on a sorted a correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn_obj, CAS, NUM, on a did not sort a correctly");
    }
    //r = a.sort( cmp_fn_obj, Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn_obj, untyped Array.RETURNINDEXEDARRAY | 0]);
    //check_equals( r.toString(), "0,1,2,3,4,5" );
    if (r.toString() == "0,1,2,3,4,5") {
        DejaGnu.pass("cmp_fn_obj, RIA on a returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn_obj, RIA on a does not return correct indexes");
    }
    //r = a.sort( cmp_fn_obj, Array.RETURNINDEXEDARRAY | Array.DESCENDING );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn_obj, untyped Array.RETURNINDEXEDARRAY |
                            Array.DESCENDING]);
    //check_equals( r.toString(), "5,4,3,2,1,0" );
    if (r.toString() == "5,4,3,2,1,0") {
        DejaGnu.pass("cmp_fn_obj, RIA, DES on a returns correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn_obj, RIA, DES on a does not return correct indexes");
    }
    //e.sort( cmp_fn_obj, Array.UNIQUESORT | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( e, Reflect.field(e, "sort"), 
                           [cmp_fn_obj, untyped Array.UNIQUESORT |
                            Array.CASEINSENSITIVE]);
    //check_equals( e.toString(), "singleton" );
    if (e.toString() == "singleton") {
        DejaGnu.pass("cmp_fn_obj, UNI, CAS on e leaves single element");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn_obj, UNI, CAS on e does not leave single element");
    }

    a.push("ED");
    b.push(untyped 3.0);
    c.push(9/0);

    //trace(" -- UNIQUESORT tests -- ");
    DejaGnu.note("** Testing UNIQUESORT option");

    //r = a.sort( Array.UNIQUESORT );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
    if (r.toString() == ",ED,Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("UNIQ sort on a returned correct array");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNIQ sort on a did not return correct array");
    }
    //check_equals( a.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
    if (a.toString() == ",ED,Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("UNIQ sort on a correclty sorted a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNIQ sort on a did not correctly sort a");
    }
    //r = a.sort( Array.UNIQUESORT | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.UNIQUESORT | 
                            untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI, CAS sort on a correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI, CAS sort on a did not return correctly");
    }
    //check_equals( a.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
    if (a.toString() == ",ED,Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("UNI, CAS sort on a correctly sorted a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI, CAS sort on a did not correctly sort a");
    }
    //r = a.sort( Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.DESCENDING );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.UNIQUESORT | 
                            untyped Array.CASEINSENSITIVE |
                            untyped Array.DESCENDING]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|CAS|DES sort on a correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|CAS|DES sort on a did not return correctly");
    }
    //check_equals( a.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
    if (a.toString() == ",ED,Jedit,ed,emacs,nano,vi") {
        DejaGnu.pass("UNI|CAS|DES sort on a correctly sorted a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|CAS|DES sort on a did not correctly sort a");
    }
    //r = a.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.UNIQUESORT | 
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0,1,2,3,4,5,6" );
    if (r.toString() == "0,1,2,3,4,5,6") {
        DejaGnu.pass("UNI|RIA sort on a returned correct indexes");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|RIA sort on a did not correctly return");
    }
    //r = a.sort( Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.CASEINSENSITIVE |
                            untyped Array.DESCENDING | 
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|CAS|DES|RIA sort on a correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|CAS|DES|RIA sort on a did not correctly return");
    }

    //r = b.sort( Array.UNIQUESORT );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI sort on b correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on b did not return correctly");
    }
    //check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9,3" );
    if (b.toString() == "8,5,3,1,0,-2,-7,-9,3") {
        DejaGnu.pass("UNI sort on b sorted b correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on b did not sort b correctly");
    }
    //r = b.sort( Array.UNIQUESORT | Array.NUMERIC );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.NUMERIC]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|NUM sort on b correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM sort on b did not correctly return 0");
    }
    //r = b.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.NUMERIC |
                            untyped Array.DESCENDING]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|NUM|DES sort on b correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM|DES sort on b did no return correctly");
    }
    //r = b.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|RIA sort on b correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|RIA sort on b did not correctly return 0");
    }
    //r = b.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( b, Reflect.field(b, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.NUMERIC |
                            untyped Array.DESCENDING |
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|NUM|DES|RIA sort on b correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM|DES|RIA sort on did not return correctly");
    }

    //r = c.sort( Array.UNIQUESORT );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0" ) {
        DejaGnu.pass("UNI sort on c correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on c did not return correctly");
    }
    //check_equals( c.toString(), "Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7,Infinity" );
    if (c.toString() ==  "Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7,Infinity") {
        DejaGnu.pass("UNI sort on c correctly sorted c");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on c did not correctly sort c");
    }
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.NUMERIC]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|NUM sort on c correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM sort on c did not return correctly");
    }
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.UNIQUESORT | 
                            untyped Array.NUMERIC |
                            untyped Array.DESCENDING]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|NUM|DES sort on c correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM|DES sort on c did not return corrrectly");
    }
    //r = c.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|RIA sort on c correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|RIA sort on c did not return correctly");
    }
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
                           [untyped Array.UNIQUESORT |
                            untyped Array.NUMERIC |
                            untyped Array.DESCENDING |
                            untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
    if (r.toString() == "0") {
        DejaGnu.pass("UNI|NUM|DES|RIA sort on c correctly returned 0");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM|DES|RIA sort on c did not return correctly");
    }

    //r = a.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.UNIQUESORT |
			                untyped Array.CASEINSENSITIVE]);
    if ( r.toString() != "0" ) {
    //check_equals( tolen(r), "[0, 2, 2, 2, 4, 5, 5]" );
		if (tolen(r) == "[0, 2, 2, 2, 4, 5, 5]") {
			DejaGnu.pass("cmp_fn, UNI|CAS sort on a returned correctly");
		} else {
			DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, UNI|CAS sort on a did not return correctly");
		}
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sort returned 0");
	}
    //check_equals( tolen(a), "[0, 2, 2, 2, 4, 5, 5]" );
    if (tolen(a) == "[0, 2, 2, 2, 4, 5, 5]") {
        DejaGnu.pass("cmp_fn, UNI|CAS sort on a correctly sorted a");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, UNI|CAS sort on a did not correctly sort a");
    }
    //r = a.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.UNIQUESORT |
			                untyped Array.CASEINSENSITIVE |
			                untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0,1,2,3,4,5,6" ); 
    if (r.toString() == "0,1,2,3,4,5,6") {
        DejaGnu.pass("cmp_fn, UNI|CAS|RIA sort on a returned correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, UNI|CAS|RIA sort on a did not return correctly");
    }
    //r = a.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY | Array.DESCENDING );
    r = Reflect.callMethod( a, Reflect.field(a, "sort"), 
                           [cmp_fn, untyped Array.UNIQUESORT |
			                untyped Array.CASEINSENSITIVE |
			                untyped Array.RETURNINDEXEDARRAY |
			                untyped Array.DESCENDING]);
    //check_equals( r.toString(), "6,5,4,3,2,1,0" );
    if (r.toString() == "6,5,4,3,2,1,0") {
        DejaGnu.pass("cmp_fn, UNI|CAS|RIA|DES sort on a returned correctly");
    } else {
        DejaGnu.fail("[ln:"+here.lineNumber+"] cmp_fn, UNI|CAS|RIA|DES sort on a did not return correctly");
    }

    //trace(" -- Array with null value  -- ");
	DejaGnu.note("** Testing Array with null value");
	//c.push(null);
	Reflect.callMethod( c, Reflect.field(c, "push"), [untyped 'null']);

    //r = c.sort();
	var r = Reflect.callMethod( c, Reflect.field(c, "sort"), []);
    //check_equals( r.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity,Infinity,null" ); 
	if (r.toString() == "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity,Infinity,null") {
		DejaGnu.pass("sort() on c with null value returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on c with null value did not return correclty");
	}
    //check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity,Infinity,null" );
	if (c.toString() == "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity,Infinity,null") {
		DejaGnu.pass("sort() on c with null value sorted c correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sort() on c with null value did not sort c correctly");
	}
	DejaGnu.note("** Flash 9 does not allow numeric sort on null, skipping tests");
	#if !flash9
	// flash9/gflashplayer crashes when trying to numeric sort on null
    //c.sort( Array.NUMERIC );
	Reflect.callMethod( c, Reflect.field(c, "sort"), [untyped Array.NUMERIC]);
    //check_equals( c.toString(), "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity,Infinity,null" );
	if (c.toString() == "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity,Infinity,null") {
		DejaGnu.pass("NUM sort on c with null sorted c correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] NUM sort on c with null did not sort c correctly");
	}
    //c.sort( Array.DESCENDING | Array.NUMERIC );
	Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                   [untyped Array.DESCENDING |
					    untyped Array.NUMERIC]);
    //check_equals( c.toString(), "null,Infinity,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
	if (c.toString() == "null,Infinity,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7") {
		DejaGnu.pass("NUM|DES sort on c with a null sorted c correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] NUM|DES sort on c with a null did not sort c correctly");
	}
    //r = c.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.RETURNINDEXEDARRAY |
					        untyped Array.NUMERIC]);
    //check_equals( r.toString(), "9,8,7,6,5,4,3,1,2,0" );
	if (r.toString() == "9,8,7,6,5,4,3,1,2,0") {
		DejaGnu.pass("RIA|NUM sort on c with null returned correct indexes");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] RIA|NUM sort on c with null did not return correctly");
	}
	#end
    //r = c.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.RETURNINDEXEDARRAY |
					        untyped Array.DESCENDING |
							untyped Array.CASEINSENSITIVE]);
    //check_equals( r.toString(), "0,1,2,3,4,5,6,7,9,8" );
	if (r.toString() == "0,1,2,3,4,5,6,7,9,8") {
		DejaGnu.pass("RIA|DES|CAS sort on c returned correct indexes");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] RIA|DES|CAS sort on c did not return correctly");
	}
    //r = c.sort( Array.UNIQUESORT );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on c did not return correctly");
	}
	#if !flash9
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT |
						    untyped Array.NUMERIC]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI|NUM sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM sort on c did not return correctly");
	}
	#end
    //r = c.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT |
						    untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI|RIA sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|RIA sort on c did not retur correctly: \n\t" + r.toString());
	}
	#if !flash9
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT |
						    untyped Array.NUMERIC |
							untyped Array.DESCENDING |
						    untyped Array.RETURNINDEXEDARRAY]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("all ARGS sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] all ARGS sort on c did not return correctly");
	}
	#end
	
    //trace(" -- Array with 2 null values  -- ");
	DejaGnu.note("* Testing array with 2 null values");
    c = [7.2, 2.0, null, -0.5, 3/0, 0.0, null, 8.35, 0.001, -3.7];
    //c.sort( Array.NUMERIC );
	Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                   [untyped Array.NUMERIC]);
    //check_equals( c.toString(), "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity,null,null" );
	if (c.toString() == "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity,null,null") {
		DejaGnu.pass("NUM sort on c correctly sorted c");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] NUM sort on c did not correctly sort c");
	}
    //c.sort( Array.DESCENDING | Array.NUMERIC );
	Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                   [untyped Array.DESCENDING | untyped Array.NUMERIC]);
    //check_equals( c.toString(), "null,null,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
	if (c.toString() == "null,null,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7") {
		DejaGnu.pass("DES|NUM sort on c correctly sorted c");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] DES|NUM sort on c did not correctly sort c");
	}
    //r = c.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.RETURNINDEXEDARRAY |
					        untyped Array.NUMERIC]);
    //check_equals( r.toString(), "9,8,7,6,5,4,3,2,0,1" );
	if (r.toString() == "9,8,7,6,5,4,3,2,0,1") {
		DejaGnu.pass("RIA|NUM sort on c returned correct indexes");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] RIA|NUM sort on c did not return correctly");
	}
    //check_equals( c.toString(), "null,null,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
	if (c.toString() == "null,null,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7") {
		DejaGnu.pass("RIA|NUM sort on c correctly sorted c");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] RIA|NUM sort on c did not correctly sort c");
	}
    //r = c.sort( Array.UNIQUESORT );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on c did not return correctly");
	}
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT |
						    untyped Array.NUMERIC]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI|NUM sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM sort on c did not return correctly");
	}

    //trace(" -- Array with 2 undefined values  -- ");
	DejaGnu.note("** Testing sort with undefined");
    //c = [7.2, 2.0, undefined, -0.5, 3/0, 0.0, undefined, 8.35, 0.001, -3.7];
    c = [7.2, 2.0, undef, -0.5, 3/0, 0.0, undef, 8.35, 0.001, -3.7];
    //r = c.sort( Array.UNIQUESORT );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on c did not return correctly");
	}
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT |
						    untyped Array.NUMERIC]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI|NUM sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM sort on c did not return correctly \n" 
		             + r.toString());
	}

    //trace(" -- Array with 2 NaN values  -- ");
    //c = [7.2, 2.0, NaN, -0.5, 3/0, 0.0, NaN, 8.35, 0.001, -3.7];
    c = [7.2, 2.0, Math.NaN, -0.5, 3/0, 0.0, Math.NaN, 8.35, 0.001,
	     -3.7];
	//r = c.sort( Array.UNIQUESORT );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI sort on c witn 'NaN' correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI sort on c with 'NaN' did not return correctly \n"
		             + r.toString());
	}
    //r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
	r = Reflect.callMethod( c, Reflect.field(c, "sort"), 
	                       [untyped Array.UNIQUESORT | 
						    untyped Array.NUMERIC]);
    //check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("UNI|NUM sort on c correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] UNI|NUM sort on c did not return correctly");
	}
	
	}// end sort_Test()


	//-------------------------------
	// Test sortOn
	//-------------------------------
	static public function sortOn_Test() {
	DejaGnu.note("*** Begin testing Array.sortOn() function ***");
	
	//id = new Object();
    //id.toString = function () { return "Name"; };
    var id = { toString : function() {return "Name";} };
    //yr = new Object();
    //yr.toString = function () { return "Year"; };
    var yr = { toString : function() {return "Year";} };
	
	//numeric = new Object();
    //numeric.valueOf = function () { return Array.NUMERIC; };
	var numeric = { valueOf : function() {return untyped Array.NUMERIC;} };

	//a = [];
	var a = [];
	//a.push({Name: "Zuse Z3", Year: 1941, Electronic: false});
	Reflect.callMethod(a, Reflect.field(a, "push"), [{Name: "Zuse Z3",
	                   Year: "1941", Electronic: false}]);
	//a.push({Name: "Colossus", Year: 1943, Electronic: true});
	Reflect.callMethod(a, Reflect.field(a, "push"), [{Name: "Colossus",
	                   Year: "1943", Electronic: true}]);
	//a.push({Name: "ENIAC", Year: 1944, Electronic: true});
	Reflect.callMethod(a, Reflect.field(a, "push"), [{Name: "ENIAC",
	                   Year: "1944", Electronic: true}]);

	//b = [];
	var b = [];
	//b.push({Name: id, Year: yr, Electronic: yr});
	Reflect.callMethod(b, Reflect.field(b, "push"), [{Name: id,
	                   Year: yr, Electronic: yr}]);
	//b.push({Name: yr, Year: id, Electronic: yr});
	Reflect.callMethod(b, Reflect.field(b, "push"), [{Name: yr,
	                   Year: id, Electronic: yr}]);

	//function tostr(x)
	//{
		//var i;
		//str = "";
		//for(i = 0; i < x.length; i++)
		//{
			//y = x[i];
			//str += (y.Name + "," + y.Year + "," + y.Electronic );
			//if (i != x.length - 1) str += " | ";
		//}
		//return str;
	//}
	
	var tostr = function(x) {
		var i = 0;
		var str = "";
		while( i < x.length ) {
			var y = untyped x[i];
			str += (y.Name + "," + y.Year + "," + y.Electronic);
			if (i != x.length -1) str += " | ";
			i++;
		}
		return str;
	}
	

	//trace("sortOn a single property ");
	DejaGnu.note("** Testing sortOn single property");
	//r = a.sortOn( "Name" );
	var r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Name"]);
	//check_equals( tostr(r), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("sortOn('Name') on a returned correct array");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Name') on a did not return correctly");
	}
	//check_equals( tostr(a), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("sortOn('Name') on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Name') on a did not correctly sort a");
	}

	//r = a.sortOn( "Year" );
	var r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Year"]);
	//check_equals( tostr(r), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(r) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn('Year') on a returned correct array");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Year') on a did not return correctly");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn('Year') on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Year') on a did not correctly sort a");
	}

	//a.sortOn( "Electronic" );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Electronic"]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn('Electronic') on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Electronic') on a did not correctly sort a");
	}

	//a.sortOn("Year", Array.NUMERIC );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Year",
	                   untyped Array.NUMERIC]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn('Year', NUM) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Year', NUM) on a did not correctly sort a");
	}

	//a.sortOn("Year", Array.NUMERIC | Array.DESCENDING );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Year",
	                   untyped Array.NUMERIC | untyped Array.DESCENDING]);
	//check_equals ( tostr(a), "ENIAC,1944,true | Colossus,1943,true | Zuse Z3,1941,false" );
	if (tostr(a) == "ENIAC,1944,true | Colossus,1943,true | Zuse Z3,1941,false") {
		DejaGnu.pass("sortOn('Year', NUM|DES) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Year', NUM|DES) on a did not correctly sort a");
	}

	//r = a.sortOn("Year", Array.UNIQUESORT | Array.NUMERIC );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Year",
	                       untyped Array.UNIQUESORT | untyped Array.NUMERIC]);
	//check_equals ( tostr(r), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(r) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("('Year', UNI|NUM) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Year', UNI|NUM) on a did not return correctly");
	}
	//check_equals ( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn('Year', UNI|NUM) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn('Year', UNI|NUM) on a did not correctly sort a");
	}

	//r = a.sortOn("Year", Array.RETURNINDEXEDARRAY | Array.NUMERIC );
	var r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Year",
	                           untyped Array.RETURNINDEXEDARRAY |
						       untyped Array.NUMERIC]);
	//check_equals( r.toString(), "0,1,2" );
	if (r.toString() == "0,1,2") {
		DejaGnu.pass("('Year', RIA|NUM) on a returns correct indexes");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Year', RIA|NUM) on a did not return correclty");
	}
	//check_equals ( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("('Year', RIA|NUM) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Year', RIA|NUM) on a did not correctly sort a");
	}

	//r = a.sortOn("Name", Array.UNIQUESORT );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Name",
	                       untyped Array.UNIQUESORT]);
	//check_equals( tostr(r), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("('Name', UNI) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Name', UNI) on a did not return correctly");
	}
	//check_equals( tostr(a), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("('Name', UNI) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Name', UNI) on a did not correctly sort a");
	}

	//r = a.sortOn("Name", Array.UNIQUESORT | Array.DESCENDING );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Name",
	                       untyped Array.UNIQUESORT | 
						   untyped Array.DESCENDING]);
	//check_equals( tostr(r), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true" );
	if (tostr(r) == "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true") {
		DejaGnu.pass("('Name', UNI|DES) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Name', UNI|DES) on a did not return correctly");
	}

	//r = a.sortOn("Name", Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Name",
	                       untyped Array.UNIQUESORT | 
						   untyped Array.RETURNINDEXEDARRAY]);
	//check_equals( r.toString(), "2,1,0" );
	if (r.toString() == "2,1,0") {
		DejaGnu.pass("('Name', UNI|RIA) on a returns correct indexes");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Name', UNI|RIA) on a does not return correctly");
	}

	//r = a.sortOn("Electronic", Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Electronic",
	                       untyped Array.UNIQUESORT | 
						   untyped Array.RETURNINDEXEDARRAY]);
	//check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("('Electronic', UNI|RIA) on a correctly returned 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Electronic', UNI|RIA) on a does not return correctly");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true");
	if (tostr(a) == "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true") {
		DejaGnu.pass("('Electronic', UNI|DES) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Electronic', UNI|DES) on a did not correctly sort a");
	}

	//trace("sortOn multiple properties");
	DejaGnu.note("** testing sortOn multiple properties");
	
	//a.push({Name: "Atanasoff-Berry", Year: 1941, Electronic: true, Mass: 320});
	Reflect.callMethod(a, Reflect.field(a, "push"), [{Name: "Atanasoff-Berry",
	                   Year: "1941", Electronic: true, Mass: 320}]);

	//r = a.sortOn( ["Name", "Year"] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year"]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year']) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year']) on a did not return correctly");
	}
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year']) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year']) on a did not correctly sort a");
	}

	//a.sortOn( ["Electronic", "Year"] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Year"]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Electronic','Year']) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year']) on a did not correctly sort a");
	}

	//a.sortOn( ["Electronic", "Year"], [Array.DESCENDING, Array.NUMERIC] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Year"], 
	                   [untyped Array.DESCENDING, untyped Array.NUMERIC]]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Electronic','Year'],[DES,NUM]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year'],[DES,NUM]) on a did not correctly sort a");
	}

	//r = a.sortOn( ["Name", "Year"], [Array.UNIQUESORT, Array.NUMERIC] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year"], 
	                       [untyped Array.UNIQUESORT, untyped Array.NUMERIC]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[DES,NUM]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[DES,NUM]) on a did not return correctly");
	}

	//r = a.sortOn( ["Electronic", "Name"], [Array.UNIQUESORT, Array.NUMERIC] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), 
	                       [["Electronic","Year"], 
	                       [untyped Array.UNIQUESORT, untyped Array.NUMERIC]]);
	//check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(r) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Electronic','Year'],[DES,NUM]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year'],[DES,NUM]) on a did not return correctly");
	}


	//trace("sortOn missing properties" );
	DejaGnu.note("** Testing sortOn missing properties");
	
	//r = a.sortOn(["Megaflops"] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["Megaflops"]);
	//check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(r) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("('Megaflops') on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Megaflops') on a did not return correctly");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("('Megaflops') on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('Megaflops') on a did not correctly sort a");
	}

	//a.sortOn(["Binary", "Turing complete"] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Binary","Turing complete"]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Binary','Turing complete']) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Binary','Turing complete']) on a did not correctly sort a");
	}

	//a.sortOn(["Inventor", "Cost"], [Array.DESCENDING, 0] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Inventor","Cost"],
	                   [untyped Array.DESCENDING, 0]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Inventor','Cost'],[DES,0]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Inventor','Cost'],[DES,0]) on a did not correctly sort a");
	}

	//r = a.sortOn(["Name", "Year", "Cost"], [Array.DESCENDING, Array.NUMERIC, 0] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year","Cost"],
	                   [untyped Array.DESCENDING, untyped Array.NUMERIC, 0]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true" );
	if (tostr(a) == "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true") {
		DejaGnu.pass("(['Name','Year','Cost'],[DES,NUM,0]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year','Cost'],[DES,NUM,0]) on a did not correctly sort a");
	}

	//r = a.sortOn(["Name", "Cost", "Year"], [0, 0, Array.NUMERIC] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Cost","Year"],
	                   [0, 0, untyped Array.NUMERIC]]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Cost','Year'],[0,0,NUM]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Cost','Year'],[0,0,NUM]) on a did not correctly sort a");
	}

	//r = a.sortOn(["Electronic", "Year", "Cost"], [Array.UNIQUESORT, Array.NUMERIC, Array.NUMERIC] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Year","Cost"],
	                       [untyped Array.UNIQUESORT, untyped Array.NUMERIC,
						    untyped Array.NUMERIC]]);
	//check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(r) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Electronic','Year','Cost'],[UNI,NUM,NUM]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year','Cost'],[UNI,NUM,NUM]) on a did not return correctly");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Electronic','Year','Cost'],[UNI,NUM,NUM]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year','Cost'],[UNI,NUM,NUM]) on a did not correctly sort a");
	}

	//r = a.sortOn(["Electronic", "Cost" ], [Array.UNIQUESORT, Array.NUMERIC] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Cost"],
	                       [untyped Array.UNIQUESORT, untyped Array.NUMERIC]]);
	//check_equals( r.toString(), "0" );
	if (r.toString() == "0") {
		DejaGnu.pass("(['Electronic','Cost'],[UNI,NUM]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Cost'],[UNI,NUM]) on a did not return correctly");
	}


	//trace("sortOn with mismatching array lengths");
	DejaGnu.note("**Testing sortOn with mismatching array lengths");
	
	//r = a.sortOn( ["Name", "Year"], [0] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year"],
	                       [0]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[0]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[0]) on a did not return correctly");
	}
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[0]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[0]) on a did not correctly sort a");
	}

	//r = a.sortOn( ["Name", "Year"], [Array.DESCENDING] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year"],
	                       [untyped Array.DESCENDING]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[DES]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[DES]) on a did not return correctly");
	}
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[DES]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[DES]) on a did not correctly sort a");
	}

	//a.sortOn(["Name", "Electronic"], [Array.DESCENDING] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Electronic"],
	                       [untyped Array.DESCENDING]]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Electronic'],[DES]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Electronic'],[DES]) on a did not correctly sort a");
	}

	//r = a.sortOn(["Name", "Year"], [Array.RETURNINDEXEDARRAY] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year"],
	                       [untyped Array.RETURNINDEXEDARRAY]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[RIA]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[RIA]) on a did not return correctly");
	}
	

	//trace("sortOn, undocumented invocation");
	DejaGnu.note("** Testing sortOn, undocumented invocation");
	
	//r = a.sortOn( ["Name", "Year"], Array.DESCENDING );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name","Year"],
	                       untyped Array.DESCENDING]);
	//check_equals( tostr(r), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true" );
	if (tostr(r) == "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true") {
		DejaGnu.pass("(['Name','Year'],DES) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],DES) on a did not return correctly");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true" );
	if (tostr(a) == "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true") {
		DejaGnu.pass("(['Name','Year'],DES) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],DES) on a did not correctly sort a");
	}

	//a.sortOn( ["Year", "Name"], Array.NUMERIC );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Year","Name"],
	                       untyped Array.NUMERIC]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Name','Year'],NUM) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],NUM) on a did not correctly sort a");
	}

	//a.sortOn( ["Electronic", "Year", "Name"], Array.NUMERIC | Array.DESCENDING );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Year","Name"],
	                       untyped Array.NUMERIC | untyped Array.DESCENDING]);
	//check_equals( tostr(a), "ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Zuse Z3,1941,false" );
	if (tostr(a) == "ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Electronic','Year','Name'],NUM|DES) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year','Name'],NUM|DES) on a did not correctly sort a");
	}

	//r = a.sortOn(["Name", "Electronic"], [Array.DESCENDING] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name", "Electronic"],
	                       [untyped Array.DESCENDING]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Electronic'],[DES]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Electronic'],[DES]) on a did not return correctly");
	}
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Electronic'],[DES]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Electronic'],[DES]) on a did not correctly sort a");
	}

	//r = a.sortOn(["Name", "Year"], [Array.RETURNINDEXEDARRAY]);
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Name", "Year"],
	                       [untyped Array.RETURNINDEXEDARRAY]]);
	//check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(r) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[RIA]) on a returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[RIA]) on a did not return correctly");
	}
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Name','Year'],[RIA]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name','Year'],[RIA]) on a did not correctly sort a");
	}


	//trace("sortOn using an object implementing/over-riding the toString() method as the property argument");
	DejaGnu.note("** Testing sortOn with an object");

	//a.sortOn( id );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [id]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(id) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (id) on a did not correctly sort a");
	}

	//a.sortOn( id, Array.CASEINSENSITIVE | Array.DESCENDING );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [id, 
	                   untyped Array.CASEINSENSITIVE |
					   untyped Array.DESCENDING]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(id, CAS|DES) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (id, CAS|DES) on a did not correctly sort a");
	}

	//a.sortOn( [id], 0 );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [[id], 0]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("([id], 0) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ([id], 0) on a did not correctly sort a");
	}

	//a.sortOn( [yr, id], [Array.NUMERIC, Array.DESCENDING] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [[yr, id],
	                   [untyped Array.NUMERIC, untyped Array.DESCENDING]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("([yr, id],[NUM,DES]) on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ([yr,id],[NUM,DES]) on a did not correctly sort a");
	}


	//trace("sortOn with properties that are objects implementing the toString() method");
	DejaGnu.note("** Testing sortOn with object properties");

	//r = b.sortOn( "Name" );
	r = Reflect.callMethod(b, Reflect.field(b, "sortOn"), ["Name"]);
	//check_equals( tostr(r), "Name,Year,Year | Year,Name,Year");
	if (tostr(r) == "Name,Year,Year | Year,Name,Year") {
		DejaGnu.pass("(['Name']) on b returned correctly");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name']) on b did not return correctly");
	}
	//check_equals( tostr(b), "Name,Year,Year | Year,Name,Year");
	if (tostr(b) == "Name,Year,Year | Year,Name,Year") {
		DejaGnu.pass("(['Name']) on b correctly sorted b");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Name']) on b did not correctly sort b");
	}
	//b.sortOn( "Year" );
	Reflect.callMethod(b, Reflect.field(b, "sortOn"), ["Year"]);
	//check_equals( tostr(b), "Year,Name,Year | Name,Year,Year");
	if (tostr(b) == "Year,Name,Year | Name,Year,Year") {
		DejaGnu.pass("(['Year']) on b correctly sorted b");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Year']) on b did not correctly sort b");
	}
	//b.sortOn( ["Year", "Name"], [Array.NUMERIC | Array.DESCENDING, 0] );
	Reflect.callMethod(b, Reflect.field(b, "sortOn"), [["Year", "Name"], 
	                   [untyped Array.NUMERIC | untyped Array.DESCENDING, 0]]);
	//check_equals( tostr(b), "Name,Year,Year | Year,Name,Year");
	if (tostr(b) == "Name,Year,Year | Year,Name,Year") {
		DejaGnu.pass("([['Year','Name'],[NUM|DES,0]]) on b correctly sorted b");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ([['Year','Name'],[NUM|DES,0]]) on b did not correctly sort b");
	}
	


	//trace("sortOn invalid calls");
	DejaGnu.note("** Testing invalid calls to sortOn");
	
	#if !flash9
	// flash9/gflashplayer crashes with a call to sortOn with no arguments
	//r = a.sortOn();
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), []);
	//check( r == undefined );
	if (r == null) {
		DejaGnu.pass("sortOn() correctly returned 'undefined'");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn() did not return correctly");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn() on a correctly sorted a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn() on a did not correctly sort a");
	}
	#end

	//r = a.sortOn(undefined);
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [null]);
	//check_equals( typeof(r) , 'object' );
	if (Reflect.isObject(r)) {
		DejaGnu.pass("sortOn(undefined) on a returned an object");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn(undefined) on a did not return an object");
	}
	//check( r instanceof Array );
	#if !flash9
	if (untyped __instanceof__(r, Array)) {
		DejaGnu.pass("sortOn(undefined) on a returned an Array");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn(undefined) on a did not return an Array");
	}
	#else
	if (Std.is(r, "Array")) {
		DejaGnu.pass("sortOn(undefined) on a returned an Array");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn(undefined) on a returned an Array");
	}
	#end
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("sortOn(undefined) on a correctly did not sort a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] sortOn(undefined) on a changed a");
	}


	
	DejaGnu.note("** Testing miscellaneous sortOn() calls");
	//trace("sortOn with flag as an object overriding the valueOf method");
	//a.sortOn( ["Year", "Electronic", "Name"], numeric );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Year", "Electronic", "Name"],
	                   numeric ]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Year','Electronic','Name'],numeric) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Year','Electronic','Name'],numeric) on a did not correctly sort a");
	}


	#if flash6
	//trace("sortOn property name case-mismatch");
	//a.sortOn( "name" );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), ["name"]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("('name') on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] ('name') on a did not correctly sort a");
	}
	//a.sortOn( ["year", "name"], Array.NUMERIC );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["year","name"], 
	                   untyped Array.NUMERIC]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['year','name'],NUM) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['year','name'],NUM) on a did not correctly sort a");
	}
	#else

	//trace("sortOn with some properties undefined");
	//a.push({Name: "Harvard Mark I", Year: 1944, Mass: 4500});
	Reflect.callMethod(a, Reflect.field(a, "push"), [{Name: "Harvard Mark I", 
	                   Year: 1944, Mass: 4500}]);

	//a.sortOn(["Electronic", "Year"], Array.DESCENDING | Array.IGNORECASE );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Year"], 
	                   untyped Array.DESCENDING | untyped Array.IGNORECASE]);
	//check_equals( tostr(a), "Harvard Mark I,1944,undefined | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Harvard Mark I,1944,undefined | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Electronic','Year'],DES|IGN) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year'],DES|IGN) on a did not correctly sort a");
	}

	//a.sortOn( ["Electronic", "Name"], [Array.NUMERIC, Array.DESCENDING] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Name"], 
	                   [untyped Array.NUMERIC, untyped Array.DESCENDING]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined" );
	if (tostr(a) == "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined") {
		DejaGnu.pass("(['Electronic','Year'],[NUM,DES]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Year'],[NUM,DES]) on a did not correctly sort a");
	}
	
	//r = a.sortOn( ["Electronic", "Name"], [Array.UNIQUESORT, Array.NUMERIC] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Name"], 
	                   [untyped Array.UNIQUESORT, untyped Array.NUMERIC]]);
	if ( r.toString() != "0" ) {
	//check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Harvard Mark I,1944,undefined" );
		if (tostr(r) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Harvard Mark I,1944,undefined") {
			DejaGnu.pass("(['Electronic','Name'],[UNI,NUM]) on a returns correct array");
		} else {
			DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Name'],[UNI,NUM]) on a did not return correctly");
		}
	} else {
		DejaGnu.fail("[ls:"+here.lineNumber+"] sortOn returned 0");
	}
	//check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Harvard Mark I,1944,undefined" );
	if (tostr(a) == "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Harvard Mark I,1944,undefined") {
		DejaGnu.pass("(['Electronic','Name'],[UNI,NUM]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Name'],[UNI,NUM]) on a did not correctly sort a");
	}

	//a.sortOn( ["Mass", "Name"], [0, 0] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Mass","Name"], 
	                   [0, 0]]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false") {
		DejaGnu.pass("(['Mass','Name'],[0,0]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Mass','Name'],[0,0]) on a did not correctly sort a");
	}

	//a.sortOn( ["Mass", "Year", "Name"], [Array.NUMERIC | Array.DESCENDING, Array.NUMERIC | Array.DESCENDING | 0] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Mass","Year","Name"], 
	                   [untyped Array.NUMERIC | untyped Array.DESCENDING,
					    untyped Array.NUMERIC | untyped Array.DESCENDING | 0]]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true") {
		DejaGnu.pass("(['Mass','Year','Name'],[NUM|DES,NUM|DES|0]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Mass','Year','Name'],[NUM|DES,NUM|DES|0]) on a did not correctly sort a");
	}

	//a.sortOn( ["Mass", "Name"], [Array.UNIQUESORT, Array.DESCENDING] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Mass","Name"], 
	                   [untyped Array.UNIQUESORT, untyped Array.DESCENDING]]);
	//check_equals( tostr(a), "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true" );
	if (tostr(a) == "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true") {
		DejaGnu.pass("(['Mass','Name'],[UNI,DES]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Mass','Name'],[UNI,DES]) on a did not correctly sort a");
	}

	//a.sortOn( ["Electronic", "Mass", "Name"], [0, Array.NUMERIC | Array.DESCENDING, 0] );
	Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Mass","Name"], 
	                   [0,untyped Array.NUMERIC | untyped Array.DESCENDING,0]]);
	//check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true | Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined" );
	if (tostr(a) == "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true | Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined") {
		DejaGnu.pass("(['Electronic','Mass','Name'],[0,NUM|DES,0]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Mass','Name'],[0,NUM|DES,0]) on a did not correctly sort a");
	}

	//r = a.sortOn( ["Electronic", "Mass", "Year", "Name"], [Array.RETURNINDEXEDARRAY, Array.NUMERIC, Array.NUMERIC, Array.DESCENDING] );
	r = Reflect.callMethod(a, Reflect.field(a, "sortOn"), [["Electronic","Mass",
	                       "Year","Name"], [untyped Array.RETURNINDEXEDARRAY,
						    untyped Array.NUMERIC, untyped Array.NUMERIC,
							untyped Array.DESCENDING]]);
	//check_equals( r.toString(), "0,3,1,2,4");
	if (r.toString() == "0,3,1,2,4") {
		DejaGnu.pass("(['Electronic','Mass','Year','Name'],[RIA,NUM,NUM,DES]) on a correctly sorts a");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] (['Electronic','Mass','Year','Name'],[RIA,NUM,NUM,DES]) on a did not correctly sort a");
	}
	#end
	}//end sortOn_Test


	//-------------------------------------------------------
	// Test array enumeration
	//------------------------------------------------------
	static public function enumeration_Test() {
	DejaGnu.note("*** Begin Testing Array enumeration");

	//b = ["a","b","c"];
	var b = ["a", "b", "c"];
	//out = {len:0};
	var out = {len:0};
	// NOTE: Haxe does not use for..in loops in this way. So far I have not been
	//       able to override haxe's behavior in these types of loops. One could
	//       write a while loop that ends with the correct results, but it is 
	//       not clear if that is what is desired in this case.
	
	//for (var i in b)
	//{
			//check_equals(typeof(i), 'string');
			//out[i] = 1;
			//out['len']++;
	//}
	// NOTE: Tried to do the loop this way, but I'm not sure we are testing the 
	//       same thing now
	for(i in 0...b.length) {
		if (Std.is(b[i],String)) {
			DejaGnu.pass("i is a String");
		} else {
			DejaGnu.fail("[ln:"+here.lineNumber+"] i is not a String");
		}
		untyped out[i] = 1;
		untyped out['len']++;
	}
	
	//check_equals(out['len'], 3);
	if (untyped out['len'] == 3) {
		DejaGnu.pass("out['len'] == 3");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] out['len'] != 3");
	}
	//check_equals(out[0], 1);
	if (untyped out[0] == 1) {
		DejaGnu.pass("out[0] == 1");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"]out[0] != 1");
	}
	//check_equals(out[1], 1);
	if (untyped out[1] == 1) {
		DejaGnu.pass("out[1] == 1");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"]out[1] != 1");
	}
	//check_equals(out[2], 1);
	if (untyped out[2] == 1) {
		DejaGnu.pass("out[2] == 1");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"]out[2] != 1");
	}

	//b = [];
	b = [];
	//out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	out = {len:0}; 
	for (i in 0...b.length) { 
		untyped out[i] = 1;
		untyped out['len']++;
	}
	//check_equals(out['len'], 0);
	if (untyped out['len'] == 0) {
		DejaGnu.pass("out['len'] == 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"]out['len'] != 0");
	}

	// Changing length doesn't trigger enumeration of undefined values
	//b.length = 100;
	untyped b.length = 100;
	DejaGnu.note("b.length = " + b.length);
	//out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	out = {len:0};
	for(i in 0...b.length) {
		untyped out[i] = 1;
		untyped out['len']++;
	}
	// NOTE: Here is where the differences between the for...in loops start to
	//       cause problems. The test does not pass at this point, so changing
	//       to unresolved
	//DejaGnu.note("len = " + untyped out['len']);
	//check_equals(out['len'], 0);
	if (untyped out['len'] == 0) {
		DejaGnu.pass("out['len'] == 0");
	} else {
		DejaGnu.unresolved("[ln:"+here.lineNumber+"] out['len'] != 0");
	}

	//b[1] = undefined;
	b[1] = null;
	//out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	out = {len:0};
	for(i in 0...b.length) {
		untyped out[i] = 1;
		untyped out['len']++;
	}
	//check_equals(out['len'], 1);
	if (untyped out['len'] == 1) {
		DejaGnu.pass("out['len'] ==1");
	} else {
		DejaGnu.unresolved("[ln:"+here.lineNumber+"] out['len'] != 1");
	}
	//check_equals(out[1], 1);
	if (untyped out[1] == 1) {
		DejaGnu.pass("out[1] == 1");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] out[1] != 1");
	}

	//b[0] = undefined;
	b[0] = null;
	//out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	out = {len:0};
	for(i in b) {
		untyped out[i] = 1;
		untyped out['len']++;
	}
	//check_equals(out['len'], 2);
	if (untyped out['len'] == 2) {
		DejaGnu.pass("out['len'] == 2");
	} else {
		DejaGnu.unresolved("[ln:"+here.lineNumber+"] out['len'] != 2");
	}
	//check_equals(out[0], 1);
	if (untyped out[0] == 1) {
		DejaGnu.pass("out[0] == 1");
	} else {
		DejaGnu.unresolved("[ln:"+here.lineNumber+"] out[0] != 1");
	}
	//check_equals(out[1], 1);
	if (untyped out[1] == 1) {
		DejaGnu.pass("out[1] == 1");
	} else {
		DejaGnu.unresolved("[ln:"+here.lineNumber+"] out[1] != 1");
	}
	
	/*
	//Here are the ming tests since the haxe tests don't seem to work yet
	//-------------------------------------------------------
	// Test array enumeration
	//------------------------------------------------------

	b = ["a","b","c"];
	out = {len:0};
	for (var i in b)
	{
			check_equals(typeof(i), 'string');
			out[i] = 1;
			out['len']++;
	}
	check_equals(out['len'], 3);
	check_equals(out[0], 1);
	check_equals(out[1], 1);
	check_equals(out[2], 1);

	b = [];
	out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	check_equals(out['len'], 0);

	// Changing length doesn't trigger enumeration of undefined values
	b.length = 100;
	out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	check_equals(out['len'], 0);

	b[1] = undefined;
	out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	check_equals(out['len'], 1);
	check_equals(out[1], 1);

	b[0] = undefined;
	out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
	check_equals(out['len'], 2);
	check_equals(out[1], 1);
	check_equals(out[0], 1);
	*/
	}//end enumeration_Test
	
	
	//-------------------------------
	// Test length property
	//-------------------------------
	static public function lengthProperty_Test() {
	DejaGnu.note("*** Beging testing Array.length property");

	//a = new Array();
	var a = new Array();
	//check_equals(a.length, 0);
	if (a.length == 0) {
		DejaGnu.pass("new array a has length 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] new array a does not have length 0");
	}
	//a[-1] = 'minusone';
	a[-1] = 'minusone';
	//check_equals(a.length, 0);
	if (a.length == 0) {
		DejaGnu.pass("assigning a[-1] does not change a.length");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] assigning a[-1] changed a.length");
	}
	//check_equals(a[-1], 'minusone');
	if (a[-1] == 'minusone') {
		DejaGnu.pass("a[-1] == 'minusone' still");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a[-1] != 'minusone'");
	}
	//a["Infinite"] = 'inf';
	untyped a["Infinite"] = 'inf';
	//check_equals(a.length, 0);
	if (a.length == 0) {
		DejaGnu.pass("assigning a['Infinite'] does not change a.length");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] assigning a['Infinite'] changed a.length");
	}
	//check_equals(a["Infinite"], 'inf');
	if (untyped a["Infinite"] == 'inf') {
		DejaGnu.pass("a['Infinite'] == 'inf'");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a['Infinite'] != 'inf'");
	}
	
	}//end lengthProperty_Test
	
	
	//----------------------------------------------
	// Force an indexed property to a getter/setter
	//---------------------------------------------
	static public function propertyGetSet_Test() {
	DejaGnu.note("*** Begin testing getter/setter");

	// addProperty was added in SWF6
	// addProperty removed in flash 9
	#if !flash9

	var getCalls=0; var setCalls=0;
	//function get() { getCalls++; }
	var get = function() { getCalls++; }
	//function set() { setCalls++; }
	var set = function() { setCalls++; }
	//a = new Array();
	var a = new Array();
	//a[2] = 2;
	a[2] = 2;
	//ret = a.addProperty('1', get, set);
	var ret = untyped a.addProperty('1', get, set);
	//check_equals(ret, true);
	if (ret == true) {
		DejaGnu.pass("a.addProperty returned true");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a.addProperty returned false");
	}
	//junk = a[1];
	var junk = a[1];
	//check_equals(getCalls, 1);
	if (getCalls == 1) {
		DejaGnu.pass("getCalls increased by 1");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] getCalls not increased by 1");
	}
	//check_equals(setCalls, 0);
	if (setCalls == 0) {
		DejaGnu.pass("setCalls == 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] setCalls != 0");
	}
	
	getCalls=0; setCalls=0;
	//a[1] = 1;
	a[1] = 1;
	//check_equals(getCalls, 0);
	if (getCalls == 0) {
		DejaGnu.pass("getCalls == 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] getCalls != 0");
	}
	//xcheck_equals(setCalls, 1);
	if (setCalls == 1) {
		DejaGnu.xpass("[ln:"+here.lineNumber+"]setCalls increased by 1");
	} else {
		DejaGnu.xfail("[ln:"+here.lineNumber+"] setCalls not increased by 1");
	}

	//ret = a.addProperty('2', get, set);
	ret = untyped a.addProperty('2', get, set);
	//check_equals(ret, true);
	if (ret == true) {
		DejaGnu.pass("a.addProperty('2') returned true");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a.addProperty('2') did not return true");
	}
	
	getCalls=0; setCalls=0;
	//junk = a[2];
	junk = a[2];
	//xcheck_equals(getCalls, 1);
	if (getCalls == 1) {
		DejaGnu.xpass("[ln:"+here.lineNumber+"] getCalls increased by 1");
	} else {
		DejaGnu.xfail("[ln:"+here.lineNumber+"] getCalls not increased by 1");
	}
	//check_equals(setCalls, 0);
	if (setCalls == 0) {
		DejaGnu.pass("setCalls == 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] setCalls != 0");
	}
	
	getCalls=0; setCalls=0;
	//a[2] = 2;
	a[2] = 2;
	//check_equals(getCalls, 0);
	if (getCalls == 0) {
		DejaGnu.pass("getCalls == 0");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] getCalls != 0");
	}
	//xcheck_equals(setCalls, 1);
	if (setCalls == 1) {
		DejaGnu.xpass("[ln:"+here.lineNumber+"]setCalls increased by 1");
	} else {
		DejaGnu.xfail("[ln:"+here.lineNumber+"] setCalls not increased by 1");
	}

	//check_equals(a.length, 3);
	if (a.length == 3) {
		DejaGnu.pass("a.length == 3");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 3");
	}
	//ret = a.addProperty('3', get, set);
	ret = untyped a.addProperty('3', get, set);
	//xcheck_equals(a.length, 4);
	if (a.length == 4) {
		DejaGnu.xpass("a.length == 4");
	} else {
		DejaGnu.xfail("[ln:"+here.lineNumber+"] a.length != 4");
	}

	//a.length = 3;
	untyped a.length = 3;
	getCalls=0; setCalls=0;
	a.push(2);
	//check_equals(getCalls, 0);
	if (getCalls == 0) {
		DejaGnu.pass("a.push did not call get");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a.push made a call to get");
	}
	//check_equals(setCalls, 0);
	if (setCalls == 0) {
		DejaGnu.pass("a.push did not call set");
	} else {
		DejaGnu.fail("[ln:"+here.lineNumber+"] a.push made a call to set");
	}

	#end
	
	}//end propertyGetSet_Test
	
	
	//--------------------------------------------------------
	// pop an array with delete-protected elements
	//--------------------------------------------------------
	static public function popProtected_Test() {
        DejaGnu.note("*** Begin testing pop on delete protected array");
	
	
        //~ a = new Array();
        var a = new Array();
        //~ a[0] = 'zero';
        a[0] = 'zero';
        //~ a[1] = 'one';
        a[1] = 'one';
#if !flash9
        // Undocumented AS2 function?
        //~ ASSetPropFlags(a, "0", 7, 0); // protect 0 from deletion
        untyped ASSetPropFlags(a, "0", 7, 0);
        //~ check_equals(a.length, 2);
        if (a.length == 2) {
            DejaGnu.pass("a.length == 2");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 2");
        }
        //~ f = a.shift();
        var f = a.shift();
        //~ check_equals(a.length, 1);
        if (a.length == 1) {
            DejaGnu.pass("a.length == 1");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 1");
        }
        //~ check_equals(f, 'zero');
        if (f == 'zero') {
            DejaGnu.pass("f == 'zero'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] f != 'zero'");
        }
        //~ xcheck_equals(a[0], 'zero'); // could not delete for override
        if (a[0] == 'zero') {
            DejaGnu.xpass("[ln:"+here.lineNumber+"] a[0] == 'zero'");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] a[0] != 'zero'");
        }
        //~ check_equals(typeof(a[1]), 'undefined');
        if (Type.typeof(a[1]) == ValueType.TNull) {
            DejaGnu.pass("Type of a[1] is 'undefined'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] Type of a[1] is not 'undefined'");
        }

        //~ check(!a.hasOwnProperty(1));
        if ( !(untyped a.hasOwnProperty(1))) {
            DejaGnu.pass("a no longer has property '1'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a still has property '1'");
        }

        a = new Array();
        a[0] = 'zero';
        a[1] = 'one';
        //~ ASSetPropFlags(a, "1", 7, 0); // protect 1 from deletion
        untyped ASSetPropFlags(a, "1", 7, 0);
        //~ check_equals(a.length, 2);
        if (a.length == 2) {
            DejaGnu.pass("a.length == 2");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 2");
        }
        //~ f = a.shift();
        f = a.shift();
        //~ check_equals(a.length, 1);
        if (a.length == 1) {
            DejaGnu.pass("a.length == 1");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 1");
        }
        //~ check_equals(f, 'zero');
        if (f == 'zero') {
            DejaGnu.pass("f == 'zero'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] f != 'zero'");
        }
        //~ check_equals(a[0], 'one'); // could replace
        if (a[0] == 'one') {
            DejaGnu.pass("a[0] == 1");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a[0] != 1");
        }
        //~ xcheck_equals(a[1], 'one'); // couldn't delete
        if (a[1] == 'one') {
            DejaGnu.xpass("[ln:"+here.lineNumber+"] a[1] == 1");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] a[1] != 1");
        }
        //~ check(a.hasOwnProperty(0)); 
        if (untyped a.hasOwnProperty(0)) {
            DejaGnu.pass("a still has the property '0'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a no longer has the property '0'");
        }
        //~ xcheck(a.hasOwnProperty(1)); 
        if (untyped a.hasOwnProperty(1)) {
            DejaGnu.xpass("a still has the property '1'");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] a no longer has the property '1'");
        }
#end
	}//end popProtected_Test
	
	
	//--------------------------------------------------------
	// pop an array with read-only elements
	//--------------------------------------------------------
	static public function popReadOnly_Test() {
        DejaGnu.note("*** Begin testing pop on array with read-only elements");

        var a = new Array();
        a[0] = 'zero';
        a[1] = 'one';
#if !flash9
        //ASSetPropFlags(a, "0", 4, 0); // protect 0 from override
        untyped ASSetPropFlags(a, "0", 4, 0);
        //check_equals(a.length, 2);
        if (a.length == 2) {
            DejaGnu.pass("a.length == 2");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 2");
        }
        
        a[0] = 'overridden';
        //xcheck_equals(a[0], 'zero'); // was protected..
        if (a[0] == 'zero') {
            DejaGnu.xpass("[ln:"+here.lineNumber+"] a[0] == 'zero'");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] a[0] != 'zero'");
        }
        var f = a.shift();
        //check_equals(a.length, 1); 
        if (a.length == 1) {
            DejaGnu.pass("a.length == 1");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 1");
        }
        //xcheck_equals(f, 'zero');
        if (f == 'zero') {
            DejaGnu.xpass("[ln:"+here.lineNumber+"] f == 'zero'");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] f != 'zero'");
        }
        //check_equals(a[0], 'one'); // 0 was replaced anyway, didn't care about protection
        if (a[0] == 'one') {
            DejaGnu.pass("a[0] == 'one'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a[0] != 'one'");
        }
        //check_equals(typeof(a[1]), 'undefined');
        if (Type.typeof(a[1]) == ValueType.TNull) {
            DejaGnu.pass("Type of a[1] is 'undefined'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] Type of a[1] is not 'undefined'");
        }
        a[0] = 'overridden';
        //check_equals(a[0], 'overridden'); // flag was lost
        if (a[0] == 'overridden') {
            DejaGnu.pass("a[0] == 'overridden'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a[0] != 'overridden'");
        }
        //check(!a.hasOwnProperty(1)); 
        if ( !(untyped a.hasOwnProperty(1))) {
            DejaGnu.pass("a no longer has the property '1'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a still has the property '1'");
        }

        a = new Array();
        a[0] = 'zero';
        a[1] = 'one';
        a[2] = 'two';
        //ASSetPropFlags(a, "1", 4, 0); // protect 1 from override
        untyped ASSetPropFlags(a, "1", 4, 0);
        a[1] = 'overridden';
        //xcheck_equals(a[1], 'one'); // was protected
        if (a[1] == 'one') {
            DejaGnu.xpass("[ln:"+here.lineNumber+"] a[1] == 'one'");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] a[1] != 'one'");
        }
        //check_equals(a.length, 3);
        if (a.length == 3) {
            DejaGnu.pass("a.length == 3");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 3");
        }
        f = a.shift();
        //check_equals(a.length, 2);
        if (a.length == 2) {
            DejaGnu.pass("a.length == 2");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a.length != 2");
        }
        //check_equals(f, 'zero');
        if (f == 'zero') {
            DejaGnu.pass("f == 'zero'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] f != 'zero'");
        }
        //xcheck_equals(a[0], 'one'); // 0 was replaced anyway, didn't care about protection
        if (a[0] == 'one') {
            DejaGnu.xpass("[ln:"+here.lineNumber+"] a[0] == 'one'");
        } else {
            DejaGnu.xfail("[ln:"+here.lineNumber+"] a[0] == 'one'");
        }
        //check_equals(a[1], 'two');
        if (a[1] == 'two') {
            DejaGnu.pass("a[1] == 'two'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a[1] == 'two'");
        }
        //check_equals(typeof(a[2]), 'undefined');
        if (Type.typeof(a[2]) == ValueType.TNull) {
            DejaGnu.pass("Type of a[2] is 'undefined'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] Type of a[2] is not 'undefined'");
        }
        a[1] = 'overridden';
        //check_equals(a[1], 'overridden'); // flag was lost
        if (a[1] == 'overridden') {
            DejaGnu.pass("a[1] == 'overridden'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a[1] != overridden");
        }
        //check(a.hasOwnProperty(0));
        if (untyped a.hasOwnProperty(0)) {
            DejaGnu.pass("a still has property '0'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a no longer has property '0'");
        }
        //check(a.hasOwnProperty(1)); 
        if (untyped a.hasOwnProperty(1)) {
            DejaGnu.pass("a still has property '1'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a no longer has property '1'");
        }
        //check(!a.hasOwnProperty(2)); 
        if ( !(untyped a.hasOwnProperty(2))) {
            DejaGnu.pass("a no longer has property '2'");
        } else {
            DejaGnu.fail("[ln:"+here.lineNumber+"] a still has property '2'");
        }
#end
	}//end popReadOnly
	
}//end class Array_as

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
