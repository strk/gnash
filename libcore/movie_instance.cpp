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

#include "movie_instance.h"
#include "movie_definition.h"
#include "movie_root.h"
#include "log.h"

#include <vector>
#include <string>
#include <cmath>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each, std::min

namespace gnash {

movie_instance::movie_instance(movie_definition* def, DisplayObject* parent)
	:
	MovieClip(def, this, parent, parent ? 0 : -1),
	_def(def)
{
}

void
movie_instance::stagePlacementCallback(as_object* initObj)
{

    assert (!initObj);

	saveOriginalTarget();

	// Load first frame  (1-based index)
	size_t nextframe = 1;
	if ( !_def->ensure_frame_loaded(nextframe) )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("Frame %d never loaded. Total frames: %d",
		                nextframe, get_frame_count());
		);
	}

	// Invoke parent placement event handler
	MovieClip::stagePlacementCallback();  
}

// Advance of an SWF-defined movie instance
void
movie_instance::advance()
{
	// Load next frame if available (+2 as m_current_frame is 0-based)
	//
	// We do this inside advance_root to make sure
	// it's only for a root sprite (not a sprite defined
	// by DefineSprite!)
	size_t nextframe = std::min<size_t>(get_current_frame() + 2,
            get_frame_count());
	if ( !_def->ensure_frame_loaded(nextframe) )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("Frame %d never loaded. Total frames: %d.",
		            nextframe, get_frame_count());
		);
	}

	advance_sprite(); 
}

} // namespace gnash
