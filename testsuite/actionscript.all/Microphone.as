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

// Test case for Microphone ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Microphone;

// test the Microphone constuctor
if (tmp) {
	trace("PASSED: Microphone::Microphone() constructor");
} else {
	trace("FAILED: Microphone::Microphone()");		
}

// test the Microphone::get method
if (tmp.get) {
	trace("PASSED: Microphone::get() exists");
} else {
	trace("FAILED: Microphone::get() doesn't exist");
}
// test the Microphone::setgain method
if (tmp.setgain) {
	trace("PASSED: Microphone::setgain() exists");
} else {
	trace("FAILED: Microphone::setgain() doesn't exist");
}
// test the Microphone::setrate method
if (tmp.setrate) {
	trace("PASSED: Microphone::setrate() exists");
} else {
	trace("FAILED: Microphone::setrate() doesn't exist");
}
// test the Microphone::setsilencelevel method
if (tmp.setsilencelevel) {
	trace("PASSED: Microphone::setsilencelevel() exists");
} else {
	trace("FAILED: Microphone::setsilencelevel() doesn't exist");
}
// test the Microphone::setuseechosuppression method
if (tmp.setuseechosuppression) {
	trace("PASSED: Microphone::setuseechosuppression() exists");
} else {
	trace("FAILED: Microphone::setuseechosuppression() doesn't exist");
}
