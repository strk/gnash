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

rcsid="$Id: Date.as,v 1.16 2007/02/07 19:08:44 martinwguy Exp $";

#include "check.as"

check (Date);

// test the Date constuctor
var date = new Date(70,1,2,3,4,5,6);
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
check(Date.UTC != undefined);

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

// var date = new Date(70,1,2,3,4,5,6);
trace ("Testing random date");
check_equals (date.getFullYear(), 1970);
check_equals (date.getYear(), 70);
check_equals (date.getMonth(), 1);
check_equals (date.getDate(), 2);
check_equals (date.getHours(), 3);
check_equals (date.getMinutes(), 4);
check_equals (date.getSeconds(), 5);
check_equals (date.getMilliseconds(), 6);

// Test decoding methods
// Check the epoch, 1 Jan 1970
trace ("Testing 1 Jan 1970 UTC");
check_equals (date.setTime(0), 0);
check_equals (date.getTime(), 0);
check_equals (date.getUTCFullYear(), 1970);
check_equals (date.getUTCMonth(), 0);
check_equals (date.getUTCDate(), 1);
check_equals (date.getUTCDay(), 4);	// It was a Thursday
check_equals (date.getUTCHours(), 0);
check_equals (date.getUTCMinutes(), 0);
check_equals (date.getUTCSeconds(), 0);
check_equals (date.getUTCMilliseconds(), 0);
check_equals (date.valueOf(), 0);

trace ("Testing 1 Jan 2000 UTC");
date.setUTCFullYear(2000, 0, 1);
date.setUTCHours(0, 0, 0);
check_equals (date.getUTCFullYear(), 2000);
check_equals (date.getUTCMonth(), 0);
check_equals (date.getUTCDate(), 1);
check_equals (date.getUTCDay(), 6);	// It was a Saturday
check_equals (date.getUTCHours(), 0);
check_equals (date.getUTCMinutes(), 0);
check_equals (date.getUTCSeconds(), 0);
check_equals (date.getUTCMilliseconds(), 0);
check_equals (date.valueOf(), 946684800000.0);	// I asked flashplayer

trace ("Testing 1 Jul 2000 UTC");
date.setUTCFullYear(2000, 6, 1);
date.setUTCHours(0, 0, 0);
check_equals (date.getUTCFullYear(), 2000);
check_equals (date.getUTCMonth(), 6);
check_equals (date.getUTCDate(), 1);
check_equals (date.getUTCDay(), 6);	// It was a Saturday
check_equals (date.getUTCHours(), 0);
check_equals (date.getUTCMinutes(), 0);
check_equals (date.getUTCSeconds(), 0);
check_equals (date.getUTCMilliseconds(), 0);
check_equals (date.valueOf(), 962409600000.0);	// I asked flashplayer

trace ("Testing 1 Jan 2000 localtime");
// The many-argument version of the Date constructor sets the date in localtime
delete date;
var date = new Date(2000, 0, 1, 0, 0, 0, 0);
check_equals (date.getFullYear(), 2000);
check_equals (date.getYear(), 100);
check_equals (date.getMonth(), 0);
check_equals (date.getDate(), 1);
check_equals (date.getDay(), 6);	// It was a Saturday
check_equals (date.getHours(), 0);
check_equals (date.getMinutes(), 0);
check_equals (date.getSeconds(), 0);
check_equals (date.getMilliseconds(), 0);

trace ("Testing 1 Jul 2000 localtime");
// The many-argument version of the Date constructor sets the date in localtime
delete date;
var date = new Date(2000, 6, 1, 0, 0, 0, 0);
check_equals (date.getFullYear(), 2000);
check_equals (date.getYear(), 100);
check_equals (date.getMonth(), 6);
check_equals (date.getDate(), 1);
check_equals (date.getDay(), 6);	// It was a Saturday
check_equals (date.getHours(), 0);
check_equals (date.getMinutes(), 0);
check_equals (date.getSeconds(), 0);
check_equals (date.getMilliseconds(), 0);

// Test TimezoneOffset and local hours by setting a date to the 1 Jan 2000 UTC
// offset by the tzoffset so that localtime should be 00:00 or 01:00
// (according to whether DST is active or not)

trace ("Testing timezone offset");
var tzoffset = new Number(date.getTimezoneOffset());	// in mins east of GMT
trace("timezone offset = " + tzoffset.toString());
date.setUTCFullYear(2000, 0, 1);
date.setUTCHours(0, 0, 0, 0);
date.setTime(date.getTime() - (60000*tzoffset));
check (date.getHours() >= 0);
check (date.getHours() <= 1);

// Test behaviour when you set the time during DST then change the date to
// a non-DST date.
// setUTCHours should preserve the time of day in UTC;
// setHours should preserve the time of day in localtime.
trace ("Testing hour when setting date into/out of DST");
date.setUTCFullYear(2000, 0, 1);
date.setUTCHours(0, 0, 0, 0);
date.setUTCMonth(6);
check_equals (date.getUTCHours(), 0);

date.setUTCFullYear(2000, 6, 1);
date.setUTCHours(0, 0, 0, 0);
date.setUTCMonth(11);
check_equals (date.getUTCHours(), 0);

date.setFullYear(2000, 0, 1);
date.setHours(0, 0, 0, 0);
date.setMonth(6);
check_equals (date.getHours(), 0);

date.setFullYear(2000, 6, 1);
date.setHours(0, 0, 0, 0);
date.setMonth(11);
check_equals (date.getHours(), 0);

// It's not easy to test the toString() code here cos we cannot find out from
// within AS whether DST is in effect or not.
