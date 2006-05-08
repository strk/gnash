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
	struct shape_character_def;
	struct stream;
	struct bitmap_info;

	
	// Struct for holding (cached) textured glyph info.
	struct texture_glyph : public ref_counted
	{
		smart_ptr<bitmap_info>	m_bitmap_info;
		rect	m_uv_bounds;
		point	m_uv_origin;	// the origin of the glyph box, in uv coords

		texture_glyph() : m_bitmap_info(NULL) {}

		~texture_glyph()
		{
		}

		bool	is_renderable() const
		// Return true if this can be used for rendering.
		{
			return m_bitmap_info != NULL;
		}

		void	set_bitmap_info(bitmap_info* bi)
		{
			m_bitmap_info = bi;
		}
	};


	class font : public resource
	{
	public:
		font();
		~font();

		// override from resource.
		virtual font*	cast_to_font() { return this; }

		int	get_glyph_count() const { return m_glyphs.size(); }
		shape_character_def*	get_glyph(int glyph_index) const;
		void	read(stream* in, int tag_type, movie_definition* m);
		void	read_font_info(stream* in);

		void	output_cached_data(tu_file* out, const cache_options& options);
		void	input_cached_data(tu_file* in);

		void	wipe_texture_glyphs();

		const char*	get_name() const { return m_name; }
		movie_definition* get_owning_movie() const { return m_owning_movie; }

		const texture_glyph&	get_texture_glyph(int glyph_index) const;
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
