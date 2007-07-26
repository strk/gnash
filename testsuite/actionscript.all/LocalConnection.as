// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for LocalConnection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: LocalConnection.as,v 1.16 2007/07/26 03:41:18 strk Exp $";

#include "dejagnu.as"

#if OUTPUT_VERSION < 6

check_equals(LocalConnection, undefined);

#else // OUTPUT_VERSION >= 6

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

// There's no such 'getname' method of LocalConnection !!!
check_equals(typeof(tmp.getname), 'undefined');
check_equals(typeof(tmp.getName), 'undefined');
check_equals(typeof(tmp.getsize), 'undefined');
check_equals(typeof(tmp.getSize), 'undefined');
check_equals(typeof(tmp.getallocated), 'undefined');
check_equals(typeof(tmp.getAllocated), 'undefined');
check_equals(typeof(LocalConnection.getname), 'undefined');
check_equals(typeof(LocalConnection.getName), 'undefined');
check_equals(typeof(LocalConnection.getsize), 'undefined');
check_equals(typeof(LocalConnection.getSize), 'undefined');
check_equals(typeof(LocalConnection.getallocated), 'undefined');
check_equals(typeof(LocalConnection.getAllocated), 'undefined');

// NOTE: This test will fail if a shared memory segment of the same
// name exists. So the first time it'll pass, then it'll fail.
if (ret == true) {
	pass("LocalConnection::connect()");
} else {	
	fail("LocalConnection::connect()");
}

// Close the connection, and then check the state
ret = tmp.close();
xcheck(ret);

// There's no such 'exists' method of LocalConnection !!!
check_equals(typeof(tmp.exists), 'undefined');


#endif // OUTPUT_VERSION >= 6

totals();
