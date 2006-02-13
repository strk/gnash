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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Date.h"
#include <time.h>
#include <sys/time.h>

namespace gnash {

Date::Date() {
}

Date::~Date() {
}

void
Date::getTime()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getTimezoneOffset()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Date::getUTC()
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
Date::set()
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
Date::setUTC()
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
void
date_new(const fn_call& fn)
{
    date_as_object *date_obj = new date_as_object;

    date_obj->set_member("getdate", &date_getdate);
    date_obj->set_member("getday", &date_getday);
    date_obj->set_member("getfullyear", &date_getfullyear);
    date_obj->set_member("gethours", &date_gethours);
    date_obj->set_member("getmilliseconds", &date_getmilliseconds);
    date_obj->set_member("getminutes", &date_getminutes);
    date_obj->set_member("getmonth", &date_getmonth);
    date_obj->set_member("getseconds", &date_getseconds);
    date_obj->set_member("gettime", &date_gettime);
    date_obj->set_member("gettimezoneoffset", &date_gettimezoneoffset);
    date_obj->set_member("getutc", &date_getutc);
    date_obj->set_member("getutcday", &date_getutcday);
    date_obj->set_member("getutcfullyear", &date_getutcfullyear);
    date_obj->set_member("getutchours", &date_getutchours);
    date_obj->set_member("getutcmilliseconds", &date_getutcmilliseconds);
    date_obj->set_member("getutcminutes", &date_getutcminutes);
    date_obj->set_member("getutcmonth", &date_getutcmonth);
    date_obj->set_member("getutcseconds", &date_getutcseconds);
    date_obj->set_member("getyear", &date_getyear);
    date_obj->set_member("set", &date_set);
    date_obj->set_member("setfullyear", &date_setfullyear);
    date_obj->set_member("sethours", &date_sethours);
    date_obj->set_member("setmilliseconds", &date_setmilliseconds);
    date_obj->set_member("setminutes", &date_setminutes);
    date_obj->set_member("setmonth", &date_setmonth);
    date_obj->set_member("setseconds", &date_setseconds);
    date_obj->set_member("settime", &date_settime);
    date_obj->set_member("setutc", &date_setutc);
    date_obj->set_member("setutcfullyear", &date_setutcfullyear);
    date_obj->set_member("setutchours", &date_setutchours);
    date_obj->set_member("setutcmilliseconds", &date_setutcmilliseconds);
    date_obj->set_member("setutcminutes", &date_setutcminutes);
    date_obj->set_member("setutcmonth", &date_setutcmonth);
    date_obj->set_member("setutcseconds", &date_setutcseconds);
    date_obj->set_member("setyear", &date_setyear);
    date_obj->set_member("tostring", &date_tostring);
    date_obj->set_member("utc", &date_utc);

    if (fn.nargs == 0)
    {
        struct timeval tEnd;
        gettimeofday(&tEnd,NULL);
        date_obj->obj.millisecond = tEnd.tv_usec;
        time_t t = time(&t);
        struct tm *ti = localtime(&t);
        date_obj->obj.second = ti->tm_sec;
        date_obj->obj.minute = ti->tm_min;
        date_obj->obj.hour = ti->tm_hour;
        date_obj->obj.date = ti->tm_mday;
        date_obj->obj.month = ti->tm_mon;
        date_obj->obj.year = ti->tm_year;
        date_obj->obj.dayWeek = ti->tm_wday;
    }
    else
        log_error("date_new constructor with %d arguments unimplemented!",fn.nargs);

    fn.result->set_as_object(date_obj);
}
void date_getdate(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.date);
}
void date_getday(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.dayWeek);
}
void date_getfullyear(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.year + 1900);
}
void date_gethours(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.hour);
}
void date_getmilliseconds(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.millisecond);
}
void date_getminutes(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.minute);
}
void date_getmonth(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.month);
}
void date_getseconds(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.second);
}
void date_gettime(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_gettimezoneoffset(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutc(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutcday(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutcfullyear(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutchours(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutcmilliseconds(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutcminutes(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutcmonth(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getutcseconds(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_getyear(const fn_call& fn) {
    date_as_object* date = (date_as_object*) (as_object*) fn.this_ptr;
    fn.result->set_int(date->obj.year);
}
void date_set(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setfullyear(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_sethours(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setmilliseconds(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setminutes(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setmonth(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setseconds(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_settime(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutc(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutcfullyear(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutchours(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutcmilliseconds(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutcminutes(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutcmonth(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setutcseconds(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_setyear(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_tostring(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void date_utc(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

