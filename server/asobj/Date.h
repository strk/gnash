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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
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
	// Use double to avoid overflow
	double getTime();
	void getTimezoneOffset();
	void getUTCDate();
	void getUTCDay();
	void getUTCFullYear();
	void getUTCHours();
	void getUTCMilliseconds();
	void getUTCMinutes();
	void getUTCMonth();
	void getUTCSeconds();
	void getYear();
	void setDate();
	void setFullYear();
	void setHours();
	void setMilliseconds();
	void setMinutes();
	void setMonth();
	void setSeconds();
	void setTime();
	void setUTCDate();
	void setUTCFullYear();
	void setUTCHours();
	void setUTCMilliseconds();
	void setUTCMinutes();
	void setUTCMonth();
	void setUTCSeconds();
	void setYear();
	void toString();
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
	// This function normalizes the time - for example, if we set the date to
	// Jan-32, 1:61:60, after normalize the time will be Feb-1, 2:02:00
	void Normalize();
private:
};

class date_as_object : public as_object
{
public:
	 Date obj;
};

void date_new(const fn_call& fn);
void date_getdate(const fn_call& fn);
void date_getday(const fn_call& fn);
void date_getfullyear(const fn_call& fn);
void date_gethours(const fn_call& fn);
void date_getmilliseconds(const fn_call& fn);
void date_getminutes(const fn_call& fn);
void date_getmonth(const fn_call& fn);
void date_getseconds(const fn_call& fn);
void date_gettime(const fn_call& fn);
void date_gettimezoneoffset(const fn_call& fn);
void date_getutcdate(const fn_call& fn);
void date_getutcday(const fn_call& fn);
void date_getutcfullyear(const fn_call& fn);
void date_getutchours(const fn_call& fn);
void date_getutcmilliseconds(const fn_call& fn);
void date_getutcminutes(const fn_call& fn);
void date_getutcmonth(const fn_call& fn);
void date_getutcseconds(const fn_call& fn);
void date_getyear(const fn_call& fn);
void date_setdate(const fn_call& fn);
void date_setfullyear(const fn_call& fn);
void date_sethours(const fn_call& fn);
void date_setmilliseconds(const fn_call& fn);
void date_setminutes(const fn_call& fn);
void date_setmonth(const fn_call& fn);
void date_setseconds(const fn_call& fn);
void date_settime(const fn_call& fn);
void date_setutcdate(const fn_call& fn);
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

