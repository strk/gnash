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


// We need more than one frame to test the connection properly.

rcsid="$Id: LocalConnection.as,v 1.21 2008/05/05 15:26:38 bwy Exp $";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(LocalConnection, undefined);
totals(1);

#else // OUTPUT_VERSION >= 6

xcheck (LocalConnection.prototype.hasOwnProperty("send"));
xcheck (LocalConnection.prototype.hasOwnProperty("connect"));
xcheck (LocalConnection.prototype.hasOwnProperty("close"));
xcheck (LocalConnection.prototype.hasOwnProperty("domain"));
check (! LocalConnection.prototype.hasOwnProperty("allowDomain"));
check (! LocalConnection.prototype.hasOwnProperty("onStatus"));

var rec = new LocalConnection();

// test the LocalConnection constuctor
check_equals (typeof(rec), 'object');

var snd = new LocalConnection();

// Not sure if this is a sensible test.
check(rec != snd);

// test the LocalConnection::close method
check_equals (typeof(rec.close), 'function');

// test the LocalConnection::connect method
check_equals (typeof(rec.connect), 'function');

// test the LocalConnection::domain method
check_equals (typeof(rec.domain), 'function');

// test the LocalConnection::send method
check_equals (typeof(rec.send), 'function');

// Get the domain. By default this should be "localhost" because we
// haven't made any connections yet,
var domain = rec.domain();
check_equals (domain, "localhost");

// If the listen() times out waiting for a connection, it'll set the
// main socket file descriptor to an error condition, although the
// initial file descriptor returned by bind is still fine, since we
// could always (in a normal application) check later for incoming
// connections.

rec.close();
result = rec.connect("lc_test");
xcheck_equals (rec.domain(), "localhost");

// NOTE: This test will fail if a shared memory segment of the same
// name exists. So the first time it'll pass, then it'll fail.
check_equals (result, true);

// Checks only for syntactical correctness, not success
result = snd.send("lc_test", "testfunc", "val");
xcheck_equals (result, true);

// The function name may not be send or any other LC property.
result = snd.send("lc_test", "send");
xcheck_equals (result, false);
result = snd.send("lc_test", "onStatus");
xcheck_equals (result, false);
// Numbers are also bad
result = snd.send("lc_test", 1);
xcheck_equals (result, false);
// undefined
result = snd.send("lc_test", funcname);
xcheck_equals (result, false);


// But anything else is fine.
result = snd.send("lc_test", "getSeconds");
xcheck_equals (result, true);
funcname = "onFullScreen";
result = snd.send("lc_test", funcname);
xcheck_equals (result, true);

rec.close();

totals(22);

#endif // OUTPUT_VERSION >= 6


