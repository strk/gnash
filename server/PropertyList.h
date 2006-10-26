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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef GNASH_PROPERTYLIST_H
#define GNASH_PROPERTYLIST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_member.h" // for use within map

#include <map> 
#include <string> // for use within map and for StringNoCaseLessThen

// Forward declaration
namespace gnash {
	class as_object;
	class as_environment;
	class as_function;
}

namespace gnash {

/// A case-insensitive string comparator (probably not very performant)
struct StringNoCaseLessThen {
	bool operator() (const std::string& a, const std::string& b) const
	{
		size_t a_len = a.length();
		size_t b_len = b.length();

		size_t cmplen = a_len < b_len ? a_len : b_len;

		for (size_t i=0; i<cmplen; ++i)
		{
			char cha = toupper(a[i]);
			char chb = toupper(b[i]);

			if (cha < chb) return true;
			else if (cha > chb) return false;
			assert(cha==chb);
		}

		// strings are equal for whole lenght of a,
		// a is LessThen b only if 'b' contains more
		// characters then 'a' (if same number of
		// chars 'a' is NOT less then 'b')

		if ( a_len < b_len ) return true;
		return false; // equal or greater

	}
};

/// Set of properties associated to an ActionScript object.
//
class PropertyList
{

private:

	/// TODO: change this to a <boost/ptr_container/ptr_map.hpp>
	///       so we can store as_member by pointer allowing polymorphism
	///	  of it (planning to add a getset_as_member) w/out much
	///	  overhead and with manager ownerhips. See:
	/// http://www.boost.org/libs/ptr_container/doc/ptr_container.html
	///
	typedef std::map<std::string, as_member, StringNoCaseLessThen> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef container::reverse_iterator reverse_iterator;
	typedef container::const_reverse_iterator const_reverse_iterator;

	container _props;

	// this will be used to setup environemnt for
	// getter-setter properties
	//as_object& _owner;

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

	//PropertyList(as_object& owner);

	/// Get the as_value value of a named property
	//
	/// @param key
	///	name of the property. search will be case-insensitive
	///
	/// @param value
	///	a reference to the as_value to which a found property
	///	value will be copied (it will be left untouched if
	///	no property was found)
	///
	/// @return true if the value was successfully retrived, false
	///         otherwise (and value will be untouched)
	///
	bool getValue(const std::string& key, as_value& value) const;

	/// Set the value of a property, creating a new one if unexistent.
	//
	/// @param key
	///	name of the property. search will be case-insensitive
	///
	/// @param value
	///	a const reference to the as_value to use for setting
	///	or creating the property. 
	///
	/// @return true if the value was successfully set, false
	///         otherwise (found a read-only property, most likely).
	///
	bool setValue(const std::string& key, const as_value& value);

	/// \brief
	/// Add a getter/setter property, if not already existing
	/// (or should we allow override ?)
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
	bool addGetterSetter(const std::string& key, as_function& getter,
		as_function& setter) { assert(0); }

	/// Set the flags of a property.
	//
	/// @param key
	///	name of the property. search will be case-insensitive
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
	void clear()
	{
		_props.clear();
	}

	size_t size() const
	{
		return _props.size();
	}

	/// Dump all members (using log_msg)
	void dump() const;
};


} // namespace gnash

#endif // GNASH_PROPERTYLIST_H
