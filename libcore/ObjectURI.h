
#ifndef GNASH_OBJECTURI_H
#define GNASH_OBJECTURI_H

#include "string_table.h"
#include <string>
#include <ostream>

namespace gnash {

/// A URI for describing as_objects.
//
/// This is used as a unique identifier for any object member, especially
/// prototypes, class, constructors.
struct ObjectURI
{

    class Logger;

    /// Default constructor, no name, no caseless name
    ObjectURI()
        :
        name(0),
        nameNoCase(0)
    {}

    /// Construct an ObjectURI from name and namespace.
    ObjectURI(string_table::key name)
        :
        name(name),
        nameNoCase(0)
    {}

    operator const void*() const {
        return (name == 0) ? 0 : this;
    }

    const std::string&
    toString(string_table& st) const
    {
        return st.value(name);
    }


    string_table::key noCase(string_table& st) const {
        if ( ! nameNoCase ) nameNoCase = st.noCase(name);
        return nameNoCase;
    }

    string_table::key name;

    mutable string_table::key nameNoCase;
};

inline bool
equalsNoCase(string_table& st, const ObjectURI& a, const ObjectURI& b)
{
    return a.noCase(st) == b.noCase(st);
}

/// ObjectURIs are equal if name is equal
inline bool
operator==(const ObjectURI& a, const ObjectURI& b)
{
    return a.name == b.name;
}

/// Comparator for ObjectURI so it can serve as a key in stdlib containers.
inline bool
operator<(const ObjectURI& a, const ObjectURI& b)
{
    return a.name < b.name;
}

/// Get the name element of an ObjectURI
inline string_table::key
getName(const ObjectURI& o)
{
    return o.name;
}

class ObjectURI::Logger
{
public:
    Logger(string_table& st) : _st(st) {}

    std::string operator()(const ObjectURI& uri) const {
        const string_table::key name = getName(uri);
        return _st.value(name);
    }
private:
    string_table& _st;
};

} // namespace gnash
#endif
