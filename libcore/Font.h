// Font.h -- Font class, for Gnash
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
//

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003


#ifndef GNASH_FONT_H
#define GNASH_FONT_H

#include <string>
#include <memory>
#include <cstdint>
#include <memory>
#include <vector>
#include <map>

#include "ref_counted.h"

namespace gnash {
    class FreetypeGlyphsProvider;
    namespace SWF {
        class ShapeRecord;
        class DefineFontTag;
    }
}

namespace gnash {



// @@ replace this with a flat hash, or else a sorted array
//    (binary search)
class kerning_pair
{
public:
    std::uint16_t m_char0;
    std::uint16_t m_char1;

    bool operator==(const kerning_pair& k) const
    {
        return m_char0 == k.m_char0 && m_char1 == k.m_char1;
    }
};

// for use in standard algorithms
inline bool
operator< (const kerning_pair& p1, const kerning_pair& p2)
{
    if (p1.m_char0 < p2.m_char0) return true;
    if (p1.m_char0 == p2.m_char0) {
        if (p1.m_char1 < p2.m_char1) return true;
    }
    
    return false;
}


/// A Font resource.
//
/// All fonts used in the course of rendering a SWF are represented by this
/// class. There are two types of Font object: device fonts and glyph fonts
/// (also called embedded fonts). Device fonts contain no embedded glyphs,
/// but glyph fonts may be rendered using device fonts if requested during
/// runtime.
//
/// The fact that one Font object may represent an embedded and a device
/// font simultaneously means that callers must themselves ensure they
/// specify which font they require. Failure to do this consistently may mean
/// callers end up with the wrong information about a font.
//
/// TODO: check whether it really needs to be ref_counted.
class Font : public ref_counted
{
public:

    // This table maps from Unicode DisplayObject number to glyph index.
    typedef std::map<std::uint16_t, int> CodeTable;

    Font(std::unique_ptr<SWF::DefineFontTag> ft);

    /// Create a device-font only font, using the given name to find it
    //
    /// @param name
    ///    Name of the font face to look for.
    ///
    /// @param bold
    ///    Whether to use the bold variant of the font.
    ///
    /// @param italic
    ///    Whether to use the italic variant of the font.
    Font(std::string name, bool bold = false, bool italic = false);

    ~Font();

    std::uint16_t codeTableLookup(int glyph, bool embedded) const;

    /// Return true if this font matches given name and flags
    //
    /// @param name
    ///    Font name
    ///
    /// @param bold
    ///    Bold flag
    ///
    /// @param italic
    ///    Italic flag
    bool matches(const std::string& name, bool bold, bool italic) const;

    /// Get glyph by index.
    //
    /// @param glyph_index
    ///   Index of the glyph. See get_glyph_index() to obtain by character code.
    ///
    /// @param embedded
    ///    If true, queries the 'embedded' glyphs table, 
    ///    otherwise, looks in the 'device' font table.
    ///
    /// @return
    ///    The glyph outline, or NULL if out of range. (would be a
    /// programming error most likely). The ShapeRecord is owned by
    /// the Font class.
    SWF::ShapeRecord* get_glyph(int glyph_index, bool embedded) const;

    /// Get name of this font. 
    const std::string& name() const { return _name; }

    /// Return the glyph index for a given character code
    //
    /// @param code
    ///    Character code to fetch the corresponding glyph index of.
    ///
    /// @param embedded
    ///    If true, queries the 'embedded' glyphs table, 
    ///    otherwise, looks in the 'device' font table.
    ///
    /// Note, when querying device fonts, glyphs are created on demand,
    /// this never happens for embedded fonts, in which case an unexistent
    /// glyph results in a return of -1
    ///
    /// @return -1 if there is no glyph for the specified code or a valid
    ///         positive index to use in subsequent calls to other
    ///         glyph-index-based methods.
    ///
    int get_glyph_index(std::uint16_t code, bool embedded) const;

    /// Return the advance value for the given glyph index
    //
    /// Note: use unitsPerEM() to get the EM square.
    //
    /// @param glyph_index      Index of the glyph. See get_glyph_index()
    ///                         to obtain by character code.
    ///
    /// @param embedded         If true, queries the 'embedded' glyphs table, 
    ///                         otherwise, looks in the 'device' font table.
    float get_advance(int glyph_index, bool embedded) const;

    /// Return the adjustment in advance between the given two
    /// DisplayObjects (makes sense for embedded glyphs only)
    //
    /// Normally this will be 0
    ///
    /// NOTE: don't call this method when willing to work with device
    ///       fonts, or you'll end up mixing information from device fonts
    ///      with information from embedded fonts.
    ///
    float get_kerning_adjustment(int last_code, int this_code) const;

    /// Return height of the EM square used for glyphs definition
    //
    /// @param embedded     If true, return is based on the SWF tag the font
    ///                     was read from, otherwise will query the
    ///                     FreeTypeGlyphsProvider
    size_t unitsPerEM(bool embedded) const;

    /// Return the ascent value of the font.
    //
    /// Note: use unitsPerEM() to get the EM square.
    float ascent(bool embedded) const;
        
    /// Return the descent value of the font in EM units.
    //
    /// Note: use unitsPerEM() to get the EM square.
    float descent(bool embedded) const;

    /// Return the leading value of the font.
    //
    /// Note: use unitsPerEM() to get the EM square.
    float leading() const;

    /// Return true if the font is bold.
    bool isBold() const {
        return _bold;
    }
    
    /// Return true if the font is italic.
    bool isItalic() const {
        return _italic;
    }

    /// A pair of strings describing the font.
    //
    /// Used by the DefineFontName tag, usefulness unclear.
    struct FontNameInfo
    {
        std::string displayName;
        std::string copyrightName;
    };

    /// Glyph info structure
    struct GlyphInfo
    {
        // no glyph, default textured glyph, 0 advance
        GlyphInfo();

        /// Construct default textured glyph
        //
        /// Takes ownership of the SWF::ShapeRecord.
        GlyphInfo(std::unique_ptr<SWF::ShapeRecord> glyph, float advance);

        GlyphInfo(const GlyphInfo& o);

        std::shared_ptr<SWF::ShapeRecord> glyph;

        float advance;
    };

    typedef std::vector<GlyphInfo> GlyphInfoRecords;

    /// Add display name and copyright name for an embedded font.
    //
    /// It's a string copy, but a decent standard library implementation
    /// should be able to avoid actually copying. Since it's only two
    /// strings, it doesn't seem worth the effort to avoid the copy.
    void addFontNameInfo(const FontNameInfo& fontName);

    /// Set the name of the font.
    //
    /// This is used by SWF::DefineFontInfoTag
    void setName(const std::string& name);

    /// Set the language and encoding flags of the font.
    //
    /// This is used by SWF::DefineFontInfoTag
    void setFlags(std::uint8_t flags);

    /// Add a CodeTable to the font.
    //
    /// This is used by SWF::DefineFontInfoTag
    void setCodeTable(std::unique_ptr<CodeTable> table);

    /// Retrieve the number of embedded glyphs in this font.
    GlyphInfoRecords::size_type glyphCount() const;

    /// Retrieve the FreetypeGlyphsProvider, initializing it if necessary.
    //
    /// Always use this method rather than directly accessing the _ftProvider
    /// member to ensure that the provider is initialized. May return null.
    FreetypeGlyphsProvider* ftProvider() const;

private:

    /// Add a glyph from the os font into the device glyphs table
    //
    /// It is assumed that the glyph tables do NOT contain
    /// an entry for the given code.
    /// Initializes the rasterizer if not already done so.
    ///
    /// @return index of the newly added glyph, or -1 on error.
    ///
    int add_os_glyph(std::uint16_t code);

    /// If we were constructed from a definition, this is not NULL.
    std::unique_ptr<SWF::DefineFontTag> _fontTag;

    // Device glyphs
    GlyphInfoRecords _deviceGlyphTable;

    std::string    _name;
    std::string _displayName;
    std::string _copyrightName;

    bool _unicodeChars;
    bool _shiftJISChars;
    bool _ansiChars;
    bool _italic;
    bool _bold;

    /// Code to index table for embedded glyphs
    //
    /// This can be NULL if an embedded font should not be
    /// substituted by a device font. This can arise with
    /// a) a DefineFont tag without a corresponding DefineFontInfo
    ///    or DefineFontInfo2, or
    /// b) a DefineFont2 or DefineFont3 tag with no CodeTable.
    ///
    /// It is a shared_ptr to avoid changing an original
    /// DefineFont2Tag, while allowing this class to take ownership
    /// of CodeTables from a DefineFontInfo tag.
    std::shared_ptr<const CodeTable> _embeddedCodeTable;

    /// Code to index table for device glyphs
    CodeTable _deviceCodeTable; 

    typedef std::map<kerning_pair, float> kernings_table;
    kernings_table m_kerning_pairs;

    mutable std::unique_ptr<FreetypeGlyphsProvider> _ftProvider;

};


}    // end namespace gnash



#endif // GNASH_FONT_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
