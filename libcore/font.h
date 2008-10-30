// font.h -- font class, for Gnash
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
//

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003


#ifndef GNASH_FONT_H
#define GNASH_FONT_H

#include "smart_ptr.h" // GNASH_USE_GC

#include "ExportableResource.h" 
#include "swf.h" // for tag_type definition
#include "bitmap_info.h" // for dtor visibility by smart pointer
#include "FreetypeGlyphsProvider.h" // for device fonts support
#include "log.h"
#ifdef GNASH_USE_GC
# include "GC.h"
#endif

#include <map>


namespace gnash {

class movie_definition;
class shape_character_def;
class SWFStream;


// @@ replace this with a flat hash, or else a sorted array
//    (binary search)
class kerning_pair
{
public:
	boost::uint16_t	m_char0, m_char1;

	bool	operator==(const kerning_pair& k) const
	{
		return m_char0 == k.m_char0 && m_char1 == k.m_char1;
	}


};

// for use in standard algorithms
inline bool operator< (const kerning_pair& p1, const kerning_pair& p2)
{
	if ( p1.m_char0 < p2.m_char0 )
	{
		return true;
	}
	else if ( p1.m_char0 == p2.m_char0 )
	{
		if ( p1.m_char1 < p2.m_char1 ) return true;
		else return false;
	}
	else
	{
		return false;
	}
}

/// Glyph info structure
struct GlyphInfo
{
	// no glyph, default textured glyph, 0 advance
	GlyphInfo();

	// given glyph and advance, default textured glyph
	GlyphInfo(boost::intrusive_ptr<shape_character_def> nGlyph, float nAdvance);

	GlyphInfo(const GlyphInfo&);

#ifdef GNASH_USE_GC
	/// Mark any glyph and texture glyph resources as reachable
	void markReachableResources() const;
#endif

	boost::intrusive_ptr<shape_character_def> glyph;

	float advance;
};

/// \brief
/// A 'font' definition as read from SWF::DefineFont,
/// SWF::DefineFont2 or SWF::DefineFont3 tags.
/// Includes definitions from SWF::DefineFontInfo tags
///
class font : public ExportableResource
{
public:
	font();

	/// Create a device-font only font, using the given name to find it
	//
	/// @param name
	///	Name of the font face to look for.
	///
	/// @param bold
	///	Whether to use the bold variant of the font.
	///
	/// @param italic
	///	Whether to use the italic variant of the font.
	///
	font(const std::string& name, bool bold=false, bool italic=false);

	~font();

	/// Return true if this font matches given name and flags
	//
	/// @param name
	///	Font name
	///
	/// @param bold
	///	Bold flag
	///
	/// @param italic
	///	Italic flag
	///
	bool matches(const std::string& name, bool bold, bool italic) const;

	void testInvariant()
	{
	}

	/// Get number of embedded glyphs defined for this font
	//
	/// Callers of this function are:
	///
	///	- fontlib, for writing cache data (known to be not working anyway).
	///	- edit_text_character, for validating the font (obsoleted too).
	///
	int	getEmbedGlyphCount() const
	{
		return _embedGlyphTable.size();
	}

	/// Get glyph by index.
	//
	/// @param glyph_index
	///	Index of the glyph. See get_glyph_index() to obtain by character code.
	///
	/// @param embedded
	///	If true, queries the 'embedded' glyphs table, 
	///	otherwise, looks in the 'device' font table.
	///
	/// @return
	///	The glyph outline, or NULL if out of range.
	///	(would be a programming error most likely)
	///
	///
	shape_character_def*	get_glyph(int glyph_index, bool embedded) const;

	/// Read a DefineFont or DefineFont2 tag from an SWF stream 
	//
	/// @param in is the SWF stream
	/// @param tag is the tag type either DefineFont or DefineFont2
	/// @param m is the movie_definition containing this definition
	///          (used to resolve dictionary simbols referred to by glyphs, if any)
	///
	void	read(SWFStream& in, SWF::tag_type tag, movie_definition& m);

	/// \brief
	/// Read additional information about this font, from a
	/// DefineFontInfo or DefineFontInfo2 tag. 
	//
	/// The caller has already read the tag type and font id.
	///
	/// @see SWF::define_font_info_loader
	///
	void	read_font_info(SWFStream& in, SWF::tag_type tag, movie_definition& m);

    /// \brief
    /// Read the name of this font, from a DEFINEFONTNAME tag.
    //
    /// The caller has already read the tag type and font id.
    //
    /// @see SWF::define_font_name_loader
    ///
    void read_font_name(SWFStream& in, SWF::tag_type tag, movie_definition& m);

	/// Get name of this font. Warning: can be NULL.
	const std::string& get_name() const { return m_name; }

	/// Return the glyph index for a given character code
	//
	/// @param code
	///	Character code to fetch the corresponding glyph index of.
	///
	/// @param embedded
	///	If true, queries the 'embedded' glyphs table, 
	///	otherwise, looks in the 'device' font table.
	///
	/// Note, when querying device fonts, glyphs are created on demand,
	/// this never happens for embedded fonts, in which case an unexistent
	/// glyph results in a return of -1
	///
	/// @return -1 if there is no glyph for the specified code or a valid
	///         positive index to use in subsequent calls to other glyph-index-based
	///	    methods.
	///
	int	get_glyph_index(boost::uint16_t code, bool embedded) const;

	/// Return the advance value for the given glyph index
	//
	/// @param glyph_index
	///	Index of the glyph. See get_glyph_index() to obtain by character code.
	///
	/// @param embedded
	///	If true, queries the 'embedded' glyphs table, 
	///	otherwise, looks in the 'device' font table.
	///
	float	get_advance(int glyph_index, bool embedded) const;

	/// \brief
	/// Return the adjustment in advance between the given two
	/// characters (makes sense for embedded glyphs only)
	//
	/// Normally this will be 0
	///
	/// NOTE: don't call this method when willing to work with device
	///       fonts, or you'll end up mixing information from device fonts
	///	  with information from embedded fonts.
	///
	float	get_kerning_adjustment(int last_code, int this_code) const;

	/// Return height of the EM square used for glyphs definition
	//
	/// @param embedded
	///	If true, return is based on the SWF tag the font
	///	was read from, otherwise will query the FreeTypeGlyphsProvider
	///
	unsigned short int unitsPerEM(bool embedded) const;

	float	get_leading() const { return m_leading; }
	float	get_descent() const { return m_descent; }

	bool	is_subpixel_font() const { return m_subpixel_font; }
	void	set_subpixel_font(bool isit) { m_subpixel_font = isit; }

	bool	isBold() const { return m_is_bold; }
	bool	isItalic() const { return m_is_italic; }
private:

	/// Read the table that maps from glyph indices to character codes.
	void	read_code_table(SWFStream& in);

	/// Read a DefineFont2 or DefineFont3 tag
	void readDefineFont2_or_3(SWFStream& in, movie_definition& m);

	/// Read a DefineFont tag
	void readDefineFont(SWFStream& in, movie_definition& m);

	/// Add a glyph from the os font into the device glyphs table
	//
	/// It is assumed that the glyph tables do NOT contain
	/// an entry for the given code.
	/// Initializes the rasterizer if not already done so.
	///
	/// @return index of the newly added glyph, or -1 on error.
	///
	int add_os_glyph(boost::uint16_t code);

	/// Initialize the freetype rasterizer
	//
	/// NOTE: this is 'const' for lazy initialization.
	///
	/// Return true on success, false on error
	///
	bool initDeviceFontProvider() const;

	typedef std::vector< GlyphInfo > GlyphInfoVect;

	// Embedded glyphs
	GlyphInfoVect _embedGlyphTable;

	// Device glyphs
	GlyphInfoVect _deviceGlyphTable;

	std::string	m_name;
        std::string     m_display_name;
        std::string     m_copyright_name;

	bool	m_has_layout;
	bool	m_unicode_chars;
	bool	m_shift_jis_chars;
	bool	m_ansi_chars;
	bool	m_is_italic;
	bool	m_is_bold;
	bool	m_wide_codes;
	bool	m_subpixel_font;

	// This table maps from Unicode character number to glyph index.
	typedef std::map<boost::uint16_t, int> code_table;

	/// Code to index table for embedded glyphs
	code_table _embedded_code_table; 

	/// Code to index table for device glyphs
	code_table _device_code_table; 

	// Layout stuff.
	float	m_ascent;
	float	m_descent;
	float	m_leading;
	//std::vector<float>	m_advance_table;

	typedef std::map<kerning_pair, float> kernings_table;
	kernings_table m_kerning_pairs;

	mutable std::auto_ptr<FreetypeGlyphsProvider> _ftProvider;

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	//
	/// Reachable resources are:
	///	- shape_character_defs (vector glyphs)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC
};


}	// end namespace gnash



#endif // GNASH_FONT_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
