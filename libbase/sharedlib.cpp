// sharedlib.cpp:  Shared Library support, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "sharedlib.h"

#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <cstdlib>

#if defined(WIN32) || defined(_WIN32)
#define LIBLTDL_DLL_IMPORT 1
#endif
#include <ltdl.h>
#ifdef HAVE_DLFCN_H
	#include <dlfcn.h>
#endif
#ifdef HAVE_LIBGEN_H
	#include <libgen.h>
#endif
#include <boost/thread/mutex.hpp>

#if defined(_WIN32) || defined(WIN32)
#	define PLUGINSDIR "./"
#endif


namespace gnash {

SharedLib::SharedLib()
{
//    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
}

SharedLib::SharedLib(const std::string& filespec)
{
//    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
    _filespec = filespec;
    scoped_lock lock(_libMutex);
    
    // Initialize libtool's dynamic library loader
    int errors = lt_dlinit ();
    if (errors) {
        log_error (_("Couldn't initialize ltdl: %s"), lt_dlerror());
//     } else {
//         log_debug ("Initialized ltdl");
    }
    std::string pluginsdir = PLUGINSDIR;
    
    char *env = std::getenv ("GNASH_PLUGINS");
    if (env) pluginsdir = env;
   

    lt_dlsetsearchpath(pluginsdir.c_str());
}

SharedLib::~SharedLib()
{
    //   GNASH_REPORT_FUNCTION;
//    closeLib();
//    lt_dlexit();
}

bool
SharedLib::closeLib()
{
    return lt_dlclose(_dlhandle);
}

bool
SharedLib::openLib()
{
    return openLib(_filespec);
}

bool
SharedLib::openLib (const std::string& filespec)
{
//    GNASH_REPORT_FUNCTION;
    
#if 0
    // ltdl should use the same mallocation as us
    lt_dlmalloc = (lt_ptr (*) (size_t)) xmalloc;
    lt_dlfree = (void (*) (lt_ptr)) free;
    
#endif
    
    // Make sure preloaded modules are initialised
//  LTDL_SET_PRELOADED_SYMBOLS();
    
    scoped_lock lock(_libMutex);
    
//     // libtool's dynamic library loader is already initialized in constructor
    
//     cerr << "Searching in " << lt_dlgetsearchpath()
//          << "for database drivers" << endl;

//    log_debug ("Trying to open shared library \"%s\"", filespec);
    _dlhandle = lt_dlopenext (filespec.c_str());
    
    if (_dlhandle == NULL) {
        log_error ("%s", lt_dlerror());
        return false;
    }

    // Make this module unloadable
    lt_dlmakeresident(_dlhandle);
    
    log_debug (_("Opened dynamic library \"%s\""), filespec);

    _filespec = filespec;
    
    return true;
}

SharedLib::initentry *
SharedLib::getInitEntry (const std::string& symbol)
{
//    GNASH_REPORT_FUNCTION;
    lt_ptr run = NULL;
    
    scoped_lock lock(_libMutex);

    run  = lt_dlsym (_dlhandle, symbol.c_str());
    
    if (run == NULL) {
        log_error (_("Couldn't find symbol: %s"), symbol);
        return NULL;
    } else {
        log_debug (_("Found symbol %s @ %p"), symbol, (void *)run);
    }
    
    return (initentry*)(run);
}

SharedLib::entrypoint *
SharedLib::getDllSymbol(const std::string& symbol)
{
    GNASH_REPORT_FUNCTION;
    
    lt_ptr run = NULL;
    
    scoped_lock lock(_libMutex);

    run  = lt_dlsym (_dlhandle, symbol.c_str());
    
    /* 
    Realistically, we should never get a valid pointer with a value of 0
    Markus: 'Id est NULL.'
    */
    if (run == NULL) {
        log_error (_("Couldn't find symbol: %s"), symbol);
        return NULL;
    } else {
        log_debug (_("Found symbol %s @ %p"), symbol, (void *)run);
    }
    
    return (entrypoint*)(run);
}

} // end of gnash namespace
