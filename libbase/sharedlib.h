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

#ifndef __SHAREDLIB_H__
#define __SHAREDLIB_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <map>
#include <ltdl.h>
#include "as_object.h"

// Used on Darwin for basename
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

namespace gnash 
{
  

/// TODO: document this class
class SharedLib
{
public:
    // Typedefs for function pointers to keep the code readable
    typedef bool entrypoint (void *obj);
    typedef void initentry (as_object &obj);
    
    SharedLib();
    SharedLib(const char *filespec);
    ~SharedLib();
    bool openLib();
    bool openLib(std::string &filespec);
    bool openLib(const char *filespec);
    bool closeLib();
    
    // Get a C symbol from the shared library based on the name
    entrypoint *getDllSymbol (std::string &name);
    entrypoint *getDllSymbol (const char *name);
    initentry *getInitEntry (const char *name);

    // Extract file info from the shared library
    const char *getDllFileName();
    const char *getDllModuleName();
    int getDllRefCount();
    const char *moduleName();
//    lt_dlhandle getDllHandle { return _dlhandle; }
    const char *getFilespec() { return _filespec; };
    
    
private:
    lt_dlhandle _dlhandle;
    const char *_filespec;
    const char *_pluginsdir;    
};

} // end of gnash namespace

// __SHAREDLIB_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
