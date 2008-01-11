// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Original author: Mike Carlson - June 19th, 2006

rcsid="$Id: String.as,v 1.32 2008/01/11 08:50:15 strk Exp $";

#include "check.as"

check_equals(typeof(String), 'function');
check_equals(typeof(String.prototype), 'object');
check_equals(String.__proto__, Function.prototype); // both undefined in swf5
check_equals(typeof(String.prototype.valueOf), 'function');
check_equals(typeof(String.prototype.toString), 'function');
check_equals(typeof(String.prototype.toUpperCase), 'function');
check_equals(typeof(String.prototype.toLowerCase), 'function');
check_equals(typeof(String.prototype.charAt), 'function');
check_equals(typeof(String.prototype.charCodeAt), 'function');
check_equals(typeof(String.prototype.concat), 'function');
check_equals(typeof(String.prototype.indexOf), 'function');
check_equals(typeof(String.prototype.lastIndexOf), 'function');
check_equals(typeof(String.prototype.slice), 'function');
check_equals(typeof(String.prototype.substring), 'function');
check_equals(typeof(String.prototype.split), 'function');
check_equals(typeof(String.prototype.substr), 'function');
check_equals(typeof(String.prototype.fromCharCode), 'undefined');
#if OUTPUT_VERSION > 5
 check_equals(typeof(String.valueOf), 'function');
 check_equals(typeof(String.toString), 'function');
#else
 check_equals(typeof(String.valueOf), 'undefined');
 check_equals(typeof(String.toString), 'undefined');
#endif
check_equals(typeof(String.toUpperCase), 'undefined');
check_equals(typeof(String.toLowerCase), 'undefined');
check_equals(typeof(String.charAt), 'undefined');
check_equals(typeof(String.charCodeAt), 'undefined');
check_equals(typeof(String.concat), 'undefined');
check_equals(typeof(String.indexOf), 'undefined');
check_equals(typeof(String.lastIndexOf), 'undefined');
check_equals(typeof(String.slice), 'undefined');
check_equals(typeof(String.substring), 'undefined');
check_equals(typeof(String.split), 'undefined');
check_equals(typeof(String.substr), 'undefined');
check_equals(typeof(String.fromCharCode), 'function');

#if OUTPUT_VERSION > 5
check(String.hasOwnProperty('fromCharCode'));
check(!String.hasOwnProperty('toString'));
check(!String.hasOwnProperty('valueOf'));
check(String.prototype.hasOwnProperty('valueOf'));
check(String.prototype.hasOwnProperty('toString'));
check(String.prototype.hasOwnProperty('toUpperCase'));
check(String.prototype.hasOwnProperty('toLowerCase'));
check(String.prototype.hasOwnProperty('charAt'));
check(String.prototype.hasOwnProperty('charCodeAt'));
check(String.prototype.hasOwnProperty('concat'));
check(String.prototype.hasOwnProperty('indexOf'));
check(String.prototype.hasOwnProperty('lastIndexOf'));
check(String.prototype.hasOwnProperty('slice'));
check(String.prototype.hasOwnProperty('substring'));
check(String.prototype.hasOwnProperty('split'));
check(String.prototype.hasOwnProperty('substr'));
#endif


check_equals(typeof(String()), 'string');

var a;
a = new String("wallawallawashinGTON");
check_equals(typeof(a), 'object');
check(a instanceof String);
check(a instanceof Object);
check_equals ( a.charCodeAt(0), 119 );
check_equals ( a.charCodeAt(1), 97 );
check_equals ( a.charCodeAt(2), 108 );
check_equals ( a.charCodeAt(3), 108 );
check_equals ( a.charCodeAt(4), 97 );
check_equals ( a.charAt(0), "w" );
check_equals ( a.charAt(1), "a" );
check_equals ( a.charAt(2), "l" );
check_equals ( a.charAt(3), "l" );
check_equals ( a.charAt(4), "a" );
isNaN ( a.charAt(-1) );
isNaN (a.charAt(21) );
check_equals ( a.lastIndexOf("lawa"), 8);

//----------------------------------------
// Check String.indexOf
// TODO: test with ASnative(251,8)
//-----------------------------------------


// wallawallawashinGTON
check_equals ( a.indexOf("lawa"), 3 );
check_equals ( a.indexOf("lawas"), 8 );
check_equals ( a.indexOf("hinG"), 13 );
check_equals ( a.indexOf("hing"), -1 );
check_equals ( a.indexOf("lawas", -1), 8 );
check_equals ( a.indexOf("a", 2), 4 );
check_equals ( a.indexOf("a", -1), 1 ); 
check_equals ( a.indexOf("a", -2), 1 ); 
check_equals ( a.indexOf("l"), 2 ); 
check_equals ( a.indexOf("l", 2.1), 2 ); 
check_equals ( a.indexOf("l", 2.8), 2 ); 
check_equals ( a.indexOf("l", 3), 3 ); 
check_equals ( a.indexOf("l", 3.5), 3 ); 
check_equals ( a.indexOf("l", 3.8), 3 ); 
check_equals ( a.indexOf("l", -3.8), 2 ); 
check_equals ( a.indexOf("l", -4.8), 2 ); 
check_equals ( a.indexOf("l", -4), 2 ); 
o = {}; o.valueOf = function() { return 2; };
check_equals ( a.indexOf("a", o), 4 ); 
o2 = {}; o2.toString = function() { return "a"; };
check_equals ( a.indexOf(o2, o), 4 ); 

//----------------------------------------
// Check String.split
// TODO: test with ASnative(251,12)
//-----------------------------------------

check_equals ( typeof(a.split), 'function' );
check ( ! a.hasOwnProperty('split') );
#if OUTPUT_VERSION > 5
check ( a.__proto__.hasOwnProperty('split') );
check ( a.__proto__ == String.prototype );
#endif

check_equals ( a.split()[0], "wallawallawashinGTON" );
check_equals ( a.split().length, 1 );
check ( a.split() instanceof Array );
check_equals ( a.split("w").length, 4);
check_equals ( a.split("  w").length, 1);

#if OUTPUT_VERSION > 5
check_equals ( a.split("")[0], "w" );
check_equals ( a.split("")[19], "N" );
check_equals ( a.split("la")[0], "wal" );
check_equals ( a.split("la")[1], "wal" );
check_equals ( a.split("la")[2], "washinGTON" );
check_equals ( a.split("la").length, 3 );
#else
// empty delimiter doesn't have a special meaning in SWF5
check_equals ( a.split("")[0], "wallawallawashinGTON" );
check_equals ( a.split("")[19], undefined );
// mulit-char delimiter doesn't work in SWF5
check_equals ( a.split("la")[0], "wallawallawashinGTON" );
check_equals ( a.split("la")[1], undefined );
check_equals ( a.split("la")[2], undefined );
check_equals ( a.split("la").length, 1 );
#endif

// TODO: test String.split(delim, limit)  [ second arg ]

primitiveString = '';
ret = primitiveString.split('x');
check_equals(typeof(ret), 'object');
check_equals(ret.length, 1);
check_equals(typeof(ret[0]), 'string');
check_equals(ret[0], '');

ret = primitiveString.split('x', -1);
#if OUTPUT_VERSION < 6
	check_equals(ret.length, 0); 
#else // OUTPUT_VERSION >= 6
	check_equals(ret.length, 1); 
#endif // OUTPUT_VERSION >= 6
ret = primitiveString.split('x', 0);
#if OUTPUT_VERSION < 6
	check_equals(ret.length, 0); 
#else // OUTPUT_VERSION >= 6
	check_equals(ret.length, 1); 
#endif // OUTPUT_VERSION >= 6
ret = primitiveString.split('x', 1);
check_equals(ret.length, 1);
ret = primitiveString.split('x', 2);
check_equals(ret.length, 1);

primitiveString = 'abcde';
ret = primitiveString.split('x');
check_equals(typeof(ret), 'object');
check_equals(ret.length, 1);
check_equals(typeof(ret[0]), 'string');
check_equals(ret[0], 'abcde');


//----------------------------------------
// Check String.fromCharCode
// TODO: test with ASnative(251,14)
//-----------------------------------------


// This is the correct usage pattern
var b = String.fromCharCode(97,98,99,100);
check_equals ( b, "abcd" );

//-------------------------------------------
// Check String.toUpperCase and toLowerCase
// TODO: test with ASnative(251,3)
//-------------------------------------------

check_equals ( a.toUpperCase(), "WALLAWALLAWASHINGTON" );
check_equals ( a.toLowerCase(), "wallawallawashington" );

//-------------------------------------------
// Check substr 
// TODO: test with ASnative(251,13)
//-------------------------------------------

a = new String("abcdefghijklmnopqrstuvwxyz");
check_equals ( a.substr(5,2), "fg" );
check_equals ( a.substr(5,7), "fghijkl" );
check_equals ( a.substr(-1,1), "z" );
check_equals ( a.substr(-2,3), "yz" );
check_equals ( a.substr(-3,2), "xy" );
var b = new String("1234");
check_equals ( b.substr(3, 6), "4");

//-------------------------------------------
// Check slice 
// TODO: test with ASnative(251,10)
//-------------------------------------------

a = new String("abcdefghijklmnopqrstuvwxyz");
check_equals ( a.slice(-5,-3), "vw" );
check_equals ( typeof(a.slice()), "undefined" );
check_equals ( typeof(a.slice(-5,3)), "string" );
check_equals ( a.slice(-5,3), "" );
check_equals ( typeof(a.slice(-10,22)), "string" );
check_equals ( a.slice(-10,22), "qrstuv" );
check_equals ( a.slice(0,0), "" );
check_equals ( a.slice(0,1), "a" );
check_equals ( a.slice(1,1), "" );
check_equals ( a.slice(1,2), "b" );
#if OUTPUT_VERSION > 5
check_equals ( a.slice.call(a, -5, -3), "vw" );
check_equals ( String.prototype.slice.call(a, -5, -3), "vw" );
#else
// There was no 'call' or 'apply' thing up to SWF5
// Actually, there was no Function interface at all!
check_equals ( a.slice.call(a, -5, -3), undefined );
check_equals ( String.prototype.slice.call(a, -5, -3), undefined );
#endif
check_equals ( a.slice(-4), "wxyz" );

//-------------------------------------------
// Check substring
// TODO: test with ASnative(251,11)
//-------------------------------------------

a = new String("abcdefghijklmnopqrstuvwxyz");
check_equals ( a.substring(5,2), "cde" );
check_equals ( a.substring(5,7), "fg" );
check_equals ( a.substring(3,3), "" );

check_equals ( a.length, 26 );
check_equals ( a.concat("sir ","william",15), "abcdefghijklmnopqrstuvwxyzsir william15");

var b = new String("1234");
check_equals ( b.substring(3, 6), "4");

// see check.as
#ifdef MING_SUPPORTS_ASM

// We need ming-0.4.0beta2 or later for this to work...
// This is the only way to generate an SWFACTION_SUBSTRING
// tag (the calls above generate a CALLMETHOD tag)
//
asm {
	push "b"
	push "ciao"
	push "2"
	push "10" // size is bigger then string length,
	          // we expect the interpreter to adjust it
	substring
	setvariable
};
check_equals( b, "iao");
asm {
	push "b"
	push "boowa"
	push "2"
	push "-1" // size is bigger then string length,
	          // we expect the interpreter to adjust it
	substring
	setvariable
};
check_equals( b, "oowa");
asm {
	push "b"
	push "ciao"
	push "-2" // negative base should be interpreted as 1
	push "1" 
	substring
	setvariable
};
check_equals( b, "c");
asm {
	push "b"
	push "ciao"
	push "-2" // negative base should be interpreted as 1
	push "10" // long size reduced 
	substring
	setvariable
};
check_equals( b, "ciao");
asm {
	push "b"
	push "ciao"
	push "0" // zero base is invalid, but taken as 1
	push "1" 
	substring
	setvariable
};
check_equals( b, "c");
asm {
	push "b"
	push "ciao"
	push "10" // too large base ...
	push "1" 
	substring
	setvariable
};
check_equals( b, "");
asm {
	push "b"
	push "all"
	push "3" // base is 1-based!
	push "1" 
	substring
	setvariable
};
check_equals( b, "l");
#endif


// Test inheritance with built-in functions
var stringInstance = new String();
check (stringInstance.__proto__ != undefined);
check (stringInstance.__proto__ == String.prototype);
check_equals (typeOf(String.prototype.constructor), 'function');
check (String.prototype.constructor == String);
check (stringInstance.__proto__.constructor == String);

// Test the instanceof operator
check ( stringInstance instanceof String );
check ( ! "literal string" instanceof String );

// Test automatic cast of string values to String objects
// this should happen automatically when invoking methods
// on a primitive string type
var a_string = "a_string";
check_equals(typeof(a_string), "string");
check_equals (a_string.substring(0, 4), "a_st");
check_equals (a_string.substring(-3, 4), "a_st");
check_equals (a_string.substring(0, -1), "");
check_equals (a_string.substring(0, 1), "a");
check_equals (a_string.substring(4), "ring");
check_equals (a_string.substring(16), "");
check_equals (a_string.substring(-16), "a_string");
check_equals (a_string.toUpperCase(), "A_STRING");
check_equals (a_string.indexOf("hing"), -1 );
check_equals (a_string.indexOf("string"), 2 );
check_equals (a_string.charCodeAt(0), 97 );
a_string = ""; // empty
check_equals (a_string.substring(0, 1), "");

// Test String.length not being overridable
a_string = "1234567890";
check_equals(a_string.length, 10);
a_string.length = 4;
check_equals(a_string.length, 10);
check_equals(a_string, "1234567890");


//----------------------------------------------------
// Test automatic string conversion when adding stuff
//-----------------------------------------------------
a = "one";
b = "two";
check_equals(a+b, "onetwo");
c = new Object();

check_equals(b+c, "two[object Object]");

// check that calls to toString() use the current environment
c.toString = function() { return a; };
prevToStringFunc = c.toString;
check_equals(b+c, "twoone");

// this won't be used as a valid toString method !
c.toString = function() { return 4; };
#if OUTPUT_VERSION >= 6
check(c.toString != prevToStringFunc);
#endif
check_equals(b+c, "two[type Object]");

ObjectProtoToStringBackup = Object.prototype.toString;
Object.prototype.toString = undefined;
check_equals(typeof(c.toString), 'function');
check_equals(b+c, "two[type Object]");
Object.prototype.toString = ObjectProtoToStringBackup;

c.toString = undefined;
check_equals(typeof(c.toString), 'undefined');
check_equals(b+c, "two[type Object]");

stringObject = new String("1234");
check_equals(typeof(stringObject.valueOf), 'function');
check_equals(stringObject.valueOf, String.prototype.valueOf);

#if OUTPUT_VERSION > 5
check(stringObject.valueOf != Object.prototype.valueOf);
check(String.prototype.hasOwnProperty('valueOf'));
#endif

check_equals(typeof(stringObject.valueOf()), 'string');
check_equals(stringObject, "1234");
check_equals(stringObject, 1234);
check_equals(1234, stringObject);
numberObject = new Number(1234);
#if OUTPUT_VERSION >= 6
check(stringObject != numberObject);
#else
check_equals(stringObject, numberObject); // SWF5 always converts to primitive before comparison !!
#endif
check_equals(numberObject.toString(), stringObject);
check_equals(numberObject.toString(), stringObject.toString());

//----------------------------------------------------------------------
// Drop the toString method of a string (also a test for ASSetPropFlags)
//----------------------------------------------------------------------

s = new String("a");
check_equals(typeof(Object.prototype.toString), 'function');
check_equals(typeof(s.toString), 'function');
check(! delete String.prototype.toString);
ASSetPropFlags(String.prototype, "toString", 0, 7); // unprotect from deletion
StringProtoToStringBackup = String.prototype.toString;
check(delete String.prototype.toString);
check_equals(typeof(s.toString), 'function');
check(!delete Object.prototype.toString);
ASSetPropFlags(Object.prototype, "toString", 0, 7); // unprotect from deletion
ObjectProtoToStringBackup = Object.prototype.toString;
check(delete Object.prototype.toString);
check_equals(typeof(s.toString), 'undefined');
Object.prototype.toString = ObjectProtoToStringBackup;
String.prototype.toString = StringProtoToStringBackup;

#if OUTPUT_VERSION < 6
 check_totals(173);
#else
 check_totals(194);
#endif
