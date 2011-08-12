// extension.cpp:  Read and enable plug-in extensions to Flash, for Gnash.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#if defined(WIN32) || defined(_WIN32)
#define LIBLTDL_DLL_IMPORT 1
#endif
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <ltdl.h>

#include "log.h"
#include "sharedlib.h"
#include "extension.h"

namespace gnash {
    class as_object;
}

#if defined(WIN32) || defined(_WIN32)
int        lt_dlsetsearchpath   (const char *search_path);
#endif

#if HAVE_DIRENT_H || WIN32==1    // win32 hack
# include <dirent.h>
# define NAMLEN(dirent) std::strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

namespace gnash {

Extension::Extension() 
{
    char *env = std::getenv("GNASH_PLUGINS");
    if (!env) {
        _pluginsdir = PLUGINSDIR;
    }
    else {
        _pluginsdir = env;
    }

    log_debug("Plugins path: %s", _pluginsdir);
#ifdef HAVE_LTDL
    lt_dlsetsearchpath(_pluginsdir.c_str());
#endif
}

Extension::Extension(const std::string& dir)
{
//    GNASH_REPORT_FUNCTION;
    _pluginsdir = dir;

#ifdef HAVE_LTDL
    lt_dlsetsearchpath(_pluginsdir.c_str());
#endif
}

bool
Extension::scanAndLoad(const std::string& dir, as_object &obj)
{
//    GNASH_REPORT_FUNCTION;
    
#ifdef HAVE_LTDL
    lt_dlsetsearchpath(_pluginsdir.c_str());
#endif
    _pluginsdir = dir;
    
    return scanAndLoad(obj);
}

bool
Extension::scanAndLoad(as_object& where)
{
//    GNASH_REPORT_FUNCTION;
    
    if (_modules.empty()) {
        scanDir(_pluginsdir);
    }
    
    std::vector<std::string>::iterator it;
    for (it = _modules.begin(); it != _modules.end(); ++it) {
        const std::string& mod = *it;
        log_security(_("Loading module: %s from %s"), mod, _pluginsdir);
        initModule(mod, where);
    }
    return true;
}

bool
Extension::initModule(const std::string& module, as_object &where)
{

    SharedLib *sl;
    std::string symbol(module);

    log_security(_("Initializing module: \"%s\" from %s"), symbol, _pluginsdir);
    
    if (_plugins[module] == 0) {
        sl = new SharedLib(_pluginsdir + "/" + module, "GNASH_PLUGINS");
        sl->openLib();
        _plugins[module] = sl;
    } else {
        sl = _plugins[module];
    }
    
    symbol.append("_class_init");
    
    SharedLib::initentry *symptr = sl->getInitEntry(symbol);

    if (symptr) {    
        symptr(where);
    } else {
        log_error(_("Couldn't get class_init symbol"));
    }
    
    return true;
}

bool
Extension::initModuleWithFunc(const std::string& module,
        const std::string& func, as_object &obj)
{
    GNASH_REPORT_FUNCTION;

    SharedLib *sl;

    log_security(_("Initializing module: \"%s\""), module);

    if (_plugins[module] == 0) {
        sl = new SharedLib(module);
        sl->openLib();
        _plugins[module] = sl;
    } else {
        sl = _plugins[module];
    }

    SharedLib::initentry *symptr = sl->getInitEntry(func);

    if (symptr) {
        symptr(obj);
    } else {
        log_error(_("Couldn't get class_init symbol: \"%s\""), func);
    }

    return true;
}

bool
Extension::scanDir()
{
//    GNASH_REPORT_FUNCTION;
    scanDir(_pluginsdir);
    return true;
}

bool
Extension::scanDir(const std::string& dirlist)
{
    GNASH_REPORT_FUNCTION;
    
    Tok t(dirlist, Sep(":"));
    for (Tok::iterator i = t.begin(), e = t.end(); i != e; ++i) {

        const std::string& dir = *i;

        log_debug(_("Scanning directory \"%s\" for plugins"), dir);
        DIR *libdir = opendir(dir.c_str());

        if (!libdir) {
            log_error(_("Can't open directory %s"), dir);
            return false;
        }   
        
        struct dirent *entry;

        while ((entry = readdir(libdir)) != NULL) {
            // We only want shared libraries that end with the suffix, otherwise
            // we get all the duplicates.
            std::string name(entry->d_name);

            // Hidden files.
            if (name.at(0) == '.') {
                continue;
            }            
                
            const std::string::size_type pos = name.find_last_of('.');
 
            if (pos == std::string::npos) continue;
 
            const std::string suffix = name.substr(pos);
            name.erase(pos);

            if (suffix == ".so") {
                log_debug(_("Gnash Plugin name: %s"), name);
                _modules.push_back(name);
            }
            else {
                continue;
            }
        }
        
        if (closedir(libdir) != 0) {
            return false;
        }
    }
    return true;
}

void
Extension::dumpModules()
{
    GNASH_REPORT_FUNCTION;
    
    std::cerr << _modules.size() << " plugin(s) for Gnash installed" << std::endl;    
    std::vector<std::string>::iterator it;
    for (it = _modules.begin(); it != _modules.end(); ++it) {
        std::cerr << "Module name is: \"" << *(it) << "\"" << std::endl;
    }
}

} // end of gnash namespace
