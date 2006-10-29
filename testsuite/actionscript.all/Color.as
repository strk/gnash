// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA

//

// Test case for Color ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Color.as,v 1.5 2006/10/29 18:34:18 rsavoye Exp $";

#include "check.as"

var colorObj = new Color;

// test the Color constuctor
check (colorObj != undefined);

// test the Color::getrgb method
check (colorObj.getrgb != undefined);

// test the Color::gettransform method
check (colorObj.gettransform != undefined);

// test the Color::setrgb method
check (colorObj.setrgb != undefined);

// test the Color::settransform method
check (colorObj.settransform != undefined);
