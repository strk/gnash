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

/* $Id: extension.cpp,v 1.16 2008/01/21 20:55:44 rsavoye Exp $ */

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
# define NAMLEN(dirent) strlen((dirent)->d_name)
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

using namespace std;
namespace gnash {

Extension::Extension() 
{
//    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
    char *env = getenv ("GNASH_PLUGINS");
    if (env == 0) {
        _pluginsdir = PLUGINSDIR;
    } else {
        _pluginsdir = env;
    }

    log_msg("Plugins path: %s", _pluginsdir);
    lt_dlsetsearchpath(_pluginsdir);
}

Extension::Extension(const char *dir)
{
//    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
    _pluginsdir = dir;
    lt_dlsetsearchpath(_pluginsdir);
}

Extension::~Extension()
{
//    GNASH_REPORT_FUNCTION;
}

bool
Extension::scanAndLoad(const char *dir, as_object &obj)
{
//    GNASH_REPORT_FUNCTION;
    
    lt_dlsetsearchpath(_pluginsdir);
    _pluginsdir = dir;
    
    return scanAndLoad(obj);
}

bool
Extension::scanAndLoad(as_object &obj)
{
//    GNASH_REPORT_FUNCTION;
    string mod;
    
    if (_modules.size() == 0) {
        scanDir(_pluginsdir);
    }
    
    vector<string>::iterator it;
    for (it = _modules.begin(); it != _modules.end(); it++) {
        mod = *(it);
        log_msg(_("Loading module: %s"), mod.c_str());
        SharedLib sl;
        initModule(mod.c_str(), obj);
    }   
		return true;
}

bool
Extension::initModule(const char *module, as_object &obj)
{
//    GNASH_REPORT_FUNCTION;

    SharedLib::initentry *symptr;
    SharedLib *sl;
    string symbol;

    log_msg(_("Initializing module: \"%s\""), module);
    
    symbol = module;
    if (_plugins[module] == 0) {
        sl = new SharedLib(module);
        sl->openLib();
        _plugins[module] = sl;
    } else {
        sl = _plugins[module];
    }
    
    symbol += "_class_init";
    symptr = sl->getInitEntry(symbol.c_str());

    if (symptr) {    
        symptr(obj);
    } else {
        log_error(_("Couldn't get class_init symbol"));
    }
    
    return true;
}

bool
Extension::initModuleWithFunc(const char *module, const char *func,
	as_object &obj)
{
	SharedLib::initentry *symptr;
	SharedLib *sl;

	log_msg(_("Initializing module: \"%s\""), module);

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
Extension::scanDir(const char *dirlist)
{
//    GNASH_REPORT_FUNCTION;
    
    int i;
    struct dirent *entry;
    //string::size_type pos;
    char *dirlistcopy;
    char *dir, libsdir;
    char *suffix = 0;

//    scoped_lock lock(lib_mutex);

    dirlistcopy = strdup(dirlist);
    
    dir = strtok(dirlistcopy, ":");
    if (dir == NULL) {
        dir = dirlistcopy;
    }

            
    while (dir) {
        log_msg(_("Scanning directory \"%s\" for plugins"), dir);
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

            if (strncmp(entry->d_name, ".", 1) == 0) {
                continue;
            }            
            
            suffix = strrchr(entry->d_name, '.');
            if (suffix == 0) {
                continue;
            }

            log_msg(_("Gnash Plugin name: %s"), entry->d_name);
            
            if (strcmp(suffix, ".so") == 0) {
                *suffix = 0;
                log_msg(_("Gnash Plugin name: %s"), entry->d_name);
                _modules.push_back(entry->d_name);
            } else {
                continue;
            }
        }
        
        if (closedir(library_dir) != 0) {
            return false;
        }
        dir = strtok(NULL, ":");
    }
	return true;
}

void
Extension::dumpModules()
{
    GNASH_REPORT_FUNCTION;
    
    cerr << _modules.size() << " plugin(s) for Gnash installed" << endl;    
    vector<string>::iterator it;
    for (it = _modules.begin(); it != _modules.end(); it++) {
        cerr << "Module name is: \"" << *(it) << "\"" << endl;
    }
}

} // end of gnash namespace
