// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#include "SWFStream.h"

#include <cassert>

// Forward declarations
namespace gnash {
    class movie_definition;
    class RunResources;
}

namespace gnash {
namespace SWF {

/// Silently ignore the contents of this tag.
void null_loader(SWFStream&, TagType, movie_definition&, const RunResources&);

/// This is like null_loader except it prints a message to nag us to fix it.
void fixme_loader(SWFStream&, TagType, movie_definition&, const RunResources&);

/// \brief
/// Load JPEG compression tables that can be used to load
/// images further along in the SWFStream. (SWF::JPEGTABLES)
void jpeg_tables_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// \brief
/// A JPEG image without included tables; those should be in an
/// existing jpeg::input object stored in the movie. (SWF::DEFINEBITS)
void define_bits_jpeg_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Handler for SWF::DEFINEBITSJPEG2 tag
void define_bits_jpeg2_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// \brief
/// Loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
/// channel using zlib compression. (SWF::DEFINEBITSJPEG3)
void define_bits_jpeg3_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// SWF Tags Reflex (777)
//
void reflex_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

void define_bits_lossless_2_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Create and initialize a sprite, and add it to the movie. 
//
/// Handles a SWF::DEFINESPRITE tag
///
void sprite_loader(SWFStream&, TagType, movie_definition&, const RunResources&);

// end_tag doesn't actually need to exist.
// TODO: drop this loader ?
inline void end_loader(SWFStream& in, TagType tag, movie_definition&,
        const RunResources&)
{
    assert(tag == SWF::END); // 0
    assert(in.tell() == in.get_tag_end_position());
}

void remove_object_2_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

void do_action_loader(SWFStream&, TagType, movie_definition&, const RunResources&);

/// Label the current frame  (SWF::FRAMELABEL)
void frame_label_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Load a SWF::DEFINESOUND tag.
void define_sound_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

void do_init_action_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Load SWF::SOUNDSTREAMHEAD or SWF::SOUNDSTREAMHEAD2 tag.
void sound_stream_head_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

/// Load a SWF::SOUNDSTREAMBLOCK tag.
void sound_stream_block_loader(SWFStream&, TagType, movie_definition&,
		const RunResources&);

void
define_video_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r);

void
video_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r);

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
