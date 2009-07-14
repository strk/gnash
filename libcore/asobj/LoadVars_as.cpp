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
	as_value loadvars_onLoad(const fn_call& fn);
	
    as_object* getLoadVarsInterface();
	void attachLoadVarsInterface(as_object& o);
}

//--------------------------------------------

/// LoadVars ActionScript class
//
class LoadVars_as : public LoadableObject
{

public:

	/// @param env
	/// 	Environment to use for event handlers calls
	///
	LoadVars_as();

	~LoadVars_as() {};

    /// Convert the LoadVars Object to a string.
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether URL encoding is necessary. This is
    ///                 ignored because LoadVars objects are always
    ///                 URL encoded.
    void toString(std::ostream& o, bool encode) const;

protected:

#ifdef GNASH_USE_GC
	/// Mark all reachable resources, for the GC
	//
	/// There are no special reachable resources here
	///
	virtual void markReachableResources() const
	{

		// Invoke generic as_object marker
		markAsObjectReachable();
	}

#endif // GNASH_USE_GC

private:

	boost::intrusive_ptr<as_function> _onLoad;

};


void
LoadVars_as::toString(std::ostream& o, bool /*post*/) const
{

	typedef PropertyList::SortedPropertyList VarMap;
	VarMap vars;

	enumerateProperties(vars);

    as_object* global = getGlobal(*this);
    assert(global);

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

}



LoadVars_as::LoadVars_as()
		:
		as_object(getLoadVarsInterface())
{
}


// extern (used by Global.cpp)
void
loadvars_class_init(as_object& global)
{
	// This is going to be the global LoadVars "class"/"function"
	static boost::intrusive_ptr<as_object> cl;

	if ( cl == NULL )
	{
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&loadvars_ctor, getLoadVarsInterface());;
	}

	// Register _global.LoadVars, only visible for SWF6 up
	int swf6flags = as_prop_flags::dontEnum | 
                    as_prop_flags::dontDelete | 
                    as_prop_flags::onlySWF6Up;

	global.init_member("LoadVars", cl.get(), swf6flags);

}

namespace {

void
attachLoadVarsInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    VM& vm = getVM(o);

	o.init_member("addRequestHeader", new builtin_function(
	            LoadableObject::loadableobject_addRequestHeader));
	o.init_member("decode", vm.getNative(301, 3));
	o.init_member("getBytesLoaded", new builtin_function(
	            LoadableObject::loadableobject_getBytesLoaded));
	o.init_member("getBytesTotal", new builtin_function(
                LoadableObject::loadableobject_getBytesTotal));
	o.init_member("load", vm.getNative(301, 0));
	o.init_member("send", vm.getNative(301, 1));
	o.init_member("sendAndLoad", vm.getNative(301, 2));
	o.init_member("toString", gl->createFunction(loadvars_tostring));
	o.init_member("onData", gl->createFunction(loadvars_onData));
	o.init_member("onLoad", gl->createFunction(loadvars_onLoad));
}

as_object*
getLoadVarsInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if (!o) {
		o = new as_object(getObjectInterface());
		attachLoadVarsInterface(*o);
	}
	return o.get();
}

as_value
loadvars_onData(const fn_call& fn)
{

	as_object* thisPtr = fn.this_ptr.get();
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
	boost::intrusive_ptr<LoadVars_as> ptr =
        ensureType<LoadVars_as>(fn.this_ptr);

    std::ostringstream data;
    ptr->toString(data, true);
    return as_value(data.str()); 
}

as_value
loadvars_ctor(const fn_call& fn)
{

    if (!fn.isInstantiation()) return as_value();

	boost::intrusive_ptr<as_object> obj = new LoadVars_as;

	if ( fn.nargs )
	{
        IF_VERBOSE_ASCODING_ERRORS(
		    std::ostringstream ss;
		    fn.dump_args(ss);
		    log_aserror("new LoadVars(%s) - arguments discarded",
                ss.str());
        );
	}
	
	return as_value(obj.get()); // will keep alive
}

} // anonymous namespace
} // end of gnash namespace
