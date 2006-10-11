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
 * Linking Gnash statically or dynamically with other modules is making a
 * combined work based on Gnash. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Gnash give you
 * permission to combine Gnash with free software programs or libraries
 * that are released under the GNU LGPL and with code included in any
 * release of Talkback distributed by the Mozilla Foundation. You may
 * copy and distribute such a system following the terms of the GNU GPL
 * for all but the LGPL-covered parts and Talkback, and following the
 * LGPL for the LGPL-covered parts.
 *
 * Note that people who make modified versions of Gnash are not obligated
 * to grant this special exception for their modified versions; it is their
 * choice whether to do so. The GNU General Public License gives permission
 * to release a modified version without this exception; this exception
 * also makes it possible to release a modified version which carries
 * forward this exception.
 *
 */ 

#include "MovieTester.h"
#include "GnashException.h"
#include "URL.h"
#include "noseek_fd_adapter.h"
#include "movie_definition.h"
#include "movie_instance.h"
#include "movie_interface.h"
#include "sprite_instance.h"
#include "as_environment.h"
#include "gnash.h" // for create_movie and create_library_movie

#include <cstdio>
#include <string>
#include <memory> // for auto_ptr

namespace gnash {

MovieTester::MovieTester(const std::string& url)
{
	if ( url == "-" )
	{
		tu_file* in = noseek_fd_adapter::make_stream(fileno(stdin));
		_movie_def = gnash::create_movie(in, url);
	}
	else
	{
		// _url should be always set at this point...
		_movie_def = gnash::create_library_movie(URL(url));
	}

	if ( ! _movie_def )
	{
		throw GnashException("Could not load movie from "+url);
	}

	// Make sure to load the whole movie
	_movie_def->ensure_frame_loaded(_movie_def->get_frame_count());

	movie_interface* root = _movie_def->create_instance();
	assert(root);
	_movie = root->get_root_movie();
	assert(_movie);
}

void
MovieTester::advance() 
{
	_movie->advance(1.0);
}

const character*
MovieTester::findDisplayItemByName(const sprite_instance& mc,
		const std::string& name_str) 
{
	const DisplayList& dlist = mc.getDisplayList();
	const tu_string name = name_str.c_str();
	return dlist.get_character_by_name(name);
}

} // namespace gnash
