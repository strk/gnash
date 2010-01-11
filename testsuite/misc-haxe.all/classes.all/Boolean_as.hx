// Boolean_as.hx: ActionScript 3 "Boolean" class, for Gnash.
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

// Test case for Boolean ActionScript class
// Migrated to haXe by Jonathan Crider

#if flash9
import flash.display.MovieClip;
#else
import flash.MovieClip;
#end

import flash.Lib;
import Type;
import Reflect;
import Std;

// importing our testing api
import DejaGnu;

class Boolean_as {
	static function main() {
	
	//NOTE: These tests do not compile in swf version 9. This is due to the way
	//     in which haxe implements a Bool. Compilation in swf9 gives the error
	//     'Unbound variable Boolean' at each place the Boolean name is used.
	//     So far I have been unable to find a workaround for this. For now we
	//     will be unable to compile these test for flash9.
	#if flash9
	DejaGnu.note("These tests do not currently compile in flash version 9: see comments in Boolean_as.hx");
    
	#else
	
	//check_equals(typeof(Boolean), 'function');
	if ( untyped __typeof__(Boolean) == 'function') {
			DejaGnu.pass("Boolean class exists");
    } else {
			DejaGnu.fail("Boolean class does not exist");
    }
	//check_equals(typeof(Boolean()), 'undefined');
	if ( untyped __typeof__(Boolean()) == 'undefined') {
		DejaGnu.pass("Call to Boolean() returns null");
	} else {
		DejaGnu.fail("Call to Boolean() does not return null");
	}
	//check_equals(typeof(Boolean(true)), 'boolean');
	if ( untyped __typeof__(Boolean(true)) == 'boolean') {
		DejaGnu.pass("Boolean(true) correctly returns a boolean value");
	} else {
		DejaGnu.fail("Boolean(true) does not return a boolean value");
	}
	//check_equals(typeof(new Boolean()), 'object');
	if (Reflect.isObject(untyped __new__(Boolean))) {
		DejaGnu.pass("new Boolean(); correctly constructs an object");
	} else {
		DejaGnu.fail("new Boolean(); does not correctly construct an object");
	}
	
	//var boolObj = new Boolean;
	var boolObj = untyped __new__(Boolean);

	// test the Boolean constuctor
	//check (boolObj);
	if (boolObj	) {
		DejaGnu.pass("Boolean object successfully constructed and assigned");
	} else {
		DejaGnu.fail("Boolean object not successfully constructed or assigned");
	}

	//check (boolObj.toString);
	if (untyped boolObj.toString) {
		DejaGnu.pass("boolObj.toString() inherited correctly");
	} else {
		DejaGnu.fail("boolObj.toString() was not inherited correctly");
	}
	//check (boolObj.valueOf);
	if (untyped boolObj.valueOf) {
		DejaGnu.pass("boolObj.valueOf() inherited correctly");
	} else {
		DejaGnu.fail("boolObj.valueOf() not inherited correctly");
	}


	#if flash6
	// flash6 is not case sensitive
	//check (boolObj.tostring)
	if (untyped boolObj.tostring) {
		DejaGnu.pass("boolObj.tostring property exists (Not Case Sensitive)");
	} else {
		DejaGnu.fail("boolObj.tostring property does not exist");
	}
	//check (boolObj.valueof)
	if (untyped boolObj.valueof) {
		DejaGnu.pass("boolObj.valueof property exists (Not Case Sensitive)");
	} else {
		DejaGnu.fail("boolObj.valueof property does not exist");
	}
	#else
	// SWF7 and up is case-sensitive
	//check_equals (boolObj.tostring, undefined);
	if (Type.typeof(untyped boolObj.tostring) == ValueType.TNull) {
		DejaGnu.pass("tostring property does not exist (Case Sensitive)");
	} else {
		DejaGnu.fail("tostring property exists when it should not");
	}
	//check_equals (boolObj.valueof, undefined);
	if (Type.typeof(untyped boolObj.valueof) == ValueType.TNull) {
		DejaGnu.pass("valueof property does not exist (Case Sensitive)");
	} else {
		DejaGnu.fail("valueof property exists when it should not");
	}
	#end

	//var defaultBool = new Boolean();
	var defaultBool = untyped __new__(Boolean);
	//check_equals(defaultBool.toString(), "false");
	if (untyped defaultBool.toString() == "false") {
		DejaGnu.pass("Default constructor correctly sets value to false");
	} else {
		DejaGnu.fail("Default constructor does not set value to false");
	}
	//check_equals(defaultBool.valueOf(), false);
	if (untyped defaultBool.valueOf() == false) {
		DejaGnu.pass("Default valueOf() correctly returns false");
	} else {
		DejaGnu.fail("Default valueOf() does not return false");
	}

	//var trueBool = new Boolean(true);
	var trueBool = untyped __new__(Boolean, true);
	//check_equals(trueBool.toString(), "true");
	if (untyped trueBool.toString() == "true") {
		DejaGnu.pass("Correctly constructed Boolean with value 'true'");
	} else {
		DejaGnu.fail("Did not correctly construct Boolean with value 'true'");
	}
	//check_equals(trueBool.valueOf(), true);
	if (untyped trueBool.valueOf() == true) {
		DejaGnu.pass("trueBool.valueOf() correctly returned true");
	} else {
		DejaGnu.fail("trueBool.valueOf() did not correctly return true");
	}

	//var falseBool = new Boolean(false);
	var falseBool = untyped __new__(Boolean, false);
	//check_equals(falseBool.toString(), "false");
	if (untyped falseBool.toString() == "false") {
		DejaGnu.pass("Boolean correctly constructed with argument 'false'");
	} else {
		DejaGnu.fail("Boolean not correctly constructed with argument 'false'");
	}
	//check_equals(falseBool.valueOf(), false);
	if (untyped falseBool.valueOf() == false) {
		DejaGnu.pass("falseBool.valueOf() correctly returned false");
	} else {
		DejaGnu.fail("falseBool.valueOf() did not correctly return false");
	}


	//---------------------------------------------------
	// Test convertion to boolean
	//---------------------------------------------------
	DejaGnu.note("*** Begin testing convertion to Boolean");

	// boolean
	//check( true );
	if (true) {
		DejaGnu.pass("keyword 'true' correctly evaluates to true");
	} else {
		DejaGnu.fail("keyword 'true' does not correctly evaluate to true");
	}
	//check( ! false );
	if ( ! false ) {
		DejaGnu.pass("expression '! false' correctly evaluates to true");
	} else {
		DejaGnu.fail("expression '! false' did not evaluate to true");
	}
	 
	// number
	//check( 1 );
	if (untyped 1) {
		DejaGnu.pass("expression '1' correctly evaluates to true");
	} else {
		DejaGnu.fail("expression '1' did not correctly evaluate to true");
	}
	
	if (untyped 0) {
		DejaGnu.fail("expression '0' evaluated to true");
	} else {
		DejaGnu.pass("expression '0' evaluated to false");
	}
	//check( !0 );
	if ( untyped !0 ) {
		DejaGnu.pass("expression '!0' correctly evaluates to true");
	} else {
		DejaGnu.fail("expression '!0' did not evaluate to true");
	}

	// movieclip
	//check( _root );
	if (untyped flash.Lib.current) {
		DejaGnu.pass("_root; (flash.lib.current in haxe) evaluated true");
	} else {
		DejaGnu.fail("_root; (flash.lib.current in haxe) evaluated false");
	}

	// string
	//check( "1" );
	if (untyped "1") {
		DejaGnu.pass("String expression '1' evaluated true");
	} else {
		DejaGnu.fail("String expression '1' did not evaluate true");
	}
	//#if OUTPUT_VERSION < 7
	#if flash6
	//check( ! "0" );
	if ( untyped !"0" ) {
		DejaGnu.pass("string expression !'0' evaluated true");
	} else {
		DejaGnu.fail("string expression !'0' did not evaluate true");
	}
	//check( ! "true" );
	if ( untyped !"true") {
		DejaGnu.pass("string expression !'true' evaluated true");
	} else {
		DejaGnu.fail("string expression !'true' did not evaluate true");
	}
	//check( ! "false" );
	if ( untyped !"false") {
		DejaGnu.pass("string expression !'false' evaluated true");
	} else {
		DejaGnu.fail("string expression !'false' did not evaluate true");
	}
	#else
	//check( "0" );
	if ( untyped "0" ) {
		DejaGnu.pass("string expression '0' evaluated true");
	} else {
		DejaGnu.fail("string expression '0' did not evaluate true");
	}
	//check( "true" );
	if ( untyped "true" ) {
		DejaGnu.pass("string expression 'true' evaluated true");
	} else {
		DejaGnu.fail("string expression 'true' did not evaluate true");
	}
	//check( "false" );
	if ( untyped "false" ) {
		DejaGnu.pass("string expression 'false' evaluated true");
	} else {
		DejaGnu.fail("string expression 'false' did not evaluate true");
	}
	//#endif
	#end

	// Null 
	//check_equals(typeOf(null), "null" );
	if (untyped __typeof__(null) == "null") {
		DejaGnu.pass("typeof null is null");
	} else {
		DejaGnu.fail("typeof null is not null");
	}
	//check( ! null );
	if ( ! null ) {
		DejaGnu.pass("expression '!null' evaluates to true");
	} else {
		DejaGnu.fail("expression '!null' did not evaluate to true");
	}

	// Undefined
	//check( ! undefined );
	if ( !(untyped undefined) ) {
		DejaGnu.pass("expression '! undefined' evaluates to true");
	} else {
		DejaGnu.fail("expression '! undefined' did not evaluate to true");
	}

	// Function
	//var playfunc = untyped __global__["play"];
	//check( play );
	if ( untyped play ) {
		DejaGnu.pass("Global function play evaluates to true");
	} else {
		DejaGnu.fail("Global function play does not evaluate to true");
	}

	// Object - conversion might depend on object type
	//emptyArray = new Array();
	var emptyArray = untyped __new__(Array);
	//check( emptyArray );
	if ( emptyArray ) {
		DejaGnu.pass("emptyArray object evaluates to true");
	} else {
		DejaGnu.fail("emptyArray object does not evaluate to true");
	}
	
	#end //end if !flash9

	
	//NOTE: may need to retain Ming tests at the end of the file somehow
		
	DejaGnu.done();
	}
}

//NOTE: Haxe does not give acces to the Boolean class directly.
//      In haXe Bool is an Enum value and does all the processing in the
//      background.

