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

// Test case for NetStream ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: NetStream.as,v 1.5 2006/10/29 18:34:18 rsavoye Exp $";

#include "check.as"

var netstreamObj = new NetStream;

// test the NetStream constuctor
check (netstreamObj != undefined);

// test the NetStream::close method
check (netstreamObj.close != undefined);
// test the NetStream::pause method
check (netstreamObj.pause != undefined);
// test the NetStream::play method
check (netstreamObj.play != undefined);
// test the NetStream::seek method
check (netstreamObj.seek != undefined);
// test the NetStream::setbuffertime method
check (netstreamObj.setbuffertime != undefined);
