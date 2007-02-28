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
MovieTester::advance() 
{
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
	_movie_root->notify_mouse_moved(x, y);
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
	notify_key_event(code, true);
}

void
MovieTester::releaseKey(key::code code)
{
	notify_key_event(code, false);
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

} // namespace gnash
