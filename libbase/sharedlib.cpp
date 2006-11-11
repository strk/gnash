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

/* $Id: sharedlib.cpp,v 1.3 2006/11/11 14:36:33 strk Exp $ */

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
#else
# include <boost/detail/lightweight_mutex.hpp>
  using boost::detail::lightweight_mutex;
# define scoped_lock lightweight_mutex::scoped_lock
  static lightweight_mutex lib_mutex;
#endif

#include "log.h"
#include "sharedlib.h"

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

using namespace std;
namespace gnash {

#ifdef LT_DLMUTEX

static void
gnash_mutex_seterror (const char *err)
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
{
#ifdef LT_DLMUTEX
//     return lt_dlmutex_register (gnash_mutex_lock, gnash_mutex_unlock,
//                                 gnash_mutex_seterror, gnash_mutex_geterror);
#endif
}

bool
SharedLib::closeLib ()
{
    return lt_dlclose (_dlhandle);
}

bool
SharedLib::openLib (string &filespec)
{
    GNASH_REPORT_FUNCTION;
    
    int errors = 0;
    char pwd[512];

#if 0
    struct stat ostats;
    if (stat (filespec.c_str(), &ostats)) {
        switch (errno) {
          case EBADF:
          case ENOENT:
              Err.SetMsg("Specified shared library doesn't exist");
              dbglogfile << "ERROR: Dynamic library, " << filespec << " doesn't exist!" << endl;
              return false;
              break;
        }
    }
#endif  
    
#if 0
    // ltdl should use the same mallocation as us
    lt_dlmalloc = (lt_ptr (*) (size_t)) xmalloc;
    lt_dlfree = (void (*) (lt_ptr)) free;
    
#endif
    
    // Make sure preloaded modules are initialised
//  LTDL_SET_PRELOADED_SYMBOLS();
    
    scoped_lock lock(lib_mutex);
    
    // Initialize libtool's dynamic library loader
    errors = lt_dlinit ();
    
    if (errors) {
        dbglogfile << "Couldn't initialize ltdl";
        return false;
    }
    
    dbglogfile << "Initialized ltdl" << endl;
    
    // Get the path to look for libraries in, or force a default one 
    // if the GNASH_PLUGINS environment variable isn't set.
    const char *plugindir = (char *)getenv ("GNASH_PLUGINS");
    if (plugindir == NULL) {
        getcwd((char *)&pwd, 512);
        plugindir = pwd;
        dbglogfile << "WARNING: using default DL search path" << endl;
    }
    
    errors = lt_dladdsearchdir (plugindir);
    if (errors) {
        dbglogfile << lt_dlerror();
        return false;
    }
    
    dbglogfile << "Added " << plugindir << " to the search paths" << endl;
    
    dbglogfile << "Trying to open shared library " << filespec << endl;
    
    _dlhandle = lt_dlopenext (filespec.c_str());
    
    if (_dlhandle == NULL) {
        dbglogfile << lt_dlerror();
        return false;
    }
    
    lt_dlmakeresident(_dlhandle);
    
    dbglogfile << "Opened dynamic library " << filespec << endl;
    return true;
}

SharedLib::entrypoint *
SharedLib::getDllSymbol (std::string &symbol)
{
    GNASH_REPORT_FUNCTION;
    
    lt_ptr run = NULL;
    
    scoped_lock lock(lib_mutex);

    run  = lt_dlsym (_dlhandle, symbol.c_str());
    
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

#if 0
// Open the database
bool
SharedLib::ScanDir (void) {
    GNASH_REPORT_FUNCTION;
    
    int i;
    struct device_info *info;
    struct dirent *entry;
    lt_dlhandle dlhandle;
    bool (*InitDBaddr)(void);
    lt_ptr_t addr;
    struct errcond err;
    
    scoped_lock lock(lib_mutex);

    // Initialize libdl
    lt_dlinit ();
    LTDL_SET_PRELOADED_SYMBOLS();
    
    // Get the path to look for libraries in, or force a default one 
    // if the GNASH_PLUGINS environment variable isn't set.
    const char *plugindir = (char *)getenv ("GNASH_PLUGINS");
    if (plugindir == NULL) {
        plugindir = "/usr/local/lib/gnash";
        dbglogfile << "ERROR: You need to set GNASH_PLUGINS" << endl;
    }
    
    lt_dladdsearchdir (plugindir);
    // dbglogfile << timestamp << "Searching in " << gnash << "for database drivers" << endl;
    
    DIR *library_dir = opendir (plugindir);
    
    // By convention, the first two entries in each directory are for . and
    // .. (``dot'' and ``dot dot''), so we ignore those.
    entry = readdir(library_dir);
    entry = readdir(library_dir);
    
    for (i=0; entry>0; i++) {
        // We only want shared libraries than end with the suffix, otherwise
        // we get all the duplicates.
        entry = readdir(library_dir);
        if ((int)entry < 1)
            return SUCCESS;
        
        //    handle = dlopen (entry->d_name, RTLD_NOW|RTLD_GLOBAL);
        _dlhandle = lt_dlopen (entry->d_name);
        if (_dlhandle == NULL) {
            continue;
        }
        cout << "Opening " << entry->d_name << endl;
        //    InitDBaddr = (bool (*)(...))dlsym (handle, "InitDB");
        (lt_ptr_t) InitDBaddr = lt_dlsym (_dlhandle, "InitDB");
        if (InitDBaddr != NULL) {
            //      dbglogfile << "Found OpenDB in " << entry->d_name << endl;
            cout << "Found InitDB in " << entry->d_name << " at " << addr << endl;
            InitDBaddr();
        } else {
            //      dbglogfile << "Didn't find OpenDB in " << entry->d_name << endl;
            cout << "Didn't find InitDB in " << entry->d_name << endl;
        }
        lt_dlclose (_dlhandle);
    }
    closedir(library_dir);
    
}
#endif

// Get information about the DLL
char *
SharedLib::getDllFileName ()
{
    GNASH_REPORT_FUNCTION;

    return  lt_dlgetinfo(_dlhandle)->filename;
}

char *
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
