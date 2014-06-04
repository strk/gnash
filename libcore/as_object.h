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

#ifndef GNASH_AS_OBJECT_H
#define GNASH_AS_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <vector>
#include <cmath>
#include <utility> 
#include <memory>
#include <boost/noncopyable.hpp>

#include "GC.h" // for inheritance from GcResource (to complete)
#include "PropertyList.h"
#include "PropFlags.h"
#include "Relay.h"
#include "ObjectURI.h"
#include "dsodefs.h" // for DSOTEXPORT

// Forward declarations
namespace gnash {
    class as_function;
    class MovieClip;
    class DisplayObject;
    class as_environment;
    class VM;
    class IOChannel;
    class movie_root;
    class RunResources;
    class Global_as;
    class as_value;
    class string_table;
}

namespace gnash {


/// A trigger that can be associated with a property name
class Trigger
{
public:

    Trigger(std::string propname, as_function& trig,
            as_value customArg)
        :
        _propname(std::move(propname)),
        _func(&trig),
        _customArg(std::move(customArg)),
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

/// The base class for all ActionScript objects
//
/// Everything in ActionScript is an object or convertible to an object. This
/// class is the base class for all object types and the implementation of the
/// ActionScript Object type itself. See asobj/Object.cpp for the ActionScript
/// Object interface.
//
/// An AS2 object has 3 principle tasks:
//
/// 1. to store a set of ActionScript properties and to control dynamic access
///    to them.
/// 2. to store native type information.
/// 3. to store 'watches' that report on changes to any member property.
//
/// A fourth but relatively minor task is to store a list of implemented
/// interfaces (see as_object::instanceOf()).
//
/// ActionScript has two different levels of Object typing:
//
/// Dynamic Typing
//
/// 1a. Native type information, stored in a Relay and inaccessible
///     to ActionScript.
/// 1b. Array type information. This is very similar (and possibly identical
///     to) Relays.
/// 2.  Link to a DisplayObject.
//
/// Both these dynamic types can be changed independently at runtime using
/// native functions (often, but not always, constructor functions).
//
/// Static Typing
//
/// Functions (as_function), Super objects (as_super) and AS3 Class types
/// (as_class) have a static type, that is, they are not convertible to each
/// other once created.
class as_object : public GcResource, boost::noncopyable
{

public:
    
    /// Construct an ActionScript object with no prototype associated.
    //
    /// @param  global  A reference to the Global object the new
    ///                 object ultimately belongs to. The created object
    ///                 uses the resources of the Global object.
    explicit DSOTEXPORT as_object(const Global_as& global);

    /// The as_object dtor does nothing special.
    virtual ~as_object() {}

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
    virtual std::string stringValue() const;

    /// The most common flags for built-in properties.
    //
    /// Most API properties, including classes and objects, have these flags.
    static const int DefaultFlags = PropFlags::dontDelete |
                                    PropFlags::dontEnum;

    /// Find a property, scanning the inheritance chain
    //
    /// @param uri      Property identifier.
    /// @param owner    If not null, this is set to the object which contained
    ///                 an inherited property.
    /// @returns        A property if found and visible, NULL if not found or
    ///                 not visible in current VM version
    Property* findProperty(const ObjectURI& uri, as_object** owner = nullptr);

    /// Return a reference to this as_object's global object.
    VM& vm() const {
        return _vm;
    }

    /// Dump all properties using log_debug
    //
    /// Note that it is very likely that this will result in changes to the
    /// object, as accessing getter/setters or destructive properties can
    /// modify properties.
    //
    /// Only use this function for temporary debugging!
    void dump_members();

    /// Set a member value
    //
    /// @param uri      Property identifier.
    /// @param val      Value to assign to the named property.
    /// @param ifFound  If true, don't create a new member, but rather only
    ///                 update an existing one.
    /// @return         true if the given member existed, false otherwise.
    ///                 NOTE: the return doesn't tell if the member exists
    ///                 after the call, as watch triggers might have deleted
    ///                 it after setting.
    virtual bool set_member(const ObjectURI& uri, const as_value& val,
        bool ifFound = false);

    /// Initialize a member value by string
    //
    /// This is just a wrapper around the other init_member method
    /// used as a trampoline to avoid changing all classes to 
    /// use string_table::key directly.
    //
    /// @param name         Name of the member.
    /// @param val          Value to assign to the member.
    /// @param flags        Flags for the new member. By default dontDelete
    ///                     and dontEnum.
    void init_member(const std::string& name, const as_value& val, 
        int flags = DefaultFlags);

    /// Initialize a member value by key
    //
    /// This method has to be used by built-in classes initialization
    /// (VM initialization in general) as will avoid to scan the
    /// inheritance chain.
    ///
    /// By default, members initialized by calling this function will
    /// be protected from deletion and not shown in enumeration.
    /// These flags can be explicitly set using the third argument.
    //
    /// @param uri      Property identifier.
    /// @param val      Value to assign to the member.
    ///
    /// @param flags    Flags for the new member. By default dontDelete
    ///                 and dontEnum.
    void init_member(const ObjectURI& uri, const as_value& val, 
        int flags = DefaultFlags);

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
    void init_property(const std::string& key, as_function& getter,
        as_function& setter, int flags = DefaultFlags);
        

    /// Initialize a getter/setter property by name
    //
    /// This is just a wrapper around the other init_property method
    /// used as a trampoline to avoid changing all classes to 
    /// use string_table::key directly.
    ///
    /// @param key      Name of the property. Will be converted to lowercase
    ///                 if VM is initialized for SWF6 or lower.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param setter   A function to invoke when setting this property's
    ///                 value.
    /// @param flags    Flags for the new member. By default dontDelete and
    ///                 dontEnum. See PropFlags::Flags.
    void init_property(const std::string& key, as_c_function_ptr getter,
        as_c_function_ptr setter, int flags = DefaultFlags);

    /// Initialize a getter/setter property by key
    //
    /// This method has to be used by built-in classes initialization
    /// (VM initialization in general) as will avoid to scan the
    /// inheritance chain.
    //
    /// @param uri      Property identifier.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param setter   A function to invoke when this property value is
    ///                 set.
    /// @param flags    Flags for the new member. By default dontEnum and
    ///                 dontDelete.
    void init_property(const ObjectURI& uri, as_function& getter,
        as_function& setter, int flags = DefaultFlags);

    /// Initialize a getter/setter property by key
    //
    /// This method has to be used by built-in classes initialization
    /// (VM initialization in general) as will avoid to scan the
    /// inheritance chain.
    ///
    /// @param uri      Property identifier.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param setter   A function to invoke when this property value is
    ///                 set.
    /// @param flags    Flags for the new member. By default dontEnum and
    ///                 dontDelete.
    void init_property(const ObjectURI& uri, as_c_function_ptr getter,
        as_c_function_ptr setter, int flags = DefaultFlags);

    /// Initialize a destructive getter property
    //
    /// A destructive getter can be used as a place holder for the real
    /// value of a property.  As soon as getValue is invoked on the getter,
    /// it destroys itself after setting its property to the return value of
    /// getValue.
    //
    /// @param uri      Property identifier.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param flags    Flags for the new member. By default dontEnum.
    bool init_destructive_property(const ObjectURI& uri, as_function& getter,
            int flags = PropFlags::dontEnum);

    /// Initialize a destructive getter property
    //
    /// A destructive getter can be used as a place holder for the real
    /// value of a property.  As soon as getValue is invoked on the getter,
    /// it destroys itself after setting its property to the return value of
    /// getValue.
    //
    /// @param uri      Property identifier.
    /// @param getter   A function to invoke when this property value is
    ///                 requested.
    /// @param flags    Flags for the new member. By default dontEnum.
    bool init_destructive_property(const ObjectURI& uri, 
            as_c_function_ptr getter, int flags = PropFlags::dontEnum);

    /// Use this method for read-only properties.
    //
    /// This method achieves the same as the above init_property method.
    /// Additionally, it sets the property as read-only so that a default
    /// handler will be triggered when ActionScript attempts to set the
    /// property.
    // 
    /// The arguments are the same as the above init_property arguments,
    /// although the setter argument is omitted.
    //
    /// @param key      Property name id
    /// @param getter   The getter function
    /// @param flags    Property flags
    void init_readonly_property(const std::string& key, as_function& getter,
            int flags = DefaultFlags);

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
    /// @param key      Property name id
    /// @param getter   The getter function
    /// @param flags    Property flags
    void init_readonly_property(const std::string& key,
            as_c_function_ptr getter, int flags = DefaultFlags);

    /// Add a watch trigger, overriding any other defined for same name.
    //
    /// @param uri      property identifier
    /// @param trig     A function to invoke when this property value is
    ///                 assigned to. The function will be called with old
    ///                 val, new val and the custom value below. Its
    ///                 return code will be used to set actual value
    /// @param cust     Custom value to always pass to the trigger as third arg
    /// @return         true if the trigger was successfully added, false
    ///                 otherwise.
    bool watch(const ObjectURI& uri, as_function& trig, const as_value& cust);

    /// Remove a watch trigger.
    //
    /// @param uri      Property identifier.
    /// @return         true if the trigger was successfully removed, false
    ///                 otherwise (no such trigger exists).
    bool unwatch(const ObjectURI& uri);

    /// Get a property by name if it exists.
    //
    /// NOTE: accessing a getter/setter property may modify the object.
    //
    /// See getMember() for a property accessor that corresponds to
    /// ActionScript behaviour.
    //
    /// @param uri      Property identifier.
    /// @param val      Variable to assign an existing value to.
    ///                 Will be untouched if no property with the given name
    ///                 was found.
    /// @return         true if the named property was found, false otherwise.
    virtual bool get_member(const ObjectURI& uri, as_value* val);

    /// Get the super object of this object.
    ///
    /// The super should be __proto__ if this is a prototype object
    /// itself, or __proto__.__proto__ if this is not a prototype
    /// object. This is only conceptual however, and may be more
    /// convoluted to obtain the actual super.
    virtual as_object* get_super(const ObjectURI& fname);
    as_object* get_super();

    /// Delete a property of this object, unless protected from deletion.
    //
    /// This function does *not* recurse in this object's prototype.
    //
    /// @param uri      Property identifier. 
    /// @return         a pair of boolean values expressing whether the property
    ///                 was found (first) and whether it was deleted (second).
    ///                 Of course a pair(false, true) would be invalid (deleted
    ///                 a non-found property!). Valid returns are:
    ///                 - (false, false) : property not found
    ///                 - (true, false) : property protected from deletion
    ///                 - (true, true) : property successfully deleted
    DSOTEXPORT std::pair<bool, bool> delProperty(const ObjectURI& uri);

    /// Get this object's own named property, if existing.
    //
    /// This function does *not* recurse in this object's prototype.
    //
    /// @param uri      Property identifier. 
    /// @return         A Property pointer, or NULL if this object doesn't
    ///                 contain the named property.
    Property* getOwnProperty(const ObjectURI& uri);

    /// Set member flags (probably used by ASSetPropFlags)
    //
    /// @param name     Name of the property. Must be all lowercase
    ///                 if the current VM is initialized for a  target
    ///                 up to SWF6.
    /// @param setTrue  The set of flags to set
    /// @param setFalse The set of flags to clear
    void set_member_flags(const ObjectURI& uri, int setTrue, int setFalse = 0);

    /// Cast to a as_function, or return NULL
    virtual as_function* to_function() { return nullptr; }

    /// Return true if this is a 'super' object
    virtual bool isSuper() const { return false; }

    /// Add an interface to the list of interfaces.
    //
    /// This is used by the action "implements". This opcode is a compile-time
    /// promise that a class will implement all the methods of an
    /// otherwise unrelated interface class. The only use in AVM1 is to
    /// allow instanceOf to return true when a class implements another
    /// class.
    //
    /// @param ctor     An as_object to specify as an interface implemented
    ///                 by this object.
    void addInterface(as_object* ctor);

    /// Check whether this object is an instance of the given constructor
    //
    /// An object is an instance of a constructor if constructor.prototype is
    /// found anywhere in the object's prototype chain (e.g. if
    /// object.__proto__ == constructor.prototype).
    //
    /// It is also an instance of a constructor if the constructor is
    /// listed in the object's interfaces (this is a compile-time promise
    /// and has no runtime effects other than for instanceOf).
    //
    /// @param ctor     The as_object to compare this object to. For more
    ///                 ActionScript-like syntax it can be any object
    ///                 or null.
    /// @return         true if this object is an instance of ctor. The return
    ///                 is always false if ctor is null.
    bool instanceOf(as_object* ctor);

    /// Check whether this object is in another object's inheritance chain.
    //
    /// This is roughly the inverse of instanceOf().
    //
    /// @param instance     The instance object to check for inheritance from
    ///                     this object.
    /// @return             true if instance inherits from this object.
    bool prototypeOf(as_object& instance);

    /// Set property flags
    //
    /// @param props    A comma-delimited list of property names as a string,
    ///                 a NULL value. This is in fact a string, which should
    ///                 be split on the ',' to an array then processed.
    ///                 TODO: this would be much better as a free function.
    //
    /// @param set_false    A mask of flags to set to false.
    /// @param set_true     A mask of flags to set to true.
    void setPropFlags(const as_value& props, int set_false, int set_true);

    /// Copy properties from the given object
    //
    /// NOTE: the __proto__ member will NOT be copied.
    //
    /// @param o    The object to copy properties from.
    void copyProperties(const as_object& o);

    /// Drop all properties from this object
    void clearProperties() {
        _members.clear();
    }

    /// Visit the properties of this object by key/as_value pairs
    //
    /// The method will invoke the given visitor method with the identifier
    /// and value of the property. Note that this access values, which may
    /// change the object.
    //
    /// @param visitor  The visitor function. Will be invoked for each property
    ///                 of this object with an ObjectURI as first argument and
    ///                 a const as_value as second argument.
    template<typename T>
    void visitProperties(PropertyVisitor& visitor) const {
        _members.visitValues<T>(visitor);
    }

    /// Visit all visible property identifiers.
    //
    /// NB: this function does not access the property values, so callers
    /// can be certain no values will be changed.
    //
    /// The enumeration recurses through the prototype chain. This
    /// implementation will keep track of visited object to avoid infinite
    /// loops in the prototype chain.  NOTE: the MM player just chokes in
    /// this case.
    //
    /// @param visitor  The visitor function. Will be invoked for each property
    ///                 of this object with an ObjectURI as the only argument.
    void visitKeys(KeyVisitor& visitor) const;

    /// Add a getter/setter property if no member already has that name.
    //
    /// @param key      Property identifier.
    /// @param getter   A function to invoke when this property value
    ///                 is requested.
    /// @param setter   A function to invoke when setting this property's
    ///                 value. By passing null, the property will have no
    ///                 setter. This is valid.
    void add_property(const std::string& key, as_function& getter,
        as_function* setter);

    /// Return this object's __proto__ member.
    //
    /// The __proto__ member is the exported interface (prototype)
    /// of the class this object is an instance of.
    ///
    /// NOTE: can return NULL (and it is expected to do for Object.prototype)
    as_object* get_prototype() const;

    /// Set this object's __proto__ member
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
        if (_relay) _relay->clean();
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

    /// Return the DisplayObject associated with this object.
    //
    /// @return     A DisplayObject if this is as_object is associated with
    ///             one, otherwise 0.
    DisplayObject* displayObject() const {
        return _displayObject;
    }

    /// Set the DisplayObject associated with this as_object.
    void setDisplayObject(DisplayObject* d) {
        _displayObject = d;
    }

protected:

    /// Construct an as_object associated with a VM.
    //
    /// This constructor is intended for subclasses. Although they could call
    /// the public constructor that accepts a Global_as, this could imply
    /// that that constructor can access members of the passed Global_as
    /// other than getVM(), which might not be available because the Global_as
    /// will not be fully constructed yet. While that is currently not the
    /// case, using this constructor eliminates this potential initialization
    /// order problem.
    /// @param vm The VM to associate the newly created as_object with.
    explicit as_object(VM& vm);

    /// Mark all reachable resources, override from GcResource.
    //
    /// The default implementation marks all properties
    ///
    /// If a derived class provides access to more GC-managed
    /// resources, it should override this function and call 
    /// this function directly as the last step.
    virtual void markReachableResources() const;

private:

    /// Find an existing property for update
    //
    /// Scans the inheritance chain only for getter/setters or statics.
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
    std::unique_ptr<Relay> _relay;

    /// The VM containing this object.
    VM& _vm;

    /// Properties of this as_object
    PropertyList _members;

    /// The constructors of the objects implemented by this as_object.
    //
    /// There is no need to use a complex container as the list of 
    /// interfaces is generally small and the opcode rarely used anyway.
    std::vector<as_object*> _interfaces;

    typedef std::map<ObjectURI, Trigger, ObjectURI::LessThan> TriggerContainer;
    std::unique_ptr<TriggerContainer> _trigs;
};

/// Send a system event
//
/// This is used for broadcasting system events. The prototype search is
/// carried out, but there is no call to __resolve and triggers
/// are not processed.
//
/// The function is called with no arguments.
//
/// @param o    The object to send the event to.
/// @param env  The environment to use, generally provided by the calling
///             DisplayObject
/// @param name The name of the function to call.
void sendEvent(as_object& o, const as_environment& env, const ObjectURI& name);

/// Get a member of an object using AS lookup rules
//
/// This is a wrapper round as_object::get_member that returns undefined if
/// the member is not found.
//
/// Note: this is the only full lookup process available in ActionScript code.
//
//
/// @param uri      Property identifier. 
/// @param o        The object whose member is required.
/// @return         Value of the member (possibly undefined),
///                 or undefined if not found. Use get_member if you
///                 need to know whether it was found or not.
inline as_value
getMember(as_object& o, const ObjectURI& uri)
{
    as_value ret;
    o.get_member(uri, &ret);
    return ret;
}

/// Get an own member of an object.
//
/// This is a wrapper round as_object::getOwnProperty that returns undefined if
/// the member is not found.
//
/// Note: this requires two steps in ActionScript (hasOwnProperty + lookup), so
/// is probably only for use in native functions.
//
/// @param uri      Property identifier.
/// @param o        The object whose own member is required.
/// @return         Value of the member (possibly undefined),
///                 or undefined if not found. Use get_member if you
///                 need to know whether it was found or not.
inline as_value
getOwnProperty(as_object& o, const ObjectURI& uri)
{
    Property* p = o.getOwnProperty(uri);
    return p ? p->getValue(o) : as_value();
}

/// Function objects for visiting properties.
class IsVisible
{
public:
    IsVisible(int version) : _version(version) {}
    bool operator()(const Property& prop) const {
        return visible(prop, _version);
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
        return !p.getFlags().test<PropFlags::dontEnum>();
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
/// @return         the url-encoded variables string without any leading
///                 delimiter.
std::string getURLEncodedVars(as_object& o);

/// Resolve the given relative path component
//
/// Path components are only objects, if the given string
/// points to a non-object member, NULL is returned.
///
/// Main use if for getvariable and settarget resolution,
/// currently implemented in as_environment.
as_object* getPathElement(as_object& o, const ObjectURI& uri);


/// Extract the DisplayObject attached to an object
//
/// @return     0 if no DisplayObject is attached, or if it is not the
///             requested type
/// @param o    The object to check.
template<typename T>
T*
get(as_object* o)
{
    if (!o) return nullptr;
    return dynamic_cast<T*>(o->displayObject());
}

/// Return true if this object has the named property
//
/// @param o        The object whose property should be searched for.
/// @param uri      Property identifier. 
/// @return         true if the object has the property, false otherwise.
inline bool
hasOwnProperty(as_object& o, const ObjectURI& uri)
{
    return (o.getOwnProperty(uri));
}

DSOTEXPORT as_object* getObjectWithPrototype(Global_as& gl, const ObjectURI& c);

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
isNativeType(const as_object* obj, T*& relay)
{
    if (!obj) return false;
    relay = dynamic_cast<T*>(obj->relay());
    return relay;
}

/// This is used to hold an intermediate copy of an as_object's properties.
//
/// AS enumerates in reverse order of creation because these values are
/// pushed to the stack. The first value to be popped is then the oldest
/// property.
typedef std::vector<std::pair<ObjectURI, as_value> > SortedPropertyList;
    
/// Enumerate all non-hidden properties to the passed container
//
/// NB: it is likely that this call will change the object, as accessing
/// property values may call getter-setters.
//
/// The enumeration recurses through the prototype chain. This implementation
/// will keep track of visited object to avoid infinite loops in the
/// prototype chain.  NOTE: the Adobe player just chokes in this case.
//
/// Note that the last element of the returned container is the oldest
/// property, so iterate in reverse to mimic AS behaviour.
//
/// @param o        The object whose properties should be enumerated.
/// @return         A list of properties in reverse creation order.
SortedPropertyList enumerateProperties(as_object& o);

/// Get the VM from an as_object.
DSOTEXPORT VM& getVM(const as_object& o);

/// Get the movie_root from an as_object.
DSOTEXPORT movie_root& getRoot(const as_object& o);

/// Get the string_table from an as_object.
DSOTEXPORT string_table& getStringTable(const as_object& o);

/// Get the RunResources from an as_object.
const RunResources& getRunResources(const as_object& o);

/// Get the executing VM version from an as_object.
int getSWFVersion(const as_object& o);

/// Get the Global object from an as_object.
DSOTEXPORT Global_as& getGlobal(const as_object& o);

/// Return whether property matching is caseless
inline bool caseless(const as_object& o) {
    return getSWFVersion(o) < 7;
}

} // namespace gnash

#endif // GNASH_AS_OBJECT_H
