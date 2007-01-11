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

// Test case for Stage ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Stage.as,v 1.7 2007/01/11 12:47:26 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION > 5
check_equals (typeof(Stage), 'object');
#else // Gnash doesn't register a Stage object if SWF < 6 !
xcheck_equals (typeof(Stage), 'object');
#endif

var stageObj = new Stage;
check_equals (typeof(stageObj), 'undefined');

#if OUTPUT_VERSION > 5

// test the Stage::addlistener method
check_equals (typeof(Stage.addListener), 'function');
// test the Stage::removelistener method
check_equals (typeof(Stage.removeListener), 'function');

#else // OUTPUT_VERSION <= 5

check_equals (typeof(Stage.addListener), 'undefined');
check_equals (typeof(Stage.removeListener), 'undefined');

#endif // OUTPUT_VERSION <= 5

