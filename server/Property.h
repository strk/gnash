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

// 
//

#ifndef GNASH_PROPERTY_H
#define GNASH_PROPERTY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_prop_flags.h"
#include "as_value.h"
#include "GetterSetter.h" // for GetterSetterProperty

namespace gnash {


/// An abstract property
//
/// This is intended for use only by PropertyList class
///
class Property
{
	/// Properties flags
	as_prop_flags _flags;

public:
	/// Default constructor
	Property()
	{
	}

	Property(const Property& p)
		:
		_flags(p._flags)
	{
	}

	/// Constructor taking initial flags
	Property(const as_prop_flags& flags)
		:
		_flags(flags)
	{
	}

	/// \brief
	/// Virtual destructor, to make sure the appropriate
	/// destructor is called for derivated classes
	//
	/// We've nothing to do here, as our only member is
	/// the as_prop_flags which should take care of it's
	/// destruction.
	///
	virtual ~Property() {}

	/// accessor to the properties flags
	const as_prop_flags& getFlags() const { return _flags; }
	as_prop_flags& getFlags() { return _flags; }

	/// Get value of this property
	//
	/// @param this_ptr
	/// 	The as_object used to set the 'this' pointer.
	/// 	for calling getter function (GetterSetterProperty);
	/// 	it will be unused when getting or setting SimpleProperty
	/// 	properties.
	///	This parameter is non-const as nothing prevents an
	///	eventual "Setter" function from actually modifying it,
	///	so we can't promise constness.
	///
	/// @return the value of this property
	///
	virtual as_value getValue(as_object& this_ptr) const=0;

	/// Set value of this property
	//
	/// @param this_ptr
	/// 	The as_object used to set the 'this' pointer.
	/// 	for calling getter/setter function (GetterSetterProperty);
	/// 	it will be unused when getting or setting SimpleProperty
	/// 	properties.
	///	This parameter is non-const as nothing prevents an
	///	eventual "Setter" function from actually modifying it,
	///	so we can't promise constness.
	///
	/// @param value
	///	The new value for this property. It will be used as first
	///	argument of the 'setter' function if this is a Getter/Setter
	///	property. @see isGetterSetter().
	///
	/// TODO: have this function check for readOnly property...
	///
	virtual void setValue(as_object& this_ptr, const as_value &value)=0;

	// clone this property
	virtual Property* clone() const=0;
	
	/// is this a read-only member ?
	bool isReadOnly() const { return _flags.get_read_only(); }

	/// is this a Getter/Setter property ?
	virtual bool isGetterSetter() const { return false; }

	/// Mark this property as being reachable (for the GC)
	virtual void setReachable() const=0;
};

/// A simple property, consisting only of an as_value
//
/// This is intended for use only by PropertyList class
///
class SimpleProperty: public Property
{
	/// value
	as_value _value;

public:

	SimpleProperty()
		:
		Property(),
		_value()
	{
	}

	SimpleProperty(const SimpleProperty& p)
		:
		Property(p),
		_value(p._value)
	{
	}

	SimpleProperty(const as_value& value)
		:
		Property(),
		_value(value)
	{
	}

	SimpleProperty(const as_value &value, const as_prop_flags& flags)
		:
		Property(flags),
		_value(value)
	{
	}

	Property* clone() const { return new SimpleProperty(*this); }

	as_value getValue(as_object&) const { return _value; }

	void setValue(as_object&, const as_value &value)  { _value = value; }

	void setReachable() const { _value.setReachable(); }

};

/// A Getter/Setter property
//
/// Basically a small wrapper around GetterSetter.
/// This is intended for use only by PropertyList.
///
class GetterSetterProperty: public Property
{
	/// Actual Getter / Setter  (the workhorse)
	GetterSetter _getset;

public:

	/// Construct a GetterSetterProperty given a GetterSetter
	GetterSetterProperty(const GetterSetter& getset)
		:
		Property(),
		_getset(getset)
	{
	}

	/// Overridden constructor to allow flags specification
	GetterSetterProperty(const GetterSetter& getset,
			const as_prop_flags& flags)
		:
		Property(flags),
		_getset(getset)
	{
	}

	// Copy constructor
	GetterSetterProperty(const GetterSetterProperty& o)
		:
		Property(o),
		_getset(o._getset)
	{
	}

	Property* clone() const { return new GetterSetterProperty(*this); }

	/// Get the value (invokes the getter)
	as_value getValue(as_object& this_ptr) const
	{
		return _getset.getValue(&this_ptr);
	}

	/// Set the value (invokes the setter)
	void setValue(as_object& this_ptr, const as_value &value) 
	{
		_getset.setValue(&this_ptr, value);
	}

	/// This *is* a Getter/Setter property !
	virtual bool isGetterSetter() const { return true; }

	/// Set GetterSetter as reachable (for GC)
	void setReachable() const
	{
		_getset.setReachable();
	}
};


} // namespace gnash

#endif // GNASH_PROPERTY_H
