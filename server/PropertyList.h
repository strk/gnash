// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef GNASH_PROPERTYLIST_H
#define GNASH_PROPERTYLIST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Property.h" // for templated functions
#include "as_value.h" // for templated functions

#include <map> 
#include <string> // for use within map 
#include <cassert> // for inlines
#include <cctype> // for toupper
#include <utility> // for std::pair

// Forward declaration
namespace gnash {
	class as_object;
	class as_environment;
	class as_function;
}

namespace gnash {


/// Set of properties associated to an ActionScript object.
//
/// The PropertyList container is the sole owner of the Property
/// elements in it contained and has full responsibility of their
/// construction and destruction.
///
class PropertyList
{

private:

	/// The actual container
	//
	/// We store Property objects by pointer to allow polymorphism
	/// so that we can store either SimpleProperty or GetterSetterProperty
	/// the destructor will take care of deleting all elements.
	///
	/// TODO: change this to a <boost/ptr_container/ptr_map.hpp>
	///       so we can store as_member by pointer allowing polymorphism
	///	  of it (planning to add a getset_as_member) w/out much
	///	  overhead and with manager ownerhips. See:
	/// http://www.boost.org/libs/ptr_container/doc/ptr_container.html
	///
	typedef std::map<std::string, Property*> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef container::reverse_iterator reverse_iterator;
	typedef container::const_reverse_iterator const_reverse_iterator;

	container _props;

	iterator find(const std::string& key) {
		return _props.find(key);
	}
	const_iterator find(const std::string& key) const {
		return _props.find(key);
	}
	iterator end() {
		return _props.end();
	}
	const_iterator end() const {
		return _props.end();
	}
	iterator begin() {
		return _props.begin();
	}
	const_iterator begin() const {
		return _props.begin();
	}

public:

	/// Construct the PropertyList 
	PropertyList();

	/// Copy constructor
	PropertyList(const PropertyList& pl);

	/// Assignment operator
	PropertyList& operator=(const PropertyList&);

	/// Delete all Property objects in the container
	~PropertyList();

	/// Visit the list of properties 
	//
	/// The method will invoke the given visitor method
	/// passing it two arguments: name of the property and
	/// value of it.
	///
	/// @param visitor
	///	The visitor function. Must take a const std::string
	///	reference as first argument and a const as_value reference
	///	as second argument.
	///
	/// @param this_ptr
	///	The object reference used to extract values from properties.
	///
	template <class V>
	void visitValues(V& visitor, as_object& this_ptr) const
	{
		for (const_iterator it = begin(), itEnd = end();
				it != itEnd; ++it)
		{
			const std::string& name = it->first;
			const Property* prop = it->second;
			as_value val = prop->getValue(this_ptr);

			visitor(name, val);
		}
	}

	/// Get the as_value value of a named property
	//
	/// If the named property is a getter/setter one it's getter
	/// will be invoked using this instance's _owner as 'this' pointer.
	///
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @param value
	///	a reference to the as_value to which a found property
	///	value will be copied (it will be left untouched if
	///	no property was found)
	///
	/// @param this_ptr
	/// 	The as_object used to set the 'this' pointer
	/// 	for calling getter/setter function (GetterSetterProperty);
	/// 	it will be unused when getting or setting SimpleProperty
	/// 	properties.
	///	This parameter is non-const as nothing prevents an
	///	eventual "Getter" function from actually modifying it,
	///	so we can't promise constness.
	///	Note that the PropertyList itself might be changed
	///	from this call, accessed trough the 'this' pointer,
	///	so this method too is non-const.
	///
	/// @return true if the value was successfully retrived, false
	///         otherwise (and value will be untouched)
	///
	bool getValue(const std::string& key, as_value& value,
			as_object& this_ptr);

	/// Set the value of a property, creating a new one if unexistent.
	//
	/// If the named property is a getter/setter one it's setter
	/// will be invoked using the given as_object as 'this' pointer.
	/// If the property is not found a SimpleProperty will be created.
	///
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @param value
	///	a const reference to the as_value to use for setting
	///	or creating the property. 
	///
	/// @param this_ptr
	/// 	The as_object used to set the 'this' pointer
	/// 	for calling getter/setter function (GetterSetterProperty);
	/// 	it will be unused when getting or setting SimpleProperty
	/// 	properties.
	///	This parameter is non-const as nothing prevents an
	///	eventual "Setter" function from actually modifying it,
	///	so we can't promise constness.
	///
	/// @return true if the value was successfully set, false
	///         otherwise (found a read-only property, most likely).
	///
	bool setValue(const std::string& key, const as_value& value,
			as_object& this_ptr);

	/// Get a property, if existing
	//
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @return a Property or NULL, if no such property exists
	///	ownership of returned Propery is kept by the PropertyList,
	///	so plase *don't* delete it !
	///
	Property* getProperty(const std::string& key);

	/// Delete a propery, if exising and not protected from deletion.
	//
	///
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @return a pair of boolean values expressing whether the property
	///	was found (first) and whether it was deleted (second).
	///	Of course a pair(false, true) would be invalid (deleted
	///	a non-found property!?). Valid returns are:
	///	- (false, false) : property not found
	///	- (true, false) : property protected from deletion
	///	- (true, true) : property successfully deleted
	///
	std::pair<bool,bool> delProperty(const std::string& key);

	/// \brief
	/// Add a getter/setter property, if not already existing
	/// (or should we allow override ?)
	//
	/// @param key
	///	Name of the property. Search is case-*sensitive*
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
	bool addGetterSetter(const std::string& key, as_function& getter,
		as_function& setter);

	/// Set the flags of a property.
	//
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return true if the value was successfully set, false
	///         otherwise (either not found or protected)
	///
	bool setFlags(const std::string& key, int setTrue, int setFalse);

	/// Set the flags of all properties.
	//
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return a pair containing number of successes 
	///         (first) and number of failures (second).
	///         Failures are due to protected properties,
	///	    on which flags cannot be set.
	///
	std::pair<size_t,size_t> setFlagsAll(int setTrue, int setFalse);

	/// Set the flags of all properties whose name matches
	/// any key in the given PropertyList object
	//
	/// @param props
	///	the properties to use for finding names
	///
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return a pair containing number of successes 
	///         (first) and number of failures (second).
	///         Failures are due to either protected properties
	///	    of keys in the props argument not found in
	///	    this properties set.
	///
	std::pair<size_t,size_t> setFlagsAll(
			const PropertyList& props,
			int setTrue, int setFalse);

	/// \brief
	/// Copy all properties from the given PropertyList
	/// instance.
	//
	/// Unexistent properties are created. Existing properties
	/// are updated with the new value.
	///
	/// @param props
	///	the properties to copy from
	///
	void import(const PropertyList& props);

	/// \brief
	/// Enumerate all non-hidden properties pushing
	/// their keys to the given as_environment.
	void enumerateKeys(as_environment& env) const;

	/// \brief
	/// Enumerate all non-hidden properties inserting
	/// their name/value pair to the given map
	///
	/// @param this_ptr
	/// 	The as_object used to set the 'this' pointer
	/// 	for calling getter/setter function (GetterSetterProperty);
	/// 	it will be unused when getting or setting SimpleProperty
	/// 	properties.
	///	This parameter is non-const as nothing prevents an
	///	eventual "Getter" function from actually modifying it,
	///	so we can't promise constness.
	///	Note that the PropertyList itself might be changed
	///	from this call, accessed trough the 'this' pointer,
	///	so this method too is non-const (this is crazy, should cut it out)
	/// 
	void enumerateKeyValue(as_object& this_ptr, std::map<std::string, std::string>& to);

	/// Remove all entries in the container
	void clear();

	/// Return number of properties in this list
	size_t size() const
	{
		return _props.size();
	}

	/// Dump all members (using log_msg)
	//
	/// @param this_ptr
	/// 	The as_object used to set the 'this' pointer
	/// 	for calling getter/setter function (GetterSetterProperty);
	/// 	it will be unused when getting or setting SimpleProperty
	/// 	properties.
	///	This parameter is non-const as nothing prevents an
	///	eventual "Getter" function from actually modifying it,
	///	so we can't promise constness.
	///	Note that the PropertyList itself might be changed
	///	from this call, accessed trough the 'this' pointer,
	///	so this method too is non-const.
	///
	void dump(as_object& this_ptr);

	/// Mark all simple properties, getters and setters
	/// as being reachable (for the GC)
	void setReachable() const;
};


} // namespace gnash

#endif // GNASH_PROPERTYLIST_H
