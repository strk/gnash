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

#include "Relay.h"

namespace gnash {

class as_object;
class as_value;
class ObjectURI;
class Global_as;
class movie_root;
}

namespace gnash {

class ExternalInterface_as: public ActiveRelay
{
public:
    ExternalInterface_as(as_object* owner);
    static ExternalInterface_as &Instance();
    virtual ~ExternalInterface_as();

    // This is a flag that specifies whether exceptions in ActionScript
    // should be propogated to JavaScript in the browser.
    void setMarshallExceptions(bool x) { _marshallExceptions = x; };
    bool getMarshallExceptions() { return _marshallExceptions; };
    
    /// Add an ActionScript function as a callback by JavaScript
    // in the browser.
    bool addCallback(const std::string &name, as_object *method);

    ///
    bool addRootCallback(movie_root &mr);    

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

    /// Call a callback if it's registered already.
    bool call(as_object* callback, const std::string& name,
              const std::vector<as_value>& args, size_t firstArg);
    
    // These are our implementations of ActiveRelay methods.
    // virtual bool advance() = 0;
    // virtual void setReachable() const = 0;
    
    virtual void update();

    // Parse the XML Invoke message.
    void processInvoke(const std::string &str);

    void setFD(int x) { _fd = x; };

    as_object *getCallback(const std::string &name);

    as_value parseXML(const std::string &xml);
    std::vector<as_value> parseArguments(const std::string &xml);
    
private:
    int         _fd;
    std::map<std::string, as_object *> _methods;
    bool		_marshallExceptions;
};

/// Initialize the global ExternalInterface class
void externalinterface_class_init(gnash::as_object& where,
                                  const gnash::ObjectURI& uri);

} // end of gnash namespace

// __GNASH_ASOBJ_EXTERNALINTERFACE_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
