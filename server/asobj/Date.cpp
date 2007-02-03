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

#include <sys/timeb.h>

namespace gnash {

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
static void date_gettime(const fn_call& fn);
static void date_gettimezoneoffset(const fn_call& fn);
static void date_getutcdate(const fn_call& fn);
static void date_getutcday(const fn_call& fn);
static void date_getutcfullyear(const fn_call& fn);
static void date_getutchours(const fn_call& fn);
static void date_getutcmilliseconds(const fn_call& fn);
static void date_getutcminutes(const fn_call& fn);
static void date_getutcmonth(const fn_call& fn);
static void date_getutcseconds(const fn_call& fn);
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
static void date_setutcmilliseconds(const fn_call& fn);
static void date_setutcminutes(const fn_call& fn);
static void date_setutcmonth(const fn_call& fn);
static void date_setutcseconds(const fn_call& fn);
static void date_setyear(const fn_call& fn);
static void date_tostring(const fn_call& fn);
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
	o.init_member("getTime", &date_gettime);
	o.init_member("getTimezoneOffset", &date_gettimezoneoffset);
	o.init_member("getUTCDate", &date_getutcdate);
	o.init_member("getUTCDay", &date_getutcday);
	o.init_member("getUTCFullYear", &date_getutcfullyear);
	o.init_member("getUTCHours", &date_getutchours);
	o.init_member("getUTCMilliseconds", &date_getutcmilliseconds);
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
	o.init_member("setUTCMilliseconds", &date_setutcmilliseconds);
	o.init_member("setUTCMinutes", &date_setutcminutes);
	o.init_member("setUTCMonth", &date_setutcmonth);
	o.init_member("setUTCSeconds", &date_setutcseconds);
	o.init_member("setYear", &date_setyear);
	o.init_member("toString", &date_tostring);
}

static void
attachDateStaticInterface(as_object& o)
{
	// TODO: This should *only* be available when SWF version is > 6
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
	// Use double to avoid overflow
	double getTime();
	void UTC();

	// These store the local time
	bool isDST;
	long year,month,date,hour,minute,second,millisecond;
	int minutesEast;
	int dayWeek,dayYear;

	// This returns a tm struct representing this date in UTC time
	tm convertUTC();
	// This returns a tm struct representing this date in local time
	tm convertTM();
	// This sets the values in the date object to those in the tm struct
	// And ignores any values not stored in the tm struct
	void setFromTM(const tm newtime);
	// This function normalizes the time - for example, if we set the date
	// to Jan-32, 1:61:60, after normalize the time will be Feb-1, 2:02:00
	void Normalize();

	date_as_object()
		:
		as_object(getDateInterface())
	{
	}

private:
};

// Return time as number of milliseconds since 1 Jan 1970 UTC
double
date_as_object::getTime()
{
	tm result = convertTM();
	time_t count = mktime(&result);
	return double(count) * 1000.0;
}

tm
date_as_object::convertUTC()
{
	tm utctime;

	utctime.tm_sec = second;
	utctime.tm_min = minute;
	utctime.tm_hour = hour;
	utctime.tm_mday = date;
	utctime.tm_mon = month;
	utctime.tm_year = year;
	utctime.tm_wday = dayWeek;
	utctime.tm_yday = dayYear;
	utctime.tm_isdst = isDST;

	time_t normalized;

	normalized = mktime(&utctime);

	tm *result = gmtime(&normalized);

	return *result;
}

tm
date_as_object::convertTM()
{
	tm thistime;

	thistime.tm_sec = second;
	thistime.tm_min = minute;
	thistime.tm_hour = hour;
	thistime.tm_mday = date;
	thistime.tm_mon = month;
	thistime.tm_year = year;
	thistime.tm_wday = dayWeek;
	thistime.tm_yday = dayYear;
	thistime.tm_isdst = isDST;

	time_t normalized;

	normalized = mktime(&thistime);

	tm *result = localtime(&normalized);

	return *result;
}

void
date_as_object::setFromTM(const tm newtime)
{
	second = newtime.tm_sec;
	minute = newtime.tm_min;
	hour = newtime.tm_hour;
	date = newtime.tm_mday;
	month = newtime.tm_mon;
	year = newtime.tm_year;
	dayWeek = newtime.tm_wday;
	dayYear = newtime.tm_yday;
	isDST = newtime.tm_isdst;
}


void
date_as_object::Normalize()
{
	second += (millisecond / 1000);
	millisecond = millisecond % 1000;

	tm thistime = convertTM();
	time_t newtime = mktime(&thistime);
	setFromTM(*(localtime(&newtime)));
}

void
date_new(const fn_call& fn)
{
	// TODO: just make date_as_object constructor
	//       register the exported interface, don't
	//       replicate all functions !!

	date_as_object *date_obj = new date_as_object;


	// TODO: move this to date_as_object constructor
	struct tm *ti;
	if (fn.nargs == 0) {
#ifndef HAVE_GETTIMEOFDAY
		struct timeb tb;
		
		ftime (&tb);
		ti = localtime(&tb.time); 
		log_error("date_new constructor doesn't set timezone or milliseconds on your system - using defaults\n");
		date_obj->millisecond = 0;
		date_obj->minutesEast = 0;
#else		
		struct timeval tEnd;
		struct timezone tZone;
		gettimeofday(&tEnd,&tZone);
		date_obj->millisecond = tEnd.tv_usec;
		date_obj->minutesEast = -tZone.tz_minuteswest;
		time_t t = time(&t);
		ti = localtime(&t);
#endif
		date_obj->second = ti->tm_sec;
		date_obj->minute = ti->tm_min;
		date_obj->hour = ti->tm_hour;
		date_obj->date = ti->tm_mday;
		date_obj->month = ti->tm_mon;
		date_obj->year = ti->tm_year;
		date_obj->dayWeek = ti->tm_wday;
		date_obj->dayYear = ti->tm_yday;
		date_obj->isDST = ti->tm_isdst;
	}
	else
		log_error("date_new constructor with %d arguments unimplemented!", fn.nargs);

	fn.result->set_as_object(date_obj);
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

static void date_getdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->date);
}
static void date_getday(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->dayWeek);
}
static void date_getfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->year + 1900);
}
static void date_gethours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->hour);
}
static void date_getmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->millisecond);
}
static void date_getminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->minute);
}
static void date_getmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->month);
}
static void date_getseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->second);
}
static void date_gettime(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_double(date->getTime());
}
static void date_gettimezoneoffset(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->minutesEast);
}
static void date_getutcdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->convertUTC();

	fn.result->set_int(int(result.tm_mday));
}
static void date_getutcday(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->convertUTC();

	fn.result->set_int(int(result.tm_wday));
}
static void date_getutcfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->convertUTC();

	fn.result->set_int(int(result.tm_year)+1900);
}
static void date_getutchours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->convertUTC();

	fn.result->set_int(int(result.tm_hour));
}
static void date_getutcmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	// Milliseconds (value between 0 and 999) won't be affected by timezone
	fn.result->set_int(int(date->millisecond));
}
static void date_getutcminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->convertUTC();

	fn.result->set_int(int(result.tm_min));
}
static void date_getutcmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->convertUTC();

	fn.result->set_int(int(result.tm_mon));
}
static void date_getutcseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	// Seconds (value between 0 and 59) won't be affected by timezone
	fn.result->set_int(int(date->second));
}
static void date_getyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->year);
}

// TODO: for all these "set" functions, what do we do if sent illegal values?
// Clamp them to a proper range? Ignore and keep previous value? Throw an error?
// For now I'm "normalizing" them e.g. Jan-33 25:60:60 -> Feb-3 2:01:00

// TODO: Also, confirm this is the appropriate behavior for the setUTC()
// functions. Right now, we convert the time to UTC, set the variable,
// then convert back to local time. We should confirm the official behavior!

static void date_setdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setDate needs one argument");
	    )
	} else {
		date->date = (long int)(fn.arg(0).to_number());
		date->Normalize();
	}

	if (fn.nargs > 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setDate has more than one argument");
	    )
	}

	fn.result->set_double(date->getTime());
}

/// \brief Set year [, month [, date]]

static void date_setfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setFullYear needs one argument");
	    )
	} else {
	    date->year = (long int)(fn.arg(0).to_number() - 1900);
	    if (fn.nargs >= 2)
		    date->month = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		    date->date = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setFullYear has more than three arguments");
		)
	    }
	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_sethours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 4);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setHours needs one argument");
	    )
	} else {
	    date->hour = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		    date->minute = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		    date->second = (long int)(fn.arg(2).to_number());
	    if (fn.nargs >= 4)
		    date->millisecond = (long int)(fn.arg(3).to_number());
	    if (fn.nargs > 4) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setHours has more than four arguments");
		)
	    }
	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_setmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setMilliseconds needs one argument");
	    )
	} else {
	    date->millisecond = (long int)(fn.arg(0).to_number());
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMilliseconds has more than one argument");
		)
	    }
	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_setminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	//assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setMinutes needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->minute = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2) date->second = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3) date->millisecond = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMinutes has more than three arguments");
		)
	    }
	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_setmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setMonth needs one argument");
	    )
	} else {
	    date->month = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		date->date = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setMonth has more than two arguments");
		)
	    }

	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_setseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setSeconds needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->second = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		date->millisecond = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setSeconds has more than two arguments");
		)
	    }

	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_settime(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setTime needs one argument");
	    )
	} else {
	    double millitime = fn.arg(0).to_number();
	    date->millisecond = (long) std::fmod(millitime, 1000.0);
	    time_t sectime = (time_t) (millitime / 1000.0);
	    tm *tm = gmtime(&sectime);
	    date->second = tm->tm_sec;
	    date->minute = tm->tm_min;
	    date->hour = tm->tm_hour;
	    date->date = tm->tm_mday;
	    date->month = tm->tm_mon;
	    date->year = tm->tm_year; // No of years since 1900, the same.
	    
	    date->Normalize();
	}
	if (fn.nargs > 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setTime has more than one argument");
	    )
	}

	fn.result->set_double(date->getTime());
}
static void date_setutcdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCDate needs one argument");
	    )
	} else {
	    tm utctime = date->convertUTC();
	    // Set mday to our new UTC date (yday and wday don't need to be set)
	    utctime.tm_mday = int(fn.arg(0).to_number());

	    // Convert back from UTC to local time
	    utctime.tm_min += date->minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->setFromTM(*(localtime(&newtime)));

	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCDate has more than one argument");
		)
	    }
	}
	
	fn.result->set_double(date->getTime());
}
static void date_setutcfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCFullYear needs one argument");
	    )
	} else {
	    tm utctime = date->convertUTC();
	    // Set year to our new UTC date
	    utctime.tm_year = int(fn.arg(0).to_number()-1900);
	    if (fn.nargs >= 2)
		utctime.tm_mon = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		utctime.tm_mday = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCFullYear has more than three arguments");
		)
	    }

	    // Convert back from UTC to local time
	    utctime.tm_min += date->minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->setFromTM(*(localtime(&newtime)));
	}
	
	fn.result->set_double(date->getTime());
}
static void date_setutchours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 4);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCHours needs one argument");
	    )
	} else {

	    if (fn.nargs >= 4)
	    {
		date->millisecond = (long int)(fn.arg(3).to_number());
		date->Normalize();
	    }

	    tm utctime = date->convertUTC();
	    // Set year to our new UTC date
	    utctime.tm_hour = int(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		utctime.tm_min = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		utctime.tm_sec = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 4) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCHours has more than four arguments");
		)
	    }

	    // Convert back from UTC to local time
	    utctime.tm_min += date->minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->setFromTM(*(localtime(&newtime)));
	}
	
	fn.result->set_double(date->getTime());
}
static void date_setutcmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCMilliseconds needs one argument");
	    )
	} else {
	    date->millisecond = (long int)(fn.arg(0).to_number());
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCMilliseconds has more than one argument");
		)
	    }

	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_setutcminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCMinutes needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->minute = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2) date->second = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3) date->millisecond = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCMinutes has more than three arguments");
		)
	    }

	    // Setting milliseconds to less than 0 or more than 999 affects seconds

	    // TODO: both of these lines are wrong for negative values
	    date->second += date->millisecond / 1000;
	    date->millisecond = date->millisecond % 1000;

	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}
static void date_setutcmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCMonth needs one argument");
	    )
	} else {
	    tm utctime = date->convertUTC();
	    // Set year to our new UTC date
	    utctime.tm_mon = int(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		utctime.tm_mday = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCMonth has more than two arguments");
		)
	    }

	    // Convert back from UTC to local time
	    utctime.tm_min += date->minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->setFromTM(*(localtime(&newtime)));
	}
	fn.result->set_double(date->getTime());
}
static void date_setutcseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setUTCSeconds needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->second = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		date->millisecond = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setUTCSeconds has more than two arguments");
		)
	    }

	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}

// If year is an integer between 0-99, setYear sets the year at 1900 + year;
// otherwise, the year is the value of the year parameter.
static void date_setyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Date.setYear needs one argument");
	    )
	} else {
	    date->year = (long int)(fn.arg(0).to_number());
	    if (date->year < 100) date->year += 1900;
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror("Date.setYear has more than two arguments");
		)
	    }

	    date->Normalize();
	}
	fn.result->set_double(date->getTime());
}

static void date_tostring(const fn_call& fn) {
	// TODO: I have no idea what the real flash player does, but at least this
	// gives something functional for now. Tried to mimic the "date" command
	char buffer[128];
	char* monthname[12] =
		{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char* dayweekname[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

	date_as_object* date = ensure_date_object(fn.this_ptr);

	snprintf((char *)&buffer,128,"%s %s %2ld %.2ld:%.2ld:%.2ld %ld",
		dayweekname[date->dayWeek],monthname[date->month],
		date->date,date->hour,date->minute,date->second,
		1900+date->year);

	fn.result->set_string((char *)&buffer);
}

// This should be a static method - not quite sure what that means...
static void date_utc(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	UNUSED(date);
	log_msg("Date.UTC unimplemented\n");
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

