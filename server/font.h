// font.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A font type for gnash.


#ifndef GNASH_FONT_H
#define GNASH_FONT_H


#include "container.h"
#include "gnash.h"
#include "types.h"
#include "impl.h"
class tu_file;

namespace gnash {
	struct movie;
	class shape_character_def;
	struct stream;
	struct bitmap_info;

	
	/// Struct for holding (cached) textured glyph info.
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

		/// Argument will be assigned to a smart_ptr
		void	set_bitmap_info(bitmap_info* bi)
		{
			m_bitmap_info = bi;
		}

	// too early to make these private, fontlib directly accesses
	// them, postponed.
	//private:

		smart_ptr<bitmap_info>	m_bitmap_info;

		rect	m_uv_bounds;

		// the origin of the glyph box, in uv coords
		point	m_uv_origin;

	};

	/// \brief
	/// A 'font' definition as read from SWF::DefineFont or
	/// SWF::DefineFont2 tags.
	/// Includes definitions from SWF::DefineFontInfo tags
	///
	class font : public resource
	{
	public:
		font();
		~font();

		// override from resource.
		virtual font*	cast_to_font() { return this; }

		/// Get number of glyphs defined for this font
		int	get_glyph_count() const { return m_glyphs.size(); }

		/// Get glyph by index. Return NULL if out of range
		shape_character_def*	get_glyph(int glyph_index) const;

		/// Read a DefineFont or DefineFont2 tag from an SWF stream 
		//
		/// @param in the SWF stream
		/// @param tag_type either DefineFont or DefineFont2
		/// @param m the movie_definition containing this definition
		///          (or "owning" this font)
		///
		void	read(stream* in, int tag_type, movie_definition* m);

		/// \brief
		/// Read additional information about this font, from a
		/// DefineFontInfo tag. 
		//
		/// The caller has already read the tag type and font id.
		///
		/// @see SWF::define_font_info_loader
		///
		void	read_font_info(stream* in);

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
		/// Return a pointer to a texture_glyph struct
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
		void	add_texture_glyph(int glyph_index, const texture_glyph& glyph);

		void	set_texture_glyph_nominal_size(int size) { m_texture_glyph_nominal_size = imax(1, size); }
		int	get_texture_glyph_nominal_size() const { return m_texture_glyph_nominal_size; }

		int	get_glyph_index(uint16_t code) const;
		float	get_advance(int glyph_index) const;
		float	get_kerning_adjustment(int last_code, int this_code) const;
		float	get_leading() const { return m_leading; }
		float	get_descent() const { return m_descent; }

	private:
		void	read_code_table(stream* in);

		/// Read a DefineFont2 tag
		void readDefineFont2(stream* in, movie_definition* m);

		// Read a DefineFont tag
		void readDefineFont(stream* in, movie_definition* m);

		std::vector< smart_ptr<shape_character_def> >	m_glyphs;

		std::vector< texture_glyph >	m_texture_glyphs;	// cached info, built by gnash_fontlib.

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
		// m_code_table[character_code] = glyph_index
		//
		// @@ TODO: avoid little allocs; replace this with a flat hash, or else a sorted array (binary search)
		template<class T>
		struct simple_code_hash
		// Dummy hash functor.
		{
			size_t	operator()(const T& data) const { return data; }
		};
		hash<uint16_t, int, simple_code_hash<uint16_t> > m_code_table;

		// Layout stuff.
		float	m_ascent;
		float	m_descent;
		float	m_leading;
		std::vector<float>	m_advance_table;
		// @@ we don't seem to use this thing at all, so don't bother keeping it.
		// std::vector<rect>	m_bounds_table;	// @@ this thing should be optional.

		// @@ replace this with a flat hash, or else a sorted array (binary search)
		struct kerning_pair
		{
			uint16_t	m_char0, m_char1;

			bool	operator==(const kerning_pair& k) const
			{
				return m_char0 == k.m_char0 && m_char1 == k.m_char1;
			}
		};
		hash<kerning_pair, float>	m_kerning_pairs;
	};

}	// end namespace gnash



#endif // GNASH_FONT_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
