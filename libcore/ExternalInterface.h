// ExternalInterface.h:  ActionScript "ExternalInterface" support
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

#ifndef GNASH_EXTERNALINTERFACE_H
#define GNASH_EXTERNALINTERFACE_H

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "dsodefs.h" /* For DSOEXPORT */

namespace gnash {

class as_object;
class as_value;
struct ObjectURI;
class Global_as;
class movie_root;
class IOChannel;
}

namespace gnash {

struct DSOEXPORT ExternalInterface
{
    typedef struct DSOLOCAL {
        std::string name;
        std::string type;
        std::vector<as_value> args;
    } invoke_t;

    // Some of these appear to be undocumented helper functions of this class
    // that while probably designed to be used internally, get used
    // by ActionScript coders.

    /// Convert an AS object to an XML string.
    DSOEXPORT static std::string toXML(const as_value &obj);
    
    /// Convert an XML string to an AS value.
    DSOEXPORT static as_value toAS(Global_as& as, const std::string &xml);

    /// Convert an XML string of properties to a data structure.
    DSOEXPORT static std::map<std::string, as_value> propertiesToAS(Global_as& gl,
                                                   std::string &xml);
    
    DSOEXPORT static as_value argumentsToXML(std::vector<as_value> &args);
//    as_value argumentsToAS();
    
    DSOEXPORT static std::string objectToXML(as_object *obj);
    DSOEXPORT static as_value objectToAS(Global_as& gl, const std::string &xml);
//  std::string objectToJS(as_object &obj);
//  as_value toJS(const std::string &xml);;
    
    DSOEXPORT static std::string arrayToXML(as_object *obj);

//  static std::string arrayToJS();
//  static as_value arrayToAS();

//  static std::string jsQuoteString();
//  static void initJS();
//  static bool evalJS();
    
//  static callOut"));
//  static callIn"));

    static std::string escapeXML(as_object &obj);
    static std::string unescapeXML(as_object &obj);

    static as_value parseXML(const std::string &xml);
    static std::vector<as_value> parseArguments(const std::string &xml);

    // Parse the XML Invoke message.
    static boost::shared_ptr<invoke_t> parseInvoke(const std::string &str);
    // Check for data from the browser and parse it.
    DSOEXPORT static boost::shared_ptr<invoke_t> ExternalEventCheck(int fd);

    // These methods are for constructing Invoke messages.
    // Create an Invoke message for the standalone Gnash
    DSOEXPORT static std::string makeInvoke (const std::string &method,
              		                     const std::vector<as_value> &args);
    
    static std::string makeNull ();
    static std::string makeTrue ();
    static std::string makeFalse ();
    static std::string makeString (const std::string &str);
    static std::string makeProperty (const std::string &str, const std::string &data);
    static std::string makeProperty (const std::string &str, double num);
    static std::string makeProperty (const std::string &str, int num);
    static std::string makeNumber (double num);
    static std::string makeNumber (int num);
    static std::string makeNumber (unsigned int num);
    static std::string makeArray (std::vector<std::string> &args);
    static std::string makeObject (std::map<std::string, std::string> &args);

    DSOEXPORT static size_t writeBrowser(int fd, const std::string &xml);
    DSOEXPORT static std::string readBrowser(int fd);
};

} // end of gnash namespace

// __GNASH_ASOBJ_EXTERNALINTERFACE_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
