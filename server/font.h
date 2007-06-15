// font.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A font type for gnash.


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

class tu_file;

namespace gnash {
	class movie;
	class shape_character_def;
	class stream;

	
	/// class for holding (cached) textured glyph info.
	class texture_glyph : public ref_counted
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

	// too early to make these private, fontlib directly accesses
	// them, postponed.
	//private:

		boost::intrusive_ptr<bitmap_info>	m_bitmap_info;

		rect	m_uv_bounds;

		// the origin of the glyph box, in uv coords
		point	m_uv_origin;
	protected:

#ifdef GNASH_USE_GC
		/// Mark the contained bitmap info as being reachable
		void markReachableResources() const
		{
			if ( m_bitmap_info.get() ) m_bitmap_info->setReachable();
		}
#endif

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

	/// \brief
	/// A 'font' definition as read from SWF::DefineFont,
	/// SWF::DefineFont2 or SWF::DefineFont3 tags.
	/// Includes definitions from SWF::DefineFontInfo tags
	///
	class font : public resource
	{
	public:
		font();
		~font();

		// override from resource.
		font*	cast_to_font() { return this; }

		void testInvariant()
		{
			assert(m_texture_glyphs.size() == m_glyphs.size());
#ifndef NDEBUG
			if (m_texture_glyphs.size() != m_advance_table.size())
			{
				log_error("Font '%s': Number of texture glyphs: %lu, advance records: %lu",
						m_name,
						static_cast<unsigned long>(m_texture_glyphs.size()),
						static_cast<unsigned long>(m_advance_table.size()));
				abort();
			}
#endif
		}

		/// Get number of glyphs defined for this font
		//
		/// NOTE: for device fonts, this method will returns whatever
		///       number of glyphs the cache happens to have at time of
		///	  calls. Anyway, the cache will grow if any font user
		///	  requests more glyphs... 
		///	  
		/// Callers of this function are:
		///
		///	- fontlib, for writing cache data (known to be not working anyway).
		///	- edit_text_character, for validating the font (obsoleted too).
		///
		int	get_glyph_count() const
		{
			log_error("FIXME: font::get_glyph_count() is a deprecated method");
			return m_glyphs.size();
		}

		/// Get glyph by index. Return NULL if out of range
		shape_character_def*	get_glyph(int glyph_index) const;

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

		/// Delete all our texture glyph info.
		void	wipe_texture_glyphs();

		/// Get name of this font. Warning: can be NULL.
		const char*	get_name() const { return m_name; }

		/// Return the movie_definition "owning" this font
		movie_definition* get_owning_movie() const { return m_owning_movie; }

		/// \brief
		/// Return a pointer to a texture_glyph class
		/// corresponding to the given glyph_index, if we
		/// have one.  Otherwise return a "dummy" texture_glyph.
		//
		/// Note: the "dummy" texture_glyph is a default-constructed
		/// texture_glyph.
		///
		const texture_glyph&	get_texture_glyph(int glyph_index) const;

		/// \brief
		/// Register some texture info for the glyph at the specified
		/// index.  The texture_glyph can be used later to render the
		/// glyph.
		//
		/// TODO: deprecate this, probably only used by the caching mechanism
		///
		void	add_texture_glyph(int glyph_index, const texture_glyph& glyph);

		void	set_texture_glyph_nominal_size(int size) { m_texture_glyph_nominal_size = imax(1, size); }
		int	get_texture_glyph_nominal_size() const { return m_texture_glyph_nominal_size; }

		int	get_glyph_index(uint16_t code) const;
		float	get_advance(int glyph_index) const;

		/// \brief
		/// Return the adjustment in advance between the given two
		/// characters. 
		//
		/// Normally this will be 0
		///
		float	get_kerning_adjustment(int last_code, int this_code) const;
		float	get_leading() const { return m_leading; }
		float	get_descent() const { return m_descent; }

	private:
		void	read_code_table(stream* in);

		/// Read a DefineFont2 or DefineFont3 tag
		void readDefineFont2_or_3(stream* in, movie_definition* m);

		/// Read a DefineFont tag
		void readDefineFont(stream* in, movie_definition* m);

		/// Add a glyph from the os font.
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

		typedef std::vector< boost::intrusive_ptr<shape_character_def> > GlyphVect;
		GlyphVect m_glyphs;

		typedef std::vector< texture_glyph > TextureGlyphVect;
		TextureGlyphVect m_texture_glyphs;	// cached info, built by gnash_fontlib.

		int	m_texture_glyph_nominal_size;

		char*	m_name;
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
		code_table m_code_table; 

		// Layout stuff.
		float	m_ascent;
		float	m_descent;
		float	m_leading;
		std::vector<float>	m_advance_table;

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
