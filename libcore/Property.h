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


#ifndef GNASH_PROPERTY_H
#define GNASH_PROPERTY_H

#include "as_prop_flags.h"
#include "as_value.h"
#include "string_table.h"
#include "log.h"

#include <boost/variant.hpp>
#include <cassert>

namespace gnash {

typedef as_value (*as_c_function_ptr)(const fn_call& fn);

class as_function;
class PropertyList;

/// Holder for getter/setter functions
//
/// Getter setter can be user-defined or native ones.
/// This class abstracts the two.
///
class GetterSetter
{
public:

	/// Construct a user-defined getter-setter
	GetterSetter(as_function* getter, as_function* setter)
		:
		_getset(UserDefinedGetterSetter(getter, setter))
	{/**/}

	/// Construct a native getter-setter
	GetterSetter(as_c_function_ptr getter, as_c_function_ptr setter)
		:
		_getset(NativeGetterSetter(getter, setter))
	{/**/}

	/// Invoke the getter
	as_value get(fn_call& fn) const
	{
		switch ( _getset.which() )
		{
			case 0: // user-defined
				return boost::get<UserDefinedGetterSetter>(_getset).get(fn);
				break;
			case 1: // native 
				return boost::get<NativeGetterSetter>(_getset).get(fn);
				break;
		}
		return as_value(); // not reached (TODO: log error ? assert ?)
	}

	/// Invoke the setter
	void set(fn_call& fn)
	{
		switch ( _getset.which() )
		{
			case 0: // user-defined
				boost::get<UserDefinedGetterSetter>(_getset).set(fn);
				break;
			case 1: // native 
				boost::get<NativeGetterSetter>(_getset).set(fn);
				break;
		}
	}

	/// Set the cache value (for user-defined getter-setters)
	void setCache(const as_value& v)
	{
		switch ( _getset.which() )
		{
			case 0: // user-defined
				boost::get<UserDefinedGetterSetter>(_getset).setUnderlying(v);
				break;
			case 1: // native 
				// nothing to do for native
				break;
		}
	}

	/// Get the cache value (for user-defined getter-setters)
	const as_value& getCache() const
	{
		switch ( _getset.which() )
		{
			case 0: // user-defined
				return boost::get<UserDefinedGetterSetter>(_getset).getUnderlying();
		}
		static as_value undefVal;
		return undefVal;
	}

	/// Set a user-defined getter
	void setGetter(as_function* fun)
	{
		if ( _getset.which() == 0 )
		{
			boost::get<UserDefinedGetterSetter>(_getset).setGetter(fun);
		}
	}

	/// Set a user-defined setter
	void setSetter(as_function* fun)
	{
		if ( _getset.which() == 0 )
		{
			boost::get<UserDefinedGetterSetter>(_getset).setSetter(fun);
		}
	}

	void markReachableResources() const
	{
		if ( _getset.which() == 0 )
		{
			boost::get<UserDefinedGetterSetter>(_getset).markReachableResources();
		}
	}

private:

	/// User-defined getter/setter
	class UserDefinedGetterSetter {
	public:
		UserDefinedGetterSetter(as_function* get, as_function* set)
			:
			mGetter(get),
			mSetter(set),
			underlyingValue(),
			beingAccessed(false)
		{}

		/// Invoke the getter
		as_value get(fn_call& fn) const;

		/// Invoke the setter
		void set(fn_call& fn);

		/// Get the underlying value
		const as_value& getUnderlying() const { return underlyingValue; }

		/// Set the underlying value
		void setUnderlying(const as_value& v) { underlyingValue=v; }

		/// Set the setter
		void setSetter(as_function* setter) { mSetter = setter; }

		/// Set the getter
		void setGetter(as_function* getter) { mGetter = getter; }

		void markReachableResources() const;

	private:
		as_function* mGetter;
		as_function* mSetter;

		as_value underlyingValue;

		mutable bool beingAccessed;

		/// For SWF6 (not higher) a user-defined getter-setter would not be invoked
		/// while being set. This ScopedLock helps marking a
		/// Getter-Setter as being invoked in an exception-safe
		/// manner.
		class ScopedLock {
			const UserDefinedGetterSetter& a;
			bool obtainedLock;

			ScopedLock(ScopedLock&);
			ScopedLock& operator==(ScopedLock&);
		public:

			ScopedLock(const UserDefinedGetterSetter& na) : a(na)
			{
				if ( a.beingAccessed ) obtainedLock=false;
				else {
					a.beingAccessed = true;
					obtainedLock = true;
				}
			}

			~ScopedLock() { if ( obtainedLock) a.beingAccessed = false; }

			/// Return true if the lock was obtained
			//
			/// If false is returned, we're being called recursively,
			/// which means we should set the underlyingValue instead
			/// of calling the setter (for SWF6, again).
			///
			bool obtained() const { return obtainedLock; }
		};

		friend class ScopedLock;

	};

	/// Native GetterSetter
	class NativeGetterSetter {

	public:

		NativeGetterSetter(as_c_function_ptr get, as_c_function_ptr set)
			:
			cGetter(get), cSetter(set) {}

		/// Invoke the getter
		as_value get(fn_call& fn) const
		{
			return cGetter(fn);
		}

		/// Invoke the setter
		void set(fn_call& fn)
		{
			cSetter(fn);
		}

	private:

		as_c_function_ptr cGetter;

		as_c_function_ptr cSetter;
	};

	boost::variant<UserDefinedGetterSetter, NativeGetterSetter> _getset;

};

/// An abstract property
class Property
{
private:
	friend class PropertyList; // For index access

	/// Properties flags
	as_prop_flags _flags;

	// Store the various types of things that can be held.
	typedef boost::variant<boost::blank, as_value, GetterSetter> boundType;

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
	Property(string_table::key name = 0, string_table::key nsId = 0)
        : 
		mBound(as_value()),
        mDestructive(false),
        mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	/// Copy constructor
	Property(const Property& p)
        :
		_flags(p._flags),
        mBound(p.mBound),
        mDestructive(p.mDestructive),
		mName(p.mName),
        mNamespace(p.mNamespace),
        mOrderId(p.mOrderId)
	{}

	/// Constructor taking initial flags
	Property(string_table::key name, string_table::key nsId,
		const as_prop_flags& flags)
        :
        _flags(flags),
		mBound(as_value()),
        mDestructive(false),
        mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	Property(string_table::key name, string_table::key nsId, 
		const as_value& value)
        :
        mBound(value),
        mDestructive(false),
		mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	Property(string_table::key name, string_table::key nsId,
		const as_value& value, const as_prop_flags& flags)
        :
		_flags(flags),
        mBound(value),
        mDestructive(false),
		mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	Property(string_table::key name, string_table::key nsId,
		as_function *getter, as_function *setter, 
		const as_prop_flags& flags, bool destroy = false)
        :
		_flags(flags), 
        mBound(GetterSetter(getter, setter)),
		mDestructive(destroy),
        mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	Property(string_table::key name, string_table::key nsId,
		as_function *getter, as_function *setter, bool destroy = false)
        :
		_flags(),
        mBound(GetterSetter(getter, setter)),
        mDestructive(destroy),
		mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	Property(string_table::key name, string_table::key nsId,
		as_c_function_ptr getter, as_c_function_ptr setter,
		const as_prop_flags& flags, bool destroy = false)
		:
		_flags(flags),
        mBound(GetterSetter(getter, setter)),
        mDestructive(destroy),
		mName(name),
        mNamespace(nsId),
		mOrderId(0)
	{}

	/// Set a user-defined setter
	void setSetter(as_function* fun);

	/// Set a user-defined getter
	void setGetter(as_function* fun);

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
	/// @return the value of this property
	///
	as_value getValue(const as_object& this_ptr) const;

	/// Get internal cached value of this property
	//
	/// For simple properties, this is the usual value;
	/// for user-defined getter-setter this is a cached value
	/// to watch for infinitely recurse on calling the getter
	/// or setter; Native getter-setter has no cache,
	/// undefined will be returned for them.
	///
	const as_value& getCache() const;

	/// Set internal cached value of this property
	//
	/// For simple properties, this is the usual value;
	/// for user-defined getter-setter this is a cached value
	/// to watch for infinitely recurse on calling the getter
	/// or setter; Native getter-setter has no cache,
	/// nothing would happen for them.
	///
	void setCache(const as_value& v);

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
	void setValue(as_object& this_ptr, const as_value &value);

	/// Set the order id
	//
	/// NOTE: this field is used by one of the indexes
	///       in the boost::multi_index used by PropertyList,
	///       so changing this value on an instance which was
	///       put in that index might result in corruption of
	///       the index invariant. (at least this is what happens
	///       with standard containers indexed on an element's member).
	///
	void setOrder(int order) { mOrderId = order; }

	/// Get the order id
	int getOrder() const { return mOrderId; }

	/// is this a read-only member ?
	bool isReadOnly() const { return _flags.get_read_only(); }

	/// Is this a getter/setter property?
	bool isGetterSetter() const { return mBound.which() == 2; }

	/// is this a destructive property ?
	bool isDestructive() const { return mDestructive; }

	/// Is this a static property?
	bool isStatic() const { return _flags.get_static(); }

	/// Is this member supposed to be visible by a VM of given version ?
	bool isVisible(int swfVersion) const {
        return _flags.get_visible(swfVersion);
    }

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
