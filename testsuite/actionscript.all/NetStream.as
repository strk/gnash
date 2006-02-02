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

// Test case for NetStream ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new NetStream;

// test the NetStream constuctor
if (tmp) {
	trace("PASSED: NetStream::NetStream() constructor");
} else {
	trace("FAILED: NetStream::NetStream()");		
}

// test the NetStream::close method
if (tmp.close) {
	trace("PASSED: NetStream::close() exists");
} else {
	trace("FAILED: NetStream::close() doesn't exist");
}
// test the NetStream::pause method
if (tmp.pause) {
	trace("PASSED: NetStream::pause() exists");
} else {
	trace("FAILED: NetStream::pause() doesn't exist");
}
// test the NetStream::play method
if (tmp.play) {
	trace("PASSED: NetStream::play() exists");
} else {
	trace("FAILED: NetStream::play() doesn't exist");
}
// test the NetStream::seek method
if (tmp.seek) {
	trace("PASSED: NetStream::seek() exists");
} else {
	trace("FAILED: NetStream::seek() doesn't exist");
}
// test the NetStream::setbuffertime method
if (tmp.setbuffertime) {
	trace("PASSED: NetStream::setbuffertime() exists");
} else {
	trace("FAILED: NetStream::setbuffertime() doesn't exist");
}
