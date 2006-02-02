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

// Test case for Camera ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Camera;

// test the Camera constuctor
if (tmp) {
	trace("PASSED: Camera::Camera() constructor");
} else {
	trace("FAILED: Camera::Camera()");		
}

// test the Camera::get method
if (tmp.get) {
	trace("PASSED: Camera::get() exists");
} else {
	trace("FAILED: Camera::get() doesn't exist");
}
// test the Camera::setmode method
if (tmp.setmode) {
	trace("PASSED: Camera::setmode() exists");
} else {
	trace("FAILED: Camera::setmode() doesn't exist");
}
// test the Camera::setmotionlevel method
if (tmp.setmotionlevel) {
	trace("PASSED: Camera::setmotionlevel() exists");
} else {
	trace("FAILED: Camera::setmotionlevel() doesn't exist");
}
// test the Camera::setquality method
if (tmp.setquality) {
	trace("PASSED: Camera::setquality() exists");
} else {
	trace("FAILED: Camera::setquality() doesn't exist");
}
