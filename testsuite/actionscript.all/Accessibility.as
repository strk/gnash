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

// Test case for Accessibility ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Accessibility.as";
#include "check.as"

check_equals ( typeof(Accessibility), 'object' );

// Accessibility object can't be instantiated !
var accObj = new Accessibility;
check_equals(accObj, undefined);

check_equals (typeof(Accessibility.__proto__), 'object');
check_equals (Accessibility.__proto__, Object.prototype);

#if OUTPUT_VERSION > 5

check_equals ( typeof(Accessibility.isActive), 'function' );
check_equals ( typeof(Accessibility.updateProperties), 'function' );
check_equals ( typeof(Accessibility.sendEvent), 'function' );

check(Accessibility.hasOwnProperty("isActive"));
check(Accessibility.hasOwnProperty("updateProperties"));
check(Accessibility.hasOwnProperty("sendEvent"));

xcheck_equals ( typeof(Accessibility.isActive()), 'boolean' );

#else

// SWF 5:
xcheck_equals ( typeof(Accessibility.isActive), 'undefined' );
xcheck_equals ( typeof(Accessibility.updateProperties), 'undefined' );
xcheck_equals ( typeof(Accessibility.sendEvent), 'undefined' );

#endif // OUTPUT_VERSION > 5

// Methods return void (just undefined in SWF 5).
check_equals ( typeof(Accessibility.updateProperties()), 'undefined' );
check_equals ( typeof(Accessibility.sendEvent()), 'undefined' );

#if OUTPUT_VERSION > 5
check_totals(13);
#else
check_totals(9);
#endif
