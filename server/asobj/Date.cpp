// 
//	Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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


// Date.cpp
//
// Implements methods of the ActionScript "Date" class for Gnash
//
// TODO:
//	implement static method Date.UTC
//
// To probe FlashPlayer functionality put something like:
// class Test {
//        }
//
//        static function main(mc) {
//		var now = new Date();
//               _root.createTextField("tf",0,0,0,320,200);
//              _root.tf.text = now.toString();
//        }
// }
// in test.as, compile with
//	mtasc -swf test.swf -main -header 320:200:10 test.as
// and open test.swf in the commercial Flash Player.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Date.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"

#include <ctime>
#include <cmath>

#if defined(_WIN32) || defined(WIN32)
# define snprintf _snprintf
#else
# include <sys/time.h>
#endif

// Declaration for replacement timezone functions
// In the absence of gettimeofday() we use ftime() to get milliseconds,
// but not for timezone offset bcos ftime's TZ stuff is unreliable.
// For that we use tzset()/timezone if it is available

#ifndef HAVE_GETTIMEOFDAY
# include <sys/timeb.h>		// for ftime()
# ifdef HAVE_TZSET
	extern long timezone;	// for tzset()/timezone
# endif
#endif

namespace gnash {

#ifdef HAVE_LOCALTIME_R
	// Use the library functions
#	define _gmtime_r gmtime_r
#	define _localtime_r localtime_r
#else
// Roll our own compatible versions rather checking the ifdef everywhere
static struct tm *
_localtime_r(time_t *t, struct tm *tm)
{
	struct tm *tmp;
	tmp = localtime(t);
	memcpy(tm, tmp, sizeof(struct tm));
	return(tm);
}
static struct tm *
_gmtime_r(time_t *t, struct tm *tm)
{
	struct tm *tmp;
	tmp = gmtime(t);
	memcpy(tm, tmp, sizeof(struct tm));
	return(tm);
}
#endif

// forwrd declarations
static void date_new(const fn_call& fn);
static void date_getdate(const fn_call& fn);
static void date_getday(const fn_call& fn);
static void date_getfullyear(const fn_call& fn);
static void date_gethours(const fn_call& fn);
static void date_getmilliseconds(const fn_call& fn);
static void date_getminutes(const fn_call& fn);
static void date_getmonth(const fn_call& fn);
static void date_getseconds(const fn_call& fn);
// static void date_gettime(const fn_call& fn); == date_valueof()
static void date_gettimezoneoffset(const fn_call& fn);
static void date_getutcdate(const fn_call& fn);
static void date_getutcday(const fn_call& fn);
static void date_getutcfullyear(const fn_call& fn);
static void date_getutchours(const fn_call& fn);
static void date_getutcminutes(const fn_call& fn);
static void date_getutcmonth(const fn_call& fn);
static void date_getutcseconds(const fn_call& fn);
//static void date_getutcmilliseconds(const fn_call& fn); // == getmilliseconds
static void date_getyear(const fn_call& fn);
static void date_setdate(const fn_call& fn);
static void date_setfullyear(const fn_call& fn);
static void date_sethours(const fn_call& fn);
static void date_setmilliseconds(const fn_call& fn);
static void date_setminutes(const fn_call& fn);
static void date_setmonth(const fn_call& fn);
static void date_setseconds(const fn_call& fn);
static void date_settime(const fn_call& fn);
static void date_setutcdate(const fn_call& fn);
static void date_setutcfullyear(const fn_call& fn);
static void date_setutchours(const fn_call& fn);
//static void date_setutcmilliseconds(const fn_call& fn); // == setmilliseconds
static void date_setutcminutes(const fn_call& fn);
static void date_setutcmonth(const fn_call& fn);
static void date_setutcseconds(const fn_call& fn);
static void date_setyear(const fn_call& fn);
static void date_tostring(const fn_call& fn);
static void date_valueof(const fn_call& fn);

static void date_utc(const fn_call& fn);
static as_object* getDateInterface();
static void attachDateInterface(as_object& o);
static void attachDateStaticInterface(as_object& o);

static void
attachDateInterface(as_object& o)
{
	o.init_member("getDate", &date_getdate);
	o.init_member("getDay", &date_getday);
	o.init_member("getFullYear", &date_getfullyear);
	o.init_member("getHours", &date_gethours);
	o.init_member("getMilliseconds", &date_getmilliseconds);
	o.init_member("getMinutes", &date_getminutes);
	o.init_member("getMonth", &date_getmonth);
	o.init_member("getSeconds", &date_getseconds);
	o.init_member("getTime", &date_valueof);
	o.init_member("getTimezoneOffset", &date_gettimezoneoffset);
	o.init_member("getUTCDate", &date_getutcdate);
	o.init_member("getUTCDay", &date_getutcday);
	o.init_member("getUTCFullYear", &date_getutcfullyear);
	o.init_member("getUTCHours", &date_getutchours);
	o.init_member("getUTCMilliseconds", &date_getmilliseconds); // same
	o.init_member("getUTCMinutes", &date_getutcminutes);
	o.init_member("getUTCMonth", &date_getutcmonth);
	o.init_member("getUTCSeconds", &date_getutcseconds);
	o.init_member("getYear", &date_getyear);
	o.init_member("setDate", &date_setdate);
	o.init_member("setFullYear", &date_setfullyear);
	o.init_member("setHours", &date_sethours);
	o.init_member("setMilliseconds", &date_setmilliseconds);
	o.init_member("setMinutes", &date_setminutes);
	o.init_member("setMonth", &date_setmonth);
	o.init_member("setSeconds", &date_setseconds);
	o.init_member("setTime", &date_settime);
	o.init_member("setUTCDate", &date_setutcdate);
	o.init_member("setUTCFullYear", &date_setutcfullyear);
	o.init_member("setUTCHours", &date_setutchours);
	o.init_member("setUTCMilliseconds", &date_setmilliseconds); // same
	o.init_member("setUTCMinutes", &date_setutcminutes);
	o.init_member("setUTCMonth", &date_setutcmonth);
	o.init_member("setUTCSeconds", &date_setutcseconds);
	o.init_member("setYear", &date_setyear);
	o.init_member("toString", &date_tostring);
	o.init_member("valueOf", &date_valueof);
}

static void
attachDateStaticInterface(as_object& o)
{
	// TODO: This should *only* be available when SWF version is > 6
	// Are you sure? the references say it's in from v5 -martin
	o.init_member("UTC", &date_utc);
}

static as_object*
getDateInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		attachDateInterface(*o);
	}
	return o.get();
}

class date_as_object : public as_object
{
public:
	// value is the master field and the Date's value, and holds the
	// date as the number of milliseconds since midnight 1 Jan 1970.
	// All other "fields" are calculated from this.
	double value;		// milliseconds UTC since the epoch

	date_as_object()
		:
		as_object(getDateInterface())
	{
	}

private:
};

/// \brief Date constructor
//
/// The constructor has three forms: 0 args, 1 arg and 2-7 args.
/// new Date() sets the Date to the current time of day
/// new Date(timeValue:Number) sets the date to a number of milliseconds since
///	1 Jan 1970 UTC
/// new Date(year:Number, month:Number [, date:Number [, hour:Number
///	     [, minute:Number [, second:Number [, millisecond:Number ]]]]])
///	sets the date object to a specified year/month etc in local time
///	Defaults are 0 except for date (day of month) whose default it 1.

void
date_new(const fn_call& fn)
{
	// TODO: just make date_as_object constructor
	//       register the exported interface, don't
	//       replicate all functions !!

	date_as_object *date = new date_as_object;

	// TODO: move this to date_as_object constructor
	if (fn.nargs < 1) {
		// Set from system clock
#ifdef HAVE_GETTIMEOFDAY
		struct timeval tv;
		struct timezone tz;

		gettimeofday(&tv,&tz);
		date->value = (double)tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
#else
		struct timeb tb;
		
		ftime (&tb);
		date->value = (double)tb.time * 1000.0 + tb.millitm;
#endif
	} else if (fn.nargs == 1) {
		// Set the value in milliseconds since 1970 UTC
		date->value = fn.arg(0).to_number();
	} else {
		struct tm tm;	// time constructed from localtime components
		time_t utcsecs;	// Seconds since 1970 as returned by mktime()
		double millisecs = 0; // milliseconds if specified by caller

		tm.tm_sec = 0;
                tm.tm_min = 0;
                tm.tm_hour = 0;
                tm.tm_mday = 1;
                tm.tm_mon = (int) fn.arg(1).to_number();
                tm.tm_year = (int) fn.arg(0).to_number();
		// We need to know whether DST was in effect at the date they
		// mention to know how to interpret the hour they specify.
		// In fact this is impossible bcos on the day that the
		// clocks go back there are two hours of time with the same
		// local year-month-day-hour-minute values.
		// Example: 1995 Apr 2 01:00-59 and 02:00-59 give the same UTC,
		// both in GMT (which is wrong: BST started 26 March!)
		// We'll do our best...

		switch (fn.nargs) {
		default:
		    IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("Date constructor called with more than 7 arguments");
		    )
		case 7:
			millisecs = fn.arg(6).to_number();
		case 6:
			tm.tm_sec = (int)fn.arg(5).to_number();
		case 5:
			tm.tm_min = (int)fn.arg(4).to_number();
		case 4:
			tm.tm_hour = (int)fn.arg(3).to_number();
		case 3:
			tm.tm_mday = (int)fn.arg(2).to_number();
		case 2:
			tm.tm_mon = (int)fn.arg(1).to_number();
			tm.tm_year = (int)fn.arg(0).to_number();
			// 0-99 means years since 1900
			// 100- means full year
			// negative values are so much before 1900
			// (so 55AC is only expressible as -1845)
			// Fractional part of a year is ignored.
			if (tm.tm_year >= 100) tm.tm_year -= 1900;
		}
		// Experimentally, glibc mktime ignores and tm_isdst and sets
		// it from the date, applying it at midnight (not at 2am)
		utcsecs = mktime(&tm);	// mktime converts from local time
		if (utcsecs == -1) {
			// mktime could not represent the time
			log_error("Date() failed to initialise from arguments");
			date->value = 0;
		} else {
			date->value = (double)utcsecs * 1000.0 + millisecs;
		}
	}
		
	fn.result->set_as_object(date);
}

// Wrapper around dynamic_cast to implement user warning.
// To be used by builtin properties and methods.
static date_as_object*
ensure_date_object(as_object* obj)
{
	date_as_object* ret = dynamic_cast<date_as_object*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for date objects called against non-date instance");
	}
	return ret;
}

//
//    =========    Functions to get dates in various ways    ========
//

// Date.getTime() is implemented by Date.valueOf()

// Functions to return broken-out elements of the date and time.

// We use a prototype macro to generate the function bodies because the many
// individual functions are almost identical.
//
// localtime() and gmtime() return pointers to static structures, which is
// not thread-safe, so we use the local-storage variants localtime_r gmtime_r
// if they are available, hence two versions of the macro here.

#if HAVE_LOCALTIME_R
#define date_get_proto(function, timefn, element) \
	static void function(const fn_call& fn) { \
		date_as_object* date = ensure_date_object(fn.this_ptr); \
		time_t t = (time_t)(date->value / 1000.0); \
		struct tm tm; \
		fn.result->set_int(timefn##_r(&t, &tm)->element); \
	}
#else
#define date_get_proto(function, timefn, element) \
	static void function(const fn_call& fn) { \
		date_as_object* date = ensure_date_object(fn.this_ptr); \
		time_t t = (time_t)(date->value / 1000.0); \
		fn.result->set_int(timefn(&t)->element); \
	}
#endif

/// \brief Date.getYear returns the year of the specified Date object
/// according to local time. The year is the full year minus 1900.
/// For example, the year 2000 is represented as 100.

date_get_proto(date_getyear, localtime, tm_year);

/// \brief Date.getFullYear returns the full year (a four-digit number,
/// such as 2000) of the specified Date object, according to local time.

date_get_proto(date_getfullyear, localtime, tm_year + 1900)

/// \brief Date.getMonth returns the month (0 for January, 1 for February,
/// and so on) of the specified Date object, according to local time

date_get_proto(date_getmonth, localtime, tm_mon)

/// \brief Date.getDate returns the day of the month (an integer from 1 to 31)
/// of the specified Date object according to local time.

date_get_proto(date_getdate, localtime, tm_mday)

/// \brief Date.getDay returns the day of the week (0 for Sunday, 1 for Monday,
/// and so on) of the specified Date object according to local time.

date_get_proto(date_getday, localtime, tm_wday)

/// \brief Date.getHours returns the hour (an integer from 0 to 23)
/// of the specified Date object, according to local time

date_get_proto(date_gethours, localtime, tm_hour)

/// \brief Date.getMinutes returns the minutes (an integer from 0 to 59)
/// of the specified Date object, according to local time

date_get_proto(date_getminutes, localtime, tm_min)

/// \brief Date.getSeconds returns the seconds (an integer from 0 to 59)
/// of the specified Date object, according to local time

date_get_proto(date_getseconds, localtime, tm_sec)

/// \brief Date.getMilliseconds returns the milliseconds (an integer
/// from 0 to 999) of the specified Date object. Localtime is irrelevant!
//
// Also implements Date.getUTCMilliseconds

static void date_getmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int((int) std::fmod(date->value, 1000.0));
}

// The same functions for universal time.
//
date_get_proto(date_getutcfullyear, gmtime, tm_year + 1900)
date_get_proto(date_getutcmonth,    gmtime, tm_mon)
date_get_proto(date_getutcdate,     gmtime, tm_mday)
date_get_proto(date_getutcday,      gmtime, tm_wday)
date_get_proto(date_getutchours,    gmtime, tm_hour)
date_get_proto(date_getutcminutes,  gmtime, tm_min)
date_get_proto(date_getutcseconds,  gmtime, tm_sec)
// date_getutcmilliseconds is implemented by date_getmilliseconds.


/// \brief Date.getTimezoneOffset returns the difference in minutes between
/// the computer's local time and universal time as minutes east of GMT.
//
/// In fact this has nothing to do with the Date object, since it depends on
/// the system timezone settings, not on the value in a Date.

// Return number of minutes east of GMT, also used in toString()

// Yet another implementation option is suggested by localtime(3):
// "The glibc version of struct tm has additional fields
// long tm_gmtoff;           /* Seconds east of UTC */
// defined when _BSD_SOURCE was set before including <time.h>"

static int minutes_east_of_gmt()
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	return(-tz.tz_minuteswest);
#elif defined(HAVE_TZSET)
	tzset();
	return(-timezone/60); // timezone is seconds west of GMT
#else
	// ftime(3): "These days the contents of the timezone and dstflag
	// fields are undefined."
	// In practice, timezone is -120 in Italy when it should be -60.
	// Still, mancansa d'asu, t'acuma i buoi.
	struct timeb tb;
		
	ftime (&tb);
	// tb.timezone is number of minutes west of GMT
	return(-tb.timezone);
#endif
}

static void date_gettimezoneoffset(const fn_call& fn) {
	fn.result->set_int(minutes_east_of_gmt());
}


//
//    =========    Functions to set dates in various ways    ========
//

/// \brief Date.setTime sets the date for the specified Date object
/// in milliseconds since midnight on January 1, 1970, and returns
/// the new time in milliseconds.
static void date_settime(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setTime needs one argument");
	    )
	} else
		date->value = fn.arg(0).to_number();

	if (fn.nargs > 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setTime was called with more than one argument");
	    )
	}

	fn.result->set_double(date->value);
}

//
// Functions to set just some components of a Date.
//
// We do this by exploding the datestamp into the calendar components,
// setting the fields that are to be changed, then converting back.
//

// The Adobe player 9 behaves strangely. e.g., after "new date = Date(0)":
// date.setYear(1970); date.setMonth(1); date.setDate(29); gives Mar 1 but
// date.setYear(1970); date.setDate(29); date.setMonth(1); gives Feb 28
// I doubt we can reproduce its exact operation, so just let mktime do what
// it wants with rogue values.

// We need two sets of the same functions: those that take localtime values
// and those that take UTC (GMT) values.
// Since there are a lot of them and they are hairy, we write one set that,
// if an additional extra parameter is passed, switch to working in UTC
// instead. Apart from the bottom-level conversions they are identical.


// Two low-level functions to convert between datestamps and time structures
// whose contents are in local time

// convert flash datestamp (number of milliseconds since the epoch as a double)
// to time structure and remaining milliseconds expressed in localtime.
static void
local_date_to_tm_msec(date_as_object* &date, struct tm &tm, double &msec)
{
	time_t t = (time_t)(date->value / 1000.0);
	msec = std::fmod(date->value, 1000.0);
	_localtime_r(&t, &tm);	// break out date/time elements
}

// convert Unix time structure and the remaining milliseconds to
// Flash datestamp
static void
local_tm_msec_to_date(struct tm &tm, double &msec, date_as_object* &date)
{
	time_t t = mktime(&tm);

	// Reconstruct the time value and put the milliseconds back in.
	// If mktime fails to reconstruct the date, change nothing.
	if (t == (time_t)(-1)) {
		log_error("Failed to set a date.\n");
	} else {
		date->value = t * 1000.0 + msec;
	}
}

// Two low-level functions to convert between datestamps and time structures
// whose contents are in UTC
//
// Unfortunately, mktime() only works in localtime.
// gmtime() will split it for us, but how do we put it back together again?

static void
utc_date_to_tm_msec(date_as_object* &date, struct tm &tm, double &msec)
{
	time_t t = (time_t)(date->value / 1000.0);
	msec = std::fmod(date->value, 1000.0);
	_gmtime_r(&t, &tm);
}

// TODO:
// Until we find the correct algorithm, we can use mktime which, by
// experiment, seems to flip timezone at midnight, not at 2 in the morning,
// so we use that to do year/month/day and put the unadjusted hours/mins/secs
// in by hand. It's probably not right but it'll do for the moment.

static void
utc_tm_msec_to_date(struct tm &tm, double &msec, date_as_object* &date)
{
	time_t t = mktime(&tm);
	if (t == (time_t)(-1)) {
	    log_error("utc_tm_msec_to_date failed to convert back to Date");
	} else {
	    // Knock out the H:M:S part of t and replace with UTC time-of-day
	    t = t - (t % 86400) + tm.tm_sec + 60 * (tm.tm_min + 60 * tm.tm_hour);
	}
	
	date->value = t * 1000.0 + msec;
}

// Now the generic version of these two functions that switch according to
// what the customer asked for

static void
tm_msec_to_date(struct tm &tm, double &msec, date_as_object* &date, bool utc)
{
    if (utc)
	utc_tm_msec_to_date(tm, msec, date);
    else
	local_tm_msec_to_date(tm, msec, date);
}

static void
date_to_tm_msec(date_as_object* &date, struct tm &tm, double &msec, bool utc)
{
    if (utc)
	utc_date_to_tm_msec(date, tm, msec);
    else
	local_date_to_tm_msec(date, tm, msec);
}

//
// Compound functions that can set one, two, three or four fields at once.
//
// There are two flavours: those that work with localtime and those that do so
// in UTC (except for setYear, which has no UTC version). We avoid duplication
// by passing an optional variable "utc": if present and true, we use the
// UTC conversion funtions. Otherwise the localtime ones.
//

/// \brief Date.setFullYear(year[,month[,day]])
/// sets the year of the specified Date object according to local time
/// and returns the new time in milliseconds.
//
/// If the month and date parameters are specified, they are set in local time.
/// year: A four-digit number specifying a year.
///	Two-digit numbers do not represent four-digit years;
///	for example, 99 is not the year 1999, but the year 99.
/// month: An integer from 0 (January) to 11 (December). [optional]
/// day: An integer from 1 to 31. [optional]
///
/// If the month and/or day are omitted, they are left at their current values.
/// If changing the year or month results in an impossible date, it is
/// normalised: 29 Feb becomes 1 Mar, 31 April becomes 1 May etc.

static void _date_setfullyear(const fn_call& fn, bool utc) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setFullYear needs one argument");
	    )
	} else {
	    struct tm tm; double msec;

	    date_to_tm_msec(date, tm, msec, utc);
	    tm.tm_year = (int) fn.arg(0).to_number() - 1900;
	    if (fn.nargs >= 2)
		    tm.tm_mon = (int) fn.arg(1).to_number();
	    if (fn.nargs >= 3)
		    tm.tm_mday = (int) fn.arg(2).to_number();
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setFullYear was called with more than three arguments");
		)
	    }
	    tm_msec_to_date(tm, msec, date, utc);
	}
	fn.result->set_double(date->value);
}

/// \brief Date.setYear(year[,month[,day]])
/// sets the year for the specified Date object in local time
/// and returns the new time in milliseconds
/// If year is an integer between 0-99, setYear sets the year at 1900 + year;
/// otherwise, the year is a four-digit one.
//
/// Contrary to the spec at sephiroth.it, this takes and acts on optional
/// parameters month and day; if they are unspecified their values are not
/// changed.
///
/// There is no setUTCYear() function.

static void date_setyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setYear needs one argument");
	    )
	} else {
	    struct tm tm; double msec;

	    date_to_tm_msec(date, tm, msec, false);
	    tm.tm_year = (int) fn.arg(0).to_number();
	    if (tm.tm_year < 100) tm.tm_year += 1900;
	    if (fn.nargs >= 2)
		    tm.tm_mon = (int) fn.arg(1).to_number();
	    if (fn.nargs >= 3)
		    tm.tm_mday = (int) fn.arg(2).to_number();
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setYear was called with more than three arguments");
		)
	    }
	    tm_msec_to_date(tm, msec, date, false);
	}
	fn.result->set_double(date->value);
}

/// \brief Date.setMonth(month[,date]) sets the month and optionally the day
/// of the month for the specified Date object in local time and returns
/// the new time in milliseconds
//
/// month: An integer from 0 (January) to 11 (December).
/// date: An integer from 1 to 31. [optional]

static void _date_setmonth(const fn_call& fn, bool utc=false) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setMonth needs one argument");
	    )
	} else {
	    struct tm tm; double msec;

	    date_to_tm_msec(date, tm, msec, utc);
	    tm.tm_mon = (int) fn.arg(0).to_number();
	    if (fn.nargs >= 2)
		    tm.tm_mday = (int) fn.arg(2).to_number();
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMonth was called with more than three arguments");
		)
	    }
	    tm_msec_to_date(tm, msec, date, utc);
	}
	fn.result->set_double(date->value);
}

/// \brief Date.setDate(date) sets the day of the month for the specified Date
/// object according to local time, and returns the new time in milliseconds.
//
/// date: An integer from 1 to 31

static void _date_setdate(const fn_call& fn, bool utc=false) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setDate needs one argument");
	    )
	} else {
		struct tm tm; double msec;

		date_to_tm_msec(date, tm, msec, utc);
		tm.tm_mday = (int)(fn.arg(0).to_number());
		tm_msec_to_date(tm, msec, date, utc);
	}
	if (fn.nargs > 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setDate was called with more than one argument");
	    )
	}
	fn.result->set_double(date->value);
}

/// \brief Date.setHours(hour[,min[,sec[,millisec]]])
/// sets the components of the specified Date object according to local time
/// and returns the new time in milliseconds
//
/// hour: An integer from 0 (midnight) to 23 (11 p.m.).
/// min: An integer from 0 to 59. [optional]
/// sec: An integer from 0 to 59. [optional]
/// millisec: An integer from 0 to 999. [optional]
///
/// If optional fields are omitted, their values in the Date object
/// are left the same as they were.
///
/// Contrary to the spec at sephiroth.it, this takes and acts on optional
/// parameters min, sec and millisec.

static void _date_sethours(const fn_call& fn, bool utc=false) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 4);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setHours needs one argument");
	    )
	} else {
	    struct tm tm; double msec;

	    date_to_tm_msec(date, tm, msec, utc);
	    tm.tm_hour = (int) fn.arg(0).to_number();
	    if (fn.nargs >= 2)
		    tm.tm_min = (int) fn.arg(1).to_number();
	    if (fn.nargs >= 3)
		    tm.tm_sec = (int) fn.arg(2).to_number();
	    if (fn.nargs >= 4)
		    msec = fn.arg(3).to_number();
	    if (fn.nargs > 4) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setHours was called with more than four arguments");
		)
	    }
	    tm_msec_to_date(tm, msec, date, utc);
	}
	fn.result->set_double(date->value);
}

/// \brief Date.setMinutes(minutes[,secs[,millisecs]]) sets the given fields
/// for a specified Date object according to local time and returns the new
/// time in milliseconds.
//
/// Contrary to the spec at sephiroth.it, this takes and acts on optional
/// extra parameters secs and millisecs.

static void _date_setminutes(const fn_call& fn, bool utc=false) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	//assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setMinutes needs one argument");
	    )
	} else {
	    struct tm tm; double msec;

	    date_to_tm_msec(date, tm, msec, utc);
	    tm.tm_min = (int) fn.arg(0).to_number();
	    if (fn.nargs >= 2)
		    tm.tm_sec = (int) fn.arg(1).to_number();
	    if (fn.nargs >= 3)
		    msec = fn.arg(2).to_number();
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMinutes was called with more than three arguments");
		)
	    }
	    tm_msec_to_date(tm, msec, date, utc);
	}
	fn.result->set_double(date->value);
}

/// \brief Date.setSeconds(secs[,millisecs]) sets the seconds and optionally
/// the milliseconds for the specified Date object in local time
/// and returns the new time in milliseconds.

static void _date_setseconds(const fn_call& fn, bool utc=false) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setSeconds needs one argument");
	    )
	} else {
	    // We *could* set seconds [and milliseconds] without breaking the
	    // structure out and reasembling it. We do it the same way as the
	    // rest for simplicity and in case anyone's date routines ever
	    // take account of leap seconds.
	    struct tm tm; double msec;

	    date_to_tm_msec(date, tm, msec, utc);
	    tm.tm_sec = (int) fn.arg(0).to_number();
	    if (fn.nargs >= 2)
		    msec = fn.arg(1).to_number();
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMinutes was called with more than three arguments");
		)
	    }
	    tm_msec_to_date(tm, msec, date, utc);
	}
	fn.result->set_double(date->value);
}

static void date_setmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setMilliseconds needs one argument");
	    )
	} else {
	    // Zero the milliseconds and set them from the argument.
	    date->value = std::fmod(date->value, 1000.0) + fn.arg(0).to_number();
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMilliseconds was called with more than one argument");
		)
	    }
	}
	fn.result->set_double(date->value);
}

// Bindings for localtime versions
#define local_proto(item) \
	static void date_set##item(const fn_call& fn) { \
		_date_set##item(fn, false); \
	}
local_proto(fullyear)
local_proto(month)
local_proto(date)
local_proto(hours)
local_proto(minutes)
local_proto(seconds)
#undef local_proto

// The same things for UTC.
#define utc_proto(item) \
	static void date_setutc##item(const fn_call& fn) { \
		_date_set##item(fn, true); \
	}
utc_proto(fullyear)
utc_proto(month)
utc_proto(date)
utc_proto(hours)
utc_proto(minutes)
utc_proto(seconds)
#undef utc_proto


/// \brief Date.toString()
/// returns a string value for the specified date object in a readable format
//
/// The format is "Thu Jan 1 00:00:00 GMT+0000 1970" and it is displayed in
/// local time.

static void date_tostring(const fn_call& fn) {
	char buffer[40]; // 32 chars + slop
	char* monthname[12] =
		{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char* dayweekname[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

	date_as_object* date = ensure_date_object(fn.this_ptr);
	
	time_t t = (time_t) (date->value / 1000.0);
	struct tm tm;
	int tzhours, tzminutes;

	_localtime_r(&t, &tm);

	// At the meridian we need to print "GMT+0100" when Daylight Saving
	// Time is in force, "GMT+0000" when it isn't, and other values for
	// other places around the globe when DST is/isn't in force there.
	//
	// For now, just take time East of GMT and add an hour if DST is
	// active. To get it right I guess we should call both gmtime()
	// and localtime() and look at the difference.
	//
	// According to http://www.timeanddate.com/time/, the only place that
	// uses DST != +1 hour is Lord Howe Island with half an hour. Tough.

	tzhours = (tzminutes = minutes_east_of_gmt()) / 60, tzminutes %= 60;

	if (tm.tm_isdst == 0) {
		// DST exists and is not in effect
	} else if (tm.tm_isdst > 0) {
		// DST exists and is in effect
		tzhours++;
		// The range of standard time is GMT-11 to GMT+14.
		// The most extreme with DST is Chatham Island GMT+12:45 +1DST
	} else {
		// tm_isdst is negative: cannot get TZ info.
		// Convert and print in UTC instead.
		_gmtime_r(&t, &tm);
		tzhours = tzminutes = 0;
	}

	// If timezone is negative, both hours and minutes will be negative
	// but for the purpose of printing a string, only the hour needs to
	// produce a minus sign.
	if (tzminutes < 0) tzminutes = -tzminutes;

	snprintf((char *)&buffer, sizeof(buffer),
		"%s %s %d %02d:%02d:%02d GMT%+03d%02d %d",
		dayweekname[tm.tm_wday], monthname[tm.tm_mon],
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		tzhours, tzminutes, tm.tm_year+1900);

	fn.result->set_string((char *)&buffer);
}

// Date.UTC(year:Number,month[,date[,hour[,minute[,second[,millisecond]]]]]
// returns the number of milliseconds between midnight on January 1, 1970,
// universal time, and the time specified in the parameters.
//
// This is a static method that is invoked through the Date object constructor,
// not through a specific Date object. This method lets you create a Date object
// that assumes universal time, whereas the Date constructor assumes local time.
//
// year: A four-digit integer that represents the year (for example, 2000).
//
// e.g. var maryBirthday_date:Date = new Date(Date.UTC(1974, 7, 12));

static void date_utc(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	UNUSED(date);
	log_msg("Date.UTC is unimplemented\n");
}

/// \brief Date.valueOf() returns the number of milliseconds since midnight
/// January 1, 1970, universal time, for this Date.
/// Also used for Date.getTime()
static void date_valueof(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_double(date->value);
}

// extern (used by Global.cpp)
void date_class_init(as_object& global)
{
	// This is going to be the global String "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&date_new, getDateInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachDateStaticInterface(*cl);
	}

	// Register _global.Date
	global.init_member("Date", cl.get());

}

} // end of gnash namespace

