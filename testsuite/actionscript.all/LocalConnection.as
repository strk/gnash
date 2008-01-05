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

rcsid="$Id: LocalConnection.as,v 1.18 2008/01/05 03:55:00 rsavoye Exp $";

#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(LocalConnection, undefined);

#else // OUTPUT_VERSION >= 6

var tmp = new LocalConnection;

// test the LocalConnection constuctor
check_equals (typeof(tmp), 'object');

// test the LocalConnection::close method
check_equals (typeof(tmp.close), 'function');

// test the LocalConnection::connect method
// FIXME: this should not be failing as later we find the function
// actually works!
xcheck_equals (typeof(tmp.connnect), 'function');

// test the LocalConnection::domain method
check_equals (typeof(tmp.domain), 'function');

// test the LocalConnection::send method
check_equals (typeof(tmp.send), 'function');

// Get the domain. By default this should be "localhost" because we
// haven't made any connections yet,
var domain = tmp.domain();
check_equals (domain, "localhost");

// If the listen() times out waiting for a connection, it'll set the
// main socket file descriptor to an error condition, although the
// initial file descriptor returned by bind is still fine, since we
// could always (in a normal application) check later for incoming
// connections.

tmp.close();
var ret = tmp.connect("lc_test");

// NOTE: This test will fail if a shared memory segment of the same
// name exists. So the first time it'll pass, then it'll fail.
check_equals (ret, true);

tmp.close();

#endif // OUTPUT_VERSION >= 6

totals();
