// CharacterProxy.h - rebindable DisplayObject reference, for Gnash
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_CHARACTER_PROXY_H
#define GNASH_CHARACTER_PROXY_H

#include <string>
#include <cassert>
#include "dsodefs.h"

// Forward declarations
namespace gnash {
	class DisplayObject;
    class movie_root;
}

namespace gnash {

DisplayObject* findDisplayObjectByTarget(const std::string& target,
            movie_root& mr);

/// A proxy for DisplayObject pointers.
//
/// The proxy will store a pointer to a DisplayObject until the 
/// DisplayObject is destroyed, in which case it will only store the original
/// target path of it and always use that for rebinding when needed.
///
class CharacterProxy
{
public:

	/// Construct a CharacterProxy pointing to the given sprite
	CharacterProxy(DisplayObject* sp, movie_root& mr)
		:
		_ptr(sp),
        _mr(&mr)
	{
		checkDangling();
	}

	/// Construct a copy of the given CharacterProxy 
	//
	/// @param sp
	///	The CharacterProxy to make a copy of.
	///	NOTE: if the given proxy is dangling, this proxy
	///	      will also be dangling. If you want to 
	///	      create a non-dangling proxy you can
	///           use the constructor taking a DisplayObject
	///	      as in CharacterProxy newProxy(oldProxy.get())
	///
	CharacterProxy(const CharacterProxy& sp)
        :
        _mr(sp._mr)
	{
		sp.checkDangling();
		_ptr = sp._ptr;
		if (!_ptr) _tgt = sp._tgt;
	}

	/// Make this proxy a copy of the given one
	//
	/// @param sp
	///	The CharacterProxy to make a copy of.
	///	NOTE: if the given proxy is dangling, this proxy
	///	      will also be dangling. If you want to 
	///	      create a non-dangling proxy you can
	///           use the constructor taking a DisplayObject
	///	      as in CharacterProxy newProxy(oldProxy.get())
	CharacterProxy& operator=(const CharacterProxy& sp)
	{
		sp.checkDangling();
		_ptr = sp._ptr;
		if (!_ptr) _tgt = sp._tgt;
        _mr = sp._mr;
		return *this;
	}

	/// Get the pointed sprite, either original or rebound
	//
	/// @return the currently bound sprite, NULL if none
	///
	DisplayObject* get(bool skipRebinding = false) const
	{
		if (skipRebinding) return _ptr;

        // set _ptr to NULL and _tgt to original target if destroyed
		checkDangling(); 
		if (_ptr) return _ptr;
		return findDisplayObjectByTarget(_tgt, *_mr);
	}

	/// Get the sprite target, either current (if not dangling) or
    /// bound one.
	std::string getTarget() const;

	/// Return true if this sprite is dangling
	//
	/// Dangling means that it doesn't have a pointer to the original
	/// sprite anymore, not that it doesn't point to anything.
	/// To know if it points to something or not use get(), which will
	/// return NULL if it doesn't point to anyhing.
	///
	bool isDangling() const
	{
		checkDangling();
		return !_ptr;
	}

	/// \brief
	/// Two sprite_proxies are equal if they point to the
	/// same sprite
	///
	bool operator==(const CharacterProxy& sp) const
	{
		return get() == sp.get();
	}

	/// Set the original sprite (if any) as reachable
	//
	/// NOTE: if this value is dangling, we won't keep anything
	///       alive.
	///
	void setReachable() const;

private:

	/// If we still have a sprite pointer check if it was destroyed
	/// in which case we drop the pointer and only keep the target.
	DSOEXPORT void checkDangling() const;

	mutable DisplayObject* _ptr;

	mutable std::string _tgt;

    movie_root* _mr;

};

} // end namespace gnash

#endif // GNASH_CHARACTER_PROXY_H

