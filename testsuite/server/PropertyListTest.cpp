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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "check.h"
#include "PropertyList.h"
#include "as_object.h" // need to set as owner of PropertyList
#include "as_value.h"
#include "log.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity();

	as_object obj;
	PropertyList props;

	as_value val("value");
	as_value val2("value2");
	as_value val3("value3");
	as_value ret;

	check_equals(props.size(), 0);
	check ( props.setValue("Var0", val, obj) );
	check_equals(props.size(), 1);

	check ( props.getValue("Var0", ret, obj) );
	check_equals ( ret, val );

	// search should be case-insensitive
	check ( props.getValue("var0", ret, obj) );
	check_equals ( ret, val );

	// new value overrides existing value
	check ( props.setValue("Var0", val2, obj) );
	check_equals(props.size(), 1);
	check ( props.getValue("Var0", ret, obj) );
	check_equals ( ret, val2 );

	// case-insensitive setting value overrides existing value
	check ( props.setValue("var0", val3, obj) );
	check_equals(props.size(), 1);
	check ( props.getValue("vAr0", ret, obj) );
	check_equals ( ret, val3 );

	// Now add some new labels
	check ( props.setValue("var1", val, obj) );
	check_equals(props.size(), 2);
	check ( props.setValue("var2", val, obj) );
	check_equals(props.size(), 3);
	check ( props.setValue("var3", val, obj) );
	check_equals(props.size(), 4);

}

