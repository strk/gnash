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

// Test case for Mouse ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Mouse;

// test the Mouse constuctor
if (tmp) {
	trace("PASSED: Mouse::Mouse() constructor");
} else {
	trace("FAILED: Mouse::Mouse()");		
}

// test the Mouse::addlistener method
if (tmp.addlistener) {
	trace("PASSED: Mouse::addlistener() exists");
} else {
	trace("FAILED: Mouse::addlistener() doesn't exist");
}
// test the Mouse::hide method
if (tmp.hide) {
	trace("PASSED: Mouse::hide() exists");
} else {
	trace("FAILED: Mouse::hide() doesn't exist");
}
// test the Mouse::removelistener method
if (tmp.removelistener) {
	trace("PASSED: Mouse::removelistener() exists");
} else {
	trace("FAILED: Mouse::removelistener() doesn't exist");
}
// test the Mouse::show method
if (tmp.show) {
	trace("PASSED: Mouse::show() exists");
} else {
	trace("FAILED: Mouse::show() doesn't exist");
}
