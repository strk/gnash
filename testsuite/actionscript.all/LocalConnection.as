// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

rcsid="LocalConnection.as";
#include "check.as"

snd = new LocalConnection();

#if OUTPUT_VERSION < 6

check_equals(LocalConnection, undefined);
totals(1);

#else // OUTPUT_VERSION >= 6

check_equals(LocalConnection.prototype.__proto__, Object.prototype);
check (LocalConnection.prototype.hasOwnProperty("send"));
check (LocalConnection.prototype.hasOwnProperty("connect"));
check (LocalConnection.prototype.hasOwnProperty("close"));
check (LocalConnection.prototype.hasOwnProperty("domain"));
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

// Get the domain. This should be based on the domain of the SWF.
var domain = rec.domain();
check_equals (domain, "localhost");

// If the listen() times out waiting for a connection, it'll set the
// main socket file descriptor to an error condition, although the
// initial file descriptor returned by bind is still fine, since we
// could always (in a normal application) check later for incoming
// connections.

// Test connect. Return based on argument, not result of connection.
// Anything but a string returns false. The onStatus function is not called.
statuses = new Array;

rec.onStatus = function(obj) {
    statuses.push(obj.code);
};

ret = rec.connect();
check_equals(ret, false);
check_equals(statuses.length, 0);

ret = rec.connect(3);
check_equals(ret, false);
check_equals(statuses.length, 0);

ret = rec.connect(undefined);
check_equals(ret, false);
check_equals(statuses.length, 0);

ret = rec.connect("");
check_equals(ret, false);
check_equals(statuses.length, 0);

ret = rec.connect("string", 7);
check_equals(ret, true);
check_equals(statuses.length, 0);

ret = rec.connect("string");
check_equals(ret, false);
check_equals(statuses.length, 0);

rec.close();

ret = rec.connect("string");
check_equals(ret, true);
check_equals(statuses.length, 0);

rec.close();
result = rec.connect("lc_test");
check_equals (rec.domain(), "localhost");

// NOTE: This test will fail if already connected.
// So the first time it'll pass, then it'll fail.
check_equals (result, true);

// Checks only for syntactical correctness, not success

result = snd.send();
check_equals (result, false);

result = snd.send(3);
check_equals (result, false);

result = snd.send("string", "string", "string", "string");
check_equals (result, true);



result = snd.send("lc_test", "testfunc", "val");
check_equals (result, true);

// The function name may not be send or any other LC property.
result = snd.send("lc_test", "send");
check_equals (result, false);
result = snd.send("lc_test", "onStatus");
check_equals (result, false);
// Numbers are also bad
result = snd.send("lc_test", 1);
check_equals (result, false);
// undefined
result = snd.send("lc_test", funcname);
check_equals (result, false);

result = snd.send("lc_test", "Send");
 check_equals(result, false);

result = snd.send("lc_test", "DOMAIn");
 check_equals(result, false);

result = snd.send("lc_test", "close");
 check_equals(result, false);

result = snd.send("lc_test", "conNeCt");
 check_equals(result, false);

result = snd.send("lc_test", "onStatus");
 check_equals(result, false);

result = snd.send("lc_test", "ALLOWDOMAIN");
 check_equals(result, false);

result = snd.send("lc_test", "");
 check_equals(result, false);

// But anything else is fine.
result = snd.send("lc_test", "8");
 check_equals(result, true);

result = snd.send("lc_test", "ß");
 check_equals(result, true);

result = snd.send("lc_test", "&");
 check_equals(result, true);

result = snd.send("lc_test", ".");
 check_equals(result, true);

result = snd.send("lc_test", "g.");
 check_equals(result, true);

result = snd.send("lc_test", "getSeconds");
check_equals (result, true);
funcname = "onFullScreen";
result = snd.send("lc_test", funcname);
check_equals (result, true);

rec.close();

totals(52);

#endif // OUTPUT_VERSION >= 6


