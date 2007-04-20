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
#include "render_handler.h" // for dtor visibility by auto_ptr

#include <memory> // for auto_ptr
#include <string> 
#include <boost/shared_ptr.hpp>

#define check_pixel(radius, color, tolerance) \
	{\
		std::stringstream ss; \
		ss << "[" << __FILE__ << ":" << __LINE__ << "]"; \
		tester.checkPixel(2, rgba(0,0,0,255), 1, ss.str(), false); \
	}

#define xcheck_pixel(radius, color, tolerance) \
	{\
		std::stringstream ss; \
		ss << "[" << __FILE__ << ":" << __LINE__ << "]"; \
		tester.checkPixel(2, rgba(0,0,0,255), 1, ss.str(), true); \
	}

// Forward declarations
namespace gnash {
	class movie_definition;
	class movie_root;
	class sprite_instance;
	class character;
	class FuzzyPixel;
}

namespace gnash {

/// A table of built renderers
//
///
class TestingRenderer
{

public:

	TestingRenderer(std::auto_ptr<render_handler> renderer, const std::string& name)
		:
		_name(name),
		_renderer(renderer)
	{}

	const std::string& getName() const { return _name; }

	/// Return the underlying render handler
	render_handler& getRenderer() const { return *_renderer; }

private:

	std::string _name;
	std::auto_ptr<render_handler> _renderer;
};

typedef boost::shared_ptr<TestingRenderer> TestingRendererPtr;

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
	/// Also, initialize any built renderer capable of in-memory
	/// rendering to allow testing of it.
	/// The renderer(s) will be initialized with a memory
	/// buffer with the size found in the SWF header
	///
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

	/// Check color of the average pixel under the mouse pointer 
	//
	///
	/// This method will test any built renderer.
	///
	/// @param radius
	///	Radius defining the average zone used.
	///	1 means a single pixel.
	///	Behaviour of passing 0 is undefined.
	///
	/// @param color
	///	The color we expect to find under the pointer.
	///
	/// @param tolerance
	///	The tolerated difference of any r,g,b,a values
	///
	/// @param label
	///	A label to use in test results.
	///
	/// @param expectFailure
	///	Set to true if a failure is expected. Defaults to false.
	///
	void checkPixel(unsigned radius, const rgba& color, short unsigned tolerance, const std::string& label, bool expectFailure=false) const;

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

	/// Initialize testing renderers
	void initTestingRenderers();

	/// Render the current movie to all testing renderers
	//
	/// This function calls movie_root::display internally
	///
	void render();

	/// Render the current movie to a specific testing renderer
	//
	/// @param renderer
	///	The renderer to draw to. It will be temporarly set as
	///	the global renderer in the gnash core lib.
	///
	/// @param invalidated
	///	The invalidated ranges as computed by the core lib.
	///
	void render(render_handler& renderer, InvalidatedRanges& invalidated);

	/// Add a testing renderer to the list, initializing it with current viewport size
	void addTestingRenderer(std::auto_ptr<render_handler> h, const std::string& name);

	gnash::movie_root* _movie_root;

	gnash::movie_definition* _movie_def;

	gnash::sprite_instance* _movie;

	std::auto_ptr<TEST_sound_handler> _sound_handler;

	/// Current pointer position - X ordinate
	int _x;

	/// Current pointer position - Y ordinate
	int _y;

	/// Current viewport width
	unsigned _width;

	/// Current viewport height
	unsigned _height;

	/// I'd use ptr_list here, but trying not to spread
	/// boost 1.33 requirement for the moment.
	/// Still, I'd like to simplify things...
	/// is shared_ptr fine ?
	typedef std::vector< TestingRendererPtr > TRenderers;

	std::vector< TestingRendererPtr > _testingRenderers;
};

} // namespace gnash

#endif // _GNASH_MOVIETESTER_H
