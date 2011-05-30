// 
//   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
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

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

// The attempted connections here should always fail. See
// misc-ming.all/XMLSocketTest.c and testsuite/XmlSocketServer.pl
// for tests with a running server.

rcsid="XMLSocket.as";
#include "check.as"

#if OUTPUT_VERSION < 6
XMLSocket.prototype.hasOwnProperty = ASnative(101, 5);
#endif

check(XMLSocket.prototype.hasOwnProperty("connect"));
check(XMLSocket.prototype.hasOwnProperty("send"));
check(XMLSocket.prototype.hasOwnProperty("close"));
check(XMLSocket.prototype.hasOwnProperty("onData"));

check(!XMLSocket.prototype.hasOwnProperty("onXML"));
check(!XMLSocket.prototype.hasOwnProperty("onConnect"));
check(!XMLSocket.prototype.hasOwnProperty("onClose"));


check_equals(typeof(XMLSocket), 'function');
check_equals(typeof(XMLSocket.connect), 'undefined');
check_equals(typeof(XMLSocket.close), 'undefined');
check_equals(typeof(XMLSocket.send), 'undefined');
check_equals(typeof(XMLSocket.Connected), 'undefined');
check_equals(typeof(XMLSocket.connected), 'undefined');

check_equals(typeof(XMLSocket.prototype.connect), 'function');
check_equals(typeof(XMLSocket.prototype.close), 'function');
check_equals(typeof(XMLSocket.prototype.send), 'function');
check_equals(typeof(XMLSocket.prototype.Connected), 'undefined');
check_equals(typeof(XMLSocket.prototype.connected), 'undefined');


socketObj = new XMLSocket;

// The default onData handler calls onXML after parsing the code
check_equals(typeof(socketObj.onData), 'function');

check_equals(typeof(socketObj), 'object');
check_equals(socketObj.__proto__, XMLSocket.prototype);
check( ! socketObj.hasOwnProperty('connect') );
check( ! socketObj.hasOwnProperty('close') );
check( ! socketObj.hasOwnProperty('send') );

socketObj.secret = 4;

socketObj.onConnect = function(success) {
	check_equals(this.secret, 4);
	if ( success )
	{
		note("XMLSocket.onConnect(success) called");
	}
	else
	{
		note("XMLSocket.onConnect(failure) called");
	}
};


socketObj.onXML = function(x) {
	check_equals(this.secret, 4);
	check_equals(arguments.length, 1);
	check(x instanceof XML);
	note("XMLSocket.onXML() called with a "+typeof(arguments[0])+" as arg");
    note("Parsed XML: "+x.toString());
};

socketObj.onClose = function() {
	check_equals(this.secret, 4);
	note("XMLSocket.onClose() called with "+arguments.length);
};

host = 'madeuphost';
port = 1090929898;

// Connect should fail
check_equals(socketObj.connect(host, port), false);
// And again
check_equals(socketObj.connect(host, port), false);

// Close returns undefined, and we'd like not to crash if we call
// close when not connected.
ret = socketObj.close();
check_equals(ret, undefined);
// And again.
ret = socketObj.close();
check_equals(ret, undefined);

ret = socketObj.send("This won't work'");
check_equals(ret, undefined);

totals(29);
