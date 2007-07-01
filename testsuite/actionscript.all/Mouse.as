// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for Mouse ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Mouse.as,v 1.8 2007/07/01 10:54:39 bjacques Exp $";

#include "check.as"

// Mouse object can't be instanciated !
var mouseObj = new Mouse;
xcheck_equals(mouseObj, undefined);

// test the Mouse::addlistener method
check (Mouse.addlistener != undefined);
// test the Mouse::hide method
check (Mouse.hide != undefined);
// test the Mouse::removelistener method
check (Mouse.removelistener != undefined);
// test the Mouse::show method
check (Mouse.show != undefined);
