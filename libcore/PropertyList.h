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

#ifndef GNASH_PROPERTYLIST_H
#define GNASH_PROPERTYLIST_H

#include <set> 
#include <map> 
#include <vector> 
#include <string> // for use within map 
#include <cassert> // for inlines
#include <utility> // for std::pair
#include <boost/cstdint.hpp> 
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <algorithm>

#include "Property.h" // for templated functions

// Forward declaration
namespace gnash {
    class as_object;
    class as_environment;
    class as_function;
    struct ObjectURI;
    class as_value;
}

namespace gnash {

/// An abstract property visitor
class PropertyVisitor {
public:

    /// This function should return false if no further visits are needed.
    virtual bool accept(const ObjectURI& uri, const as_value& val) = 0;
    virtual ~PropertyVisitor() {}
};

/// An abstract key visitor
class KeyVisitor {
public:

    /// This function should return false if no further visits are needed.
    virtual void operator()(const ObjectURI& uri) = 0;
    virtual ~KeyVisitor() {}
};


/// Set of properties associated with an ActionScript object.
//
/// The PropertyList container is the sole owner of the Property
/// elements in it contained and has full responsibility of their
/// construction and destruction.
//
/// A PropertyList holds a reference to the as_object whose properties it
/// contains. This reference will always be valid if the PropertyList
/// is a member of as_object.
//
/// It is theoretically possible for a PropertyList to be used with any
/// as_object, not just original as_object it was use with. Currently (as
/// there is no use for this scenario) it is not possible to change the
/// owner.
class PropertyList : boost::noncopyable
{
public:

    typedef std::set<ObjectURI, ObjectURI::LessThan> PropertyTracker;
    typedef Property value_type;

    /// Identifier for the sequenced index
    struct CreationOrder {};

    /// The sequenced index in creation order.
    typedef boost::multi_index::sequenced<
        boost::multi_index::tag<CreationOrder> > SequencedIndex;
    
    struct KeyExtractor
    {
        typedef const ObjectURI& result_type;
        result_type operator()(const Property& p) const {
            return p.uri();
        }
    };

    /// Identifier for the case-sensitive index
    struct Case {};
    
    /// The case-sensitive index
    typedef boost::multi_index::ordered_unique<
        boost::multi_index::tag<Case>,
        KeyExtractor,
        ObjectURI::LessThan> CaseIndex;

    /// Identifier for the case-insensitive index
    struct NoCase {};
    
    /// The case-insensitive index
    typedef boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<NoCase>,
        KeyExtractor,
        ObjectURI::CaseLessThan> NoCaseIndex;

    /// The container of the Properties.
    typedef boost::multi_index_container<
        value_type,
        boost::multi_index::indexed_by<SequencedIndex, CaseIndex, NoCaseIndex>
        > container;

    typedef container::iterator iterator;
    typedef container::const_iterator const_iterator;

    /// Construct the PropertyList 
    //
    /// @param obj      The as_object to which this PropertyList belongs.
    PropertyList(as_object& obj);

    /// Visit properties 
    //
    /// The method will invoke the given visitor method
    /// passing it two arguments: name of the property and
    /// value of it.
    //
    /// @tparam V       The type of the visitor.
    /// @tparam U       An object that may check property values. The object's
    ///                 operator() should return false if the property is not
    ///                 acceptable.
    //
    /// @param visitor  The visitor function. It must implement the function:
    ///                     bool accept(const ObjectURI&, const as_value&);
    ///                 Scan is by enumeration order and stops when accept()
    ///                 returns false.
    template <class U, class V>
    void visitValues(V& visitor, U cmp = U()) const {

        for (const_iterator it = _props.begin(), ie = _props.end();
                it != ie; ++it) {

            if (!cmp(*it)) continue;
            as_value val = it->getValue(_owner);
            if (!visitor.accept(it->uri(), val)) return;
        }
    }

    /// Enumerate all non-hidden properties to the given as_environment.
    //
    /// Follows enumeration order. Note that this enumeration does not
    /// access the values. Accessing the values can result in changes to
    /// the object if the value is a getter-setter, and key enumeration must
    /// avoid this.
    ///
    /// @param donelist     Don't enumerate properties in donelist.
    ///                     Enumerated properties are added to donelist.
    void visitKeys(KeyVisitor& v, PropertyTracker& donelist) const;

    /// Set the value of a property, creating a new one if it doesn't exist.
    //
    /// If the named property is a getter/setter one it's setter
    /// will be invoked using the given as_object as 'this' pointer.
    /// If the property is not found a SimpleProperty will be created.
    ///
    /// @param uri
    ///    Name of the property.
    /// @param value
    ///    a const reference to the as_value to use for setting
    ///    or creating the property. 
    /// @param flagsIfMissing
    ///    Flags to associate to the property if a new one is created.
    /// @return true if the value was successfully set, false
    ///         otherwise (found a read-only property, most likely).
    bool setValue(const ObjectURI& uri, const as_value& value,
            const PropFlags& flagsIfMissing = 0);

    /// Get a property if it exists.
    //
    /// @param uri  Name of the property. 
    /// @return     A Property or 0, if no such property exists.
    ///             All Property objects are owned by this PropertyList. Do
    ///             not delete them.
    Property* getProperty(const ObjectURI& uri) const;

    /// Delete a Property, if existing and not protected from deletion.
    //
    ///
    /// @param uri      Name of the property.
    /// @return         a pair of boolean values expressing whether the property
    ///                 was found (first) and whether it was deleted (second).
    ///                 Of course a pair(false, true) would be invalid (deleted
    ///                 a non-found property!?). Valid returns are:
    ///                     - (false, false) : property not found
    ///                     - (true, false) : property protected from deletion
    ///                     - (true, true) : property successfully deleted
    std::pair<bool,bool> delProperty(const ObjectURI& uri);

    /// Add a getter/setter property, if not already existing
    //
    /// TODO: this function has far too many arguments.
    //
    /// @param uri      Name of the property. 
    /// @param getter   A function to invoke when this property value is
    ///                 requested. 
    /// @param setter   A function to invoke when setting this property's value.
    /// @param cacheVal The value to use as a cache. If null uses any cache
    ///                 from pre-existing property with same name.
    /// @param flagsIfMissing Flags to associate to the property if a new one
    ///                       is created.
    /// @return         true if the property was successfully added, false
    ///                 otherwise.
    bool addGetterSetter(const ObjectURI& uri, as_function& getter,
        as_function* setter, const as_value& cacheVal,
        const PropFlags& flagsIfMissing = 0);

    /// Add a getter/setter property, if not already existing
    //
    /// @param uri      Name of the property.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param setter   A function to invoke when setting this property's value.
    /// @return         true if the property was successfully added, false
    ///                 otherwise.
    bool addGetterSetter(const ObjectURI& uri, as_c_function_ptr getter,
        as_c_function_ptr setter, const PropFlags& flagsIfMissing);

    /// Add a destructive getter property, if not already existant.
    //
    /// @param uri      Name of the property.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param flagsIfMissing Flags to associate to the property if a new
    ///                             one is created.
    /// @return         true if the property was successfully added.
    bool addDestructiveGetter(const ObjectURI& uri, as_function& getter,
        const PropFlags& flagsIfMissing = 0);

    /// Add a destructive getter property, if not already existant.
    ///
    /// @param uri      Name of the property. 
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    ///
    /// @param flagsIfMissing   Flags to associate to the property if a new
    //                          one is created.
    /// @return         true if the property was successfully added, false
    ///                 otherwise.
    bool addDestructiveGetter(const ObjectURI& uri, as_c_function_ptr getter, 
        const PropFlags& flagsIfMissing = 0);

    /// Set the flags of a property.
    //
    /// @param uri      Name of the property. 
    /// @param setTrue  The set of flags to set
    /// @param setFalse The set of flags to clear
    void setFlags(const ObjectURI& uri, int setTrue, int setFalse);

    /// Set the flags of all properties.
    //
    /// @param setTrue      The set of flags to set
    /// @param setFalse     The set of flags to clear
    void setFlagsAll(int setTrue, int setFalse);

    /// Remove all entries in the container
    void clear();

    /// Return number of properties in this list
    size_t size() const {
        return _props.size();
    }

    /// Dump all members (using log_debug)
    //
    /// This does not reflect the normal enumeration order. It is sorted
    /// lexicographically by property.
    void dump();

    /// Mark all properties reachable
    //
    /// This can be called very frequently, so is inlined to allow the
    /// compiler to optimize it.
    void setReachable() const {
        std::for_each(_props.begin(), _props.end(),
                boost::mem_fn(&Property::setReachable));
    }

private:

    container _props;

    as_object& _owner;

};


} // namespace gnash

#endif // GNASH_PROPERTYLIST_H
