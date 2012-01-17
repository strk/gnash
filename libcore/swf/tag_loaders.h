// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#ifndef GNASH_SWF_TAG_LOADERS_H
#define GNASH_SWF_TAG_LOADERS_H

#include "SWF.h" 

// Forward declarations
namespace gnash {
    class movie_definition;
    class RunResources;
    class SWFStream;
}

namespace gnash {
namespace SWF {

/// SWF Tags Reflex (777)
//
void reflex_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Create and initialize a sprite, and add it to the movie. 
//
/// Handles a SWF::DEFINESPRITE tag
///
void sprite_loader(SWFStream&, TagType, movie_definition&, const RunResources&);

/// Label the current frame  (SWF::FRAMELABEL)
void frame_label_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Load a SWF::DEFINESOUND tag.
void define_sound_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

void
file_attributes_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r);

void
metadata_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r);

/// Load a SWF::SERIALNUMBER tag.
void
serialnumber_loader(SWFStream& in, TagType tag, movie_definition& /*m*/,
        const RunResources& /*r*/);


} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_TAG_LOADERS_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
