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
// BUGS:
//	Flash Player does not seem to respect TZ or the zoneinfo database;
//	It changes to/from daylight saving time according to its own rules.
//	We use the operating system's localtime routines.
//
//	Flash player handles a huge range of dates, including
//	thousands of years BC, but the localtime code here only works for
//	valid Unix dates Dec 13 20:45:52 1901 - Jan 19 03:14:07 2038.
//	Our UTC code could be hacked into working outside this range but
//	this is not worth doing unless we also implement full-range localtime
//	operations too.
//
// To probe FlashPlayer functionality put something like:
// class Test {
//        }
//
//        static function main(mc) {
//		var now = new Date();
//		var s:String;
//               _root.createTextField("tf",0,0,0,320,200);
//		s = now.toString();	// or whatever
//		trace(s);			// output in gnash -v
//              _root.tf.text = now.toString(); // output in flash player
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
// However, ftime() is the only reliable way to get the timezone offset.

#if HAVE_FTIME
# include <sys/timeb.h>		// for ftime()
#endif

// Is broken on BSD where timezone(int,int) is a function (!)
// See comment at line 497 for a better implementation
#undef HAVE_TZSET	

#if HAVE_TZSET
extern long timezone;		// for tzset()/timezone
#endif

namespace gnash {

// We have our own UTC time converter that works with Flash timestamps
// (double milliseconds after 1970) rather than the time-limited time_t.
// Define USE_UTCCONV as 1 to use these versions;
// Undefine USE_UTCCONV or define it as 0 to use gmtime_r() and mktime()
// for UTC date conversions (the mktime() versions sometimes set the time
// wrong by an hour).
//
// Currently, without this, setting times in UTC to a moment when DST is active
// gets the hour and datestamp wrong, and changing the date into/out of a
// DST period without adjusts the UTC time of day (it shouldn't).
#define USE_UTCCONV 1

#if USE_UTCCONV
// forward declarations
static void utctime(double tim, struct tm *tmp, double *msecp);
static double mkutctime(struct tm *tmp, double msec);
#endif

// Select functions to implement _localtime_r and _gmtime_r
// For localtime we use the glibc stuff; for UTC we prefer our own routines
// because the C library does not provide a function to convert
// from struct tm to datestamp in UTC.

#if HAVE_LOCALTIME_R
	// Use the library function
#	define _localtime_r localtime_r

# if USE_UTCCONV
static struct tm *
_gmtime_r(time_t *t, struct tm *tm)
{
	double msec;
	utctime(*t * 1000.0, tm, &msec);
	return(tm);
}
# else
#	define _gmtime_r gmtime_r
# endif

#else // HAVE_LOCALTIME_R

// Roll our own compatible versions rather than checking the ifdef everywhere
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
#if USE_UTCCONV
	double msec;
	utctime(*t * 1000.0, &tm, &msec);
#else
	struct tm *tmp;
	tmp = gmtime(t);
	memcpy(tm, tmp, sizeof(struct tm));
	return(tm);
#endif
}
#endif // HAVE_LOCALTIME_R

// A modified version of mktime that works in localtime on struct tm
// without you having to set tm_isdst.
// If the real mktime() sees isdst==0 with a DST date, it sets
// t_isdst and modifies the hour fields, but we need to set the
// specified hour in the localtime in force at that time.
//
// To do this we set tm_isdst to the correct value for that moment in time
// by doing an initial conversion of the time to find out is_dst for that
// moment without DST, then do the real conversion.
// This may get things wrong around the hour when the clocks go back or forth.
static time_t
_mktime(struct tm *tmp)
{
	struct tm tm2 = *tmp;
	time_t t2;

       	tm2.tm_isdst = 0;
	t2 = mktime(&tm2);		// Convert the time without DST,
	_localtime_r(&t2, &tm2);	// find out whether DST was in force
	tmp->tm_isdst = tm2.tm_isdst;	// and apply that to the given time
	return(mktime(tmp));
}


// forward declarations
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

		switch (fn.nargs) {
		default:
		    IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("Date constructor called with more than 7 arguments");
		    )
		case 7:
			// fractions of milliseconds are ignored
			millisecs = (int)fn.arg(6).to_number();
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

		utcsecs = _mktime(&tm);	// convert from local time
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
// individual functions are small and almost identical.

// This calls _gmtime_r and _localtime_r, which are defined above into
// gmtime_r and localtime_r or our own local equivalents.

#define date_get_proto(function, timefn, element) \
	static void function(const fn_call& fn) { \
		date_as_object* date = ensure_date_object(fn.this_ptr); \
		time_t t = (time_t)(date->value / 1000.0); \
		struct tm tm; \
		fn.result->set_int(_##timefn##_r(&t, &tm)->element); \
	}

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
#if HAVE_TZSET
	tzset();
	return(-timezone/60); // timezone is seconds west of GMT
#elif HAVE_FTIME
	// ftime(3): "These days the contents of the timezone and dstflag
	// fields are undefined."
	// In practice, timezone is -120 in Italy when it should be -60.
	struct timeb tb;
		
	ftime (&tb);
	// tb.timezone is number of minutes west of GMT
	return(-tb.timezone);
#elif HAVE_GETTIMEOFDAY
	// gettimeofday(3):
	// "The use of the timezone structure is obsolete; the tz argument
	// should normally be specified as NULL. The tz_dsttime field has
	// never been used under Linux; it has not been and will not be
	// supported by libc or glibc."
	// Still, mancansa d'asu, t'acuma i buoi.
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	return(-tz.tz_minuteswest);
#else
	return(0);	// No idea.
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
// For now, just let mktime do as it pleases with rogue values.

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

// Convert Unix time structure and the remaining milliseconds to
// Flash datestamp.
static void
local_tm_msec_to_date(struct tm &tm, double &msec, date_as_object* &date)
{
	time_t t = _mktime(&tm);

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
// gmtime() will split it for us, but mktime() only works in localtime.

static void
utc_date_to_tm_msec(date_as_object* &date, struct tm &tm, double &msec)
{
#if USE_UTCCONV
	utctime(date->value, &tm, &msec);
#else
	time_t t = (time_t)(date->value / 1000.0);
	msec = std::fmod(date->value, 1000.0);
	_gmtime_r(&t, &tm);
#endif
}

// TODO:
// Until we find the correct algorithm, we can use mktime which, by
// experiment, seems to flip timezone at midnight, not at 2 in the morning,
// so we use that to do year/month/day and put the unadjusted hours/mins/secs
// in by hand. It's probably not right but it'll do for the moment.

static void
utc_tm_msec_to_date(struct tm &tm, double &msec, date_as_object* &date)
{
#if USE_UTCCONV
	date->value = mkutctime(&tm, msec);
#else
	time_t t = mktime(&tm);
	if (t == (time_t)(-1)) {
	    log_error("utc_tm_msec_to_date failed to convert back to Date");
	} else {
	    // Knock out the H:M:S part of t and replace with UTC time-of-day
	    t = t - (t % 86400) + tm.tm_sec + 60 * (tm.tm_min + 60 * tm.tm_hour);
	}
	
	date->value = t * 1000.0 + msec;
#endif
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

// When changing the year/month/date from a date in Daylight Saving Time to a
// date not in DST or vice versa, with setYear and setFullYear the hour of day
// remains the same in *local time* not in UTC.
// So if a date object is set to midnight in January and you change the date
// to June, it will still be midnight localtime.
//
// When using setUTCFullYear instead, the time of day remains the same *in UTC*
// so, in the northern hemisphere, changing midnight from Jan to June gives
// 01:00 localtime.
//
// Heaven knows what happens if it is 1.30 localtime and you change the date
// to the day the clocks go forward.

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
// Contrary to the spec at sephiroth.it, this takes and acts on optional
// parameters month and day; if they are unspecified their values are not
// changed.
//
// There is no setUTCYear() function.

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

static void _date_setmonth(const fn_call& fn, bool utc) {
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

static void _date_setdate(const fn_call& fn, bool utc) {
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
//
// Contrary to the spec at sephiroth.it, this takes and acts on optional
// parameters min, sec and millisec.
//
// Flash Player only takes notice of the whole part of millisec,
// truncating it, not rounding it. The only way to set a fractional number of
// milliseconds is to use setTime(n) or call the constructor with one argmuent.

static void _date_sethours(const fn_call& fn, bool utc) {
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
		    msec = (int) fn.arg(3).to_number();
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

static void _date_setminutes(const fn_call& fn, bool utc) {
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
		    msec = (int) fn.arg(2).to_number();
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

static void _date_setseconds(const fn_call& fn, bool utc) {
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
		    msec = (int) fn.arg(1).to_number();
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
	    date->value = std::fmod(date->value, 1000.0) + (int) fn.arg(0).to_number();
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

#ifdef USE_UTCCONV
/*
 * This routine converts time as follows.
 * The epoch is 00:00 Jan 1 1970 GMT.
 * The argument time is in seconds since then.
 * utctime() fills the structure pointed to by tmp as follows:
 *
 *	tm_sec		seconds (0-59)
 *	tm_min		minutes (0-59)
 *	tm_hour		hours (0-23)
 *	tm_mday		day of month (1-31)
 *	tm_mon		month (0-11)
 *	tm_year		year - 1900 (70- )
 *	tm_wday		weekday (0-6, Sun is 0)
 *	tm_yday		day of the year (0-364/5)
 *	tm_isdst	is daylight saving time in force? (always 0 = GMT)
 *
 *	Reference: Algorithm 199 by Robert G. Tantzen
 *	from "Collected Algorithms from ACM" Volume 1
 *	Published by the Association for Computing Machinery, 1980.
 *	See http://portal.acm.org/citation.cfm?id=390020 (pay-to-know site)
 *	See also http://ftp.unicamp.br/pub/unix-c/calendars/jday-jdate.c
 *
 *	When munging this, bear in mind that for calculation
 *	the year starts on March the 1st (yday == 0)
 *
 *	These routines have been tested exhaustively against gmtime() and
 *	may work for dates outside the range that gmtime() can handle
 *	(Dec 13 20:45:52 1901 - Jan 19 03:14:07 2038)
 *	Dates up to 2100 should work; ones before 1900 I doubt it.
 */

static void
utctime(double tim, struct tm *tmp, double *msecp)
{
	register int d; /* workhorse variable */
	register int y, m, a;

	/*
	 *	This routine is good until year 2100.
	 */

	*msecp = std::fmod(tim, 1000.0); tim = trunc(tim / 1000.0);
	tmp->tm_sec = (d = (int) std::fmod(tim, 86400.0)) % 60;
	tmp->tm_min = (d/=60) % 60;
	tmp->tm_hour = d/60;

	d = (int) trunc(tim / 86400.0); /* no of days after 1 Jan 1970 */

	/* Make time of day positive when time is negative */
	if (tim < 0) {
		if (*msecp < 0) { *msecp += 1000; tmp->tm_sec--; }
		if (tmp->tm_sec < 0) { tmp->tm_sec += 60; tmp->tm_min--; }
		if (tmp->tm_min < 0) { tmp->tm_min += 60; tmp->tm_hour--; }
		if (tmp->tm_hour < 0) { tmp->tm_hour += 24; d--; }
	}

	/*
	 * d is the day number after 1 Jan 1970. Generate day of the week.
	 * The addend is 4 mod 7 (1/1/1970 was Thursday)
	 */

	if (d >= -4) tmp->tm_wday = (d+4)%7;
	else tmp->tm_wday = 6 - (((-5)-d)%7);

	// 693902 is the days from 1st March 0000 to 1st Jan 1900
	// 25567 is the days from 1st Jan 1900 to 1st Jan 1970
	// 10957 is the days from 1st Jan 1970 to 1st Jan 2000
	// 1461 is the number of days in 4 years

	/* deal with years 2000-2099 */
	if ( d > 10957+59 /* 29 Feb 2000 */ ) y = 100; else y = 0;
	y += ( d = ( 4 * ( d + 693902+25567 ) - 1 ) % 146097 | 3 ) / 1461;
	a = ( d = ( d % 1461 ) / 4 + 1 ) - 307;
	m = ( ( d *= 5 ) - 3 ) / 153;
	tmp->tm_mday = ( d + 2 - 153 * m ) / 5;

	/* adjust for fact that day==0 is 1st Mar */
	if ((m += 2) > 11) { m -= 12; y++; }
	tmp->tm_yday = ( a >= 0 ? a : a+365 );
	if ( (y & 3) == 0 && a < 0) tmp->tm_yday++;
	tmp->tm_mon=m;
	tmp->tm_year=y;
	tmp->tm_isdst = 0;
}

/* Convert a gregorian calendar date to milliseconds since 1 Jan 1970 UTC */
static double
mkutctime(struct tm *tmp, double msec)
{
	int d = tmp->tm_mday;
	int m = tmp->tm_mon + 1;
	int ya = tmp->tm_year;	/* Years since 1900 */
	int k;	/* day number since 1 Jan 1900 */

	// For calculation, convert to a year starting on 1 March
	if (m > 2) m -= 3;
	else {
		m += 9;
		ya--;
	}

	k = (1461 * ya) / 4 + (153 * m + 2) / 5 + d + 58;

	/* K is now the day number since 1 Jan 1900.
	 * Convert to minutes since 1 Jan 1970 */
	/* 25567 is the number of days from 1 Jan 1900 to 1 Jan 1970 */
	k = ((k - 25567) * 24 + tmp->tm_hour) * 60 + tmp->tm_min;
	
	// Converting to double after minutes allows for +/- 4082 years with
	// 32-bit signed integers.
	return  (k * 60.0 + tmp->tm_sec) * 1000.0 + msec;
}
#endif // UTCCONV

} // end of gnash namespace

