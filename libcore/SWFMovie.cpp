// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "SWFMovie.h"
#include "movie_definition.h"
#include "movie_root.h"
#include "log.h"

#include <vector>
#include <string>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each, std::min
#include <utility>
#include <map>

namespace gnash {

SWFMovie::SWFMovie(as_object* object, const SWFMovieDefinition* def,
        DisplayObject* parent)
	:
	Movie(object, def, parent),
	_def(def)
{
    assert(object);
}

void
SWFMovie::construct(as_object* /*init*/)
{

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
    MovieClip::construct();  
}

// Advance of an SWF-defined movie instance
void
SWFMovie::advance()
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

    MovieClip::advance(); 
}
    
SWF::DefinitionTag*
SWFMovie::exportedCharacter(const std::string& symbol)
{
    const boost::uint16_t id = _def->exportID(symbol);
    if (!id) return 0;
    Characters::iterator it = _characters.find(id);
    if (it == _characters.end()) return 0;
    return _def->getDefinitionTag(id);
}

void
SWFMovie::addCharacter(boost::uint16_t id)
{
    // If a character is already known, we don't want to mark it uninitialized
    // again.
    _characters.insert(std::make_pair(id, false));
}

bool
SWFMovie::initializeCharacter(boost::uint16_t cid)
{
    Characters::iterator it = _characters.find(cid);
    if (it == _characters.end()) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Attempt to perform initialized for a character %s "
                "that does not exist (either not exported or not defined)",
                cid);
        );
        return false;
    }
    if (it->second) return false;
    it->second = true;
    return true;
}

} // namespace gnash
