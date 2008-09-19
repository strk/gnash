// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "dsodefs.h" // DSOEXPORT

#include "Property.h" // for templated functions
#include "as_prop_flags.h" // for templated functions
#include "as_value.h" // for templated functions
#include "string_table.h"

#include <map> 
#include <string> // for use within map 
#include <cassert> // for inlines
#include <cctype> // for toupper
#include <utility> // for std::pair
#include <set> // for propNameSet
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/key_extractors.hpp>

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
public:
	/// A tag type for multi-index
	struct oType {/**/};

	/// The actual container
	/// index 0 is the fully indexed name/namespace pairs, which are unique
	/// Because of the way searching works, this index can also be used to search
	/// for the names alone (composite keys are sorted lexographically, beginning
	/// with the first element specified)
	///
	/// index 1 is an ordered sequence, and it is used for the AS3 style
	/// enumeration (which requires an order number for each property),
	/// for slot access, and for array access.
	typedef boost::multi_index_container<
		Property,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::composite_key<
					Property,
					boost::multi_index::member<Property,string_table::key,&Property::mName>,
					boost::multi_index::member<Property,string_table::key,&Property::mNamespace>
				>
			>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<PropertyList::oType>,
				boost::multi_index::member<Property,int,&Property::mOrderId>
			>
		>
	> container;

	/// Construct the PropertyList 
	PropertyList()
		:
		_props(),
		mDefaultOrder(0) // correct ?
	{/**/}

	/// Copy constructor
	PropertyList(const PropertyList& pl);

	/// Assignment operator
	PropertyList& operator=(const PropertyList&);

	/// Visit properties 
	//
	/// The method will invoke the given visitor method
	/// passing it two arguments: name of the property and
	/// value of it.
	///
	/// @param visitor
	///	The visitor function. Must take a string_table::key 
	///	reference as first argument and a const as_value reference
	///	as second argument.
	///
	/// @param this_ptr
	///	The object reference used to extract values from properties.
	///
	template <class V>
	void visitValues(V& visitor, as_object& this_ptr) const
	{
		for (container::const_iterator it = _props.begin(),
			itEnd = _props.end(); it != itEnd; ++it)
		{
			as_value val = it->getValue(this_ptr);
			visitor.accept(it->mName, val);
		}
	}

	/// Visit non hidden properties 
	//
	/// The method will invoke the given visitor method
	/// passing it two arguments: name of the property and
	/// value of it.
	///
	/// @param visitor
	///	The visitor function. Must take a string_table::key 
	///	reference as first argument and a const as_value reference
	///	as second argument.
	///
	/// @param this_ptr
	///	The object reference used to extract values from properties.
	///
	template <class V>
	void visitNonHiddenValues(V& visitor, as_object& this_ptr) const
	{
		for (container::const_iterator it = _props.begin(),
			itEnd = _props.end(); it != itEnd; ++it)
		{
	        if (it->getFlags().get_dont_enum()) continue;
			as_value val = it->getValue(this_ptr);
			visitor.accept(it->mName, val);
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
	bool getValue(string_table::key key, as_value& value,
			as_object& this_ptr, string_table::key nsId = 0);

	/// Get the as_value value of an ordered property
	///
	/// getter/setter will be invoked, just as for getValue
	///
	/// @param order
	/// The order number: negative for default values,  non-negative for
	/// properties which were specifically positioned.
	///
	bool getValueByOrder(int order, as_value& val, as_object& this_ptr);
	
	/// Get the order number just after the passed order number.
	///
	/// @param order
	/// 0 is a special value indicating the first order should be returned,
	/// otherwise, this should be the result of a previous call to
	/// getOrderAfter
	///
	/// @return
	/// A value which can be used for ordered access. 
	const Property* getOrderAfter(int order);

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
	/// @param namespaceId
	///	The namespace in which this should be entered. If 0 is given,
	///	this will use the first value found, if it exists.
	///
	/// @param flagsIfMissing
	///	Flags to associate to the property if a new one is created.
	///
	/// @return true if the value was successfully set, false
	///         otherwise (found a read-only property, most likely).
	///
	bool setValue(string_table::key key, as_value value,
			as_object& this_ptr, string_table::key namespaceId = 0,
			const as_prop_flags& flagsIfMissing=0);

	/// Reserves a slot number for a property
	///
	/// @param slotId
	/// The slot id to use. (Note that getOrder() on this property will return
	/// this slot number + 1 if the assignment was successful.)
	///
	/// @param key
	/// Name of the property.
	///
	/// @param nsId
	/// The namespace in which the property should be found.
	///
	/// @return true if the slot did not previously exist.
	bool reserveSlot(unsigned short slotId, string_table::key key,
		string_table::key nsId = 0);

	/// Get a property, if existing
	//
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @param nsId
	/// The id of the namespace to search
	///
	/// @return a Property or NULL, if no such property exists
	///	ownership of returned Propery is kept by the PropertyList,
	///	so plase *don't* delete it !
	///
	Property* getProperty(string_table::key key, string_table::key nsId = 0);

	/// Get a property, if existing, by order
	///
	/// @param order
	/// The ordering id
	///
	const Property* getPropertyByOrder(int order);
	
	/// Delete a propery, if exising and not protected from deletion.
	//
	///
	/// @param key
	///	Name of the property. Search is case-*sensitive*
	///
	/// @param nsId
	/// Name of the namespace
	///
	/// @return a pair of boolean values expressing whether the property
	///	was found (first) and whether it was deleted (second).
	///	Of course a pair(false, true) would be invalid (deleted
	///	a non-found property!?). Valid returns are:
	///	- (false, false) : property not found
	///	- (true, false) : property protected from deletion
	///	- (true, true) : property successfully deleted
	///
	std::pair<bool,bool> delProperty(string_table::key key,
		string_table::key nsId = 0);

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
	/// @param cacheVal
	///	The value to use as a cache. If null uses any cache
	///	from pre-existing property with same name.
	///
	/// @param flagsIfMissing
	///	Flags to associate to the property if a new one is created.
	///
	/// @return true if the property was successfully added, false
	///         otherwise (property already existent?)
	///
	bool addGetterSetter(string_table::key key, as_function& getter,
		as_function* setter, const as_value& cacheVal,
		const as_prop_flags& flagsIfMissing=0, string_table::key ns=0);

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
	bool addGetterSetter(string_table::key key, as_c_function_ptr getter,
		as_c_function_ptr setter, const as_prop_flags& flagsIfMissing,
		string_table::key ns = 0);

	/// \brief
	/// Add a destructive getter property, if not already existant.
	///
	/// @param key
	/// Name of the property. Case-sensitive search.
	///
	/// @param getter
	/// A function to invoke when this property value is requested.
	///
	/// @param flagsIfMissing
	///	Flags to associate to the property if a new one is created.
	///
	/// @return true if the property was successfully added, false
	/// otherwise.
	///
	bool addDestructiveGetter(string_table::key key,
		as_function& getter, string_table::key ns = 0,
		const as_prop_flags& flagsIfMissing=0);

	/// \brief
	/// Add a destructive getter property, if not already existant.
	///
	/// @param key
	/// Name of the property. Case-sensitive search.
	///
	/// @param getter
	/// A function to invoke when this property value is requested.
	///
	/// @param flagsIfMissing
	///	Flags to associate to the property if a new one is created.
	///
	/// @return true if the property was successfully added, false
	/// otherwise.
	///
	bool addDestructiveGetter(string_table::key key,
		as_c_function_ptr getter, string_table::key ns = 0,
		const as_prop_flags& flagsIfMissing=0);

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
	bool setFlags(string_table::key key, int setTrue, int setFalse,
		string_table::key ns = 0);

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

	// Used to keep track of which properties have been enumerated.
	typedef std::set<std::pair<string_table::key, string_table::key> > propNameSet;

	/// \brief
	/// Enumerate all non-hidden properties pushing
	/// their keys to the given as_environment.
	///
	/// @param donelist
	/// Don't enumerate those in donelist. Add those done to donelist.
	///
	void enumerateKeys(as_environment& env, propNameSet& donelist) const;

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
	void enumerateKeyValue(const as_object& this_ptr, std::map<std::string, std::string>& to) const;

	/// Remove all entries in the container
	void clear();

	/// Return number of properties in this list
	size_t size() const
	{
		return _props.size();
	}

	/// Dump all members (using log_debug)
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

	/// Dump all members into the given map
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
	void dump(as_object& this_ptr, std::map<std::string, as_value>& to);

	/// Mark all simple properties, getters and setters
	/// as being reachable (for the GC)
	void DSOEXPORT setReachable() const;

private:
	container _props;
	unsigned short mDefaultOrder;
};


} // namespace gnash

#endif // GNASH_PROPERTYLIST_H
