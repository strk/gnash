// ExternalInterface.cpp:  ActionScript "ExternalInterface" support
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
#include <vector>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <boost/algorithm/string/erase.hpp>
#include <algorithm>

#include "StringPredicates.h"
#include "Relay.h" // for inheritance
#include "ExternalInterface.h"
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "rc.h"
#include "as_value.h"
#include "as_object.h"
#include "xml/XMLDocument_as.h"
#include "Array_as.h"
#include "namedStrings.h"
#include "Global_as.h"
#include "Globals.h"
#include "PropertyList.h"
#include "movie_root.h"
#include "log.h"

using namespace std;

namespace gnash {

/// Class used to serialize properties of an object to a buffer
class PropsSerializer : public AbstractPropertyVisitor
{

public:
    
    PropsSerializer(VM& vm)
        : _st(vm.getStringTable()),
          _error(false)
        { /* do nothing */}
    
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
        _xml << ExternalInterface::toXML(val);
        _xml << "</property>";

        _noprops.push_back(val);
            
        return true;
    }

    std::string getXML() { return _xml.str(); };
    std::vector<as_value> getArgs() { return _noprops; };
    
private:
    string_table&       _st;
    mutable bool        _error;
    std::stringstream   _xml;
    std::vector<as_value>   _noprops;
};

#if 0
class ExternalExecutor: public movie_root::AbstractExternalCallback {
public:
	void notify()
	{
	    log_debug(_("external_callback()"));
	}
};
#endif

/// Convert an AS object to an XML string.
std::string
ExternalInterface::objectToXML(as_object *obj)
{
    // GNASH_REPORT_FUNCTION;
    
    std::stringstream ss;

    if (obj == 0) {
        //log_error("Need a valid AS Object!");
        return ss.str();
    }

    VM& vm = getVM(*obj);
    
    ss << "<object>";
    
    // Get all the properties
    PropsSerializer props(vm);
    obj->visitProperties<IsEnumerable>(props);
    if (!props.success()) {
        log_error("Could not serialize object");
        return false;
    } else {
        std::vector<as_value> properties = props.getArgs();
    }
    ss << "</object>";
    
    return ss.str();
}

/// Convert an AS object to an XML string.
std::string
ExternalInterface::arrayToXML(as_object *obj)
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
ExternalInterface::toXML(const as_value &val)
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
ExternalInterface::toAS(Global_as& /*gl*/, const std::string &xml)
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
ExternalInterface::argumentsToXML(std::vector<as_value> &args)
{
    // GNASH_REPORT_FUNCTION;

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

std::map<std::string, as_value>
ExternalInterface::propertiesToAS(Global_as& gl, std::string &xml)
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
ExternalInterface::objectToAS(Global_as& /*gl*/, const std::string &/*xml*/)
{
    // GNASH_REPORT_FUNCTION;

    return as_value();
}

#if 0
void
ExternalInterface::update()
{
    log_debug(__PRETTY_FUNCTION__);
    
    if (_fd > 0) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(_fd, &fdset);
        struct timeval tval;
        tval.tv_sec  = 0;
        tval.tv_usec = 100;
        errno = 0;
        int ret = ::select(_fd+1, &fdset, NULL, NULL, &tval);
        if (ret == 0) {
//            log_debug ("The pipe for fd #%d timed out waiting to read", _fd);
            return;
        } else if (ret == 1) {
            log_debug ("The pipe for fd #%d is ready", _fd);
        } else {
            log_error("The pipe has this error: %s", strerror(errno));
            return;
        }

        int bytes = 0;
#ifndef _WIN32
        ioctl(_fd, FIONREAD, &bytes);
#else
        ioctlSocket(_fd, FIONREAD, &bytes);
#endif
        log_debug("There are %d bytes in the network buffer", bytes);

        char *buf = new char[bytes+1];
        // Since we know how bytes are in the network buffer, allocate
        // some memory to read the data.
        // terminate incase we want to treat the data like a string.
        buf[bytes+1] = 0;
        ret = ::read(_fd, buf, bytes);
        if (ret) {
            processInvoke(buf);
        }
    }
}

bool
ExternalInterface::call(as_object */*asCallback*/, const std::string& name,
                           const std::vector<as_value>& /*args*/, size_t /*firstArg*/)
{
    GNASH_REPORT_FUNCTION;

    // log_debug("Calling External method \"%s\"", name);

    // as_object *method = getCallback(name);

    
    // call(asCallback, name, args, firstArg);

    // startAdvanceTimer();
    
    return false;
}

// Parse the XML Invoke message, which looks like this:
//
// <invoke name="LoadMovie" returntype="xml">
//      <arguments>
//              <number>2</number>
//              <string>bogus</string>
//      </arguments>
// </invoke>
//
void
ExternalInterface::processInvoke(const std::string &xml)
{
    GNASH_REPORT_FUNCTION;

    if (xml.empty()) {
        return;
    }
    
    std::vector<as_value> args;
    string::size_type start = 0;
    string::size_type end;
    string tag;
    string name;

    // Look for the ending > in the first part of the data for the tag
    end = xml.find(">");
    if (end != std::string::npos) {
        end++;                  // go past the > character
        tag = xml.substr(start, end);
        // Look for the easy ones first
        if (tag.substr(0, 7) == "<invoke") {
            // extract the name of the method to invoke
            start = tag.find("name=") + 5;
            end   = tag.find(" ", start);
            name  = tag.substr(start, end-start);
            // Ignore any quote characters around the string
            boost::erase_first(name, "\"");
            boost::erase_last(name, "\"");

#if 0
            // extract the return type of the method
            start = tag.find("returntype=") + 11;
            end   = tag.find(">", start);
            invoke->type  = tag.substr(start, end-start);
            // Ignore any quote characters around the string
            boost::erase_first(invoke->type, "\"");
            boost::erase_last(invoke->type, "\"");
#endif
            // extract the arguments to the method
            start = xml.find("<arguments>");
            end   = xml.find("</invoke");
            tag   = xml.substr(start, end-start);
            args = ExternalInterface::parseArguments(tag);
        }
    }

#if 0
    // call(as_object* callback, const std::string& name,
    std::map<std::string, as_object *>::const_iterator it;
    for (it=_methods.begin(); it != _methods.end(); it++) {
        log_debug("Method name %s", it->first);
        if (name == it->first) {
            // call(it->second, args, 0);
        }
    }
#endif    
}
#endif

as_value
ExternalInterface::parseXML(const std::string &xml)
{
    GNASH_REPORT_FUNCTION;

    if (xml.empty()) {
        return as_value();
    }

    std::string::size_type start = 0;
    std::string::size_type end;
    std::string tag;
    as_value value;

    // Look for the ending > in the first part of the data for the tag
    end = xml.find(">");
    if (end != std::string::npos) {
        end++;                  // go past the > character
        tag = xml.substr(start, end);
        // Look for the easy ones first
        if (tag == "<null/>") {
            value.set_null();
        } else if (tag == "<void/>") {
            value.set_undefined();
        } else if (tag == "<true/>") {
            value.set_bool(true);
        } else if (tag == "<false/>") {
            value.set_bool(false);
        } else if (tag == "<number>") {
            start = end;
            end = xml.find("</number>");
            std::string str = xml.substr(start, end-start);
            double num = strtod(str.c_str(), NULL);
            value.set_double(num);
        } else if (tag == "<string>") {
            start = end;
            end = xml.find("</string>");
            std::string str = xml.substr(start, end-start);
            value.set_string(str);
        }
    }

    log_debug("Argument is: %s", value.to_string());
    return value;
}

std::vector<as_value>
ExternalInterface::parseArguments(const std::string &xml)
{
    GNASH_REPORT_FUNCTION;

    std::vector<as_value> args;
    std::string::size_type start = 0;
    std::string::size_type end;

    std::string name;
    std::string data = xml;
    std::string tag = "<arguments>";
    start = data.find(tag);
    if (start != std::string::npos) {
        data.erase(0, tag.size());
    }
    while (!data.empty()) {
        // Extract the data
        start = data.find("<", 1); // start past the opening <
        end = data.find(">", start) + 1;
        std::string sub = data.substr(0, end);
        if (data == "</arguments>") {
            break;
        }
        args.push_back(parseXML(sub));
        data.erase(0, end);
    }

    return args;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
