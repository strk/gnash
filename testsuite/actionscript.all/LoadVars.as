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

// Test case for LoadVars ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: LoadVars.as,v 1.7 2007/01/11 11:26:50 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 6

xcheck_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
xcheck_equals (typeof(loadvarsObj), 'object');

#else // OUTPUT_VERSION >= 6

check_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
check_equals (typeof(loadvarsObj), 'object');

// test the LoadVars::addrequestheader method
check_equals (typeof(loadvarsObj.addRequestHeader), 'function');
// test the LoadVars::decode method
check_equals (typeof(loadvarsObj.decode), 'function');
// test the LoadVars::getbytesloaded method
check_equals (typeof(loadvarsObj.getBytesLoaded), 'function');
// test the LoadVars::getbytestotal method
check_equals (typeof(loadvarsObj.getBytesTotal), 'function');
// test the LoadVars::load method
check_equals (typeof(loadvarsObj.load), 'function');
// test the LoadVars::send method
check_equals (typeof(loadvarsObj.send), 'function');
// test the LoadVars::sendandload method
check_equals (typeof(loadvarsObj.sendAndLoad), 'function');
// test the LoadVars::tostring method
check_equals (typeof(loadvarsObj.toString), 'function');

#endif //  OUTPUT_VERSION >= 6
