// LoadVars.cpp:  ActionScript "LoadVars" class (HTTP variables), for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include "smart_ptr.h" // GNASH_USE_GC
#include "builtin_function.h" // need builtin_function
#include "as_function.h" // for calling event handlers
#include "as_value.h" // for setting up a fn_call
#include "gnash.h" // for get_base_url
#include "VM.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "array.h"

#include <list>
#include <boost/algorithm/string/case_conv.hpp>
#include <functional>

//#define DEBUG_LOADS 1

namespace gnash {

static as_value loadvars_tostring(const fn_call& fn);
static as_value loadvars_ctor(const fn_call& fn);

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

	static as_object* getLoadVarsInterface();

	static void attachLoadVarsInterface(as_object& o);

protected:

    /// Convert the LoadVars Object to a string.
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether URL encoding is necessary. This is
    ///                 ignored because LoadVars objects are always
    ///                 URL encoded.
    void toString(std::ostream& o, bool encode) const;

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

	static as_value onLoad_getset(const fn_call& fn);

	static as_value getBytesLoaded_method(const fn_call& fn);

	static as_value getBytesTotal_method(const fn_call& fn);

	static as_value decode_method(const fn_call& fn);

	static as_value onData_method(const fn_call& fn);

	static as_value onLoad_method(const fn_call& fn);

	boost::intrusive_ptr<as_function> _onLoad;

};


void
LoadVars_as::toString(std::ostream& o, bool /*post*/) const
{

	typedef std::map<std::string, std::string> VarMap;
	VarMap vars;

	enumerateProperties(vars);

	for (VarMap::iterator it=vars.begin(), itEnd=vars.end();
			it != itEnd; ++it)
	{
	    if (it != vars.begin()) o << "&";
        const std::string& val = it->second;
        o << URL::encode(it->first) << "="
                    << URL::encode(val);
	}

}



LoadVars_as::LoadVars_as()
		:
		as_object(getLoadVarsInterface())
{
}


void
LoadVars_as::attachLoadVarsInterface(as_object& o)
{
	o.init_member("addRequestHeader", new builtin_function(
	            LoadableObject::loadableobject_addRequestHeader));
	o.init_member("decode", new builtin_function(LoadVars_as::decode_method));
	o.init_member("getBytesLoaded", new builtin_function(
	            LoadVars_as::getBytesLoaded_method));
	o.init_member("getBytesTotal", new builtin_function(
	            LoadVars_as::getBytesTotal_method));
	o.init_member("load", new builtin_function(
	            LoadableObject::loadableobject_load));
	o.init_member("send", new builtin_function(
                LoadableObject::loadableobject_send));
	o.init_member("sendAndLoad", new builtin_function(
	            LoadableObject::loadableobject_sendAndLoad));
	o.init_member("toString", new builtin_function(loadvars_tostring));
	o.init_member("onData", new builtin_function(LoadVars_as::onData_method));
	o.init_member("onLoad", new builtin_function(LoadVars_as::onLoad_method));
}

as_object*
LoadVars_as::getLoadVarsInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachLoadVarsInterface(*o);
	}
	return o.get();
}


as_value
LoadVars_as::decode_method(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars_as> ptr = ensureType<LoadVars_as>(fn.this_ptr);

	if ( ! fn.nargs ) return as_value(false);

	typedef std::map<std::string, std::string> ValuesMap;

	ValuesMap vals;

	URL::parse_querystring(fn.arg(0).to_string(), vals);

	string_table& st = ptr->getVM().getStringTable();
	for  (ValuesMap::const_iterator it=vals.begin(), itEnd=vals.end();
			it != itEnd; ++it)
	{
		ptr->set_member(st.find(it->first), as_value(it->second.c_str()));
	}

	return as_value(); 
}

as_value
LoadVars_as::getBytesLoaded_method(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars_as> ptr = ensureType<LoadVars_as>(fn.this_ptr);
	return as_value(ptr->getBytesLoaded());
}

as_value
LoadVars_as::getBytesTotal_method(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars_as> ptr = ensureType<LoadVars_as>(fn.this_ptr);
	return as_value(ptr->getBytesTotal());
}

as_value
LoadVars_as::onData_method(const fn_call& fn)
{
	//GNASH_REPORT_FUNCTION;

	as_object* thisPtr = fn.this_ptr.get();
	if ( ! thisPtr ) return as_value();

	// See http://gitweb.freedesktop.org/?p=swfdec/swfdec.git;a=blob;f=libswfdec/swfdec_initialize.as

	as_value src; src.set_null();
	if ( fn.nargs ) src = fn.arg(0);

	if ( ! src.is_null() )
	{
		VM& vm = thisPtr->getVM();
		string_table& st = vm.getStringTable();
		string_table::key decodeKey = st.find("decode"); // add to namedStrings ?

		as_value tmp(true);
		thisPtr->set_member(NSV::PROP_LOADED, tmp);
		thisPtr->callMethod(decodeKey, src);
		thisPtr->callMethod(NSV::PROP_ON_LOAD, tmp);
	}
	else
	{
		as_value tmp(true);
		thisPtr->set_member(NSV::PROP_LOADED, tmp);
		thisPtr->callMethod(NSV::PROP_ON_LOAD, tmp);
	}

	return as_value();
}

as_value
LoadVars_as::onLoad_method(const fn_call& /*fn*/)
{
	//GNASH_REPORT_FUNCTION;
	return as_value();
}


static as_value
loadvars_tostring(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars_as> ptr = ensureType<LoadVars_as>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value(); 
}

static as_value
loadvars_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new LoadVars_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		log_unimpl("new LoadVars(%s) - arguments discarded", ss.str().c_str()); // or ASERROR ?
	}
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void
loadvars_class_init(as_object& global)
{
	// This is going to be the global LoadVars "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&loadvars_ctor, LoadVars_as::getLoadVarsInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		LoadVars_as::attachLoadVarsInterface(*cl);
		     
	}

	// Register _global.LoadVars
	global.init_member("LoadVars", cl.get());

}


} // end of gnash namespace
