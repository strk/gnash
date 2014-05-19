// 
//   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc
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

#ifndef GNASH_PLUGIN_EXTERNAL_H
#define GNASH_PLUGIN_EXTERNAL_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "npapi.h"
#include "npruntime.h"

#include "GnashNPVariant.h"

namespace gnash {

namespace plugin {

struct ExternalInterface
{
    typedef struct {
        std::string name;
        std::string type;
        std::vector<GnashNPVariant> args;
    } invoke_t;
    
    // Create an Invoke message for the standalone Gnash
    static std::string makeInvoke (const std::string &method, std::vector<std::string> args);
    
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
    
    static GnashNPVariant parseXML(const std::string &xml);
    static std::shared_ptr<invoke_t> parseInvoke(const std::string &xml);
    
    static std::map<std::string, GnashNPVariant> parseProperties(const std::string &xml);
    static std::vector<GnashNPVariant> parseArguments(const std::string &xml);
    static std::string convertNPVariant (const NPVariant *npv);
};

}
}

#endif // GNASH_PLUGIN_EXTERNAL_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
