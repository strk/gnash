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

// Test case for Date ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Date.as,v 1.11 2006/11/24 08:42:44 strk Exp $";

#include "check.as"

check (Date);

// test the Date constuctor
var date = new Date;
check (date);

// test methods existance
check (date.getDate != undefined);
check (date.getDay != undefined);
check (date.getFullYear != undefined);
check (date.getHours != undefined);
check (date.getMilliseconds != undefined);
check (date.getMinutes != undefined);
check (date.getMonth != undefined);
check (date.getSeconds != undefined);
check (date.getTime != undefined);
check (date.getTimezoneOffset != undefined);
check (date.getUTCDate != undefined);
check (date.getUTCDay != undefined);
check (date.getUTCFullYear != undefined);
check (date.getUTCHours != undefined);
check (date.getUTCMilliseconds != undefined);
check (date.getUTCMinutes != undefined);
check (date.getUTCMonth != undefined);
check (date.getUTCSeconds != undefined);
check (date.getYear != undefined);
check (date.setDate != undefined);
check (date.setFullYear != undefined);
check (date.setHours != undefined);
check (date.setMilliseconds != undefined);
check (date.setMinutes != undefined);
check (date.setMonth != undefined);
check (date.setSeconds != undefined);
check (date.setTime != undefined);
check (date.setUTCDate != undefined);
check (date.setUTCFullYear != undefined);
check (date.setUTCHours != undefined);
check (date.setUTCMilliseconds != undefined);
check (date.setUTCMinutes != undefined);
check (date.setUTCMonth != undefined);
check (date.setUTCSeconds != undefined);
check (date.setYear != undefined);
check (date.toString != undefined);
xcheck_equals (date.UTC, undefined);

#if OUTPUT_VERSION > 6
xcheck(Date.UTC != undefined);

// From SWF 7 up methods are case-sensitive !
xcheck_equals (date.getdate, undefined);
xcheck_equals (date.getday, undefined);
xcheck_equals (date.getfullYear, undefined);
xcheck_equals (date.gethours, undefined);
xcheck_equals (date.getmilliseconds, undefined);
xcheck_equals (date.getminutes, undefined);
xcheck_equals (date.getmonth, undefined);
xcheck_equals (date.getseconds, undefined);
xcheck_equals (date.gettime, undefined);
xcheck_equals (date.gettimezoneOffset, undefined);
xcheck_equals (date.getUTCdate, undefined);
xcheck_equals (date.getUTCday, undefined);
xcheck_equals (date.getUTCfullYear, undefined);
xcheck_equals (date.getUTChours, undefined);
xcheck_equals (date.getUTCmilliseconds, undefined);
xcheck_equals (date.getUTCminutes, undefined);
xcheck_equals (date.getUTCmonth, undefined);
xcheck_equals (date.getUTCseconds, undefined);
xcheck_equals (date.getyear, undefined);
xcheck_equals (date.setdate, undefined);
xcheck_equals (date.setfullYear, undefined);
xcheck_equals (date.sethours, undefined);
xcheck_equals (date.setmilliseconds, undefined);
xcheck_equals (date.setminutes, undefined);
xcheck_equals (date.setmonth, undefined);
xcheck_equals (date.setseconds, undefined);
xcheck_equals (date.settime, undefined);
xcheck_equals (date.setUTCdate, undefined);
xcheck_equals (date.setUTCfullYear, undefined);
xcheck_equals (date.setUTChours, undefined);
xcheck_equals (date.setUTCmilliseconds, undefined);
xcheck_equals (date.setUTCminutes, undefined);
xcheck_equals (date.setUTCmonth, undefined);
xcheck_equals (date.setUTCseconds, undefined);
xcheck_equals (date.setyear, undefined);
xcheck_equals (date.tostring, undefined);

#endif
