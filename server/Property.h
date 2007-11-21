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

/* $Id: Property.h,v 1.15 2007/11/21 09:21:49 cmusick Exp $ */ 

#ifndef GNASH_PROPERTY_H
#define GNASH_PROPERTY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/variant.hpp>

#include "as_prop_flags.h"
#include "as_value.h"
#include "string_table.h"

namespace gnash {

class as_function;
class PropertyList;

/// Simple Holder for getter/setter functions
class as_accessors
{
public:
	as_function* mGetter;
	as_function* mSetter;

	as_accessors(as_function* getter, as_function* setter) : mGetter(getter),
		mSetter(setter)
	{/**/}
};

/// An abstract property
class Property
{
private:
	friend class PropertyList; // For index access

	/// Properties flags
	as_prop_flags _flags;

	// Store the various types of things that can be held.
	typedef boost::variant<boost::blank, as_value, as_accessors> boundType;
	// Changing this doesn't change the identity of the property, so it is
	// mutable.
	mutable boundType mBound;

	// If true, as soon as getValue has been invoked once, the
	// returned value becomes a fixed return (though it can be
	// overwritten if not readOnly)
	mutable bool mDestructive;
	
	string_table::key mName;
	string_table::key mNamespace;
	// An ordering number, for access by order
	// (AS3 enumeration and slots, AS2 arrays)
	int mOrderId;

	/// Get a value from a getter function.
	as_value getDelayedValue(const as_object& this_ptr) const;

	/// Set a value using a setter function.
	void setDelayedValue(as_object& this_ptr, const as_value& value);

public:
	/// Default constructor
	Property(string_table::key name = 0, string_table::key nsId = 0) : 
		mBound(as_value()), mName(name), mNamespace(nsId)
	{/**/}

	/// Copy constructor
	Property(const Property& p) :
		_flags(p._flags), mBound(p.mBound), mDestructive(p.mDestructive),
		mName(p.mName), mNamespace(p.mNamespace)
	{/**/}

	/// Constructor taking initial flags
	Property(string_table::key name, string_table::key nsId,
		const as_prop_flags& flags) : _flags(flags),
		mBound(as_value()), mDestructive(false), mName(name), mNamespace(nsId)
	{/**/}

	Property(string_table::key name, string_table::key nsId, 
		const as_value& value) : mBound(value), mDestructive(false),
		mName(name), mNamespace(nsId)
	{/**/}

	Property(string_table::key name, string_table::key nsId,
		const as_value& value, const as_prop_flags& flags) :
		_flags(flags), mBound(value), mDestructive(false),
		mName(name), mNamespace(nsId)
	{/**/}

	Property(string_table::key name, string_table::key nsId,
		as_function *getter, as_function *setter, 
		const as_prop_flags& flags, bool destroy = false) :
		_flags(flags), mBound(as_accessors(getter, setter)),
		mDestructive(destroy), mName(name), mNamespace(nsId)
	{/**/}

	Property(string_table::key name, string_table::key nsId,
		as_function *getter, as_function *setter, bool destroy = false) :
		_flags(), mBound(as_accessors(getter, setter)), mDestructive(destroy),
		mName(name), mNamespace(nsId)
	{/**/}

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
	as_value getValue(const as_object& this_ptr) const
	{
		switch (mBound.which())
		{
		case 0: // blank, nothing to do.
			return as_value();
		case 1: // Bound value
			return boost::get<as_value>(mBound);
		case 2: // Getter/setter
			return getDelayedValue(this_ptr);
		} // end of switch
		return as_value(); // Not reached.
	}

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
	void setValue(as_object& this_ptr, const as_value &value)
	{
		switch (mBound.which())
		{
		case 0: // As yet unbound, so make it a simple
		case 1: // Bound value, set. Trust our callers to check read-only.
			mBound = value;
			return;
		case 2: // Getter/setter
			// Destructive are always overwritten.
			if (mDestructive)
			{
				mDestructive = false;
				mBound = value;
			}
			else
				setDelayedValue(this_ptr, value);
			return;
		}
	}

	/// Set the order id
	void setOrder(int order) { mOrderId = order; }

	/// Get the order id
	int getOrder() const { return mOrderId; }

	/// Set the setter
	void setSetter(as_function*);
	/// Get the setter, throws if not a getter/setter
	as_function *getSetter()
	{ return boost::get<as_accessors>(mBound).mSetter; }

	/// Set the getter
	void setGetter(as_function*);
	/// Get the getter, throws if not a getter/setter
	as_function *getGetter()
	{ return boost::get<as_accessors>(mBound).mGetter; }

	/// is this a read-only member ?
	bool isReadOnly() const { return _flags.get_read_only(); }

	/// Is this a getter/setter property?
	bool isGetterSetter() const { return mBound.which() == 2; }

	/// is this a destructive property ?
	bool isDestructive() const { return mDestructive; }

	/// Is this a static property?
	bool isStatic() const { return _flags.get_static(); }

	/// Is this member supposed to be visible by a VM of given version ?
	bool isVisible(int swfVersion) const { return _flags.get_visible(swfVersion); }

	/// Clear visibility flags
	void clearVisible(int swfVersion) { _flags.clear_visible(swfVersion); }

	/// What is the name of this property?
	string_table::key getName() const { return mName; }

	/// What is the namespace of this property?
	string_table::key getNamespace() const { return mNamespace; }

	/// Mark this property as being reachable (for the GC)
	void setReachable() const;
};

} // namespace gnash

#endif // GNASH_PROPERTY_H
