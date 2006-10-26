// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
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

#include "PropertyList.h"
#include "dejagnu.h"
#include "log.h"

#include "check.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{

	dbglogfile.setVerbosity();

	PropertyList props;

	as_value val("value");
	as_value val2("value2");
	as_value val3("value3");
	as_value ret;

	check_equals(props.size(), 0);
	check ( props.setValue("Var0", val) );
	check_equals(props.size(), 1);

	check ( props.getValue("Var0", ret) );
	check_equals ( ret, val );

	// search should be case-insensitive
	check ( props.getValue("var0", ret) );
	check_equals ( ret, val );

	// new value overrides existing value
	check ( props.setValue("Var0", val2) );
	check_equals(props.size(), 1);
	check ( props.getValue("Var0", ret) );
	check_equals ( ret, val2 );

	// case-insensitive setting value overrides existing value
	check ( props.setValue("var0", val3) );
	check_equals(props.size(), 1);
	check ( props.getValue("vAr0", ret) );
	check_equals ( ret, val3 );

	// Now add some new labels
	check ( props.setValue("var1", val) );
	check_equals(props.size(), 2);
	check ( props.setValue("var2", val) );
	check_equals(props.size(), 3);
	check ( props.setValue("var3", val) );
	check_equals(props.size(), 4);

}

