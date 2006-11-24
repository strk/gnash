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

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: test.as,v 1.1 2006/11/24 04:45:05 rsavoye Exp $";

var tmp = new DejaGnu();

// test the XML constuctor
if (tmp) {
    pass("DejaGnu() constructor");
} else {
    fail("DejaGnu() constructor");
}

if (tmp.pass) {
    trace("DejaGnu::pass exists");
} else {
    trace("DejaGnu::pass doesn't exist");
}

if (tmp.fail) {
    trace("DejaGnu::fail exists");
} else {
    trace("DejaGnu::fail doesn't exist");
}
    
