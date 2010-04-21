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

#include <map>
#include <sstream>

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
#include "Array_as.h"
#include "namedStrings.h"
#include "Global_as.h"
#include "PropertyList.h"

namespace gnash {

namespace {
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
}

/// Class used to serialize properties of an object to a buffer
class PropsSerializer : public AbstractPropertyVisitor
{

public:
    
    PropsSerializer(VM& vm)
        : _st(vm.getStringTable()),
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
        _xml << ExternalInterface_as::toXML(val);
        _xml << "</property>";
            
        return true;
    }

    std::string getXML() { return _xml.str(); };
    
private:
    string_table&       _st;
    mutable bool        _error;
    std::stringstream   _xml;
};

// extern 
void
externalinterface_class_init(as_object& where, const ObjectURI& uri)
{
#if 1
    where.init_destructive_property(uri, externalInterfaceConstructor, 0);
#else
    Global_as& gl = getGlobal(where);
    as_object* proto = gl.createObject();
    as_object* cl = gl.createClass(&externalInterfaceConstructor, proto);

    attachTextFieldInterface(*proto);
    attachTextFieldStaticMembers(*cl);
             
    where.init_member(uri, cl, as_object::DefaultFlags);

    // ASSetPropFlags is called on the TextField class.
    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, cl, null, 131);
#endif
}

namespace {

void
attachExternalInterfaceInterface(as_object& /*o*/)
{
}

void
attachExternalInterfaceStaticProperties(as_object& o)
{
    
//    const int swf6Flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
    const int swf7Flags = as_object::DefaultFlags | PropFlags::onlySWF7Up;
    const int swf8Flags = as_object::DefaultFlags | PropFlags::onlySWF8Up;

    Global_as& gl = getGlobal(o);
    
    // Initialize the properties
    o.init_readonly_property("available", &externalinterface_available, swf7Flags);

    o.init_property("marshallExceptions", externalinterface_marshallExceptions,
                    externalinterface_marshallExceptions, swf8Flags);
    o.init_readonly_property("objectID", &externalinterface_objectID, swf8Flags);
    
    // Initialize the methods, most of which are undocumented helper functions
    o.init_member("addCallback", gl.createFunction(
                externalinterface_addCallback), swf7Flags);
    o.init_member("call", gl.createFunction(externalinterface_call), swf7Flags);
    o.init_member("_argumentsToXML",
                  gl.createFunction(externalinterface_uArgumentsToXML), swf8Flags);
    o.init_member("_argumentsToAS",
                  gl.createFunction(externalinterface_uArgumentsToAS), swf8Flags);
    o.init_member("_addCallback",
                  gl.createFunction(externalinterface_uAddCallback), swf8Flags);
    o.init_member("_arrayToAS",
                  gl.createFunction(externalinterface_uArrayToAS), swf8Flags);
    o.init_member("_arrayToJS",
                  gl.createFunction(externalinterface_uArrayToJS), swf8Flags);
    o.init_member("_arrayToXML",
                  gl.createFunction(externalinterface_uArrayToXML), swf8Flags);
    o.init_member("_callIn",
                  gl.createFunction(externalinterface_uCallIn), swf8Flags);
    o.init_member("_callOut",
                  gl.createFunction(externalinterface_uCallOut), swf8Flags);
    o.init_member("_escapeXML",
                  gl.createFunction(externalinterface_uEscapeXML), swf8Flags);
    o.init_member("_evalJS",
                  gl.createFunction(externalinterface_uEvalJS), swf8Flags);
    o.init_member("_initJS",
                  gl.createFunction(externalinterface_uInitJS), swf8Flags);
    o.init_member("_jsQuoteString",
                  gl.createFunction(externalinterface_uJsQuoteString), swf8Flags);
    o.init_member("_objectID",
                  gl.createFunction(externalinterface_uObjectID), swf8Flags);
    o.init_member("_objectToAS",
                  gl.createFunction(externalinterface_uObjectToAS), swf8Flags);
    o.init_member("_objectToJS",
                  gl.createFunction(externalinterface_uObjectToJS), swf8Flags);
    o.init_member("_objectToXML",
                  gl.createFunction(externalinterface_uObjectToXML), swf8Flags);
    o.init_member("_toAS",
                  gl.createFunction(externalinterface_uToAS), swf8Flags);
    o.init_member("_toJS",
                  gl.createFunction(externalinterface_uToJS), swf8Flags);
    o.init_member("_toXML",
                  gl.createFunction(externalinterface_uToXML), swf8Flags);
    o.init_member("_unescapeXML",
                  gl.createFunction(externalinterface_uUnescapeXML), swf8Flags);

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
    const std::string &str = ptr->objectID();
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
    
    if (fn.nargs > 0) {
        std::vector<as_value> args;
        for (size_t i=0; i<fn.nargs; i++) {
            args.push_back(fn.arg(i));
        }
        return ExternalInterface_as::argumentsToXML(args);
    }
    
    return as_value();
}

as_value
externalinterface_uArgumentsToAS(const fn_call& /*fn*/)
{
    // GNASH_REPORT_FUNCTION;
    
    // std::string str(fn.arg(0).to_string());
    // ExternalInterface_as &ptr = (ExternalInterface_as &)(fn);
    // if (fn.nargs > 0) {
    //     return ptr->argumentsToAS();
    // }

    return as_value();
}

as_value
externalinterface_uAddCallback(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uArrayToAS(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uArrayToJS(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uArrayToXML(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    if (fn.nargs == 1) {
        as_object *obj = fn.arg(0).to_object(getGlobal(fn));
        std::string str = ExternalInterface_as::arrayToXML(obj);
        return as_value(str);
    }
    
    return as_value();
}

as_value
externalinterface_uCallIn(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uCallOut(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uEvalJS(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uInitJS(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uJsQuoteString(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uObjectID(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uObjectToAS(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    if (fn.nargs == 1) {
        return ExternalInterface_as::objectToAS(getGlobal(fn), fn.arg(0).to_string());
    }
    
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
    
    if (fn.nargs == 1) {
        if (!fn.arg(0).is_null() && !fn.arg(0).is_undefined()) {
            as_object *obj = fn.arg(0).to_object(getGlobal(fn));
            std::string str = ExternalInterface_as::objectToXML(obj);
            return as_value(str);
        } else {
            return as_value("<object></object>");
        }
    }
    
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

    if (fn.nargs == 1) {
        as_value val = fn.arg(0);
        as_value(ExternalInterface_as::toXML(val));
    }
    
    return as_value();
}

as_value
externalinterface_uToAS(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    if (fn.nargs == 1) {
        as_value val = ExternalInterface_as::toAS(getGlobal(fn), fn.arg(0).to_string());
        return val;
    }
    
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

} // end of anonymous namespace used for callbacks

// namespace gnash {

ExternalInterface_as::ExternalInterface_as(as_object* /*owner*/)
    : _exceptions(false)

{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
}

ExternalInterface_as::~ExternalInterface_as()
{
//    LOG_ONCE( log_unimpl (__FUNCTION__) );
}

bool
ExternalInterface_as::addCallback(const std::string &name, as_object *method)
{
    // GNASH_REPORT_FUNCTION;
    _methods[name] = method;
    
    return true;
}

bool
ExternalInterface_as::call(as_object */*asCallback*/, const std::string& /*name*/,
                           const std::vector<as_value>& /*args*/, size_t /*firstArg*/)
{
    // GNASH_REPORT_FUNCTION;
    LOG_ONCE( log_unimpl (__FUNCTION__) );

    return false;
}

/// Convert an AS object to an XML string.
std::string
ExternalInterface_as::objectToXML(as_object *obj)
{
    // GNASH_REPORT_FUNCTION;
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
    PropsSerializer props(vm);
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
    // GNASH_REPORT_FUNCTION;
    std::stringstream ss;
    if (obj == 0) {
        //log_error("Need a valid AS Object!");
        return ss.str();
    }

    VM& vm = getVM(*obj);    
    
    ss << "<array>";
    PropsSerializer props(vm);
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
ExternalInterface_as::toXML(const as_value &val)
{
    // GNASH_REPORT_FUNCTION;
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
ExternalInterface_as::toAS(Global_as& /*gl*/, const std::string &xml)
{
    // GNASH_REPORT_FUNCTION;

    std::string::size_type start = 0;
    std::string::size_type end;
    std::string tag;
    as_value val;
    
    // Look for the ending > in the first part of the data for the tag
    end = xml.find(">");
    if (end != std::string::npos) {
        end++;                  // go past the > character
        tag = xml.substr(start, end);
        // Look for the easy ones first
        if (tag == "<null/>") {
            val.set_null();
        } else if (tag == "<void/>") {
            val.set_null();     // FIXME: we need a void type in as_value
        } else if (tag == "<true/>") {
            val.set_bool(true);
        } else if (tag == "<false/>") {
            val.set_bool(false);
        } else if (tag == "<number>") {
            start = end;
            end = xml.find("</number>");
            std::string str = xml.substr(start, end-start);
            if (str.find(".") != std::string::npos) {
                double num = strtod(str.c_str(), NULL);
                val.set_double(num);
            } else {
                int num = strtol(str.c_str(), NULL, 0);
                val.set_double(num);
            }
        } else if (tag == "<string>") {
            start = end;
            end = xml.find("</string>");
            std::string str = xml.substr(start, end-start);
            int length = str.size();;
            char *data = new char[length+1];
            std::copy(str.begin(), str.end(), data);
            data[length] = 0;  // terminate the new string or bad things happen
            // When an NPVariant becomes a string object, it *does not* make a copy.
            // Instead it stores the pointer (and length) we just allocated.
            val.set_string(data);
        } else if (tag == "<array>") {
            start = end;
            end = xml.find("</array");
            std::string str = xml.substr(start, end-start);
            // std::map<std::string, NPVariant *> props = parseProperties(str);
            // std::map<std::string, NPVariant *>::iterator it;
            // for (it=props.begin(); it != props.end(); ++it) {
            //     // NPIdentifier id = NPN_GetStringIdentifier(it->first.c_str());
            //     // NPVariant *value = it->second;
            // }
            // as_object *obj = new as_object(gl);
        } else if (tag == "<object>") {
            start = end;
            end = xml.find("</object");
            std::string str = xml.substr(start, end-start);
            // std::map<std::string, as_value> props = parseProperties(str);
            // std::map<std::string, NPVariant *>::iterator it;
            // for (it=props.begin(); it != props.end(); ++it) {
            //     // NPIdentifier id = NPN_GetStringIdentifier(it->first.c_str());
            //     // NPVariant *value = it->second;
            // }
            // as_object *obj = val.to_object();
            // val.set_as_object(obj); 
            // as_object *obj = new as_object(gl);
        }
    }

    return val;
}

as_value
ExternalInterface_as::argumentsToXML(std::vector<as_value> &args)
{
    // GNASH_REPORT_FUNCTION;
    std::vector<as_value>::iterator it;
    std::stringstream ss;

    ss << "<arguments>";
    for (it=args.begin(); it != args.end(); it++) {
        as_value val = *it;
        // ss << externalinterface_uToXML(val);
    }
    ss << "</arguments>";
    
    return as_value(ss.str());
}

std::map<std::string, as_value>
ExternalInterface_as::propertiesToAS(Global_as& gl, std::string &xml)
{
    // GNASH_REPORT_FUNCTION;
    std::map<std::string, as_value> props;

    std::string::size_type start = 0;
    std::string::size_type end;

    std::string id;
    start = xml.find(" id=");
    while (start != std::string::npos) {
        // Extract the id from the property tag
        start++;
        end = xml.find(">", start) - 1;
        id = xml.substr(start, end-start);
        id.erase(0, 4);

        // Extract the data
        start = end + 2;
        end = xml.find("</property>", start) ;
        std::string data = xml.substr(start, end-start);
        props[id] = toAS(gl, data);
        start = xml.find(" id=", end);
    }

    return props;
}

as_value
ExternalInterface_as::objectToAS(Global_as& /*gl*/, const std::string &/*xml*/)
{
    // GNASH_REPORT_FUNCTION;

    return as_value();
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
