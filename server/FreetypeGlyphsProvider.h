// FreetypeGlyphsProvider.h:  Freetype glyphs manager
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_FREETYPE_H
#define GNASH_FREETYPE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "rect.h"
#include "smart_ptr.h" // for intrusive_ptr

#include <string>
#include <memory> // for auto_ptr

#ifdef USE_FREETYPE 
# include <ft2build.h>
# include FT_FREETYPE_H
# include FT_GLYPH_H
#endif

// Forward declarations
namespace gnash {
	class bitmap_info;
	class shape_character_def;
}
namespace image {
	class alpha;
}


namespace gnash {

/// Truetype font rasterizer/converter based on freetype library
//
/// Instances of this class provide rasterized or vectorial glyphs
/// for a given truetype font face.
///
/// The rasterized glyphs have a max size of 96 (TODO: make parametrizable)
/// but I think the actual size could change between glyphs (see the 'box'
/// parameter of getRenderedGlyph() method).
///
/// Vectorial glyphs are instances of a shape_character_def, same class
/// resulting from parsing of embedded fonts.
///
/// TODO: rename this class to something like FreetypeGlyphProvider...
///
class FreetypeGlyphsProvider 
{

public:

	/// Named constructor for a face-bound rasterizer.
	//
	/// @param name
	///	Name of the font to get glyphs info from
	///
	/// @param bold
	///	Whether to use a bold version of the font
	///
	/// @param italic
	///	Whether to use an italic version of the font
	///
	/// @return a rasterizer bound to the given font name,
	///         or a NULL auto_ptr if the given truetype font
	///         could not be found.
	///
	static std::auto_ptr<FreetypeGlyphsProvider> createFace(const std::string& name, bool bold, bool italic);

	/// Destructor
	//
	/// Release face resources
	///
	~FreetypeGlyphsProvider();


	/// Return the given character glyph as a shape character definition in 1024 EM coordinates.
	//
	///
	/// TODO: allow using a custom EM square ?
	///
	/// @param code
	/// 	Character code.
	///
	/// @param advance
	///	Output parameter... units to advance horizontally from this glyph to the next,
	///	in 1024 EM units.
	///
	/// @return A shape_character_def in 1024 EM coordinates, or a NULL pointer if the given
	///         character code doesn't exist in this font.
	///
	boost::intrusive_ptr<shape_character_def> getGlyph(boost::uint16_t code, float& advance);


private:

	/// Use the named constructor to create an instance
	//
	/// throw a GnashException on error (unkonwn font name or similar).
	///
	FreetypeGlyphsProvider(const std::string& fontname, bool bold, bool italic);

#ifdef USE_FREETYPE 

	/// Scale factor to make the freetype glyph metrix match our 1024 EM square
	/// coordinate space. Not all font faces have am EM square of 1024, so we
	/// use this value to scale both coordinates and advance values
	/// The value is computed by the costructor, as soon as a face is initialized.
	float scale;

	/// Get filename containing given font
	//
	/// @param name
	///	Font name
	///
	/// @param bold
	///	Want bold version
	///
	/// @param italic
	///	Want italic version
	///
	/// @param filename
	///	Where to return the filename to
	///
	/// @return true if the font was found, false otherwise.
	///	Actually, this function should return a default
	///	filename in any case, so false should only be
	///	returned if not even a default font was found.
	///
	bool getFontFilename(const std::string& name, bool bold, bool italic,
			std::string& filename);

	static void init();

	static void close();

	/// Used by getRenderedGlyph to get the glyph bitmap.
	//
	/// NOTE: calls the currently registered renderer (create_alpha)
	///
	std::auto_ptr<image::alpha> draw_bitmap(const FT_Bitmap& bitmap);


	static FT_Library	m_lib;
	FT_Face	m_face;

#endif // USE_FREETYPE

};

} // namespace gnash


#endif	// GNASH_FREETYPE_H
