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

#ifndef GNASH_PROPERTYLIST_H
#define GNASH_PROPERTYLIST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <map> 
#include <string> // for use within map 
#include <cassert> // for inlines
#include <cctype> // for toupper

// Forward declaration
namespace gnash {
	class as_object;
	class as_environment;
	class as_function;
	class as_value;
	class Property;
}

namespace gnash {


/// Set of properties associated to an ActionScript object.
//
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
	PropertyList& operator==(const PropertyList&);

	/// Delete all Property objects in the container
	~PropertyList();

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
	/// their value to the given as_environment.
	void enumerateValues(as_environment& env) const;

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
};


} // namespace gnash

#endif // GNASH_PROPERTYLIST_H
