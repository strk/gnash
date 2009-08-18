// externalinterface_as.cpp:  ActionScript "ExternalInterface" class, for Gnash.
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

#include "externalinterface_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

namespace {
    as_value externalinterface_addCallback(const fn_call& fn);
    as_value externalinterface_call(const fn_call& fn);
    as_value externalinterface_uArgumentsToXML(const fn_call& fn);
    as_value externalinterface_uArgumentsToAS(const fn_call& fn);
    as_value externalinterface_uAddCallback(const fn_call& fn);
    as_value externalinterface_uArrayToAS(const fn_call& fn);
    as_value externalinterface_uArrayToJS(const fn_call& fn);
    as_value externalinterface_uArrayToXML(const fn_call& fn);
    as_value externalinterface_uCallIn(const fn_call& fn);
    as_value externalinterface_uCallOut(const fn_call& fn);
    as_value externalinterface_uEscapeXML(const fn_call& fn);
    as_value externalinterface_uEvalJS(const fn_call& fn);
    as_value externalinterface_uInitJS(const fn_call& fn);
    as_value externalinterface_uJsQuoteString(const fn_call& fn);
    as_value externalinterface_uObjectID(const fn_call& fn);
    as_value externalinterface_uObjectToAS(const fn_call& fn);
    as_value externalinterface_uObjectToJS(const fn_call& fn);
    as_value externalinterface_uObjectToXML(const fn_call& fn);
    as_value externalinterface_uToAS(const fn_call& fn);
    as_value externalinterface_uToJS(const fn_call& fn);
    as_value externalinterface_uToXML(const fn_call& fn);
    as_value externalinterface_uUnescapeXML(const fn_call& fn);
    as_value externalinterface_available(const fn_call& fn);
    as_value externalinterface_uctor(const fn_call& fn);
    as_value externalInterfaceConstructor(const fn_call& fn);

    as_object* getExternalInterfaceInterface();
}

class externalinterface_as: public as_object
{

public:

	externalinterface_as()
		:
		as_object(getExternalInterfaceInterface())
	{}
};

// extern 
void
externalinterface_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(getName(uri), externalInterfaceConstructor,
            flags, getNamespace(uri));
}

namespace {

void
attachExternalInterfaceInterface(as_object& /*o*/)
{
}

void
attachExternalInterfaceStaticProperties(as_object& o)
{
    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::readOnly;

    Global_as* gl = getGlobal(o);
    o.init_member("addCallback", gl->createFunction(
                externalinterface_addCallback), flags);
    o.init_member("call", gl->createFunction(externalinterface_call), flags);
    o.init_member("_argumentsToXML",
            gl->createFunction(externalinterface_uArgumentsToXML), flags);
    o.init_member("_argumentsToAS",
            gl->createFunction(externalinterface_uArgumentsToAS), flags);
    o.init_member("_addCallback",
            gl->createFunction(externalinterface_uAddCallback), flags);
    o.init_member("_arrayToAS",
            gl->createFunction(externalinterface_uArrayToAS), flags);
    o.init_member("_arrayToJS",
            gl->createFunction(externalinterface_uArrayToJS), flags);
    o.init_member("_arrayToXML",
            gl->createFunction(externalinterface_uArrayToXML), flags);
    o.init_member("_callIn",
            gl->createFunction(externalinterface_uCallIn), flags);
    o.init_member("_callOut",
            gl->createFunction(externalinterface_uCallOut), flags);
    o.init_member("_escapeXML",
            gl->createFunction(externalinterface_uEscapeXML), flags);
    o.init_member("_evalJS",
            gl->createFunction(externalinterface_uEvalJS), flags);
    o.init_member("_initJS",
            gl->createFunction(externalinterface_uInitJS), flags);
    o.init_member("_jsQuoteString",
            gl->createFunction(externalinterface_uJsQuoteString), flags);
    o.init_member("_objectID",
            gl->createFunction(externalinterface_uObjectID), flags);
    o.init_member("_objectToAS",
            gl->createFunction(externalinterface_uObjectToAS), flags);
    o.init_member("_objectToJS",
            gl->createFunction(externalinterface_uObjectToJS), flags);
    o.init_member("_objectToXML",
            gl->createFunction(externalinterface_uObjectToXML), flags);
    o.init_member("_toAS",
            gl->createFunction(externalinterface_uToAS), flags);
    o.init_member("_toJS",
            gl->createFunction(externalinterface_uToJS), flags);
    o.init_member("_toXML",
            gl->createFunction(externalinterface_uToXML), flags);
    o.init_member("_unescapeXML",
            gl->createFunction(externalinterface_uUnescapeXML), flags);

    int protectedFlags = PropFlags::dontEnum |
                         PropFlags::dontDelete |
                         PropFlags::isProtected;

    o.init_member("available",
            gl->createFunction(externalinterface_available), protectedFlags);
}

as_object*
getExternalInterfaceInterface()
{
	boost::intrusive_ptr<as_object> o;

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



as_value
externalinterface_addCallback(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_call(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uArgumentsToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uArgumentsToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uAddCallback(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uArrayToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uArrayToJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uArrayToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uCallIn(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uCallOut(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uEscapeXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uEvalJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uInitJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uJsQuoteString(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uObjectID(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uObjectToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uObjectToJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uObjectToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uToAS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uToJS(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uToXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uUnescapeXML(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_available(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}


as_value
externalinterface_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new externalinterface_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("ExternalInterface(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

as_value
externalInterfaceConstructor(const fn_call& fn)
{
    log_debug("Loading flash.external.ExternalInterface class");
    Global_as* gl = getGlobal(fn);
    as_object* proto = getExternalInterfaceInterface();
    as_object* cl = gl->createClass(&externalinterface_ctor, proto);
    attachExternalInterfaceStaticProperties(*cl);
    return cl;
}

}

} // end of gnash namespace
