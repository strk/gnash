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

// Test case for Date ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Date;

// test the Date constuctor
if (tmp) {
	trace("PASSED: Date::Date() constructor");
} else {
	trace("FAILED: Date::Date()");		
}

// test the Date::get method
if (tmp.get) {
	trace("PASSED: Date::get() exists");
} else {
	trace("FAILED: Date::get() doesn't exist");
}
// test the Date::getday method
if (tmp.getday) {
	trace("PASSED: Date::getday() exists");
} else {
	trace("FAILED: Date::getday() doesn't exist");
}
// test the Date::getfullyear method
if (tmp.getfullyear) {
	trace("PASSED: Date::getfullyear() exists");
} else {
	trace("FAILED: Date::getfullyear() doesn't exist");
}
// test the Date::gethours method
if (tmp.gethours) {
	trace("PASSED: Date::gethours() exists");
} else {
	trace("FAILED: Date::gethours() doesn't exist");
}
// test the Date::getmilliseconds method
if (tmp.getmilliseconds) {
	trace("PASSED: Date::getmilliseconds() exists");
} else {
	trace("FAILED: Date::getmilliseconds() doesn't exist");
}
// test the Date::getminutes method
if (tmp.getminutes) {
	trace("PASSED: Date::getminutes() exists");
} else {
	trace("FAILED: Date::getminutes() doesn't exist");
}
// test the Date::getmonth method
if (tmp.getmonth) {
	trace("PASSED: Date::getmonth() exists");
} else {
	trace("FAILED: Date::getmonth() doesn't exist");
}
// test the Date::getseconds method
if (tmp.getseconds) {
	trace("PASSED: Date::getseconds() exists");
} else {
	trace("FAILED: Date::getseconds() doesn't exist");
}
// test the Date::gettime method
if (tmp.gettime) {
	trace("PASSED: Date::gettime() exists");
} else {
	trace("FAILED: Date::gettime() doesn't exist");
}
// test the Date::gettimezoneoffset method
if (tmp.gettimezoneoffset) {
	trace("PASSED: Date::gettimezoneoffset() exists");
} else {
	trace("FAILED: Date::gettimezoneoffset() doesn't exist");
}
// test the Date::getutc method
if (tmp.getutc) {
	trace("PASSED: Date::getutc() exists");
} else {
	trace("FAILED: Date::getutc() doesn't exist");
}
// test the Date::getutcday method
if (tmp.getutcday) {
	trace("PASSED: Date::getutcday() exists");
} else {
	trace("FAILED: Date::getutcday() doesn't exist");
}
// test the Date::getutcfullyear method
if (tmp.getutcfullyear) {
	trace("PASSED: Date::getutcfullyear() exists");
} else {
	trace("FAILED: Date::getutcfullyear() doesn't exist");
}
// test the Date::getutchours method
if (tmp.getutchours) {
	trace("PASSED: Date::getutchours() exists");
} else {
	trace("FAILED: Date::getutchours() doesn't exist");
}
// test the Date::getutcmilliseconds method
if (tmp.getutcmilliseconds) {
	trace("PASSED: Date::getutcmilliseconds() exists");
} else {
	trace("FAILED: Date::getutcmilliseconds() doesn't exist");
}
// test the Date::getutcminutes method
if (tmp.getutcminutes) {
	trace("PASSED: Date::getutcminutes() exists");
} else {
	trace("FAILED: Date::getutcminutes() doesn't exist");
}
// test the Date::getutcmonth method
if (tmp.getutcmonth) {
	trace("PASSED: Date::getutcmonth() exists");
} else {
	trace("FAILED: Date::getutcmonth() doesn't exist");
}
// test the Date::getutcseconds method
if (tmp.getutcseconds) {
	trace("PASSED: Date::getutcseconds() exists");
} else {
	trace("FAILED: Date::getutcseconds() doesn't exist");
}
// test the Date::getyear method
if (tmp.getyear) {
	trace("PASSED: Date::getyear() exists");
} else {
	trace("FAILED: Date::getyear() doesn't exist");
}
// test the Date::set method
if (tmp.set) {
	trace("PASSED: Date::set() exists");
} else {
	trace("FAILED: Date::set() doesn't exist");
}
// test the Date::setfullyear method
if (tmp.setfullyear) {
	trace("PASSED: Date::setfullyear() exists");
} else {
	trace("FAILED: Date::setfullyear() doesn't exist");
}
// test the Date::sethours method
if (tmp.sethours) {
	trace("PASSED: Date::sethours() exists");
} else {
	trace("FAILED: Date::sethours() doesn't exist");
}
// test the Date::setmilliseconds method
if (tmp.setmilliseconds) {
	trace("PASSED: Date::setmilliseconds() exists");
} else {
	trace("FAILED: Date::setmilliseconds() doesn't exist");
}
// test the Date::setminutes method
if (tmp.setminutes) {
	trace("PASSED: Date::setminutes() exists");
} else {
	trace("FAILED: Date::setminutes() doesn't exist");
}
// test the Date::setmonth method
if (tmp.setmonth) {
	trace("PASSED: Date::setmonth() exists");
} else {
	trace("FAILED: Date::setmonth() doesn't exist");
}
// test the Date::setseconds method
if (tmp.setseconds) {
	trace("PASSED: Date::setseconds() exists");
} else {
	trace("FAILED: Date::setseconds() doesn't exist");
}
// test the Date::settime method
if (tmp.settime) {
	trace("PASSED: Date::settime() exists");
} else {
	trace("FAILED: Date::settime() doesn't exist");
}
// test the Date::setutc method
if (tmp.setutc) {
	trace("PASSED: Date::setutc() exists");
} else {
	trace("FAILED: Date::setutc() doesn't exist");
}
// test the Date::setutcfullyear method
if (tmp.setutcfullyear) {
	trace("PASSED: Date::setutcfullyear() exists");
} else {
	trace("FAILED: Date::setutcfullyear() doesn't exist");
}
// test the Date::setutchours method
if (tmp.setutchours) {
	trace("PASSED: Date::setutchours() exists");
} else {
	trace("FAILED: Date::setutchours() doesn't exist");
}
// test the Date::setutcmilliseconds method
if (tmp.setutcmilliseconds) {
	trace("PASSED: Date::setutcmilliseconds() exists");
} else {
	trace("FAILED: Date::setutcmilliseconds() doesn't exist");
}
// test the Date::setutcminutes method
if (tmp.setutcminutes) {
	trace("PASSED: Date::setutcminutes() exists");
} else {
	trace("FAILED: Date::setutcminutes() doesn't exist");
}
// test the Date::setutcmonth method
if (tmp.setutcmonth) {
	trace("PASSED: Date::setutcmonth() exists");
} else {
	trace("FAILED: Date::setutcmonth() doesn't exist");
}
// test the Date::setutcseconds method
if (tmp.setutcseconds) {
	trace("PASSED: Date::setutcseconds() exists");
} else {
	trace("FAILED: Date::setutcseconds() doesn't exist");
}
// test the Date::setyear method
if (tmp.setyear) {
	trace("PASSED: Date::setyear() exists");
} else {
	trace("FAILED: Date::setyear() doesn't exist");
}
// test the Date::tostring method
if (tmp.tostring) {
	trace("PASSED: Date::tostring() exists");
} else {
	trace("FAILED: Date::tostring() doesn't exist");
}
// test the Date::utc method
if (tmp.utc) {
	trace("PASSED: Date::utc() exists");
} else {
	trace("FAILED: Date::utc() doesn't exist");
}
