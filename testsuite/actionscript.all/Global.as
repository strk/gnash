// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for ActionScript _global Object
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Global.as,v 1.14 2006/11/05 00:45:27 rsavoye Exp $";

#include "check.as"

// Check that _global.parseInt is in effect what parseInt resolves to
check ( parseInt == _global.parseInt );

// Test parseInt
check ( parseInt('45b') == 45 );
check ( parseInt('65') == 65 );
check ( parseInt('-1234') == -1234 );
check ( parseInt('-1.234') == -1 );
// Test parseint with hex
check ( parseInt('0x111') == 273 );
// Test parseint with octal
xcheck_equals (parseInt('   0352'), 352 );
// Test parseint with 36 base
check ( parseInt('2GA',36) == (10+16*36+2*36*36) );
// Test parseint with base 17 - the 'H' is not part of base 17, only the first two digits are valid
check ( parseInt('FGH',17) == (16+17*15) );
check ( parseInt('513x51') == 513 );
check ( isNan(parseInt('a1023')) );
check ( isNaN(parseInt('zero')) );
// parseInt returns NaN (which is different from infinity)
check ( ! isFinite(parseInt('none')) );
check ( ! isFinite(1/0) );
check ( ! isNaN(1/0) );

// It's not reliable to compare a double type with ==, so we'll give it a
// small range using >= and <=
check ( isNaN(parseFloat('test')) );
check ( parseFloat('1.5') >= 1.499 && parseFloat('1.5') <= 1.501 );
check ( parseFloat('   	    -2001.5') >= -2001.51 && parseFloat('   	    -2001.5') <= -2001.49 );
check ( parseFloat('		 5.13123abc2.35387') >= 5.1312 && parseFloat('		 5.13123abc2.35387') <= 5.1313 );
check ( isNaN(parseFloat('         x1.234')) );

// All %NN must become the corresponding ascii char
check ( unescape('%3A%2F%3F%3D%26') == ':/?=&' );

// All ascii char become the corresponding %NN hex
xcheck (escape(':/?=&') == '%3A%2F%3F%3D%26');

// How to test failure of setInterval and success of clearInterval ?
// The problem is that there is no way 
// to run an interval before a frame is executed
// so this will require onEnterFrame to work.
// We don't want to test onEnterFrame here, do we ?
INTERVALRUN=0;
INTERVALVARIABLE=undefined;
function intervalChecker() {
	check("setInterval called" | true);
	if ( INTERVALRUN ) check("clearInterval not called" & false);
	INTERVALRUN=1;
	clearInterval(INTERVALVARIABLE);
}
INTERVALVARIABLE=setInterval(intervalChecker, 5); // 1 second

