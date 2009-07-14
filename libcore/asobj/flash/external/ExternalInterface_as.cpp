// ExternalInterface_as.cpp:  ActionScript "ExternalInterface" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ExternalInterface_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value ExternalInterface_addCallback(const fn_call& fn);
static as_value ExternalInterface_call(const fn_call& fn);
static as_value ExternalInterface_uArgumentsToXML(const fn_call& fn);
static as_value ExternalInterface_uArgumentsToAS(const fn_call& fn);
static as_value ExternalInterface_uAddCallback(const fn_call& fn);
static as_value ExternalInterface_uArrayToAS(const fn_call& fn);
static as_value ExternalInterface_uArrayToJS(const fn_call& fn);
static as_value ExternalInterface_uArrayToXML(const fn_call& fn);
static as_value ExternalInterface_uCallIn(const fn_call& fn);
static as_value ExternalInterface_uCallOut(const fn_call& fn);
static as_value ExternalInterface_uEscapeXML(const fn_call& fn);
static as_value ExternalInterface_uEvalJS(const fn_call& fn);
static as_value ExternalInterface_uInitJS(const fn_call& fn);
static as_value ExternalInterface_uJsQuoteString(const fn_call& fn);
static as_value ExternalInterface_uObjectID(const fn_call& fn);
static as_value ExternalInterface_uObjectToAS(const fn_call& fn);
static as_value ExternalInterface_uObjectToJS(const fn_call& fn);
static as_value ExternalInterface_uObjectToXML(const fn_call& fn);
static as_value ExternalInterface_uToAS(const fn_call& fn);
static as_value ExternalInterface_uToJS(const fn_call& fn);
static as_value ExternalInterface_uToXML(const fn_call& fn);
static as_value ExternalInterface_uUnescapeXML(const fn_call& fn);
static as_value ExternalInterface_available(const fn_call& fn);

as_value ExternalInterface_uctor(const fn_call& fn);

static void
attachExternalInterfaceInterface(as_object& /*o*/)
{
}

static void
attachExternalInterfaceStaticProperties(as_object& o)
{
    const int flags = as_prop_flags::dontEnum |
                      as_prop_flags::dontDelete |
                      as_prop_flags::readOnly;

    o.init_member("addCallback", new builtin_function(
                ExternalInterface_addCallback), flags);
    o.init_member("call", new builtin_function(ExternalInterface_call), flags);
    o.init_member("_argumentsToXML",
            new builtin_function(ExternalInterface_uArgumentsToXML), flags);
    o.init_member("_argumentsToAS",
            new builtin_function(ExternalInterface_uArgumentsToAS), flags);
    o.init_member("_addCallback",
            new builtin_function(ExternalInterface_uAddCallback), flags);
    o.init_member("_arrayToAS",
            new builtin_function(ExternalInterface_uArrayToAS), flags);
    o.init_member("_arrayToJS",
            new builtin_function(ExternalInterface_uArrayToJS), flags);
    o.init_member("_arrayToXML",
            new builtin_function(ExternalInterface_uArrayToXML), flags);
    o.init_member("_callIn",
            new builtin_function(ExternalInterface_uCallIn), flags);
    o.init_member("_callOut",
            new builtin_function(ExternalInterface_uCallOut), flags);
    o.init_member("_escapeXML",
            new builtin_function(ExternalInterface_uEscapeXML), flags);
    o.init_member("_evalJS",
            new builtin_function(ExternalInterface_uEvalJS), flags);
    o.init_member("_initJS",
            new builtin_function(ExternalInterface_uInitJS), flags);
    o.init_member("_jsQuoteString",
            new builtin_function(ExternalInterface_uJsQuoteString), flags);
    o.init_member("_objectID",
            new builtin_function(ExternalInterface_uObjectID), flags);
    o.init_member("_objectToAS",
            new builtin_function(ExternalInterface_uObjectToAS), flags);
    o.init_member("_objectToJS",
            new builtin_function(ExternalInterface_uObjectToJS), flags);
    o.init_member("_objectToXML",
            new builtin_function(ExternalInterface_uObjectToXML), flags);
    o.init_member("_toAS",
            new builtin_function(ExternalInterface_uToAS), flags);
    o.init_member("_toJS",
            new builtin_function(ExternalInterface_uToJS), flags);
    o.init_member("_toXML",
            new builtin_function(ExternalInterface_uToXML), flags);
    o.init_member("_unescapeXML",
            new builtin_function(ExternalInterface_uUnescapeXML), flags);

    int protectedFlags = as_prop_flags::dontEnum |
                         as_prop_flags::dontDelete |
                         as_prop_flags::isProtected;

    o.init_member("available",
            new builtin_function(ExternalInterface_available), protectedFlags);
}

static as_object*
getExternalInterfaceInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachExternalInterfaceInterface(*o);

	}

	return o.get();
}

class ExternalInterface_as: public as_object
{

public:

	ExternalInterface_as()
		:
		as_object(getExternalInterfaceInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "ExternalInterface"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};



static as_value
ExternalInterface_addCallback(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ExternalInterface_call(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uArgumentsToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uArgumentsToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uAddCallback(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uArrayToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uArrayToJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uArrayToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uCallIn(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uCallOut(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uEscapeXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uEvalJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uInitJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uJsQuoteString(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uObjectID(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uObjectToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uObjectToJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uObjectToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uToJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_uUnescapeXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_available(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}


as_value
ExternalInterface_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new ExternalInterface_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("ExternalInterface(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

as_function*
getFlashExternalExternalInterfaceConstructor()
{
    static builtin_function* cl=NULL;
    if ( ! cl )
    {
        cl=new builtin_function(&ExternalInterface_ctor,
                getExternalInterfaceInterface());
        VM::get().addStatic(cl);
	    attachExternalInterfaceStaticProperties(*cl);
    }
    return cl;
}


static as_value
get_flash_external_external_interface_constructor(const fn_call& /*fn*/)
{
    log_debug("Loading flash.external.ExternalInterface class");
    return getFlashExternalExternalInterfaceConstructor();
}


// extern 
void externalinterface_class_init(as_object& where)
{
    // Register _global.Point
    string_table& st = getStringTable(where);
    
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(st.find("ExternalInterface"),
            get_flash_external_external_interface_constructor, flags);
}


} // end of gnash namespace
