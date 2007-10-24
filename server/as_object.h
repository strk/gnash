// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "tu_config.h"

#include "string_table.h"
#include "container.h"
#include "ref_counted.h" // for inheritance  (to drop)
#include "GC.h" // for inheritance from GcResource (to complete)
#include "PropertyList.h"
#include "as_value.h" // for return of get_primitive_value
#include "smart_ptr.h"
#include "as_prop_flags.h" // for enum
#include "GnashException.h"
#define NEW_KEY_LISTENER_LIST_DESIGN
#ifdef NEW_KEY_LISTENER_LIST_DESIGN
  #include "event_id.h" // for event_id
#endif
#include <sstream>

#if defined(__GNUC__) && __GNUC__ > 2
#  include <cxxabi.h>
#endif

#include <cmath>
#include <utility> // for std::pair
#include <set>

// Forward declarations
namespace gnash {
	class as_function;
	class sprite_instance;
	class as_environment;
	class VM;
	class Machine;
}

namespace gnash {

class asClass;
class asName;

/// \brief
/// A generic bag of attributes. Base class for all ActionScript-able objects.
//
/// Base-class for ActionScript script-defined objects.
/// This would likely be ActionScript's 'Object' class.
///
class DSOEXPORT as_object
	:
#ifdef GNASH_USE_GC
	public GcResource
#else
	public ref_counted
#endif
{
	friend class asClass;
	friend class Machine;

	typedef std::set<std::pair<string_table::key, string_table::key> > propNameSet;
private:
	/// Properties of this objects 
	PropertyList _members;

	/// Don't allow implicit copy, must think about behaviour
	as_object& operator=(const as_object&)
	{
		assert(0);
		return *this;
	}

#if 0
	/// Look for a Getter/setter property scanning the inheritance chain
	//
	/// @returns a Getter/Setter propery if found, NULL if not found
	///
	/// TODO: drop this function, is unused
	Property* findGetterSetter(string_table::key name, 
		string_table::key nsname = 0);
#endif

	/// Find an existing property for update, only scaning the inheritance chain for getter/setters
	/// or statics.
	//
	/// NOTE: updatable here doesn't mean the property isn't protected from update
	///       but only that a set_member will NOT create a new property (either
	///	  completely new or as an override).
	///
	/// @returns a propery if found, NULL if not found
	///
	Property* findUpdatableProperty(string_table::key name, 
		string_table::key nsname = 0);

	/// Find a property scanning the inheritance chain
	///
	/// @param name
	/// The string id to look for
	///
	/// @param owner
	/// If not null, this is set to the object which contained the property.
	///
	/// @returns a Propery if found, NULL if not found
	///
	Property* findProperty(string_table::key name, string_table::key nsname,
		as_object **owner = NULL);

public:

	/// \brief
	/// Return a reference to the Virtual Machine that created
	/// this object. 
	VM& getVM() const {
		return _vm;
	}


	/// Dump all properties using log_msg
	//
	/// Note that this method is non-const
	/// as some properties might be getter/setter
	/// ones, thus simple read of them might execute
	/// user code actually changing the object itsef.
	///
	void dump_members();

	/// Dump all properties into the given map
	//
	/// Note that this method is non-const
	/// as some properties might be getter/setter
	/// ones, thus simple read of them might execute
	/// user code actually changing the object itsef.
	///
	void dump_members(std::map<std::string, as_value>& to);

	/// Construct an ActionScript object with no prototype associated.
	as_object();

	/// \brief
	/// Construct an ActionScript object based on the given prototype.
	/// Adds a reference to the prototype, if any.
	as_object(as_object* proto);

	/// Construct an ActionScript object based on the given prototype.
	as_object(boost::intrusive_ptr<as_object> proto);

	/// Copy an as_object
	//
	/// TODO: write more about this, is it allowed ? is it safe ?
	///
	as_object(const as_object& other);
	
	/// Return a text representation for this object
	virtual std::string get_text_value() const { return "[object Object]"; }

	/// Return true if instances of this ActionScript class should use 
	/// a custom toString method, when available, for converting the object
	/// to a string.
	virtual bool useCustomToString() const { return true; }

	/// Return the numeric value of this object
	//
	/// The default implementation converts the text value
	/// to a number, override for a more performant implementation
	///
	virtual double get_numeric_value() const {
		std::string txt = get_text_value();
		if ( ! txt.empty() ) return atof(txt.c_str());
		else return 0; 
	}

	/// Return the "primitive" value of this object
	//
	/// The default implementation returns an Object value,
	/// other objects can override this function
	/// to provide another "preferred" primitive. Primitive
	/// values are: undefined, null, boolean, string, number, object.
	///
	/// See ECMA-2.6.2 (section 4.3.2).
	///
	virtual as_value get_primitive_value() const
	{
		// Since as_value(const as_object*) doesn't exist
		// we have to cast the value to non-const, or
		// the as_value will result as beeing a BOOLEAN ! (?)
		return as_value(const_cast<as_object*>(this)); 
	}

	/// Set a member value
	//
	/// The default behaviour is to call set_member_default,
	/// but this function is kept virtual to allow special
	/// handling of property assignment in derivate class.
	/// NOTE: This might change in the near future trough use of
	///       getter/setter properties instead..
	///
	/// @param key
	///	Id of the property. 
	///
	/// @param val
	///	Value to assign to the named property.
	///
	/// @param nsname
	///	Id of the namespace.
	///
	virtual void set_member(string_table::key key, const as_value& val,
		string_table::key nsname = 0)
	{
		return set_member_default(key, val, nsname);
	}

	/// Update an existing member value
	//
	/// NOTE that getter-setter in the inheritance chaing are
	/// considered as existing members. See with.as and Object.as
	/// testcases under actionscript.all.
	/// Also be aware that 'special' (non-proper) properties
	/// are considered non-existing, this is surely true for
	/// childs of MovieClips, also tested in with.as.
	///
	/// @param key
	///	Id of the property. 
	///
	/// @param val
	///	Value to assign to the named property.
	///
	/// @param nsname
	///	Id of the namespace.
	///
	/// @return a pair in which first element express wheter the property01apl0mb
	///         was found and the second wheter it was set (won't set if read-only).
	///
	std::pair<bool,bool> update_member(string_table::key key, const as_value& val,
		string_table::key nsname = 0);

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	virtual bool on_event(const event_id& id );
#endif

	/// Reserve a slot
	///
	/// Reserves a slot for a property to follow.
	void reserveSlot(string_table::key name, string_table::key nsId,
		unsigned short slotId);

	/// Initialize a member value by string
	//
	/// This is just a wrapper around the other init_member method
	/// used as a trampoline to avoid changing all classes to 
	/// use string_table::key directly.
	///
	/// @param name
	///     Name of the member.
	///	Will be converted to lowercase if VM is initialized for SWF6 or lower.
	///
	/// @param val
	///     Value to assign to the member.
	///
	/// @param flags
	///     Flags for the new member. By default dontDelete and dontEnum.
	///	See as_prop_flags::Flags.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	void init_member(const std::string& name, const as_value& val, 
		int flags=as_prop_flags::dontDelete|as_prop_flags::dontEnum, 
		string_table::key nsname = 0);

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
	///	See as_prop_flags::Flags.
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
	void init_member(string_table::key key, const as_value& val, 
		int flags=as_prop_flags::dontDelete|as_prop_flags::dontEnum,
		string_table::key nsname = 0, int slotId = -1);

	/// \brief
	/// Initialize a getter/setter property by name
	//
	/// This is just a wrapper around the other init_property method
	/// used as a trampoline to avoid changing all classes to 
	/// use string_table::key directly.
	///
	/// @param key
	///     Name of the property.
	///	Will be converted to lowercase if VM is initialized for SWF6 or lower.
	///
	/// @param getter
	///	A function to invoke when this property value is requested.
	///	add_ref will be called on the function.
	///
	/// @param setter
	///	A function to invoke when setting this property's value.
	///	add_ref will be called on the function.
	///
	/// @param flags
	///     Flags for the new member. By default dontDelete and dontEnum.
	///	See as_prop_flags::Flags.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	void init_property(const std::string& key, as_function& getter,
		as_function& setter, int flags=as_prop_flags::dontDelete|as_prop_flags::dontEnum,
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
	///	A function to invoke when this property value is requested.
	///	add_ref will be called on the function.
	///
	/// @param setter
	///	A function to invoke when setting this property's value.
	///	add_ref will be called on the function.
	///
	/// @param flags
	///     Flags for the new member. By default dontDelete and dontEnum.
	///	See as_prop_flags::Flags.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	void init_property(string_table::key key, as_function& getter,
		as_function& setter, int flags=as_prop_flags::dontDelete|as_prop_flags::dontEnum,
		string_table::key nsname = 0);


	/// \brief
	/// Initialize a destructive getter/setter property
	///
	/// A destructive getter/setter can be used as a place holder for the real
	/// value of a property.  As soon as getValue is invoked on the getter,
	/// it destroys itself after setting its property to the return value of
	/// getValue.
	///
	/// @param key
	///     Name of the property.
	///	Will be converted to lowercase if VM is initialized for SWF6 or lower.
	///
	/// @param getter
	///	A function to invoke when this property value is requested.
	///	add_ref will be called on the function.
	///
	/// @param setter
	///	A function to invoke when setting this property's value.
	///	add_ref will be called on the function.
	///
	/// @param flags
	///     Flags for the new member. By default dontDelete and dontEnum.
	///	See as_prop_flags::Flags.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	///
	bool init_destructive_property(string_table::key key, as_function& getter,
		as_function& setter, int flags=as_prop_flags::dontEnum,
		string_table::key nsname = 0);

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
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	void init_readonly_property(const std::string& key, as_function& getter,
			int flags=as_prop_flags::dontDelete|as_prop_flags::dontEnum,
			string_table::key nsname = 0);

	/// Get a member as_value by name
	//
	/// The default behaviour is to call set_member_default,
	/// but this function is kept virtual to allow special
	/// handling of property fetching in derivate class.
	///
	/// NOTE that this method is non-const becase a property
	///      could also be a getter/setter and we can't promise
	///      that the 'getter' won't change this object trough
	///	 use of the 'this' reference. 
	///
	/// @param name
	///	Name of the property. Must be all lowercase
	///	if the current VM is initialized for a  target
	///	up to SWF6.
	///
	/// @param val
	///	Variable to assign an existing value to.
	///	Will be untouched if no property with the given name
	///	was found.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	///
	/// @return true of the named property was found, false otherwise.
	///
	virtual bool get_member(string_table::key name, as_value* val,
		string_table::key nsname = 0)
	{
		return get_member_default(name, val, nsname);
	}

	/// Chad: Document
	bool isQName() const { return false; /* TODO: Implement */ }
	bool isXML() const { return false; /* TODO */ }
	bool isDictionary() const { return false; /* TODO */ }

	/// Get a member as_value by name in an AS-compatible way
	//
	/// NOTE that this method is non-const becase a property
	///      could also be a getter/setter and we can't promise
	///      that the 'getter' won't change this object trough
	///	 use of the 'this' reference. 
	///
	/// @param name
	///	Name of the property. Will be converted to lowercase
	///	if the current VM is initialized for a  target
	///	up to SWF6.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	///
	/// @return value of the member (possibly undefined),
	///	or undefined if not found. Use get_member if you
	///	need to know wheter it was found or not.
	///
	as_value getMember(string_table::key name, string_table::key nsname = 0);

	/// Call a method of this object in an AS-compatible way
	//
	/// @param name
	///	Name of the method. Will be converted to lowercase
	///	if the current VM is initialized for a  target
	///	up to SWF6.
	///
	/// @param env
	///	The environment to use for setting up call frame stack
	///
	/// @param nargs
	///	Number of arguments
	///
	/// @param ...
	///	nargs as_value references
	///
	/// @return value of the member (possibly undefined),
	///	or undefined if not found. Use get_member if you
	///	need to know wheter it was found or not.
	///
	as_value callMethod(string_table::key name, as_environment& env);
	as_value callMethod(string_table::key name, as_environment& env, const as_value& arg0);
	as_value callMethod(string_table::key name, as_environment& env, const as_value& arg0, const as_value& arg1);
	as_value callMethod(string_table::key name, as_environment& env, const as_value& arg0, const as_value& arg1, const as_value& arg2);

	/// Delete a property of this object, unless protected from deletion.
	//
	/// This function does *not* recurse in this object's prototype.
	///
	/// @parame name
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	///
	/// @return a pair of boolean values expressing whether the property
	///	was found (first) and whether it was deleted (second).
	///	Of course a pair(false, true) would be invalid (deleted
	///	a non-found property!?). Valid returns are:
	///	- (false, false) : property not found
	///	- (true, false) : property protected from deletion
	///	- (true, true) : property successfully deleted
	///
	std::pair<bool,bool> delProperty(string_table::key name, string_table::key nsname = 0);

	/// Get this object's own named property, if existing.
	//
	/// This function does *not* recurse in this object's prototype.
	///
	/// @parame name
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	///
	/// @return
	///	a Property pointer, or NULL if this object doesn't
	///	contain the named property.
	///
	Property* getOwnProperty(string_table::key name, string_table::key nsname = 0);

	/// Get a property from this object (or a prototype) by ordering index.
	///
	/// @param index
	/// An index returned by nextIndex
	///
	/// @return
	/// The property associated with the order index.
	Property *getByIndex(int index);

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
	///	Name of the property. Must be all lowercase
	///	if the current VM is initialized for a  target
	///	up to SWF6.
	///
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @param nsname
	/// The id of the namespace to which this member belongs. 0 is a wildcard
	/// and will be matched by anything not asking for a specific namespace.
	///
	/// @return true on success, false on failure
	///	(non-existent or protected member)
	///
	bool set_member_flags(string_table::key name,
			int setTrue, int setFalse=0, string_table::key nsname = 0);

	/// Cast to a sprite, or return NULL
	virtual sprite_instance* to_movie() { return NULL; }

	/// Cast to a as_function, or return NULL
	virtual as_function* to_function() { return NULL; }

	/// Add an interface to the list of interfaces.
	/// Used by instanceOf
	void add_interface(as_object* ctor);

	/// \brief
	/// Check whether this object is an instance of the given
	/// as_function constructor
	//
	/// NOTE: built-in classes should NOT be C_FUNCTIONS for this to
	/// work
	///
	bool instanceOf(as_function* ctor);

	/// \brief
	/// Check whether this object is a 'prototype' in the given
	/// object's inheritance chain.
	//
	bool prototypeOf(as_object& instance);

	/// Set property flags
	//
	/// @param props
	///	A comma-delimited list of property names as a string,
	///	a NULL value, or an array? (need to check/test, probably
	///     somehting is broken).
	///	Property strings are case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///	
	///
	/// @param set_false
	/// @param set_true
	///
	void setPropFlags(as_value& props, int set_false, int set_true);

#ifdef USE_DEBUGGER
	/// Get the properties of this objects 
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

	/// \brief
	/// Enumerate all non-hidden properties pushing
	/// their value to the given as_environment.
	//
	/// The enumeration recurse in prototype.
	/// This implementation will keep track of visited object
	/// to avoid loops in prototype chain. 
	/// NOTE: the MM player just chokes in this case (loop)
	///
	void enumerateProperties(as_environment& env) const;

	/// Enumerate any non-proper properties
	//
	/// This function is called by enumerateProperties(as_environment&) 
	/// to allow for enumeration of properties that are not "proper"
	/// (not contained in the as_object PropertyList).
	///
	/// The default implementation adds nothing
	///
	virtual void enumerateNonProperties(as_environment&) const
	{
	}

	/// \brief
	/// Enumerate all non-hidden properties inserting
	/// their name/value pair to the given map.
	//
	/// The enumeration recurse in prototype.
	/// This implementation will keep track of visited object
	/// to avoid loops in prototype chain. 
	/// NOTE: the MM player just chokes in this case (loop)
	///
	void enumerateProperties(std::map<std::string, std::string>& to);

	/// \brief
	/// Add a getter/setter property, if no member already has
	/// that name (or should we allow override ? TODO: check this)
	//
	/// @param key
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///
	/// @param getter
	///	A function to invoke when this property value is requested.
	///	add_ref will be called on the function.
	///
	/// @param setter
	///	A function to invoke when setting this property's value.
	///	add_ref will be called on the function.
	///
	/// @return true if the property was successfully added, false
	///         otherwise (property already existent?)
	///
	bool add_property(const std::string& key, as_function& getter,
		as_function& setter);

	/// Return this object '__proto__' member.
	//
	/// The __proto__ member is the exported interface ('prototype')
	/// of the class this object is an instance of.
	///
	/// NOTE: can return NULL (and it is expected to do for Object.prototype)
	///
	boost::intrusive_ptr<as_object> get_prototype();

	const boost::intrusive_ptr<as_object> get_prototype() const {
		// cast away constness
		return const_cast<as_object*>(this)->get_prototype();
	}

	/// Set this object's '__proto__' member
	//
	/// There is no point to make this function
	/// protected or private, as a call to the
	/// public: set_member("__proto__", <anyting>)
	/// will do just the same
	///
	void set_prototype(boost::intrusive_ptr<as_object> proto, int flags=as_prop_flags::dontDelete|as_prop_flags::dontEnum);

	std::string asPropName(string_table::key name);
	
	/// @{ Common ActionScript methods for characters
	/// TODO: make protected

	static as_value tostring_method(const fn_call& fn);

	static as_value valueof_method(const fn_call& fn);

	/// @} Common ActionScript getter-setters for characters
	
protected:

	/// Get a property value by name
	//
	/// This is the default implementation, taking care of
	/// the inheritance chain and getter/setter functions.
	///
        /// The derived class should not override this method,
        /// but instead implement its own gettersetter properties.
	///
	/// NOTE that this method is non-const becase a property
	///      could also be a getter/setter and we can't promise
	///      that the 'getter' won't change this object trough
	///	 use of the 'this' reference.
	///
	/// @param name
	///	Name of the property. Must be all lowercase
	///	if the current VM is initialized for a  target
	///	up to SWF6.
	///
	/// @param val
	///     The as_value to store a found variable's value in.
	///
	bool get_member_default(string_table::key name, as_value* val, 
		string_table::key nsname);

	/// Set a member value
	//
	/// This is the default implementation, taking care of
	/// the inheritance chain and getter/setter functions.
	///
        /// The derived class should not override this method,
        /// but instead implement its own gettersetter properties.
	///
	/// @param name
	///	Name of the property. Must be all lowercase
	///	if the current VM is initialized for a  target
	///	up to SWF6.
	///
	/// @param val
	///	Value to assign to the named property.
	///
	void set_member_default(string_table::key name, const as_value& val, 
		string_table::key nsname);

#ifdef GNASH_USE_GC
	/// Mark all reachable resources, override from GcResource.
	//
	/// The default implementation marks all properties
	/// as being reachable, calling markAsObjectReachable().
	///
	/// If a derived class provides access to more GC-managed
	/// resources, it should override this method and call 
	/// markAsObjectReachable() as the last step.
	///
	virtual void markReachableResources() const
	{
		markAsObjectReachable();
	}

	/// Mark properties and __proto__ as reachable (for the GC)
	void markAsObjectReachable() const
	{
		_members.setReachable();
		//if ( m_prototype.get() ) m_prototype->setReachable();
	}
#endif // GNASH_USE_GC

	/// The Virtual Machine used to create this object
	VM& _vm;

private:

	/// The constructors of the objects which are the interfaces
	/// implemented by this one.
	std::list<as_object*> mInterfaces;

	/// Reference to this object's '__proto__'
	//boost::intrusive_ptr<as_object> m_prototype;

};

/// Template which does a dynamic cast for as_object pointers. It throws an
/// exception if the dynamic cast fails.
/// @param T the class to which the obj pointer should be cast.
/// @param obj the pointer to be cast.
/// @return If the cast succeeds, the pointer cast to the requested type.
///         Otherwise, NULL.
template <typename T>
boost::intrusive_ptr<T>
ensureType (boost::intrusive_ptr<as_object> obj)
{
	boost::intrusive_ptr<T> ret = boost::dynamic_pointer_cast<T>(obj);

	if (!ret) {
		std::string     target = typeid(T).name(),
				source = typeid(*obj.get()).name();
#if defined(__GNUC__) && __GNUC__ > 2
		int status;
		char* target_unmangled = 
			abi::__cxa_demangle (target.c_str(), NULL, NULL,
					     &status);
		if (status == 0) {
			target = target_unmangled;
			free(target_unmangled);
		}

		char* source_unmangled =
			abi::__cxa_demangle (source.c_str(), NULL, NULL,
					     &status);

		if (status == 0) {
			source = source_unmangled;
			free(source_unmangled);
		}
#endif // __GNUC__ > 2

		std::string msg = "builtin method or gettersetter for " +
			target + " called from " + source + " instance.";

		throw ActionException(msg);
	}
	return ret;
}



} // namespace gnash

#endif // GNASH_AS_OBJECT_H
