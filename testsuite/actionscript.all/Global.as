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

// Define USE_XTRACE to use "visual" trace
#ifdef USE_XTRACE
# include "xtrace.as"
# define trace xtrace
#endif


// Check that _global.parseInt is in effect what parseInt resolves to
if ( parseInt == _global.parseInt ) {
	trace("PASSED: parseInt == _global.parseInt");
} else {
	trace("FAILED: parseInt != _global.parseInt");
}

// Test parseInt
var a = parseInt('45b');
if ( a == 45 ) {
	trace("PASSED: parseInt('45b')");
} else {
	trace("FAILED: parseInt('45b') == "+a);
}

// Test isNaN and NaN
var a = parseInt('zero');
if ( isNaN(a) ) {
	trace("PASSED: parseInt('zero') is NaN");
} else {
	trace("FAILED: parseInt('zero') == "+a);
}
if ( ! isFinite(a) ) {
	trace("PASSED: parseInt('zero') is not finite");
} else {
	trace("FAILED: parseInt('zero') == "+a);
}

// All %NN must become the corresponding ascii char
var url = "http%3A%2F%2Fwww.gnu.org%3Fp%3Dgnash%26u%3Dyes";
var unescaped = unescape(url);
if ( unescaped == 'http://www.gnu.org?p=gnash&u=yes' ) {
	trace("PASSED: unescape");
} else {
	trace("FAILED: unescape "+unescaped);
}

// How to test failure of setInterval and success of clearInterval ?
// The problem is that there is no way 
// to run an interval before a frame is executed
// so this will require onEnterFrame to work.
// We don't want to test onEnterFrame here, do we ?
INTERVALRUN=0;
INTERVALVARIABLE=undefined;
function intervalChecker() {
	trace("PASSED: setInterval");
	if ( INTERVALRUN ) trace("FAILED: clearInterval");
	INTERVALRUN=1;
	clearInterval(INTERVALVARIABLE);
}
INTERVALVARIABLE=setInterval(intervalChecker, 5); // 1 second

