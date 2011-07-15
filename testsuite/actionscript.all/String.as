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


rcsid="String.as";

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
check_equals(typeof(String.prototype.length), 'undefined');
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

// Tests for SWF5 at the end of the file.

check(String.hasOwnProperty('fromCharCode'));
check(!String.hasOwnProperty('toString'));
check(!String.hasOwnProperty('valueOf'));
check(String.hasOwnProperty('__proto__'));
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
check(!String.prototype.hasOwnProperty('length'));

#endif

check_equals(typeof(String()), 'string');

var a;
a = new String("wallawallawashinGTON");
check_equals(a.length, 20);
#if OUTPUT_VERSION > 5
check(a.hasOwnProperty('length'));
#endif
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

// lastIndexOf properly tested.
r = "abcdefghik7abedc";
check_equals(r.lastIndexOf("dc"), 14);
check_equals(r.lastIndexOf("abcde"), 0);
check_equals(r.lastIndexOf("ab"), 11);
check_equals(r.lastIndexOf("df"), -1);
check_equals(r.lastIndexOf("ik"), 8);
check_equals(r.lastIndexOf("cd"), 2);

check_equals(r.lastIndexOf("edc", 15), 13);
check_equals(r.lastIndexOf("edc", 14), 13);
check_equals(r.lastIndexOf("edc", 13), 13);
check_equals(r.lastIndexOf("edc", 12), -1);
check_equals(r.lastIndexOf("edc", 11), -1);

check_equals(r.lastIndexOf("ghi", "rt"), -1);
check_equals(r.lastIndexOf("ghi", 15, 8), 6);
check_equals(r.lastIndexOf("ghi", 15, 8, 9), 6);
check_equals(r.lastIndexOf("ghi", -1), -1);
check_equals(r.lastIndexOf("ghi", 17287638764), 6);

check_equals(r.lastIndexOf(""), 16);
check_equals(r.lastIndexOf(7), 10);

// UTF8 lastIndexOf
s = "tést";
#if OUTPUT_VERSION > 5
 check_equals(s.lastIndexOf('s'), 2);
#else
 check_equals(s.lastIndexOf('s'), 3);
#endif


// Applied to object.
o = new Object;
o.charCodeAt = String.prototype.charCodeAt;
o.charAt = String.prototype.charAt;
c = o.charAt(4);
check_equals(c, "e");
c = o.charCodeAt(4);
check_equals(c, "101");
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

// Applied to object.
o = new Object;
o.indexOf = String.prototype.indexOf;
p = o.indexOf("b");
check_equals(p, 2);

// UTF8 indexOf
s = "tést";
#if OUTPUT_VERSION > 5
 check_equals(s.indexOf('s'), 2);
#else
 check_equals(s.indexOf('s'), 3);
#endif

//----------------------------------------
// Check String.split
// See ASNative.as for more tests.
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
// For SWF6 and above the following condititions apply in
// this order:
// Full string returned in 1-element array:
// 1. If no arguments are passed.
// 2. If delimiter undefined.
// 3: empty string, non-empty delimiter.
//
// Empty array returned:
// 4. string and delimiter are empty but defined.
// 5. non-empty string, non-empty delimiter; 0 or less elements required.
//
// If the delimiter is empty, each character is placed in a separate element.
ret = a.split('');
check_equals(typeof(ret), 'object');
check(ret instanceof Array);
check_equals( ret.length, 20 );
check_equals ( ret[0], "w" );
check_equals ( ret[1], "a" );
check_equals ( ret[2], "l" );
check_equals ( ret[3], "l" );
check_equals ( ret[18], "O" );
check_equals ( ret[19], "N" );
check_equals ( a.split("la")[0], "wal" );
check_equals ( a.split("la")[1], "wal" );
check_equals ( a.split("la")[2], "washinGTON" );
check_equals ( a.split("la").length, 3 );

str = "h";
ar = str.split("h");
check_equals(ar.length, 2);
check_equals(ar.toString(), ",");

str = "";
ar = str.split("h");
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "hh";
ar = str.split("h");
check_equals(ar.length, 3);
check_equals(ar.toString(), ",,");

str = "h";
ar = str.split("g");
check_equals(ar.length, 1);
check_equals(ar.toString(), "h");

str = "a";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "a");

str = "b";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "b");

str = "a";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "a");

str = "aa";
ar = str.split("aa");
check_equals(ar.length, 2);
check_equals(ar.toString(), ",");

str = "";
ar = str.split("");
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "b";
ar = str.split("");
check_equals(ar.length, 1);
check_equals(ar.toString(), "b");

str = "aa";
ar = str.split("");
check_equals(ar.length, 2);
check_equals(ar.toString(), "a,a");

str = "";
ar = str.split();
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "b";
ar = str.split();
check_equals(ar.length, 1);
check_equals(ar.toString(), "b");

str = "aa";
ar = str.split();
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");


// Limit 0 or less:
str = "aa";
ar = str.split("", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "";
ar = str.split("x", 0);
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "";
ar = str.split("x", -1);
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "";
ar = str.split("", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "";
ar = str.split("", -1);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split("", -1);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split("aa", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split("aa", -1);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split(undefined, 0);
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");

str = "aa";
ar = str.split("a", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

// Limit undefined
str = "aa";
ar = str.split("aa", undefined);
check_equals(ar.length, 2);
check_equals(ar.toString(), ",");

// String methods on object.
o = new Object;
o.split = String.prototype.split;
ar = o.split("b");
check_equals(ar.length, 3);
check_equals(ar.toString(), "[o,ject O,ject]");

o = new Date(0);
o.split = String.prototype.split;
ar = o.split(":");
check_equals(ar.length, 3);


#else
// SWF5:

// empty delimiter doesn't have a special meaning in SWF5
check_equals ( a.split("")[0], "wallawallawashinGTON" );
check_equals ( a.split("")[19], undefined );
// multi-char delimiter doesn't work in SWF5
check_equals ( a.split("la")[0], "wallawallawashinGTON" );
check_equals ( a.split("la")[1], undefined );
check_equals ( a.split("la")[2], undefined );
check_equals ( a.split("la").length, 1 );

str = "h";
ar = str.split("h");
check_equals(ar.length, 2);
check_equals(ar.toString(), ",");

str = "";
ar = str.split("h");
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "hh";
ar = str.split("h");
check_equals(ar.length, 3);
check_equals(ar.toString(), ",,");

str = "h";
ar = str.split("g");
check_equals(ar.length, 1);
check_equals(ar.toString(), "h");

// For SWF5, the following conditions mean that an array with a single
// element containing the entire string is returned:
// 1. No arguments are passed.
// 2. The delimiter is empty.
// 3. The delimiter has more than one character or is undefined and limit is not 0.
// 4. The delimiter is not present in the string and the limit is not 0.
//
// Accordingly, an empty array is returned only when the limit is less
// than 0 and a non-empty delimiter is passed.str = "";
str = "a";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "a");

str = "b";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "b");

str = "a";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "a");

str = "aa";
ar = str.split("aa");
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");

str = "";
ar = str.split("");
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "b";
ar = str.split("");
check_equals(ar.length, 1);
check_equals(ar.toString(), "b");

str = "aa";
ar = str.split("");
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");

str = "";
ar = str.split();
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "b";
ar = str.split();
check_equals(ar.length, 1);
check_equals(ar.toString(), "b");

str = "aa";
ar = str.split();
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");


// Limit 0 or less:
str = "aa";
ar = str.split("", 0);
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");

str = "aa";
ar = str.split("", -1);
check_equals(ar.length, 1);
check_equals(ar.toString(), "aa");

str = "";
ar = str.split("x", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "";
ar = str.split("x", -1);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "";
ar = str.split("", 0);
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "";
ar = str.split("", -1);
check_equals(ar.length, 1);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split("aa", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split("aa", -1);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split(undefined, 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

str = "aa";
ar = str.split("a", 0);
check_equals(ar.length, 0);
check_equals(ar.toString(), "");

// Limit undefined
str = "aa";
ar = str.split("a", undefined);
check_equals(ar.length, 3);
check_equals(ar.toString(), ",,");

o = new Object;
o.split = String.prototype.split;
ar = o.split("b");
check_equals(ar.length, 3);
check_equals(ar.toString(), "[o,ject O,ject]");

o = new Date(0);
o.split = String.prototype.split;
ar = o.split(":");
check_equals(ar.length, 3);
// It will be different according to the timezone...

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

st = "";
g = st.split("", 0);
#if OUTPUT_VERSION > 5
check_equals(g.length, 0);
check_equals(typeof(g[0]), "undefined");
check_equals(g[0], undefined);
#else
check_equals(g.length, 1);
check_equals(typeof(g[0]), "string");
check_equals(g[0], "");
#endif

st = "";
g = st.split("x", 0);
#if OUTPUT_VERSION > 5
check_equals(g.length, 1);
check_equals(typeof(g[0]), "string");
check_equals(g[0], "");
#else
check_equals(g.length, 0);
check_equals(typeof(g[0]), "undefined");
check_equals(g[0], undefined);
#endif

st = "";
g = st.split("x", 1);
check_equals(g.length, 1);
check_equals(typeof(g[0]), "string");
check_equals(g[0], "");

st = "f";
g = st.split("", 0);
#if OUTPUT_VERSION > 5
check_equals(g.length, 0);
check_equals(typeof(g[0]), "undefined");
check_equals(g[0], undefined);
#else
check_equals(g.length, 1);
check_equals(typeof(g[0]), "string");
check_equals(g[0], "f");
#endif


st = "f";
g = st.split("x", 0);
check_equals(g.length, 0);
check_equals(typeof(g[0]), "undefined");
check_equals(g[0], undefined);

st = "f";
g = st.split("x", 1);
check_equals(g.length, 1);
check_equals(typeof(g[0]), "string");
check_equals(g[0], "f");


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

o = new Object;
o.substr = String.prototype.substr;
check_equals(o.substr(0,2), "[o");

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

o = new Object;
o.slice = String.prototype.slice;
check_equals(o.slice(0,1), "[");

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

o = new Object;
o.substring = String.prototype.substring;
check_equals(o.substring(3,4), "j");

//-------------------------------------------
// Concat
//-------------------------------------------

// Fails if object isn't a String (the only
// case of this, it seems).
e = new String("h");
e.concat("hh");
e.concat(new Object);
e.concat(new String);
check_equals(e, "h");

o = new Object;
o.concat = String.prototype.concat;
o.concat("h");
check_equals(o.toString(), "[object Object]");

//-------------------------------------------
// Chr and ord
//-------------------------------------------

check_equals (chr(0), "");
check_equals (chr(65), "A");
check_equals (ord("A"), 65);
check_equals (ord(""), 0);

// Chars greater than 128
#if OUTPUT_VERSION > 5
check_equals (chr(246), "ö");
check_equals (chr(486), "Ǧ");
check_equals (chr(998), "Ϧ");
check_equals (ord("ö"), 246);
check_equals (ord("Ϧ"), 998);
#else // version <= 5
check_equals (typeof(chr(486)), 'string');
check_equals (chr(865), "a");
check_equals (ord("ö"), 195);
check_equals (ord("Ö"), 195);
check_equals (ord("ǵ"), 199);
check_equals (ord("Ϧ"), 207);
#endif

//-------------------------------------------
// Mbchr and mbord
//-------------------------------------------
#ifdef MING_SUPPORTS_ASM

// All versions, especially 5:
var c;

i = "Ǧ";

asm {
    push "c"   
    push "i"   
    getvariable
    mbord  
    setvariable
};

#if OUTPUT_VERSION > 5
check_equals (c, 486);
#else
xcheck_equals (c, 199);
#endif

i = "Ϧ";

asm {
    push "c"   
    push "i"   
    getvariable
    mbord  
    setvariable
};

#if OUTPUT_VERSION > 5
check_equals (c, 998);
#else
xcheck_equals (c, 207);
#endif

// And the reverse procedure:

i = 998;

asm {
    push "c"   
    push "i"   
    getvariable
    mbchr  
    setvariable
};

#if OUTPUT_VERSION > 5
check_equals (c, "Ϧ");
#else
check_equals (typeof(c), "string"); 
// c == "" fails, but when displayed it evaluates to the empty string
#endif

// Should return the same as mbchr(90000 - 65536) 

i = 90000;

asm {
    push "c"   
    push "i"   
    getvariable
    mbchr  
    setvariable
};

#if OUTPUT_VERSION > 5
check_equals (c, "徐");
#else
check_equals (typeof(c), "string");
// c == "" fails, but when displayed it evaluates to the empty string
#endif

#endif //MING_SUPPORTS_ASM 
//-------------------------------------------
// Check multi-byte chars with all string
// functions
//-------------------------------------------

// These tests are only correct with SWF6 and above.

var a = new String("Längere Wörter");

#if OUTPUT_VERSION > 5
check_equals (a.length, 14);
check_equals (a.substring(2,4), "ng");
check_equals (a.charAt(1), "ä");
check_equals (a.charAt(2), "n");
check_equals (a.slice(3,5), "ge");
check_equals (a.charCodeAt(9), 246);
#else
check_equals (a.length, 16);
check_equals (a.slice(3,5), "ng");
check_equals (a.charCodeAt(10), 195);
#endif

//-----------------------------------------------------------
// Test SWFACTION_SUBSTRING
//-----------------------------------------------------------

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

asm {
	push "b"
	push "f"
	push "1" 
	push "1" 
	substring
	setvariable
};
check_equals( b, "f");

asm {
	push "b"
	push ""
	push "0" 
	push "1" 
	substring
	setvariable
};
check_equals( b, "");

teststr = "Heöllo";
count1 = 0;
count2 = 0;

for (i = -5; i < 10; i++)
{
    for (j = -5; j < 10; j++)
    {
        asm {
            push "a"
            push "teststr"
            getvariable
            push "i"
            getvariable
            push "j"
            getvariable
            substring
            setvariable
        };
        
        b = teststr.substr( i >= 1 ? i - 1 : 0, j >= 0 ? j : teststr.length);

        // Test for undefined.
        c = teststr.substr( i >= 1 ? i - 1 : 0, j >= 0 ? j : teststr.undef());
        
        // There are 225 tests
        if (a == b) count1++;
        else note(i + " : " + j + " -- " + a + ":" + b);

        if (b == c) count2++;

    }
}

check_equals (count1, 225); // String.substr / substring consistency
check_equals (count2, 225); // undefined value same as no value passed (or length of string)

#endif

//-----------------------------------------------------------
// Test SWFACTION_MBSUBSTRING
//-----------------------------------------------------------

// see check.as
#ifdef MING_SUPPORTS_ASM

asm {
	push "b"
	push "ciao"
	push "2"
	push "10" // size is bigger then string length,
	          // we expect the interpreter to adjust it
	mbsubstring
	setvariable
};
check_equals( b, "iao");
asm {
	push "b"
	push "boowa"
	push "2"
	push "-1" // size is bigger then string length,
	          // we expect the interpreter to adjust it
	mbsubstring
	setvariable
};
check_equals( b, "oowa");
asm {
	push "b"
	push "ciao"
	push "-2" // negative base should be interpreted as 1
	push "1" 
	mbsubstring
	setvariable
};
check_equals( b, "c");
asm {
	push "b"
	push "ciao"
	push "-2" // negative base should be interpreted as 1
	push "10" // long size reduced 
	mbsubstring
	setvariable
};
check_equals( b, "ciao");
asm {
	push "b"
	push "ciao"
	push "0" // zero base is invalid, but taken as 1
	push "1" 
	mbsubstring
	setvariable
};
check_equals( b, "c");
asm {
	push "b"
	push "ciao"
	push "10" // too large base ...
	push "1" 
	mbsubstring
	setvariable
};
check_equals( b, "");
asm {
	push "b"
	push "all"
	push "3" // base is 1-based!
	push "1" 
	mbsubstring
	setvariable
};
check_equals( b, "l");

asm {
	push "b"
	push "f"
	push "1" 
	push "1" 
	mbsubstring
	setvariable
};
check_equals( b, "f");

#endif

//-----------------------------------------------------------
// Test inheritance with built-in functions
//-----------------------------------------------------------

var stringInstance = new String();
check (stringInstance.__proto__ != undefined);
#if OUTPUT_VERSION > 5
check_equals("" + stringInstance.__proto__.valueOf, "[type Function]");
#else
check_equals("" + stringInstance.__proto__.valueOf, "");
#endif

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
ASSetPropFlags(String.prototype, "toString", 0, 2); // unprotect from deletion
StringProtoToStringBackup = String.prototype.toString;
check(delete String.prototype.toString);
check_equals(typeof(s.toString), 'function');
check(!delete Object.prototype.toString);
ASSetPropFlags(Object.prototype, "toString", 0, 2); // unprotect from deletion
ObjectProtoToStringBackup = Object.prototype.toString;
check(delete Object.prototype.toString);
check_equals(typeof(s.toString), 'undefined');

String.prototype.toString = function ()
{
    return "fake to string";
};

Object.prototype.toString = function ()
{
    return "fake object";
};

g = "teststring";
check_equals(g+' ', "teststring ");
check_equals (g.substr(0,4), "test");
g = new String("teststring");
check_equals (g.substr(0,4), "test");

o = new Object;
check_equals(o.substr(0,4), undefined);
o.substr = String.prototype.substr;
check_equals(o.substr(0,4), "fake");

Object.prototype.toString = ObjectProtoToStringBackup;
String.prototype.toString = StringProtoToStringBackup;

//----------------------------------------------------------------------
// Test concatenation of string objects
//----------------------------------------------------------------------

s = new String("hello");
r = "s:"+s;
check_equals(r, "s:hello");

s = new String("");
r = "s:"+s;
check_equals(r, "s:");

//----------------------------------------------------------------------
// Test the 'length' property
//----------------------------------------------------------------------

a = "123";
check_equals(a.length, 3);
a.length = 2;
check_equals(a.length, 3); // well, it's a string after all, not an object
a = new String("123");
check_equals(a.length, 3);
a.length = 2;
check_equals(a.length, 2); // can override
check_equals(a, "123"); // not changing the actual string
a.length = "another string";
check_equals(a.length, "another string"); // can also be of a different type
delete a["length"];
check_equals(a.length, "another string"); // can't be deleted
#if OUTPUT_VERSION > 5
 check(a.hasOwnProperty('length'));
#endif

//----------------------------------------------------------------------
// Test that __proto__ is only hidden, but still existing , in SWF5
//----------------------------------------------------------------------

a=new Array(); for (v in String) a.push(v); a.sort();
#if OUTPUT_VERSION > 5
 check_equals(a.toString(), "toString");
#else
 check_equals(a.length, 0);
#endif

ASSetPropFlags(String, "__proto__", 0, 128); // unhide String.__proto__

a=new Array(); for (v in String) a.push(v); a.sort();
check_equals(a.toString(), "toString"); 

check_equals(typeof(String.__proto__), 'object'); 
check_equals(typeof(Object.prototype), 'object');
Object.prototype.gotcha = 1;
check_equals(String.gotcha, 1);
ASSetPropFlags(Object.prototype, "hasOwnProperty", 0, 128); // unhide Object.prototype.hasOwnProperty
check(!String.__proto__.hasOwnProperty("gotcha"));
check(String.__proto__.__proto__.hasOwnProperty("gotcha")); // function
check_equals(String.__proto__.__proto__, Object.prototype);  // hasOwnProperty doesn't exist in gnash !

a=new Array(); for (v in String) a.push(v); a.sort();
check_equals(a.toString(), "gotcha,toString"); 

#if OUTPUT_VERSION == 5
// This here to avoid changing SWF5 String properties
// before testing them.

String.prototype.hasOwnProperty = ASnative(101, 5);
String.hasOwnProperty = ASnative(101, 5);

check(!String.hasOwnProperty('toString'));
check(!String.hasOwnProperty('valueOf'));
check(String.hasOwnProperty('__proto__'));
check(String.hasOwnProperty('fromCharCode'));

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
check(!String.prototype.hasOwnProperty('length'));

#endif

//----------------------------------------------------------------------
// Test lifetime of temporary objects created from string
//----------------------------------------------------------------------

a='string';
String.prototype.saveMe = function(saved) { saved.value=this; };
saved1={}; a.saveMe(saved1);
check_equals(typeof(saved1.value), 'object');
check_equals(saved1.value, 'string');
a='another string';
check_equals(saved1.value, 'string');
saved2={}; a.saveMe(saved2);
check_equals(saved1.value, 'string');
check_equals(saved2.value, 'another string');
// Test that the syntethized object return is always a new one!
saved2.value.prop = 6;
saved3 = {}; a.saveMe(saved3); 
check(saved2.value.prop != saved3.value.prop);
check(saved2.value !== saved3.value);

//----------------------------------------------------------------------
// Test that objects created from string literals will be constructed
// by user-defined _global.String, if defined 
//----------------------------------------------------------------------

// _global isn't accessible in SWF5
#if OUTPUT_VERSION > 5

OrigString = _global.String;

a='a string';
_global.String = function() { this.id = 'wonder1'; };
check_equals(a.id, 'wonder1'); 
_global.String = function() { this.id = 'wonder2'; };
check_equals(a.id, 'wonder2'); 
_global.String = function() { this.id = 'wonder3'; };
check_equals(a.id, 'wonder3'); 

_global.String = OrigString;

#endif // OUTPUT_VERISION > 5

//----- END OF TESTS

var baseTests = 329;
var asmTests = 23;
var ge6Tests = 19;

var totalTests = baseTests;
#ifdef MING_SUPPORTS_ASM
 totalTests += asmTests;
#endif

#if OUTPUT_VERSION >= 6
 totalTests += ge6Tests;
#endif

check_totals(totalTests);
