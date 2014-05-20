// ExternalInterface.cpp:  ActionScript "ExternalInterface" support
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "ExternalInterface.h"

#include <map>
#include <vector>
#include <sstream>
#include <boost/algorithm/string/erase.hpp>
#include <algorithm>

#ifdef SOLARIS_HOST
# include <sys/filio.h> // for FIONREAD
#endif

#include "GnashSystemNetHeaders.h"
#include "GnashSystemFDHeaders.h"

#include "StringPredicates.h"
#include "fn_call.h"
#include "Global_as.h"
#include "VM.h"
#include "rc.h"
#include "as_value.h"
#include "as_object.h"
#include "XML_as.h"
#include "Array_as.h"
#include "namedStrings.h"
#include "Global_as.h"
#include "PropertyList.h"
#include "movie_root.h"
#include "log.h"

namespace gnash {

namespace {

class Enumerator : public KeyVisitor
{
public:
    Enumerator(std::vector<ObjectURI>& uris) : _uris(uris) {}
    void operator()(const ObjectURI& u) {
        _uris.push_back(u);
    }
private:
    std::vector<ObjectURI>& _uris;
};

}

/// Convert an AS object to an XML string.
std::string
ExternalInterface::_objectToXML(as_object *obj)
{
    // GNASH_REPORT_FUNCTION;

    if ( ! _visited.insert(obj).second ) { 
        return "<circular/>";
    }
    
    std::stringstream ss;

    ss << "<object>";

    if (obj) {
        // Get all the properties
        VM& vm = getVM(*obj);
        string_table& st = vm.getStringTable();
        typedef std::vector<ObjectURI> URIs;
        URIs uris;
        Enumerator en(uris);
        obj->visitKeys(en);
        for (URIs::const_reverse_iterator i = uris.rbegin(), e = uris.rend();
                i != e; ++i) {
            as_value val = getMember(*obj, *i); 
            const std::string& id = i->toString(st);
            ss << "<property id=\"" << id << "\">";
            ss << _toXML(val);
            ss << "</property>";
        }
    }

    ss << "</object>";
    
    return ss.str();
}

/// Convert an AS object to an XML string.
std::string
ExternalInterface::_toXML(const as_value &val)
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
        ss << (val.to_bool(8) ? "<true/>" : "<false/>");
        // Function also isn't listed, but it's the only other type
        // supported by as_value, so leaving it out doesn't seem right.
    } else if (val.is_function()) {
        ss << "<function>" << val.to_string() << "</function>";
    } else if (val.is_object()) {
        as_object *obj = val.get_object();
        ss << _objectToXML(obj);
    } else {
        log_error(_("Can't convert unknown type %d"), val.to_string());
    }

    return ss.str();
}

std::shared_ptr<ExternalInterface::invoke_t>
ExternalInterface::ExternalEventCheck(int fd)
{
//    GNASH_REPORT_FUNCTION;
    
    std::shared_ptr<ExternalInterface::invoke_t> error;

    if (fd > 0) {
        int bytes = 0;
        ioctlSocket(fd, FIONREAD, &bytes);
        if (bytes == 0) {
            return error;
        }
        log_debug("There are %d bytes in the network buffer", bytes);
        std::unique_ptr<char[]> buffer(new char[bytes + 1]);
        // Since we know how bytes are in the network buffer, allocate
        // some memory to read the data.
        // terminate incase we want to treat the data like a string.
        buffer[bytes] = 0;
        const int ret = ::read(fd, buffer.get(), bytes);
        if (ret > 0) {
            return parseInvoke(std::string(buffer.get(), ret));
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
std::shared_ptr<ExternalInterface::invoke_t>
ExternalInterface::parseInvoke(const std::string &xml)
{
    std::shared_ptr<ExternalInterface::invoke_t> invoke;
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
        // Should we avoid re-serializing the same object ?
        ss << toXML(*it);
    }
    
    ss << "</arguments>";
    ss << "</invoke>";

    // Add a CR on the end so the output is more readable on the other
    // end. XL should be ignoring the CR anyway.
    ss << std::endl;
    
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

    ioctlSocket(fd, FIONREAD, &bytes);

    // No data yet
    if (bytes == 0) {
        return empty;
    }

    log_debug("There are %d bytes in the network buffer", bytes);

    std::string buf(bytes, '\0');

    const int ret = ::read(fd, &buf[0], bytes);
    if (ret <= 0) {
        return empty;
    }

    if (ret < bytes) {
        buf.resize(ret);
    }

    return buf;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
