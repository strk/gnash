// freetype.h	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_FREETYPE_H
#define GNASH_FREETYPE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#ifdef HAVE_FREETYPE_FREETYPE_H
# define HAVE_FREETYPE2 1
#endif

#include "rect.h"
#include "smart_ptr.h" // for intrusive_ptr

#include <string>
#include <memory> // for auto_ptr

#ifdef HAVE_FREETYPE2 
# include <ft2build.h>
# include FT_FREETYPE_H
# include FT_GLYPH_H
#endif

// Forward declarations
namespace gnash {
	class bitmap_info;
}
namespace image {
	class alpha;
}


namespace gnash {

/// Truetype font rasterizer based on freetype library
//
/// Instances of this class provide rasterized glyphs for a given
/// truetype font face.
///
/// The rasterized glyphs have a max size of 96 (TODO: make parametrizable)
/// but I think the actual size could change between glyphs (see the 'box'
/// parameter of getRenderedGlyph() method).
///
class FreetypeRasterizer 
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
	static std::auto_ptr<FreetypeRasterizer> createFace(const std::string& name, bool bold, bool italic);

	/// Return the given character glyph as a bitmap
	//
	/// @param code
	/// 	Character code.
	///
	/// @param box
	///	Output parameter... TODO: describe what it is.
	///
	/// @param advance
	///	Output parameter... TODO: describe what it is.
	///
	/// @return A bitmap_info, or a NULL pointer if the given character code
	///	    doesn't exist in this font.
	///
	boost::intrusive_ptr<bitmap_info> getRenderedGlyph(uint16_t code, rect& box, float& advance);

private:

	/// Use the named constructor to create an instance
	//
	/// throw a GnashException on error (unkonwn font name or similar).
	///
	FreetypeRasterizer(const std::string& fontname, bool bold, bool italic);

#ifdef HAVE_FREETYPE2 

	// TODO: drop ?
	//float get_advance_x(uint16_t code);

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

#endif // HAVE_FREETYPE2

};

} // namespace gnash


#endif	// GNASH_FREETYPE_H
