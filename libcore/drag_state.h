// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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



#ifndef GNASH_DRAG_STATE_H
#define GNASH_DRAG_STATE_H

#include "rect.h" // for composition
#include "smart_ptr.h" // we keep character being dragged by intrusive_ptr

namespace gnash
{

// Forward declarations
class character;


/// What is being dragged and how
class drag_state
{

	bool _hasbounds;

	/// Boundaries to constraint the drag into.
	/// Coordinates in TWIPS.
	/// TODO: use Range2d<float> directly ?
	rect _bounds;

	boost::intrusive_ptr<character> _character;

	bool	_lock_centered;

	/// Offsets of displacement from character origin
	/// at time of drag start. These are used for non
	/// lock-centered dragging.
	/// Coordinates are in stage space (TWIPS)
	boost::int32_t  _xoffset;
	boost::int32_t  _yoffset;

public:

	bool isLockCentered() const {
		return _lock_centered;
	}

	void setLockCentered(bool lock) {
		_lock_centered = lock;
	}

	/// Set displacement offset from origin
	/// at time of drag start.
	/// Coordinates are in stage space (twips)
	///
	void setOffset(boost::int32_t x, boost::int32_t y)
	{
		_xoffset = x;
		_yoffset = y;
	}

	boost::int32_t xOffset() const { return _xoffset; }
	boost::int32_t yOffset() const { return _yoffset; }

	bool hasBounds() const {return _hasbounds; }

	/// \brief
	/// Get the boundaries to constraint
	/// the drag into.
	//
	/// Coordinates of the rectangle are
	/// expected in TWIPS.
	///
	/// Note that if hasBounds() is false
	/// the returned rectangle is the NULL
	/// rectangle - see rect::is_null().
	///
	const rect& getBounds() const { return _bounds; }

	/// \brief
	/// Set the boundaries to constraint
	/// the drag into.
	//
	/// Coordinates of the rectangle are
	/// expected in TWIPS.
	///
	void setBounds(const rect& bounds) {
		_bounds = bounds;
		_hasbounds = true;
	}

	/// May return NULL !!
	character* getCharacter() const {
		return _character.get();
	}

	/// Stores character in an intrusive pointer
	void setCharacter(character* ch) {
		_character = ch;
	}

	/// Reset drag state to its initial condition
	void reset()
	{
		_character = NULL;
		_hasbounds = false;
		_bounds.set_null();
		_lock_centered = false;
	}

	drag_state()
		:
		_hasbounds(false),
		_bounds(),
		_character(0),
		_lock_centered(false)
	{
	}

	/// Mark character as reachable (if any)
	void markReachableResources() const
	{
		if ( _character ) _character->setReachable();
	}
};


} // namespace gnash

#endif // GNASH_DRAG_STATE_H
