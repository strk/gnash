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

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


var tmp = new DejaGnu();

// test the constuctor
if (tmp) {
    tmp.pass("DejaGnu() constructor");
} else {
    tmp.fail("DejaGnu() constructor");
}

if (tmp) {
    
if (tmp.pass) {
   tmp. pass("DejaGnu::pass exists");
} else {
    tmp.fail("DejaGnu::pass doesn't exist");
}

if (tmp.fail) {
    tmp.pass("DejaGnu::fail exists");
} else {
    tmp.fail("DejaGnu::fail doesn't exist");
}

} else {
    trace("UNTESTED: extensions not built!");
}
