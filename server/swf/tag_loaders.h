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
//

/* $Id: tag_loaders.h,v 1.20 2007/08/08 18:26:38 strk Exp $ */

#ifndef GNASH_SWF_TAG_LOADERS_H
#define GNASH_SWF_TAG_LOADERS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "swf.h" // for SWF::tag_type

// Forward declarations
namespace gnash {
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// Tag loader functions.
namespace tag_loaders {

/// Silently ignore the contents of this tag.
void	null_loader(stream*, tag_type, movie_definition*);

/// This is like null_loader except it prints a message to nag us to fix it.
void	fixme_loader(stream*, tag_type, movie_definition*);

/// Set background color (SWF::SETBACKGROUNDCOLOR)
void	set_background_color_loader(stream*, tag_type, movie_definition*);

/// \brief
/// Load JPEG compression tables that can be used to load
/// images further along in the stream. (SWF::JPEGTABLES)
void	jpeg_tables_loader(stream*, tag_type, movie_definition*);

/// \brief
/// A JPEG image without included tables; those should be in an
/// existing jpeg::input object stored in the movie. (SWF::DEFINEBITS)
void	define_bits_jpeg_loader(stream*, tag_type, movie_definition*);

/// Handler for SWF::DEFINEBITSJPEG2 tag
void	define_bits_jpeg2_loader(stream*, tag_type, movie_definition*);

/// \brief
/// Loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
/// channel using zlib compression. (SWF::DEFINEBITSJPEG3)
void	define_bits_jpeg3_loader(stream*, tag_type, movie_definition*);

void	define_shape_loader(stream*, tag_type, movie_definition*);

void	define_shape_morph_loader(stream*, tag_type, movie_definition*);

/// SWF Tags DefineFont (10), DefineFont2 (48) and DefineFont3 (75)
//
/// Load a font and adds it the the movie definition.
///
void	define_font_loader(stream*, tag_type, movie_definition*);

/// SWF Tags Reflex (777)
//
void	reflex_loader(stream*, tag_type, movie_definition*);

/// SWF Tag DefineFontInfo (13 or 62) 
//
/// Load a DefineFontInfo or DefineFontInfo2 tag. 
/// This adds information to an existing font.
///
void	define_font_info_loader(stream*, tag_type, movie_definition*);

/// Read SWF::DEFINETEXT and SWF::DEFINETEXT2 tags.
void	define_text_loader(stream*, tag_type, movie_definition*);

/// Read an SWF::DEFINEEDITTEXT tag.
void	define_edit_text_loader(stream*, tag_type, movie_definition*);

void	place_object_2_loader(stream*, tag_type, movie_definition*);

void	define_bits_lossless_2_loader(stream*, tag_type, movie_definition*);

/// Create and initialize a sprite, and add it to the movie. 
//
/// Handles a SWF::DEFINESPRITE tag
///
void	sprite_loader(stream*, tag_type, movie_definition*);

void	end_loader(stream*, tag_type, movie_definition*);

void	remove_object_2_loader(stream*, tag_type, movie_definition*);

void	do_action_loader(stream*, tag_type, movie_definition*);

void	button_character_loader(stream*, tag_type, movie_definition*);

/// Label the current frame  (SWF::FRAMELABEL)
void	frame_label_loader(stream*, tag_type, movie_definition*);

void	export_loader(stream*, tag_type, movie_definition*);

/// Load an SWF::IMPORTASSETS or SWF::IMPORTASSETS2 tag (for pulling in external resources)
void	import_loader(stream*, tag_type, movie_definition*);

/// Load a SWF::DEFINESOUND tag.
void	define_sound_loader(stream*, tag_type, movie_definition*);

/// Load a SWF::STARTSOUND tag.
void	start_sound_loader(stream*, tag_type, movie_definition*);

void	button_sound_loader(stream*, tag_type, movie_definition*);

void	do_init_action_loader(stream*, tag_type, movie_definition*);

/// Load SWF::SOUNDSTREAMHEAD or SWF::SOUNDSTREAMHEAD2 tag.
void	sound_stream_head_loader(stream*, tag_type, movie_definition*);

/// Load a SWF::SOUNDSTREAMBLOCK tag.
void	sound_stream_block_loader(stream*, tag_type, movie_definition*);

void
define_video_loader(stream* in, tag_type tag, movie_definition* m);

void
video_loader(stream* in, tag_type tag, movie_definition* m);

void
file_attributes_loader(stream* in, tag_type tag, movie_definition* m);

void
metadata_loader(stream* in, tag_type tag, movie_definition* m);

/// Load a SWF::SERIALNUMBER tag.
void
serialnumber_loader(stream* in, tag_type tag, movie_definition* /*m*/);

} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_TAG_LOADERS_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
