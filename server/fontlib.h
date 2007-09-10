// fontlib.h - Internal interfaces to fontlib, for Gnash.
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


#ifndef GNASH_FONTLIB_H
#define GNASH_FONTLIB_H


#include "types.h"

// Forward declarations
class tu_file;
namespace gnash {
	class movie_def_impl;
	class texture_glyph;
}

namespace gnash {

/// Library management
//
/// Font library control.  gnash is able to substitute fonts
/// from the font library, in case a movie lacks glyphs for a
/// declared font.  This would come into play since in recent
/// versions of SWF, the movie is allowed to use "system
/// fonts".  E.g. it can declare a font named "Arial", but not
/// provide glyphs for it, and then the OS is expected to
/// provide the font or a suitable replacement.
///
/// gnash does not try to handle this automatically; if your
/// host program wants to emulate this behavior, it needs to
/// load a movie that includes glyph info for the standard
/// fonts you want, and then explicitly pull those fonts out of
/// the movie_def and add them to fontlib.
///
/// @@ TODO: not all public APIs to enable this are in place
/// yet!  Need md::get_font_count()/get_font(), and
/// fontlib::add_font().
///
/// Otherwise, text written in a font with no glyphs just
/// doesn't render at all.  (@@ Hm, should probably render it
/// as boxes or something?)
///
namespace fontlib {

	// For adding fonts.
	void	add_font(font* f);

	// For drawing a textured glyph w/ current render transforms.
	void	draw_glyph(const matrix& m, const texture_glyph& g, const rgba& color, int nominal_glyph_height);

	// Return the pixel height of text, such that the
	// texture glyphs are sampled 1-to-1 texels-to-pixels.
	// I.e. the height of the glyph box, in texels.
	float	get_texture_glyph_max_height(const font* f);

	// Builds cached glyph textures from shape info.
	void	generate_font_bitmaps(const std::vector<font*>& fonts, movie_definition* owner);
	
	// Save cached font data, including glyph textures, to a
	// stream.
	void	output_cached_data(
		tu_file* out,
		const std::vector<font*>& fonts,
		movie_definition* owner,
		const cache_options& options);
	
	// Load a stream containing previously-saved cachded font
	// data, including glyph texture info.
	void	input_cached_data(tu_file* in, const std::vector<font*>& fonts, movie_definition* owner);

	// Controls how large to render textured glyphs.
	// Applies to fonts processed *after* this call only.
	// The "nominal" size is perhaps around twice the
	// average glyph height.
	void	set_nominal_glyph_pixel_size(size_t pixel_size);

	/// Clean up the font library
	void	clear();

	int	get_font_count();

	font*	get_font(int index);

	font*	get_font(const char* name);

	/// Return a default device font.
	boost::intrusive_ptr<font> get_default_font();

	const char*	get_font_name(const font* f);

	// @@ also need to add color controls (or just set the diffuse color
	// in the API?), perhaps matrix xform, and maybe spacing, etc.
	//
	// // For direct text rendering from the host app.
	void	draw_string(const font* f, float x, float y, float size, const char* text);

	
}	// end namespace fontlib
}	// end namespace gnash



#endif // GNASH_FONTLIB_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
