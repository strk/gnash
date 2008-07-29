// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <vector>
#include <string>
#include "sharedlib.h"
#include "dsodefs.h"

namespace gnash 
{  
  
class DSOEXPORT Extension
{
  public:
//    typedef bool init_func_t (as_object &obj);
    Extension();
    Extension(const std::string& dir);
    ~Extension();
    // scan a directory for Gnash modules
    bool scanDir();
    bool scanDir(const std::string& dir);
    // scan the directory and open the module
    bool scanAndLoad(as_object &obj);
    bool scanAndLoad(const std::string& dir, as_object &obj);
    // open a module
    // initialize the module within Gnash
    bool initModule(const std::string& module, as_object &obj);
	// open a module, initialize the module within Gnash. Known function name.
	bool initModuleWithFunc(const std::string& module, const std::string& func, as_object &obj);
    bool initNewObject(as_object &obj);
    void dumpModules();
private:
    std::vector<std::string> _modules;
    std::map<std::string, SharedLib *> _plugins;
    std::string _pluginsdir;
};

} // end of gnash namespace

// GNASH_EXTENSION_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
