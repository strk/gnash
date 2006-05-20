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
//

#ifndef GNASH_SWF_TAG_LOADERS_H
#define GNASH_SWF_TAG_LOADERS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "swf.h" // for SWF::tag_type

// Forward declarations
namespace gnash {
	struct movie_definition;
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

void	define_font_loader(stream*, tag_type, movie_definition*);

/// SWF Tag DefineFontInfo (13) 
//
/// Load a DefineFontInfo tag.  This adds information to an
/// existing font.
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

/// Load an import tag (for pulling in external resources)
void	import_loader(stream*, tag_type, movie_definition*);

void	define_sound_loader(stream*, tag_type, movie_definition*);

/// Load a SWF::STARTSOUND tag.
void	start_sound_loader(stream*, tag_type, movie_definition*);

void	button_sound_loader(stream*, tag_type, movie_definition*);

void	do_init_action_loader(stream*, tag_type, movie_definition*);

// sound_stream_loader();	// head, head2, block


} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_TAG_LOADERS_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
