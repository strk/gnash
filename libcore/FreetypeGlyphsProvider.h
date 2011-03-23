// FreetypeGlyphsProvider.h:  Freetype glyphs manager
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

#ifndef GNASH_FREETYPE_H
#define GNASH_FREETYPE_H

#include <string>
#include <memory> // for auto_ptr
#include <boost/thread/mutex.hpp>
#include <boost/cstdint.hpp>

#ifdef USE_FREETYPE 
# include <ft2build.h>
# include FT_FREETYPE_H
# include FT_GLYPH_H
#endif

// Forward declarations
namespace gnash {
    namespace SWF {
        class ShapeRecord;
    }
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
/// Vectorial glyphs are instances of a SWF::ShapeRecord, same class
/// resulting from parsing of embedded fonts.
///
class FreetypeGlyphsProvider 
{

public:

    /// Named constructor for a face-bound rasterizer.
    //
    /// @param name
    ///    Name of the font to get glyphs info from
    ///
    /// @param bold
    ///    Whether to use a bold version of the font
    ///
    /// @param italic
    ///    Whether to use an italic version of the font
    ///
    /// @return a rasterizer bound to the given font name,
    ///         or a NULL auto_ptr if the given truetype font
    ///         could not be found.
    ///
    static std::auto_ptr<FreetypeGlyphsProvider> createFace(
            const std::string& name, bool bold, bool italic);

    /// Destructor
    //
    /// Release face resources
    ///
    ~FreetypeGlyphsProvider();


    /// \brief
    /// Return the given DisplayObject glyph as a shape DisplayObject definition
    /// in unitsPerEM() coordinates.
    //
    ///
    /// TODO: allow using a custom EM square ?
    ///
    /// @param code
    ///     Character code.
    ///
    /// @param advance
    ///    Output parameter... units to advance horizontally from this
    ///     glyph to the next, in unitsPerEM() units.
    ///
    /// @return A DefineShapeTag in unitsPerEM() coordinates,
    ///         or a NULL pointer if the given DisplayObject code
    ///         doesn't exist in this font.
    ///
    std::auto_ptr<SWF::ShapeRecord> getGlyph(boost::uint16_t code,
            float& advance);

    /// Return the font's ascender in terms of its EM own square.
    float ascent() const;
    
    /// Return the font's descender in terms of its own EM square.
    float descent() const;

    /// Return the number of units of glyphs EM
    //
    /// This is currently hard-coded to 1024, but could in future depend
    /// on actual font file being used.
    ///
    unsigned short unitsPerEM() const;

private:

    /// Use the named constructor to create an instance
    //
    /// throw a GnashException on error (unkonwn font name or similar).
    ///
    FreetypeGlyphsProvider(const std::string& fontname, bool bold, bool italic);

#ifdef USE_FREETYPE 

    /// Scale factor to make the freetype glyph metrix match our unitsPerEM()
    /// coordinate space. Not all font faces have am EM square of unitsPerEM(), so we
    /// use this value to scale both coordinates and advance values
    /// The value is computed by the costructor, as soon as a face is initialized.
    float scale;

    /// Get filename containing given font
    //
    /// @param name
    ///    Font name
    ///
    /// @param bold
    ///    Want bold version
    ///
    /// @param italic
    ///    Want italic version
    ///
    /// @param filename
    ///    Where to return the filename to
    ///
    /// @return true if the font was found, false otherwise.
    ///    Actually, this function should return a default
    ///    filename in any case, so false should only be
    ///    returned if not even a default font was found.
    ///
    bool getFontFilename(const std::string& name, bool bold, bool italic,
            std::string& filename);

    /// Initialize the FreeType library if not done so yet
    static void init();

    static void close();

    /// Mutex protecting FreeType library (for initialization basically)
    static boost::mutex    m_lib_mutex;

    /// FreeType library
    static FT_Library    m_lib;

    FT_Face    _face;

#endif // USE_FREETYPE

};

} // namespace gnash


#endif    // GNASH_FREETYPE_H
