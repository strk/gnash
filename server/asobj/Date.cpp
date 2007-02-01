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

//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Date.h"
#include "fn_call.h"
#include "GnashException.h"

#include <ctime>

#if defined(_WIN32) || defined(WIN32)
# define snprintf _snprintf
#else
# include <sys/time.h>
#endif

#include <sys/timeb.h>

namespace gnash {

Date::Date() {
}

Date::~Date() {
}

double
Date::getTime()
{
	tm result = convertTM();
	time_t count = mktime(&result);
	return double(count) * 1000.0;
}

void
Date::getTimezoneOffset()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCDate()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCDay()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCFullYear()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCHours()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCMilliseconds()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCMinutes()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCMonth()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTCSeconds()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getYear()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setDate()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setFullYear()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setHours()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setMilliseconds()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setMinutes()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setMonth()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setSeconds()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setTime()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCDate()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCFullYear()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCHours()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCMilliseconds()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCMinutes()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCMonth()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setUTCSeconds()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::setYear()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::toString()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::UTC()
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

tm
Date::convertUTC()
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
Date::convertTM()
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
Date::setFromTM(const tm newtime)
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
Date::Normalize()
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

	date_obj->init_member("getDate", &date_getdate);
	date_obj->init_member("getDay", &date_getday);
	date_obj->init_member("getFullYear", &date_getfullyear);
	date_obj->init_member("getHours", &date_gethours);
	date_obj->init_member("getMilliseconds", &date_getmilliseconds);
	date_obj->init_member("getMinutes", &date_getminutes);
	date_obj->init_member("getMonth", &date_getmonth);
	date_obj->init_member("getSeconds", &date_getseconds);
	date_obj->init_member("getTime", &date_gettime);
	date_obj->init_member("getTimezoneOffset", &date_gettimezoneoffset);
	date_obj->init_member("getUTCDate", &date_getutcdate);
	date_obj->init_member("getUTCDay", &date_getutcday);
	date_obj->init_member("getUTCFullYear", &date_getutcfullyear);
	date_obj->init_member("getUTCHours", &date_getutchours);
	date_obj->init_member("getUTCMilliseconds", &date_getutcmilliseconds);
	date_obj->init_member("getUTCMinutes", &date_getutcminutes);
	date_obj->init_member("getUTCMonth", &date_getutcmonth);
	date_obj->init_member("getUTCSeconds", &date_getutcseconds);
	date_obj->init_member("getYear", &date_getyear);
	date_obj->init_member("setDate", &date_setdate);
	date_obj->init_member("setFullYear", &date_setfullyear);
	date_obj->init_member("setHours", &date_sethours);
	date_obj->init_member("setMilliseconds", &date_setmilliseconds);
	date_obj->init_member("setMinutes", &date_setminutes);
	date_obj->init_member("setMonth", &date_setmonth);
	date_obj->init_member("setSeconds", &date_setseconds);
	date_obj->init_member("setTime", &date_settime);
	date_obj->init_member("setUTCDate", &date_setutcdate);
	date_obj->init_member("setUTCFullYear", &date_setutcfullyear);
	date_obj->init_member("setUTCHours", &date_setutchours);
	date_obj->init_member("setUTCMilliseconds", &date_setutcmilliseconds);
	date_obj->init_member("setUTCMinutes", &date_setutcminutes);
	date_obj->init_member("setUTCMonth", &date_setutcmonth);
	date_obj->init_member("setUTCSeconds", &date_setutcseconds);
	date_obj->init_member("setYear", &date_setyear);
	date_obj->init_member("toString", &date_tostring);

	struct tm *ti;
	if (fn.nargs == 0) {
#ifndef HAVE_GETTIMEOFDAY
		struct timeb tb;
		
		ftime (&tb);
		ti = localtime(&tb.time); 
		log_error("date_new constructor doesn't set timezone or milliseconds on your system - using defaults\n");
		date_obj->obj.millisecond = 0;
		date_obj->obj.minutesEast = 0;
#else		
		struct timeval tEnd;
		struct timezone tZone;
		gettimeofday(&tEnd,&tZone);
		date_obj->obj.millisecond = tEnd.tv_usec;
		date_obj->obj.minutesEast = -tZone.tz_minuteswest;
		time_t t = time(&t);
		ti = localtime(&t);
#endif
		date_obj->obj.second = ti->tm_sec;
		date_obj->obj.minute = ti->tm_min;
		date_obj->obj.hour = ti->tm_hour;
		date_obj->obj.date = ti->tm_mday;
		date_obj->obj.month = ti->tm_mon;
		date_obj->obj.year = ti->tm_year;
		date_obj->obj.dayWeek = ti->tm_wday;
		date_obj->obj.dayYear = ti->tm_yday;
		date_obj->obj.isDST = ti->tm_isdst;
	}
	else
		log_error("date_new constructor with %d arguments unimplemented!\n",fn.nargs);

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

void date_getdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.date);
}
void date_getday(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.dayWeek);
}
void date_getfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.year + 1900);
}
void date_gethours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.hour);
}
void date_getmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.millisecond);
}
void date_getminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.minute);
}
void date_getmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.month);
}
void date_getseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.second);
}
void date_gettime(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_double(date->obj.getTime());
}
void date_gettimezoneoffset(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.minutesEast);
}
void date_getutcdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->obj.convertUTC();

	fn.result->set_int(int(result.tm_mday));
}
void date_getutcday(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->obj.convertUTC();

	fn.result->set_int(int(result.tm_wday));
}
void date_getutcfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->obj.convertUTC();

	fn.result->set_int(int(result.tm_year)+1900);
}
void date_getutchours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->obj.convertUTC();

	fn.result->set_int(int(result.tm_hour));
}
void date_getutcmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	// Milliseconds (value between 0 and 999) won't be affected by timezone
	fn.result->set_int(int(date->obj.millisecond));
}
void date_getutcminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->obj.convertUTC();

	fn.result->set_int(int(result.tm_min));
}
void date_getutcmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	tm result = date->obj.convertUTC();

	fn.result->set_int(int(result.tm_mon));
}
void date_getutcseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	// Seconds (value between 0 and 59) won't be affected by timezone
	fn.result->set_int(int(date->obj.second));
}
void date_getyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);
	fn.result->set_int(date->obj.year);
}

// TODO: for all these "set" functions, what do we do if sent illegal values?
// Clamp them to a proper range? Ignore and keep previous value? Throw an error?
// For now I'm "normalizing" them e.g. Jan-33 25:60:60 -> Feb-3 2:01:00

// TODO: Also, confirm this is the appropriate behavior for the setUTC()
// functions. Right now, we convert the time to UTC, set the variable,
// then convert back to local time. We should confirm the official behavior!

void date_setdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
		date->obj.date = (long int)(fn.arg(0).to_number());
		date->obj.Normalize();
	}

	if (fn.nargs > 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " has more than one argument");
	    )
	}

	fn.result->set_double(date->obj.getTime());
}

/// \brief Set year [, month [, date]]

void date_setfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    date->obj.year = (long int)(fn.arg(0).to_number() - 1900);
	    if (fn.nargs >= 2)
		    date->obj.month = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		    date->obj.date = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than three arguments");
		)
	    }
	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_sethours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 4);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    date->obj.hour = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		    date->obj.minute = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		    date->obj.second = (long int)(fn.arg(2).to_number());
	    if (fn.nargs >= 4)
		    date->obj.millisecond = (long int)(fn.arg(3).to_number());
	    if (fn.nargs > 4) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than four arguments");
		)
	    }
	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    date->obj.millisecond = (long int)(fn.arg(0).to_number());
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than one argument");
		)
	    }
	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	//assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->obj.minute = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2) date->obj.second = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3) date->obj.millisecond = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than three arguments");
		)
	    }
	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    date->obj.month = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		date->obj.date = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than two arguments");
		)
	    }

	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->obj.second = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		date->obj.millisecond = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than two arguments");
		)
	    }

	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_settime(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    log_msg("%s:unimplemented \n", __FUNCTION__);
	}

	fn.result->set_double(date->obj.getTime());
}
void date_setutcdate(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    tm utctime = date->obj.convertUTC();
	    // Set mday to our new UTC date (yday and wday don't need to be set)
	    utctime.tm_mday = int(fn.arg(0).to_number());

	    // Convert back from UTC to local time
	    utctime.tm_min += date->obj.minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->obj.setFromTM(*(localtime(&newtime)));

	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than one argument");
		)
	    }
	}
	
	fn.result->set_double(date->obj.getTime());
}
void date_setutcfullyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    tm utctime = date->obj.convertUTC();
	    // Set year to our new UTC date
	    utctime.tm_year = int(fn.arg(0).to_number()-1900);
	    if (fn.nargs >= 2)
		utctime.tm_mon = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		utctime.tm_mday = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than three arguments");
		)
	    }

	    // Convert back from UTC to local time
	    utctime.tm_min += date->obj.minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->obj.setFromTM(*(localtime(&newtime)));
	}
	
	fn.result->set_double(date->obj.getTime());
}
void date_setutchours(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 4);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {

	    if (fn.nargs >= 4)
	    {
		date->obj.millisecond = (long int)(fn.arg(3).to_number());
		date->obj.Normalize();
	    }

	    tm utctime = date->obj.convertUTC();
	    // Set year to our new UTC date
	    utctime.tm_hour = int(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		utctime.tm_min = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3)
		utctime.tm_sec = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 4) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than four arguments");
		)
	    }

	    // Convert back from UTC to local time
	    utctime.tm_min += date->obj.minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->obj.setFromTM(*(localtime(&newtime)));
	}
	
	fn.result->set_double(date->obj.getTime());
}
void date_setutcmilliseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    date->obj.millisecond = (long int)(fn.arg(0).to_number());
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than one argument");
		)
	    }

	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setutcminutes(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 3);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->obj.minute = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2) date->obj.second = (long int)(fn.arg(1).to_number());
	    if (fn.nargs >= 3) date->obj.millisecond = (long int)(fn.arg(2).to_number());
	    if (fn.nargs > 3) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than three arguments");
		)
	    }

	    // Setting milliseconds to less than 0 or more than 999 affects seconds

	    // TODO: both of these lines are wrong for negative values
	    date->obj.second += date->obj.millisecond / 1000;
	    date->obj.millisecond = date->obj.millisecond % 1000;

	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setutcmonth(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    tm utctime = date->obj.convertUTC();
	    // Set year to our new UTC date
	    utctime.tm_mon = int(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		utctime.tm_mday = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than two arguments");
		)
	    }

	    // Convert back from UTC to local time
	    utctime.tm_min += date->obj.minutesEast;

	    // Normalize the time, then set as this object's new time
	    time_t newtime = mktime(&utctime);
	    date->obj.setFromTM(*(localtime(&newtime)));
	}
	fn.result->set_double(date->obj.getTime());
}
void date_setutcseconds(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs >= 1 && fn.nargs <= 2);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    // Seconds (value between 0 and 59) won't be affected by timezone
	    date->obj.second = (long int)(fn.arg(0).to_number());
	    if (fn.nargs >= 2)
		date->obj.millisecond = (long int)(fn.arg(1).to_number());
	    if (fn.nargs > 2) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than two arguments");
		)
	    }

	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}

void date_setyear(const fn_call& fn) {
	date_as_object* date = ensure_date_object(fn.this_ptr);

	// assert(fn.nargs == 1);
	if (fn.nargs < 1) {
	    IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(__FUNCTION__ " needs one argument");
	    )
	} else {
	    date->obj.year = (long int)(fn.arg(0).to_number());
	    if (fn.nargs > 1) {
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(__FUNCTION__ " has more than two arguments");
		)
	    }

	    date->obj.Normalize();
	}
	fn.result->set_double(date->obj.getTime());
}

void date_tostring(const fn_call& fn) {
	// TODO: I have no idea what the real flash player does, but at least this
	// gives something functional for now. Tried to mimic the "date" command
	char buffer[128];
	char* monthname[12] =
		{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char* dayweekname[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

	date_as_object* date = ensure_date_object(fn.this_ptr);

	snprintf((char *)&buffer,128,"%s %s %2ld %.2ld:%.2ld:%.2ld %ld",
		dayweekname[date->obj.dayWeek],monthname[date->obj.month],
		date->obj.date,date->obj.hour,date->obj.minute,date->obj.second,
		1900+date->obj.year);

	fn.result->set_string((char *)&buffer);
}

} // end of gnash namespace

