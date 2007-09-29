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


// Test case for Color ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Color.as,v 1.10 2007/09/29 16:22:57 strk Exp $";

#include "check.as"

//--------------------------------
// Color was introduced in SWF5
//--------------------------------

check_equals ( typeof(Color), 'function')
var colorObj = new Color;

// test the Color constuctor
check_equals ( typeof(colorObj), 'object')

// test the Color::getrgb method
check_equals ( typeof(colorObj.getRGB), 'function');

// test the Color::gettransform method
check_equals ( typeof(colorObj.getTransform), 'function');

// test the Color::setrgb method
check_equals ( typeof(colorObj.setRGB), 'function');

// test the Color::settransform method
check_equals ( typeof(colorObj.setTransform), 'function');
totals();
