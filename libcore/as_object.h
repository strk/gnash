// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_AS_OBJECT_H
#define GNASH_AS_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "string_table.h"
#include "GC.h" // for inheritance from GcResource (to complete)
#include "PropertyList.h"
#include "as_value.h" // for return of get_primitive_value
#include "smart_ptr.h"
#include "PropFlags.h" // for enum
#include "GnashException.h"
#include "Relay.h"
#include "ObjectURI.h"

#include <cmath>
#include <utility> // for std::pair
#include <set>
#include <sstream>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <deque>

// Forward declarations
namespace gnash {
    namespace abc {
        class Machine;
        class Class;
    }
    class as_function;
    class MovieClip;
    class DisplayObject;
    class as_environment;
    class VM;
    class IOChannel;
    class movie_root;
    class RunResources;
    class Global_as;
}

namespace gnash {


/// An abstract property visitor
class AbstractPropertyVisitor {
public:

    /// This function should return false if no further visits are needed.
    virtual bool accept(const ObjectURI& uri, const as_value& val) = 0;
    virtual ~AbstractPropertyVisitor() {}
};

/// A trigger that can be associated with a property name
class Trigger
{
public:

    Trigger(const std::string& propname, as_function& trig,
            const as_value& customArg)
        :
        _propname(propname),
        _func(&trig),
        _customArg(customArg),
        _executing(false),
        _dead(false)
    {}

    /// Call the trigger
    //
    /// @param oldval
    ///    Old value being modified
    ///
    /// @param newval
    ///    New value requested
    /// 
    /// @param this_obj
    ///     Object of which the property is being changed
    ///
    as_value call(const as_value& oldval, const as_value& newval, 
            as_object& this_obj);

    /// True if this Trigger has been disposed of.
    bool dead() const { return _dead; }

    void kill() {
        _dead = true;
    }

    void setReachable() const;

private:

    /// Name of the property
    //
    /// By storing a string_table::key we'd save CPU cycles
    /// while adding/removing triggers and some memory
    /// on each trigger, but at the cost of looking up
    /// the string_table on every invocation of the watch...
    ///
    std::string _propname;

    /// The trigger function 
    as_function* _func;

    /// A custom argument to pass to the trigger
    /// after old and new value.
    as_value _customArg;

    /// Flag to protect from infinite loops
    bool _executing;

    /// Flag to check whether this trigger has been deleted.
    //
    /// As a trigger can be removed during execution, it shouldn't be
    /// erased from the container straight away, so this flag prevents
    /// any execution.
    bool _dead;

};


/// \brief
/// A generic bag of attributes. Base class for all ActionScript-able objects.
//
/// Base-class for ActionScript script-defined objects.
/// This would likely be ActionScript's 'Object' class.
///
class as_object : public GcResource, boost::noncopyable
{

public:
    
    typedef std::pair<std::string, std::string> KeyValuePair;

    /// This is used to hold an intermediate copy of an as_object's properties.
    //
    /// AS enumerates in reverse order of creation. In order to make sure
    /// that the properties are in the correct order, the first element of
    /// a SortedPropertyList should hold the last created property.
    //
    /// We use a deque because we push to the front in order to preserve the
    /// ordering for the copy.
    typedef std::deque<KeyValuePair> SortedPropertyList;
    
    /// Construct an ActionScript object with no prototype associated.
    //
    /// @param  global  A reference to the Global object the new
    ///                 object ultimately belongs to. The created object
    ///                 uses the resources of the Global object.
    explicit as_object(Global_as& global);

    /// Construct an ActionScript object with no prototype associated.
    //
    /// This constructor is deprecated!
    as_object();

    /// Function dispatch
    //
    /// Various objects can be called, including functions and super objects.
    /// A normal object has no call functionality, so the default
    /// implementation throws an ActionTypeError.
    virtual as_value call(const fn_call& fn);

    /// Return the string representation for this object
    //
    /// This is dependent on the VM version and the type of object, function,
    /// or class.
    virtual const std::string& stringValue() const;

    /// The most common flags for built-in properties.
    //
    /// Most API properties, including classes and objects, have these flags.
    static const int DefaultFlags = PropFlags::dontDelete |
                                    PropFlags::dontEnum;

    /// Find a property scanning the inheritance chain
    ///
    /// @param name
    /// The string id to look for
    ///
    /// @param nsname
    /// The namespace id to look for, 0 for any.
    ///
    /// @param owner
    /// If not null, this is set to the object which contained the property.
    ///
    /// @returns a Property if found, NULL if not found
    ///          or not visible in current VM version
    ///
    Property* findProperty(const ObjectURI& uri, as_object **owner = NULL);

    /// Return a reference to this as_object's global object.
    VM& vm() const {
        return _vm;
    }

    /// Dump all properties using log_debug
    //
    /// Note that this method is non-const
    /// as some properties might be getter/setter
    /// ones, thus simple read of them might execute
    /// user code actually changing the object itsef.
    ///
    void dump_members();

    /// Dump all properties into the given container
    //
    /// Note that this method is non-const
    /// as some properties might be getter/setter
    /// ones, thus simple read of them might execute
    /// user code actually changing the object itsef.
    ///
    void dump_members(std::map<std::string, as_value>& to);

    /// Set a member value
    //
    ///
    /// @param key
    ///    Id of the property. 
    ///
    /// @param val
    ///    Value to assign to the named property.
    ///
    /// @param nsname
    ///    Id of the namespace.
    ///
    /// @param ifFound
    ///    If true, don't create a new member, but only update
    ///    an existing one.
    ///
    /// @return true if the given member existed, false otherwise.
    ///    NOTE: the return doesn't tell if the member exists after
    ///          the call, as watch triggers might have deleted it
    ///          after setting.
    ///    
    ///
    virtual bool set_member(const ObjectURI& uri, const as_value& val,
        bool ifFound = false);

    /// Reserve a slot
    ///
    /// Reserves a slot for a property to follow.
    void reserveSlot(const ObjectURI& uri, boost::uint16_t slotId);

    /// Initialize a member value by string
    //
    /// This is just a wrapper around the other init_member method
    /// used as a trampoline to avoid changing all classes to 
    /// use string_table::key directly.
    ///
    /// @param name
    ///     Name of the member.
    ///    Will be converted to lowercase if VM is initialized for SWF6 or lower.
    ///
    /// @param val
    ///     Value to assign to the member.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    void init_member(const std::string& name, const as_value& val, 
        int flags = DefaultFlags, string_table::key nsname = 0);

    /// Initialize a member value by key
    //
    /// This method has to be used by built-in classes initialization
    /// (VM initialization in general) as will avoid to scan the
    /// inheritance chain.
    ///
    /// By default, members initialized by calling this function will
    /// be protected from deletion and not shown in enumeration.
    /// These flags can be explicitly set using the third argument.
    ///
    /// @param key
    ///     Member key.
    ///
    /// @param val
    ///     Value to assign to the member.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    ///
    /// @param slotId
    /// If this is a non-negative value which will fit in an unsigned short,
    /// this is used as the slotId and can be subsequently found with
    /// get_slot
    ///
    void init_member(const ObjectURI& uri, const as_value& val, 
        int flags = DefaultFlags, int slotId = -1);

    /// \brief
    /// Initialize a getter/setter property by name
    //
    /// This is just a wrapper around the other init_property method
    /// used as a trampoline to avoid changing all classes to 
    /// use string_table::key directly.
    ///
    /// @param key
    ///     Name of the property.
    ///    Will be converted to lowercase if VM is initialized for SWF6 or lower.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param setter
    ///    A function to invoke when setting this property's value.
    ///    add_ref will be called on the function.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    void init_property(const std::string& key, as_function& getter,
        as_function& setter, int flags = DefaultFlags,
        string_table::key nsname = 0);

    /// \brief
    /// Initialize a getter/setter property by name
    //
    /// This is just a wrapper around the other init_property method
    /// used as a trampoline to avoid changing all classes to 
    /// use string_table::key directly.
    ///
    /// @param key
    ///     Name of the property.
    ///    Will be converted to lowercase if VM is initialized for SWF6 or lower.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param setter
    ///    A function to invoke when setting this property's value.
    ///    add_ref will be called on the function.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    void init_property(const std::string& key, as_c_function_ptr getter,
        as_c_function_ptr setter, int flags = DefaultFlags,
        string_table::key nsname = 0);

    /// \brief
    /// Initialize a getter/setter property by key
    //
    /// This method has to be used by built-in classes initialization
    /// (VM initialization in general) as will avoid to scan the
    /// inheritance chain.
    ///
    /// @param key
    ///     Key of the property.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param setter
    ///    A function to invoke when setting this property's value.
    ///    add_ref will be called on the function.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    void init_property(const ObjectURI& uri, as_function& getter,
        as_function& setter, int flags = DefaultFlags);

    /// \brief
    /// Initialize a getter/setter property by key
    //
    /// This method has to be used by built-in classes initialization
    /// (VM initialization in general) as will avoid to scan the
    /// inheritance chain.
    ///
    /// @param key
    ///     Key of the property.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param setter
    ///    A function to invoke when setting this property's value.
    ///    add_ref will be called on the function.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    void init_property(const ObjectURI& uri, as_c_function_ptr getter,
        as_c_function_ptr setter, int flags = DefaultFlags);


    /// \brief
    /// Initialize a destructive getter property
    ///
    /// A destructive getter can be used as a place holder for the real
    /// value of a property.  As soon as getValue is invoked on the getter,
    /// it destroys itself after setting its property to the return value of
    /// getValue.
    ///
    /// @param key
    ///     Name of the property.
    ///    Will be converted to lowercase if VM is initialized for SWF6 or lower.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    ///
    bool init_destructive_property(const ObjectURI& uri, as_function& getter,
            int flags = PropFlags::dontEnum);

    /// \brief
    /// Initialize a destructive getter property
    ///
    /// A destructive getter can be used as a place holder for the real
    /// value of a property.  As soon as getValue is invoked on the getter,
    /// it destroys itself after setting its property to the return value of
    /// getValue.
    ///
    /// @param key
    ///     Name of the property.
    ///    Will be converted to lowercase if VM is initialized for SWF6 or lower.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param flags
    ///     Flags for the new member. By default dontDelete and dontEnum.
    ///    See PropFlags::Flags.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    ///
    bool init_destructive_property(const ObjectURI& uri, 
            as_c_function_ptr getter, int flags = PropFlags::dontEnum);


    /// \brief
    /// Use this method for read-only properties.
    //
    /// This method achieves the same as the above init_property method.
    /// Additionally, it sets the property as read-only so that a default
    /// handler will be triggered when ActionScript attempts to set the
    /// property.
    /// 
    /// The arguments are the same as the above init_property arguments,
    /// although the setter argument is omitted.
    ///
    /// @param key
    ///     Property name id
    ///
    /// @param getter
    ///     The getter function
    ///
    /// @param flags
    ///     Property flags
    ///
    /// @param nsname
    ///     The id of the namespace to which this member belongs.
    ///     0 is a wildcard and will be matched by anything not asking
    ///     for a specific namespace.
    ///
    ///
    void init_readonly_property(const std::string& key, as_function& getter,
            int flags = DefaultFlags, string_table::key nsname = 0);

    void init_readonly_property(const ObjectURI& uri,
            as_function& getter, int flags = DefaultFlags);

    /// Use this method for read-only properties.
    //
    /// This method achieves the same as the above init_property method.
    /// Additionally, it sets the property as read-only so that a default
    /// handler will be triggered when ActionScript attempts to set the
    /// property.
    /// 
    /// The arguments are the same as the above init_property arguments,
    /// although the setter argument is omitted.
    ///
    /// @param key
    ///     Property name id
    ///
    /// @param getter
    ///     The getter function
    ///
    /// @param flags
    ///     Property flags
    ///
    /// @param nsname
    ///     The id of the namespace to which this member belongs.
    ///     0 is a wildcard and will be matched by anything not asking
    ///     for a specific namespace.
    ///
    void init_readonly_property(const std::string& key,
            as_c_function_ptr getter, int flags = DefaultFlags,
            string_table::key nsname = 0);

    void init_readonly_property(const ObjectURI& uri,
            as_c_function_ptr getter, int flags = DefaultFlags);


    /// Enumerate all non-hidden property keys to the given as_environment.
    //
    /// NB: this function does not access the property values, so callers
    /// can be certain no values will be changed.
    //
    /// The enumeration recurses through the prototype chain. This
    /// implementation will keep track of visited object to avoid infinite
    /// loops in the prototype chain.  NOTE: the MM player just chokes in
    /// this case.
    void enumeratePropertyKeys(as_environment& env) const;

    /// \brief
    /// Add a watch trigger, overriding any other defined for same name.
    //
    /// @param key
    ///    property name (key)
    ///
    /// @param ns
    ///    property namespace.
    ///
    /// @param trig
    ///    A function to invoke when this property value is assigned to.
    ///    The function will be called with old val, new val and the custom
    ///    value below. It's return code will be used to set actual value
    ///
    /// @param cust
    ///    Custom value to always pass to the trigger as third arg
    ///
    /// @return true if the trigger was successfully added, false
    ///         otherwise (error? should always be possible to add...)
    ///
    bool watch(const ObjectURI& uri, as_function& trig, const as_value& cust);

    /// \brief
    /// Remove a watch trigger.
    //
    /// @param key
    ///    property name (key)
    ///
    /// @param ns
    ///    property namespace.
    ///
    /// @return true if the trigger was successfully removed, false
    ///         otherwise (no such trigger...)
    ///
    bool unwatch(const ObjectURI& uri);

    /// Get a member as_value by name
    ///
    /// NOTE that this method is non-const becase a property
    ///      could also be a getter/setter and we can't promise
    ///      that the 'getter' won't change this object trough
    ///     use of the 'this' reference. 
    ///
    /// @param name
    ///    Name of the property. Must be all lowercase
    ///    if the current VM is initialized for a  target
    ///    up to SWF6.
    ///
    /// @param val
    ///    Variable to assign an existing value to.
    ///    Will be untouched if no property with the given name
    ///    was found.
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    ///
    /// @return true of the named property was found, false otherwise.
    ///
    virtual bool get_member(const ObjectURI& uri, as_value* val);

    /// Resolve the given relative path component
    //
    /// Path components are only objects, if the given string
    /// points to a non-object member, NULL is returned.
    ///
    /// Main use if for getvariable and settarget resolution,
    /// currently implemented in as_environment.
    ///
    ///
    virtual as_object* get_path_element(string_table::key key);

    /// Get the super object of this object.
    ///
    /// The super should be __proto__ if this is a prototype object
    /// itself, or __proto__.__proto__ if this is not a prototype
    /// object. This is only conceptual however, and may be more
    /// convoluted to obtain the actual super.
    virtual as_object* get_super(string_table::key fname = 0);

    /// Get the constructor for this object.
    ///
    /// This is the AS constructor for this object. When invoked, it
    /// should initialize the object passed as 'this'
    as_function* get_constructor();

    /// Get a member as_value by name in an AS-compatible way
    //
    /// NOTE that this method is non-const becase a property
    ///      could also be a getter/setter and we can't promise
    ///      that the 'getter' won't change this object trough
    ///     use of the 'this' reference. 
    ///
    /// @param uri      Name and namespace of the property. Note that
    ///                 if you do not care about the namespace (AS2 does not),
    ///                 you can call this function with the name key only.
    ///
    /// @return value of the member (possibly undefined),
    ///    or undefined if not found. Use get_member if you
    ///    need to know wheter it was found or not.
    ///
    as_value getMember(const ObjectURI& uri);

    /// Delete a property of this object, unless protected from deletion.
    //
    /// This function does *not* recurse in this object's prototype.
    //
    /// @param uri      Name and namespace of the property. Note that
    ///                 if you do not care about the namespace (AS2 does not),
    ///                 you can call this function with the name key only.
    ///
    /// @return a pair of boolean values expressing whether the property
    ///    was found (first) and whether it was deleted (second).
    ///    Of course a pair(false, true) would be invalid (deleted
    ///    a non-found property!?). Valid returns are:
    ///    - (false, false) : property not found
    ///    - (true, false) : property protected from deletion
    ///    - (true, true) : property successfully deleted
    ///
    std::pair<bool,bool> delProperty(const ObjectURI& uri);

    /// Get this object's own named property, if existing.
    //
    /// This function does *not* recurse in this object's prototype.
    //
    /// @param uri      The name and namespace of the property. Note that
    ///                 if you do not care about the namespace (AS2 does not),
    ///                 you can call this function with the name key only.
    /// @return         A Property pointer, or NULL if this object doesn't
    ///                 contain the named property.
    Property* getOwnProperty(const ObjectURI& uri);

    /// Return true if this object has the named property
    //
    /// @param uri      Name and namespace of the property. Note that
    ///                 if you do not care about the namespace (AS2 does not),
    ///                 you can call this function with the name key only.
    ///
    /// @return
    ///    true if the object has the property, false otherwise.
    ///
    bool hasOwnProperty(const ObjectURI& uri);

    /// Get a property from this object (or a prototype) by ordering index.
    ///
    /// @param index
    /// An index returned by nextIndex
    ///
    /// @return
    /// The property associated with the order index.
    const Property* getByIndex(int index);

    /// Get the next index after the one whose index was used as a parameter.
    ///
    /// @param index
    /// 0 is a starter index -- use it to get the first index. Using the
    /// return value in subsequent calls will walk through all enumerable
    /// properties in the list.
    ///
    /// @param owner
    /// If owner is not NULL, it will be set to the exact object to which
    /// the property used for the value of index belongs, if such a property
    /// exists, and left untouched otherwise.
    ///
    /// @return
    /// A value which can be fed to getByIndex, or 0 if there are no more.
    int nextIndex(int index, as_object **owner = NULL);

    /// Set member flags (probably used by ASSetPropFlags)
    //
    /// @param name
    ///    Name of the property. Must be all lowercase
    ///    if the current VM is initialized for a  target
    ///    up to SWF6.
    ///
    /// @param setTrue
    ///    the set of flags to set
    ///
    /// @param setFalse
    ///    the set of flags to clear
    ///
    /// @param nsname
    /// The id of the namespace to which this member belongs. 0 is a wildcard
    /// and will be matched by anything not asking for a specific namespace.
    ///
    /// @return true on success, false on failure
    ///    (non-existent or protected member)
    ///
    bool set_member_flags(const ObjectURI& uri, int setTrue, int setFalse = 0);

    /// Cast to a as_function, or return NULL
    virtual as_function* to_function() { return NULL; }

    /// Return true if this is a 'super' object
    virtual bool isSuper() const { return false; }

    /// Add an interface to the list of interfaces.
    //
    /// This is used by the action "implements". This opcode is a compile-time
    /// promise that a class will implement all the methods of an
    /// otherwise unrelated interface class. The only use in AVM1 is to
    /// allow instanceOf to return true when a class implements another
    /// class.
    void addInterface(as_object* ctor);

    /// \brief
    /// Check whether this object is an instance of the given
    /// constructor
    //
    /// NOTE: built-in classes should NOT be C_FUNCTIONS for this to
    /// work
    ///
    bool instanceOf(as_object* ctor);

    /// \brief
    /// Check whether this object is a 'prototype' in the given
    /// object's inheritance chain.
    //
    bool prototypeOf(as_object& instance);

    /// Set property flags
    //
    /// @param props
    ///    A comma-delimited list of property names as a string,
    ///    a NULL value, or an array? (need to check/test, probably
    ///     somehting is broken).
    ///    Property strings are case insensitive up to SWF6,
    ///    case *sensitive* from SWF7 up.
    ///    
    ///
    /// @param set_false
    /// @param set_true
    ///
    void setPropFlags(const as_value& props, int set_false, int set_true);

#ifdef USE_DEBUGGER
    /// Get the properties of this object
    PropertyList &get_properties() { return _members; };
#endif

    /// Copy properties from the given object
    //
    /// NOTE: the __proto__ member will NOT be copied.
    ///
    void copyProperties(const as_object& o);

    /// Drop all properties from this object
    void clearProperties()
    {
        _members.clear();
    }

    /// Visit the properties of this object by key/as_value pairs
    //
    /// The method will invoke the given visitor method
    /// passing it two arguments: key of the property and
    /// value of it.
    ///
    /// @param visitor
    ///    The visitor function. Will be invoked for each property
    ///    of this object with a string_table::key
    ///    reference as first argument and a const as_value reference
    ///    as second argument.
    ///
    template<typename T>
    void visitProperties(AbstractPropertyVisitor& visitor) const {
        _members.visitValues<T>(visitor);
    }

    /// \brief
    /// Add a getter/setter property, if no member already has
    /// that name (or should we allow override ? TODO: check this)
    //
    /// @param key
    ///     Name of the property.
    ///    Case insensitive up to SWF6,
    ///    case *sensitive* from SWF7 up.
    ///
    /// @param getter
    ///    A function to invoke when this property value is requested.
    ///    add_ref will be called on the function.
    ///
    /// @param setter
    ///    A function to invoke when setting this property's value.
    ///    By passing NULL, the property will have no setter
    ///    (valid ActionScript - see actionscript.all/Object.as)
    void add_property(const std::string& key, as_function& getter,
        as_function* setter);

    /// Return this object '__proto__' member.
    //
    /// The __proto__ member is the exported interface ('prototype')
    /// of the class this object is an instance of.
    ///
    /// NOTE: can return NULL (and it is expected to do for Object.prototype)
    as_object* get_prototype() const;

    /// Set this object's '__proto__' member
    //
    /// This does more or less what set_member("__proto__") does, but without
    /// the lookup process.
    void set_prototype(const as_value& proto);

    /// Set the as_object's Relay object.
    //
    /// This is a pointer to a native object that contains special type
    /// characteristics. Setting the Relay object allows native functions
    /// to get or set non-ActionScript properties.
    //
    /// This function should only be used in native functions such as
    /// constructors and special creation functions like
    /// MovieClip.createTextField(). As Relay objects are not available to
    /// ActionScript, this should never appear in built-in functions.
    //
    /// This function also removes Array typing from an object when a Relay
    /// is assigned. There are tests verifying this behaviour in
    /// actionscript.all and the swfdec testsuite.
    void setRelay(Relay* p) {
        if (p) _array = false;
        _relay.reset(p);
    }

    /// Access the as_object's Relay object.
    //
    /// The Relay object is a polymorphic object containing native type
    /// characteristics. It is rarely useful to use this function directly.
    /// Instead use the convenience functions ensure<>() and
    /// isNativeType() to access the Relay object.
    //
    /// Relay objects are not available to ActionScript, so this object
    /// should not be used in built-in functions (that is, functions
    /// implemented in ActionScript).
    Relay* relay() const {
        return _relay.get();
    }

    /// Return true if this object should be treated as an array.
    bool array() const {
        return _array;
    }

    /// Set whether this object should be treated as an array.
    void setArray(bool array = true) {
        _array = array;
    }

    /// Return true if this is a DisplayObject.
    DisplayObject* displayObject() const {
        return _displayObject;
    }

    /// Indicate that this object is a DisplayObject
    //
    /// This enables DisplayObject properties such as _x and _y. A flag
    /// is used to avoid RTTI on every get and set of properties.
    void setDisplayObject(DisplayObject* d) {
        _displayObject = d;
    }

    ///Get a member value at a given slot.
    //
    /// @param order
    /// The slot index of the property.
    ///
    /// @param val
    /// The as_value to store a found variable's value in.
    ///
    /// @return true if a member exists at the given slot, 
    /// and the member's value is successfully retrieved,
    /// false otherwise.
    bool get_member_slot(int order, as_value* val);

    ///Set a member value at a given slot.
    //
    ///This is a wrapper around set_member_default.
    /// @param order
    ///
    /// The slot index of the property.
    /// @param val
    ///    Value to assign to the named property.
    ///
    /// @param ifFound
    ///    If true, don't create a new member, but only update
    ///    an existing one.
    ///
    /// @return true if the member exists at the given slot, 
    /// false otherwise.
    ///    NOTE: the return doesn't tell if the member exists after
    ///          the call, as watch triggers might have deleted it
    ///          after setting.
    ///
    bool set_member_slot(int order, const as_value& val, bool ifFound = false);

protected:

    /// Mark all reachable resources, override from GcResource.
    //
    /// The default implementation marks all properties
    /// as being reachable, calling markAsObjectReachable().
    ///
    /// If a derived class provides access to more GC-managed
    /// resources, it should override this method and call 
    /// markAsObjectReachable() as the last step.
    virtual void markReachableResources() const {
        markAsObjectReachable();
    }

    /// Mark properties and triggers list as reachable (for the GC)
    void markAsObjectReachable() const;

private:

    /// A utility class for processing this as_object's inheritance chain
    template<typename T> class PrototypeRecursor;

    /// DisplayObjects have properties not in the AS inheritance chain
    //
    /// These magic properties are invoked in get_member only if the
    /// object is a DisplayObject
    DisplayObject* _displayObject;

    /// An array is a special type of object.
    //
    /// Like DisplayObjects, Arrays handle property setting differently. We
    /// use an extra flag to avoid checking Relay type on every property
    /// set, but tests show that the Array constructor removes the Relay. It
    /// would be possible to implement using a Relay, but as an Array stores
    /// no extra native data, it's not clear what the point is.
    bool _array;

    /// The polymorphic Relay object for native types.
    //
    /// This is owned by the as_object and destroyed when the as_object's
    /// destructor is called.
    boost::scoped_ptr<Relay> _relay;

    /// The VM containing this object.
    VM& _vm;   

    /// Properties of this as_object
    PropertyList _members;

    /// \brief
    /// Find an existing property for update, only scanning the
    /// inheritance chain for getter/setters or statics.
    //
    /// NOTE: updatable here doesn't mean the property isn't protected
    /// from update but only that a set_member will NOT create a new
    /// property (either completely new or as an override).
    ///
    /// @returns a property if found, NULL if not found
    ///          or not visible in current VM version
    ///
    Property* findUpdatableProperty(const ObjectURI& uri);

    void executeTriggers(Property* prop, const ObjectURI& uri,
            const as_value& val);

    /// The constructors of the objects implemented by this as_object.
    //
    /// There is no need to use a complex container as the list of 
    /// interfaces is generally small and the opcode rarely used anyway.
    std::vector<as_object*> _interfaces;

    typedef std::map<ObjectURI, Trigger> TriggerContainer;
    boost::scoped_ptr<TriggerContainer> _trigs;
};

/// Function objects for visiting properties.
class IsVisible
{
public:
    IsVisible(int version) : _version(version) {}
    bool operator()(const Property& prop) const {
        return prop.visible(_version);
    }
private:
    const int _version;
};

class Exists
{
public:
    Exists() {}
    bool operator()(const Property&) const {
        return true;
    }
};

class IsEnumerable
{
public:
    IsEnumerable() {}
    bool operator()(const Property& p) const {
        return !p.getFlags().get_dont_enum();
    }
};

/// Get url-encoded variables
//
/// This method will be used for loadVariables and loadMovie
/// calls, to encode variables for sending over a network.
/// Variables starting with a dollar sign will be skipped,
/// as non-enumerable ones.
//
/// @param o        The object whose properties should be encoded.
/// @param data     Output parameter, will be set to the url-encoded
///                 variables string without any leading delimiter.
void getURLEncodedVars(as_object& o, std::string& data);


/// Extract the DisplayObject attached to an object
//
/// @return     0 if no DisplayObject is attached, or if it is not the
///             requested type
/// @param o    The object to check.
template<typename T>
T*
get(as_object* o)
{
    if (!o) return 0;
    return dynamic_cast<T*>(o->displayObject());
}

as_object* getObjectWithPrototype(Global_as& gl, string_table::key c);

/// Check whether the object is an instance of a known type.
//
/// This is used to check the type of certain objects when it can't be
/// done through ActionScript and properties. Examples include conversion
/// of Date and String objects.
//
/// @tparam T       The expected native type
/// @param obj      The object whose type should be tested
/// @param relay    This points to the native type information if the object
///                 is of the expected type
/// @return         If the object is of the expected type, true; otherwise
///                 false.
template<typename T>
bool
isNativeType(as_object* obj, T*& relay)
{
    if (!obj) return false;
    relay = dynamic_cast<T*>(obj->relay());
    return relay;
}

/// Enumerate all non-hidden properties to the passed container
//
/// NB: it is likely that this call will change the object, as accessing
/// propertyproperty  values may call getter-setters.
//
/// The enumeration recurses through the prototype chain. This implementation
/// will keep track of visited object to avoid infinite loops in the
/// prototype chain.  NOTE: the MM player just chokes in this case.
void enumerateProperties(as_object& o, as_object::SortedPropertyList& to);

/// Get the VM from an as_object.
VM& getVM(const as_object& o);

/// Get the movie_root from an as_object.
movie_root& getRoot(const as_object& o);

/// Get the string_table from an as_object.
string_table& getStringTable(const as_object& o);

/// Get the RunResources from an as_object.
const RunResources& getRunResources(const as_object& o);

/// Get the executing VM version from an as_object.
int getSWFVersion(const as_object& o);

/// Get the Global object from an as_object.
Global_as& getGlobal(const as_object& o);

/// Return whether the object is an AS3 object.
bool isAS3(const as_object& o);


} // namespace gnash

#endif // GNASH_AS_OBJECT_H
