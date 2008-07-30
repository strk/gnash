// extension.cpp:  Read and enable plug-in extensions to Flash, for Gnash.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// #if defined(_WIN32) || defined(WIN32)
// # define lock(lib_mutex);
// # define scoped_lock ;
// #define PLUGINSDIR "./"	//hack
// #define USE_DIRENT 1
// #else
// # include <boost/detail/lightweight_mutex.hpp>
//   using boost::detail::lightweight_mutex;
// # define scoped_lock lightweight_mutex::scoped_lock
//   static lightweight_mutex lib_mutex;
// #endif

#include <ltdl.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>

#include "log.h"
#include "sharedlib.h"
#include "extension.h"
#include "as_object.h"

#if HAVE_DIRENT_H || WIN32==1	// win32 hack
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
//    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
    char *env = std::getenv("GNASH_PLUGINS");
    if (!env) {
        _pluginsdir = PLUGINSDIR;
    }
    else {
        _pluginsdir = env;
    }

    log_debug("Plugins path: %s", _pluginsdir);
    lt_dlsetsearchpath(_pluginsdir.c_str());
}

Extension::Extension(const std::string& dir)
{
//    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
    _pluginsdir = dir;
    lt_dlsetsearchpath(_pluginsdir.c_str());
}

Extension::~Extension()
{
//    GNASH_REPORT_FUNCTION;
}

bool
Extension::scanAndLoad(const std::string& dir, as_object &obj)
{
//    GNASH_REPORT_FUNCTION;
    
    lt_dlsetsearchpath(_pluginsdir.c_str());
    _pluginsdir = dir;
    
    return scanAndLoad(obj);
}

bool
Extension::scanAndLoad(as_object& where)
{
//    GNASH_REPORT_FUNCTION;
    std::string mod;
    
    if (_modules.empty()) {
        scanDir(_pluginsdir);
    }
    
    std::vector<std::string>::iterator it;
    for (it = _modules.begin(); it != _modules.end(); it++) {
        mod = *(it);
        log_security(_("Loading module: %s"), mod);
        initModule(mod, where);
    }   
		return true;
}

bool
Extension::initModule(const std::string& module, as_object &where)
{

    SharedLib *sl;
    std::string symbol(module);

    log_security(_("Initializing module: \"%s\""), symbol);
    
    if (_plugins[module] == 0) {
        sl = new SharedLib(module);
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
Extension::initModuleWithFunc(const std::string& module, const std::string& func,
	as_object &obj)
{
	SharedLib::initentry *symptr;
	SharedLib *sl;

	log_security(_("Initializing module: \"%s\""), module);

	if (_plugins[module] == 0) {
		sl = new SharedLib(module);
		sl->openLib();
		_plugins[module] = sl;
	} else {
		sl = _plugins[module];
	}

	symptr = sl->getInitEntry(func);

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
//    GNASH_REPORT_FUNCTION;
    
    int i;
    struct dirent *entry;
    //string::size_type pos;
    char *dirlistcopy;
    char *dir;
    char *suffix = 0;

//    scoped_lock lock(lib_mutex);

    dirlistcopy = strdup(dirlist.c_str());
    
    dir = std::strtok(dirlistcopy, ":");
    if (dir == NULL) {
        dir = dirlistcopy;
    }

            
    while (dir) {
        log_debug(_("Scanning directory \"%s\" for plugins"), dir);
        DIR *library_dir = opendir(dir);

        if (library_dir == NULL) {
            log_error(_("Can't open directory %s"), dir);
            return false;
        }
        
        entry = readdir(library_dir);
        for (i=0; entry>0; i++) {
            // We only want shared libraries than end with the suffix, otherwise
            // we get all the duplicates.
            entry = readdir(library_dir);

            if (entry <= NULL) { // All done
                continue;
            }

            if (std::strncmp(entry->d_name, ".", 1) == 0) {
                continue;
            }            
            
            suffix = std::strrchr(entry->d_name, '.');
            if (suffix == 0) {
                continue;
            }

            log_debug(_("Gnash Plugin name: %s"), entry->d_name);
            
            if (std::strcmp(suffix, ".so") == 0) {
                *suffix = 0;
                log_debug(_("Gnash Plugin name: %s"), entry->d_name);
                _modules.push_back(entry->d_name);
            } else {
                continue;
            }
        }
        
        if (closedir(library_dir) != 0) {
            return false;
        }
        dir = std::strtok(NULL, ":");
    }
	return true;
}

void
Extension::dumpModules()
{
    GNASH_REPORT_FUNCTION;
    
    std::cerr << _modules.size() << " plugin(s) for Gnash installed" << std::endl;    
    std::vector<std::string>::iterator it;
    for (it = _modules.begin(); it != _modules.end(); it++) {
        std::cerr << "Module name is: \"" << *(it) << "\"" << std::endl;
    }
}

} // end of gnash namespace
