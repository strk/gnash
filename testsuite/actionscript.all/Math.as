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



// Test case for Math ActionScript class
//
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

// Note that "check_equals (Math.acos(2), notanumber)" fails even though
// acos(2) does return a NaN, both in Gnash and in FlashPlayer.
//
// This is because IEEE 754 NaNs are represented with the exponential field
// filled with ones and some non-zero number in the mantissa, so there are
// actually many different bitwise values that represent NaN.
// Hence we string expected NaN results and compare the strings.

rcsid="Math.as";
#include "check.as"

var mathObj = new Math;
check_equals ( typeof(mathObj), 'undefined' );

check_equals (typeof(Math), 'object');
check_equals (typeof(Math.__proto__), 'object');
check_equals (Math.__proto__, Object.prototype);

count = 0;
for (m in Math) { trace(m); count++; };
check_equals(count, 0);

// test methods existance
check_equals (typeOf(Math.abs), 'function');
check_equals (typeOf(Math.acos), 'function');
check_equals (typeOf(Math.asin), 'function');
check_equals (typeOf(Math.atan), 'function');
check_equals (typeOf(Math.ceil), 'function');
check_equals (typeOf(Math.cos), 'function');
check_equals (typeOf(Math.exp), 'function');
check_equals (typeOf(Math.floor), 'function');
check_equals (typeOf(Math.log), 'function');
check_equals (typeOf(Math.random), 'function');
check_equals (typeOf(Math.round), 'function');
check_equals (typeOf(Math.sin), 'function');
check_equals (typeOf(Math.sqrt), 'function');
check_equals (typeOf(Math.tan), 'function');
check_equals (typeOf(Math.atan2), 'function');
check_equals (typeOf(Math.max), 'function');
check_equals (typeOf(Math.min), 'function');
check_equals (typeOf(Math.pow), 'function');
check_equals (typeOf(Math.E), 'number');
check_equals (typeOf(Math.LN2), 'number');
check_equals (typeOf(Math.LOG2E), 'number');
check_equals (typeOf(Math.LN10), 'number');
check_equals (typeOf(Math.LOG10E), 'number');
check_equals (typeOf(Math.PI), 'number');
check_equals (typeOf(Math.SQRT1_2), 'number');
check_equals (typeOf(Math.SQRT2), 'number');

#if OUTPUT_VERSION > 6
check(Date.UTC != undefined);

// From SWF 7 up methods are case-sensitive !
check_equals (Math.e, undefined);
check_equals (Math.ln2, undefined);
check_equals (Math.log2e, undefined);
check_equals (Math.ln10, undefined);
check_equals (Math.log10e, undefined);
check_equals (Math.pi, undefined);
check_equals (Math.sqrt1_2, undefined);
check_equals (Math.sqrt2, undefined);

#endif

// Check against 15-digit rounded values to allow for last-bits imprecision

// Constants
check_equals (Math.E.toString(), "2.71828182845905");
check_equals (Math.LN2.toString(), "0.693147180559945");
check_equals (Math.LOG2E.toString(), "1.44269504088896");
check_equals (Math.LN10.toString(), "2.30258509299405");
check_equals (Math.LOG10E.toString(), "0.434294481903252");
check_equals (Math.PI.toString(), "3.14159265358979");
check_equals (Math.SQRT1_2.toString(), "0.707106781186548");
check_equals (Math.SQRT2.toString(), "1.4142135623731");

//
// Named constants, used in the following tests
//

// Some handy values: PI/2 and PI/4, and their string values
var pi_2 = new Number(Math.PI/2);
var pi_4 = new Number(Math.PI/4);
var pi_34 = new Number(Math.PI * 3/4);
var pis = new String(Math.PI.toString());
var pi_2s = new String(pi_2.toString());
var pi_4s = new String(pi_4.toString());

// Gnash is currently missing NaN and Infinity
var plusinf = new Number(1.0/0.0);
var minusinf = new Number(-1.0/0.0);
var notanumber = new Number(Math.acos(2));

// We check computed results in two ways: the printed form for large values
// (which is 15 significant digits) or being within a range of +/-delta from
// the expected value for small ones (typically values close to 0).

// makeswf accepts the 1e-15 but gives a value of 0
//var delta = new Number(1e-15);
var delta = new Number(0.000000000000001);


//-----------------------------------------------------------------
// Test Math.abs
//-----------------------------------------------------------------

check_equals (Math.abs(15), 15);
check_equals (Math.abs(-15), 15);
check_equals (Math.abs(plusinf), plusinf);
check_equals (Math.abs(minusinf), plusinf);
check_equals (Math.abs(notanumber).toString(), "NaN");

#if OUTPUT_VERSION < 7
check_equals (Math.abs(undefined).toString(), "0");
#else
check_equals (Math.abs(undefined).toString(), "NaN");
#endif
check_equals (Math.abs().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.acos
//-----------------------------------------------------------------

check_equals (Math.acos(0).toString(), pi_2s);
check_equals (Math.acos(1), 0);
check_equals (Math.acos(0.5).toString(), "1.0471975511966");
check_equals (Math.acos(-0.5).toString(), "2.0943951023932");
check_equals (Math.acos(-1).toString(), pis);
check_equals (Math.acos(2).toString(), "NaN");
check_equals (Math.acos(-2).toString(), "NaN");

#if OUTPUT_VERSION < 7
check_equals (Math.acos(undefined).toString(), "1.5707963267949");
check_equals (Math.acos(acos(2)).toString(), "1.5707963267949");
check_equals (Math.acos(undefined).toString(), "1.5707963267949");
#else
check_equals (Math.acos(undefined).toString(), "NaN");
check_equals (Math.acos(acos(2)).toString(), "NaN");
check_equals (Math.acos(undefined).toString(), "NaN");
#endif

check_equals (Math.acos(1.0/0.0).toString(), "NaN");
check_equals (Math.acos(-1.0/0.0).toString(), "NaN");
check_equals (Math.acos(plusinf).toString(), "NaN");
check_equals (Math.acos(minusinf).toString(), "NaN");
check_equals (Math.acos(notanumber).toString(), "NaN");
check_equals (Math.acos().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.asin
//-----------------------------------------------------------------

check_equals (Math.asin(0), 0);
check_equals (Math.asin(1).toString(), pi_2s);
check_equals (Math.asin(-1).toString(), "-" + pi_2s);
check_equals (Math.asin(0.5).toString(), "0.523598775598299");
check_equals (Math.asin(-0.5).toString(), "-0.523598775598299");
check_equals (Math.asin(2).toString(), "NaN");
check_equals (Math.asin(-2).toString(), "NaN");
check_equals (Math.asin(plusinf).toString(), "NaN");
check_equals (Math.asin(minusinf).toString(), "NaN");
check_equals (Math.asin(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
check_equals (Math.asin(undefined).toString(), "0");
#else
check_equals (Math.asin(undefined).toString(), "NaN");
#endif
check_equals (Math.asin().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.atan
//-----------------------------------------------------------------

check_equals (Math.atan(0), 0);
check_equals (Math.atan(0.5).toString(), "0.463647609000806");
check_equals (Math.atan(-0.5).toString(), "-0.463647609000806");
check_equals (Math.atan(1).toString(), "0.785398163397448");
check_equals (Math.atan(-1).toString(), "-0.785398163397448");
check_equals (Math.atan(2).toString(), "1.10714871779409");
check_equals (Math.atan(-2).toString(), "-1.10714871779409");
check_equals (Math.atan(plusinf).toString(), pi_2s);
check_equals (Math.atan(minusinf).toString(), "-" + pi_2s);
check_equals (Math.atan(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.atan(undefined).toString(), "0");
#else
 check_equals (Math.atan(undefined).toString(), "NaN");
#endif
check_equals (Math.atan().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.ceil
//-----------------------------------------------------------------

check_equals (Math.ceil(0), 0);
check_equals (Math.ceil(1), 1);
check_equals (Math.ceil(-1), -1);
check_equals (Math.ceil(0.1), 1);
check_equals (Math.ceil(0.5), 1);
check_equals (Math.ceil(0.9), 1);
check_equals (Math.ceil(-0.1), 0);
check_equals (Math.ceil(-0.5), 0);
check_equals (Math.ceil(-0.9), 0);
check_equals (Math.ceil(notanumber).toString(), "NaN");
check_equals (Math.ceil(plusinf), plusinf);
check_equals (Math.ceil(minusinf), minusinf);
#if OUTPUT_VERSION < 7
 check_equals (Math.ceil(undefined).toString(), "0");
#else
 check_equals (Math.ceil(undefined).toString(), "NaN");
#endif
check_equals (Math.ceil().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.cos
//-----------------------------------------------------------------

check_equals (Math.cos(0), 1);
// Flash gives 6.12303176911189e-17 so check that our answer is within similar
// accuracy. Testing shows that actually we give exactly the same result.
check (Math.cos(Math.PI / 2) < delta && Math.cos(Math.PI / 2) > -delta);
check (Math.cos(-Math.PI / 2) < delta && -Math.cos(Math.PI / 2) > -delta);
check_equals (Math.cos(Math.PI), -1);
check_equals (Math.cos(-Math.PI), -1);
check_equals (Math.cos(Math.PI * 2), 1);
check_equals (Math.cos(-Math.PI * 2), 1);
check_equals (Math.cos(1).toString(), "0.54030230586814");
check_equals (Math.cos(-1).toString(), "0.54030230586814");
check_equals (Math.cos(plusinf).toString(), "NaN");
check_equals (Math.cos(minusinf).toString(), "NaN");
check_equals (Math.cos(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.cos(undefined).toString(), "1");
#else
 check_equals (Math.cos(undefined).toString(), "NaN");
#endif
check_equals (Math.cos().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.exp
//-----------------------------------------------------------------

check_equals (Math.exp(0), 1);
check_equals (Math.exp(1).toString(), "2.71828182845905");
check_equals (Math.exp(2).toString(), "7.38905609893065");
check_equals (Math.exp(-1).toString(), "0.367879441171442");
check_equals (Math.exp(plusinf).toString(), "Infinity");
check_equals (Math.exp(minusinf), 0);
check_equals (Math.exp(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.exp(undefined).toString(), "1");
#else
 check_equals (Math.exp(undefined).toString(), "NaN");
#endif
check_equals (Math.exp().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.floor
//-----------------------------------------------------------------

check_equals (Math.floor(0), 0);
check_equals (Math.floor(0.1), 0);
check_equals (Math.floor(0.5), 0);
check_equals (Math.floor(0.9), 0);
check_equals (Math.floor(1), 1);
check_equals (Math.floor(-0.1), -1);
check_equals (Math.floor(-0.5), -1);
check_equals (Math.floor(-0.9), -1);
check_equals (Math.floor(-2), -2);
check_equals (Math.floor(plusinf).toString(), "Infinity");
check_equals (Math.floor(minusinf).toString(), "-Infinity");
check_equals (Math.floor(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.floor(undefined).toString(), "0");
#else
 check_equals (Math.floor(undefined).toString(), "NaN");
#endif
check_equals (Math.floor().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.log
//-----------------------------------------------------------------

check_equals (Math.log(0).toString(), "-Infinity");
check_equals (Math.log(1), 0);
check_equals (Math.log(Math.E), 1);
check_equals (Math.log(2).toString(), "0.693147180559945");
check_equals (Math.log(-1).toString(), "NaN");
check_equals (Math.log(plusinf).toString(), "Infinity");
check_equals (Math.log(minusinf).toString(), "NaN");
check_equals (Math.log(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.log(undefined).toString(), "-Infinity");
#else
 check_equals (Math.log(undefined).toString(), "NaN");
#endif
check_equals (Math.log().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.random
//-----------------------------------------------------------------

var math_random = new Number(Math.random());
check (math_random >= 0.0 && math_random < 1.0)
var math_random2 = new Number(Math.random());
check (math_random2 >= 0.0 && math_random2 < 1.0)
check (math_random != math_random2);		// very unlikely, anyhow!

//-----------------------------------------------------------------
// Test Math.round
//-----------------------------------------------------------------

check_equals (Math.round(0), 0);
check_equals (Math.round(0.1), 0);
check_equals (Math.round(0.49), 0);
check_equals (Math.round(0.5), 1);
check_equals (Math.round(0.9), 1);
check_equals (Math.round(1), 1);
check_equals (Math.round(-0.1), 0);
check_equals (Math.round(-0.5), 0);
check_equals (Math.round(-0.9), -1);
check_equals (Math.round(plusinf).toString(), "Infinity");
check_equals (Math.round(minusinf).toString(), "-Infinity");
check_equals (Math.round(notanumber).toString(), "NaN");
check(isNaN(Math.round('')));
check_equals(typeof(Math.round('')), 'number');
#if OUTPUT_VERSION < 7
 check_equals (Math.round(undefined).toString(), "0"); 
#else
 check_equals (Math.round(undefined).toString(), "NaN");
#endif
check_equals (Math.round().toString(), "NaN");
// Don't know what round() and friends do with huge numbers that cannot be
// resolved to individual integer resolution. Don't really care either...

//-----------------------------------------------------------------
// Test Math.sin
//-----------------------------------------------------------------

check_equals (Math.sin(0), 0);
check_equals (Math.sin(Math.PI / 2), 1);
//check_equals (Math.sin(-Math.PI / 2), -1);
// Flash gives 1.22460635382238e-16 so check that our answer is within similar
// accuracy. Testing shows that actually we give exactly the same result.
check (Math.sin(Math.PI) < delta && Math.sin(Math.PI) > -delta);
check (Math.sin(-Math.PI) < delta && Math.sin(-Math.PI) > -delta);
check_equals (Math.sin(1).toString(), "0.841470984807897");
check_equals (Math.sin(-1).toString(), "-0.841470984807897");
// check wrapping of random large values
check_equals (Math.sin(100).toString(), "-0.506365641109759");
check_equals (Math.sin(-100).toString(), "0.506365641109759");
check_equals (Math.sin(plusinf).toString(), "NaN");
check_equals (Math.sin(minusinf).toString(), "NaN");
check_equals (Math.sin(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.sin(undefined).toString(), "0"); 
#else
 check_equals (Math.sin(undefined).toString(), "NaN"); 
#endif
check_equals (Math.sin().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.sqrt
//-----------------------------------------------------------------

check_equals (Math.sqrt(0), 0);
check_equals (Math.sqrt(1), 1);
check_equals (Math.sqrt(-1).toString(), "NaN");
check_equals (Math.sqrt(100), 10);
check_equals (Math.sqrt(0.01), 0.1);
check_equals (Math.sqrt(2).toString(), "1.4142135623731");
check_equals (Math.sqrt(plusinf).toString(), "Infinity");
check_equals (Math.sqrt(minusinf).toString(), "NaN");
check_equals (Math.sqrt(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.sqrt(undefined).toString(), "0"); 
#else
 check_equals (Math.sqrt(undefined).toString(), "NaN"); 
#endif
check_equals (Math.sqrt().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.tan
//-----------------------------------------------------------------

check_equals (Math.tan(0), 0);
// Should either be very large or very small, according to inaccuracy of PI
check (Math.tan(Math.PI / 2) > 1/delta || Math.tan(Math.PI / 2) < -1/delta);
// Should be zero, allowing for inaccuracy of pi.
check (Math.tan(Math.PI) < delta && Math.tan(Math.PI) > -delta);
check (Math.tan(-Math.PI) < delta && Math.tan(-Math.PI) > -delta);
check_equals (Math.tan(1).toString(), "1.5574077246549");
// check wrapping of random large values
check_equals (Math.tan(100).toString(), "-0.587213915156929");
check_equals (Math.tan(-100).toString(), "0.587213915156929");
check_equals (Math.tan(plusinf).toString(), "NaN");
check_equals (Math.tan(minusinf).toString(), "NaN");
check_equals (Math.tan(notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.tan(undefined).toString(), "0"); 
#else
 check_equals (Math.tan(undefined).toString(), "NaN"); 
#endif
check_equals (Math.tan().toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.atan2
//-----------------------------------------------------------------

check_equals (Math.atan2().toString(), "NaN");
check_equals (Math.atan2(1).toString(), "NaN");
// Check the 8 compass points for atan2(y, x)
check_equals (Math.atan2(0, 1), 0);
check_equals (Math.atan2(1, 1).toString(), pi_4s);
check_equals (Math.atan2(1, 0).toString(), pi_2s);
check_equals (Math.atan2(1, -1).toString(), pi_34.toString());
check_equals (Math.atan2(0, -1).toString(), pis);
check_equals (Math.atan2(-1, -1).toString(), "-" + pi_34.toString());
check_equals (Math.atan2(-1, 0).toString(), "-" + pi_2s);
check_equals (Math.atan2(-1, 1).toString(), "-" + pi_4s);
// Same thing should work for infinities
check_equals (Math.atan2(0, plusinf), 0);
check_equals (Math.atan2(plusinf, plusinf).toString(), pi_4s);
check_equals (Math.atan2(plusinf, 0).toString(), pi_2s);
check_equals (Math.atan2(plusinf, minusinf).toString(), pi_34.toString());
check_equals (Math.atan2(0, minusinf).toString(), pis);
check_equals (Math.atan2(minusinf, minusinf).toString(), "-" + pi_34.toString());
check_equals (Math.atan2(minusinf, 0).toString(), "-" + pi_2s);
check_equals (Math.atan2(minusinf, plusinf).toString(), "-" + pi_4s);
// Rogue values
check_equals (Math.atan2(notanumber,1).toString(), "NaN");
check_equals (Math.atan2(1,notanumber).toString(), "NaN");
#if OUTPUT_VERSION < 7
 check_equals (Math.atan2(undefined,1).toString(), "0"); 
 check_equals (Math.atan2(1,undefined).toString(), "1.5707963267949"); 
#else
 check_equals (Math.atan2(undefined,1).toString(), "NaN"); 
 check_equals (Math.atan2(1,undefined).toString(), "NaN"); 
#endif

//-----------------------------------------------------------------
// Test Math.max
//-----------------------------------------------------------------

check_equals (Math.max().toString(), "-Infinity");  // Heaven knows why!
check_equals (Math.max(1).toString(), "NaN");
check_equals (Math.max(1,2), 2);
check_equals (Math.max(2,1), 2);
check_equals (Math.max(1,2,3), 2);
check_equals (Math.max(-1,-2), -1);
check_equals (Math.max(-2,-1), -1);
check_equals (Math.max(0,plusinf).toString(), "Infinity");
check_equals (Math.max(0,minusinf), 0);
check_equals (Math.max(plusinf,minusinf).toString(), "Infinity");
#if OUTPUT_VERSION < 7
 check_equals (Math.max(0,undefined).toString(), "0"); 
#else
 check_equals (Math.max(0,undefined).toString(), "NaN"); 
#endif
check_equals (Math.max(0,notanumber).toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.min
//-----------------------------------------------------------------

check_equals (Math.min().toString(), "Infinity");  // Heaven knows why!
check_equals (Math.min(1).toString(), "NaN");
check_equals (Math.min(1,2), 1);
check_equals (Math.min(2,1), 1);
check_equals (Math.min(1,2,3), 1);
check_equals (Math.min(1,2,0), 1);
check_equals (Math.min(-1,-2), -2);
check_equals (Math.min(-2,-1), -2);
check_equals (Math.min(0,plusinf), 0);
check_equals (Math.min(0,minusinf).toString(), "-Infinity");
check_equals (Math.min(plusinf,minusinf).toString(), "-Infinity");
#if OUTPUT_VERSION < 7
 check_equals (Math.min(0,undefined).toString(), "0"); 
#else
 check_equals (Math.min(0,undefined).toString(), "NaN"); 
#endif
check_equals (Math.min(0,notanumber).toString(), "NaN");

//-----------------------------------------------------------------
// Test Math.pow
//-----------------------------------------------------------------

check_equals (Math.pow().toString(), "NaN");
check_equals (Math.pow(0).toString(), "NaN");
check_equals (Math.pow(1), 1);			// !!
check_equals (Math.pow(2).toString(), "NaN");
check_equals (Math.pow(plusinf).toString(), "NaN");
check_equals (Math.pow(minusinf).toString(), "NaN");
check_equals (Math.pow(undefined).toString(), "NaN");
check_equals (Math.pow(notanumber).toString(), "NaN");
check_equals (Math.pow(1,10), 1);
check_equals (Math.pow(10,1), 10);
check_equals (Math.pow(1,-10), 1);
check_equals (Math.pow(-10,1), -10);
check_equals (Math.pow(2, 10), 1024);
// These lines make makeswf bomb out.
check_equals (Math.pow(2, 0.5).toString(), Math.SQRT2.toString());
check_equals (Math.pow(2, -0.5).toString(), Math.SQRT1_2.toString());
check_equals (Math.pow(-2, 0), 1);
check_equals (Math.pow(-2, 1), -2);
check_equals (Math.pow(-2, 2), 4);
check_equals (Math.pow(-2, 3), -8);
check_equals (Math.pow(-2, -1), -0.5);
check_equals (Math.pow(-2, -2), 0.25);
// These lines make makeswf bomb out.
check_equals (Math.pow(-2, 0.5).toString(), "NaN");
check_equals (Math.pow(-2, -0.5).toString(), "NaN");

//-----------------------------------------------------------------
// END OF TESTS
//-----------------------------------------------------------------

// End of Math testsuite
#if OUTPUT_VERSION <= 6
 check_totals(272);
#else
 check_totals(281);
#endif
