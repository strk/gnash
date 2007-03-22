// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//
// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA
// Test case for Number ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

//
// TODO: have ming output ACTION_EQUAL
//       it seems it will only do it for SWF4 output
//
// TODO: test with SWF target != 6 (the only one tested so far)
//	

rcsid="$Id: Number.as,v 1.15 2007/03/22 00:30:45 strk Exp $";

#include "check.as"

var n1=new Number(268);

// strict-equality operator was introduced in SWF6
#if OUTPUT_VERSION > 5
check ( ! (n1 === 268) );
// They are not the same object !
check ( ! (n1 === Number(268)) );
#endif

// but they have the same numeric value
check_equals (n1 , 268 );
check_equals (268 , n1 );

// Test Number.toString 
check_equals(typeof(n1.toString), "function");
check_equals(typeof(n1.toString()), "string"); 
check_equals(n1.toString(), "268");

var backup = Object.prototype.toString;
Object.prototype.toString = function() { return "fake_string"; };
check_equals(n1.toString(), "268"); // doesn't inherit from Object
Object.prototype.toString = backup;

// Test Number.valueOf 
check_equals(typeof(n1.valueOf), "function");
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
check(NaN != NaN);
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
xcheck(isNaN(o));
xcheck(isNaN(0+o));
o.valueOf = function() { return 3; };
check_equals(0+o, 3);
check_equals(0+"string", "0string");

#if OUTPUT_VERSION < 6
check(!isNaN(2+Number));
#else
xcheck(isNaN(2+Number));
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

