// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: XMLSocket.as,v 1.1 2007/03/30 14:51:38 strk Exp $";

#include "check.as"

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
#if OUTPUT_VERSION >= 6
check(XMLSocket.prototype.hasOwnProperty('connect'));
check(XMLSocket.prototype.hasOwnProperty('close'));
check(XMLSocket.prototype.hasOwnProperty('send'));
#endif

socketObj = new XMLSocket;

check_equals(typeof(socketObj), 'object');
check_equals(socketObj.__proto__, XMLSocket.prototype);
check( ! socketObj.hasOwnProperty('connect') );
check( ! socketObj.hasOwnProperty('close') );
check( ! socketObj.hasOwnProperty('send') );
