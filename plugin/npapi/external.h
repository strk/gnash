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

#ifndef GNASH_PLUGIN_EXTERNAL_H
#define GNASH_PLUGIN_EXTERNAL_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "npapi.h"
#include "npruntime.h"

class ExternalInterface
{
public:
    ExternalInterface ();
    ~ExternalInterface ();
    
    // Create an Invoke message for the standalone Gnash
    std::string makeInvoke (const std::string &method, std::vector<std::string> args);
    
    std::string makeNull ();
    std::string makeTrue ();
    std::string makeFalse ();
    std::string makeString (const std::string &str);
    std::string makeProperty (const std::string &str, const std::string &data);
    std::string makeProperty (const std::string &str, double num);
    std::string makeProperty (const std::string &str, int num);
    std::string makeNumber (double num);
    std::string makeNumber (int num);
    std::string makeNumber (unsigned int num);
    std::string makeArray (std::vector<std::string> &args);
    std::string makeObject (std::map<std::string, std::string> &args);
    
    NPVariant *parseXML(const std::string &xml);
    std::map<std::string, NPVariant *> parseProperties(const std::string &xml);
    std::vector<NPVariant *> parseArguments(const std::string &xml);
    std::string convertNPVariant (NPVariant *npv);
};

#endif // GNASH_PLUGIN_EXTERNAL_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
