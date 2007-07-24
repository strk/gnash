// font.h -- font class, for Gnash
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

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003


#ifndef GNASH_FONT_H
#define GNASH_FONT_H


#include "container.h"
#include "rect.h" // for composition of class texture_glyph 
#include "types.h"
#include "resource.h" // for inheritance of font
#include "ref_counted.h" // for inheritance of texture_glyph
#include "swf.h" // for tag_type definition
#include "bitmap_info.h" // for dtor visibility by smart pointer
#include "FreetypeGlyphsProvider.h" // for device fonts support
#include "log.h"
#ifdef GNASH_USE_GC
# include "GC.h"
#endif

#include <map>

// Forward declarations
class tu_file;

namespace gnash {

class movie;
class shape_character_def;
class stream;


/// class for holding (cached) textured glyph info.
class texture_glyph // : public ref_counted
{

public:

	texture_glyph() : m_bitmap_info(NULL) {}

	~texture_glyph()
	{
	}

	/// Return true if this can be used for rendering.
	bool	is_renderable() const
	{
		return m_bitmap_info != NULL;
	}

	/// Argument will be assigned to a boost::intrusive_ptr
	void	set_bitmap_info(bitmap_info* bi)
	{
		m_bitmap_info = bi;
	}

#ifdef GNASH_USE_GC
	/// Mark the contained bitmap info as being reachable
	void markReachableResources() const
	{
		if ( m_bitmap_info.get() ) m_bitmap_info->setReachable();
	}
#endif


// too early to make these private, fontlib directly accesses
// them, postponed.
//private:

	boost::intrusive_ptr<bitmap_info>	m_bitmap_info;

	rect	m_uv_bounds;

	// the origin of the glyph box, in uv coords
	point	m_uv_origin;

};

// @@ replace this with a flat hash, or else a sorted array
//    (binary search)
class kerning_pair
{
public:
	uint16_t	m_char0, m_char1;

	bool	operator==(const kerning_pair& k) const
	{
		return m_char0 == k.m_char0 && m_char1 == k.m_char1;
	}


};

// for use in standard algorithms
inline bool operator < (const kerning_pair& p1, const kerning_pair& p2)
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

	texture_glyph textureGlyph;

	float advance;
};

/// \brief
/// A 'font' definition as read from SWF::DefineFont,
/// SWF::DefineFont2 or SWF::DefineFont3 tags.
/// Includes definitions from SWF::DefineFontInfo tags
///
class font : public resource
{
public:
	font();

	// Create a device-font only font, using the given name to find it
	font(const std::string& name);

	~font();

	// override from resource.
	font*	cast_to_font() { return this; }

	void testInvariant()
	{
#if 0
		assert(m_texture_glyphs.size() == m_glyphs.size());
#ifndef NDEBUG
		if (m_texture_glyphs.size() != m_advance_table.size())
		{
			log_error("Font '%s': Number of texture glyphs: %lu, advance records: %lu",
					m_name.c_str(),
					static_cast<unsigned long>(m_texture_glyphs.size()),
					static_cast<unsigned long>(m_advance_table.size()));
			abort();
		}
#endif
#endif
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
	///          (or "owning" this font)
	///
	void	read(stream* in, SWF::tag_type tag, movie_definition* m);

	/// \brief
	/// Read additional information about this font, from a
	/// DefineFontInfo or DefineFontInfo2 tag. 
	//
	/// The caller has already read the tag type and font id.
	///
	/// @see SWF::define_font_info_loader
	///
	void	read_font_info(stream* in, SWF::tag_type tag, movie_definition* m);

	/// Dump our cached data into the given stream.
	void	output_cached_data(tu_file* out, const cache_options& options);

	/// Read our cached data from the given stream.
	void	input_cached_data(tu_file* in);

	/// Delete all our texture glyph info (both embedded and device)
	void	wipe_texture_glyphs();

	/// Get name of this font. Warning: can be NULL.
	const std::string& get_name() const { return m_name; }

	/// Return the movie_definition "owning" this font
	movie_definition* get_owning_movie() const { return m_owning_movie; }

	/// \brief
	/// Return a pointer to a texture_glyph class
	/// corresponding to the given glyph_index, if we
	/// have one.  Otherwise return a "dummy" texture_glyph.
	//
	/// @param glyph_index
	///	Index of the glyph. See get_glyph_index() to obtain by character code.
	///
	/// @param embedded
	///	If true, queries the 'embedded' glyphs table, 
	///	otherwise, looks in the 'device' font table.
	///
	/// Note: the "dummy" texture_glyph is a default-constructed
	/// texture_glyph.
	///
	const texture_glyph& get_texture_glyph(int glyph_index, bool embedded) const;

	/// \brief
	/// Register some texture info for the glyph at the specified
	/// index.  The texture_glyph can be used later to render the
	/// glyph.
	//
	/// @param glyph_index
	///	Index of the glyph. See get_glyph_index() to obtain by character code.
	///
	/// @param glyph
	///	The textured glyph.
	///
	/// @param embedded
	///	If true, queries the 'embedded' glyphs table, 
	///	otherwise, looks in the 'device' font table.
	///
	/// TODO: deprecate this, probably only used by the caching mechanism
	///
	void	add_texture_glyph(int glyph_index, const texture_glyph& glyph, bool embedded);

	void	set_texture_glyph_nominal_size(int size) { m_texture_glyph_nominal_size = imax(1, size); }
	int	get_texture_glyph_nominal_size() const { return m_texture_glyph_nominal_size; }

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
	int	get_glyph_index(uint16_t code, bool embedded) const;

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

	float	get_leading() const { return m_leading; }
	float	get_descent() const { return m_descent; }

private:

	/// Read the table that maps from glyph indices to character codes.
	void	read_code_table(stream* in);

	/// Read a DefineFont2 or DefineFont3 tag
	void readDefineFont2_or_3(stream* in, movie_definition* m);

	/// Read a DefineFont tag
	void readDefineFont(stream* in, movie_definition* m);

	/// Add a glyph from the os font into the device glyphs table
	//
	/// It is assumed that the glyph tables do NOT contain
	/// an entry for the given code.
	/// Initializes the rasterizer if not already done so.
	///
	/// @return index of the newly added glyph, or -1 on error.
	///
	int add_os_glyph(uint16_t code);

	/// Initialize the freetype rasterizer
	//
	/// Return true on success, false on error
	bool initDeviceFontProvider();

	typedef std::vector< GlyphInfo > GlyphInfoVect;

	// Embedded glyphs
	GlyphInfoVect _embedGlyphTable;

	// Device glyphs
	GlyphInfoVect _deviceGlyphTable;

	int	m_texture_glyph_nominal_size;

	std::string	m_name;
	movie_definition*	m_owning_movie;

	bool	m_has_layout;
	bool	m_unicode_chars;
	bool	m_shift_jis_chars;
	bool	m_ansi_chars;
	bool	m_is_italic;
	bool	m_is_bold;
	bool	m_wide_codes;

	// This table maps from Unicode character number to glyph index.
	typedef std::map<uint16_t, int> code_table;

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

	std::auto_ptr<FreetypeGlyphsProvider> _ftProvider;

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	//
	/// Reachable resources are:
	///	- texture_glyphs
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
