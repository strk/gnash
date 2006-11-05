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

// Test case for System ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: System.as,v 1.7 2006/11/05 00:45:27 rsavoye Exp $";

#include "check.as"

// test the System constuctor (should fail, it's not instantiatable)
var systemObj = new System;
check (systemObj == undefined);

// test the System::security.allowdomain method
check (System.security.allowdomain != undefined);

// test the System.security.allowinsecuredomain method
check (System.security.allowinsecuredomain != undefined);

// test the System.security.loadpolicyfile method
check (System.security.loadpolicyfile != undefined);

// test the System.setclipboard method
check (System.setclipboard != undefined);

// test the System.showsettings method
check (System.showsettings != undefined);
