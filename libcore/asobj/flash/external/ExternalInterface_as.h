// ExternalInterface_as.h:  ActionScript "ExternalInterface" class, for Gnash.
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

#ifndef GNASH_ASOBJ_EXTERNALINTERFACE_H
#define GNASH_ASOBJ_EXTERNALINTERFACE_H

#include <string>
#include <vector>
#include <map>

namespace gnash {

class as_object;
class as_value;
class ObjectURI;
class Global_as;
}

namespace gnash {

class ExternalInterface_as
{
public:
    ExternalInterface_as(as_object* owner);
    ~ExternalInterface_as();

    /// Add an ActionScript function as a callback by JavaScript
    // in the browser.
    bool addCallback(const std::string &name, as_object *method);

    // This is a flag that specifies wether exceptions in ActionScript
    // should be propogated to JavaScript in the browser.
    void marshallExceptions(bool flag) { _exceptions = flag; };
    bool marshallExceptions() { return _exceptions; };

    /// Returns the id attribute of the object tag in Internet Explorer,
    /// or the name attribute of the embed tag in Netscape. 
    const std::string &objectID() { return _objectid; };
    std::string objectID(as_object &obj);    
    
    /// Call a callback if it's registered already.
    static bool call(as_object* asCallback, const std::string& methodName,
              const std::vector<as_value>& args, size_t firstArg);

    // These appear to be undocumented helper functions of this class
    // that while propably designed to be used internally, get used
    // by ActionScript coders.

    /// Convert an AS object to an XML string.
    static std::string toXML(const as_value &obj);
    
    /// Convert an XML string to an AS value.
    static as_value toAS(Global_as& as, const std::string &xml);

    /// Convert an XML string of properties to a data structure.
    static std::map<std::string, as_value> propertiesToAS(Global_as& gl,
                                                   std::string &xml);
    
    static as_value argumentsToXML(std::vector<as_value> &args);
//    as_value argumentsToAS();
    
    static std::string objectToXML(as_object *obj);
    static as_value objectToAS(Global_as& gl, const std::string &xml);
//  std::string objectToJS(as_object &obj);
//  as_value toJS(const std::string &xml);;
    
    static std::string arrayToXML(as_object *obj);

//  std::string arrayToJS();
//  as_value arrayToAS();

//  std::string jsQuoteString();
//  void initJS();
//  bool evalJS();
    
//  callOut"));
//  callIn"));

    static std::string escapeXML(as_object &obj);
    static std::string unescapeXML(as_object &obj);
    
private:
    std::string _objectid;
    bool        _exceptions;
    std::map<std::string, as_object *> _methods;
};

/// Initialize the global ExternalInterface class
void externalinterface_class_init(as_object& where, const ObjectURI& uri);

} // end of gnash namespace

// __GNASH_ASOBJ_EXTERNALINTERFACE_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
