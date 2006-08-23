// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

//#include "action.h" 
//#include "gnash.h"
#include "movie_instance.h"
#include "movie_definition.h"
#include "movie_def_impl.h"
#include "movie_root.h"
#include "tu_random.h"
#include "log.h" 

#include <vector>
#include <string>
#include <math.h>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each

using namespace std;

namespace gnash {

movie_instance::movie_instance(movie_def_impl* def,
		movie_root* r, character* parent)
	:
	sprite_instance(def, r, parent, -1),
	_def(def)
{
}

// Advance of an SWF-defined movie instance
void
movie_instance::advance(float delta_time)
{
	//GNASH_REPORT_FUNCTION;

	assert ( get_root()->get_root_movie() == this );

	//_def->stopLoader();

	// Load next frame if available (+2 as m_current_frame is 0-based)
	//
	// We do this inside advance_root to make sure
	// it's only for a root sprite (not a sprite defined
	// by DefineSprite!)
	_def->ensure_frame_loaded(min(get_current_frame()+2,
		get_frame_count()));

	// Vitaly: random should go continuously that:
	// 1. after restart of the player the situation has not repeated
	// 2. by different machines the random gave different numbers
	tu_random::next_random();

	advance_sprite(delta_time);

	if (m_on_event_load_called == false)
	{
		on_event(event_id::LOAD);	// root onload
		m_on_event_load_called = true;
	}

	//_def->resumeLoader();
}

} // namespace gnash
