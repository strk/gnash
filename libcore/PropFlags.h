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

#ifndef GNASH_AS_PROP_FLAGS_H
#define GNASH_AS_PROP_FLAGS_H

#include <ostream>

namespace gnash {

/// Flags defining the level of protection of a member
class PropFlags
{

	/// Numeric flags
	int _flags;

	/// if true, this value is protected (internal to gnash)
	//bool _protected;

public:

	/// Actual flags
	enum Flags {

		/// Protect from enumeration
		dontEnum	= 1 << 0, // 1

		/// Protect from deletion
		dontDelete	= 1 << 1, // 2

		/// Protect from assigning a value
		readOnly	= 1 << 2, // 4

		/// Only visible by VM initialized for version 6 or higher 
		onlySWF6Up 	= 1 << 7, // 128

		/// Ignore in SWF6-initialized VM
		ignoreSWF6	= 1 << 8, // 256

		/// Only visible by VM initialized for version 7 or higher 
		onlySWF7Up 	= 1 << 10, // 1024

		/// Only visible by VM initialized for version 8 or higher 
		onlySWF8Up	= 1 << 12, // 4096

		/// Only visible by VM initialized for version 9 or higher 
		onlySWF9Up	= 1 << 13 // 8192

	};

	/// Default constructor
	PropFlags() : _flags(0)
	{
	}

	/// Constructor
	PropFlags(const bool read_only, const bool dont_delete,
            const bool dont_enum)
		:
		_flags(((read_only) ? readOnly : 0) |
				((dont_delete) ? dontDelete : 0) |
				((dont_enum) ? dontEnum : 0))
	{
	}

	/// Constructor, from numerical value
	PropFlags(const int flags)
		: _flags(flags)
	{
	}

	bool operator== (const PropFlags& o) const
	{
		return ( _flags == o._flags );
	}

	bool operator!= (const PropFlags& o) const
	{
		return ( _flags != o._flags );
	}

	/// Get "read-only" flag 
	bool get_read_only() const
	{
	    return (_flags & readOnly);
	}

	/// Set "read-only" flag 
	void set_read_only() { _flags |= readOnly; }

	/// Clear "read-only" flag 
	void clear_read_only() { _flags &= ~readOnly; }

	/// Get "don't delete" flag
	bool get_dont_delete() const
	{
	    return (_flags & dontDelete);
	}

	/// Set "don't delete" flag
	void set_dont_delete() { _flags |= dontDelete; }

	/// Clear "don't delete" flag 
	void clear_dont_delete() { _flags &= ~dontDelete; }

	/// Get "don't enum" flag
	bool get_dont_enum() const
	{
	    return (_flags & dontEnum);
	}

	/// Set "don't enum" flag
	void set_dont_enum() { _flags |= dontEnum; }

	/// Clear "don't enum" flag 
	void clear_dont_enum() { _flags &= ~dontEnum; }

	/// Get version-based visibility 
	bool get_visible(int swfVersion) const
	{
		if ( _flags & onlySWF6Up && swfVersion < 6 ) return false;
		if ( _flags & ignoreSWF6 && swfVersion == 6 ) return false;
		if ( _flags & onlySWF7Up && swfVersion < 7 ) return false;
		if ( _flags & onlySWF8Up && swfVersion < 8 ) return false;
		if ( _flags & onlySWF9Up && swfVersion < 9 ) return false;
		return true;
	}

	void clear_visible(int swfVersion) 
	{
		if ( swfVersion == 6)
		{
			// version 6, so let's forget onlySWF7Up flag!
			// we will still set the value though, even if that flag is set
			_flags &= ~(onlySWF6Up|ignoreSWF6|onlySWF8Up|onlySWF9Up);
		}
		else
		{
			_flags &= ~(onlySWF6Up|ignoreSWF6|onlySWF7Up|onlySWF8Up|onlySWF9Up);
		}
		
	}

	/// accesor to the numerical flags value
	int get_flags() const { return _flags; }

	/// set the numerical flags value (return the new value )
	/// If unlocked is false, you cannot un-protect from over-write,
	/// you cannot un-protect from deletion and you cannot
	/// un-hide from the for..in loop construct
	///
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return true on success, false on failure (is protected)
	///
	bool set_flags(const int setTrue, const int setFalse = 0)
	{
		_flags &= ~setFalse;
		_flags |= setTrue;
		return true;
	}
};

inline std::ostream&
operator << (std::ostream& os, const PropFlags& fl)
{
	os << "(";
	if ( fl.get_read_only() ) os << " readonly";
	if ( fl.get_dont_delete() ) os << " nodelete";
	if ( fl.get_dont_enum() ) os << " noenum";
	os << " )";
	// TODO: visibility flags
	return os;
}



} // namespace gnash

#endif // GNASH_AS_PROP_FLAGS_H
