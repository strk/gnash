// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "PropertyList.h"
#include "DummyMovieDefinition.h"
#include "VM.h"
#include "movie_root.h"
#include "as_object.h" // need to set as owner of PropertyList
#include "as_value.h"
#include "log.h"
#include "smart_ptr.h"
#include "as_prop_flags.h"
#include "ManualClock.h"
#include "RunResources.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <string>
#include <utility> // for make_pair

#include "check.h"

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{
	cout << "sizeof(Property): " << sizeof(Property) << endl;
	cout << "sizeof(PropertyList): " << sizeof(PropertyList) << endl;

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity();

	// Initialize gnash lib
	gnashInit();

    // We don't care about the base URL.
    RunResources runResources("");
	
    boost::intrusive_ptr<movie_definition> md5(
            new DummyMovieDefinition(runResources, 5));
	boost::intrusive_ptr<movie_definition> md7(
            new DummyMovieDefinition(runResources, 7));

	// TODO: test both SWF5 and SWF7 as they are different !!

	ManualClock clock;

    movie_root root(*md5, clock, runResources);

    root.setRootMovie( md5->createMovie() );

    VM& vm = root.getVM();

	log_debug("VM version %d", vm.getSWFVersion());

	as_object obj;
	PropertyList props(vm);

	as_value val("value");
	as_value val2("value2");
	as_value val3("value3");
	as_value ret;

	string_table& st = vm.getStringTable();

	if (vm.getSWFVersion() > 6) // SWF 7 or higher is case sensitive.
	{
		check_equals(props.size(), 0);
		check ( props.setValue(st.find("Var0"), val, obj) );
		check_equals(props.size(), 1);

		check ( props.getValue(st.find("Var0"), ret, obj) );
		check_equals ( ret, val );

		// search should be case-sensitive
		check ( ! props.getValue(st.find("var0"), ret, obj) );

		// new value overrides existing value
		check ( props.setValue(st.find("Var0"), val2, obj) );
		check_equals(props.size(), 1);
		check ( props.getValue(st.find("Var0"), ret, obj) );
		check_equals ( ret, val2 );

		// case-sensitive setting value doesn't overrides existing value
		check ( props.setValue(st.find("var0"), val3, obj) );
		check_equals(props.size(), 2);
		check ( ! props.getValue(st.find("vAr0"), ret, obj) );

		// Now add some new labels
		check ( props.setValue(st.find("var1"), val, obj) );
		check_equals(props.size(), 3);
		check ( props.setValue(st.find("var2"), val, obj) );
		check_equals(props.size(), 4);
		check ( props.setValue(st.find("var3"), val, obj) );
		check_equals(props.size(), 5);

		// Test deletion of properties

		// this succeeds
		check(props.delProperty(st.find("var3")).second);
		check_equals(props.size(), 4);

		// this fails (non existent property)
		check(!props.delProperty(st.find("non-existent")).first);
		check_equals(props.size(), 4);

		// Set property var2 as protected from deletion!
		check(props.setFlags(st.find("var2"), as_prop_flags::dontDelete, 0));
		// this fails (protected from deletion)
		std::pair<bool, bool> delpair = props.delProperty(st.find("var2"));
		check_equals(delpair.first, true); // property was found
		check_equals(delpair.second, false); // property was NOT deleted
		check_equals(props.size(), 4);

        PropertyList::SortedPropertyList vals;
		props.enumerateKeyValue(obj, vals);
		check_equals( vals.size(), 4 );
		check_equals( vals[0].first, "var0");
		check_equals( vals[0].second, "value3");
		check_equals( vals[1].first, "Var0");
		check_equals( vals[1].second, "value2");
		check_equals( vals[2].first, "var1");
		check_equals( vals[2].second, "value");

	}
	else
	{
		// Below SWF or is not case sensitive.

		check_equals(props.size(), 0);
		check ( props.setValue(st.find("Var0"), val, obj) );
		check_equals(props.size(), 1);

		check ( props.getValue(st.find("Var0"), ret, obj) );
		check_equals ( ret, val );

		// search should be case-insensitive
		check ( props.getValue(st.find("var0"), ret, obj) );
		check_equals ( ret, val );

		// new value overrides existing value
		check ( props.setValue(st.find("Var0"), val2, obj) );
		check_equals(props.size(), 1);
		check ( props.getValue(st.find("Var0"), ret, obj) );
		check_equals ( ret, val2 );

		// case-insensitive setting value should override existing value
		check ( props.setValue(st.find("var0"), val3, obj) );
		check_equals(props.size(), 1);
		check ( props.getValue(st.find("vAr0"), ret, obj) );
		check_equals ( ret, val3 );

		// Now add some new labels
		check ( props.setValue(st.find("var1"), val, obj) );
		check_equals(props.size(), 2);
		check ( props.setValue(st.find("var2"), val, obj) );
		check_equals(props.size(), 3);
		check ( props.setValue(st.find("var3"), val, obj) );
		check_equals(props.size(), 4);

		// Test deletion of properties

		// this succeeds
		check(props.delProperty(st.find("var3")).second);
		check_equals(props.size(), 3);

		// this fails (non existent property)
		check(!props.delProperty(st.find("non-existent")).first);
		check_equals(props.size(), 3);

		// Set property var2 as protected from deletion!
		check(props.setFlags(st.find("var2"), as_prop_flags::dontDelete, 0));
		// this fails (protected from deletion)
		std::pair<bool, bool> delpair = props.delProperty(st.find("var2"));
		check_equals(delpair.first, true); // property was found
		check_equals(delpair.second, false); // property was NOT deleted
		check_equals(props.size(), 3);

        PropertyList::SortedPropertyList vals;
		props.enumerateKeyValue(obj, vals);
		check_equals( vals.size(), 3 );
		check_equals( vals[0].first, "var2");
		check_equals( vals[0].second, "value");
		check_equals( vals[1].first, "var1");
		check_equals( vals[1].second, "value");
		check_equals( vals[2].first, "Var0");
		check_equals( vals[2].second, "value3");

	}
}

