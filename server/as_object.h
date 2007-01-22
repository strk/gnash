// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//

#ifndef GNASH_AS_OBJECT_H
#define GNASH_AS_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "container.h"
#include "ref_counted.h" // for inheritance 
#include "PropertyList.h"
#include "as_value.h" // for return of get_primitive_value
#include "smart_ptr.h"

#include <cmath>
#include <utility> // for std::pair

// Forward declarations
namespace gnash {
	class as_function;
	class sprite_instance;
	class as_environment;
	class VM;
}

namespace gnash {

/// \brief
/// A generic bag of attributes. Base class for all ActionScript-able objects.
//
/// Base-class for ActionScript script-defined objects.
/// This would likely be ActionScript's 'Object' class.
///
//class as_object : public resource
class DSOEXPORT as_object : public ref_counted
{
	/// Properties of this objects 
	PropertyList _members;

	/// Don't allow implicit copy, must think about behaviour
	as_object& operator==(const as_object&)
	{
		assert(0);
		return *this;
	}

	/// Look for a Getter/setter property scanning the inheritance chain
	//
	/// @returns a Getter/Setter propery if found, NULL if not found
	///
	Property* findGetterSetter(const std::string& name);

	/// Find a property scanning the inheritance chain
	//
	/// @returns a Propery if found, NULL if not found
	///
	Property* findProperty(const std::string& name);

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

	/// Construct an ActionScript object with no prototype associated.
	as_object();

	/// \brief
	/// Construct an ActionScript object based on the given prototype.
	/// Adds a reference to the prototype, if any.
	as_object(as_object* proto);

	/// Copy an as_object
	//
	/// TODO: write more about this, is it allowed ? is it safe ?
	///
	as_object(const as_object& other);

	/// Default destructor for ActionScript objects.
	//
	/// Drops reference on prototype member, if any.
	///
	virtual ~as_object() {}
	
	/// Return a text representation for this object
	virtual const char* get_text_value() const { return NULL; }

	/// Return the numeric value of this object
	//
	/// The default implementation converts the text value
	/// to a number, override for a more performant implementation
	///
	virtual double get_numeric_value() const {
		const char* txt = get_text_value();
		if ( txt ) return atof(get_text_value());
		else return 0; 
	}

	/// Return the "primitive" value of this object
	//
	/// The default implementation returns the numeric value
	/// of this object, other objects can override this function
	/// to provide another "preferred" primitive. Primitive
	/// values are: undefined, null, boolean, string, number.
	///
	/// See ECMA-2.6.2 (section 4.3.2).
	///
	virtual as_value get_primitive_value() const {
		return as_value(get_numeric_value());
	}

	/// Set a member value
	//
	/// The default behaviour is to call set_member_default,
	/// but this function is kept virtual to allow special
	/// handling of property assignment in derivate class.
	/// NOTE: This might change in the near future trough use of
	///       getter/setter properties instead..
	///
	virtual void set_member(const std::string& name, const as_value& val)
	{
		return set_member_default(name, val);
	}

	/// Initialize a member value
	//
	/// This method has to be used by built-in classes initialization
	/// (VM initialization in general) as will avoid to scan the
	/// inheritance chain and perform lowercase conversion when
	/// VM version is initialized at versions < 7.
	/// Also, members initialized by calling this function will
	/// be protected from deletion and not shown in enumeration
	/// TODO: provide an additional argument to prevent 'protection' ?
	///
	/// @param name
	///     Name of the member.
	///	Will be converted to lowercase if VM is initialized for SWF6 or lower.
	///
	/// @param val
	///     Value to assign to the member.
	///
	void init_member(const std::string& name, const as_value& val);

	/// \brief
	/// Initialize a getter/setter property
	//
	/// This method has to be used by built-in classes initialization
	/// (VM initialization in general) as will avoid to scan the
	/// inheritance chain and perform lowercase conversion when
	/// VM version is initialized at versions < 7.
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
	void init_property(const std::string& key, as_function& getter,
		as_function& setter);

	/// Get a member as_value by name
	//
	/// The default behaviour is to call set_member_default,
	/// but this function is kept virtual to allow special
	/// handling of property fetching in derivate class.
	/// NOTE: This might change in the near future trough use of
	///       getter/setter properties instead..
	///
	/// NOTE that this method is non-const becase a property
	///      could also be a getter/setter and we can't promise
	///      that the 'getter' won't change this object trough
	///	 use of the 'this' reference.
	///
	virtual bool get_member(const std::string& name, as_value* val)
	{
		return get_member_default(name, val);
	}

	/// Delete a property of this object, unless protected from deletion.
	//
	/// This function does *not* recurse in this object's prototype.
	///
	/// @parame name
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///
	/// @return a pair of boolean values expressing whether the property
	///	was found (first) and whether it was deleted (second).
	///	Of course a pair(false, true) would be invalid (deleted
	///	a non-found property!?). Valid returns are:
	///	- (false, false) : property not found
	///	- (true, false) : property protected from deletion
	///	- (true, true) : property successfully deleted
	///
	std::pair<bool,bool> delProperty(const std::string& name);

	/// Set member flags (probably used by ASSetPropFlags)
	//
	/// @parame name
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return true on success, false on failure
	///	(non-existent or protected member)
	///
	bool set_member_flags(const std::string& name,
			int setTrue, int setFalse=0);

	/// Cast to a sprite, or return NULL
	virtual sprite_instance* to_movie() { return NULL; }

	void	clear();

	/// Check whether this object is an instance of the given
	/// as_function constructor
	//
	/// NOTE: built-in classes should NOT be C_FUNCTIONS for this to
	/// work
	///
	bool instanceOf(as_function* ctor);

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

	/// Copy properties from the given object
	void copyProperties(const as_object& o);

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
	as_object* get_prototype() {
		return m_prototype.get();
	}

	const as_object* get_prototype() const {
		return m_prototype.get();
	}

protected:

	/// Get a property value by name
	//
	/// This is the default implementation, taking care of
	/// the inheritance chain and getter/setter functions.
	///
	/// If a derivate class needs special handling of get member
	/// request it should override the get_member() method instead.
	/// NOTE: This might change in the near future trough use of
	///       getter/setter properties instead..
	///
	/// NOTE that this method is non-const becase a property
	///      could also be a getter/setter and we can't promise
	///      that the 'getter' won't change this object trough
	///	 use of the 'this' reference.
	///
	/// @param name
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///	TODO: be *always* case-sensitive, and delegate
	///	      convertion to lowercase to callers
	///
	/// @param val
	///     The as_value to store a found variable's value in.
	///
	bool get_member_default(const std::string& name, as_value* val);

	/// Set a member value
	//
	/// This is the default implementation, taking care of
	/// the inheritance chain and getter/setter functions.
	///
	/// If a derivate class needs special handling of get member
	/// request it should override the get_member() method instead.
	/// NOTE: This might change in the near future trough use of
	///       getter/setter properties instead..
	///
	/// TODO: take a std::string rather then a tu_stringi
	///
	/// @parame name
	///     Name of the property.
	///	Case insensitive up to SWF6,
	///	case *sensitive* from SWF7 up.
	///
	void set_member_default(const std::string& name, const as_value& val);

	/// Set this object's '__proto__' member
	//
	/// This is protected to allow character instances to set a prototype,
	/// since character instances are NOT direct subclasses of as_object
	/// ( as_object -> character -> specific_character ).
	///
	void set_prototype(as_object* proto);

	/// The Virtual Machine used to create this object
	VM& _vm;

private:

	/// Reference to this object's '__proto__'
	boost::intrusive_ptr<as_object> m_prototype;

};

} // namespace gnash

#endif // GNASH_AS_OBJECT_H
