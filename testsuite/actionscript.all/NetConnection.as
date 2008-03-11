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

// Test case for NetConnection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: NetConnection.as,v 1.18 2008/03/11 19:31:47 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(NetConnection, undefined);
check_totals(1);

#else // OUTPUT_VERSION >= 7

check_equals(typeof(NetConnection), 'function');
check_equals(typeof(NetConnection.prototype), 'object');
check_equals(typeof(NetConnection.prototype.isConnected), 'undefined');
check_equals(typeof(NetConnection.prototype.connect), 'function');
// TODO: add tests for all interfaces

var tmp = new NetConnection;
check_equals(typeof(tmp), 'object');
check_equals(tmp.__proto__, NetConnection.prototype);
check(tmp instanceof NetConnection);
xcheck_equals(typeof(tmp.isConnected), 'boolean');
xcheck_equals(tmp.isConnected, false);
// TODO: add tests for all properties

// test the NetConnection::connect method
tmp.connect();
if ( ! tmp.connect("rtmp://www.mediacollege.com/flash/media-player/testclip-4sec.flv") )
{
	// FIXME: this would fail in the reference player too...
	xfail("NetConnection::connect() didn't initialized correctly");
}
else
{
	pass("NetConnection::connect() initialized correctly");
}

check_totals(10);

#endif // OUTPUT_VERSION >= 7
