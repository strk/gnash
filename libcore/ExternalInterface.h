// ExternalInterface.h:  ActionScript "ExternalInterface" support
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

#ifndef GNASH_EXTERNALINTERFACE_H
#define GNASH_EXTERNALINTERFACE_H

#include <memory>
#include <string>
#include <vector>
#include <set>

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
    struct DSOLOCAL invoke_t {
        std::string name;
        std::string type;
        std::vector<as_value> args;
    };

    /// Convert an AS object to an XML string.
    static std::string toXML(const as_value &obj) {
        ExternalInterface ei;
        return ei._toXML(obj);
    }
    
    static as_value parseXML(const std::string &xml);
    static std::vector<as_value> parseArguments(const std::string &xml);

    // Parse the XML Invoke message.
    static std::shared_ptr<invoke_t> parseInvoke(const std::string &str);
    // Check for data from the browser and parse it.
    DSOEXPORT static std::shared_ptr<invoke_t> ExternalEventCheck(int fd);

    // These methods are for constructing Invoke messages.
    // Create an Invoke message for the standalone Gnash
    DSOEXPORT static std::string makeInvoke (const std::string &method,
              		                     const std::vector<as_value> &args);
    
    static std::string makeString (const std::string &str) {
        return "<string>" + str + "</string>";
    }

    DSOEXPORT static size_t writeBrowser(int fd, const std::string &xml);
    DSOEXPORT static std::string readBrowser(int fd);

private:

    DSOEXPORT std::string _toXML(const as_value &obj);
    DSOEXPORT std::string _objectToXML(as_object* obj);
    DSOEXPORT std::string _arrayToXML(as_object *obj);

    std::set<as_object*> _visited;
};

} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
