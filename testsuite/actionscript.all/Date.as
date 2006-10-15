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

// Test case for Date ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Date.as,v 1.5 2006/10/15 02:30:55 rsavoye Exp $";

#include "check.as"

var date = new Date;

// test the Date constuctor
check (date);

// test the Date::get method
xcheck (date.get != undefined);

// test the Date::getday method
check (date.getday != undefined);

// test the Date::getfullyear method
check (date.getfullyear != undefined);

// test the Date::gethours method
check (date.gethours != undefined);

// test the Date::getmilliseconds method
check (date.getmilliseconds != undefined);

// test the Date::getminutes method
check (date.getminutes != undefined);

// test the Date::getmonth method
check (date.getmonth != undefined);

// test the Date::getseconds method
check (date.getseconds != undefined);

// test the Date::gettime method
check (date.gettime != undefined);

// test the Date::gettimezoneoffset method
check (date.gettimezoneoffset != undefined);

// test the Date::getutc method
xcheck (date.getutc != undefined);

// test the Date::getutcday method
check (date.getutcday != undefined);

// test the Date::getutcfullyear method
check (date.getutcfullyear != undefined);

// test the Date::getutchours method
check (date.getutchours != undefined);

// test the Date::getutcmilliseconds method
check (date.getutcmilliseconds != undefined);

// test the Date::getutcminutes method
check (date.getutcminutes != undefined);

// test the Date::getutcmonth method
check (date.getutcmonth != undefined);

// test the Date::getutcseconds method
check (date.getutcseconds != undefined);

// test the Date::getyear method
check (date.getyear != undefined);

// test the Date::set method
xcheck (date.set != undefined);

// test the Date::setfullyear method
check (date.setfullyear != undefined);

// test the Date::sethours method
check (date.sethours != undefined);

// test the Date::setmilliseconds method
check (date.setmilliseconds != undefined);

// test the Date::setminutes method
check (date.setminutes != undefined);

// test the Date::setmonth method
check (date.setmonth != undefined);

// test the Date::setseconds method
check (date.setseconds != undefined);

// test the Date::settime method
check (date.settime != undefined);

// test the Date::setutc method
xcheck (date.setutc != undefined);

// test the Date::setutcfullyear method
check (date.setutcfullyear != undefined);

// test the Date::setutchours method
check (date.setutchours != undefined);

// test the Date::setutcmilliseconds method
check (date.setutcmilliseconds != undefined);

// test the Date::setutcminutes method
check (date.setutcminutes != undefined);

// test the Date::setutcmonth method
check (date.setutcmonth != undefined);

// test the Date::setutcseconds method
check (date.setutcseconds != undefined);

// test the Date::setyear method
check (date.setyear != undefined);

// test the Date::tostring method
check (date.tostring != undefined);

// test the Date::utc method
check (date.utc != undefined);
