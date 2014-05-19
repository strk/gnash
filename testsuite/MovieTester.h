/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *   Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#ifndef GNASH_MOVIETESTER_H
#define GNASH_MOVIETESTER_H

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h" // For exp2 test
#endif

#include "Range2d.h"
#include "GnashKey.h"
#include "sound_handler.h" // for creating the "test" sound handlers
#include "Renderer.h" // for dtor visibility by unique_ptr
#include "Movie.h" 
#include "ManualClock.h" // for composition
#include "RunResources.h" // For initialization.
#include "movie_root.h"

#include <boost/intrusive_ptr.hpp>
#include <vector>
#include <memory> // for unique_ptr
#include <string> 
#include <boost/shared_ptr.hpp>
#include <cmath>

#define check_pixel(x, y, radius, color, tolerance) \
	{\
		std::stringstream ss; \
		ss << "[" << __FILE__ << ":" << __LINE__ << "]"; \
		tester.checkPixel(x, y, radius, color, tolerance, ss.str(), false); \
	}

#define xcheck_pixel(x, y, radius, color, tolerance) \
	{\
		std::stringstream ss; \
		ss << "[" << __FILE__ << ":" << __LINE__ << "]"; \
		tester.checkPixel(x, y, radius, color, tolerance, ss.str(), true); \
	}

// Forward declarations
namespace gnash {
	class movie_definition;
	class movie_root;
	class MovieClip;
	class DisplayObject;
	class FuzzyPixel;
	class VirtualClock;
	class rgba;
}

namespace gnash {

/// A table of built renderers
//
///
class TestingRenderer
{

public:

	TestingRenderer(boost::shared_ptr<Renderer> renderer,
            const std::string& name)
		:
		_name(name),
		_renderer(renderer)
	{}

	const std::string& getName() const { return _name; }

	/// Return the underlying render handler
    boost::shared_ptr<Renderer> getRenderer() const { return _renderer; }

private:

	std::string _name;
    boost::shared_ptr<Renderer> _renderer;
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
	/// Also, initialize any built renderer capable of in-memory
	/// rendering to allow testing of it.
	/// The renderer(s) will be initialized with a memory
	/// buffer with the size found in the SWF header
	///
	MovieTester(const std::string& filespec);

	/// Advance the movie by one frame
	//
    /// Note that the default testing behaviour does not mirror actual
    /// playback; under normal circumstances, movie_root::advance() is
    /// called at a heartbeat rate, while movie_root calculates when to
    /// advance the frame. In order to reproduce this behaviour, call
    /// MovieTester::advance(false), then MovieTester::advanceClock(x), where
    /// x is the heartbeat interval in ms.
    //
    /// Rendering is only triggered when the frame advanced.
    //
	/// @param updateClock
	///	If true (the default), this method also
	///     advances the clock by the nominal delay expected
	///     between frame advancements before performing the
	///     actual playhead advancement.
	void advance(bool updateClock = true);

	/// Advance the clock by the given amount of milliseconds
	void advanceClock(unsigned long ms);

	/// Fully redraw of current frame
	//
	/// This function forces complete redraw in all testing
	/// renderers.
	///
	void redraw();

	/// Return the invalidated ranges in PIXELS
	//
	/// This is to debug/test partial rendering
	///
	geometry::SnappingRanges2d<int> getInvalidatedRanges() const;

	/// Find a DisplayObject in the display list of a sprite by name.
	//
	/// Return NULL if there's no DisplayObject with that name in
	/// the sprite's display list.
	///
	DSOTEXPORT const DisplayObject* findDisplayItemByName(const MovieClip& mc,
			const std::string& name);

	/// Find a DisplayObject on the stage by full target name.
	//
	/// Return NULL if there's no DisplayObject reachable with that target.
	///
	const DisplayObject* findDisplayItemByTarget(const std::string& tgt);

	/// Find a DisplayObject in the display list of a sprite by depth.
	//
	/// Return NULL if there's no DisplayObject at that depth in
	/// the sprite's display list.
	///
	const DisplayObject* findDisplayItemByDepth(const MovieClip& mc,
			int depth);

	/// Get the topmost sprite instance of this movie
    //
    /// We const_cast this because we don't care.
	gnash::MovieClip* getRootMovie() {
		return const_cast<Movie*>(&_movie_root->getRootMovie());
	}

	/// Get the nominal frame rate of the movie associated with this run
	float getFrameRate() const;

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
	/// @param x
	///	The x coordinate of the point being the center
	///	of the circle you want to compute the average color of.
	///
	/// @param y
	///	The y coordinate of the point being the center
	///	of the circle you want to compute the average color of.
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
	///	The tolerated difference of any r,g,b,a values.
	///	Note that the actual tolerance used for comparison might
	///	be bigger then the given one depending on the minimum tolerance
	///	supported by the renderers being tested, being a function of color
	///	depth. For example, comparisions against 16bpp renderers will use
	///	at tolerance of at least 8.
	///
	/// @param label
	///	A label to use in test results.
	///
	/// @param expectFailure
	///	Set to true if a failure is expected. Defaults to false.
	///
	void checkPixel(int x, int y, unsigned radius, const rgba& color,
			short unsigned tolerance, const std::string& label, bool expectFailure=false) const;

    VM& vm() {
        assert(_movie_root);
        return _movie_root->getVM();
    }

	/// Notify mouse button was pressed
	void pressMouseButton();

	/// Notify mouse button was depressed
	void depressMouseButton();

	/// Simulate a mouse click (press and depress mouse button)
	void click();

    /// Simulate a mouse scroll.
    //
    /// The only values seen so far are -1 and 1, but documented to be
    /// usually between -3 and 3. 1 is up, -1 is down.
    void scrollMouse(int delta);

	/// Notify key press
	//
	/// See key codes in namespace gnash::key (GnashKey.h)
	///
	void pressKey(key::code k);

	/// Notify key release
	//
	/// See key codes in namespace gnash::key (GnashKey.h)
	///
	void releaseKey(key::code k);

	/// Return true if the currently active 
	/// DisplayObject is over a DisplayObject that
	/// handles mouse events
	bool isMouseOverMouseEntity();

	/// Return true if a gui would be using an hand
	/// cursor in the current position.
	bool usingHandCursor();

    /// Return true if a streaming sound is active, false if not.
    bool streamingSound() const;

	/// \brief
	/// Return the number of times a sound has been stopped,
	/// or 0 if sound testing is not supported. See canTestSound().
	//
	int soundsStopped();

	/// \brief
	/// Return the number of times a sound has been started,
	/// or 0 if sound testing is not supported. See canTestSound().
	//
	int soundsStarted();

	/// Return true if this build of MovieTester supports sound testing
	//
	/// Sound will be supported as long as a sound handler was compiled in.
	///
	bool canTestSound() const { return _sound_handler.get() != NULL; }

	/// Return true if this build of MovieTester supports pixel checking 
	//
	/// Pixel checking will be supported as long as a testing-capable render handler
	/// was compiled in. Testing-capable means capable of off-screen rendering, which
	/// is implementing the Renderer::initTestBuffer method.
	///
	bool canTestRendering() const { return ! _testingRenderers.empty(); }

	/// Return true if this build of gnash supports video
	bool canTestVideo() const;

	/// Restart the movie
	//
	/// NOTE: the movie returned by getRootMovie() will likely be
	///       NOT the real root movie anymore, so call getRootMovie
	///	  again after this call.
	///
	void restart();

    /// Simulate a manually resized view.
    //
    /// If scaleMode != noScale, the renderers are instructed
    /// to scale the view.
    void resizeStage(int x, int y) ; 

private:

	/// Initialize testing renderers
	void initTestingRenderers();

	/// Initialize sound handlers
	//
	/// For now this function initializes a single sound handler,
	/// the one enabled at configure time.
	/// In the future it might initialize multiple ones (maybe)
	///
	void initTestingSoundHandlers();

	/// Initialize media handlers
	//
	/// For now this function initializes a single media handler,
	/// the one enabled at configure time.
	/// In the future it might initialize multiple ones (maybe)
	///
	void initTestingMediaHandlers();

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
	void render(boost::shared_ptr<Renderer> renderer,
            InvalidatedRanges& invalidated);

	/// Add a testing renderer to the list, initializing it with current
    //viewport size
	void addTestingRenderer(boost::shared_ptr<Renderer> h,
            const std::string& name);

	gnash::movie_root* _movie_root;

	boost::intrusive_ptr<gnash::movie_definition> _movie_def;

    boost::shared_ptr<sound::sound_handler> _sound_handler;

    boost::shared_ptr<media::MediaHandler> _mediaHandler;

    std::unique_ptr<RunResources> _runResources;
	/// Current pointer position - X ordinate
	int _x;

	/// Current pointer position - Y ordinate
	int _y;

	/// Current viewport width
	unsigned _width;

	/// Current viewport height
	unsigned _height;

	/// Invalidated bounds of the movie after last
	/// advance call. They are cached here so we
	/// can safely call ::display w/out wiping this
	/// information out.
	InvalidatedRanges _invalidatedBounds;

    typedef std::vector<TestingRenderer> TestingRenderers;

	TestingRenderers _testingRenderers;

	// When true, pass world invalidated ranges
	// to the renderer(s) at ::render time.
	bool _forceRedraw;

	/// Virtual clock to use to let test runners
	/// control time flow
	ManualClock _clock;

	/// number of samples fetched 
	unsigned int _samplesFetched;      
};

} // namespace gnash

#endif // _GNASH_MOVIETESTER_H
