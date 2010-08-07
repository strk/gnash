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
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>

#include "StringPredicates.h"
#include "ExternalInterface.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "VM.h"
#include "rc.h"
#include "as_value.h"
#include "as_object.h"
#include "XML_as.h"
#include "Array_as.h"
#include "namedStrings.h"
#include "Global_as.h"
#include "Globals.h"
#include "PropertyList.h"
#include "movie_root.h"
#include "log.h"

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
            // as_value copies the string, so we don't need data anymore
            delete[] data;
        } else if (tag == "<array>") {
            start = end;
            end = xml.find("</array");
            std::string str = xml.substr(start, end-start);
            log_unimpl("array processing for ExternalInterface");
        } else if (tag == "<object>") {
            start = end;
            end = xml.find("</object");
            std::string str = xml.substr(start, end-start);
            log_unimpl("object processing for ExternalInterface");
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

boost::shared_ptr<ExternalInterface::invoke_t>
ExternalInterface::ExternalEventCheck(int fd)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::shared_ptr<ExternalInterface::invoke_t> error;

    if (fd > 0) {
#if 0
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        struct timeval tval;
        tval.tv_sec  = 0;
        tval.tv_usec = 100;
        errno = 0;
        int ret1 = ::select(fd+1, &fdset, NULL, NULL, &tval);
        if (ret1 == 0) {
//            log_debug ("The pipe for fd #%d timed out waiting to read", fd);
            return error;
        } else if (ret1 == 1) {
            log_debug ("The pipe for fd #%d is ready", fd);
        } else {
            log_error("The pipe has this error: %s", strerror(errno));
            return error;
        }
#endif
        
        int bytes = 0;
#ifndef _WIN32
        ioctl(fd, FIONREAD, &bytes);
#else
        ioctlSocket(fd, FIONREAD, &bytes);
#endif
        if (bytes == 0) {
            return error;
        }
        log_debug("There are %d bytes in the network buffer", bytes);
        boost::scoped_array<char> buffer(new char[bytes+1]);
        // Since we know how bytes are in the network buffer, allocate
        // some memory to read the data.
        // terminate incase we want to treat the data like a string.
        buffer[bytes+1] = 0;
        int ret = ::read(fd, buffer.get(), bytes);
        if (ret) {
            return parseInvoke(buffer.get());
        }
    }

    return error;
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
boost::shared_ptr<ExternalInterface::invoke_t>
ExternalInterface::parseInvoke(const std::string &xml)
{
    //    GNASH_REPORT_FUNCTION;

    boost::shared_ptr<ExternalInterface::invoke_t> invoke;
    if (xml.empty()) {
        return invoke;
    }
    
    invoke.reset(new ExternalInterface::invoke_t);
    std::string::size_type start = 0;
    std::string::size_type end;
    std::string tag;

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
            invoke->name  = tag.substr(start, end-start);
            // Ignore any quote characters around the string
            boost::erase_first(invoke->name, "\"");
            boost::erase_last(invoke->name, "\"");

            // extract the return type of the method
            start = tag.find("returntype=") + 11;
            end   = tag.find(">", start);
            invoke->type  = tag.substr(start, end-start);
            // Ignore any quote characters around the string
            boost::erase_first(invoke->type, "\"");
            boost::erase_last(invoke->type, "\"");

            // extract the arguments to the method
            start = xml.find("<arguments>");
            end   = xml.find("</invoke");
            tag   = xml.substr(start, end-start);
            invoke->args = ExternalInterface::parseArguments(tag);
        }
    }

    return invoke;
}

as_value
ExternalInterface::parseXML(const std::string &xml)
{
    //    GNASH_REPORT_FUNCTION;

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

//    log_debug("Argument is: %s", value.to_string());
    return value;
}

std::vector<as_value>
ExternalInterface::parseArguments(const std::string &xml)
{
    // GNASH_REPORT_FUNCTION;

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

// Create an Invoke message for the standalone Gnash
std::string
ExternalInterface::makeInvoke (const std::string &method,
                               const std::vector<as_value> &args)
{
    std::stringstream ss;
    std::vector<as_value>::const_iterator it;

    ss << "<invoke name=\"" << method << "\" returntype=\"xml\">";
    ss << "<arguments>";
    for (it=args.begin(); it != args.end(); ++it) {
        ss << ExternalInterface::toXML(*it);
    }
    
    ss << "</arguments>";
    ss << "</invoke>";

    // Add a CR on the end so the output is more readable on the other
    // end. XL should be ignoring the CR anyway.
    ss << std::endl;
    
    return ss.str();
}

std::string
ExternalInterface::makeNull ()
{
    return "<null/>";
}

std::string
ExternalInterface::makeTrue ()
{
    return "<true/>";
}

std::string
ExternalInterface::makeFalse ()
{
    return "<false/>";
}

std::string
ExternalInterface::makeString (const std::string &str)
{
    std::stringstream ss;

    ss << "<string>" << str << "</string>";
    
    return ss.str();
}


std::string
ExternalInterface::makeProperty (const std::string &id, double num)
{
    std::stringstream ss;
    ss << num;
    return makeProperty(id, ss.str());
}

std::string
ExternalInterface::makeProperty (const std::string &id, int num)
{
    std::stringstream ss;
    ss << num;
    return makeProperty(id, ss.str());
}

std::string
ExternalInterface::makeProperty (const std::string &id, const std::string &data)
{
    std::stringstream ss;

    ss << "<property id=\"" << id << "\">" << data << "</property>";
    
    return ss.str();
}

std::string
ExternalInterface::makeNumber (double num)
{
    std::stringstream ss;

    ss << "<number>" << num << "</number>";
    
    return ss.str();
}

std::string
ExternalInterface::makeNumber (int num)
{
    std::stringstream ss;

    ss << "<number>" << num << "</number>";
    
    return ss.str();
}

std::string
ExternalInterface::makeNumber (unsigned int num)
{
    std::stringstream ss;
    
    ss << "<number>" << num << "</number>";

    return ss.str();
}

std::string
ExternalInterface::makeArray (std::vector<std::string> &args)
{
    std::stringstream ss;
    std::vector<std::string>::iterator it;
    int index = 0;
    
    ss << "<array>";
    for (it=args.begin(); it != args.end(); ++it) {
        ss << "<property id=\"" << index << "\">" << *it << "</property>";
        index++;
    }
    
    ss << "</array>";
    
    return ss.str();
}

std::string
ExternalInterface::makeObject (std::map<std::string, std::string> &args)
{
    std::stringstream ss;
    std::map<std::string, std::string>::iterator it;

    ss << "<object>";
    for (it = args.begin(); it != args.end(); ++it) {
        ss << "<property id=\"" << it->first << "\">" << it->second << "</property>";
    }
    ss << "</object>";
    
    return ss.str();
}

size_t
ExternalInterface::writeBrowser(int fd, const std::string &data)
{
    if (fd > 0) {
        return ::write(fd, data.c_str(), data.size());
    }

    return -1;
}

std::string
ExternalInterface::readBrowser(int fd)
{
    std::string empty;
    // Wait for some data from the player
    int bytes = 0;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    struct timeval tval;
    tval.tv_sec = 10;
    tval.tv_usec = 0;
    // log_debug("Waiting for data... ");
    if (select(fd+1, &fdset, NULL, NULL, &tval)) {
        // log_debug("There is data in the network");
#ifndef _WIN32
        ioctl(fd, FIONREAD, &bytes);
#else
        ioctlSocket(fd, FIONREAD, &bytes);
#endif
    }  

    // No data yet
    if (bytes == 0) {
        return empty;
    }

    log_debug("There are %d bytes in the network buffer", bytes);

    std::string buf(bytes, '\0');

    int ret = ::read(fd, &buf[0], bytes);
    if (ret <= 0) {
        return empty;
    }

    if (ret < bytes) {
        buf.resize(ret);
    }

    std::cout << buf << std::endl;
    
    return buf;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
