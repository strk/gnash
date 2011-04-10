// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
#include "PropFlags.h"
#include "ManualClock.h"
#include "RunResources.h"
#include "StreamProvider.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <string>
#include <utility> // for make_pair

#include "check.h"

#define check_strictly_equals(a, b) check(a.strictly_equals(b))

using namespace std;
using namespace gnash;

bool
getVal(PropertyList& p, const ObjectURI& k, as_value& val, as_object& obj)
{
    if (Property* prop = p.getProperty(k)) {
        val = prop->getValue(obj);
        return true;
    }
    return false;
}

int
main(int /*argc*/, char** /*argv*/)
{
	cout << "sizeof(Property): " << sizeof(Property) << endl;
	cout << "sizeof(PropertyList): " << sizeof(PropertyList) << endl;

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity();

    // We don't care about the base URL.
    RunResources runResources;
    const URL url("");
    runResources.setStreamProvider(
            boost::shared_ptr<StreamProvider>(new StreamProvider(url, url)));
	
    boost::intrusive_ptr<movie_definition> md5(
            new DummyMovieDefinition(runResources, 5));
	boost::intrusive_ptr<movie_definition> md7(
            new DummyMovieDefinition(runResources, 7));

	// TODO: test both SWF5 and SWF7 as they are different !!

	ManualClock clock;

    movie_root root(clock, runResources);

    root.init(md5.get(), MovieClip::MovieVariables());

    VM& vm = root.getVM();

	log_debug("VM version %d", vm.getSWFVersion());

	as_object* obj = new as_object(getGlobal(vm));
	PropertyList props(*obj);

	as_value val("value");
	as_value val2("value2");
	as_value val3("value3");
	as_value ret;

	if (vm.getSWFVersion() > 6) // SWF 7 or higher is case sensitive.
	{
		check_equals(props.size(), 0);
		check ( props.setValue(getURI(vm, "Var0"), val) );
		check_equals(props.size(), 1);

		check (getVal(props, getURI(vm, "Var0"), ret, *obj) );
		check_strictly_equals ( ret, val );

		// search should be case-sensitive
		check (!getVal(props, getURI(vm, "var0"), ret, *obj) );

		// new value overrides existing value
		check ( props.setValue(getURI(vm, "Var0"), val2) );
		check_equals(props.size(), 1);
		check (getVal(props, getURI(vm, "Var0"), ret, *obj) );
		check_strictly_equals ( ret, val2 );

		// case-sensitive setting value doesn't overrides existing value
		check ( props.setValue(getURI(vm, "var0"), val3) );
		check_equals(props.size(), 2);
		check (!getVal(props, getURI(vm, "vAr0"), ret, *obj) );

		// Now add some new labels
		check ( props.setValue(getURI(vm, "var1"), val) );
		check_equals(props.size(), 3);
		check ( props.setValue(getURI(vm, "var2"), val) );
		check_equals(props.size(), 4);
		check ( props.setValue(getURI(vm, "var3"), val) );
		check_equals(props.size(), 5);

		// Test deletion of properties

		// this succeeds
		check(props.delProperty(getURI(vm, "var3")).second);
		check_equals(props.size(), 4);

		// this fails (non existent property)
		check(!props.delProperty(getURI(vm, "non-existent")).first);
		check_equals(props.size(), 4);

		// Set property var2 as protected from deletion!
		props.setFlags(getURI(vm, "var2"), PropFlags::dontDelete, 0);
        check(props.getProperty(getURI(vm, "var2")));
		// this fails (protected from deletion)
		std::pair<bool, bool> delpair = props.delProperty(getURI(vm, "var2"));
		check_equals(delpair.first, true); // property was found
		check_equals(delpair.second, false); // property was NOT deleted
		check_equals(props.size(), 4);

	}
	else
	{

		// Below SWF or is not case sensitive.

		check_equals(props.size(), 0);
		check ( props.setValue(getURI(vm, "Var0"), val) );
		check_equals(props.size(), 1);

		check (getVal(props, getURI(vm, "Var0"), ret, *obj) );
		check_strictly_equals ( ret, val );

		// search should be case-insensitive
		check (getVal(props, getURI(vm, "var0"), ret, *obj) );
		check_strictly_equals ( ret, val );

		// new value overrides existing value
		check ( props.setValue(getURI(vm, "Var0"), val2) );
		check_equals(props.size(), 1);
		check (getVal(props, getURI(vm, "Var0"), ret, *obj) );
		check_strictly_equals ( ret, val2 );

		// case-insensitive setting value should override existing value
		check ( props.setValue(getURI(vm, "var0"), val3) );
		check_equals(props.size(), 1);
		check (getVal(props, getURI(vm, "vAr0"), ret, *obj) );
		check_strictly_equals ( ret, val3 );

		// Now add some new labels
		check ( props.setValue(getURI(vm, "var1"), val) );
		check_equals(props.size(), 2);
		check ( props.setValue(getURI(vm, "var2"), val) );
		check_equals(props.size(), 3);
		check ( props.setValue(getURI(vm, "var3"), val) );
		check_equals(props.size(), 4);

		// Test deletion of properties

		// this succeeds
		check(props.delProperty(getURI(vm, "var3")).second);
		check_equals(props.size(), 3);

		// this fails (non existent property)
		check(!props.delProperty(getURI(vm, "non-existent")).first);
		check_equals(props.size(), 3);

		// Set property var2 as protected from deletion!
		props.setFlags(getURI(vm, "var2"), PropFlags::dontDelete, 0);
        check(props.getProperty(getURI(vm, "var2")));
		// this fails (protected from deletion)
		std::pair<bool, bool> delpair = props.delProperty(getURI(vm, "var2"));
		check_equals(delpair.first, true); // property was found
		check_equals(delpair.second, false); // property was NOT deleted
		check_equals(props.size(), 3);

	}
}

