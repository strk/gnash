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

// Test case for NetConnection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="NetConnection.as";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(NetConnection, undefined);
check_totals(1);

#else // OUTPUT_VERSION >= 6

check(NetConnection.prototype.hasOwnProperty("call"));
check(NetConnection.prototype.hasOwnProperty("connect"));
check(NetConnection.prototype.hasOwnProperty("addHeader"));
check(NetConnection.prototype.hasOwnProperty("close"));

check(!NetConnection.prototype.hasOwnProperty("isConnected"));
check(!NetConnection.prototype.hasOwnProperty("uri"));

check_equals(typeof(NetConnection), 'function');
check_equals(typeof(NetConnection.prototype), 'object');
check_equals(typeof(NetConnection.prototype.isConnected), 'undefined');
check_equals(typeof(NetConnection.prototype.connect), 'function');

var tmp = new NetConnection;
check_equals(typeof(tmp), 'object');
check_equals(tmp.__proto__, NetConnection.prototype);
check(tmp instanceof NetConnection);
check_equals(typeof(tmp.isConnected), 'boolean');
check_equals(typeof(tmp.uri), 'undefined');
check_equals(tmp.uri, undefined);
check_equals(tmp.isConnected, false);

tmp.isConnected = true;
check_equals(tmp.isConnected, false);

tmp.isConnected = 56;
check_equals(tmp.isConnected, false);

#if 0
// Provided this connection is allowed, the player creates a connection.
// Because the Adobe player has a different sandbox method from Gnash,
// we can't test this connection *and* the blacklisted one.

// Starting this connection also causes different results later.

// test the NetConnection::connect method
if ( ! tmp.connect("rtmp://www.mediacollege.com/flash/media-player/testclip-4sec.flv") )
{
	// FIXME: this would fail in the reference player too...
	xfail("NetConnection::connect() didn't initialize correctly");
}
else
{
	pass("NetConnection::connect() initialized correctly");
}
#endif

statuses = new Array;
tmp.onStatus = function(info) {
    result = info.code;
    level = info.level;
    statuses.push(info.code);
};

result = "";
level = "";

statuses = new Array();
ret = tmp.connect();
check_equals(ret, undefined);
check_equals(tmp.isConnected, false);
check_equals(result, "");
check_equals(level, "");
check_equals(statuses.toString(), "");

statuses = new Array();
ret = tmp.connect("");
check_equals(ret, false);
check_equals(tmp.isConnected, false);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "");
check_equals(statuses.toString(), "NetConnection.Connect.Failed");

ret = tmp.connect("null");
check_equals(ret, false);
check_equals(tmp.isConnected, false);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "null");

ret = tmp.connect(null, "another argument");
check_equals(ret, true);
check_equals(tmp.isConnected, true);
check_equals(result, "NetConnection.Connect.Success");
check_equals(level, "status");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "null");

// Can't set
tmp.uri = 6;
check_equals(tmp.uri, "null");


statuses = new Array();
ret = tmp.connect(1);
check_equals(ret, false);
check_equals(tmp.isConnected, false);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "1");
check_equals(statuses.toString(),
            "NetConnection.Connect.Closed,NetConnection.Connect.Failed");

statuses = new Array();
ret = tmp.connect("string");
check_equals(ret, false);
check_equals(tmp.isConnected, false);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "string");
check_equals(statuses.toString(),
            "NetConnection.Connect.Failed");

ret = tmp.connect(undefined);

#if OUTPUT_VERSION > 6
check_equals(ret, true);
check_equals(tmp.isConnected, true);
check_equals(result, "NetConnection.Connect.Success");
check_equals(level, "status");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "undefined");
#else
check_equals(ret, false);
check_equals(tmp.isConnected, false);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");
check_equals(typeof(tmp.uri), "string");
check_equals(tmp.uri, "");
#endif

statuses = new Array;
ret = tmp.connect(null);
check_equals(ret, true);
check_equals(tmp.isConnected, true);
check_equals(result, "NetConnection.Connect.Success");
check_equals(level, "status");

// This depends on whether isConnected() was true or not.
#if OUTPUT_VERSION > 6
check_equals(statuses.toString(),
        "NetConnection.Connect.Closed,NetConnection.Connect.Success");
#else
check_equals(statuses.toString(),
        "NetConnection.Connect.Success");
#endif
// The pp and Gnash sandboxes behave differently. The pp rejects any
// network connection from filesystem-loaded SWFs unless the SWF location
// is added to the player configuration file. This server is blacklisted
// in the testsuite gnashrc file, so Gnash should refuse to load this too.
// The test should work on both players for a SWF loaded from anywhere but
// www.blacklistedserver.org (domain still available, in case anyone wants to
// mess up the test).
ret = tmp.connect("http://www.blacklistedserver.org");
check_equals(ret, false);
check_equals(tmp.isConnected, false);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");


// Close() doesn't reset uri if no connection is made. It does seem to
// reset it if a connection is genuinely closed.
tmp.close();
check_equals(tmp.uri, "http://www.blacklistedserver.org");

// Test call()

statuses = new Array;
// No Call onStatus event when not connected.
ret = tmp.call("o");
check_equals(ret, undefined);
check_equals(statuses.length, 0);

// No Call onStatus event when connected with null.
tmp.connect(null);
ret = tmp.call("o");
check_equals(ret, undefined);
check_equals(statuses.length, 1);
check_equals(result, "NetConnection.Connect.Success");
check_equals(typeof(tmp.uri), 'string');
check_equals(tmp.uri, 'null');

// Check onStatus object.

nc = new NetConnection;
nc.onStatus = function(info) {
    infoObj = info;
};

nc.connect(6);
nc.onStatus = undefined;
check_equals(infoObj.code, "NetConnection.Connect.Failed");

// It is a full object
check(infoObj instanceof Object);
check_equals(infoObj.toString(), "[object Object]");

// Check whether the original object is modified on a new connect attempt.
nc.connect(null);
check_equals(infoObj.code, "NetConnection.Connect.Failed");

/// Check call

result = "";
level = "";

nc.onStatus = function(info) {
    result = info.code;
    level = info.level;
    statuses.push(info.code);
};

// Sanity check
check(nc.isConnected);

ret = nc.call();
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "");
check_equals(level, "");

ret = nc.call(1);
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "");
check_equals(level, "");

ret = nc.call("string");
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "");
check_equals(level, "");

// NetConnection close

statuses = new Array;
check(nc.isConnected);
ret = nc.close();
check_equals(nc.isConnected, false);
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "NetConnection.Connect.Closed");
check_equals(level, "status");

ret = nc.close();
check_equals(nc.isConnected, false);
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "NetConnection.Connect.Closed");
check_equals(level, "status");

// Only called once
check_equals(statuses.toString(), "NetConnection.Connect.Closed");

nc.connect(1);
check_equals(nc.isConnected, false);
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");
ret = nc.close();
check_equals(nc.isConnected, false);
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);
check_equals(result, "NetConnection.Connect.Failed");
check_equals(level, "error");

check_totals(120);



#endif // OUTPUT_VERSION >= 7
