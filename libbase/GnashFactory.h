// GnashFactory.h   A generic class template
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

#ifndef GNASH_FACTORY_H
#define GNASH_FACTORY_H

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include <map>
#include <string>
#include <algorithm>
#include <iterator>
#include <type_traits>

#include "dsodefs.h"
#include "GnashAlgorithm.h"

namespace gnash {


/// A generic factory class for registering and retrieving objects by key.
//
/// Note: there is only one GnashFactory for any combination of template
/// arguments. It's not advisable to have more than one factory for any
/// type.
//
/// Note that this relies on static initialization, so do not call get()
/// before or after main().
//
/// @tparam T       The base type to be produced by the factory
/// @tparam Init    An object whose constructor ensures that the elements
///                 are registered. This helps avoid problems with
///                 unpredictable static initialization.
/// @tparam Key     The type to be used as a key.
template<typename T, typename Init, typename Key>
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

    /// Get the GnashFactory singleton.
    static GnashFactory& instance() {
        static GnashFactory m;
        return m;
    }

    /// Dump the registered keys to the iterator.
    //
    /// Only usable with output iterators.
    template<typename Iterator>
    void
    listKeys(Iterator i) {
        typedef typename std::iterator_traits<Iterator>::iterator_category cat;
        static_assert(std::is_same<cat, std::output_iterator_tag>::value,
            "i must be an output iterator.");
        Init();
        std::transform(_handlers.begin(), _handlers.end(), i,
                std::bind(&Handlers::value_type::first, std::placeholders::_1));
    }

    /// Return a Handler identified by a name.
    //
    /// @param name     The name of the handler to return. An empty string
    ///                 will return the first available handler. If the
    ///                 string is not empty and no match is found, a null
    ///                 pointer will be returned.
    T* get(const Key& name) {
        Init();
        if (name.empty()) {
            return _handlers.empty() ? nullptr : _handlers.begin()->second();
        }

        typename Handlers::const_iterator it = _handlers.find(name);
        if (it == _handlers.end()) return nullptr;
        return it->second();
    }

    /// Register a Handler with a particular name.
    //
    /// @param name     The name to register the Handler under. Duplicated
    ///                 names will replace previous handlers!
    /// @param r        A pointer to a function that will return the 
    ///                 Handler when called.
    void registerHandler(const Key& name, CreateHandler r) {
        _handlers[name] = r;
    }

private:

    GnashFactory() {}

    Handlers _handlers;

};
 
} // namespace gnash

#endif
