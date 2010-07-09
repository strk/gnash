// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "PropFlags.h"
#include "as_value.h"
#include "ObjectURI.h"

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
	{}

	/// Construct a native getter-setter
	GetterSetter(as_c_function_ptr getter, as_c_function_ptr setter)
		:
		_getset(NativeGetterSetter(getter, setter))
	{}

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
		switch (_getset.which())
		{
			case 0: // user-defined
				return boost::get<UserDefinedGetterSetter>(
                        _getset).getUnderlying();
		}
		static as_value undefVal;
		return undefVal;
	}

	void markReachableResources() const
	{
		if (_getset.which() == 0) {
			boost::get<UserDefinedGetterSetter>(
                    _getset).markReachableResources();
		}
	}

private:

	/// User-defined getter/setter
	class UserDefinedGetterSetter
    {
	public:

		UserDefinedGetterSetter(as_function* get, as_function* set)
			:
			_getter(get),
			_setter(set),
			_underlyingValue(),
			_beingAccessed(false)
		{}

		/// Invoke the getter
		as_value get(fn_call& fn) const;

		/// Invoke the setter
		void set(fn_call& fn);

		/// Get the underlying value
		const as_value& getUnderlying() const { return _underlyingValue; }

		/// Set the underlying value
		void setUnderlying(const as_value& v) { _underlyingValue = v; }

		void markReachableResources() const;

	private:

		as_function* _getter;
		as_function* _setter;

		as_value _underlyingValue;

		mutable bool _beingAccessed;

		/// For SWF6 (not higher) a user-defined getter-setter would not
        /// be invoked while being set. This ScopedLock helps marking a
        /// Getter-Setter as being invoked in an exception-safe manner.
        //
        /// Note this is not thread safe and does not attempt to provide 
        /// thread safety.
		class ScopedLock : boost::noncopyable
        {
		public:

			ScopedLock(const UserDefinedGetterSetter& na)
                :
                _a(na),
                _obtainedLock(_a._beingAccessed ? false : true)
			{
                // If we didn't obtain the lock it would be true anyway,
                // but it's probably polite to avoid touching it.
                if (_obtainedLock) _a._beingAccessed = true;
			}

			~ScopedLock() { if ( _obtainedLock) _a._beingAccessed = false; }

			/// Return true if the lock was obtained
			//
			/// If false is returned, we're being called recursively,
			/// which means we should set the underlyingValue instead
			/// of calling the setter (for SWF6, again).
			///
			bool obtainedLock() const { return _obtainedLock; }

        private:

			const UserDefinedGetterSetter& _a;
			bool _obtainedLock;

        };
    };

	/// Native GetterSetter
	class NativeGetterSetter {

	public:

		NativeGetterSetter(as_c_function_ptr get, as_c_function_ptr set)
			:
			_getter(get), _setter(set) {}

		/// Invoke the getter
		as_value get(fn_call& fn) const
		{
			return _getter(fn);
		}

		/// Invoke the setter
		void set(fn_call& fn)
		{
			_setter(fn);
		}

	private:

		as_c_function_ptr _getter;
		as_c_function_ptr _setter;
	};

	boost::variant<UserDefinedGetterSetter, NativeGetterSetter> _getset;

};

/// An abstract property
//
/// A Property is a holder for a value or a getter-setter.
//
/// Properties have special const semantics: the value of a Property does
/// not affect its outward state, so the value of a const Property can be
/// changed.
class Property
{
public:
	/// Default constructor
	Property(const ObjectURI& uri)
        : 
		_bound(as_value()),
        _destructive(false),
        _uri(uri)
	{}

	Property(const ObjectURI& uri, const as_value& value,
            const PropFlags& flags = PropFlags())
        :
		_flags(flags),
        _bound(value),
        _destructive(false),
		_uri(uri)
	{}

	Property(const ObjectURI& uri,
		as_function *getter, as_function *setter, 
		const PropFlags& flags, bool destroy = false)
        :
		_flags(flags), 
        _bound(GetterSetter(getter, setter)),
		_destructive(destroy),
        _uri(uri)
	{}

	Property(const ObjectURI& uri, as_function *getter, as_function *setter,
            bool destroy = false)
        :
		_flags(),
        _bound(GetterSetter(getter, setter)),
        _destructive(destroy),
        _uri(uri)
	{}

	Property(const ObjectURI& uri, as_c_function_ptr getter,
            as_c_function_ptr setter, const PropFlags& flags,
            bool destroy = false)
		:
		_flags(flags),
        _bound(GetterSetter(getter, setter)),
        _destructive(destroy),
        _uri(uri)
	{}
	
    /// Copy constructor
	Property(const Property& p)
        :
		_flags(p._flags),
        _bound(p._bound),
        _destructive(p._destructive),
        _uri(p._uri)
	{}

	/// accessor to the properties flags
	const PropFlags& getFlags() const { return _flags; }

    /// Set the flags of the property
    void setFlags(const PropFlags& flags) const {
        _flags = flags;
    }

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
	const as_value& getCache() const;

	/// Set internal cached value of this property
	//
	/// For simple properties, this is the usual value;
	/// for user-defined getter-setter this is a cached value
	/// to watch for infinitely recurse on calling the getter
	/// or setter; Native getter-setter has no cache,
	/// nothing would happen for them.
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
	void setValue(as_object& this_ptr, const as_value &value) const;

	/// Is this a getter/setter property?
	bool isGetterSetter() const {
        return _bound.which() == TYPE_GETTER_SETTER;
    }

	/// is this a destructive property ?
	bool isDestructive() const { return _destructive; }

	/// Clear visibility flags
	void clearVisible(int swfVersion) { _flags.clear_visible(swfVersion); }

    /// The name-namespace pair (ObjectURI) of this Property
    const ObjectURI& uri() const {
        return _uri;
    }

	/// Mark this property as being reachable (for the GC)
	void setReachable() const;

private:
	
    /// Get a value from a getter function.
	as_value getDelayedValue(const as_object& this_ptr) const;

	/// Set a value using a setter function.
	void setDelayedValue(as_object& this_ptr, const as_value& value) const;

    enum Type {
        TYPE_EMPTY,
        TYPE_VALUE,
        TYPE_GETTER_SETTER
    };

	/// Properties flags
	mutable PropFlags _flags;

	// Store the various types of things that can be held.
	typedef boost::variant<boost::blank, as_value, GetterSetter> BoundType;

    /// The value of the property.
	mutable BoundType _bound;

	// If true, as soon as getValue has been invoked once, the
	// returned value becomes a fixed return (though it can be
	// overwritten if not readOnly)
	mutable bool _destructive;
	
    // TODO: this should be const, but the assignment operator is still needed 
    ObjectURI _uri;

};
	
/// is this a read-only member ?
inline bool
readOnly(const Property& prop) {
    return prop.getFlags().get_read_only();
}

/// Is this member supposed to be visible by a VM of given version ?
inline bool
visible(const Property& prop, int version) {
    return prop.getFlags().get_visible(version);
}

} // namespace gnash

#endif // GNASH_PROPERTY_H
