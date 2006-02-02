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

// Test case for TextSnapshot ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new TextSnapshot;

// test the TextSnapshot constuctor
if (tmp) {
	trace("PASSED: TextSnapshot::TextSnapshot() constructor");
} else {
	trace("FAILED: TextSnapshot::TextSnapshot()");		
}

// test the TextSnapshot::findtext method
if (tmp.findtext) {
	trace("PASSED: TextSnapshot::findtext() exists");
} else {
	trace("FAILED: TextSnapshot::findtext() doesn't exist");
}
// test the TextSnapshot::getcount method
if (tmp.getcount) {
	trace("PASSED: TextSnapshot::getcount() exists");
} else {
	trace("FAILED: TextSnapshot::getcount() doesn't exist");
}
// test the TextSnapshot::getselected method
if (tmp.getselected) {
	trace("PASSED: TextSnapshot::getselected() exists");
} else {
	trace("FAILED: TextSnapshot::getselected() doesn't exist");
}
// test the TextSnapshot::getselectedtext method
if (tmp.getselectedtext) {
	trace("PASSED: TextSnapshot::getselectedtext() exists");
} else {
	trace("FAILED: TextSnapshot::getselectedtext() doesn't exist");
}
// test the TextSnapshot::gettext method
if (tmp.gettext) {
	trace("PASSED: TextSnapshot::gettext() exists");
} else {
	trace("FAILED: TextSnapshot::gettext() doesn't exist");
}
// test the TextSnapshot::hittesttextnearpos method
if (tmp.hittesttextnearpos) {
	trace("PASSED: TextSnapshot::hittesttextnearpos() exists");
} else {
	trace("FAILED: TextSnapshot::hittesttextnearpos() doesn't exist");
}
// test the TextSnapshot::setselectcolor method
if (tmp.setselectcolor) {
	trace("PASSED: TextSnapshot::setselectcolor() exists");
} else {
	trace("FAILED: TextSnapshot::setselectcolor() doesn't exist");
}
// test the TextSnapshot::setselected method
if (tmp.setselected) {
	trace("PASSED: TextSnapshot::setselected() exists");
} else {
	trace("FAILED: TextSnapshot::setselected() doesn't exist");
}
