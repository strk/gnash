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

// Test case for Video ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Video;

// test the Video constuctor
if (tmp) {
	trace("PASSED: Video::Video() constructor");
} else {
	trace("FAILED: Video::Video()");		
}

// test the Video::attach method
if (tmp.attach) {
	trace("PASSED: Video::attach() exists");
} else {
	trace("FAILED: Video::attach() doesn't exist");
}
// test the Video::clear method
if (tmp.clear) {
	trace("PASSED: Video::clear() exists");
} else {
	trace("FAILED: Video::clear() doesn't exist");
}
