// ExternalInterface_as.cpp:  ActionScript "ExternalInterface" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "Relay.h" // for inheritance
#include "ExternalInterface_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "as_value.h"
#include "as_object.h"
#include "xml/XMLDocument_as.h"
#include "namedStrings.h"
#include "PropertyList.h"

#include <sstream>

namespace gnash {

namespace {

/// Class used to serialize properties of an object to a buffer
class PropsSerializer : public AbstractPropertyVisitor
{

public:
    
    PropsSerializer(ExternalInterface_as &ei, VM& vm)
        : _ei(ei),
        _st(vm.getStringTable()),
        _error(false)
    {}
    
    bool success() const { return !_error; }

    bool accept(const ObjectURI& uri, const as_value& val) {
        if (_error) return true;

        const string_table::key key = getName(uri);

        if (key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR) {
            log_debug(" skip serialization of specially-named property %s",
                      _st.value(key));
            return true;
        }

        // write property name
        const std::string& id = _st.value(key);

//        log_debug(" serializing property %s", id);
        
        _xml << "<property id=\"" << id << "\">";
        _xml << _ei.toXML(const_cast<as_value &>(val));
        _xml << "</property>";
            
        return true;
    }

    std::string getXML() { return _xml.str(); };
    
private:
    ExternalInterface_as &_ei;
    string_table&       _st;
    mutable bool        _error;
    std::stringstream   _xml;
};
}

as_value externalinterface_addCallback(const fn_call& fn);
as_value externalinterface_call(const fn_call& fn);
as_value externalInterfaceConstructor(const fn_call& fn);
as_value externalinterface_available(const fn_call& fn);
as_value externalinterface_marshallExceptions(const fn_call& fn);
as_value externalinterface_objectID(const fn_call& fn);

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

// extern 
void
externalinterface_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, externalInterfaceConstructor, flags);
}

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

    Global_as& gl = getGlobal(o);
    
    // Initialize the properties
    o.init_readonly_property("available", &externalinterface_available);

    // FIXME: for now, always make these available as ming doesn't support v9
//    if (getSWFVersion(o) > 8) {
    o.init_readonly_property("marshallExceptions", &externalinterface_marshallExceptions);
    o.init_property("objectID", externalinterface_objectID, externalinterface_objectID);
//    }
    
    // Initialize the methods, most of which are undocumented helper functions
    o.init_member("addCallback", gl.createFunction(
                externalinterface_addCallback), flags);
    o.init_member("call", gl.createFunction(externalinterface_call), flags);

    if (getSWFVersion(o) > 6) {
        o.init_member("_argumentsToXML",
                      gl.createFunction(externalinterface_uArgumentsToXML), flags);
        o.init_member("_argumentsToAS",
                      gl.createFunction(externalinterface_uArgumentsToAS), flags);
        o.init_member("_addCallback",
                      gl.createFunction(externalinterface_uAddCallback), flags);
        o.init_member("_arrayToAS",
                      gl.createFunction(externalinterface_uArrayToAS), flags);
        o.init_member("_arrayToJS",
                      gl.createFunction(externalinterface_uArrayToJS), flags);
        o.init_member("_arrayToXML",
                      gl.createFunction(externalinterface_uArrayToXML), flags);
        o.init_member("_callIn",
                      gl.createFunction(externalinterface_uCallIn), flags);
        o.init_member("_callOut",
                      gl.createFunction(externalinterface_uCallOut), flags);
        o.init_member("_escapeXML",
                      gl.createFunction(externalinterface_uEscapeXML), flags);
        o.init_member("_evalJS",
                      gl.createFunction(externalinterface_uEvalJS), flags);
        o.init_member("_initJS",
                      gl.createFunction(externalinterface_uInitJS), flags);
        o.init_member("_jsQuoteString",
                      gl.createFunction(externalinterface_uJsQuoteString), flags);
        o.init_member("_objectID",
                      gl.createFunction(externalinterface_uObjectID), flags);
        o.init_member("_objectToAS",
                      gl.createFunction(externalinterface_uObjectToAS), flags);
        o.init_member("_objectToJS",
                      gl.createFunction(externalinterface_uObjectToJS), flags);
        o.init_member("_objectToXML",
                      gl.createFunction(externalinterface_uObjectToXML), flags);
        o.init_member("_toAS",
                      gl.createFunction(externalinterface_uToAS), flags);
        o.init_member("_toJS",
                      gl.createFunction(externalinterface_uToJS), flags);
        o.init_member("_toXML",
                      gl.createFunction(externalinterface_uToXML), flags);
        o.init_member("_unescapeXML",
                      gl.createFunction(externalinterface_uUnescapeXML), flags);
    }
    
}

as_value
externalinterface_addCallback(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    ExternalInterface_as* ptr = ensure<ThisIsNative<ExternalInterface_as> >(fn);

    if (fn.nargs > 1) {
        const as_value& methodName_as = fn.arg(0);
        std::string methodName = methodName_as.to_string();
        boost::intrusive_ptr<as_object> asCallback;
        if (fn.arg(1).is_object()) {
            asCallback = (fn.arg(1).to_object(getGlobal(fn)));
        }
        
        ptr->addCallback(methodName, asCallback.get());
    }
    
    
    return as_value();    
}

as_value
externalinterface_call(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    ExternalInterface_as* ptr = ensure<ThisIsNative<ExternalInterface_as> >(fn);
    if (fn.nargs > 3) {
        const as_value& methodName_as = fn.arg(0);
        std::string methodName = methodName_as.to_string();
        boost::intrusive_ptr<as_object> asCallback;
        if (fn.arg(1).is_object()) {
            asCallback = (fn.arg(1).to_object(getGlobal(fn)));
        }
        
        const std::vector<as_value>& args = fn.getArgs();
        ptr->call(asCallback.get(), methodName, args, 2);
    }
    
    return as_value();
}

as_value
externalinterface_available(const fn_call& /* fn */)
{
//    GNASH_REPORT_FUNCTION;
    
    // Yes, Gnash supports the ExternalInterface
    return as_value(true);
}

as_value
externalinterface_marshallExceptions(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    // No, don't pass exception up to the broswer
    if (fn.nargs) {
        ExternalInterface_as* ptr = ensure<ThisIsNative<ExternalInterface_as> >(fn);
        ptr->marshallExceptions(fn.arg(0).to_bool());
    } else {
        return as_value(true);
    }
    
    return as_value(true);
}

as_value
externalinterface_objectID(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    ExternalInterface_as* ptr = ensure<ThisIsNative<ExternalInterface_as> >(fn);
    std::string &str = ptr->objectID();
    if (str.empty()) {
        return as_value();
    }

    return as_value(str);
}

as_value
externalinterface_ctor(const fn_call& fn)
{
    if (fn.nargs) {
        std::stringstream ss;
        fn.dump_args(ss);
        LOG_ONCE(log_unimpl("ExternalInterface(%s): %s", ss.str(),
                            _("arguments discarded")) );
    }
    
    return as_value(); 
}

as_value
externalInterfaceConstructor(const fn_call& fn)
{
    log_debug("Loading flash.external.ExternalInterface class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = gl.createObject();
    as_object* cl = gl.createClass(&externalinterface_ctor, proto);

    attachExternalInterfaceInterface(*proto);
    attachExternalInterfaceStaticProperties(*cl);
    return cl;
}

as_value
externalinterface_uArgumentsToXML(const fn_call& fn)
{
    // GNASH_REPORT_FUNCTION;

    std::stringstream ss;
    ExternalInterface_as &ptr = (ExternalInterface_as &)(fn);
    
    if (fn.nargs > 0) {
        std::vector<as_value> args;
        for (size_t i=0; i<fn.nargs; i++) {
            args.push_back(fn.arg(i));
        }
        return ptr.argumentsToXML(args);
    }
    
    return as_value();
}

as_value
externalinterface_uArgumentsToAS(const fn_call& fn)
{
    // GNASH_REPORT_FUNCTION;
    
    std::string str(fn.arg(0).to_string());
    escapeXML(str);

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
externalinterface_uArrayToXML(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    ExternalInterface_as &ptr = (ExternalInterface_as &)(fn);
    if (fn.nargs == 1) {
        as_object *obj = fn.arg(0).to_object(getGlobal(fn));
        std::string str = ptr.arrayToXML(obj);
        return as_value(str);
    }
    
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
externalinterface_uObjectToXML(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    ExternalInterface_as &ptr = (ExternalInterface_as &)(fn);
    if (fn.nargs == 1) {
        if (!fn.arg(0).is_null() && !fn.arg(0).is_undefined()) {
            as_object *obj = fn.arg(0).to_object(getGlobal(fn));
            std::string str = ptr.objectToXML(obj);
            return as_value(str);
        } else {
            return "<object></object>";
        }
    }
    
//    const string_table::key key = getName(uri);    
    
    return as_value();
}

as_value
externalinterface_uToJS(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uToXML(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    ExternalInterface_as &ptr = (ExternalInterface_as &)(fn);
    
    if (fn.nargs == 1) {
        as_value val = fn.arg(0);
        std::string str = ptr.toXML(val);
        return as_value(str);
    }
    
    return as_value();
}

as_value
externalinterface_uToAS(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uEscapeXML(const fn_call& fn)
{
    // GNASH_REPORT_FUNCTION;
    
    if (fn.nargs == 1) {
        std::string str(fn.arg(0).to_string());
        escapeXML(str);
        return as_value(str);
    }
    
    return as_value();
}

as_value
externalinterface_uUnescapeXML(const fn_call& fn)
{
    // GNASH_REPORT_FUNCTION;

    if (fn.nargs == 1) {
        std::string str = fn.arg(0).to_string();
        gnash::unescapeXML(str);
        return as_value(str);
    }
    
    return as_value();
}

// } // end of anonymous namespace used for callbacks

// namespace gnash {

ExternalInterface_as::ExternalInterface_as(as_object* owner)
    : ActiveRelay(owner),
      _exceptions(false)

{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
}

ExternalInterface_as::~ExternalInterface_as()
{
//    LOG_ONCE( log_unimpl (__FUNCTION__) );
}

bool
ExternalInterface_as::addCallback(const std::string &/*name */, as_object */* method */)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );

    return false;
}

bool
ExternalInterface_as::call(as_object */*asCallback*/, const std::string& /*methodName*/,
                           const std::vector<as_value>& /*args*/, size_t /*firstArg*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );

    return false;
}

/// Convert an AS object to an XML string.
std::string
ExternalInterface_as::objectToXML(as_object *obj)
{
    std::stringstream ss;

    if (obj == 0) {
        //log_error("Need a valid AS Object!");
        return ss.str();
    }

    VM& vm = getVM(*obj);
    
    ss << "<object>";
    
    // FIXME: figure out why accessing properties of a native
    // class is different.
    if (obj->relay()) {
        log_error("%s: native objects barely supported!", __FUNCTION__);
    }

    // Get all the properties
    PropsSerializer props(*this, vm);
    obj->visitProperties<IsEnumerable>(props);
    if (!props.success()) {
        log_error("Could not serialize object");
        return false;
    } else {
        ss << props.getXML();
    }
    ss << "</object>";
    
    return ss.str();
}

/// Convert an AS object to an XML string.
std::string
ExternalInterface_as::arrayToXML(as_object *obj)
{
    std::stringstream ss;
    if (obj == 0) {
        //log_error("Need a valid AS Object!");
        return ss.str();
    }

    VM& vm = getVM(*obj);    
    
    ss << "<array>";
    PropsSerializer props(*this, vm);
    obj->visitProperties<IsEnumerable>(props);
    if (!props.success()) {
        log_error("Could not serialize object");
        return false;
    }
    ss << props.getXML();
    
    ss << "</array>";
    
    return ss.str();
}

/// Convert an AS object to an XML string.
std::string
ExternalInterface_as::toXML(as_value &val)
{
    std::stringstream ss;
    
    if (val.is_string()) {
        ss << "<string>" << val.to_string() << "</string>";
    } else if (val.is_number()) {
        ss << "<number>" << val.to_string() << "</number>";
    } else if (val.is_undefined()) {
        ss << "<undefined/>";
    } else if (val.is_null()) {
        ss << "<null/>";
        // Exception isn't listed in any docs, but we'll use it for
        // marshallExceptions.
    } else if (val.is_exception()) {
        ss << "<exception>" << val.to_string()<< "</exception>";
    } else if (val.is_bool()) {
        ss << (val.to_bool() ? "<true/>" : "<false/>");
        // Function also isn't listed, but it's the only other type
        // supported by as_value, so leaving it out doesn't seem right.
    } else if (val.is_function()) {
        ss << "<function>" << val.to_string() << "</function>";
    } else if (val.is_object()) {
//        as_object *obj = (as_object *)&val;
//         ss << "<object></object>";
    } else {
        log_error("Can't convert unknown type %d", val.to_string());
    }

    return ss.str();
}

/// Convert an XML string to an AS object.
as_value
ExternalInterface_as::toAS(const std::string &xml)
{
    return as_value();
}

as_value
ExternalInterface_as::argumentsToXML(std::vector<as_value> &args)
{
    std::vector<as_value>::iterator it;
    std::stringstream ss;

    ss << "<arguments>";
    for (it=args.begin(); it != args.end(); it++) {
        as_value val = *it;
        ss << toXML(val);
    }
    ss << "</arguments>";
    
    return as_value(ss.str());
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
