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

// Test case for Selection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Selection;

// test the Selection constuctor
if (tmp) {
	trace("PASSED: Selection::Selection() constructor");
} else {
	trace("FAILED: Selection::Selection()");		
}

// test the Selection::addlistener method
if (tmp.addlistener) {
	trace("PASSED: Selection::addlistener() exists");
} else {
	trace("FAILED: Selection::addlistener() doesn't exist");
}
// test the Selection::getbeginindex method
if (tmp.getbeginindex) {
	trace("PASSED: Selection::getbeginindex() exists");
} else {
	trace("FAILED: Selection::getbeginindex() doesn't exist");
}
// test the Selection::getcaretindex method
if (tmp.getcaretindex) {
	trace("PASSED: Selection::getcaretindex() exists");
} else {
	trace("FAILED: Selection::getcaretindex() doesn't exist");
}
// test the Selection::getendindex method
if (tmp.getendindex) {
	trace("PASSED: Selection::getendindex() exists");
} else {
	trace("FAILED: Selection::getendindex() doesn't exist");
}
// test the Selection::getfocus method
if (tmp.getfocus) {
	trace("PASSED: Selection::getfocus() exists");
} else {
	trace("FAILED: Selection::getfocus() doesn't exist");
}
// test the Selection::removelistener method
if (tmp.removelistener) {
	trace("PASSED: Selection::removelistener() exists");
} else {
	trace("FAILED: Selection::removelistener() doesn't exist");
}
// test the Selection::setfocus method
if (tmp.setfocus) {
	trace("PASSED: Selection::setfocus() exists");
} else {
	trace("FAILED: Selection::setfocus() doesn't exist");
}
// test the Selection::set method
if (tmp.set) {
	trace("PASSED: Selection::set() exists");
} else {
	trace("FAILED: Selection::set() doesn't exist");
}
