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


// Test case for Date ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Date.as";
#include "check.as"

check_equals(typeof(Date), 'function');
check_equals(typeof(Date.prototype), 'object');
check_equals(typeof(Date.prototype.__proto__), 'object');
check_equals(Date.prototype.__proto__, Object.prototype);
#if OUTPUT_VERSION > 5
 check_equals(typeof(Date.__proto__), 'object');
 check_equals(Date.__proto__, Function.prototype);

 check (Date.prototype.hasOwnProperty('getDate'));
 check (Date.prototype.hasOwnProperty('getDay'));
 check (Date.prototype.hasOwnProperty('getFullYear'));
 check (Date.prototype.hasOwnProperty('getHours'));
 check (Date.prototype.hasOwnProperty('getMilliseconds'));
 check (Date.prototype.hasOwnProperty('getMinutes'));
 check (Date.prototype.hasOwnProperty('getMonth'));
 check (Date.prototype.hasOwnProperty('getSeconds'));
 check (Date.prototype.hasOwnProperty('getTime'));
 check (Date.prototype.hasOwnProperty('getTimezoneOffset'));
 check (Date.prototype.hasOwnProperty('getUTCDate'));
 check (Date.prototype.hasOwnProperty('getUTCDay'));
 check (Date.prototype.hasOwnProperty('getUTCFullYear'));
 check (Date.prototype.hasOwnProperty('getUTCHours'));
 check (Date.prototype.hasOwnProperty('getUTCMilliseconds'));
 check (Date.prototype.hasOwnProperty('getUTCMinutes'));
 check (Date.prototype.hasOwnProperty('getUTCMonth'));
 check (Date.prototype.hasOwnProperty('getUTCSeconds'));
 check (Date.prototype.hasOwnProperty('getYear'));
 check (Date.prototype.hasOwnProperty('setDate'));
 check (Date.prototype.hasOwnProperty('setFullYear'));
 check (Date.prototype.hasOwnProperty('setHours'));
 check (Date.prototype.hasOwnProperty('setMilliseconds'));
 check (Date.prototype.hasOwnProperty('setMinutes'));
 check (Date.prototype.hasOwnProperty('setMonth'));
 check (Date.prototype.hasOwnProperty('setSeconds'));
 check (Date.prototype.hasOwnProperty('setTime'));
 check (Date.prototype.hasOwnProperty('setUTCDate'));
 check (Date.prototype.hasOwnProperty('setUTCFullYear'));
 check (Date.prototype.hasOwnProperty('setUTCHours'));
 check (Date.prototype.hasOwnProperty('setUTCMilliseconds'));
 check (Date.prototype.hasOwnProperty('setUTCMinutes'));
 check (Date.prototype.hasOwnProperty('setUTCMonth'));
 check (Date.prototype.hasOwnProperty('setUTCSeconds'));
 check (Date.prototype.hasOwnProperty('setYear'));
 check (Date.prototype.hasOwnProperty('toString'));
 check (!Date.prototype.hasOwnProperty('toLocaleString'));

// UTC is a static method present from v5
check_equals (d.UTC, undefined);
check (Date.UTC);
#else
 check_equals(typeof(Date.__proto__), 'undefined');
#endif

// Static method should be available even if you haven't asked for a Date object.
//
// We have to specify "000.0" bucause ming fails to parse large integer constants,
// returning 2147483647 instead of the correct value.
check_equals (Date.UTC(2000,0,1).valueOf(), 946684800000.0);

// test the Date constructor exists.
// This specific value is used below to check conversion back to year/mon/day etc
var d = new Date(70,1,2,3,4,5,6);
check (d);

// test methods' existence
check (d.getDate);
check (d.getDay);
check (d.getFullYear);
check (d.getHours);
check (d.getMilliseconds);
check (d.getMinutes);
check (d.getMonth);
check (d.getSeconds);
check (d.getTime);
check (d.getTimezoneOffset);
check (d.getUTCDate);
check (d.getUTCDay);
check (d.getUTCFullYear);
check (d.getUTCYear);
check (d.getUTCHours);
check (d.getUTCMilliseconds);
check (d.getUTCMinutes);
check (d.getUTCMonth);
check (d.getUTCSeconds);
check (d.getYear);
check (d.setDate);
check (d.setFullYear);
check (d.setHours);
check (d.setMilliseconds);
check (d.setMinutes);
check (d.setMonth);
check (d.setSeconds);
check (d.setTime);
check (d.setUTCDate);
check (d.setUTCFullYear);
check (d.setUTCHours);
check (d.setUTCMilliseconds);
check (d.setUTCMinutes);
check (d.setUTCMonth);
check (d.setUTCSeconds);
check (d.setYear);
check (d.toString);
check (d.toLocaleString);
// UTC is a static method present from v5
check_equals (d.UTC, undefined);
check (Date.UTC);

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
check_equals (d.getUTCyear, undefined);
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
#else
check (d.getdate);
check (d.getday);
check (d.getfullYear);
check (d.gethours);
check (d.getmilliseconds);
check (d.getminutes);
check (d.getmonth);
check (d.getseconds);
check (d.gettime);
check (d.gettimezoneOffset);
check (d.getUTCdate);
check (d.getUTCday);
check (d.getUTCfullYear);
check (d.getUTCyear);
check (d.getUTChours);
check (d.getUTCmilliseconds);
check (d.getUTCminutes);
check (d.getUTCmonth);
check (d.getUTCseconds);
check (d.getyear);
check (d.setdate);
check (d.setfullYear);
check (d.sethours);
check (d.setmilliseconds);
check (d.setminutes);
check (d.setmonth);
check (d.setseconds);
check (d.settime);
check (d.setUTCdate);
check (d.setUTCfullYear);
check (d.setUTChours);
check (d.setUTCmilliseconds);
check (d.setUTCminutes);
check (d.setUTCmonth);
check (d.setUTCseconds);
check (d.setyear);
check (d.tostring);
check (Date.utc);
#endif // OUTPUT_VERSION > 6

// Some values we will use to test things
    var zero = 0.0;
    var plusinfinity = 1.0/zero;
    var minusinfinity = -1.0/zero;
    var notanumber = zero/zero;

// Check Date constructor in its many forms,
// (also uses valueOf() and toString() methods)

// Constructor with no args sets current localtime
    var d = new Date();
	check (d != undefined);
	// Check it's a valid number after 1 April 2007
	check (d.valueOf() > 1175385600000.0)
	// and before Jan 1 2037 00:00:00
	check (d.valueOf() < 2114380800000.0)

// Constructor with first arg == undefined also sets current localtime
    var d2 = new Date(undefined);
	check (d2 != undefined);
	check (d2.valueOf() >= d.valueOf());
    delete d2;

// One numeric argument sets milliseconds since 1970 UTC
    delete d; var d = new Date(0);
	// Check UTC "get" methods too
	check_equals(typeof(d.valueOf()), 'number');
	check_equals(d.valueOf(), 0);
	check_equals(typeof(d.getTime()), 'number'); 
	check_equals(d.getTime(), 0);
	check_equals(d.getUTCFullYear(), 1970);
	check_equals(d.getUTCMonth(), 0);
	check_equals(d.getUTCDate(), 1);
	check_equals(d.getUTCDay(), 4);	// It was a Thursday
	check_equals(d.getUTCHours(), 0);
	check_equals(d.getUTCMinutes(), 0);
	check_equals(d.getUTCSeconds(), 0);
	check_equals(d.getUTCMilliseconds(), 0);

    // These all vary according to timezone... 0 milliseconds since
    // midnight 1 Jan 1970 was 1969 in anything west of Greenwich.
    // It *should* fail there (and does in the pp).
	//check_equals(d.getFullYear(), 1970);
	//check_equals(d.getMonth(), 0);
	//check_equals(d.getDate(), 1);
	//check_equals(d.getDay(), 4);	// It was a Thursday
	//check_equals(d.getHours(), 1);
	//check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 0);
	
    /// No difference:
    check_equals (Date.toLocaleString(), Date.toString());

// Check other convertible types
// Booleans convert to 0 and 1
    var foo = true; delete d; var d = new Date(foo);
	check_equals(d.valueOf(), 1);
    foo = false; delete d; var d = new Date(foo);
	check_equals(d.valueOf(), 0);
// Numeric strings
    foo = "12345"; delete d; var d = new Date(foo);
	check_equals(d.valueOf(), 12345.0);
    foo = "12345.0"; delete d; var d = new Date(foo);
	check_equals(d.valueOf(), 12345.0);
    foo = "12345.5"; delete d; var d = new Date(foo);
	check_equals(d.valueOf(), 12345.5);	// Sets fractions of msec ok?
    foo = "-12345"; delete d; var d = new Date(foo);
	check_equals(d.valueOf(), -12345.0);
// Bad numeric values
	// NAN
    delete d; var d = new Date(notanumber);
	check_equals(d.valueOf().toString(), "NaN");
	check_equals(d.toString(), "Invalid Date");
	// Infinity
    delete d; var d = new Date(plusinfinity);
	check_equals(d.valueOf().toString(), "Infinity");
	check_equals(d.toString(), "Invalid Date");
	// -Infinity
    delete d; var d = new Date(minusinfinity);
	check_equals(d.valueOf().toString(), "-Infinity");
	check_equals(d.toString(), "Invalid Date");
// Bogus values: non-numeric strings
	foo = "bones"; delete d; var d = new Date(foo);
	check_equals(d.valueOf().toString(), "NaN");
	foo = "1234X"; delete d; var d = new Date(foo);
	check_equals(d.valueOf().toString(), "NaN");

// Constructor with two numeric args means year and month in localtime.
// Now we check the localtime decoding methods too.
// Negative year means <1900; 0-99 means 1900-1999; 100- means 100-)
// Month is 0-11. month>11 increments year; month<0 decrements year.
    delete d; var d = new Date(70,0);	// 1 Jan 1970 00:00:00 localtime
	check_equals(d.getYear(), 70);
	check_equals(d.getFullYear(), 1970);
	check_equals(d.getMonth(), 0);
	check_equals(d.getDate(), 1);
	check_equals(d.getDay(), 4);	// It was a Thursday
	check_equals(d.getHours(), 0);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 0);
// Check four-figure version - should be the same.
    var d2 = new Date(1970,0); check_equals(d.valueOf(), d2.valueOf());
// Check four-figure version and non-zero month
    delete d; var d = new Date(2000,3);	// 1 April 2000 00:00:00 localtime
	check_equals(d.getYear(), 100);
	check_equals(d.getFullYear(), 2000);
	check_equals(d.getMonth(), 3);
	check_equals(d.getDate(), 1);
	check_equals(d.getDay(), 6);	// It was a Saturday
	check_equals(d.getHours(), 0);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 0);
// Check month overflow/underflow
    delete d; var d = new Date(2000,12);
	check_equals(d.getFullYear(), 2001);
	check_equals(d.getMonth(), 0);
    delete d; var d = new Date(2000,-18);
	check_equals(d.getFullYear(), 1998);
	check_equals(d.getMonth(), 6);
// Bad numeric value handling: year is an invalid number with >1 arg
// The commercial player for these first three cases gives
// -6.77681005679712e+19  Tue Jan -719527 00:00:00 GMT+0000
// but that doesn't seem worth emulating...
note("The pp is known to fail the next three tests");
    delete d; var d = new Date(notanumber,0);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(plusinfinity,0);
	check_equals(d.valueOf().toString(), "Infinity");
    delete d; var d = new Date(minusinfinity,0);
	check_equals(d.valueOf().toString(), "-Infinity");
// Bad numeric value handling: month is an invalid number
    delete d; var d = new Date(0,notanumber);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(0,plusinfinity);
	xcheck_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(0,minusinfinity);
	xcheck_equals(d.valueOf().toString(), "NaN");

// Constructor with three numeric args means year month day-of-month
    delete d; var d = new Date(2000,0,1); // 1 Jan 2000 00:00:00 localtime
	check_equals(d.getFullYear(), 2000);
	check_equals(d.getMonth(), 0);
	check_equals(d.getDate(), 1);
// Check day-of-month overflow/underflow
    delete d; var d = new Date(2000,0,32); // 32 Jan -> 1 Feb
	check_equals(d.getFullYear(), 2000);
	check_equals(d.getMonth(), 1);
	check_equals(d.getDate(), 1);
    delete d; var d = new Date(2000,1,0); // 0 Feb -> 31 Jan
	check_equals(d.getFullYear(), 2000);
	check_equals(d.getMonth(), 0);
	check_equals(d.getDate(), 31);
    delete d; var d = new Date(2000,0,-6); // -6 Jan 2000 -> 25 Dec 1999
	check_equals(d.getFullYear(), 1999);
	check_equals(d.getMonth(), 11);
	check_equals(d.getDate(), 25);
// Bad numeric value handling when day-of-month is an invalid number
// A bad month always returns NaN but a bad d-o-m returns the infinities.
    delete d; var d = new Date(2000,0,notanumber);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(2000,0,plusinfinity);
	check_equals(d.valueOf().toString(), "Infinity");
    delete d; var d = new Date(2000,0,minusinfinity);
	check_equals(d.valueOf().toString(), "-Infinity");
    // Check bad string value
    foo = "bones"; delete d; var d = new Date(2000,0,foo);
	check_equals(d.valueOf().toString(), "NaN");

// Constructor with four numeric args means year month day-of-month hour
    delete d; var d = new Date(2000,0,1,12);
	check_equals(d.getHours(), 12);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 0);
    // Check that fractional parts of hours are ignored
    delete d; var d = new Date(2000,0,1,12.5);
	check_equals(d.getHours(), 12);
	check_equals(d.getMinutes(), 0);
    // Check hours overflow/underflow
    delete d; var d = new Date(2000,0,1,25);
	check_equals(d.getDate(), 2);
	check_equals(d.getHours(), 1);
    // Bad hours, like bad d-o-m, return infinites.
    delete d; var d = new Date(2000,0,1,notanumber);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(2000,0,1,plusinfinity);
	check_equals(d.valueOf().toString(), "Infinity");
    delete d; var d = new Date(2000,0,1,minusinfinity);
	check_equals(d.valueOf().toString(), "-Infinity");
    // Check bad string value
    foo = "bones"; delete d; var d = new Date(2000,0,1,foo);
	check_equals(d.valueOf().toString(), "NaN");

// Constructor with five numeric args means year month day-of-month hour min
    delete d; var d = new Date(2000,0,1,12,30);
	check_equals(d.getHours(), 12);
	check_equals(d.getMinutes(), 30);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 0);
    // Check minute overflow/underflow
    delete d; var d = new Date(2000,0,1,12,70);
	check_equals(d.getHours(), 13);
	check_equals(d.getMinutes(), 10);
	check_equals(d.getSeconds(), 0);
    delete d; var d = new Date(2000,0,1,12,-120);
	check_equals(d.getHours(), 10);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
    // Infinite minutes return infinites.
    delete d; var d = new Date(2000,0,1,0,notanumber);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(2000,0,1,0,plusinfinity);
	check_equals(d.valueOf().toString(), "Infinity");
    delete d; var d = new Date(2000,0,1,0,minusinfinity);
	check_equals(d.valueOf().toString(), "-Infinity");
    // Check bad string value
    foo = "bones"; delete d; var d = new Date(2000,0,1,0,foo);
	check_equals(d.valueOf().toString(), "NaN");

// Constructor with six numeric args means year month d-of-m hour min sec
// Check UTC seconds here too since it should be the same.
    delete d; var d = new Date(2000,0,1,0,0,45);
	check_equals(d.getHours(), 0);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 45);
	check_equals(d.getUTCSeconds(), 45);
	check_equals(d.getMilliseconds(), 0);
    // Check second overflow/underflow
    delete d; var d = new Date(2000,0,1,12,0,70);
	check_equals(d.getHours(), 12);
	check_equals(d.getMinutes(), 1);
	check_equals(d.getSeconds(), 10);
    delete d; var d = new Date(2000,0,1,12,0,-120);
	check_equals(d.getHours(), 11);
	check_equals(d.getMinutes(), 58);
	check_equals(d.getSeconds(), 0);
    // Infinite seconds return infinites.
    delete d; var d = new Date(2000,0,1,0,0,notanumber);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(2000,0,1,0,0,plusinfinity);
	check_equals(d.valueOf().toString(), "Infinity");
    delete d; var d = new Date(2000,0,1,0,0,minusinfinity);
	check_equals(d.valueOf().toString(), "-Infinity");
    // Check bad string value
    foo = "bones"; delete d; var d = new Date(2000,0,1,0,0,foo);
	check_equals(d.valueOf().toString(), "NaN");

// Constructor with seven numeric args means year month dom hour min sec msec
// Check UTC milliseconds here too since it should be the same.
    delete d; var d = new Date(2000,0,1,0,0,0,500);
	check_equals(d.getHours(), 0);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 500);
	check_equals(d.getUTCMilliseconds(), 500);
    // Fractions of milliseconds are ignored here
    delete d; var d = new Date(2000,0,1,0,0,0,500.5);
	check_equals(d.getMilliseconds(), 500.0);
    // Check millisecond overflow/underflow
    delete d; var d = new Date(2000,0,1,12,0,0,1000);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 1);
	check_equals(d.getMilliseconds(), 0);
    delete d; var d = new Date(2000,0,1,12,0,0,-120000);
	check_equals(d.getHours(), 11);
	check_equals(d.getMinutes(), 58);
	check_equals(d.getSeconds(), 0);
    // Infinite milliseconds return infinites.
    delete d; var d = new Date(2000,0,1,0,0,0,notanumber);
	check_equals(d.valueOf().toString(), "NaN");
    delete d; var d = new Date(2000,0,1,0,0,0,plusinfinity);
	check_equals(d.valueOf().toString(), "Infinity");
    delete d; var d = new Date(2000,0,1,0,0,0,minusinfinity);
	check_equals(d.valueOf().toString(), "-Infinity");
    // Check bad string value
    foo = "bones"; delete d; var d = new Date(2000,0,1,0,0,0,foo);
	check_equals(d.valueOf().toString(), "NaN");
    // Finally, check that a millisecond is enough to overflow/underflow a year
    delete d; var d = new Date(1999,11,31,23,59,59,1001);
	check_equals(d.getFullYear(), 2000);
	check_equals(d.getMonth(), 0);
	check_equals(d.getDate(), 1);
	check_equals(d.getMinutes(), 0);
	check_equals(d.getSeconds(), 0);
	check_equals(d.getMilliseconds(), 1);
    delete d; var d = new Date(2000,0,1,0,0,0,-1);
    //note (d.valueOf());
	check_equals(d.getFullYear(), 1999);
	check_equals(d.getMonth(), 11);
	check_equals(d.getDate(), 31);
	check_equals(d.getMinutes(), 59);
	check_equals(d.getSeconds(), 59);
	check_equals(d.getMilliseconds(), 999);
// If a mixture of infinities and/or NaNs are present the result is NaN.
    delete d; var d = new Date(2000,0,1,plusinfinity,minusinfinity,0,0);
	check_equals(d.valueOf().toString(), "NaN");

    wierddate = new Date (-2000, 5, 6, 12, 30, 12, 7);
	check_equals(wierddate.getFullYear(), -100);
	check_equals(wierddate.getMonth(), 5);
	check_equals(wierddate.getDate(), 6);
	check_equals(wierddate.getHours(), 12);
	check_equals(wierddate.getMinutes(), 30);
	check_equals(wierddate.getSeconds(), 12);
	check_equals(wierddate.getMilliseconds(), 7);

    // Bogus tests: depend on the timezone.
	//xcheck_equals(wierddate.valueOf().toString(), "-65309372987993");

    wierddate.setMilliseconds(300);
	check_equals(wierddate.getMilliseconds(), 300);
	check_equals(wierddate.getSeconds(), 12);

	//xcheck_equals(wierddate.valueOf().toString(), "-65309372987700");
	
    wierddate.setMilliseconds(-300);
	check_equals(wierddate.getMilliseconds(), 700);
	check_equals(wierddate.getSeconds(), 11);

	//xcheck_equals(wierddate.valueOf().toString(), "-65309372988300");
	
    wierddate.setYear(100000);
	check_equals(wierddate.getMilliseconds(), 700);
	check_equals(wierddate.getFullYear(), 100000);	   


    wierddate.setYear(-100000);
	check_equals(wierddate.getMilliseconds(), 700);
	check_equals(wierddate.getFullYear(), -100000);	   

    // The accessor functions for such a large number give different
    // results according to the platform and compiler options, so
    // we won't test them.
    h = new Date(3.0935415006117e+23);
    check_equals(h.valueOf().toString(), "3.0935415006117e+23");

    h = new Date(10000001);
    check_equals(h.getMilliseconds(), 1);
    check_equals(h.getSeconds(), 40);
	check_equals(h.getUTCMilliseconds(), 1);
	check_equals(h.getUTCSeconds(), 40);
	check_equals(h.getUTCMinutes(), 46);
	check_equals(h.getUTCDay(), 4);

// It's hard to test TimezoneOffset because the values will be different
// depending upon where geographically you run the tests.
// We do what we can without knowing where we are!

// Set midnight local time, adjust for tzoffset
// and this should give us midnight UTC.
//
// If we are in GMT+1 then TimezoneOffset is -60.
// If we set midnight localtime in the GMT+1 zone,
// that is 23:00 the day before in UTC (because in GMT+1 clock times happen
// an hour earlier than they do in "real" time).
// Thus to set UTC to midnight we need to subtract the TimezoneOffset.
delete d;
var d = new Date(2000, 0, 1, 0, 0, 0, 0);
d.setTime(d.getTime() - (60000 * d.getTimezoneOffset()));
check_equals (d.getUTCHours(), 0);

// Try the same thing in July to get one with DST and one without
d = new Date(2000, 6, 1, 0, 0, 0, 0);
d.setTime(d.getTime() - (60000 * d.getTimezoneOffset()));
check_equals (d.getUTCHours(), 0);


// Test behaviour when you set the time during DST then change
// to a non-DST date.
// setUTCHours should preserve the time of day in UTC;
// setHours should preserve the time of day in localtime.
//
// We assume that January/December and June/July will have different DST values

trace ("Testing hour when setting date into/out of DST");
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

// It's not easy to test the toString() code here because we cannot find out from
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

d1 = Date(123);
d2 = new Date;
check_equals(d1.toString(), d2.toString());

d1 = Date(2008, 10, 10, 10, 10, 10, 10);
d2 = new Date;
check_equals(d1.toString(), d2.toString());

check_equals (Date.UTC(-1000, 20).valueOf(), -33713366400000);
check_equals (Date.UTC(-70, 0).toString(), "-4417977600000");
check_equals (Date.UTC(-70, 0).valueOf(), -4417977600000);
check_equals (Date.UTC(1969, 11).toString(), "-2678400000");
check_equals (Date.UTC(1969, 11).valueOf(), -2678400000);
check_equals (Date.UTC(1969, 12).toString(), "0");
check_equals (Date.UTC(1969, 12).valueOf(), 0);

check_equals (Date.UTC(0, 0, 0, 0, 0, 0, 0).toString(), "-2209075200000");
check_equals (Date.UTC(1969, 12, 31).toString(), "2592000000");
check_equals (Date.UTC(1969, 12, 31).toString(), "2592000000");

check_equals (Date.UTC(1970, 1).toString(), "2678400000");
check_equals (Date.UTC(1970, 1).valueOf(), 2678400000);

check_equals (Date.UTC(-1, -12).toString(), "-2272060800000");
check_equals ((Date.UTC(-1, 12).valueOf() < -2208988799999.5 &&
               Date.UTC(-1, 12).valueOf() > -2208988800000.5), true);

pd = new Date();
ret = (pd < 67);
check_equals(typeof(ret), "boolean");

ret = (pd > 67);
check_equals(typeof(ret), "boolean");
check_equals(ret, true);

ret = (pd < "a string");
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);

ret = (pd > "a string");
check_equals(typeof(ret), "undefined");
check_equals(ret, undefined);


// Check if Date, concatenated to a string, is in human readable form
d = new Date(2000, 1, 15, 0, 0, 0); 
var foo = "foo "+d;   
var bar = 0+d;   
check_equals(typeof(foo), 'string');
#if OUTPUT_VERSION > 5
 // correct: "0Tue Feb 15 00:00:00 GMT+0100 2000"
 // but this probably depends on time zone, so just check for some fixed part..
 check_equals(typeof(bar), 'string');
 check_equals(bar.substring(0, 1), '0');
 check_equals(bar.indexOf("Feb"), 5);
 // correct: "foo Tue Feb 15 00:00:00 GMT+0100 2000"
 // but this probably depends on time zone, so just check for some fixed part..
 check_equals(foo.indexOf("Feb"), 8);
#else
 check_equals(typeof(bar), 'number');
 // correct: "foo 950569200000", but only for the timezone
 // of the original author
 check_equals(foo.substring(0, 7), 'foo 950');
#endif

{
    var o = new Date();
    o.setUTCMilliseconds(); // sets o's date to NaN

    ASSetPropFlags(o.__proto__, null, 0, 1);
    for (prop in o) {
        if (prop.substr(0, 3) == "set" && prop != "setTime") {
            o[prop](1,2,3,4);
            check_equals(o.valueOf().toString(), "NaN");
        }
    }
}

#if OUTPUT_VERSION == 5
totals(307);
#else
totals (349);
#endif
