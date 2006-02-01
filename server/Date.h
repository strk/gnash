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

#ifndef __DATE_H__
#define __DATE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"

namespace gnash {
  
class Date {
public:
    Date();
    ~Date();
   void get();
   void getDay();
   void getFullYear();
   void getHours();
   void getMilliseconds();
   void getMinutes();
   void getMonth();
   void getSeconds();
   void getTime();
   void getTimezoneOffset();
   void getUTC();
   void getUTCDay();
   void getUTCFullYear();
   void getUTCHours();
   void getUTCMilliseconds();
   void getUTCMinutes();
   void getUTCMonth();
   void getUTCSeconds();
   void getYear();
   void set();
   void setFullYear();
   void setHours();
   void setMilliseconds();
   void setMinutes();
   void setMonth();
   void setSeconds();
   void setTime();
   void setUTC();
   void setUTCFullYear();
   void setUTCHours();
   void setUTCMilliseconds();
   void setUTCMinutes();
   void setUTCMonth();
   void setUTCSeconds();
   void setYear();
   void toString();
   void UTC();
private:
};

struct date_as_object : public as_object
{
    Date obj;
};

void date_new(const fn_call& fn);
void date_get(const fn_call& fn);
void date_getday(const fn_call& fn);
void date_getfullyear(const fn_call& fn);
void date_gethours(const fn_call& fn);
void date_getmilliseconds(const fn_call& fn);
void date_getminutes(const fn_call& fn);
void date_getmonth(const fn_call& fn);
void date_getseconds(const fn_call& fn);
void date_gettime(const fn_call& fn);
void date_gettimezoneoffset(const fn_call& fn);
void date_getutc(const fn_call& fn);
void date_getutcday(const fn_call& fn);
void date_getutcfullyear(const fn_call& fn);
void date_getutchours(const fn_call& fn);
void date_getutcmilliseconds(const fn_call& fn);
void date_getutcminutes(const fn_call& fn);
void date_getutcmonth(const fn_call& fn);
void date_getutcseconds(const fn_call& fn);
void date_getyear(const fn_call& fn);
void date_set(const fn_call& fn);
void date_setfullyear(const fn_call& fn);
void date_sethours(const fn_call& fn);
void date_setmilliseconds(const fn_call& fn);
void date_setminutes(const fn_call& fn);
void date_setmonth(const fn_call& fn);
void date_setseconds(const fn_call& fn);
void date_settime(const fn_call& fn);
void date_setutc(const fn_call& fn);
void date_setutcfullyear(const fn_call& fn);
void date_setutchours(const fn_call& fn);
void date_setutcmilliseconds(const fn_call& fn);
void date_setutcminutes(const fn_call& fn);
void date_setutcmonth(const fn_call& fn);
void date_setutcseconds(const fn_call& fn);
void date_setyear(const fn_call& fn);
void date_tostring(const fn_call& fn);
void date_utc(const fn_call& fn);

} // end of gnash namespace

// __DATE_H__
#endif

