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

rcsid="$Id: Date.as,v 1.13 2007/01/23 17:43:09 strk Exp $";

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
check_equals (date.UTC, undefined);

#if OUTPUT_VERSION > 6
xcheck(Date.UTC != undefined);

// From SWF 7 up methods are case-sensitive !
check_equals (date.getdate, undefined);
check_equals (date.getday, undefined);
check_equals (date.getfullYear, undefined);
check_equals (date.gethours, undefined);
check_equals (date.getmilliseconds, undefined);
check_equals (date.getminutes, undefined);
check_equals (date.getmonth, undefined);
check_equals (date.getseconds, undefined);
check_equals (date.gettime, undefined);
check_equals (date.gettimezoneOffset, undefined);
check_equals (date.getUTCdate, undefined);
check_equals (date.getUTCday, undefined);
check_equals (date.getUTCfullYear, undefined);
check_equals (date.getUTChours, undefined);
check_equals (date.getUTCmilliseconds, undefined);
check_equals (date.getUTCminutes, undefined);
check_equals (date.getUTCmonth, undefined);
check_equals (date.getUTCseconds, undefined);
check_equals (date.getyear, undefined);
check_equals (date.setdate, undefined);
check_equals (date.setfullYear, undefined);
check_equals (date.sethours, undefined);
check_equals (date.setmilliseconds, undefined);
check_equals (date.setminutes, undefined);
check_equals (date.setmonth, undefined);
check_equals (date.setseconds, undefined);
check_equals (date.settime, undefined);
check_equals (date.setUTCdate, undefined);
check_equals (date.setUTCfullYear, undefined);
check_equals (date.setUTChours, undefined);
check_equals (date.setUTCmilliseconds, undefined);
check_equals (date.setUTCminutes, undefined);
check_equals (date.setUTCmonth, undefined);
check_equals (date.setUTCseconds, undefined);
check_equals (date.setyear, undefined);
check_equals (date.tostring, undefined);

#endif
