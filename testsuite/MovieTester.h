/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#ifndef _GNASH_MOVIETESTER_H
#define _GNASH_MOVIETESTER_H

#include "Range2d.h"

#include <memory> // for auto_ptr
#include <string> // for auto_ptr

// Forward declarations
namespace gnash {
	class movie_definition;
	class movie_root;
	class sprite_instance;
	class character;
}

namespace gnash {

/// An utility class for testing movie playback
//
/// This is a just born implementation and doesn't
/// have much more then simply loading a movie and
/// providing a function to find DisplayItems by name
///
/// More functions will be added when needed.
///
class MovieTester
{
public:
	/// Fully load the movie at the specified location
	/// and create an instance of it.
	MovieTester(const std::string& filespec);

	/// Advance the movie by one frame
	void advance();

	/// Return the invalidated bounds in PIXELS
	//
	/// This is to debug/test partial rendering
	///
	geometry::Range2d<int> getInvalidatedBounds() const;

	/// Find a character in the display list of a sprite by name.
	//
	/// Return NULL if there's no character with that name in
	/// the sprite's display list.
	///
	const character* findDisplayItemByName(const sprite_instance& mc,
			const std::string& name);

	/// Find a character in the display list of a sprite by depth.
	//
	/// Return NULL if there's no character at that depth in
	/// the sprite's display list.
	///
	const character* findDisplayItemByDepth(const sprite_instance& mc,
			int depth);

	/// Get the topmost sprite instance of this movie
	gnash::sprite_instance* getRootMovie() {
		return _movie;
	}

	/// Notify mouse pointer movement to the given coordinate
	//
	/// Coordinates are in pixels
	///
	void movePointerTo(int x, int y);

	/// Notify mouse button was pressed
	void pressMouseButton();

	/// Notify mouse button was depressed
	void depressMouseButton();

	/// Return true if the currently active 
	/// character is over a character that
	/// handles mouse events
	bool isMouseOverMouseEntity();

private:

	gnash::movie_root* _movie_root;

	gnash::movie_definition* _movie_def;

	gnash::sprite_instance* _movie;

};

} // namespace gnash

#endif // _GNASH_MOVIETESTER_H
