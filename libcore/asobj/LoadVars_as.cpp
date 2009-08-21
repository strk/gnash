// LoadVars.cpp:  ActionScript "LoadVars" class (HTTP variables), for Gnash.
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
//

#include "LoadableObject.h"
#include "LoadVars_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "builtin_function.h" // need builtin_function
#include "NativeFunction.h" // need builtin_function
#include "as_function.h" // for calling event handlers
#include "as_value.h" // for setting up a fn_call
#include "VM.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "PropertyList.h"
#include "Global_as.h"

#include <list>
#include <boost/algorithm/string/case_conv.hpp>
#include <functional>

//#define DEBUG_LOADS 1

namespace gnash {

namespace {

    as_value loadvars_tostring(const fn_call& fn);
    as_value loadvars_ctor(const fn_call& fn);
	as_value loadvars_onLoad(const fn_call& fn);
	as_value loadvars_getBytesLoaded(const fn_call& fn);
	as_value loadvars_getBytesTotal(const fn_call& fn);
	as_value loadvars_onData(const fn_call& fn);
	void attachLoadVarsInterface(as_object& o);
}

// extern (used by Global.cpp)
void
loadvars_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, loadvars_ctor, attachLoadVarsInterface, 0, uri);
}

namespace {

void
attachLoadVarsInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    VM& vm = getVM(o);

	o.init_member("addRequestHeader", gl->createFunction(
	            LoadableObject::loadableobject_addRequestHeader));
	o.init_member("decode", vm.getNative(301, 3));
	o.init_member("getBytesLoaded", gl->createFunction(
	            LoadableObject::loadableobject_getBytesLoaded));
	o.init_member("getBytesTotal", gl->createFunction(
                LoadableObject::loadableobject_getBytesTotal));
	o.init_member("load", vm.getNative(301, 0));
	o.init_member("send", vm.getNative(301, 1));
	o.init_member("sendAndLoad", vm.getNative(301, 2));
	o.init_member("toString", gl->createFunction(loadvars_tostring));
	o.init_member("onData", gl->createFunction(loadvars_onData));
	o.init_member("onLoad", gl->createFunction(loadvars_onLoad));
    o.init_member("contentType", "application/x-www-form-urlencoded");
}

as_value
loadvars_onData(const fn_call& fn)
{

	as_object* thisPtr = fn.this_ptr;
	if (!thisPtr) return as_value();

	// See http://gitweb.freedesktop.org/?p=swfdec/swfdec.git;a=blob;f=libswfdec/swfdec_initialize.as

	as_value src; 
	if (fn.nargs) src = fn.arg(0);

	if (src.is_undefined()) {
		thisPtr->set_member(NSV::PROP_LOADED, false);
		thisPtr->callMethod(NSV::PROP_ON_LOAD, false);
    }
    else {
		VM& vm = getVM(fn);
		string_table& st = vm.getStringTable();
		string_table::key decodeKey = st.find("decode"); 

		thisPtr->set_member(NSV::PROP_LOADED, true);
		thisPtr->callMethod(decodeKey, src);
		thisPtr->callMethod(NSV::PROP_ON_LOAD, true);
	}

	return as_value();
}

as_value
loadvars_onLoad(const fn_call& /*fn*/)
{
	//GNASH_REPORT_FUNCTION;
	return as_value();
}


as_value
loadvars_tostring(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

	typedef PropertyList::SortedPropertyList VarMap;
	VarMap vars;

	ptr->enumerateProperties(vars);

    as_object* global = getGlobal(*ptr);
    std::ostringstream o;
    
    // LoadVars.toString() calls _global.escape().
	for (VarMap::const_iterator it=vars.begin(), itEnd=vars.end();
			it != itEnd; ++it) {

        if (it != vars.begin()) o << "&";
        const std::string& var = 
            global->callMethod(NSV::PROP_ESCAPE, it->first).to_string();
        const std::string& val = 
            global->callMethod(NSV::PROP_ESCAPE, it->second).to_string();
        o << var << "=" << val;
	}
    return as_value(o.str()); 
}

as_value
loadvars_ctor(const fn_call& fn)
{

    if (!fn.isInstantiation()) return as_value();

	as_object* obj = fn.this_ptr;
    obj->setRelay(new LoadableObject(obj));

    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs) {
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("new LoadVars(%s) - arguments discarded",
                ss.str());
        }
    );
	
	return as_value(); // will keep alive
}

} // anonymous namespace
} // end of gnash namespace
