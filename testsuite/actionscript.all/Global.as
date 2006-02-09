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
//

// Test case for ActionScript _global Object
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "check.as"

// Check that _global.parseInt is in effect what parseInt resolves to
check ( parseInt == _global.parseInt );

// Test parseInt
check ( parseInt('45b') == 45 );
check ( parseInt('65') == 65 );
check ( parseInt('-1234') == -1234 );
check ( parseInt('-1.234') == -1 );
check ( isNaN(parseInt('zero')) );
check ( !isFinite(parseInt('none')) );

// All %NN must become the corresponding ascii char
check ( unescape('%3A%2F%3F%3D%26') == ':/?=&' );

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

