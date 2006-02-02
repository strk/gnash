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

// Test case for LocalConnection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new LocalConnection;

// test the LocalConnection constuctor
if (tmp) {
	trace("PASSED: LocalConnection::LocalConnection() constructor");
} else {
	trace("FAILED: LocalConnection::LocalConnection()");		
}

// test the LocalConnection::close method
if (tmp.close) {
	trace("PASSED: LocalConnection::close() exists");
} else {
	trace("FAILED: LocalConnection::close() doesn't exist");
}
// test the LocalConnection::connect method
if (tmp.connect) {
	trace("PASSED: LocalConnection::connect() exists");
} else {
	trace("FAILED: LocalConnection::connect() doesn't exist");
}
// test the LocalConnection::domain method
if (tmp.domain) {
	trace("PASSED: LocalConnection::domain() exists");
} else {
	trace("FAILED: LocalConnection::domain() doesn't exist");
}
// test the LocalConnection::send method
if (tmp.send) {
	trace("PASSED: LocalConnection::send() exists");
} else {
	trace("FAILED: LocalConnection::send() doesn't exist");
}
