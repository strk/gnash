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

// Test case for lirc ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: test.as,v 1.1 2007/09/15 16:42:55 rsavoye Exp $";

var remote = new Lirc();

// test the constuctor
if (remote) {
    trace("Lirc() constructor works");
} else {
    trace("Lirc() constructor failed");
}

if (!remote) {
    trace("UNTESTED: extensions not built!");
}

// var keyObj = new Key;

var sock = "/tmp/lircd";
if (remote.lirc_init(sock)) {
    var str = "Connected to " + sock;
    trace(str);
} else {
    var str = "ERROR: couldn't connect to " + sock;
    trace(str);
}

var button = remote.lirc_getButton();
trace(button);

button = "QUIT";

while (button != "QUIT") {
    button = remote.lirc_getButton();
    trace(button);
}

