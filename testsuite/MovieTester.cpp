/* 
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#include "MovieTester.h"
#include "GnashException.h"
#include "URL.h"
#include "noseek_fd_adapter.h"
#include "movie_definition.h"
#include "movie_instance.h"
#include "movie_root.h"
#include "sprite_instance.h"
#include "as_environment.h"
#include "gnash.h" // for create_movie and create_library_movie and for gnash::key namespace
#include "VM.h" // for initialization
#include "sound_handler_test.h" // for creating the "test" sound handler
#include "render.h" // for get_render_handler
#include "types.h" // for rgba class
#include "FuzzyPixel.h"
#include "render.h"
#include "render_handler.h"
#include "render_handler_agg.h"

#include <cstdio>
#include <string>
#include <memory> // for auto_ptr

#define SHOW_INVALIDATED_BOUNDS_ON_ADVANCE 1

#ifdef SHOW_INVALIDATED_BOUNDS_ON_ADVANCE
#include <sstream>
#endif


namespace gnash {

MovieTester::MovieTester(const std::string& url)
{
	if ( url == "-" )
	{
		std::auto_ptr<tu_file> in (
				noseek_fd_adapter::make_stream(fileno(stdin))
				);
		_movie_def = gnash::create_movie(in, url, false);
	}
	else
	{
		// _url should be always set at this point...
		_movie_def = gnash::create_library_movie(URL(url), NULL, false);
	}

	// TODO: use PWD if url == '-'
	set_base_url(url);

	if ( ! _movie_def )
	{
		throw GnashException("Could not load movie from "+url);
	}

	// Create a soundhandler
	_sound_handler.reset( static_cast<TEST_sound_handler*>(gnash::create_sound_handler_test()));
	gnash::set_sound_handler(_sound_handler.get());

	_movie_root = &(VM::init(*_movie_def).getRoot());

	// Initialize viewport size with the one advertised in the header
	_width = unsigned(_movie_def->get_width_pixels());
	_height = unsigned(_movie_def->get_height_pixels());

	// Initialize the testing renderers
	initTestingRenderers();

	// Now complete load of the movie
	_movie_def->completeLoad();
	_movie_def->ensure_frame_loaded(_movie_def->get_frame_count());

	_movie = _movie_root->get_root_movie();
	assert(_movie);

	// Activate verbosity so that self-contained testcases are
	// also used 
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);


	// Now place the root movie on the stage
	advance();
}

void
MovieTester::render(render_handler& h, InvalidatedRanges& invalidated_regions) 
{
	set_render_handler(&h);

	invalidated_regions.setWorld(); // testing
	h.set_invalidated_regions(invalidated_regions);

	// We call display here to simulate effect of a real run.
	//
	// What we're particularly interested about is 
	// proper computation of invalidated bounds, which
	// needs clear_invalidated() to be called.
	// display() will call clear_invalidated() on characters
	// actually modified so we're fine with that.
	//
	// Directly calling _movie->clear_invalidated() here
	// also work currently, as invalidating the topmost
	// movie will force recomputation of all invalidated
	// bounds. Still, possible future changes might 
	// introduce differences, so better to reproduce
	// real runs as close as possible, by calling display().
	//
	_movie_root->display();
}

void
MovieTester::render() 
{
	InvalidatedRanges ranges;
	_movie_root->add_invalidated_bounds(ranges, false);
	cout << "Invalidated ranges before any advance: " << ranges << endl;

	for (TRenderers::const_iterator it=_testingRenderers.begin(), itE=_testingRenderers.end();
				it != itE; ++it)
	{
		TestingRenderer& rend = *(*it);
		render(rend.getRenderer(), ranges);
	}
}

void
MovieTester::advance() 
{
	render();

	_movie_root->advance(1.0);
#ifdef SHOW_INVALIDATED_BOUNDS_ON_ADVANCE
	geometry::Range2d<float> invalidatedbounds = getInvalidatedBounds();
	std::stringstream ss;
	ss << "frame " << _movie->get_current_frame() << ") Invalidated bounds " << invalidatedbounds;
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile << ss.str().c_str() << std::endl;
#endif

}

const character*
MovieTester::findDisplayItemByName(const sprite_instance& mc,
		const std::string& name) 
{
	const DisplayList& dlist = mc.getDisplayList();
	return dlist.get_character_by_name(name);
}

const character*
MovieTester::findDisplayItemByDepth(const sprite_instance& mc,
		int depth)
{
	const DisplayList& dlist = mc.getDisplayList();
	return dlist.get_character_at_depth(depth);
}

void
MovieTester::movePointerTo(int x, int y)
{
	_x = x;
	_y = y;
	_movie_root->notify_mouse_moved(x, y);
}

void
MovieTester::checkPixel(unsigned radius, const rgba& color,
		short unsigned tolerance, const std::string& label, bool expectFailure) const
{
	FuzzyPixel exp(color, tolerance);
	const char* X="";
	if ( expectFailure ) X="X";

	//cout <<"BINGO: X is '"<< X<<"'"<<endl;

	for (TRenderers::const_iterator it=_testingRenderers.begin(), itE=_testingRenderers.end();
				it != itE; ++it)
	{
		const TestingRenderer& rend = *(*it);

		std::stringstream ss;
		ss << rend.getName() <<" ";
		ss << "pix:" << _x << "," << _y <<" ";

		rgba obt_col;

	        if ( ! rend.getRenderer().getAveragePixel(obt_col, _x, _y, radius) )
		{
			ss << " is out of rendering buffer";
			log_msg("%sFAILED: %s (%s)", X,
					ss.str().c_str(),
					label.c_str()
					);
		}

	        ss << "exp:" << color.toShortString() << " ";
	        ss << "obt:" << obt_col.toShortString() << " ";
	        ss << "tol:" << tolerance;

		FuzzyPixel obt(obt_col, tolerance);
		if (exp ==  obt)
		{
			log_msg("%sPASSED: %s %s", X,
					ss.str().c_str(),
					label.c_str()
					);
		}
		else
		{
			log_msg("%sFAILED: %s %s", X,
					ss.str().c_str(),
					label.c_str()
					);
		}
	}
}

void
MovieTester::pressMouseButton()
{
	_movie_root->notify_mouse_clicked(true, 1);
}

void
MovieTester::depressMouseButton()
{
	_movie_root->notify_mouse_clicked(false, 1);
}

void
MovieTester::pressKey(key::code code)
{
	_movie_root->notify_key_event(code, true);
}

void
MovieTester::releaseKey(key::code code)
{
	_movie_root->notify_key_event(code, false);
}

bool
MovieTester::isMouseOverMouseEntity()
{
	return _movie_root->isMouseOverActiveEntity();
}

geometry::Range2d<int>
MovieTester::getInvalidatedBounds() const
{
	using namespace gnash::geometry;

	rect ret;
	assert(ret.is_null());
	
	// TODO: Support multiple bounds in testsuite
	//_movie_root->get_invalidated_bounds(&ret, false);
	InvalidatedRanges ranges;
	_movie_root->add_invalidated_bounds(ranges, false);

	Range2d<float> range = ranges.getFullArea();

	// scale by 1/20 (twips to pixels)
	range.scale(1.0/20);

	// Convert to integer range.
	Range2d<int> pixrange(range);

	return pixrange;
	
}

int
MovieTester::soundsStarted()
{
	return _sound_handler.get()->test_times_started_all();
}

int
MovieTester::soundsStopped()
{
	return _sound_handler.get()->test_times_stopped_all();
}

void
MovieTester::initTestingRenderers()
{
	std::auto_ptr<render_handler> handler;

	// TODO: add support for testing multiple renderers
	// This is tricky as requires changes in the core lib

#ifdef RENDERER_AGG
	// Initialize AGG
	handler.reset( create_render_handler_agg("RGB24") );
	assert(handler.get());
	addTestingRenderer(handler, "AGG_RGB24");
#endif

#ifdef RENDERER_CAIRO
	// Initialize Cairo
#endif

#ifdef RENDERER_OPENGL
	// Initialize opengl renderer
#endif
}

void
MovieTester::addTestingRenderer(std::auto_ptr<render_handler> h, const std::string& name)
{
	if ( ! h->initTestBuffer(_width, _height) )
	{
		cout << "UNTESTED: render handler " << name
			<< " doesn't support in-memory rendering "
			<< endl;
		return;
	}

	// TODO: make the core lib support this
	if ( ! _testingRenderers.empty() )
	{
		cout << "UNTESTED: can't test render handler " << name
			<< " because gnash core lib is unable to support testing of "
			<< "multiple renderers from a single process "
			<< "and we're already testing render handler "
			<< _testingRenderers.front()->getName()
			<< endl;
		return;
	}

	_testingRenderers.push_back(TestingRendererPtr(new TestingRenderer(h, name)));
}

} // namespace gnash
