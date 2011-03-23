// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_OBJECTURI_H
#define GNASH_OBJECTURI_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // GNASH_STATS_OBJECT_URI_NOCASE
#endif

#include "string_table.h"
#include "namedStrings.h"

#include <string>
#include <sstream>

//#define GNASH_STATS_OBJECT_URI_NOCASE 1

#ifdef GNASH_STATS_OBJECT_URI_NOCASE
# include "Stats.h"
#endif

namespace gnash {

/// A URI for describing as_objects.
//
/// This is used as a unique identifier for any object member, especially
/// prototypes, class, constructors.
struct ObjectURI
{

    /// Comparison taking case into account (or not).
    class CaseLessThan;

    /// Simple, case-sensitive less-than comparison for containers.
    class LessThan;

    /// Case-sensitive equality
    class CaseEquals;

    /// Log strings.
    class Logger;

    /// Default constructor.
    //
    /// This must be equivalent to an empty string.
    ObjectURI()
        :
        name(0),
        nameNoCase(0)
    {}

    /// Construct an ObjectURI from name
    ObjectURI(NSV::NamedStrings name)
        :
        name(name),
        nameNoCase(0)
    {}


    bool empty() const {
        return (name == 0);
    }

    const std::string& toString(string_table& st) const {
        return st.value(name);
    }
    
    string_table::key noCase(string_table& st) const {

        if (!name) return 0;

        if (!nameNoCase) {
            nameNoCase = st.noCase(name);
#ifdef GNASH_STATS_OBJECT_URI_NOCASE
            static stats::KeyLookup statNonSkip("ObjectURI::noCase non-skips",
                    st, 0, 0, 0);
            statNonSkip.check(name);
#endif
        }
#ifdef GNASH_STATS_OBJECT_URI_NOCASE
        else {
            static stats::KeyLookup stat("ObjectURI::noCase skips",
                    st, 0, 0, 0);
            stat.check(name);
        }
#endif

        return nameNoCase;
    }

    string_table::key name;

private:

    mutable string_table::key nameNoCase;
};

/// Get the name element of an ObjectURI
inline string_table::key
getName(const ObjectURI& o)
{
    return o.name;
}

class ObjectURI::LessThan
{
public:
    bool operator()(const ObjectURI& a, const ObjectURI& b) const {
        return a.name < b.name;
    }
};

class ObjectURI::CaseLessThan
{
public:
    CaseLessThan(string_table& st, bool caseless = false)
        :
        _st(st),
        _caseless(caseless)
    {}
    bool operator()(const ObjectURI& a, const ObjectURI& b) const {
        if (_caseless) return a.noCase(_st) < b.noCase(_st);
        return a.name < b.name;
    }
private:
    string_table& _st;
    const bool _caseless;
};

class ObjectURI::CaseEquals
{
public:
    CaseEquals(string_table& st, bool caseless = false)
        :
        _st(st),
        _caseless(caseless)
    {}
    bool operator()(const ObjectURI& a, const ObjectURI& b) const {
        if (_caseless) return a.noCase(_st) == b.noCase(_st);
        return a.name == b.name;
    }
private:
    string_table& _st;
    const bool _caseless;
};

class ObjectURI::Logger
{
public:
    Logger(string_table& st) : _st(st) {}

    std::string operator()(const ObjectURI& uri) const {
        const string_table::key name = getName(uri);
        return _st.value(name);
    }

    std::string debug(const ObjectURI& uri) const {
        std::stringstream ss;
        const string_table::key name = getName(uri);
        const string_table::key nameNoCase = uri.noCase(_st);
        ss << _st.value(name)
           << "(" << name << ")/"
           << _st.value(nameNoCase)
           << "(" << nameNoCase << ")";
        return ss.str();
    }

private:
    string_table& _st;
};

} // namespace gnash
#endif
