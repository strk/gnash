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

	/// accessor to the value
	virtual as_value getValue(as_object* this_ptr) const=0;

	/// set the value
	virtual void setValue(as_object* this_ptr, const as_value &value)=0;

	// clone this property
	virtual Property* clone() const=0;
	
	/// is this a read-only member ?
	bool isReadOnly() const { return _flags.get_read_only(); }
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

	as_value getValue(as_object*) const { return _value; }

	/// set the value
	void setValue(as_object*, const as_value &value)  { _value = value; }

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
	as_value getValue(as_object* this_ptr) const
	{
		as_value ret;
		_getset.getValue(this_ptr, ret);
		return ret;
	}

	/// Set the value (invokes the setter)
	void setValue(as_object* this_ptr, const as_value &value) 
	{
		_getset.setValue(this_ptr, value);
	}

};


} // namespace gnash

#endif // GNASH_PROPERTY_H
