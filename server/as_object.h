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

#include <cmath>
#include "container.h"
#include "ref_counted.h" // for inheritance 
#include "PropertyList.h"

namespace gnash {

// Forward declarations
class as_function;
class movie;
class as_value;
class as_environment;

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

	/// Dump all properties using log_msg
	//
	/// Note that this method is non-const
	/// as some properties might be getter/setter
	/// ones, thus simple read of them might execute
	/// user code actually changing the object itsef.
	///
	void dump_members();

	/// Reference to this object's '__proto__'
	// TODO: make private (or protected)
	as_object*	m_prototype;

	/// Construct an ActionScript object with no prototype associated.
	as_object()
		:
		_members(),
		m_prototype(NULL)
	{
	}

	/// \brief
	/// Construct an ActionScript object based on the given prototype.
	/// Adds a reference to the prototype, if any.
	as_object(as_object* proto)
		:
		_members(),
		m_prototype(proto)
	{
		if (m_prototype) m_prototype->add_ref();
	}

	as_object(const as_object& other)
		:
		ref_counted(),
		_members(other._members),
		m_prototype(other.m_prototype)
	{
		if (m_prototype) m_prototype->add_ref();
	}

	/// \brief
	/// Default destructor for ActionScript objects.
	/// Drops reference on prototype member, if any.
	virtual ~as_object()
	{
		if (m_prototype) m_prototype->drop_ref();
	}
	
	/// Return a text representation for this object
	virtual const char* get_text_value() const { return NULL; }

	/// Return the numeric value of this object
	//
	/// The default implementation converts the text value
	/// to a number, override for a more performant implementation
	///
	virtual double get_numeric_value() const {
		return atof(get_text_value());
	}

	/// Set a member value
	//
	/// The default behaviour is to call set_member_default,
	/// but this function is kept virtual to allow special
	/// handling of property assignment in derivate class.
	/// NOTE: This might change in the near future trough use of
	///       getter/setter properties instead..
	///
	/// TODO: take a std::string rather then a tu_stringi
	///
	virtual void set_member(const tu_stringi& name,
			const as_value& val );

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
	/// TODO: take a std::string rather then a tu_stringi
	///
	virtual bool get_member(const tu_stringi& name, as_value* val);
	

	/// Set member flags (probably used by ASSetPropFlags)
	//
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return true on success, false on failure
	///	(non-existent or protected member)
	///
	bool set_member_flags(const tu_stringi& name,
			int setTrue, int setFalse=0);

	/// This object is not a movie; no conversion.
	virtual movie*	to_movie() { return NULL; }

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
	///	name of the property. search will be case-insensitive
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
		as_function& setter)
	{
		return _members.addGetterSetter(key, getter, setter);
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
	/// TODO: take a std::string rather then a tu_stringi
	///
	bool get_member_default(const tu_stringi& name, as_value* val);

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
	void set_member_default(const tu_stringi& name, const as_value& val);

private:

	/// Set this object's '__proto__' member
	void set_prototype(as_object* proto);

};

} // namespace gnash

#endif // GNASH_AS_OBJECT_H
