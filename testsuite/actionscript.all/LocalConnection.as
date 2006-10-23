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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

// Test case for LocalConnection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "dejagnu.as"

var tmp = new LocalConnection;

// test the LocalConnection constuctor
if (tmp) {
	pass("LocalConnection::LocalConnection() constructor");
} else {
	fail("LocalConnection::LocalConnection()");		
}

// test the LocalConnection::close method
if (tmp.close) {
	pass("LocalConnection::close() exists");
} else {
	fail("LocalConnection::close() doesn't exist");
}
// test the LocalConnection::connect method
if (tmp.connect) {
	pass("LocalConnection::connect() exists");
} else {
	fail("LocalConnection::connect() doesn't exist");
}
// test the LocalConnection::domain method
if (tmp.domain) {
	pass("LocalConnection::domain() exists");
} else {
	fail("LocalConnection::domain() doesn't exist");
}
// test the LocalConnection::send method
if (tmp.send) {
	pass("LocalConnection::send() exists");
} else {
	fail("LocalConnection::send() doesn't exist");
}

// Get the domain. By default this should be "localhost" because we
// haven't made any connections yet,
var domain = tmp.domain();
if (domain  == "localhost") {
	pass("LocalConnection::domain() returned localhost");
} else {
	fail("LocalConnection::domain() returned localhost");
}


// If the listen() times out waiting for a connection, it'll set the
// main socket file descriptor to an error condition, although the
// initial file descriptor returned by bind is still fine, since we
// could always (in a normal application) check later for incoming
// connections.

tmp.close();
var ret = tmp.connect("lc_test");

// NOTE: This test will fail if a shared memory segment of the same
// name exists. So the first time it'll pass, then it'll fail.
if ((tmp.getname() == "/lc_test") && (ret == true)) {
	pass("LocalConnection::connect()");
} else {	
	fail("LocalConnection::connect()");
}

// Close the connection, and then check the state
tmp.close();
if (tmp.exists() == false) {
	pass("LocalConnection::close()");
} else {
	fail("LocalConnection::close()");
}

totals();
