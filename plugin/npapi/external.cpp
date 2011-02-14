// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#include <boost/algorithm/string/erase.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>

#include "npapi.h"
#include "npruntime.h"
#include "external.h"
#include "plugin.h"

namespace gnash {
namespace plugin {

// Create an Invoke message for the standalone Gnash
std::string
ExternalInterface::makeInvoke (const std::string &method,
                               std::vector<std::string> args)
{
    std::stringstream ss;
    std::vector<std::string>::iterator it;

    ss << "<invoke name=\"" << method << "\" returntype=\"xml\">";
    ss << "<arguments>";
    for (it=args.begin(); it != args.end(); ++it) {
        ss << *it;
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
    std::stringstream ss;
    
    ss << "<null/>";
    
    return ss.str();
}

std::string
ExternalInterface::makeTrue ()
{
    std::stringstream ss;

    ss << "<true/>";
    
    return ss.str();
}

std::string
ExternalInterface::makeFalse ()
{
    std::stringstream ss;
    
    ss << "<false/>";

    return ss.str();
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

// An invoke looks like this:
// <invoke name=\"foobar\" returntype=\"xml\">
//      <arguments>
//              <string>barfoo</string>
//              <number>135.78</number>
//      </arguments>
// </invoke>
boost::shared_ptr<ExternalInterface::invoke_t>
ExternalInterface::parseInvoke(const std::string &xml)
{
    boost::shared_ptr<ExternalInterface::invoke_t> invoke;
    if (xml.empty()) {
        return invoke;
    }

    invoke.reset(new invoke_t);
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
            start = tag.find("name=");
            if ( start == std::string::npos ) {
                return invoke;
            }
            start += 5;
            end   = tag.find(" ", start);
            if ( end == std::string::npos ) {
                return invoke;
            }
            invoke->name  = tag.substr(start, end-start);
            // Ignore any quote characters around the string
            boost::erase_first(invoke->name, "\"");
            boost::erase_last(invoke->name, "\"");
            
            // extract the return type of the method
            start = tag.find("returntype=");
            if ( start == std::string::npos ) {
                return invoke;
            }
            start += 11;
            end   = tag.find(">", start);
            if ( end == std::string::npos ) {
                return invoke;
            }
            invoke->type  = tag.substr(start, end-start);
            // Ignore any quote characters around the string
            boost::erase_first(invoke->type, "\"");
            boost::erase_last(invoke->type, "\"");
            
            // extract the arguments to the method
            start = xml.find("<arguments>");
            end   = xml.find("</invoke");
            if (start != std::string::npos && end != std::string::npos) {
                    tag   = xml.substr(start, end-start);
                    invoke->args = parseArguments(tag);
                }
        }
    }
    
    return invoke;
}

GnashNPVariant 
ExternalInterface::parseXML(const std::string &xml)
{
    NPVariant value;
    NULL_TO_NPVARIANT(value);
    
    if (xml.empty()) {
        return value;
    }
    std::string::size_type start = 0;
    std::string::size_type end;
    std::string tag;

    // Look for the ending > in the first part of the data for the tag
    end = xml.find(">");
    if (end != std::string::npos) {
        end++;                  // go past the > character
        tag = xml.substr(start, end);
        // Look for the easy ones first
        if (tag == "<null/>") {
            NULL_TO_NPVARIANT(value);
        } else if (tag == "<void/>") {
            VOID_TO_NPVARIANT(value);
        } else if (tag == "<true/>") {
            BOOLEAN_TO_NPVARIANT(true, value);
        } else if (tag == "<false/>") {
            BOOLEAN_TO_NPVARIANT(false, value);
        } else if (tag == "<number>") {
            start = end;
            end = xml.find("</number>");
            std::string str = xml.substr(start, end-start);
            if (str.find(".") != std::string::npos) {
                double num = strtod(str.c_str(), NULL);
                DOUBLE_TO_NPVARIANT(num, value);
            } else {
                int num = strtol(str.c_str(), NULL, 0);
                INT32_TO_NPVARIANT(num, value);
            }
        } else if (tag == "<string>") {
            start = end;
            end = xml.find("</string>");
            std::string str = xml.substr(start, end-start);
            int length = str.size();;
            char *data = (char *)NPN_MemAlloc(length+1);
            std::copy(str.begin(), str.end(), data);
            data[length] = 0;  // terminate the new string or bad things happen
            // When an NPVariant becomes a string object, it *does not* make a copy.
            // Instead it stores the pointer (and length) we just allocated.
            STRINGN_TO_NPVARIANT(data, length, value);
        } else if (tag == "<array>") {
            NPObject *obj =  (NPObject *)NPN_MemAlloc(sizeof(NPObject));
            obj->referenceCount = 1;
            start = end;
            end = xml.find("</array");
            if ( end != std::string::npos )  {
              std::string str = xml.substr(start, end-start);
              std::map<std::string, GnashNPVariant> props = parseProperties(str);
              std::map<std::string, GnashNPVariant>::iterator it;
              for (it=props.begin(); it != props.end(); ++it) {
                  NPIdentifier id = NPN_GetStringIdentifier(it->first.c_str());
                  GnashNPVariant& value = it->second;
                  NPN_SetProperty(NULL, obj, id, &value.get());
              }
              OBJECT_TO_NPVARIANT(obj, value);
            }
        } else if (tag == "<object>") {
            start = end;
            end = xml.find("</object");
            if ( end != std::string::npos )  {
              NPObject *obj =  (NPObject *)NPN_MemAlloc(sizeof(NPObject));
              obj->referenceCount = 1;
              std::string str = xml.substr(start, end-start);
              std::map<std::string, GnashNPVariant> props = parseProperties(str);
              std::map<std::string, GnashNPVariant>::iterator it;
              for (it=props.begin(); it != props.end(); ++it) {
                  NPIdentifier id = NPN_GetStringIdentifier(it->first.c_str());
                  GnashNPVariant& value = it->second;
                  NPN_SetProperty(NULL, obj, id, &value.get());
              }
              OBJECT_TO_NPVARIANT(obj, value);
            }
        }
    }
    
    GnashNPVariant rv(value);
    NPN_ReleaseVariantValue(&value);
    return rv;
}

std::string
ExternalInterface::convertNPVariant (const NPVariant *value)
{
    std::stringstream ss;
    
    if (NPVARIANT_IS_DOUBLE(*value)) {
        double num = NPVARIANT_TO_DOUBLE(*value);
        ss << "<number>" << num << "</number>";
    } else if (NPVARIANT_IS_STRING(*value)) {
        std::string str = NPStringToString(NPVARIANT_TO_STRING(*value));
        ss << "<string>" << str << "</string>";
    } else if (NPVARIANT_IS_BOOLEAN(*value)) {
        bool flag = NPVARIANT_TO_BOOLEAN(*value);
        if (flag) {
            ss << "<true/>";
        } else {
            ss << "<false/>";
        }
    } else if (NPVARIANT_IS_INT32(*value)) {
        int num = NPVARIANT_TO_INT32(*value);
        ss << "<number>" << num << "</number>";
    } else if (NPVARIANT_IS_NULL(*value)) {
        ss << "<null/>";
    } else if (NPVARIANT_IS_VOID(*value)) {
        ss << "<void/>";
    } else if (NPVARIANT_IS_OBJECT(*value)) {
        ss << "<object></object>";
    }    
    
    return ss.str();
}

std::map<std::string, GnashNPVariant>
ExternalInterface::parseProperties(const std::string &xml)
{
    std::map<std::string, GnashNPVariant> props;

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
        props[id] = parseXML(data);
        start = xml.find(" id=", end);
    }

    return props;
}

std::vector<GnashNPVariant>
ExternalInterface::parseArguments(const std::string &xml)
{
    std::vector<GnashNPVariant> args;

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
        if (start == std::string::npos ) break;
        end = data.find(">", start);
        if (end == std::string::npos ) break;
        end += 1;
        std::string sub = data.substr(0, end);
        if (data == "</arguments>") {
            break;
        }
        args.push_back(parseXML(sub));
        data.erase(0, end);
    }

    return args;
}

} // namespace plugin
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
