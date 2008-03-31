// Date.cpp:  ActionScript class for date and time, for Gnash.
// 
//  Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// Date.cpp
//
// Implements methods of the ActionScript "Date" class for Gnash
//
// TODO:
//  What does Flash setTime() do/return if you hand it 0 parameters?
//
// BUGS:
//  Flash player handles a huge range of dates, including
//  thousands of years BC, but the localtime code here only works for
//  valid Unix dates Dec 13 20:45:52 1901 - Jan 19 03:14:07 2038.
//  Our UTC code could be hacked into working outside this range but
//  this is not worth doing unless we also implement full-range localtime
//  operations too.
//
//  We probably get negative datestamps (1901-1969) wrong sometimes but
//  don't really care that much.
//
// FEATURES:
//  Flash Player does not seem to respect TZ or the zoneinfo database;
//  It changes to/from daylight saving time according to its own rules.
//  We use the operating system's localtime routines.
//
//  Flash player does bizarre things for some argument combinations,
//  returning datestamps of /6.*e+19  We don't bother doing this...
//
// It may be useful to convert this to use libboost's date_time stuff
// http://www.boost.org/doc/html/date_time.html
// Pros:
// *  OS portability is done by libboost, not here;
// *  it extends correct date handling from POSIX to 1 Jan 1400 - 31 Dec 9999
// Cons:
// *  it doesn't handle fractions of milliseconds (and who cares?);
// *  mapping between boost's coherent date_time methods and Flash's
//  ideosyncratic ones to implement this class' methods is more tricky;
// *  it brings the need to handle all boundary cases and exceptions
//  explicitly (e.g. mapping of 38 Nov to 8 Dec, mapping negative
//  month/day-of-month/hours/min/secs/millisecs into the previous
//  year/month/day/hour/min/sec and so on).
//
// To probe FlashPlayer functionality put something like:
// class Test {
//        }
//
//        static function main(mc) {
//    var now = new Date();
//    var s:String;
//    s = now.toString(); // or whatever
//              _root.createTextField("tf",0,0,0,320,200);
//              _root.tf.text = now.toString(); // output in flash player
//    trace(s);     // output in gnash -v
//        }
// }
// in test.as, compile with
//  mtasc -swf test.swf -main -header 320:200:10 test.as
// and get someone to open test.swf for you in the commercial Flash Player.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "utility.h"
#include "Date.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "Object.h" // for getObjectInterface

#include <ctime>
#include <cmath>
#include <boost/format.hpp>

#if defined(_WIN32) || defined(WIN32)
# include <sys/time.h>
#else
# include <sys/time.h>
#endif


// Declaration for replacement timezone functions
// In the absence of gettimeofday() we use ftime() to get milliseconds,
// but not for timezone offset because ftime's TZ stuff is unreliable.
// For that we use tzset()/timezone if it is available.

// The structure of these ifdefs mimics the structure of the code below
// where these things are used if available.

#if !defined(HAVE_GETTIMEOFDAY) || (!defined(HAVE_TM_GMTOFF) && !defined(HAVE_TZSET))
#ifdef HAVE_FTIME
extern "C" {
#  include <sys/types.h>    // for ftime()
#  include <sys/timeb.h>    // for ftime()
}
#endif
#endif

#if !defined(HAVE_TM_GMTOFF)
# ifdef HAVE_LONG_TIMEZONE
extern long timezone;   // for tzset()/long timezone;
# endif
#endif

namespace gnash {

// We have our own UTC time converter that works with Flash timestamps
// (double milliseconds after 1970) rather than the time-limited time_t.
//
// Currently, without this, setting times in UTC to a moment when DST is active
// gets the hour and datestamp wrong, and changing the date into/out of a
// DST period changes the UTC time of day (it shouldn't).

// forward declarations
static void utctime(double tim, struct tm *tmp, double *msecp);
static double makeUTCTime(struct tm *tmp, double msec);
static void local_date_to_tm_msec(double value, struct tm &tm, double &msec);
static void utc_date_to_tm_msec(double value, struct tm &tm, double &msec);
static double local_tm_msec_to_date(struct tm &tm, double &msec);
static double utc_tm_msec_to_date(struct tm &tm, double &msec);
static double rogue_date_args(const fn_call& fn, unsigned maxargs);

static int minutes_east_of_gmt(struct tm &tm);

// Select functions to implement _localtime_r and _gmtime_r
// For localtime we use the glibc stuff; for UTC we prefer our own routines
// because the C library does not provide a function to convert
// from struct tm to datestamp in UTC.

#ifdef HAVE_LOCALTIME_R
  // Use the library function
# define _localtime_r localtime_r

static struct tm *
_gmtime_r(time_t *t, struct tm *tm)
{
  double msec;
  utctime(*t * 1000.0, tm, &msec);
  return(tm);
}
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
  double msec;
  utctime(*t * 1000.0, tm, &msec);
  return(tm);
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
// This may still get things wrong around the hour when the clocks go back
// or forth.
static time_t
_mktime(struct tm *tmp)
{
  struct tm tm2 = *tmp;
  time_t t2;

        tm2.tm_isdst = 0;
  t2 = mktime(&tm2);    // Convert the time without DST,
  _localtime_r(&t2, &tm2);  // find out whether DST was in force
  tmp->tm_isdst = tm2.tm_isdst; // and apply that to the given time
  return(mktime(tmp));
}


// forward declarations
static as_value date_new(const fn_call& fn);
static as_value date_getdate(const fn_call& fn);
static as_value date_getday(const fn_call& fn);
static as_value date_getfullyear(const fn_call& fn);
static as_value date_gethours(const fn_call& fn);
static as_value date_getmilliseconds(const fn_call& fn);
static as_value date_getminutes(const fn_call& fn);
static as_value date_getmonth(const fn_call& fn);
static as_value date_getseconds(const fn_call& fn);
static as_value date_gettime(const fn_call& fn); 
static as_value date_gettimezoneoffset(const fn_call& fn);
static as_value date_getutcdate(const fn_call& fn);
static as_value date_getutcday(const fn_call& fn);
static as_value date_getutcfullyear(const fn_call& fn);
static as_value date_getutchours(const fn_call& fn);
static as_value date_getutcminutes(const fn_call& fn);
static as_value date_getutcmonth(const fn_call& fn);
static as_value date_getutcseconds(const fn_call& fn);
//static as_value date_getutcmilliseconds(const fn_call& fn); // == getmilliseconds
static as_value date_getyear(const fn_call& fn);
static as_value date_setdate(const fn_call& fn);
static as_value date_setfullyear(const fn_call& fn);
static as_value date_sethours(const fn_call& fn);
static as_value date_setmilliseconds(const fn_call& fn);
static as_value date_setminutes(const fn_call& fn);
static as_value date_setmonth(const fn_call& fn);
static as_value date_setseconds(const fn_call& fn);
static as_value date_settime(const fn_call& fn);
static as_value date_setutcdate(const fn_call& fn);
static as_value date_setutcfullyear(const fn_call& fn);
static as_value date_setutchours(const fn_call& fn);
//static as_value date_setutcmilliseconds(const fn_call& fn); // == setmilliseconds
static as_value date_setutcminutes(const fn_call& fn);
static as_value date_setutcmonth(const fn_call& fn);
static as_value date_setutcseconds(const fn_call& fn);
static as_value date_setyear(const fn_call& fn);
static as_value date_tostring(const fn_call& fn);
static as_value date_valueof(const fn_call& fn);

// Static AS methods
static as_value date_utc(const fn_call& fn);

static as_object* getDateInterface();
static void attachDateInterface(as_object& o);
static void attachDateStaticInterface(as_object& o);

static void
attachDateInterface(as_object& o)
{
  o.init_member("getDate", new builtin_function(date_getdate));
  o.init_member("getDay", new builtin_function(date_getday));
  o.init_member("getFullYear", new builtin_function(date_getfullyear));
  o.init_member("getHours", new builtin_function(date_gethours));
  o.init_member("getMilliseconds", new builtin_function(date_getmilliseconds));
  o.init_member("getMinutes", new builtin_function(date_getminutes));
  o.init_member("getMonth", new builtin_function(date_getmonth));
  o.init_member("getSeconds", new builtin_function(date_getseconds));
  o.init_member("getTime", new builtin_function(date_gettime));
  o.init_member("getTimezoneOffset", new builtin_function(date_gettimezoneoffset));
  o.init_member("getUTCDate", new builtin_function(date_getutcdate));
  o.init_member("getUTCDay", new builtin_function(date_getutcday));
  o.init_member("getUTCFullYear", new builtin_function(date_getutcfullyear));
  o.init_member("getUTCHours", new builtin_function(date_getutchours));
  o.init_member("getUTCMilliseconds", new builtin_function(date_getmilliseconds)); // same
  o.init_member("getUTCMinutes", new builtin_function(date_getutcminutes));
  o.init_member("getUTCMonth", new builtin_function(date_getutcmonth));
  o.init_member("getUTCSeconds", new builtin_function(date_getutcseconds));
  o.init_member("getYear", new builtin_function(date_getyear));
  o.init_member("setDate", new builtin_function(date_setdate));
  o.init_member("setFullYear", new builtin_function(date_setfullyear));
  o.init_member("setHours", new builtin_function(date_sethours));
  o.init_member("setMilliseconds", new builtin_function(date_setmilliseconds));
  o.init_member("setMinutes", new builtin_function(date_setminutes));
  o.init_member("setMonth", new builtin_function(date_setmonth));
  o.init_member("setSeconds", new builtin_function(date_setseconds));
  o.init_member("setTime", new builtin_function(date_settime));
  o.init_member("setUTCDate", new builtin_function(date_setutcdate));
  o.init_member("setUTCFullYear", new builtin_function(date_setutcfullyear));
  o.init_member("setUTCHours", new builtin_function(date_setutchours));
  o.init_member("setUTCMilliseconds", new builtin_function(date_setmilliseconds)); // same
  o.init_member("setUTCMinutes", new builtin_function(date_setutcminutes));
  o.init_member("setUTCMonth", new builtin_function(date_setutcmonth));
  o.init_member("setUTCSeconds", new builtin_function(date_setutcseconds));
  o.init_member("setYear", new builtin_function(date_setyear));
  o.init_member("toString", new builtin_function(date_tostring));
  o.init_member("valueOf", new builtin_function(date_valueof));
}

static void
attachDateStaticInterface(as_object& o)
{
  // This should *only* be available when SWF version is > 6
  // Are you sure? The online reference say it's in from v5 -martin
  o.init_member("UTC", new builtin_function(date_utc));
}

static as_object*
getDateInterface()
{
  static boost::intrusive_ptr<as_object> o;
  if ( o == NULL )
  {
    o = new as_object(getObjectInterface());
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
    double value;   // milliseconds UTC since the epoch

    date_as_object()
            :
        as_object(getDateInterface())
    {
    }

    as_value toString();

    bool isDateObject() { return true; }

private:

};


as_value
date_as_object::toString()
{
    const char* monthname[12] = { "Jan", "Feb", "Mar",
                                  "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep",
                                  "Oct", "Nov", "Dec" };
                                        
    const char* dayweekname[7] = { "Sun", "Mon", "Tue", "Wed",
                                   "Thu", "Fri", "Sat" };
  
    /// NAN and infinities all print as "Invalid Date"
    if (isnan(value) || isinf(value)) {
        return as_value("Invalid Date");
    }
  
    // The date value split out to year, month, day, hour etc and millisecs
    struct tm tm;
    double msec;
    // Time zone offset (including DST) as hours and minutes east of GMT
    int tzhours, tzminutes; 
  
    local_date_to_tm_msec(value, tm, msec);
  
    // At the meridian we need to print "GMT+0100" when Daylight Saving
    // Time is in force, "GMT+0000" when it isn't, and other values for
    // other places around the globe when DST is/isn't in force there.
  
    // Split offset into hours and minutes
    tzminutes = minutes_east_of_gmt(tm);
    tzhours = tzminutes / 60, tzminutes %= 60;
  
    // If timezone is negative, both hours and minutes will be negative
    // but for the purpose of printing a string, only the hour needs to
    // produce a minus sign.
    if (tzminutes < 0) tzminutes = - tzminutes;
  
    boost::format dateFormat("%s %s %d %02d:%02d:%02d GMT%+03d%02d %d");
    dateFormat % dayweekname[tm.tm_wday] % monthname[tm.tm_mon]
               % tm.tm_mday % tm.tm_hour % tm.tm_min % tm.tm_sec
               % tzhours % tzminutes % (tm.tm_year + 1900);
  
    return as_value(dateFormat.str());
  
}

/// \brief Date constructor
//
/// The constructor has three forms: 0 args, 1 arg and 2-7 args.
/// new Date() sets the Date to the current time of day
/// new Date(undefined[,*]) does the same.
/// new Date(timeValue:Number) sets the date to a number of milliseconds since
/// 1 Jan 1970 UTC
/// new Date(year, month[,date[,hour[,minute[,second[,millisecond]]]]])
/// creates a Date date object and sets it to a specified year/month etc
/// in local time.
/// year 0-99 means 1900-1999, other positive values are gregorian years
/// and negative values are years prior to 1900. Thus the only way to
/// specify the year 50AD is as -1850.
/// Defaults are 0 except for date (day of month) whose default it 1.

as_value
date_new(const fn_call& fn)
{
  // TODO: just make date_as_object constructor
  //       register the exported interface, don't
  //       replicate all functions !!

  date_as_object *date = new date_as_object;

  // Reject all date specifications containing Infinities and NaNs.
  // The commercial player does different things according to which
  // args are NaNs or Infinities:
  // for now, we just use rogue_date_args' algorithm
  {
    double foo;
    if ((foo = rogue_date_args(fn, 7)) != 0.0) {
      date->value = foo;
      return as_value(date);
    }
  }

  // TODO: move this to date_as_object constructor
  if (fn.nargs < 1 || fn.arg(0).is_undefined()) {
    // Set from system clock
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv,&tz);
    date->value = (double)tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
#elif defined(HAVE_FTIME)
    struct timeb tb;

    ftime (&tb);
    date->value = (double)tb.time * 1000.0 + tb.millitm;
#else
    // Poo! Use old time() to get seconds only
    date->value = time((time_t *) 0) * 1000.0;
#endif
  } else if (fn.nargs == 1) {
    // Set the value in milliseconds since 1970 UTC
    date->value = fn.arg(0).to_number();
  } else {
    struct tm tm; // time constructed from localtime components
    time_t utcsecs; // Seconds since 1970 as returned by mktime()
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
      log_aserror(_("Date constructor called with more than 7 arguments"));
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
      // (so 55AD is only expressible as -1845)
      // Fractional part of a year is ignored.
      if (tm.tm_year >= 100) tm.tm_year -= 1900;
    }

    utcsecs = _mktime(&tm); // convert from local time
    if (utcsecs == -1) {
      // mktime could not represent the time
      log_error(_("Date() failed to initialise from arguments"));
      date->value = 0;  // or undefined?
    } else {
      date->value = static_cast<double>(utcsecs) * 1000.0 + millisecs;
    }
  }
    
  return as_value(date);
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
  static as_value function(const fn_call& fn) { \
    boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr); \
    time_t t = static_cast<time_t>(date->value) / static_cast<time_t>(1000.0); \
    struct tm tm; \
    if (!_##timefn##_r(&t, &tm)) \
    { as_value rv; rv.set_nan(); return rv; } \
    return as_value(tm.element); \
  }

/// \brief Date.getYear
/// returns a Date's Gregorian year minus 1900 according to local time.

date_get_proto(date_getyear, localtime, tm_year)

/// \brief Date.getFullYear
/// returns a Date's Gregorian year according to local time.

date_get_proto(date_getfullyear, localtime, tm_year + 1900)

/// \brief Date.getMonth
/// returns a Date's month in the range 0 to 11.

date_get_proto(date_getmonth, localtime, tm_mon)

/// \brief Date.getDate
/// returns a Date's day-of-month, from 1 to 31 according to local time.

date_get_proto(date_getdate, localtime, tm_mday)

/// \brief Date.getDay
/// returns the day of the week for a Date according to local time,
/// where 0 is Sunday and 6 is Saturday.

date_get_proto(date_getday, localtime, tm_wday)

/// \brief Date.getHours
/// Returns the hour number for a Date, from 0 to 23, according to local time.

date_get_proto(date_gethours, localtime, tm_hour)

/// \brief Date.getMinutes
/// returns a Date's minutes, from 0-59, according to localtime.
// (Yes, some places do have a fractions of an hour's timezone offset
// or daylight saving time!)

date_get_proto(date_getminutes, localtime, tm_min)

/// \brief Date.getSeconds
/// returns a Date's seconds, from 0-59.
/// Localtime should be irrelevant.

date_get_proto(date_getseconds, localtime, tm_sec)

/// \brief Date.getMilliseconds
/// returns a Date's millisecond component as an integer from 0 to 999.
/// Localtime is irrelevant!
//
// Also implements Date.getUTCMilliseconds

static as_value date_getmilliseconds(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);
  return as_value((int) std::fmod(date->value, 1000.0));
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


// Return the difference between UTC and localtime+DST for a given date/time
// as the number of minutes east of GMT.
 
static int minutes_east_of_gmt(struct tm &tm)
{
#ifdef HAVE_TM_GMTOFF
  // tm_gmtoff is in seconds east of GMT; convert to minutes.
  return((int) (tm.tm_gmtoff / 60));
#else
  // Find the geographical system timezone offset and add an hour if
  // DST applies to the date.
  // To get it really right I guess we should call both gmtime()
  // and localtime() and look at the difference.
  //
  // The range of standard time is GMT-11 to GMT+14.
  // The most extreme with DST is Chatham Island GMT+12:45 +1DST

  int minutes_east;

  // Find out system timezone offset...

# if defined(HAVE_TZSET) && defined(HAVE_LONG_TIMEZONE)
  tzset();
  minutes_east = -timezone/60; // timezone is seconds west of GMT
# elif defined(HAVE_GETTIMEOFDAY)
  // gettimeofday(3):
  // "The use of the timezone structure is obsolete; the tz argument
  // should normally be specified as NULL. The tz_dsttime field has
  // never been used under Linux; it has not been and will not be
  // supported by libc or glibc."
  // Still, mancansa d'asu, t'acuma i buoi.
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv,&tz);
  minutes_east = -tz.tz_minuteswest;

# elif defined(HAVE_FTIME)
  // ftime(3): "These days the contents of the timezone and dstflag
  // fields are undefined."
  // In practice, timezone is -120 in Italy when it should be -60.
  struct timeb tb;
    
  ftime (&tb);
  // tb.timezone is number of minutes west of GMT
  minutes_east = -tb.timezone;

# else
  minutes_east = 0; // No idea.
# endif

  // ...and adjust by one hour if DST was in force at that time.
  //
  // According to http://www.timeanddate.com/time/, the only place that
  // uses DST != +1 hour is Lord Howe Island with half an hour. Tough.

  if (tm.tm_isdst == 0) {
    // DST exists and is not in effect
  } else if (tm.tm_isdst > 0) {
    // DST exists and was in effect
    minutes_east += 60;
  } else {
    // tm_isdst is negative: cannot get TZ info.
    // Convert and print in UTC instead.
    log_error("Cannot get timezone information");
    minutes_east = 0;
  }

  return minutes_east;
#endif // HAVE_TM_OFFSET
}


/// \brief Date.getTimezoneOffset
/// returns the difference between localtime and UTC that was in effect at the
/// time specified by a Date object, according to local timezone and DST.
/// For example, if you are in GMT+0100, the offset is -60

static as_value date_gettimezoneoffset(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);
  struct tm tm;
  double msec;

  if (fn.nargs > 0) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror("Date.getTimezoneOffset was called with parameters");
      )
  }

  // Turn Flash datestamp into tm structure...
  local_date_to_tm_msec(date->value, tm, msec);

  // ...and figure out the timezone/DST offset from that.
  return as_value(-minutes_east_of_gmt(tm));
}


//
//    =========    Functions to set dates in various ways    ========
//

/// \brief Date.setTime
/// sets a Date in milliseconds after January 1, 1970 00:00 UTC.
/// The return value is the same as the parameter.
static as_value date_settime(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setTime needs one argument"));
      )
  } else
    date->value = fn.arg(0).to_number();

  if (fn.nargs > 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setTime was called with more than one argument"));
      )
  }

  return as_value(date->value);
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

// convert flash datestamp (number of milliseconds since the epoch)
// to time structure and remaining milliseconds expressed in localtime.
static void
local_date_to_tm_msec(double value, struct tm &tm, double &msec)
{
  time_t t = static_cast<time_t>(value) / static_cast<time_t>(1000.0);
  msec = std::fmod(value, 1000.0);
  if (!_localtime_r(&t, &tm))
  {
    throw GnashException("Bad localtime value");
  }
}

// Convert Unix time structure and the remaining milliseconds to
// Flash datestamp.
static double
local_tm_msec_to_date(struct tm &tm, double &msec)
{
  time_t t = _mktime(&tm);

  // Reconstruct the time value and put the milliseconds back in.
  if (t == time_t(-1)) {
    // If mktime fails to reconstruct the date, return bogus value;
    // Not sure when/how this can happen. Values outside POSIX time?
    log_error(_("Failed to set a date."));
    return(NAN);
  } else {
    return(t * 1000.0 + msec);
  }
}

// Two low-level functions to convert between Flash datestamps
// and time structures whose contents are in UTC
//
// gmtime() will split it for us, but mktime() only works in localtime.

static void
utc_date_to_tm_msec(double value, struct tm &tm, double &msec)
{
  utctime(value, &tm, &msec);
}

// Until we find a better algorithm, we can use mktime which, by
// experiment, seems to flip timezone at midnight, not at 2 in the morning,
// so we use that to do year/month/day and put the unadjusted hours/mins/secs
// in by hand. It's probably not right but it'll do for the moment.

static double
utc_tm_msec_to_date(struct tm &tm, double &msec)
{
  return (makeUTCTime(&tm, msec));  // The better algorithm :)
}

// Now the generic version of these two functions that switch according to
// what the customer asked for

static void
tm_msec_to_date(struct tm &tm, double &msec, date_as_object& date, bool utc)
{
    if (utc)
  date.value = utc_tm_msec_to_date(tm, msec);
    else
  date.value = local_tm_msec_to_date(tm, msec);
}

static void
date_to_tm_msec(date_as_object& date, struct tm &tm, double &msec, bool utc)
{
    if (utc)
  utc_date_to_tm_msec(date.value, tm, msec);
    else
  local_date_to_tm_msec(date.value, tm, msec);
}

//
// Compound functions that can set one, two, three or four fields at once.
//
// There are two flavours: those that work with localtime and those that do so
// in UTC (except for setYear, which has no UTC version). We avoid duplication
// by passing an extra parameter "utc": if true, we use the UTC conversion
// functions, otherwise the localtime ones.
//
// All non-UTC functions take dates/times to be in local time and their return
// value is the new date in UTC milliseconds after 1/1/1970 00:00 UTC.
//

/// \brief Date.setFullYear(year[,month[,day]])
//
/// If the month and date parameters are specified, they are set in local time.
/// year: A four-digit number specifying a year.
/// Two-digit numbers do not represent four-digit years;
/// for example, 99 is not the year 1999, but the year 99.
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

static as_value _date_setfullyear(const fn_call& fn, bool utc) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setFullYear needs one argument"));
      )
      date->value = NAN;
  } else if (rogue_date_args(fn, 3) != 0.0) {
      date->value = NAN;
  } else {
      struct tm tm; double msec;

      date_to_tm_msec(*date, tm, msec, utc);
      tm.tm_year = (int) fn.arg(0).to_number() - 1900;
      if (fn.nargs >= 2)
        tm.tm_mon = (int) fn.arg(1).to_number();
      if (fn.nargs >= 3)
        tm.tm_mday = (int) fn.arg(2).to_number();
      if (fn.nargs > 3) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setFullYear was called with more than three arguments"));
    )
      }
      tm_msec_to_date(tm, msec, *date, utc);
  }
  return as_value(date->value);
}

/// \brief Date.setYear(year[,month[,day]])
/// if year is 0-99, this means 1900-1999, otherwise it is a Gregorian year.
/// Negative values for year set negative years (years BC).
/// This means that you cannot set a Date to the years 0-99 AD using setYear().
/// "month" is 0 - 11 and day 1 - 31 as usual.
///
/// If month and/or day are omitted, they are left unchanged except:
/// - when the day is 29, 30 or 31 and changing to a month that has less days,
///   the month gets set to the following one and the date should wrap,
///   becoming 1, 2 or 3.
/// - when changing from 29 Feb in a leap year to a non-leap year, the date
///   should end up at March 1st of the same year.
//
// There is no setUTCYear() function.

static as_value date_setyear(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  // assert(fn.nargs == 1);
  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setYear needs one argument"));
      )
      date->value = NAN;
  } else if (rogue_date_args(fn, 3) != 0.0) {
      date->value = NAN;
  } else {
      struct tm tm; double msec;

      date_to_tm_msec(*date, tm, msec, false);
      tm.tm_year = (int) fn.arg(0).to_number();
      // tm_year is number of years since 1900, so if they gave a
      // full year spec, we must adjust it.
      if (tm.tm_year >= 100) tm.tm_year -= 1900;

      if (fn.nargs >= 2)
    tm.tm_mon = (int) fn.arg(1).to_number();
      if (fn.nargs >= 3)
    tm.tm_mday = (int) fn.arg(2).to_number();
      if (fn.nargs > 3) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setYear was called with more than three arguments"));
    )
      }
      tm_msec_to_date(tm, msec, *date, false); // utc=false: use localtime
  }
  return as_value(date->value);
}

/// \brief Date.setMonth(month[,day])
/// sets the month (0-11) and day-of-month (1-31) components of a Date.
///
/// If the day argument is omitted, the new month has less days than the
/// old one and the new day is beyond the end of the month,
/// the day should be set to the last day of the specified month.
/// This implementation currently wraps it into the next month, which is wrong.

// If no arguments are given or if an invalid type is given,
// the commercial player sets the month to January in the same year.
// Only if the second parameter is present and has a non-numeric value,
// the result is NAN.
// We do not do the same because it's a bugger to code.

static as_value _date_setmonth(const fn_call& fn, bool utc) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  // assert(fn.nargs >= 1 && fn.nargs <= 2);
  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setMonth needs one argument"));
      )
      date->value = NAN;
  } else if (rogue_date_args(fn, 2) != 0.0) {
      date->value = NAN;
  } else {
      struct tm tm; double msec;
      double monthvalue; // result from to_number()

      date_to_tm_msec(*date, tm, msec, utc);

      // It seems odd, but FlashPlayer takes all bad month values to mean
      // January.
      monthvalue =  fn.arg(0).to_number();
      if (isnan(monthvalue) || isinf(monthvalue)) monthvalue = 0.0;
      tm.tm_mon = (int) monthvalue;

      // If the day-of-month value is invalid instead, the result is NAN.
      if (fn.nargs >= 2) {
    double mdayvalue = fn.arg(1).to_number();
    if (isnan(mdayvalue) || isinf(mdayvalue)) {
        date->value = NAN;
        return as_value(date->value);
    } else {
        tm.tm_mday = (int) mdayvalue;
    }
      }
      if (fn.nargs > 2) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setMonth was called with more than three arguments"));
    )
      }
      tm_msec_to_date(tm, msec, *date, utc);
  }
  return as_value(date->value);
}

/// \brief Date.setDate(day)
/// Set the day-of-month (1-31) for a Date object.
/// If the day-of-month is beyond the end of the current month, it wraps into
/// the first days of the following  month.  This also happens if you set the
/// day > 31. Example: setting the 35th in January results in Feb 4th.

static as_value _date_setdate(const fn_call& fn, bool utc) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setDate needs one argument"));
      )
      date->value = NAN;  // Is what FlashPlayer sets
  } else if (rogue_date_args(fn, 1) != 0.0) {
      date->value = NAN;
  } else {
    struct tm tm; double msec;

    date_to_tm_msec(*date, tm, msec, utc);
    tm.tm_mday = fn.arg(0).to_int();
    tm_msec_to_date(tm, msec, *date, utc);
  }
  if (fn.nargs > 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setDate was called with more than one argument"));
      )
  }
  return as_value(date->value);
}

/// \brief Date.setHours(hour[,min[,sec[,millisec]]])
/// change the time-of-day in a Date object. If optional fields are omitted,
/// their values in the Date object are left the same as they were.
///
/// If hour>23 or min/sec>59, these are accepted and wrap into the following
/// minute, hour or calendar day.
/// Similarly, negative values carry you back into the previous minute/hour/day.
//
/// Only the integer part of millisec is used, truncating it, not rounding it.
/// The only way to set a fractional number of milliseconds is to use
/// setTime(n) or call the constructor with one argument.

static as_value _date_sethours(const fn_call& fn, bool utc) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  // assert(fn.nargs >= 1 && fn.nargs <= 4);
  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setHours needs one argument"));
      )
      date->value = NAN;  // Is what FlashPlayer sets
  } else if (rogue_date_args(fn, 4) != 0.0) {
      date->value = NAN;
  } else {
      struct tm tm; double msec;

      date_to_tm_msec(*date, tm, msec, utc);
      tm.tm_hour = fn.arg(0).to_int();
      if (fn.nargs >= 2)
        tm.tm_min = fn.arg(1).to_int();
      if (fn.nargs >= 3)
        tm.tm_sec = fn.arg(2).to_int();
      if (fn.nargs >= 4)
        msec = fn.arg(3).to_int();
      if (fn.nargs > 4) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setHours was called with more than four arguments"));
    )
      }
      tm_msec_to_date(tm, msec, *date, utc);
  }
  return as_value(date->value);
}

/// \brief Date.setMinutes(minutes[,secs[,millisecs]])
/// change the time-of-day in a Date object. If optional fields are omitted,
/// their values in the Date object are left the same as they were.
///
/// If min/sec>59, these are accepted and wrap into the following minute, hour
/// or calendar day.
/// Similarly, negative values carry you back into the previous minute/hour/day.

static as_value _date_setminutes(const fn_call& fn, bool utc) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  //assert(fn.nargs >= 1 && fn.nargs <= 3);
  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setMinutes needs one argument"));
      )
      date->value = NAN;  // FlashPlayer instead leaves the date set to
        // a random value such as 9th December 2077 BC
  } else if (rogue_date_args(fn, 3) != 0.0) {
      date->value = NAN;
  } else {
      struct tm tm; double msec;

      date_to_tm_msec(*date, tm, msec, utc);
      tm.tm_min = (int) fn.arg(0).to_number();
      if (fn.nargs >= 2)
        tm.tm_sec = (int) fn.arg(1).to_number();
      if (fn.nargs >= 3)
        msec = (int) fn.arg(2).to_number();
      if (fn.nargs > 3) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setMinutes was called with more than three arguments"));
    )
      }
      tm_msec_to_date(tm, msec, *date, utc);
  }
  return as_value(date->value);
}

/// \brief Date.setSeconds(secs[,millisecs])
/// set the "seconds" component in a date object.
///
/// Values <0, >59 for secs or >999 for millisecs take the date back to the
/// previous minute (or hour or calendar day) or on to the following ones.

static as_value _date_setseconds(const fn_call& fn, bool utc) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  // assert(fn.nargs >= 1 && fn.nargs <= 2);
  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setSeconds needs one argument"));
      )
      date->value = NAN;  // Same as commercial player
  } else if (rogue_date_args(fn, 2) != 0.0) {
      date->value = NAN;
  } else {
      // We *could* set seconds [and milliseconds] without breaking the
      // structure out and reasembling it. We do it the same way as the
      // rest for simplicity and in case anyone's date routines ever
      // take account of leap seconds.
      struct tm tm; double msec;

      date_to_tm_msec(*date, tm, msec, utc);
      tm.tm_sec = fn.arg(0).to_int();
      if (fn.nargs >= 2)
        msec = fn.arg(1).to_int();
      if (fn.nargs > 2) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setMinutes was called with more than three arguments"));
    )
      }
      // This is both setSeconds and setUTCSeconds.
      // Use utc to avoid needless worrying about timezones.
      tm_msec_to_date(tm, msec, *date, utc);
  }
  return as_value(date->value);
}

static as_value date_setmilliseconds(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

  // assert(fn.nargs == 1);
  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setMilliseconds needs one argument"));
      )
      date->value = NAN;
  } else if (rogue_date_args(fn, 1) != 0.0) {
      date->value = NAN;
  } else {
      // Zero the milliseconds and set them from the argument.
      date->value = date->value - std::fmod(date->value, 1000.0) + fn.arg(0).to_int();
      if (fn.nargs > 1) {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.setMilliseconds was called with more than one argument"));
    )
      }
  }
  return as_value(date->value);
}

// Bindings for localtime versions
#define local_proto(item) \
  static as_value date_set##item(const fn_call& fn) { \
    _date_set##item(fn, false); \
    return as_value(); \
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
  static as_value date_setutc##item(const fn_call& fn) { \
    _date_set##item(fn, true); \
    return as_value(); \
  }
utc_proto(fullyear)
utc_proto(month)
utc_proto(date)
utc_proto(hours)
utc_proto(minutes)
utc_proto(seconds)
#undef utc_proto


/// \brief Date.toString()
/// convert a Date to a printable string.
/// The format is "Thu Jan 1 00:00:00 GMT+0000 1970" and it is displayed in
/// local time.

static as_value date_tostring(const fn_call& fn) {

  boost::intrusive_ptr<date_as_object> 
    date = ensureType<date_as_object>(fn.this_ptr);
  
  return date->toString();

}

// Date.UTC(year:Number,month[,day[,hour[,minute[,second[,millisecond]]]]]
//
// Convert a UTC date/time specification to number of milliseconds since
// 1 Jan 1970 00:00 UTC.
//
// unspecified optional arguments default to 0 except for day-of-month,
// which defaults to 1.
//
// year is a Gregorian year; special values 0 to 99 mean 1900 to 1999 so it is
// impossible to specify the year 55 AD using this interface.
//
// Any fractional part in the number of milliseconds is ignored (truncated)
//
// If 0 or 1 argument are passed, the result is the "undefined" value.
//
// This probably doesn't handle exceptional cases such as NaNs and infinities
// the same as the commercial player. What that does is:
// - if any argument is NaN, the result is NaN
// - if one or more of the optional arguments are +Infinity,
//  the result is +Infinity
// - if one or more of the optional arguments are -Infinity,
//  the result is -Infinity
// - if both +Infinity and -Infinity are present in the optional args,
//  or if one of the first two arguments is not numeric (including Inf),
//  the result is NaN.
// Actually, given a first parameter of Infinity,-Infinity or NAN,
// it returns -6.77681005679712e+19 but that's just crazy.
//
// We test for < 2 parameters and return undefined, but given any other
// non-numeric arguments we give NAN.


static as_value date_utc(const fn_call& fn) {
  struct tm tm; // Date structure for values down to seconds
  double millisecs; // and the miliseconds component.
  double result;  // Resulting Flash timestamp

  if (fn.nargs < 2) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.UTC needs one argument"));
      )
      return as_value();  // undefined
  }

  // Check for presence of NaNs and Infinities in the arguments 
  // and return the appropriate value if so.
  if ( (result = rogue_date_args(fn, 7)) != 0.0) {
    return as_value(NAN);
  }

  // Preset default values
  // Year and month are always given explicitly
  tm.tm_mday = 1;
  tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  tm.tm_isdst = 0;  // Not used by our UTCTIME code.
  millisecs = 0;
  switch (fn.nargs) {
  default:  // More than 7
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.UTC was called with more than 7 arguments"));
      )
  case 7:
      // millisecs is double, but fractions of millisecs are ignored.
      millisecs = (int) fn.arg(6).to_number();
  case 6:
      tm.tm_sec = (int) fn.arg(5).to_number();
  case 5:
      tm.tm_min = (int) fn.arg(4).to_number();
  case 4:
      tm.tm_hour = (int) fn.arg(3).to_number();
  case 3:
      tm.tm_mday = (int) fn.arg(2).to_number();
  case 2:   // these last two are always performed
      tm.tm_mon = (int) fn.arg(1).to_number();
      {
    int y = (int) fn.arg(0).to_number();
    if (y < 100 && y >= 0) y += 1900;
    // y is now the Gregorian year number
    tm.tm_year = y - 1900;
      }
  }

  result = utc_tm_msec_to_date(tm, millisecs);
  return as_value(result);
}

// Auxillary function checks for Infinities and NaN in a function's args and
// returns 0.0 if there are none,
// plus (or minus) infinity if positive (or negative) infinites are present,
// NAN is there are NANs present, or a mixture of positive and negative infs.
static double
rogue_date_args(const fn_call& fn, unsigned maxargs) {
  // Two flags: Did we find any +Infinity (or -Infinity) values in the
  // argument list? If so, "infinity" must be set to the kind that we
  // found.
  int plusinf_present = 0;
  int minusinf_present = 0;
  double infinity = 0.0;  // The kind of infinity we found.
        // 0.0 == none yet.

  // Only check the present parameters, up to the stated maximum number
  if (fn.nargs < maxargs) maxargs = fn.nargs;

  for (unsigned int i = 0; i < maxargs; i++) {
    double arg = fn.arg(i).to_number();

    if (isnan(arg)) return(NAN);

    if (isinf(arg)) {
      if (arg > 0) {  // Plus infinity
        plusinf_present = 1;
      } else {  // Minus infinity
        minusinf_present = 1;
      }
      // Remember the kind of infinity we found
      infinity = arg;
    }
  }
  // If both kinds of infinity were present in the args,
  // the result is NaN.
  if (plusinf_present && minusinf_present) return(NAN);

  // If only one kind of infinity was in the args, return that.
  if (plusinf_present || minusinf_present) return(infinity);
  
  // Otherwise indicate that the function arguments contained
  // no rogue values
  return(0.0);
}

/// \brief Date.valueOf() returns the number of milliseconds since midnight
/// January 1, 1970 00:00 UTC, for a Date. The return value can be a fractional
/// number of milliseconds.

static as_value date_valueof(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);

    return as_value(date->value);
}


static as_value date_gettime(const fn_call& fn) {
  boost::intrusive_ptr<date_as_object> date = ensureType<date_as_object>(fn.this_ptr);
  
  return as_value(date->value);
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

/*
 * This routine converts time as follows.
 * The epoch is 00:00 Jan 1 1970 GMT.
 * The argument time is in seconds since then.
 * utctime() fills the structure pointed to by tmp as follows:
 *
 *  tm_sec    seconds (0-59)
 *  tm_min    minutes (0-59)
 *  tm_hour   hours (0-23)
 *  tm_mday   day of month (1-31)
 *  tm_mon    month (0-11)
 *  tm_year   year - 1900 (70- )
 *  tm_wday   weekday (0-6, Sun is 0)
 *  tm_yday   day of the year (0-364/5)
 *  tm_isdst  is daylight saving time in force? (always 0 = GMT)
 *
 *  Reference: Algorithm 199 by Robert G. Tantzen
 *  from "Collected Algorithms from ACM" Volume 1
 *  Published by the Association for Computing Machinery, 1980.
 *  See http://portal.acm.org/citation.cfm?id=390020 (pay-to-know site)
 *  See also http://ftp.unicamp.br/pub/unix-c/calendars/jday-jdate.c
 *
 *  When munging this, bear in mind that for calculation
 *  the year starts on March the 1st (yday == 0)
 *
 *  These routines have been tested exhaustively against gmtime() and
 *  may work for dates outside the range that gmtime() can handle
 *  (Dec 13 20:45:52 1901 - Jan 19 03:14:07 2038)
 *  Dates up to 2100 should work; ones before 1900 I doubt it.
 */

static void
utctime(double tim, struct tm *tmp, double *msecp)
{
  register int d; /* workhorse variable */
  register int y, m, a;

  /*
   *  This routine is good until year 2100.
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
makeUTCTime(struct tm *tmp, double msec)
{
  int d = tmp->tm_mday;
  int m = tmp->tm_mon + 1;
  int ya = tmp->tm_year;  /* Years since 1900 */
  int k;  /* day number since 1 Jan 1900 */

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

} // end of gnash namespace
