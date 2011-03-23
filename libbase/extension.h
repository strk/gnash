// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_EXTENSION_H
#define GNASH_EXTENSION_H

#include <map>
#include <vector>
#include <string>
#include <boost/tokenizer.hpp>
#include "sharedlib.h"
#include "dsodefs.h"


namespace gnash 
{  
  
class DSOEXPORT Extension
{
    typedef boost::char_separator<char> Sep;
    typedef boost::tokenizer< Sep > Tok;
      
public:

    Extension();

    Extension(const std::string& dir);

    ~Extension();

    /// Scan a directory for Gnash modules
    bool scanDir();

    /// Scan the given directory for modules
    //
    /// @param dir  The directory to scan.
    bool scanDir(const std::string& dir);

    /// Scan the plugins directory and attach any found modules to
    /// the given object.
    //
    /// @param where     The as_object to which the modules should be
    ///                  attached (usually the global object)
    bool scanAndLoad(as_object &where);

    /// Scan the given directory and attach any found modules to
    /// the given object.
    //
    /// @param where     The as_object to which the modules should be
    ///                  attached (usually the global object)
    /// @param dir       A directory to scan.
    bool scanAndLoad(const std::string& dir, as_object &where);

    // open a module, initialize the module within Gnash. Known function name.
    bool initModuleWithFunc(const std::string& module, const std::string& func, as_object &obj);
    bool initNewObject(as_object &obj);
    void dumpModules();
protected:

    /// Initialize the named module within Gnash
    //
    /// @param symbol   The name of the module to find and
    ///                 initialize.
    /// @param obj      The object to attach the module to.
    bool initModule(const std::string& module, as_object &obj);

    /// A list of modules
    std::vector<std::string> _modules;
    
    /// A map of loaded modules
    std::map<std::string, SharedLib *> _plugins;
    
    /// The default directory to search for modules.
    std::string _pluginsdir;
};

} // end of gnash namespace

// GNASH_EXTENSION_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
