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

// Test case for CustomActions ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new CustomActions;

// test the CustomActions constuctor
if (tmp) {
	trace("PASSED: CustomActions::CustomActions() constructor");
} else {
	trace("FAILED: CustomActions::CustomActions()");		
}

// test the CustomActions::get method
if (tmp.get) {
	trace("PASSED: CustomActions::get() exists");
} else {
	trace("FAILED: CustomActions::get() doesn't exist");
}
// test the CustomActions::install method
if (tmp.install) {
	trace("PASSED: CustomActions::install() exists");
} else {
	trace("FAILED: CustomActions::install() doesn't exist");
}
// test the CustomActions::list method
if (tmp.list) {
	trace("PASSED: CustomActions::list() exists");
} else {
	trace("FAILED: CustomActions::list() doesn't exist");
}
// test the CustomActions::uninstall method
if (tmp.uninstall) {
	trace("PASSED: CustomActions::uninstall() exists");
} else {
	trace("FAILED: CustomActions::uninstall() doesn't exist");
}
