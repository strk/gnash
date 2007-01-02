// Mike Carlson's test program for actionscript strings
// June 19th, 2006

rcsid="$Id: String.as,v 1.10 2007/01/02 15:43:42 strk Exp $";

#include "check.as"

var a;
a = new String("wallawallawashinGTON");
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
check_equals ( a.indexOf("lawa"), 3 );
check_equals ( a.lastIndexOf("lawa"), 8);
check_equals ( a.indexOf("lawas"), 8 );
check_equals ( a.indexOf("hinG"), 13 );
check_equals ( a.indexOf("hing"), -1 );
check_equals ( a.split()[0], "wallawallawashinGTON" );
check_equals ( a.split().length, 1 );
check ( a.split() instanceof Array );
check_equals ( a.split("")[0], "w" );
check_equals ( a.split("")[19], "N" );
check_equals ( a.split("la")[0], "wal" );
check_equals ( a.split("la")[1], "wal" );
check_equals ( a.split("la")[2], "washinGTON" );
check_equals ( a.split("la").length, 3 );


// This is the correct usage pattern
var b = String.fromCharCode(97,98,99,100);
check_equals ( b, "abcd" );

check_equals ( a.toUpperCase(), "WALLAWALLAWASHINGTON" );
check_equals ( a.toLowerCase(), "wallawallawashington" );
a = new String("abcdefghijklmnopqrstuvwxyz");
check_equals ( a.substr(5,2), "fg" );
check_equals ( a.substr(5,7), "fghijkl" );
check_equals ( a.substr(-1,1), "z" );
check_equals ( a.substr(-2,3), "yz" );
check_equals ( a.substr(-3,2), "xy" );
check_equals ( a.slice(-5,-3), "vw" );
check_equals ( a.slice(-4), "wxyz" );
check_equals ( a.substring(5,2), "cde" );
check_equals ( a.substring(5,7), "fg" );
check_equals ( a.length, 26 );
check_equals ( a.concat("sir ","william",15), "abcdefghijklmnopqrstuvwxyzsir william15");
var b = new String("1234");
check_equals ( b.substring(3, 6), "4");
check_equals ( b.substr(3, 6), "4");

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
#endif


// Test inheritance with built-in functions
var stringInstance = new String();
check (stringInstance.__proto__ != undefined);
check (stringInstance.__proto__ == String.prototype);
check (String.prototype.constructor != undefined);
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
check_equals (a_string.substring(4), "ring");
check_equals (a_string.substring(16), "");
check_equals (a_string.substring(-16), "a_string");
check_equals (a_string.toUpperCase(), "A_STRING");
check_equals (a_string.indexOf("hing"), -1 );
check_equals (a_string.indexOf("string"), 2 );
check_equals (a_string.charCodeAt(0), 97 );
