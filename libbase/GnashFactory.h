// GnashFactory.h   A generic class template
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
//

#ifndef GNASH_FACTORY_H
#define GNASH_FACTORY_H

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include <map>
#include <string>

#include "dsodefs.h"

namespace gnash {


/// A generic factory class for registering and retrieving objects by key.
//
/// Note: there is only one GnashFactory for any combination of template
/// arguments. It's not advisable to have more than one factory for any
/// type.
//
/// Note that this relies on static initialization, so do not call get()
/// before or after main().
template<typename T, typename Key = std::string>
class DSOEXPORT GnashFactory
{
public:

    typedef T value_type;
    typedef Key key_type;

    template<typename Derived>
    struct RegisterHandler
    {
        static T* createHandler() {
            return new Derived();
        }

        RegisterHandler(const Key& name) {
            GnashFactory::instance().registerHandler(name, createHandler);
        }
    };

    typedef T*(*CreateHandler)();
    typedef std::map<std::string, CreateHandler> Handlers;

    /// Get the MediaFactory singleton.
    static GnashFactory& instance() {
        static GnashFactory m;
        return m;
    }

    /// Return a MediaHandler identified by a name.
    //
    /// @param name     The name of the handler to return. An empty string
    ///                 will return the first available handler. If the
    ///                 string is not empty and no match is found, a null
    ///                 pointer will be returned.
    T* get(const Key& name) {
        if (name.empty()) {
            return _handlers.empty() ? 0 : _handlers.begin()->second();
        }

        typename Handlers::const_iterator it = _handlers.find(name);
        if (it == _handlers.end()) return 0;
        return it->second();
    }

    /// Register a MediaHandler with a particular name.
    //
    /// @param name     The name to register the MediaHandler under. Duplicated
    ///                 names will replace previous handlers!
    /// @param r        A pointer to a function that will return the 
    ///                 MediaHandler when called.
    void registerHandler(const Key& name, CreateHandler r) {
        _handlers[name] = r;
    }

private:

    Handlers _handlers;

};
 
} // namespace gnash

#endif
