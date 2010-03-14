
#ifndef GNASH_OBJECTURI_H
#define GNASH_OBJECTURI_H

#include "string_table.h"
#include <string>
#include <ostream>
/// Define this for verbose logging of ObjectURIs
//#define FULL_OBJECT_URI_LOGGING 1


namespace gnash {

/// A URI for describing as_objects.
//
/// This is used as a unique identifier for any object member, especially
/// prototypes, class, constructors.
struct ObjectURI
{

    class Logger;

    /// Construct an ObjectURI from name and namespace.
    ObjectURI(string_table::key name, string_table::key ns = 0)
        :
        name(name),
        ns(ns)
    {}

    string_table::key name;
    string_table::key ns;

};

/// ObjectURIs are equal if both name and namespace are equal.
inline bool
operator==(const ObjectURI& a, const ObjectURI& b)
{
    return a.name == b.name && a.ns == b.ns;
}

/// Comparator for ObjectURI so it can serve as a key in stdlib containers.
inline bool
operator<(const ObjectURI& a, const ObjectURI& b)
{
    if (a.name < b.name) return true;
    return (a.name == b.name) && a.ns < b.ns;
}

/// Get the name element of an ObjectURI
inline string_table::key
getName(const ObjectURI& o)
{
    return o.name;
}

/// Get the namespace element of an ObjectURI
inline string_table::key
getNamespace(const ObjectURI& o)
{
    return o.ns;
}

class ObjectURI::Logger
{
public:
    Logger(string_table& st) : _st(st) {}

    std::string operator()(const ObjectURI& uri) const {

#ifdef FULL_OBJECT_URI_LOGGING
        const string_table::key ns = getNamespace(uri);
        const string_table::key name = getName(uri);

        boost::format f = boost::format("URI: property %1%(%2%) in namespace "
               " %3%(%4%)") % _st.value(name) % name % _st.value(ns) % ns;
        return f.str();
#else
        const string_table::key ns = getNamespace(uri);
        const string_table::key name = getName(uri);
        if (ns) return _st.value(ns) + "." + _st.value(name);
        return _st.value(name);
#endif

    }
private:
    string_table& _st;
};

} // namespace gnash
#endif
