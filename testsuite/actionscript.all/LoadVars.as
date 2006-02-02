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

// Test case for LoadVars ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new LoadVars;

// test the LoadVars constuctor
if (tmp) {
	trace("PASSED: LoadVars::LoadVars() constructor");
} else {
	trace("FAILED: LoadVars::LoadVars()");		
}

// test the LoadVars::addrequestheader method
if (tmp.addrequestheader) {
	trace("PASSED: LoadVars::addrequestheader() exists");
} else {
	trace("FAILED: LoadVars::addrequestheader() doesn't exist");
}
// test the LoadVars::decode method
if (tmp.decode) {
	trace("PASSED: LoadVars::decode() exists");
} else {
	trace("FAILED: LoadVars::decode() doesn't exist");
}
// test the LoadVars::getbytesloaded method
if (tmp.getbytesloaded) {
	trace("PASSED: LoadVars::getbytesloaded() exists");
} else {
	trace("FAILED: LoadVars::getbytesloaded() doesn't exist");
}
// test the LoadVars::getbytestotal method
if (tmp.getbytestotal) {
	trace("PASSED: LoadVars::getbytestotal() exists");
} else {
	trace("FAILED: LoadVars::getbytestotal() doesn't exist");
}
// test the LoadVars::load method
if (tmp.load) {
	trace("PASSED: LoadVars::load() exists");
} else {
	trace("FAILED: LoadVars::load() doesn't exist");
}
// test the LoadVars::send method
if (tmp.send) {
	trace("PASSED: LoadVars::send() exists");
} else {
	trace("FAILED: LoadVars::send() doesn't exist");
}
// test the LoadVars::sendandload method
if (tmp.sendandload) {
	trace("PASSED: LoadVars::sendandload() exists");
} else {
	trace("FAILED: LoadVars::sendandload() doesn't exist");
}
// test the LoadVars::tostring method
if (tmp.tostring) {
	trace("PASSED: LoadVars::tostring() exists");
} else {
	trace("FAILED: LoadVars::tostring() doesn't exist");
}
