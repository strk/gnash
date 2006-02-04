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

// Test case for NetConnection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new NetConnection;


// test the NetConnection constuctor
if (tmp) {
	trace("PASSED: NetConnection::NetConnection() constructor");
} else {
	trace("FAILED: NetConnection::NetConnection()");		
}

// test the NetConnection::connect method
if (tmp.connect) {
	trace("PASSED: NetConnection::connect() exists");
} else {
	trace("FAILED: NetConnection::connect() doesn't exist");
}

// test the NetConnection::connect method
tmp.connect();
tmp.connect("rmtp://www.mediacollege.com/flash/media-player/testclip-4sec.flv");

// Check the see if the URL got parsed right
var url = tmp.geturl();
var protocol = tmp.getprotocol();
var host = tmp.gethost();
var port = tmp.getport();
var path = tmp.getpath();
if (url == "rmtp://www.mediacollege.com/flash/media-player/testclip-4sec.flv"
	 && protocol == "rmtp"
	 && host == "www.mediacollege.com"
	 && path == "/flash/media-player/testclip-4sec.flv") {
	trace("PASSED: NetConnection::connect() initialized correctly");
} else {
	trace("FAILED: NetConnection::connect() didn't initialized correctly");
}
