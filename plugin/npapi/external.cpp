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

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>

#include "npapi.h"
#include "npruntime.h"
#include "external.h"


ExternalInterface::ExternalInterface ()
{
}

ExternalInterface::~ExternalInterface ()
{
}

// Create an Invoke message for the standalone Gnash
std::string
ExternalInterface::makeInvoke (const std::string &method,
                               std::vector<std::string> args)
{
    std::stringstream ss;
    
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
    int index = 0;

    ss << "<object>";
    for (it = args.begin(); it != args.end(); ++it) {
        ss << "<property id=\"" << it->first << "\">" << it->second << "</property>";
    }
    ss << "</object>";
    
    return ss.str();
}

NPVariant *
ExternalInterface::parseXML(const std::string &xml)
{
    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));
    NULL_TO_NPVARIANT(*value);
    
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
            NULL_TO_NPVARIANT(*value);
        } else if (tag == "<void/>") {
            VOID_TO_NPVARIANT(*value);
        } else if (tag == "<true/>") {
            BOOLEAN_TO_NPVARIANT(true, *value);
        } else if (tag == "<false/>") {
            BOOLEAN_TO_NPVARIANT(false, *value);
        } else if (tag == "<number>") {
            start = end;
            end = xml.find("</number>");
            std::string str = xml.substr(start, end-start);
            if (str.find(".") != std::string::npos) {
                double num = strtod(str.c_str(), NULL);
                DOUBLE_TO_NPVARIANT(num, *value);
            } else {
                int num = strtol(str.c_str(), NULL, 0);
                INT32_TO_NPVARIANT(num, *value);
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
            STRINGN_TO_NPVARIANT(data, length, *value);
        } else if (tag == "<array>") {
            NPObject *obj =  (NPObject *)NPN_MemAlloc(sizeof(NPObject));
            start = end;
            end = xml.find("</array");
            std::string str = xml.substr(start, end-start);
            std::map<std::string, NPVariant *> props = parseProperties(str);
            std::map<std::string, NPVariant *>::iterator it;
            for (it=props.begin(); it != props.end(); ++it) {
                NPIdentifier id = NPN_GetStringIdentifier(it->first.c_str());
                NPVariant *value = it->second;
                NPN_SetProperty(NULL, obj, id, value);
            }
            OBJECT_TO_NPVARIANT(obj, *value);
        } else if (tag == "<object>") {
            NPObject *obj =  (NPObject *)NPN_MemAlloc(sizeof(NPObject));
            start = end;
            end = xml.find("</object");
            std::string str = xml.substr(start, end-start);
            std::map<std::string, NPVariant *> props = parseProperties(str);
            std::map<std::string, NPVariant *>::iterator it;
            for (it=props.begin(); it != props.end(); ++it) {
                NPIdentifier id = NPN_GetStringIdentifier(it->first.c_str());
                NPVariant *value = it->second;
                NPN_SetProperty(NULL, obj, id, value);
            }
            OBJECT_TO_NPVARIANT(obj, *value);
        }
    }
    
    return value;
}

std::string
ExternalInterface::convertNPVariant (NPVariant *value)
{
    std::stringstream ss;
    
    if (NPVARIANT_IS_DOUBLE(*value)) {
        double num = NPVARIANT_TO_DOUBLE(*value);
        ss << "<number>" << num << "</number>";
    } else if (NPVARIANT_IS_STRING(*value)) {
        std::string str(NPVARIANT_TO_STRING(*value).UTF8Characters);
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

std::map<std::string, NPVariant *>
ExternalInterface::parseProperties(const std::string &xml)
{
    std::map<std::string, NPVariant *> props;

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

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
