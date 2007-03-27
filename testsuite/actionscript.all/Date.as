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

rcsid="$Id: Date.as,v 1.20 2007/03/27 09:05:23 strk Exp $";

#include "check.as"

check (Date);

// Static method should be available even if you haven't asked for a Date object.
//
// We have to specify "000.0" bucause ming fails to parse large integer constants,
// returning 2147483647 instead of the correct value.
check_equals (Date.UTC(2000,0,1).valueOf(), 946684800000.0);

// test the Date constructor.
// This specific value is used below to check conversion back to year/mon/day etc
var d = new Date(70,1,2,3,4,5,6);
check (d);

// test methods existance
check (d.getDate != undefined);
check (d.getDay != undefined);
check (d.getFullYear != undefined);
check (d.getHours != undefined);
check (d.getMilliseconds != undefined);
check (d.getMinutes != undefined);
check (d.getMonth != undefined);
check (d.getSeconds != undefined);
check (d.getTime != undefined);
check (d.getTimezoneOffset != undefined);
check (d.getUTCDate != undefined);
check (d.getUTCDay != undefined);
check (d.getUTCFullYear != undefined);
check (d.getUTCHours != undefined);
check (d.getUTCMilliseconds != undefined);
check (d.getUTCMinutes != undefined);
check (d.getUTCMonth != undefined);
check (d.getUTCSeconds != undefined);
check (d.getYear != undefined);
check (d.setDate != undefined);
check (d.setFullYear != undefined);
check (d.setHours != undefined);
check (d.setMilliseconds != undefined);
check (d.setMinutes != undefined);
check (d.setMonth != undefined);
check (d.setSeconds != undefined);
check (d.setTime != undefined);
check (d.setUTCDate != undefined);
check (d.setUTCFullYear != undefined);
check (d.setUTCHours != undefined);
check (d.setUTCMilliseconds != undefined);
check (d.setUTCMinutes != undefined);
check (d.setUTCMonth != undefined);
check (d.setUTCSeconds != undefined);
check (d.setYear != undefined);
check (d.toString != undefined);
// UTC is a static method present from v5
check_equals (d.UTC, undefined);
check (Date.UTC != undefined);

#if OUTPUT_VERSION > 6

// From SWF 7 up methods are case-sensitive !
check_equals (d.getdate, undefined);
check_equals (d.getday, undefined);
check_equals (d.getfullYear, undefined);
check_equals (d.gethours, undefined);
check_equals (d.getmilliseconds, undefined);
check_equals (d.getminutes, undefined);
check_equals (d.getmonth, undefined);
check_equals (d.getseconds, undefined);
check_equals (d.gettime, undefined);
check_equals (d.gettimezoneOffset, undefined);
check_equals (d.getUTCdate, undefined);
check_equals (d.getUTCday, undefined);
check_equals (d.getUTCfullYear, undefined);
check_equals (d.getUTChours, undefined);
check_equals (d.getUTCmilliseconds, undefined);
check_equals (d.getUTCminutes, undefined);
check_equals (d.getUTCmonth, undefined);
check_equals (d.getUTCseconds, undefined);
check_equals (d.getyear, undefined);
check_equals (d.setdate, undefined);
check_equals (d.setfullYear, undefined);
check_equals (d.sethours, undefined);
check_equals (d.setmilliseconds, undefined);
check_equals (d.setminutes, undefined);
check_equals (d.setmonth, undefined);
check_equals (d.setseconds, undefined);
check_equals (d.settime, undefined);
check_equals (d.setUTCdate, undefined);
check_equals (d.setUTCfullYear, undefined);
check_equals (d.setUTChours, undefined);
check_equals (d.setUTCmilliseconds, undefined);
check_equals (d.setUTCminutes, undefined);
check_equals (d.setUTCmonth, undefined);
check_equals (d.setUTCseconds, undefined);
check_equals (d.setyear, undefined);
check_equals (d.tostring, undefined);
check_equals (Date.utc, undefined);

#endif

// var d = new Date(70,1,2,3,4,5,6);	// See above
trace ("Testing random d");
check_equals (d.getFullYear(), 1970);
check_equals (d.getYear(), 70);
check_equals (d.getMonth(), 1);
check_equals (d.getDate(), 2);
check_equals (d.getHours(), 3);
check_equals (d.getMinutes(), 4);
check_equals (d.getSeconds(), 5);
check_equals (d.getMilliseconds(), 6);

// Test decoding methods
// Check the epoch, 1 Jan 1970
trace ("Testing 1 Jan 1970 UTC");
check_equals (d.setTime(0), 0);
check_equals (d.getTime(), 0);
check_equals (d.getUTCFullYear(), 1970);
check_equals (d.getUTCMonth(), 0);
check_equals (d.getUTCDate(), 1);
check_equals (d.getUTCDay(), 4);	// It was a Thursday
check_equals (d.getUTCHours(), 0);
check_equals (d.getUTCMinutes(), 0);
check_equals (d.getUTCSeconds(), 0);
check_equals (d.getUTCMilliseconds(), 0);
check_equals (d.valueOf(), 0);

trace ("Testing 1 Jan 2000 UTC");
d.setUTCFullYear(2000, 0, 1);
d.setUTCHours(0, 0, 0);
check_equals (d.getUTCFullYear(), 2000);
check_equals (d.getUTCMonth(), 0);
check_equals (d.getUTCDate(), 1);
check_equals (d.getUTCDay(), 6);	// It was a Saturday
check_equals (d.getUTCHours(), 0);
check_equals (d.getUTCMinutes(), 0);
check_equals (d.getUTCSeconds(), 0);
check_equals (d.getUTCMilliseconds(), 0);
check_equals (d.valueOf(), 946684800000.0);	// Same as flashplayer gives

trace ("Testing 1 Jul 2000 UTC");
d.setUTCFullYear(2000, 6, 1);
d.setUTCHours(0, 0, 0);
check_equals (d.getUTCFullYear(), 2000);
check_equals (d.getUTCMonth(), 6);
check_equals (d.getUTCDate(), 1);
check_equals (d.getUTCDay(), 6);	// It was a Saturday
check_equals (d.getUTCHours(), 0);
check_equals (d.getUTCMinutes(), 0);
check_equals (d.getUTCSeconds(), 0);
check_equals (d.getUTCMilliseconds(), 0);
check_equals (d.valueOf(), 962409600000.0);	// Same as flashplayer gives

trace ("Testing 1 Jan 2000 localtime");
// The many-argument version of the Date constructor sets the d in localtime
delete d;
var d = new Date(2000, 0, 1, 0, 0, 0, 0);
check_equals (d.getFullYear(), 2000);
check_equals (d.getYear(), 100);
check_equals (d.getMonth(), 0);
check_equals (d.getDate(), 1);
check_equals (d.getDay(), 6);	// It was a Saturday
check_equals (d.getHours(), 0);
check_equals (d.getMinutes(), 0);
check_equals (d.getSeconds(), 0);
check_equals (d.getMilliseconds(), 0);

trace ("Testing 1 Jul 2000 localtime");
// The many-argument version of the Date constructor sets the d in localtime
delete d;
var d = new Date(2000, 6, 1, 0, 0, 0, 0);
check_equals (d.getFullYear(), 2000);
check_equals (d.getYear(), 100);
check_equals (d.getMonth(), 6);
check_equals (d.getDate(), 1);
check_equals (d.getDay(), 6);	// It was a Saturday
check_equals (d.getHours(), 0);
check_equals (d.getMinutes(), 0);
check_equals (d.getSeconds(), 0);
check_equals (d.getMilliseconds(), 0);

// Test TimezoneOffset and local hours by setting a d to the 1 Jan 2000 UTC
// offset by the tzoffset so that localtime should be 00:00 or 01:00
// (according to whether DST is active or not)

trace ("Testing timezone offset");
var tzoffset = new Number(d.getTimezoneOffset());	// in mins east of GMT
trace("timezone offset = " + tzoffset.toString());
d.setUTCFullYear(2000, 0, 1);
d.setUTCHours(0, 0, 0, 0);
d.setTime(d.getTime() - (60000*tzoffset));
note("d.getHours(): "+d.getHours());
check (d.getHours() >= 0);
//check (d.getHours() <= 1);

// Test behaviour when you set the time during DST then change the d to
// a non-DST d.
// setUTCHours should preserve the time of day in UTC;
// setHours should preserve the time of day in localtime.
trace ("Testing hour when setting d into/out of DST");
d.setUTCFullYear(2000, 0, 1);
d.setUTCHours(0, 0, 0, 0);
d.setUTCMonth(6);
check_equals (d.getUTCHours(), 0);

d.setUTCFullYear(2000, 6, 1);
d.setUTCHours(0, 0, 0, 0);
d.setUTCMonth(11);
check_equals (d.getUTCHours(), 0);

d.setFullYear(2000, 0, 1);
d.setHours(0, 0, 0, 0);
d.setMonth(6);
check_equals (d.getHours(), 0);

d.setFullYear(2000, 6, 1);
d.setHours(0, 0, 0, 0);
d.setMonth(11);
check_equals (d.getHours(), 0);

// It's not easy to test the toString() code here cos we cannot find out from
// within AS whether DST is in effect or not.

check_equals (Date.UTC(1970,0), 0);
check_equals (Date.UTC(70,0), 0);

// Check that Date.UTC gives the same as setUTC*, which we tested above.
// Test two dates: one in DST and one not.
d.setUTCFullYear(2000, 0, 1);
d.setUTCHours(0, 0, 0, 0);
check (Date.UTC(2000,0,1,0,0,0,0) == d.valueOf());
d.setUTCFullYear(2000, 6, 1);
d.setUTCHours(0, 0, 0, 0);
check (Date.UTC(2000,6,1,0,0,0,0) == d.valueOf());
