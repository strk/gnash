// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: sharedlib.cpp,v 1.6 2006/11/24 14:41:39 alexeev Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <ltdl.h>
#include <iostream>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#if defined(_WIN32) || defined(WIN32)
# define lock(lib_mutex) ;
# define scoped_lock ;
#	define PLUGINSDIR "./"
#else
# include <boost/detail/lightweight_mutex.hpp>
  using boost::detail::lightweight_mutex;
# define scoped_lock lightweight_mutex::scoped_lock
  static lightweight_mutex lib_mutex;
#endif

#include "log.h"
#include "sharedlib.h"

using namespace std;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

using namespace std;
namespace gnash {

#ifdef LT_DLMUTEX
static void
gnash_mutex_seterror (void)
{
    GNASH_REPORT_FUNCTION;
}

static const char *
gnash_mutex_geterror (void)
{
    GNASH_REPORT_FUNCTION;
    return NULL;
}

void
gnash_mutex_lock (void)
{
    GNASH_REPORT_FUNCTION;
}

void
gnash_mutex_unlock (void)
{
    GNASH_REPORT_FUNCTION;
}

#endif

SharedLib::SharedLib() 
    : _filespec(0)
{
    GNASH_REPORT_FUNCTION;

    char *plugindir;
    
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
}

SharedLib::SharedLib(const char *filespec)
{
    GNASH_REPORT_FUNCTION;
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
    _filespec = filespec;
    scoped_lock lock(lib_mutex);
    
    // Initialize libtool's dynamic library loader
    int errors = lt_dlinit ();
    if (errors) {
        dbglogfile << "Couldn't initialize ltdl";
        dbglogfile << lt_dlerror();
    } else {
        dbglogfile << "Initialized ltdl" << endl;
    }
    char *pluginsdir;
    char *env = getenv ("GNASH_PLUGINS");
    if (env == 0) {
        pluginsdir = PLUGINSDIR;
    } else {
        pluginsdir = env;
    }

    lt_dlsetsearchpath(pluginsdir);
}

SharedLib::~SharedLib()
{
    GNASH_REPORT_FUNCTION;
//    closeLib();
    lt_dlexit();
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
SharedLib::openLib (string &filespec)
{
    return openLib(filespec.c_str());
}

bool
SharedLib::openLib (const char *filespec)
{
    GNASH_REPORT_FUNCTION;
    
    int errors = 0;
    
#if 0
    // ltdl should use the same mallocation as us
    lt_dlmalloc = (lt_ptr (*) (size_t)) xmalloc;
    lt_dlfree = (void (*) (lt_ptr)) free;
    
#endif
    
    // Make sure preloaded modules are initialised
//  LTDL_SET_PRELOADED_SYMBOLS();
    
    scoped_lock lock(lib_mutex);
    
//     // Initialize libtool's dynamic library loader
//     errors = lt_dlinit ();
    
//     if (errors) {
//         dbglogfile << "Couldn't initialize ltdl";
//         dbglogfile << lt_dlerror();
//         return false;
// //    } else {
// //    dbglogfile << "Initialized ltdl" << endl;
//     }
    
//     cerr << "Searching in " << lt_dlgetsearchpath()
//          << "for database drivers" << endl;

    dbglogfile << "Trying to open shared library \"" << filespec << "\"" << endl;
    _dlhandle = lt_dlopenext (filespec);
    
    if (_dlhandle == NULL) {
        dbglogfile << lt_dlerror();
        return false;
    }

    // Make this module unloadable
    lt_dlmakeresident(_dlhandle);
    
    dbglogfile << "Opened dynamic library \"" << filespec << "\"" << endl;

    _filespec = filespec;
    
    return true;
}

const char *
SharedLib::moduleName()
{
#ifdef WIN32
	return NULL;	//TODO, hack
#else
	return basename(_filespec);
#endif
}

SharedLib::entrypoint *
SharedLib::getDllSymbol (std::string &symbol)
{
    return getDllSymbol(symbol.c_str());
}

SharedLib::initentry *
SharedLib::getInitEntry (const char *symbol)
{
    GNASH_REPORT_FUNCTION;
    lt_ptr run = NULL;
    
    scoped_lock lock(lib_mutex);

    run  = lt_dlsym (_dlhandle, symbol);
    
    if (run == NULL) {
        dbglogfile << "Couldn't find symbol: " << symbol << endl;
        return NULL;
    } else {
        dbglogfile << "Found symbol " << symbol << " @ " << (void *)run << endl;
    }
    
    return (initentry *)run;
}

SharedLib::entrypoint *
SharedLib::getDllSymbol(const char *symbol)
{
    GNASH_REPORT_FUNCTION;
    
    lt_ptr run = NULL;
    
    scoped_lock lock(lib_mutex);

    run  = lt_dlsym (_dlhandle, symbol);
    
    /* 
    Realistically, we should never get a valid pointer with a value of 0
    Markus: 'Id est NULL.'
    */
    if (run == NULL) {
        dbglogfile << "Couldn't find symbol: " << symbol << endl;
        return NULL;
    } else {
        dbglogfile << "Found symbol " << symbol << " @ " << (void *)run << endl;
    }
    
    return (entrypoint *)run;
}

// Get information about the DLL
const char *
SharedLib::getDllFileName ()
{
    GNASH_REPORT_FUNCTION;

    return  lt_dlgetinfo(_dlhandle)->filename;
}

const char *
SharedLib::getDllModuleName ()
{
    GNASH_REPORT_FUNCTION;
    return  lt_dlgetinfo(_dlhandle)->name;
}

int
SharedLib::getDllRefCount ()
{
    GNASH_REPORT_FUNCTION;
    return  lt_dlgetinfo(_dlhandle)->ref_count;
}

} // end of gnash namespace
