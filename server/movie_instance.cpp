// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "movie_instance.h"
#include "movie_definition.h"
#include "movie_root.h"

#include <vector>
#include <string>
#include <cmath>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each

using namespace std;

namespace gnash {

movie_instance::movie_instance(movie_definition* def, character* parent)
	:
	sprite_instance(def, this, parent, -1),
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
	size_t nextframe = min(get_current_frame()+2, get_frame_count());
	if ( !_def->ensure_frame_loaded(nextframe) )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("Frame " SIZET_FMT " never loaded. Total frames: "
				SIZET_FMT ".", nextframe, get_frame_count());
		);
	}

	// The parser might have reset the total frame count
	// due to SWF malformation, so we check this *after*
	// the ensure_frame_loaded call above.
	//
	if ( get_frame_count() == 0 )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("The movie contains NO frames!");
		);
		return;
	}

	if (m_on_event_load_called == false)
	{
		construct();
	}

	advance_sprite(delta_time);

	if (m_on_event_load_called == false)
	{
		on_event(event_id::LOAD);	// root onload
		m_on_event_load_called = true;
	}

	//_def->resumeLoader();
}

} // namespace gnash
