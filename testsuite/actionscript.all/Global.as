// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
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


// Test case for ActionScript _global Object
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Global.as";
#include "check.as"

#if OUTPUT_VERSION < 6
 check_equals(typeof(_global), "undefined");
#endif

#if OUTPUT_VERSION == 6
 check_equals(typeof(_gLobal), "object");
 check_equals(typeof(_gLobal), "object");
 check_equals(typeof(_GLOBAL), "object");
 check_equals(_GLOBAL, _global);
 check_equals(_GlOBAL, _global);
#endif

#if OUTPUT_VERSION < 7
 check_equals(this.toString(), "[object Object]");
 check_equals(thIs.toString(), "[object Object]");
 check_equals("" + this, "_level0");
 check_equals(this, This);
 check_equals(this, THIS);
#endif

check_equals(typeof(Button), 'function'); // random check

// Check that CustomActions isn't recognized by the player.
check_equals(typeof(_global.CustomActions), "undefined");

#if OUTPUT_VERSION > 5
check_equals(typeof(_global.updateAfterEvent), 'function');
check( ! _global.hasOwnProperty('updateAfterEvent') );
check( ! _global.__proto__.hasOwnProperty('updateAfterEvent') );

// Check that _global.parseInt is in effect what parseInt resolves to
check ( parseInt == _global.parseInt );

check_equals(typeof(Object._global), 'undefined');
check_equals(typeof(Object.prototype._global), 'undefined');
check_equals(typeof(_global._global), 'undefined');
check_equals(typeof(this.__proto__._global), 'undefined');

// odd stuff.. not every value of type 'object' is an instance of Object :!!
check_equals(typeof(_global), 'object');
check_equals(typeof(_global._global), 'undefined');
check( ! _global instanceof Object );
check_equals( typeof(_global.__proto__), 'undefined' );
check_equals(typeof(_global.toString), 'undefined');

#else

check_equals(typeof(updateAfterEvent), 'function');
check_equals ( typeof(_global.parseInt), 'undefined' );

#endif

check_equals(typeof(isNaN), 'function');
#if OUTPUT_VERSION > 5
check(!_global.hasOwnProperty('isNaN'));
#endif

check_equals(typeof(ASnative), 'function');
check_equals(typeof(ASconstructor), 'function');
check_equals(typeof(ASSetNative), 'function');
check_equals(typeof(ASSetNativeAccessor), 'function');

// Test parseInt
check_equals ( parseInt('45b'), 45 );
check_equals ( parseInt('65'), 65 );
check_equals ( parseInt('-1234'), -1234 );
check_equals ( parseInt('-1.234'), -1 );
check_equals ( parseInt('        -1234'), -1234 );
check_equals ( parseInt('          +12.34'), 12 );
check_equals ( parseInt("           234"), 234 );
check ( isNaN(parseInt('++3')));

// Test parseint with hex
// No whitespace allowed. Sign must come after0x
check ( parseInt('0x111') == 273 );
check ( isNaN(parseInt('0xw')));
check ( isNaN(parseInt('-0x111')));
check ( isNaN(parseInt('+0x111')));
check ( parseInt(' 0x111') == 0 );
check ( parseInt('0x-111') == -273 );
check ( parseInt('0x+111') == 273 );
check ( parseInt('0X-111') == -273 );
check ( parseInt('0X+111') == 273 );

// Test parseint with octal
check_equals (parseInt('0352'), 234 );
check_equals (parseInt('-0352'), -234);
check_equals (parseInt('+0352'), 234);
// Evidently only numbers with no whitespace in front and
// no digits higher than 7 are octal. These all decimal:
check_equals (parseInt('07658'), 7658);
check_equals (parseInt('   0352'), 352 );
check_equals (parseInt('        -0352'), -352);
check_equals (parseInt('03529A'), 3529);
check_equals (parseInt('0352A'), 352);
check_equals (parseInt('0352 '), 352);
// Test parseint with 36 base
check ( parseInt('2GA',36) == (10+16*36+2*36*36) );
// Test parseint with base 17 - the 'H' is not part of base 17, only the first two digits are valid
check ( parseInt('FGH',17) == (16+17*15) );
check ( parseInt('513x51') == 513 );
check ( isNaN(parseInt('a1023')) );
check ( isNaN(parseInt('zero')) );
// parseInt returns NaN (which is different from infinity)
check ( ! isFinite(parseInt('none')) );
check ( ! isFinite(1/0) );
check ( ! isNaN(1/0) );
check_equals (parseInt(new String("10")), 10);
o = {}; o.toString = function() { return "12"; };
check_equals (parseInt(o), 12);

check(isNaN(parseInt("8589934592", 5)));


// Misc
check_equals(parseInt("8589934592", 16), 573538780562);
check_equals(parseInt("800000000", 36), 22568879259648);
check_equals(parseInt(" 6 7 8", 8), 6);
check_equals(parseInt("0x123", 8), 83);
check_equals(parseInt(" 0x123", 8), 0);

// It's not reliable to compare a double type with ==, so we'll give it a
// small range using >= and <=
check ( isNaN(parseFloat('test')) );
check ( parseFloat('1.5') >= 1.499 && parseFloat('1.5') <= 1.501 );
check ( parseFloat('   	    -2001.5') >= -2001.51 && parseFloat('   	    -2001.5') <= -2001.49 );
check ( parseFloat('		 5.13123abc2.35387') >= 5.1312 && parseFloat('		 5.13123abc2.35387') <= 5.1313 );
check ( isNaN(parseFloat('         x1.234')) );
check ( isNaN(parseFloat('')) );

check ( ! isNaN (parseFloat('		 5.13123abc2.35387')));
check ( parseFloat('3.45e-5') >= 3.449e-5 && parseFloat('3.45e-5') <= 3.451e-5);
check ( parseFloat('3.45E-5') >= 3.449e-5 && parseFloat('3.45E-5') <= 3.451e-5);
check ( parseFloat('3.45eE-5') >= 3.449 && parseFloat('3.45eE-5') <= 3.451);

// All %NN must become the corresponding ascii char
check_equals ( unescape('%3A%2F%3F%3D%26'), ':/?=&' );
check_equals ( unescape('%3a%2f%3f%3d%26'), ':/?=&' );
#if OUTPUT_VERSION == 5
xcheck_equals ( unescape('%3a%2f%3f%3d%26%'), ':/?=&387' ); // SWF5
#else
// TODO: check output in SWF6 and higher
// (possibly UTF8, player LNX 7,0,25,0 reveals a memory corruption)
xcheck_equals ( unescape('%3a%2f%3f%3d%26%'), ':/?=&' ); // SWF6
#endif

#if OUTPUT_VERSION == 5
xcheck_equals ( unescape('%3a%2f%3f%3d%26%2'), ':/?=&87' ); // SWF5
#else
// TODO: check output in SWF6 and higher
// (possibly UTF8, player LNX 7,0,25,0 reveals a memory corruption)
#endif

// All URL-special chars become the corresponding %NN hex
check_equals (escape(' "#$%&+,/:;<='), '%20%22%23%24%25%26%2B%2C%2F%3A%3B%3C%3D');
check_equals (escape('>?@[\\]^`{|}~'), '%3E%3F%40%5B%5C%5D%5E%60%7B%7C%7D%7E');
//check_equals (escape('!()*-._0123456789'), '!()*-._0123456789');
xcheck_equals (escape('!()*-._0123456789'), '%21%28%29%2A%2D%2E%5F0123456789'); // SWF5
check_equals (escape('ABCDEFGHIJKLMNOPQRSTUVWXYZ'), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ');
check_equals (escape('abcdefghijklmnopqrstuvwxyz'), 'abcdefghijklmnopqrstuvwxyz');

// How to test failure of setInterval and success of clearInterval ?
// The problem is that there is no way 
// to run an interval before a frame is executed
// so this will require onEnterFrame to work.
// We don't want to test onEnterFrame here, do we ?
check_equals(typeof(setInterval), 'function');
check_equals(typeof(clearInterval), 'function');

ret = clearInterval(); // gnash used to crash on this...
check_equals(typeof(ret), 'undefined');

check_equals(typeof(setTimeout), 'function');
check_equals(typeof(clearTimeout), 'function');

ret = clearTimeout(); 
check_equals(typeof(ret), 'undefined');



//------------------------------------------------------------
// Test ASSetPropFlags
//------------------------------------------------------------

function get() { return this.s; }
function set() { this.s++; }
function get2() { return this.s2; }
function set2() { this.s2++; }

#if OUTPUT_VERSION == 5

	a = { m:1 }; 
	ASsetPropFlags(a, "m", 128);
	check_equals(a.m, undefined);
	a.m=2; // will set a own property m
	check_equals(a.m, 2); // ignore flag was cleared

	a = { m:1 }; 
	ASsetPropFlags(a, "m", 256);
	check_equals(a.m, 1);
	a.m=2; // will set a own property m
	check_equals(a.m, 2); // ignore flag was cleared

	a = { m:1 }; 
	ASsetPropFlags(a, "m", 1024);
	check_equals(a.m, undefined);
	a.m=2; // will set a own property m
	check_equals(a.m, 2); // ignore flag was cleared

	a = { m:1 }; 
	ASsetPropFlags(a, "m", 4096);
	check_equals(a.m, undefined);
	a.m=2; // will set a own property m
	check_equals(a.m, 2); // ignore flag was cleared

#endif // OUTPUT_VERSION == 5

#if OUTPUT_VERSION == 6

	c = {}; c.addProperty("m", get2, set2);
	b = {}; b.addProperty("m", get, set); b.__proto__ = c;
	a = { m:1 }; a.__proto__ = b; a.s = 9; a.s2 = 99;
	check_equals(a.m, 1);
	ASsetPropFlags(a, "m", 256);
	check_equals(a.m, 9);
	a.m=2; // won't call setter, but set a own property m and clear ignore flag
	check_equals(a.s, 9); // setter wasn't called
	check_equals(a.m, 2); // ignore flag was cleared
	ASsetPropFlags(a, "m", 256);
	check_equals(a.m, 9); // a own property was set instead
	check(delete a.m); // delete a.m
	ASsetPropFlags(b, "m", 256); // make b.m invisible
	check_equals(a.m, 99); // b.m invisible, a.m non-existent
	a.m=3; // will call b.m setter, even if invisible 
	xcheck_equals(a.s, 10); // b.m setter was called
	xcheck_equals(a.m, 99); // ignore flag on inherited property was NOT cleared
	ASsetPropFlags(b, "m", 0, 256); // make b.m visible again
	xcheck_equals(a.m, 10); // ignore flag was cleared

	c = {}; c.addProperty("m", get2, set2);
	b = {}; b.addProperty("m", get, set); b.__proto__ = c;
	a = { m:1 }; a.__proto__ = b; a.s = 9; a.s2 = 99;
	check_equals(a.m, 1);
	ASsetPropFlags(a, "m", 1024);
	check_equals(a.m, 9);
	a.m=2; // won't call setter, but set a own property m
	check_equals(a.s, 9); // setter wasn't called
	check_equals(a.m, 9); // ignore flag wasn't cleared
	ASsetPropFlags(a, "m", 0, 1024);
	check_equals(a.m, 2); // a own property was set instead
	check(delete a.m); // delete a.m
	ASsetPropFlags(b, "m", 1024); // make b.m invisible
	check_equals(a.m, 99); // b.m invisible, a.m non-existent
	a.m=3; // will call b.m setter, even if invisible 
	xcheck_equals(a.s, 10); // b.m setter was called
	xcheck_equals(a.m, 99); // ignore flag on inherited property was NOT cleared
	ASsetPropFlags(b, "m", 0, 1024); // make b.m visible again
	xcheck_equals(a.m, 10); // ignore flag was cleared

	c = {}; c.addProperty("m", get2, set2);
	b = {}; b.addProperty("m", get, set); b.__proto__ = c;
	a = { m:1 }; a.__proto__ = b; a.s = 9; a.s2 = 99;
	check_equals(a.m, 1);
	ASsetPropFlags(a, "m", 4096);
	check_equals(a.m, 9);
	a.m=2; // won't call setter, but set a own property m
	check_equals(a.s, 9); // setter wasn't called
	check_equals(a.m, 2); // ignore flag was cleared
	ASsetPropFlags(a, "m", 0, 4096);
	check_equals(a.m, 2); // a own property was set instead
	check(delete a.m); // delete a.m
	ASsetPropFlags(b, "m", 4096); // make b.m invisible
	check_equals(a.m, 99); // b.m invisible, a.m non-existent
	a.m=3; // will call c.m setter, skipping invisible b.m one
	check_equals(a.s, 9); // b.m setter was NOT called
	check_equals(a.s2, 100); // c.m setter was called
	check_equals(a.m, 100); // ignore flag on b.m getter-setter was NOT cleared
	ASsetPropFlags(b, "m", 0, 4096); // make b.m visible again
	check_equals(a.m, 9); // ignore flag on b.m was NOT cleared

#endif // OUTPUT_VERSION == 6

#if OUTPUT_VERSION == 7

	c = {}; c.addProperty("m", get2, set2);
	b = {}; b.addProperty("m", get, set); b.__proto__ = c; b.s = 8; b.s2 = 88;
	a = { m:1 }; a.__proto__ = b; a.s = 9; a.s2 = 99;
	check_equals(a.m, 1);
	ASSetPropFlags(a, "m", 4096);
	check_equals(a.m, 9); 
	a.m=2; // won't call setter, but set a own property m
	check_equals(a.s, 9); // setter wasn't called
	check_equals(a.m, 2); // ignore flag was cleared
	ASSetPropFlags(a, "m", 0, 4096);
	check_equals(a.m, 2); // a own property was set instead
	check(delete a.m); // delete a.m
	check_equals(b.m, 8); // ???
	ASsetPropFlags(b, "m", 4096); // make b.m invisible (no-op in SWF7 ?)
	check_equals(b.m, 8); // b.m getter, altought invisible, is still invoked as a getter
	check_equals(a.m, 9); // b.m, altought invisible, is still invoked as a getter
	a.m=3; // ??
	check_equals(a.s, 10); // b.m, altought invisible, is still invoked as a setter
	check_equals(a.s2, 99); // c.m is never reached (b.m was called)
	check_equals(a.m, 10); // b.m getter, altoguth invisible, is used 
	ASsetPropFlags(b, "m", 0, 4096); // make b.m visible again (looks like it wasn't really invisible before)
	check_equals(a.m, 10); // ??

#endif // OUTPUT_VERSION == 7


o = {};
var tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 0);

o.a = 1;
o.b = 2;
o.c = 3;
tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 3);

// ASSetPropFlags passing an array of property names

ret = ASSetPropFlags(o, ["b"], 1); // an array of prop names
check_equals(typeof(ret), 'undefined');
tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 2);
tmp.sort();
check_equals(tmp[0], 'a');
check_equals(tmp[1], 'c');

// ASSetPropFlags passing an non-array object for property names 
// (invalid)

ret = ASSetPropFlags(o, {c:2}, 1); // an object is not valid, must be an array
check_equals(typeof(ret), 'undefined');
tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 2);
tmp.sort();
check_equals(tmp[0], 'a');
check_equals(tmp[1], 'c');

// ASSetPropFlags passing an array-like object for property names 
// (still invalid)

p = {};
p.length = 1;
p['0'] = 'c';
ret = ASSetPropFlags(o, p, 1); 
check_equals(typeof(ret), 'undefined');
tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 2);
tmp.sort();
check_equals(tmp[0], 'a');
check_equals(tmp[1], 'c');

// ASSetPropFlags passing undefined for property names 
// (still invalid)

ret = ASSetPropFlags(o, undefined, 1); 
check_equals(typeof(ret), 'undefined');
tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 2);

// ASSetPropFlags passing null for property names 
// (still invalid)

ret = ASSetPropFlags(o, null, 1); 
check_equals(typeof(ret), 'undefined');
tmp = new Array;
for (var i in o) tmp.push(i);
check_equals(tmp.length, 0);

// ASSetPropFlags passing a string and 0,0 true/false sets

o = {a:1, b:2, c:3};
ASSetPropFlags(o, "b", 1, 0); 
tmp = new Array; for (var i in o) tmp.push(i);
check_equals(tmp.length, 2);

ASSetPropFlags(o, ['c'], 0, 0);  // 0,0 has no special meaning
tmp = new Array; for (var i in o) tmp.push(i);
check_equals(tmp.length, 2);


/// Tests for int

/// non-numeric
check_equals (int(undefined), 0);
check_equals (int("string"), 0);
check_equals (int("infinity"), 0);

/// Floats
check_equals (int(1.5), 1);
check_equals (int(-1.e-15), 0);
check_equals (int(-0.99999999999999), 0);
check_equals (int(0.99999999999), 0);
check_equals (int(-7.8), -7);
check_equals (int(6.1), 6);
check_equals (int("-7.8"), -7);
check_equals (int("6.1"), 6);
check_equals (int("      -7.8"), -7);

/// Integer values
check_equals (int(0), 0);
check_equals (int(0xffffffff), -1);
check_equals (int(2147483648), -2147483648);
check_equals (int(-2147483648), -2147483648);
check_equals (int(2147483649), -2147483647);
check_equals (int(-2147483649), 2147483647);
check_equals (int(4294967296), 0);
check_equals (int(-4294967296), 0);
check_equals (int(infinity), 0);
check_equals (int("2147483649"), -2147483647);
check_equals (int("-2147483649"), 2147483647);
check_equals (int("4294967296"), 0);
check_equals (int("-4294967296"), 0);
check_equals(int("1e+45"), 0);
check_equals(int("1e+5"), 100000);
check_equals(int("1e+10"), 1410065408);
check_equals(int("1e+12"), -727379968);
check_equals(int("1e+14"), 276447232);
check_equals(int("1.4e+7"), 14000000);

/// Octal (or not)
#if OUTPUT_VERSION < 6
check_equals(int("0123"), 123);
check_equals(int("-0123"), -123);
#else
check_equals(int("0123"), 83);
check_equals(int("-0123"), -83);
#endif
check_equals(int("   0123"), 123);
check_equals(int("-   0123"), 0);
check_equals(int("   0-123"), 0);
check_equals(int("01238"), 1238);
check_equals (int("0123.6"), 123);

/// Hex (or not)
check_equals(int("-0x10"), 0);
#if OUTPUT_VERSION < 6
check_equals(int("0x-10"), 0);
check_equals(int("0X+10"), 0);
#else
check_equals(int("0x-10"), -16);
check_equals(int("0X+10"), 16);
#endif

/// Extraneous characters
check_equals (int("0123r"), 0);
check_equals (int("0123&"), 0);
check_equals (int("   0123r"), 0);
check_equals (int("-6.1     "), 0);
check_equals (int("6,1"), 0);
check_equals (int("-7,8"), 0);
check_equals (int("7 "), 0);
check_equals (int("0x10 "), 0);
check_equals (int("0x10.8"), 0);
check_equals (int("0123.6 "), 0);
check_equals (int("0x-7.8 "), 0);
//------------------------------------------------------------
// END OF TEST
//------------------------------------------------------------

// Test _global.ASSetNative

o = new Object();
ASSetNative(o, 103, "q, w, e,6z,6u,7u, 7i, t,z,u");
r = "";
for (i in o) { r+=i + ","; };
check_equals(r, " t, 7i,u,z, e, w,q,");

// Check that toString() is called.
var o = new Object ();
var a = ["i", "u"];
a.toString = function() { trace ("toString called"); return "o, j"; };
ASSetNative (o, 200, a, 10);

r = "";
for (i in o) { r += i + ","; };
// It's not what's in the array (i, u), but what's returned by its toString
// method ("o,j");
check_equals(r, " j,o,");

// The function is not dependent on the present of the ASnative function.
ASnative = 56;
o = new Object();
ASSetNative (o, 200, "k,j,l", 10);

r = "";
for (i in o) { r += i + ","; };
check_equals(r, "l,j,k,");


#if OUTPUT_VERSION == 5
	check_totals(165); // SWF5
#else
# if OUTPUT_VERSION == 6
	check_totals(203); // SWF6
# else
#  if OUTPUT_VERSION == 7
	check_totals(175); // SWF7
#  else
	check_totals(162); // SWF8+
#  endif
# endif
#endif
