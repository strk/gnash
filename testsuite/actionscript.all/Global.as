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


// Test case for ActionScript _global Object
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Global.as,v 1.28 2007/09/27 15:42:11 strk Exp $";

#include "check.as"

// Check that _global.parseInt is in effect what parseInt resolves to
#if OUTPUT_VERSION > 5
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
check_equals ( typeof(_global.parseInt), 'undefined' );
#endif

check_equals(typeof(isNaN), 'function');
#if OUTPUT_VERSION > 5
check(!_global.hasOwnProperty('isNaN'));
#endif

// Test parseInt
check ( parseInt('45b') == 45 );
check ( parseInt('65') == 65 );
check ( parseInt('-1234') == -1234 );
check ( parseInt('-1.234') == -1 );
// Test parseint with hex
check ( parseInt('0x111') == 273 );
// Test parseint with octal
xcheck_equals (parseInt('   0352'), 352 );
// a '0' prefix turns the number into an octal one ?
xcheck (parseInt('   0352') != 0352 );
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

// It's not reliable to compare a double type with ==, so we'll give it a
// small range using >= and <=
check ( isNaN(parseFloat('test')) );
check ( parseFloat('1.5') >= 1.499 && parseFloat('1.5') <= 1.501 );
check ( parseFloat('   	    -2001.5') >= -2001.51 && parseFloat('   	    -2001.5') <= -2001.49 );
check ( parseFloat('		 5.13123abc2.35387') >= 5.1312 && parseFloat('		 5.13123abc2.35387') <= 5.1313 );
check ( isNaN(parseFloat('         x1.234')) );

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
