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
#include "gnash.h" // for namespace key
#include "sound_handler_test.h" // for creating the "test" sound handler
#include "types.h" // for rgba class

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

/// An utility class used to compare rgba values with a given tolerance
class FuzzyPixel
{

public:

	friend std::ostream& operator<< (std::ostream& o, const FuzzyPixel& p);

	/// Construct a black, alpha 0 FuzzyPixel with NO tolerance.
	//
	/// No tolerance means that any comparison will fail
	///
	FuzzyPixel()
		:
		_col(0,0,0,0),
		_tol(0)
	{
	}

	/// Construct a FuzzyPixel with given color and tolerance
	//
	/// @param color
	///	The color value
	///
	/// @param tolerance
	///	The tolerance to use in comparisons
	///
	FuzzyPixel(rgba& color, int tolerance=0)
		:
		_col(color),
		_tol(tolerance)
	{
	}

	/// Construct a FuzzyPixel with given values
	//
	/// @param r
	///	The red value.
	///
	/// @param g
	///	The green value.
	///
	/// @param b
	///	The blue value.
	///
	/// @param a
	///	The alpha value.
	///
	FuzzyPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		:
		_col(r, g, b, a),
		_tol(0)
	{
	}

	/// Set the tolerance to use in comparisons
	//
	/// @param tol
	///	The tolerance to use in comparisons
	///
	void setTolerance(int tol)
	{
		_tol = tol;
	}

	/// Compare two FuzzyPixel using the tolerance of the most tolerant of the two
	//
	/// Note that if any of the two operands has 0 tolerance, any equality
	/// comparison will fail.
	///
	bool operator==(const FuzzyPixel& other) const;

	// Return true if a and b are below a given tolerance
	static bool fuzzyEqual(int a, int b, int tol)
	{
		return abs(a-b) <= tol;
	}

private:

	rgba _col;

	// tolerance value
	int _tol;

};

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

	/// Get the average pixel under the mouse pointer
	//
	/// @param radius
	///	Radius defining the average zone used.
	///	1 means a single pixel.
	///	Behaviour of passing 0 is undefined.
	///
	/// @param tolerance
	///	The tolerance value to use for the returned FuzzyPixel.
	///
	/// Note that if current pointer is outside of the rendered region
	/// an intollerant FuzzyPixel is returned.
	///
	FuzzyPixel getAveragePixel(unsigned radius, int tolerance) const;

	/// Notify mouse button was pressed
	void pressMouseButton();

	/// Notify mouse button was depressed
	void depressMouseButton();

	/// Simulate a mouse click (press and depress mouse button)
	void click()
	{
		pressMouseButton();
		depressMouseButton();
	}

	/// Notify key press
	//
	/// See key codes in namespace gnash::key (gnash.h)
	///
	void pressKey(key::code k);

	/// Notify key release
	//
	/// See key codes in namespace gnash::key (gnash.h)
	///
	void releaseKey(key::code k);

	/// Return true if the currently active 
	/// character is over a character that
	/// handles mouse events
	bool isMouseOverMouseEntity();

	/// Return the number of times a sound has been stopped.
	int soundsStopped();

	/// Return the number of times a sound has been started.
	int soundsStarted();

private:

	gnash::movie_root* _movie_root;

	gnash::movie_definition* _movie_def;

	gnash::sprite_instance* _movie;

	std::auto_ptr<TEST_sound_handler> _sound_handler;

	int _x;

	int _y;
};

} // namespace gnash

#endif // _GNASH_MOVIETESTER_H
