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

// Test case for LoadVars ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "check.as"

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
check (loadvarsObj != undefined);

// test the LoadVars::addrequestheader method
check (loadvarsObj.addrequestheader != undefined);
// test the LoadVars::decode method
check (loadvarsObj.decode != undefined);
// test the LoadVars::getbytesloaded method
check (loadvarsObj.getbytesloaded != undefined);
// test the LoadVars::getbytestotal method
check (loadvarsObj.getbytestotal != undefined);
// test the LoadVars::load method
check (loadvarsObj.load != undefined);
// test the LoadVars::send method
check (loadvarsObj.send != undefined);
// test the LoadVars::sendandload method
check (loadvarsObj.sendandload != undefined);
// test the LoadVars::tostring method
check (loadvarsObj.tostring != undefined);
