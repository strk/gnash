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

#ifndef __SHAREDLIB_H__
#define __SHAREDLIB_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <map>
#include <ltdl.h>

namespace gnash 
{
  
typedef struct {
    char *name;
    void *funcptr;
} entry_t;


/// TODO: document this class
class SharedLib
{
public:
    // Typedefs for function pointers to keep the code readable
    typedef bool entrypoint (void *arg);
    
    SharedLib();
    ~SharedLib();
    bool openLib (std::string &name);
    bool closeLib ();
    
    // Get a C symbol from the shared library based on the name
    entrypoint *getDllSymbol (std::string &name);

    // Extract file info from the shared library
    char *getDllFileName();
    char *getDllModuleName();
    int getDllRefCount();
private:
    lt_dlhandle _dlhandle;
};

} // end of gnash namespace

// __SHAREDLIB_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
