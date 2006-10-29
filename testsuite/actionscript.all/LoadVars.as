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

rcsid="$Id: LoadVars.as,v 1.5 2006/10/29 18:34:18 rsavoye Exp $";

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
