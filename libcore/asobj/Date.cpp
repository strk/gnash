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
// TODO: What does Flash setTime() do/return if you hand it 0 parameters?
//
// Flash player handles a huge range of dates, including
// thousands of years BC. The timestamp value is correspondingly
// large: it is a double, which has a minimum size of 8 bytes
// in the C++ standard. Methods provided by ctime and sys/time.h
// generally rely on time_t whose size varies according to platform.
// It is not big enough to deal with all valid SWF timestamps,
// so this class uses its own methods to convert to and from 
// a time struct and the time stamp.
// 
//
// FEATURES:
// SWF Player does not seem to respect TZ or the zoneinfo database;
// It changes to/from daylight saving time according to its own rules.
// We use the operating system's localtime routines.
//
// SWF player does bizarre things for some argument combinations,
// returning datestamps of /6.*e+19  We don't bother doing this...
//
// Boost date-time handles a larger range of correct dates than
// the usual C and system functions. However, it is still limited
// to POSIX to 1 Jan 1400 - 31 Dec 9999 and will not handle
// dates at all outside this range. SWF isn't really that
// bothered by correctness; rather, it needs to handle a vast
// range of dates consistently. See http://www.boost.org/doc/html/date_time.html
//
// Pros:
//
// *  OS portability is done by libboost, not here;
//
// Cons:
//
// *  It doesn't handle fractions of milliseconds (and who cares?);
// *  Mapping between boost's coherent date_time methods and SWF's
//    idiosyncratic ones to implement this class' methods is more tricky;
// *  It brings the need to handle all boundary cases and exceptions
//    explicitly (e.g. mapping of 38 Nov to 8 Dec, mapping negative
//    month/day-of-month/hours/min/secs/millisecs into the previous
//    year/month/day/hour/min/sec and so on).
// *  It doesn't do what ActionScript wants. This is the best reason
//    not to use it for time and date functions (though for portable
//    timing it might well be useful).

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
#include "ClockTime.h"
#include "VM.h"

#include <cmath>
#include <boost/format.hpp>

// All clock time / localtime functions are in libbase/Time.cpp,
// so that portability problems are all in one place. It saves
// a lot of rebuilding too.

namespace gnash {

// A time struct to contain the broken-down time.
struct GnashTime
{
    boost::int32_t millisecond;
    boost::int32_t second;
    boost::int32_t minute;
    boost::int32_t hour;
    boost::int32_t monthday;
    boost::int32_t weekday;
    boost::int32_t month;
    boost::int32_t year;
    boost::int32_t timeZoneOffset;
};

static const int daysInMonth[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

// Seconds and milliseconds should be exactly the same whether in UTC
// or in localtime, so we always use localtime.

// forward declarations
static as_object* getDateInterface();
static void attachDateInterface(as_object& o);
static void attachDateStaticInterface(as_object& o);

static as_value date_new(const fn_call& fn);
static as_value date_getTime(const fn_call& fn); 
static as_value date_setTime(const fn_call& fn);
static as_value date_getTimezoneOffset(const fn_call& fn);
static as_value date_getYear(const fn_call& fn);
static as_value date_getFullYear(const fn_call& fn);
static as_value date_getMonth(const fn_call& fn);
static as_value date_getDate(const fn_call& fn);
static as_value date_getDay(const fn_call& fn);
static as_value date_getHours(const fn_call& fn);
static as_value date_getMinutes(const fn_call& fn);
static as_value date_getSeconds(const fn_call& fn);
static as_value date_getMilliseconds(const fn_call& fn);
static as_value date_getUTCFullYear(const fn_call& fn);
static as_value date_getUTCYear(const fn_call& fn);
static as_value date_getUTCMonth(const fn_call& fn);
static as_value date_getutcdate(const fn_call& fn);
static as_value date_getUTCDay(const fn_call& fn);
static as_value date_getUTCHours(const fn_call& fn);
static as_value date_getUTCMinutes(const fn_call& fn);
template<bool utc> as_value date_setDate(const fn_call& fn);
template<bool utc> as_value date_setfullyear(const fn_call& fn);
template<bool utc> as_value date_setHours(const fn_call& fn);
template<bool utc> as_value date_setMilliseconds(const fn_call& fn);
template<bool utc> as_value date_setMinutes(const fn_call& fn);
template<bool utc> as_value date_setmonth(const fn_call& fn);
template<bool utc> as_value date_setSeconds(const fn_call& fn);
static as_value date_setYear(const fn_call& fn);
static as_value date_tostring(const fn_call& fn);
static as_value date_valueof(const fn_call& fn);
static as_value date_UTC(const fn_call& fn);

static void fillGnashTime(const double& time, GnashTime& gt);
static double makeTimeValue(GnashTime& gt);
static void localTime(const double& time, GnashTime& gt);
static void universalTime(const double& time, GnashTime& gt);
static int localTimeZoneOffset(const double& time);

static double rogue_date_args(const fn_call& fn, unsigned maxargs);

// Helpers for calendar algorithms
inline bool
isLeapYear(boost::int32_t year)
{
    return !((year + 1900) % 400) ||
            ( !((year + 1900) % 4) && ((year + 1900) % 100));
}


inline size_t
countLeapYears(boost::int32_t year)
{
    return (year - 70) / 4 - (year - 70) / 100 + (year - 70) / 400;
}


class Date : public as_object
{
public:
    void setTimeValue(const double& value) { _value = value; }
    double getTimeValue() const { return _value; }

    Date(double value = clocktime::getTicks())
        :
        as_object(getDateInterface()),
        _value(value)
    {
    }

    as_value toString() const;

    bool isDateObject() { return true; }

private:
    double _value;
};

/// Return the broken-down time as a local time.
inline void
localTime(const double& time, GnashTime& gt)
{
    // find local timezone offset for the desired time.
    gt.timeZoneOffset = localTimeZoneOffset(time);
    fillGnashTime(time, gt);
}

/// Return the broken-down time as UTC
inline void
universalTime(const double& time, GnashTime& gt)
{
    // No time zone needed.
    gt.timeZoneOffset = 0;
    fillGnashTime(time, gt);
}


/// Safely truncate a double to an integer, returning the min()
/// limit on overflow.
template <typename T>
inline void truncateDouble(T& target, double value)
{
    if (value < std::numeric_limits<T>::min() ||
            value > std::numeric_limits<T>::max())
    {
        target = std::numeric_limits<T>::min();
        return;
    }
    target = static_cast<T>(value);

}

void registerDateNative(as_object& global)
{
    VM& vm = global.getVM();

    vm.registerNative(date_getFullYear, 103, 0); 
    vm.registerNative(date_getYear, 103, 1);
    vm.registerNative(date_getMonth, 103, 2);    
    vm.registerNative(date_getDate, 103, 3);
    vm.registerNative(date_getDay, 103, 4);
    vm.registerNative(date_getHours, 103, 5); 
    vm.registerNative(date_getMinutes, 103, 6);
    vm.registerNative(date_getSeconds, 103, 7);        
    vm.registerNative(date_getMilliseconds, 103, 8);
    vm.registerNative(date_setfullyear<false>, 103, 9);
    vm.registerNative(date_setmonth<false>, 103, 10);
    vm.registerNative(date_setDate<false>, 103, 11);
    vm.registerNative(date_setHours<false>, 103, 12);
    vm.registerNative(date_setMinutes<false>, 103, 13);
    vm.registerNative(date_setSeconds<false>, 103, 14);
    vm.registerNative(date_setMilliseconds<false>, 103, 15);
    vm.registerNative(date_getTime, 103, 16);     
    vm.registerNative(date_setTime, 103, 17);
    vm.registerNative(date_getTimezoneOffset, 103, 18);  
    vm.registerNative(date_tostring, 103, 19);
    vm.registerNative(date_setYear, 103, 20);
    vm.registerNative(date_getUTCFullYear, 103, 128);
    vm.registerNative(date_getUTCYear, 103, 129);    
    vm.registerNative(date_getUTCMonth, 103, 130);
    vm.registerNative(date_getutcdate, 103, 131);      
    vm.registerNative(date_getUTCDay, 103, 132);
    vm.registerNative(date_getUTCHours, 103, 133);
    vm.registerNative(date_getUTCMinutes, 103, 134);
    
    // These two are deliberately the same as non-UTC methods
    // as there should be no difference:
    vm.registerNative(date_getSeconds, 103, 135);
    vm.registerNative(date_getMilliseconds, 103, 136);

    vm.registerNative(date_setfullyear<true>, 103, 137);
    vm.registerNative(date_setmonth<true>, 103, 138);
    vm.registerNative(date_setDate<true>, 103, 139);
    vm.registerNative(date_setHours<true>, 103, 140);
    vm.registerNative(date_setMinutes<true>, 103, 141);
    vm.registerNative(date_setSeconds<true>, 103, 142);
    vm.registerNative(date_setMilliseconds<true>, 103, 143);
    
    //vm.registerNative(date_new, 103, 256);

    vm.registerNative(date_UTC, 103, 257);

}

// As UTC offset is measured in minutes, we can use the same
// functions to get seconds and milliseconds in local and utc time.
// But setting either of them can have a knock-on effect on minutes
// and hours, so both need their own set functions.
static void
attachDateInterface(as_object& o)
{
    VM& vm = o.getVM();

    o.init_member("getFullYear", vm.getNative(103, 0));
    o.init_member("getYear", vm.getNative(103, 1));
    o.init_member("getMonth", vm.getNative(103, 2));   
    o.init_member("getDate", vm.getNative(103, 3));
    o.init_member("getDay", vm.getNative(103, 4));
    o.init_member("getHours", vm.getNative(103, 5));
    o.init_member("getMinutes", vm.getNative(103, 6));
    o.init_member("getSeconds", vm.getNative(103, 7));
    o.init_member("getMilliseconds", vm.getNative(103, 8));
    o.init_member("setFullYear", vm.getNative(103, 9));
    o.init_member("setMonth", vm.getNative(103, 10));
    o.init_member("setDate", vm.getNative(103, 11));
    o.init_member("setHours", vm.getNative(103, 12));
    o.init_member("setMinutes", vm.getNative(103, 13));
    o.init_member("setSeconds", vm.getNative(103, 14));
    o.init_member("setMilliseconds", vm.getNative(103, 15));
    o.init_member("getTime", vm.getNative(103, 16));
    o.init_member("setTime", vm.getNative(103, 17));
    o.init_member("getTimezoneOffset", vm.getNative(103, 18));
    o.init_member("toString", vm.getNative(103, 19));
    o.init_member("setYear", vm.getNative(103, 20));
    o.init_member("getUTCFullYear", vm.getNative(103, 128));
    o.init_member("getUTCYear", vm.getNative(103, 129));
    o.init_member("getUTCMonth", vm.getNative(103, 130));
    o.init_member("getUTCDate", vm.getNative(103, 131));
    o.init_member("getUTCDay", vm.getNative(103, 132));
    o.init_member("getUTCHours", vm.getNative(103, 133));
    o.init_member("getUTCMinutes", vm.getNative(103, 134));
    o.init_member("getUTCSeconds", vm.getNative(103, 135));
    o.init_member("getUTCMilliseconds", vm.getNative(103, 136));
    o.init_member("setUTCFullYear", vm.getNative(103, 137));
    o.init_member("setUTCMonth", vm.getNative(103, 138));
    o.init_member("setUTCDate", vm.getNative(103, 139));
    o.init_member("setUTCHours", vm.getNative(103, 140));
    o.init_member("setUTCMinutes", vm.getNative(103, 141));
    o.init_member("setUTCSeconds", vm.getNative(103, 142));
    o.init_member("setUTCMilliseconds", vm.getNative(103, 143));
    o.init_member("valueOf", new builtin_function(date_valueof));

}   

static void
attachDateStaticInterface(as_object& o)
{
    VM& vm = o.getVM();
    o.init_member("UTC", vm.getNative(103, 257));
}

static as_object*
getDateInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( !o )
    {
        o = new as_object(getObjectInterface());
        attachDateInterface(*o);
    }
    return o.get();
}


as_value
Date::toString() const
{
    const char* monthname[12] = { "Jan", "Feb", "Mar",
                                  "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep",
                                  "Oct", "Nov", "Dec" };
                                        
    const char* dayweekname[7] = { "Sun", "Mon", "Tue", "Wed",
                                   "Thu", "Fri", "Sat" };
  
    /// NaN and infinities all print as "Invalid Date"
    if (isNaN(_value) || isInf(_value)) {
        return as_value("Invalid Date");
    }
  
    // The date value split out to year, month, day, hour etc and millisecs
    GnashTime gt;
    // Time zone offset (including DST) as hours and minutes east of GMT

    localTime(_value, gt);

    int offsetHours = gt.timeZoneOffset / 60;
    int offsetMinutes = gt.timeZoneOffset % 60;    
  
    // If timezone is negative, both hours and minutes will be negative
    // but for the purpose of printing a string, only the hour needs to
    // produce a minus sign.
    if (offsetMinutes < 0) offsetMinutes = -offsetMinutes;
  
    boost::format dateFormat("%s %s %d %02d:%02d:%02d GMT%+03d%02d %d");
    dateFormat % dayweekname[gt.weekday] % monthname[gt.month]
               % gt.monthday % gt.hour % gt.minute % gt.second
               % offsetHours % offsetMinutes % (gt.year + 1900);
  
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

	boost::intrusive_ptr<Date> date;

    // Reject all date specifications containing Infinities and NaNs.
    // The commercial player does different things according to which
    // args are NaNs or Infinities:
    // for now, we just use rogue_date_args' algorithm
    double foo;
    if (( foo = rogue_date_args(fn, 7)) != 0.0) {
        date = new Date(foo);
        return as_value(date.get());
    }

    if (fn.nargs < 1 || fn.arg(0).is_undefined() || !(fn.isInstantiation()) ) {
        // Time now
        date = new Date;
    }
    else if (fn.nargs == 1) {
        // Set the value in milliseconds since 1970 UTC
        date = new Date(fn.arg(0).to_number());
    }
    else {
        // Create a time from the supplied (at least 2) arguments.
        GnashTime gt;
    
        gt.millisecond = 0;            
        gt.second = 0;
        gt.minute = 0;
        gt.hour = 0;
        gt.monthday = 1;
        gt.month = fn.arg(1).to_int();
    
        int year = fn.arg(0).to_int();
        
        // GnashTime.year is the value since 1900 (like struct tm)
        // negative value is a year before 1900. A year between 0
        // and 99 is the year since 1900 (which is the same arithmetic).
        if (year < 100) gt.year = year;

        // A value of 100 or more is a full year and must be
        // converted to years since 1900
        else gt.year = year - 1900;

        switch (fn.nargs) {
            default:
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date constructor called with more than 7 "
                        "arguments"));
                )
            case 7:
                // fractions of milliseconds are ignored
                gt.millisecond = fn.arg(6).to_int();
            case 6:
                gt.second = fn.arg(5).to_int();
            case 5:
                gt.minute = fn.arg(4).to_int();
            case 4:
                gt.hour = fn.arg(3).to_int();
            case 3:
                gt.monthday = fn.arg(2).to_int();
            case 2:
                break;
                // Done already
        }

        // The arguments are in local time: subtract the local time offset
        // at the desired time to get UTC. This may not be completely correct
        // due to shortcomings in the timezoneoffset calculation, but should
        // be internally consistent.
        double localTime = makeTimeValue(gt);
        date = new Date(localTime - clocktime::getTimeZoneOffset(localTime) * 60000);
    }
    
    return as_value(date.get());
}

//
//    =========    Functions to get dates in various ways    ========
//

// Date.getTime() is implemented by Date.valueOf()

/// Return true if the date is invalid.
inline
bool invalidDate(double timeValue)
{
    return (isNaN(timeValue) || isInf(timeValue));
}

/// Returns an element of the Date object as an as_value
//
/// An invalid date value is returned as NaN (this is probably not correct,
/// as the pp returns something weird in this case).
//
/// @param dateFunc     The date function (either localTime or universalTime)
///                     to use to break the time value into elements.
/// @param element      A pointer-to-data-member of the GnashTime struct, 
///                     specifying which element to return.
/// @param timeValue    The time value to break into elements.
/// @param adjustment   Adjust the result by this amount (used for full year).
template<typename T>
inline as_value timeElement(T dateFunc, boost::int32_t GnashTime::* element,
        double timeValue, int adjustment = 0)
{
    if (invalidDate(timeValue)) return as_value();
    GnashTime gt;
    dateFunc(timeValue, gt);
    return as_value(gt.*element + adjustment);
}


/// Date.getYear()
//
/// Returns a Date's Gregorian year minus 1900 according to local time.
static as_value
date_getYear(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::year, date->getTimeValue());
}

/// \brief Date.getFullYear
/// returns a Date's Gregorian year according to local time.
static as_value
date_getFullYear(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(
            localTime, &GnashTime::year, date->getTimeValue(), 1900);
}

/// \brief Date.getMonth
/// returns a Date's month in the range 0 to 11.
static as_value
date_getMonth(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::month, date->getTimeValue());
}

/// \brief Date.getDate
/// returns a Date's day-of-month, from 1 to 31 according to local time.
static as_value
date_getDate(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::monthday, date->getTimeValue());
}

/// \brief Date.getDay
/// returns the day of the week for a Date according to local time,
/// where 0 is Sunday and 6 is Saturday.
static as_value
date_getDay(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::weekday, date->getTimeValue());
}


/// \brief Date.getHours
/// Returns the hour number for a Date, from 0 to 23, according to local time.
static as_value
date_getHours(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::hour, date->getTimeValue());
}

/// \brief Date.getMinutes
/// returns a Date's minutes, from 0-59, according to localtime.
/// (Yes, some places do have a fractions of an hour's timezone offset
/// or daylight saving time!)
static as_value
date_getMinutes(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::minute, date->getTimeValue());
}

/// \brief Date.getSeconds
/// returns a Date's seconds, from 0-59.
/// Localtime should be irrelevant.
static as_value
date_getSeconds(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(localTime, &GnashTime::second, date->getTimeValue());
}

/// \brief Date.getMilliseconds
/// returns a Date's millisecond component as an integer from 0 to 999.
/// Localtime is irrelevant!
//
// Also implements Date.getUTCMilliseconds
static as_value
date_getMilliseconds(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(
            localTime, &GnashTime::millisecond, date->getTimeValue());
}


// The same functions for universal time.
//
static as_value
date_getUTCFullYear(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(universalTime, &GnashTime::year,
           date->getTimeValue(), 1900);
}

static as_value
date_getUTCYear(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(universalTime, &GnashTime::year, date->getTimeValue());
}

static as_value
date_getUTCMonth(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(universalTime, &GnashTime::month, date->getTimeValue());
}

static as_value
date_getutcdate(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(
            universalTime, &GnashTime::monthday, date->getTimeValue());
}

    
static as_value
date_getUTCDay(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(
                universalTime, &GnashTime::weekday, date->getTimeValue());
}

static as_value
date_getUTCHours(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(
                universalTime, &GnashTime::hour, date->getTimeValue());
}

static as_value
date_getUTCMinutes(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return timeElement(universalTime, &GnashTime::minute, date->getTimeValue());
}


// Return the difference between UTC and localtime in minutes.
inline int
localTimeZoneOffset(const double& time)
{
    // This simply has to return the difference in minutes
    // between UTC (Greenwich Mean Time, GMT) and the localtime.
    // Obviously, this includes Daylight Saving Time if it applies.
    return clocktime::getTimeZoneOffset(time);
}


/// \brief Date.getTimezoneOffset
/// returns the difference between localtime and UTC that was in effect at the
/// time specified by a Date object, according to local timezone and DST.
/// For example, if you are in GMT+0100, the offset is -60
static as_value
date_getTimezoneOffset(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return as_value(-localTimeZoneOffset(date->getTimeValue()));
}


//
//    =========    Functions to set dates in various ways    ========
//

/// \brief Date.setTime
/// sets a Date in milliseconds after January 1, 1970 00:00 UTC.
/// The return value is the same as the parameter.
static as_value
date_setTime(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.setTime needs one argument"));
        )
    }
    else {
        // returns a double
        date->setTimeValue(fn.arg(0).to_number());
    }

    if (fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.setTime was called with more than one "
                    "argument"));
        )
    }

    return as_value(date->getTimeValue());
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
//
// We need two sets of the same functions: those that take localtime values
// and those that take UTC (GMT) values.
// Since there are a lot of them and they are hairy, we write one set that,
// if an additional extra parameter is passed, switch to working in UTC
// instead. Apart from the bottom-level conversions they are identical.

static void
gnashTimeToDate(GnashTime& gt, Date& date, bool utc)
{
    // Needs timezone.
    if (utc) date.setTimeValue(makeTimeValue(gt));

    else {
        double localTime = makeTimeValue(gt);
        date.setTimeValue(localTime - 
                clocktime::getTimeZoneOffset(localTime) * 60000);
    }
}

static void
dateToGnashTime(Date& date, GnashTime& gt, bool utc)
{
    // Needs timezone.
    if (utc) universalTime(date.getTimeValue(), gt);
    else localTime(date.getTimeValue(), gt);
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

template<bool utc>
as_value
date_setfullyear(const fn_call& fn)
{
  boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.setFullYear needs one argument"));
      )
      date->setTimeValue(NaN);
  } else if (rogue_date_args(fn, 3) != 0.0) {
      date->setTimeValue(NaN);
  } else {
      GnashTime gt;

      dateToGnashTime(*date, gt, utc);
      gt.year = fn.arg(0).to_int() - 1900;
      if (fn.nargs >= 2)
        gt.month = fn.arg(1).to_int();
      if (fn.nargs >= 3)
        gt.monthday = fn.arg(2).to_int();
      if (fn.nargs > 3) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.set%sFullYear was called with "
                        "more than three arguments"), utc ? "UTC" : "");
        )
      }
      gnashTimeToDate(gt, *date, utc);
  }
  return as_value(date->getTimeValue());
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
static as_value
date_setYear(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    // assert(fn.nargs == 1);
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.setYear needs one argument"));
        )
        date->setTimeValue(NaN);
    }
    else if (rogue_date_args(fn, 3) != 0.0) {
        date->setTimeValue(NaN);
    }
    else {
        GnashTime gt;

        dateToGnashTime(*date, gt, false);

        // TODO: Should truncation be done before or after subtracting 1900?
        truncateDouble(gt.year, fn.arg(0).to_number());
        gt.year -= 1900;

        if (fn.nargs >= 2) gt.month = fn.arg(1).to_int();
        if (fn.nargs >= 3) gt.monthday = fn.arg(2).to_int();
        if (fn.nargs > 3) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.setYear was called with more than three "
                        "arguments"));
            )
        }
        gnashTimeToDate(gt, *date, false); // utc=false: use localtime
    }
    return as_value(date->getTimeValue());
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
// the result is NaN.
// We do not do the same because it's a bugger to code.
template<bool utc>
as_value
date_setmonth(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    // assert(fn.nargs >= 1 && fn.nargs <= 2);
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.set%sMonth needs one argument"),
                utc ? "UTC" : "");
        )
        date->setTimeValue(NaN);
    }
    else if (rogue_date_args(fn, 2) != 0.0) {
        date->setTimeValue(NaN);
    }
    else {

        GnashTime gt;

        dateToGnashTime(*date, gt, utc);

        // It seems odd, but FlashPlayer takes all bad month values to mean
        // January
        double monthvalue =  fn.arg(0).to_number();
        if (isNaN(monthvalue) || isInf(monthvalue)) monthvalue = 0.0;
        truncateDouble(gt.month, monthvalue);

        // If the day-of-month value is invalid instead, the result is NaN.
        if (fn.nargs >= 2) {
            double mdayvalue = fn.arg(1).to_number();
            if (isNaN(mdayvalue) || isInf(mdayvalue)) {
                date->setTimeValue(NaN);
                return as_value(date->getTimeValue());
            }
            else {
                truncateDouble(gt.monthday, mdayvalue);
            }
        }
        if (fn.nargs > 2) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.set%sMonth was called with more than three "
                        "arguments"), utc ? "UTC" : "");
            )
        }
        gnashTimeToDate(gt, *date, utc);
    }
    return as_value(date->getTimeValue());
}

/// \brief Date.setDate(day)
/// Set the day-of-month (1-31) for a Date object.
/// If the day-of-month is beyond the end of the current month, it wraps into
/// the first days of the following  month.  This also happens if you set the
/// day > 31. Example: setting the 35th in January results in Feb 4th.
template<bool utc>
as_value
date_setDate(const fn_call& fn)
{
  boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

  if (fn.nargs < 1) {
      IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Date.set%sDate needs one argument"), utc ? "UTC" : "");
      )
      date->setTimeValue(NaN);  // Is what FlashPlayer sets
  } else if (rogue_date_args(fn, 1) != 0.0) {
      date->setTimeValue(NaN);
  } else {
    GnashTime gt;

    dateToGnashTime(*date, gt, utc);
    gt.monthday = fn.arg(0).to_int();
    gnashTimeToDate(gt, *date, utc);
  }
  if (fn.nargs > 1) {
      IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Date.set%sDate was called with more than one argument"),
                utc ? "UTC" : "");
      )
  }
  return as_value(date->getTimeValue());
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
template<bool utc>
as_value
date_setHours(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    // assert(fn.nargs >= 1 && fn.nargs <= 4);
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.set%sHours needs one argument"),
            utc ? "UTC" : "");
        )
        date->setTimeValue(NaN);  // Is what FlashPlayer sets
    }
    else if (rogue_date_args(fn, 4) != 0.0) {
        date->setTimeValue(NaN);
    }
    else {
      
        GnashTime gt;

        dateToGnashTime(*date, gt, utc);
        gt.hour = fn.arg(0).to_int();
        if (fn.nargs >= 2) gt.minute = fn.arg(1).to_int();
        if (fn.nargs >= 3) gt.second = fn.arg(2).to_int();
        if (fn.nargs >= 4) gt.millisecond = fn.arg(3).to_int();
        if (fn.nargs > 4) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.set%sHours was called with more than "
                              "four arguments"), utc ? "UTC" : "");
            )
        }
        
        gnashTimeToDate(gt, *date, utc);
    }
    return as_value(date->getTimeValue());
}

/// \brief Date.setMinutes(minutes[,secs[,millisecs]])
/// change the time-of-day in a Date object. If optional fields are omitted,
/// their values in the Date object are left the same as they were.
///
/// If min/sec>59, these are accepted and wrap into the following minute, hour
/// or calendar day.
/// Similarly, negative values carry you back into the previous minute/hour/day.
template<bool utc>
as_value
date_setMinutes(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    //assert(fn.nargs >= 1 && fn.nargs <= 3);
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.set%sMinutes needs one argument"),
                utc ? "UTC" : "");
        )
        date->setTimeValue(NaN);  // FlashPlayer instead leaves the date set to
        // a random value such as 9th December 2077 BC
    }
    else if (rogue_date_args(fn, 3) != 0.0) {
        date->setTimeValue(NaN);
    }
    else {
        GnashTime gt;

        dateToGnashTime(*date, gt, utc);
        gt.minute = fn.arg(0).to_int();
        if (fn.nargs >= 2) gt.second = fn.arg(1).to_int();
        if (fn.nargs >= 3) gt.millisecond = fn.arg(2).to_int();
        if (fn.nargs > 3) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.set%sMinutes was called with more than "
                    "three arguments"), utc ? "UTC" : "");
            )
        }
        gnashTimeToDate(gt, *date, utc);
    }
    return as_value(date->getTimeValue());
}

/// \brief Date.setSeconds(secs[,millisecs])
/// set the "seconds" component in a date object.
///
/// Values <0, >59 for secs or >999 for millisecs take the date back to the
/// previous minute (or hour or calendar day) or on to the following ones.
template<bool utc>
as_value
date_setSeconds(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    // assert(fn.nargs >= 1 && fn.nargs <= 2);
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.set%sSeconds needs one argument"),
                utc ? "UTC" : "");
        )
        date->setTimeValue(NaN);  // Same as commercial player
    }
    else if (rogue_date_args(fn, 2) != 0.0) {
        date->setTimeValue(NaN);
    }
    else {
        // We *could* set seconds [and milliseconds] without breaking the
        // structure out and reasembling it. We do it the same way as the
        // rest for simplicity and in case anyone's date routines ever
        // take account of leap seconds.
        GnashTime gt;

        dateToGnashTime(*date, gt, utc);
        gt.second = fn.arg(0).to_int();
        if (fn.nargs >= 2) gt.millisecond = fn.arg(1).to_int();
        if (fn.nargs > 2) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.set%sMinutes was called with more than "
                      "three arguments"), utc ? "UTC" : "");
            )
        }

        gnashTimeToDate(gt, *date, utc);
    }
    return as_value(date->getTimeValue());
}

template<bool utc>
as_value
date_setMilliseconds(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.set%sMilliseconds needs one argument"),
                utc ? "UTC" : "");
        )
        date->setTimeValue(NaN);
    }
    else if (rogue_date_args(fn, 1) != 0.0) {
        date->setTimeValue(NaN);
    }
    else {
    
        GnashTime gt;

        dateToGnashTime(*date, gt, utc);
        truncateDouble(gt.millisecond, fn.arg(0).to_number());

        if (fn.nargs > 1) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.setMilliseconds was called with more "
                             "than one argument"), utc ? "UTC" : "");
            )
        }

        // This is both setMilliseconds and setUTCMilliseconds.
        // Use utc to avoid needless worrying about timezones.
        gnashTimeToDate(gt, *date, utc);

    }
    return as_value(date->getTimeValue());
}


/// \brief Date.toString()
/// convert a Date to a printable string.
/// The format is "Thu Jan 1 00:00:00 GMT+0000 1970" and it is displayed in
/// local time.
static as_value
date_tostring(const fn_call& fn) 
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
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
// Actually, given a first parameter of Infinity,-Infinity or NaN,
// it returns -6.77681005679712e+19 but that's just crazy.
//
// We test for < 2 parameters and return undefined, but given any other
// non-numeric arguments we give NaN.
static as_value
date_UTC(const fn_call& fn) {

    GnashTime gt; // Date structure for values down to milliseconds

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Date.UTC needs one argument"));
        )
        return as_value();  // undefined
    }

    double result;

    // Check for presence of NaNs and Infinities in the arguments 
    // and return the appropriate value if so.
    if ( (result = rogue_date_args(fn, 7)) != 0.0) {
        return as_value(NaN);
    }

    // Preset default values
    // Year and month are always given explicitly
    gt.monthday = 1;
    gt.hour = 0;
    gt.minute = 0;
    gt.second = 0;
    gt.millisecond = 0;

    switch (fn.nargs) {
        default:  // More than 7
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Date.UTC was called with more than 7 arguments"));
            )
        case 7:
            // millisecs is double, but fractions of millisecs are ignored.
            gt.millisecond = fn.arg(6).to_int();
        case 6:
            gt.second = fn.arg(5).to_int();
        case 5:
            gt.minute = fn.arg(4).to_int();
        case 4:
            gt.hour = fn.arg(3).to_int();
        case 3:
            gt.monthday = fn.arg(2).to_int();
        case 2:   // these last two are always performed
            gt.month = fn.arg(1).to_int();
            {
                boost::int32_t year = 0;
                truncateDouble(year, fn.arg(0).to_number());
                if (year < 100) gt.year = year;
                else gt.year = year - 1900;
            }
    }

    result = makeTimeValue(gt);
    return as_value(result);
}

// Auxillary function checks for Infinities and NaN in a function's args and
// returns 0.0 if there are none,
// plus (or minus) infinity if positive (or negative) infinites are present,
// NaN is there are NaNs present, or a mixture of positive and negative infs.
static double
rogue_date_args(const fn_call& fn, unsigned maxargs)
{
  // Two flags: Did we find any +Infinity (or -Infinity) values in the
  // argument list? If so, "infinity" must be set to the kind that we
  // found.
    bool plusinf = false;
    bool minusinf = false;
    double infinity = 0.0;  // The kind of infinity we found.
                            // 0.0 == none yet.

    // Only check the present parameters, up to the stated maximum number
    if (fn.nargs < maxargs) maxargs = fn.nargs;

    for (unsigned int i = 0; i < maxargs; i++) {
        double arg = fn.arg(i).to_number();

        if (isNaN(arg)) return(NaN);

        if (isInf(arg)) {
            if (arg > 0) {  // Plus infinity
                plusinf = true;
            }
            else {  // Minus infinity
                minusinf = true;
            }
            // Remember the kind of infinity we found
            infinity = arg;
        }
    }
    // If both kinds of infinity were present in the args,
    // the result is NaN.
    if (plusinf && minusinf) return(NaN);

    // If only one kind of infinity was in the args, return that.
    if (plusinf || minusinf) return(infinity);
  
    // Otherwise indicate that the function arguments contained
    // no rogue values
    return(0.0);
}

/// \brief Date.valueOf() returns the number of milliseconds since midnight
/// January 1, 1970 00:00 UTC, for a Date. The return value can be a fractional
/// number of milliseconds.
static as_value
date_valueof(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return as_value(date->getTimeValue());
}


static as_value date_getTime(const fn_call& fn)
{
    boost::intrusive_ptr<Date> date = ensureType<Date>(fn.this_ptr);
    return as_value(date->getTimeValue());
}

// extern (used by Global.cpp)
void date_class_init(as_object& global)
{
    // This is going to be the global Date "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL ) {
        cl = new builtin_function(&date_new, getDateInterface());
        
        // replicate static interface to class (Date.UTC)
        attachDateStaticInterface(*cl);
    }

    // Register _global.Date
    global.init_member("Date", cl.get());

}

///
///
/// Date conversion functions
///
///

// Converts a time struct into a swf timestamp. Similar to
// mktime, but not limited by the size of time_t. The mathematical
// algorithm looks nicer, but does not cope with large dates.
// Bumping up the int size or using doubles more might help - I 
// haven't really looked at it.
// The first algorithm appears to mimic flash behaviour for
// all dates, though it's a bit ugly.
static double
makeTimeValue(GnashTime& t)
{

    // First, adjust years to deal with strange month
    // values.
    
    // Add or substract more than 12 months from the year
    // and adjust months to a valid value.
    t.year += t.month / 12;
    t.month %= 12;

    // Any negative remainder rolls back to the previous year.
    if (t.month < 0) {
        t.year--;
        t.month += 12;
    }

    // Now work out the years from 1970 in days.
    boost::int32_t day = t.monthday;    

    // This works but is a bit clunky.
    if (t.year < 70) {
        day = countLeapYears(t.year - 2) + ((t.year - 70) * 365);
        // Adds an extra leap year for the year 0.
        if (t.year <= 0) day++;
    }
    else {
        day = countLeapYears(t.year + 1) + ((t.year - 70) * 365);
    }
    
    // Add days for each month. Month must be 0 - 11;
    for (int i = 0; i < t.month; i++)
    {
        assert (t.month < 12);
        day += daysInMonth[isLeapYear(t.year)][i];
    }
    
    // Add the days of the month
    day += t.monthday - 1;

    /// Work out the timestamp
    double ret = static_cast<double>(day) * 86400000.0;
    ret += t.hour * 3600000.0;
    ret += t.minute * 60000.0;
    ret += t.second * 1000.0;
    ret += t.millisecond;
    return ret;
}


// The brute force way of converting days into years since the epoch.
// This also reduces the number of days accurately. Its disadvantage is,
// of course, that it iterates; its advantage that it's always correct.
boost::int32_t
getYearBruteForce(boost::int32_t& days)
{
    boost::int32_t year = 1970;

    // Handle 400-year blocks - which always have the same
    // number of days (14097) - to cut down on iterations.
    year += (days / 146097) * 400;
    days %= 146097;

    if (days >= 0)
    {
        for (;;)
	    {
            bool isleap = isLeapYear(year - 1900);
            if (days < (isleap ? 366 : 365)) break;
	        year++;
	        days -= isleap ? 366 : 365;
	    }
    }
    else
    {
        do
	    {
	        --year;
	        bool isleap = isLeapYear(year - 1900);
	        days += isleap ? 366 : 365;
	    } while (days < 0);
    }
    return year - 1900;
}


void fillGnashTime(const double& t, GnashTime& gt)
{

    // Calculate local time by adding offset from UTC in
    // milliseconds to time value. Offset is in minutes.
    double time = t + gt.timeZoneOffset * 60000;

    gt.millisecond = std::fmod(time, 1000.0);
    time /= 1000.0;
    
    // Get the sub-day part of the time, if any and reduce time
    // to number of complete days.
    // This is a safe cast.
    boost::int32_t remainder = 
        static_cast<boost::int32_t>(std::fmod(time, 86400.0));

    // This could overflow.
    boost::int32_t days;
    truncateDouble(days, time / 86400.0); // complete days
   
    gt.second = remainder % 60;
    remainder /= 60;

    gt.minute = remainder % 60;
    remainder /= 60;

    gt.hour = remainder % 24;
 
    if (time < 0)
    {
        if (gt.millisecond < 0) { gt.millisecond += 1000; --gt.second; }
        if (gt.second < 0) { gt.second += 60; --gt.minute; }
        if (gt.minute < 0) { gt.minute += 60; --gt.hour; }
        if (gt.hour < 0) { gt.hour += 24; --days; }
    }

    if (days >= -4) gt.weekday = (days + 4) % 7;
    else gt.weekday = 6 - (((-5) - days ) % 7);

    // default, brute force:
    gt.year = getYearBruteForce(days);
            
    gt.month = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (days - daysInMonth[isLeapYear(gt.year)][i] < 0)
        {
            gt.month = i;
            break;
        }
        days -= daysInMonth[isLeapYear(gt.year)][i];
    }
    
    gt.monthday = days + 1;

}

// external
as_object*
init_date_instance(double val)
{
    return new Date(val);
}

} // end of gnash namespace
