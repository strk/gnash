// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_ASOBJ_SHAREDOBJECT_H
#define GNASH_ASOBJ_SHAREDOBJECT_H

#include <string>
#include <map>

// Forward declarations
namespace gnash {
    class as_object;
    struct ObjectURI;
    class SharedObject_as;
    class VM;
}

namespace gnash {

class SharedObjectLibrary
{
public:

    typedef std::map<std::string, SharedObject_as*> SoLib;

    SharedObjectLibrary(VM& vm);

    ~SharedObjectLibrary();

    /// Return a local shared object with given name and with given root
    //
    /// May return NULL if name is invalid or can't access the given root
    ///
    as_object* getLocal(const std::string& name, const std::string& root);

    void markReachableResources() const;

    // Drop all library items
    void clear();

private:

    VM& _vm;

    /// Domain component of the VM SWF url
    std::string _baseDomain;

    /// Path component of the VM SWF url
    std::string _basePath;

    /// Base SOL dir
    std::string _solSafeDir;
    SoLib	_soLib;
};

/// Initialize the global SharedObject class
void sharedobject_class_init(as_object& where, const ObjectURI& uri);

void registerSharedObjectNative(as_object& o);

  
} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
