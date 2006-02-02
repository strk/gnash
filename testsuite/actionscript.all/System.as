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

// Test case for System ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new System;

// test the System constuctor
if (tmp) {
	trace("PASSED: System::System() constructor");
} else {
	trace("FAILED: System::System()");		
}

// test the System::security.allowdomain method
if (tmp.security.allowdomain) {
	trace("PASSED: System::security.allowdomain() exists");
} else {
	trace("FAILED: System::security.allowdomain() doesn't exist");
}
// test the System::security.allowinsecuredomain method
if (tmp.security.allowinsecuredomain) {
	trace("PASSED: System::security.allowinsecuredomain() exists");
} else {
	trace("FAILED: System::security.allowinsecuredomain() doesn't exist");
}
// test the System::security.loadpolicyfile method
if (tmp.security.loadpolicyfile) {
	trace("PASSED: System::security.loadpolicyfile() exists");
} else {
	trace("FAILED: System::security.loadpolicyfile() doesn't exist");
}
// test the System::setclipboard method
if (tmp.setclipboard) {
	trace("PASSED: System::setclipboard() exists");
} else {
	trace("FAILED: System::setclipboard() doesn't exist");
}
// test the System::showsettings method
if (tmp.showsettings) {
	trace("PASSED: System::showsettings() exists");
} else {
	trace("FAILED: System::showsettings() doesn't exist");
}
