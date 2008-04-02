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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for Number ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

//
// TODO: have ming output ACTION_EQUAL
//       it seems it will only do it for SWF4 output
//
// TODO: test with SWF target != 6 (the only one tested so far)
//  


rcsid="$Id: Number.as,v 1.53 2008/04/02 10:55:32 strk Exp $";
#include "check.as"

Number.hasOwnProperty = ASnative(101, 5);

check(Number.hasOwnProperty('MAX_VALUE'));
check_equals(typeof(Number.MAX_VALUE), "number");
// gnash fails in rounding (ends in 2316, we round up to 232)
// TODO: check that we're close, if not extremely accurate
xcheck_equals(Number.MAX_VALUE.toString(), "1.79769313486231e+308");

check(Number.hasOwnProperty('MIN_VALUE'));
check_equals(typeof(Number.MIN_VALUE), "number");
check_equals(Number.MIN_VALUE.toString(), "4.94065645841247e-324"); 

check(Number.hasOwnProperty('NEGATIVE_INFINITY'));
check_equals(Number.NEGATIVE_INFINITY, -Infinity);

check(Number.hasOwnProperty('NaN'));
check_equals(Number.NaN, NaN);

check(Number.hasOwnProperty('POSITIVE_INFINITY'));
check_equals(Number.POSITIVE_INFINITY, Infinity);

var n1=new Number(268);
check_equals(typeof(n1), 'object');
var n1prim = Number(268);
check_equals(typeof(n1prim), 'number');
// gnash used to fail below because it compares 2 objects
// rather then an object and a primitive
check_equals(n1, n1prim);

// strict-equality operator was introduced in SWF6
#if OUTPUT_VERSION > 5
check ( ! (n1 === 268) );
// They are not the same object !
check ( ! (n1 === Number(268)) );
#endif

// but they have the same numeric value
check_equals (n1 , 268 );
check_equals (268 , n1 );

//------------------------------------
// Test Number.toString 
//------------------------------------

check_equals(typeof(n1.toString), "function");
check_equals(typeof(n1.toString()), "string"); 
check_equals(n1.toString(), "268");

var backup = Object.prototype.toString;
Object.prototype.toString = function() { return "fake_string"; };
check_equals(n1.toString(), "268"); // doesn't inherit from Object
Object.prototype.toString = backup;

#if OUTPUT_VERSION >= 6
check(Number.prototype.hasOwnProperty('toString'));
#endif

tmp = new Number(10);
check_equals(tmp.toString(2), '1010'); 
tmp = 6;
check_equals(tmp.toString(2), '110'); 
check_equals(tmp.toString(3), '20'); 
check_equals(tmp.toString(8), '6'); 
check_equals(tmp.toString(-2), '6'); // invalid, returns 10 ?
check_equals(tmp.toString(0), '6'); // invalid, returns 10 ?
check_equals(tmp.toString(5), '11'); 
tmp = -5;
check_equals(tmp.toString(2), '-101'); 
check_equals(tmp.toString(16), '-5'); 
tmp = -11;
check_equals(tmp.toString(16), '-b'); 

//------------------------------------
// Test Number.valueOf 
//------------------------------------

check_equals(typeof(n1.valueOf), "function");
check_equals(typeof(n1.__proto__.valueOf), "function");
check_equals(typeof(n1.__proto__.__proto__.valueOf), "function");
check_equals(typeof(n1.valueOf()), "number");
check_equals(n1.valueOf(), 268);

#if OUTPUT_VERSION >= 6
check(Number.prototype.hasOwnProperty('valueOf'));
check(Object.prototype.hasOwnProperty('valueOf'));
#endif

var backup = Object.prototype.valueOf;
Object.prototype.valueOf = function() { return "fake_value"; };
check_equals(n1.valueOf(), 268); // doesn't inherit from Object
Object.prototype.valueOf = backup;

backup = Number.prototype.valueOf;
Number.prototype.valueOf = function() { return "fake_value"; };
check_equals(n1.valueOf(), "fake_value"); // does inherit from Number
Number.prototype.valueOf = backup;

// Check unary minus operator
n1 = -n1;
check_equals (-268 , n1);
check_equals (n1.toString(), "-268");

//---------------------------------------
// Check NaN 
//---------------------------------------

check_equals( typeof(NaN), 'number' );
check_equals( typeof(isNaN), 'function' );
check_equals( typeof(isNaN(NaN)), 'boolean' );
check(NaN == NaN); 
check((0/2) == (0/5)); 
check(! (NaN != NaN) ); 
check( isNaN(NaN) );
check_equals( typeof(isNaN(0/0)), 'boolean' );
check( isNaN(0/0) );


#if OUTPUT_VERSION >= 6
check_equals( typeof(_global.NaN), 'number' );
check_equals( typeof(isNaN(_global.NaN)), 'boolean' );
check( isNaN(_global.NaN) ); // NOTE: isNaN(undefined) is true for SWF7 up
#else // SWF5 or below
check_equals( typeof(_global), 'undefined' );
check_equals( typeof(Object), 'function' );
check_equals( typeof(Object.prototype), 'object' );
check_equals( typeof(Object.prototype.NaN), 'undefined' );
#endif

#if OUTPUT_VERSION >= 7
check( isNaN(undefined) ); 
check( isNaN(null) );
check( isNaN(Object.prototype.NaN) );
#else // SWF6 or below
check( ! isNaN(undefined) );
check( ! isNaN(null) );
check( ! isNaN(Object.prototype.NaN) );
#endif

check(! Object.hasOwnProperty('NaN'));
check(! Object.prototype.hasOwnProperty('NaN'));
check(! this.__proto__.hasOwnProperty('NaN'));

//---------------------------------------
// Check Infinity
//---------------------------------------

check_equals( typeof(Infinity), 'number' );
check_equals( typeof(isFinite), 'function' );
check_equals( typeof(isFinite(Infinity)), 'boolean' );
check_equals(Infinity, Infinity);
check( ! isFinite(Infinity) );
check_equals( typeof(isFinite(0/0)), 'boolean' );
check( ! isFinite(0/0) );


#if OUTPUT_VERSION >= 6
check_equals( typeof(_global.Infinity), 'number' );
check_equals( typeof(isFinite(_global.Infinity)), 'boolean' );
check( ! isFinite(_global.Infinity) ); // NOTE: isFinite(undefined) is false for SWF7 up
#else // SWF5 or below
check_equals( typeof(_global), 'undefined' );
check_equals( typeof(Object), 'function' );
check_equals( typeof(Object.prototype), 'object' );
check_equals( typeof(Object.prototype.Infinity), 'undefined' );
#endif

#if OUTPUT_VERSION >= 7
check( ! isFinite(undefined) ); 
check( ! isFinite(null) );
check( ! isFinite(Object.prototype.NaN) );
#else // SWF6 or below
check( isFinite(undefined) );
check( isFinite(null) );
check( isFinite(Object.prototype.NaN) );
#endif

check(! Object.hasOwnProperty('Infinity'));
check(! Object.prototype.hasOwnProperty('Infinity'));
check(! this.__proto__.hasOwnProperty('Infinity'));

//--------------------------------------------------------
// Test automatic conversion to number 
//--------------------------------------------------------

check(isNaN(0+this));
check(isNaN(this));
this.valueOf = function() { return 5; };
check(isNaN(this));
o = new Object;
check(isNaN(o));
check(isNaN(0+o));
o.valueOf = function() { return 3; };
check_equals(0+o, 3);
check_equals(0+"string", "0string");

#if OUTPUT_VERSION < 6
check(!isNaN(2+Number));
#else
check(isNaN(2+Number));
#endif

#if OUTPUT_VERSION >= 7
check(isNaN(2/undefined));
check(isNaN(undefined/2));
check(!isFinite(undefined/2));
check(2/undefined != Infinity);
#else
check(!isNaN(2/undefined));
check(!isNaN(undefined/2));
check(isFinite(undefined/2));
check_equals(2/undefined, Infinity);
#endif

check(!isFinite(Infinity));
check(!isFinite(2/undefined));

check_equals(typeof(("string"<7)), 'undefined');
check_equals(typeof((7<"string")), 'undefined');
check_equals(typeof(("18"<7)), 'boolean');
check_equals(typeof((7<"18")), 'boolean');
check_equals(("18"<"7"), true); // string comparison
check_equals(("18"<7), false); // numeric comparison
check_equals((7<"18"), true); // numeric comparison
check_equals(typeof(_root<"18"), 'undefined'); // _root is ensured to be NAN for SWF6 too

#if OUTPUT_VERSION > 6
check_equals(typeof(undefined<7), 'undefined');
check_equals(typeof(undefined>7), 'undefined');
check_equals(typeof(undefined<-7), 'undefined');
check_equals(typeof(undefined>-7), 'undefined');
check_equals(typeof(7<undefined), 'undefined');
check_equals(typeof(7>undefined), 'undefined');
check_equals(typeof(-7<undefined), 'undefined');
check_equals(typeof(-7>undefined), 'undefined');
check_equals(typeof(null<7), 'undefined');
check_equals(typeof(null>7), 'undefined');
check_equals(typeof(null<-7), 'undefined');
check_equals(typeof(null>-7), 'undefined');
check_equals(typeof(7<null), 'undefined');
check_equals(typeof(7>null), 'undefined');
check_equals(typeof(-7<null), 'undefined');
check_equals(typeof(-7>null), 'undefined');
#else
check_equals(typeof(undefined<7), 'boolean');
check_equals(typeof(undefined>7), 'boolean');
check_equals(typeof(undefined<-7), 'boolean');
check_equals(typeof(undefined>-7), 'boolean');
check_equals(typeof(7<undefined), 'boolean');
check_equals(typeof(7>undefined), 'boolean');
check_equals(typeof(-7<undefined), 'boolean');
check_equals(typeof(-7>undefined), 'boolean');

check_equals((undefined<7), true);
check_equals((undefined>7), false);
check_equals((undefined<-7), false);
check_equals((undefined>-7), true);
check_equals((7<undefined), false);
check_equals((7>undefined), true);
check_equals((-7<undefined), true);
check_equals((-7>undefined), false);
#endif

// ActionNewAdd
check_equals('0' + -1, '0-1');

// string:00 number:0 equality
check_equals('00', 0);

// string:0xFF0000 number:0xFF0000 equality
#if OUTPUT_VERSION > 5
 check_equals("0xFF0000", 0xFF0000);
 check_equals("0XFF0000", 0xFF0000);
 check_equals("0Xff0000", 0xFF0000);
 check("0Xff000000" != 0xFF000000);
#else
 check("0xFF0000" != 0xFF0000);
 check("0XFF0000" != 0xFF0000);
#endif

check(isNaN("0xff000z"));

check_equals(typeof(Number.prototype.valueOf), 'function'); 
check_equals(typeof(Number.prototype.toString), 'function'); 
#if OUTPUT_VERSION > 5
    check(isNaN(Number.valueOf()));
#else
    check(!isNaN(Number.valueOf()) );
#endif

check_equals(typeof(Number), 'function'); 
check(Number.hasOwnProperty('prototype')); 
check(Number.hasOwnProperty('__proto__')); 
check(Number.hasOwnProperty('constructor')); 
check_equals(typeof(Number.prototype), 'object'); 
#if OUTPUT_VERSION == 5
    check_equals(Number.prototype, Object);
    check_equals(Number.prototype, Function);
    check_equals(Number.prototype, Object.__proto__);
    check_equals(Number.prototype, null);
    check_equals(Number.prototype, undefined);
    check(Number.prototype != 0);
    check(Number.prototype != "string");
#else
    check(Number.prototype != Object);
#endif
 
#if OUTPUT_VERSION == 5
    // not visible in swf5 by default.
    check_equals(typeof(Number.valueOf), 'undefined'); 
    check_equals(typeof(Number.toString), 'undefined'); 
    check_equals(typeof(Number.__proto__), 'undefined'); 
    // make properties visible.
    ASSetPropFlags(Number, null, 0, 128 + 1);
    ASSetPropFlags(Object, null, 0, 128 + 1);
#endif
check_equals(typeof(Number.__proto__), 'object'); 
check_equals(Number.prototype.__proto__, Object.prototype); 

check_equals(typeof(Number.toString), 'function');
check_equals(typeof(Number.valueOf), 'function');
check(!Number.hasOwnProperty('valueOf'));
check(!Number.hasOwnProperty('toString'));
check(!Number.__proto__.hasOwnProperty('valueOf'));
check(!Number.__proto__.hasOwnProperty('toString'));
check(Number.__proto__.__proto__ == Object.prototype);

check_equals(typeof(Number.valueOf()), 'function'); // this is odd

a = 1;
check_equals(typeof(a.toString), 'function');
check_equals(typeof(a.valueOf), 'function');
check(!a.hasOwnProperty('valueOf'));
#if OUTPUT_VERSION == 5
    // make properties visible.
    ASSetPropFlags(Number.prototype, null, 0, 128 + 1);
    ASSetPropFlags(Object.prototype, null, 0, 128 + 1);
#endif
check(a.__proto__.hasOwnProperty('valueOf'));
check(!a.hasOwnProperty('toString'));


anum = new Number(1);
check_equals(typeof(anum.toString), 'function');
check_equals(typeof(anum.valueOf), 'function');
check(!anum.hasOwnProperty('valueOf'));
check(anum.__proto__.hasOwnProperty('valueOf'));
check(!anum.hasOwnProperty('toString'));


//-----------------------------------------------------------
// Check conversion to number
//-----------------------------------------------------------

#ifdef MING_SUPPORTS_ASM

asm { push 'val',4 tonumber setvariable };
check_equals(val, 4);

asm { push 'val',null tonumber setvariable };
#if OUTPUT_VERSION < 7
 check_equals(val, 0);
 check(!isNaN(val));
#else
 check(isNaN(val));
#endif

asm { push 'val',undefined tonumber setvariable };
#if OUTPUT_VERSION < 7
 check_equals(val, 0);
 check(!isNaN(val));
#else
 check(isNaN(val));
#endif

asm { push 'val','10' tonumber setvariable };
check_equals(val, 10);

asm { push 'val','3e2' tonumber setvariable };
check_equals(val, 300);

asm { push 'val','2E1' tonumber setvariable };
check_equals(val, 20);

asm { push 'val','2p' tonumber setvariable };
check(isNaN(val));

asm { push 'val','2.6' tonumber setvariable };
check_equals(val, 2.6);

asm { push 'val','string' tonumber setvariable };
check(isNaN(val));

asm { push 'val','NaN' tonumber setvariable };
check(isNaN(val));

asm { push 'val','Infinity' tonumber setvariable };
check(val != Infinity);
check(isNaN(val));

asm { push 'val','-Infinity' tonumber setvariable };
check(val != Infinity);
check(isNaN(val));

obj = new Object();
asm { push 'val','obj' getvariable tonumber setvariable };
check(isNaN(val));

obj = new Object(); obj.valueOf = function() { return 7; };
asm { push 'val','obj' getvariable tonumber setvariable };
check_equals(val, 7); 

obj = function() {}; 
asm { push 'val','obj' getvariable tonumber setvariable };
#if OUTPUT_VERSION > 5
check(isNaN(val));
#else
check(!isNaN(val));
check_equals(typeof(val), 'number');
check_equals(val, 0);
check_equals(0, val);
#endif

obj = function() {}; obj.valueOf = function() { return 9; };
asm { push 'val','obj' getvariable tonumber setvariable };
check_equals(val, 9); 
check_equals(9, val); 

#endif // defined(MING_SUPPORTS_ASM)

//-----------------------------------------------------------
// Check subtraction operator TODO: create an ops.as file 
// for testing operators in general ?
//-----------------------------------------------------------

#if OUTPUT_VERSION > 6
check( isNaN(450 - undefined) );
#else
check_equals(450 - undefined, 450);
#endif

//-----------------------------------------------------------
// Check number formatting as documented in as_value.cpp. 
// Rules are:
// Numbers should be rounded to 15 significant digits.
// Numbers above 10e+15 are expressed with exponent.
// Numbers below 0 with more than 4 leading zeros expressed
//  with exponent.
// Exponent has no leading zero.
// Trailing zeros are always trimmed.
//-----------------------------------------------------------

a=new Number(11111111111111.11111111);
check_equals(a.toString(), "11111111111111.1");

a=new Number(111111111111111.1111111);
check_equals(a.toString(), "111111111111111");

a=new Number(1111111111111111.1111111);
check_equals(a.toString(), "1.11111111111111e+15");

a=new Number(0.000123456789012346);
check_equals(a.toString(), "0.000123456789012346");

a=new Number(0.0000123456789012346);
check_equals(a.toString(), "0.0000123456789012346");

a=new Number(0.00000123456789012346);
check_equals(a.toString(), "1.23456789012346e-6");

a=new Number(0.000000123456789012346);
check_equals(a.toString(), "1.23456789012346e-7");

a=new Number(0.0999999999999999);
check_equals(a.toString(), "0.0999999999999999");

a=new Number(0.99999999999999938);
check_equals(a.toString(), "0.999999999999999");

a=new Number(9.9999999999999939 / 10);
check_equals(a.toString(), "0.999999999999999");

a=new Number(5.4 / 100000);
check_equals(a.toString(), "0.000054");

a=new Number(5.4 / 1000000);
check_equals(a.toString(), "5.4e-6");
check_equals(a.toString(10), "5.4e-6");

a=new Number(2.123456789123455);
check_equals(a.toString(), "2.12345678912346"); // rounds abs up

a=new Number(-2.123456789123455);
check_equals(a.toString(), "-2.12345678912346"); // rounds abs up

a=new Number(0.1234567891234565);
check_equals(a.toString(), "0.123456789123457"); // rounds abs up

a=new Number(-0.1234567891234565);
check_equals(a.toString(), "-0.123456789123457"); // round abs up

a=new Number(1.234567891234565e+308); 
check_equals(a.toString(), "1.23456789123456e+308"); // round abs down

a=new Number(-1.234567891234565e+308); 
check_equals(a.toString(), "-1.23456789123456e+308"); // round abs down

a=new Number(1.234567891234565e-308);  // gnash rounds this down, pp up
xcheck_equals(a.toString(), "1.23456789123457e-308"); // round abs up 

a=new Number(-1.234567891234565e-308);  // gnash rounds this down, pp up
xcheck_equals(a.toString(), "-1.23456789123457e-308"); // round abs up 

a=new Number(1.234567891234565e-6);  // gnash rounds this down, pp up
xcheck_equals(a.toString(), "1.23456789123457e-6"); // round abs up 

a=new Number(-1.234567891234565e-6);  // gnash rounds this down, pp up
xcheck_equals(a.toString(), "-1.23456789123457e-6"); // round abs up 

a=new Number(-1.244567891234565e-6);  // gnash rounds this down, pp up
check_equals(a.toString(), "-1.24456789123457e-6"); // round abs up 

a=new Number(-1.244667891234565e-6);  // gnash rounds this down, pp up
check_equals(a.toString(), "-1.24466789123457e-6"); // round abs up 

a=new Number(-0.001234567891234565); // gnash succeeds in rounding this up
check_equals(a.toString(), "-0.00123456789123457"); // round abs up 

a=new Number(-0.001244567891234565); // gnash succeeds in rounding this down
check_equals(a.toString(), "-0.00124456789123456"); 

a=new Number(-0.001234567891234564); 
check_equals(a.toString(), "-0.00123456789123456"); // round abs down (obvious?)

a=new Number(0.001234567891234565); 
check_equals(a.toString(), "0.00123456789123457"); // round abs up 

a=new Number(0.001234567891234564);
check_equals(a.toString(), "0.00123456789123456"); // round abs down (obvious?)

check( isNaN(0/0) );

// END OF TEST

#if OUTPUT_VERSION < 6
 check_totals(213);
#else
#if OUTPUT_VERSION < 7
 check_totals(206);
#else
 check_totals(204);
#endif
#endif
