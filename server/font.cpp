// font.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A font type for gnash.


#include "font.h"
#include "stream.h"
#include "impl.h"
#include "log.h"
//#include "shape.h"
#include "tu_file.h"
#include "movie_definition.h"
#include "shape_character_def.h"
#include "swf.h"

#include <utility> // for std::make_pair

namespace gnash {
	font::font()
		:
		m_texture_glyph_nominal_size(96),	// Default is not important; gets overridden during glyph generation
		m_name(NULL),
		m_owning_movie(NULL),
		m_unicode_chars(false),
		m_shift_jis_chars(false),
		m_ansi_chars(true),
		m_is_italic(false),
		m_is_bold(false),
		m_wide_codes(false),
		m_ascent(0.0f),
		m_descent(0.0f),
		m_leading(0.0f)
	{
	}

	font::~font()
	{
		m_glyphs.resize(0);

		// Delete the name string.
		delete [] m_name;
	}

	shape_character_def*	font::get_glyph(int index) const
	{
		if (index >= 0 && index < (int) m_glyphs.size())
		{
			return m_glyphs[index].get_ptr();
		}
		else
		{
			return NULL;
		}
	}


	const texture_glyph&	font::get_texture_glyph(int glyph_index) const
	// Return a pointer to a texture_glyph struct corresponding to
	// the given glyph_index, if we have one.  
	// Otherwise return a "dummy" texture_glyph.
	{
		if (glyph_index < 0 || glyph_index >= (int) m_texture_glyphs.size())
		{
			static const texture_glyph	s_dummy_texture_glyph;
			return s_dummy_texture_glyph;
		}

		return m_texture_glyphs[glyph_index];
	}


	void	font::add_texture_glyph(int glyph_index, const texture_glyph& glyph)
	// Register some texture info for the glyph at the specified
	// index.  The texture_glyph can be used later to render the
	// glyph.
	{
		assert(glyph_index >= 0 && glyph_index < (int) m_glyphs.size());
		assert(m_texture_glyphs.size() == m_glyphs.size());
		assert(glyph.is_renderable());

		assert(m_texture_glyphs[glyph_index].is_renderable() == false);

		m_texture_glyphs[glyph_index] = glyph;
	}


	void	font::wipe_texture_glyphs()
	// Delete all our texture glyph info.
	{
		assert(m_texture_glyphs.size() == m_glyphs.size());

		// Replace with default (empty) glyph info.
		texture_glyph	default_tg;
		for (int i = 0, n = m_texture_glyphs.size(); i < n; i++)
		{
			m_texture_glyphs[i] = default_tg;
		}
	}


	void	font::read(stream* in, SWF::tag_type tag, movie_definition* m)
	{
		// No add_ref() here, to avoid cycle. 
		// m_owning_movie is our owner, so it has a ref to us.
		m_owning_movie = m;

		if (tag == SWF::DEFINEFONT)
		{
			readDefineFont(in, m);
		}
		else
		{
			assert (tag == SWF::DEFINEFONT2);
			readDefineFont2(in, m);
		}
	}

	// Read a DefineFont tag
	void font::readDefineFont(stream* in, movie_definition* m)
	{
		IF_VERBOSE_PARSE (
		log_parse("reading DefineFont");
		);

		int	table_base = in->get_position();

		// Read the glyph offsets.  Offsets
		// are measured from the start of the
		// offset table.
		std::vector<int>	offsets;
		offsets.push_back(in->read_u16());

		IF_VERBOSE_PARSE (
		log_parse("offset[0] = %d", offsets[0]);
		);

		int	count = offsets[0] >> 1;
		for (int i = 1; i < count; i++)
		{
			offsets.push_back(in->read_u16());

			IF_VERBOSE_PARSE (
			log_parse("offset[%d] = %d", i, offsets[i]);
			);
		}

		m_glyphs.resize(count);
		m_texture_glyphs.resize(m_glyphs.size());

		if (m->get_create_font_shapes() == DO_LOAD_FONT_SHAPES)
		{
			// Read the glyph shapes.
			{for (int i = 0; i < count; i++)
			{
				// Seek to the start of the shape data.
				int	new_pos = table_base + offsets[i];
				in->set_position(new_pos);

				// Create & read the shape.
				shape_character_def* s = new shape_character_def;
				s->read(in, 2, false, m);

				m_glyphs[i] = s;
			}}
		}
	}

	// Read a DefineFont2 tag
	void font::readDefineFont2(stream* in, movie_definition* m)
	{
		IF_VERBOSE_PARSE (
		log_parse("reading DefineFont2");
		);

		bool	has_layout = (in->read_uint(1) != 0);
		m_shift_jis_chars = (in->read_uint(1) != 0);
		m_unicode_chars = (in->read_uint(1) != 0);
		m_ansi_chars = (in->read_uint(1) != 0);
		bool	wide_offsets = (in->read_uint(1) != 0);
		m_wide_codes = (in->read_uint(1) != 0);
		m_is_italic = (in->read_uint(1) != 0);
		m_is_bold = (in->read_uint(1) != 0);
		uint8_t	reserved = in->read_u8();

		// Inhibit warning.
		reserved = reserved;

		m_name = in->read_string_with_length();

		uint16_t glyph_count = in->read_u16();
		
		int	table_base = in->get_position();

		// Read the glyph offsets.  Offsets
		// are measured from the start of the
		// offset table. Make sure wide offsets fit into elements
		std::vector<uint32_t>	offsets;
		int	font_code_offset;
		if (wide_offsets)
		{
			// 32-bit offsets.
			for (unsigned int i = 0; i < glyph_count; i++)
			{
				uint32_t off = in->read_u32();	

				IF_VERBOSE_PARSE (
				log_parse("Glyph %d at offset %u", i, off);
				);

				offsets.push_back(off);
			}
			font_code_offset = in->read_u32();
		}
		else
		{
			// 16-bit offsets.
			for (unsigned int i = 0; i < glyph_count; i++)
			{
				uint16_t off = in->read_u16();	

				IF_VERBOSE_PARSE (
				log_parse("Glyph %d at offset %u", i, off);
				);

				offsets.push_back(off);
			}
			font_code_offset = in->read_u16();
		}

		m_glyphs.resize(glyph_count);
		m_texture_glyphs.resize(m_glyphs.size());

		if (m->get_create_font_shapes() == DO_LOAD_FONT_SHAPES)
		{
			// Read the glyph shapes.
			{for (int i = 0; i < glyph_count; i++)
			{
				// Seek to the start of the shape data.
				int	new_pos = table_base + offsets[i];
				// if we're seeking backwards, then that looks like a bug.
				assert(new_pos >= in->get_position());
				in->set_position(new_pos);

				// Create & read the shape.
				shape_character_def* s = new shape_character_def;
				s->read(in, 22, false, m);

				m_glyphs[i] = s;
			}}

			int	current_position = in->get_position();
			if (font_code_offset + table_base != current_position)
			{
				// Bad offset!  Don't try to read any more.
				log_warning("Bad offset in DefineFont2!");
				return;
			}
		}
		else
		{
			// Skip the shape data.
			int	new_pos = table_base + font_code_offset;
			if (new_pos >= in->get_tag_end_position())
			{
				// No layout data!
				return;
			}

			in->set_position(new_pos);
		}

		read_code_table(in);

		// Read layout info for the glyphs.
		if (has_layout)
		{
			m_ascent = (float) in->read_s16();
			m_descent = (float) in->read_s16();
			m_leading = (float) in->read_s16();
			
			// Advance table; i.e. how wide each character is.
			m_advance_table.resize(m_glyphs.size());
			for (int i = 0, n = m_advance_table.size(); i < n; i++)
			{
				m_advance_table[i] = (float) in->read_s16();
			}

			// Bounds table.
			//m_bounds_table.resize(m_glyphs.size());	// kill
			rect	dummy_rect;
			{for (int i = 0, n = m_glyphs.size(); i < n; i++)
			{
				//m_bounds_table[i].read(in);	// kill
				dummy_rect.read(in);
			}}

			// Kerning pairs.
			int	kerning_count = in->read_u16();
			for (int i = 0; i < kerning_count; i++)
			{
				uint16_t	char0, char1;
				if (m_wide_codes)
				{
					char0 = in->read_u16();
					char1 = in->read_u16();
				}
				else
				{
					char0 = in->read_u8();
					char1 = in->read_u8();
				}
				float	adjustment = (float) in->read_s16();

				kerning_pair	k;
				k.m_char0 = char0;
				k.m_char1 = char1;

	// Remember this adjustment; we can look it up quickly
	// later using the character pair as the key.
	if ( ! m_kerning_pairs.insert(std::make_pair(k, adjustment)).second )
	{
		log_warning("Repeated kerning pair found - ignoring\n");
	}

			}
		}
	}


	// Read additional information about this font, from a
	// DefineFontInfo tag.  The caller has already read the tag
	// type and font id.
	void	font::read_font_info(stream* in, SWF::tag_type tag, movie_definition* m)
	{
		assert(tag == SWF::DEFINEFONTINFO || tag == SWF::DEFINEFONTINFO2); 


		if ( tag == SWF::DEFINEFONTINFO2 )
		{
			log_warning("DefineFontInfo2 partially implemented");
			// See: SWFalexref/SWFalexref.html#tag_definefont2

		}

		delete [] m_name;
		
		m_name = in->read_string_with_length();

		unsigned char	flags = in->read_u8();

		// The following 3 flags are reserved
		// for SWF6+

		// this is font_info_small for SWF6 or up
		m_unicode_chars = (flags & 0x20) != 0;
		m_shift_jis_chars = (flags & 0x10) != 0;
		m_ansi_chars = (flags & 0x08) != 0;

		m_is_italic = (flags & 0x04) != 0;
		m_is_bold = (flags & 0x02) != 0;
		m_wide_codes = (flags & 0x01) != 0;

		read_code_table(in);
	}

	void	font::read_code_table(stream* in)
	// Read the table that maps from glyph indices to character
	// codes.
	{
		IF_VERBOSE_PARSE (
		log_parse("reading code table at offset %d", in->get_position());
		);

		assert(m_code_table.empty());

		if (m_wide_codes)
		{
			// Code table is made of uint16_t's.
			for (int i=0, n=m_glyphs.size(); i<n; ++i)
			{
				uint16_t code = in->read_u16();
				//m_code_table.add(code, i);
				m_code_table.insert(std::make_pair(code, i));
			}
		}
		else
		{
			// Code table is made of bytes.
			for (int i=0, n=m_glyphs.size(); i<n; ++i)
			{
				uint8_t code = in->read_u8();
				//m_code_table.add(code, i);
				m_code_table.insert(std::make_pair(code, i));
			}
		}
	}

	int	font::get_glyph_index(uint16_t code) const
	{
		code_table::const_iterator it = m_code_table.find(code);
		if ( it != m_code_table.end() )
		{
			int glyph_index = it->second;
#if 0
			log_msg("get_glyph_index(%u) returning %d",
				code, glyph_index);
#endif
			return glyph_index;
		}

#if 0
		log_msg("get_glyph_index(%u) returning -1", code);
#endif
		return -1;
	}

	float	font::get_advance(int glyph_index) const
	{
		if (glyph_index == -1)
		{
			// Default advance.
			return 512.0f;
		}

		if (m_advance_table.size() == 0)
		{
			// No layout info for this font!!!
			static bool	s_logged = false;
			if (s_logged == false)
			{
				s_logged = true;
				log_error("empty advance table in font %s\n", get_name());
			}
			return 0;
		}

		if (glyph_index < (int) m_advance_table.size())
		{
			assert(glyph_index >= 0);
			return m_advance_table[glyph_index];
		}
		else
		{
			// Bad glyph index.  Due to bad data file?
			assert(0);
			return 0;
		}
	}


	// Return the adjustment in advance between the given two
	// characters.  Normally this will be 0; i.e. the 
	float	font::get_kerning_adjustment(int last_code, int code) const
	{
		kerning_pair	k;
		k.m_char0 = last_code;
		k.m_char1 = code;
		kernings_table::const_iterator it = m_kerning_pairs.find(k);
		if ( it != m_kerning_pairs.end() )
		{
			float	adjustment = it->second;
			return adjustment;
		}
		return 0;
	}


        void	font::output_cached_data(tu_file* /* out */, const cache_options& /* options */)
	// Dump our cached data into the given stream.
	{
// @@ Disabled.  Need to fix input_cached_data, so that it has a
// reliable and cheap way to skip over data for NULL glyphs.
#if 0
		// Dump cached shape data for glyphs (i.e. this will
		// be tesselations used to render larger glyph sizes).
		int	 n = m_glyphs.size();
		out->write_le32(n);
		for (int i = 0; i < n; i++)
		{
			shape_character_def*	s = m_glyphs[i].get_ptr();
			if (s)
			{
				s->output_cached_data(out, options);
			}
		}
#endif // 0
	}

	
	void	font::input_cached_data(tu_file* /* in */)
	// Read our cached data from the given stream.
	{
// @@ Disable.  See comment in output_cached_data().
#if 0
		// Read cached shape data for glyphs.
		int	n = in->read_le32();
		if (n != m_glyphs.size())
		{
			log_error("error reading cache file in font::input_cached_data() "
				  "glyph count mismatch.\n");
			in->go_to_end();	// ensure that no more data will be read from this stream.
			return;
		}

		for (int i = 0; i < n; i++)
		{
			m_glyphs[i]->input_cached_data(in);
		}
#endif // 0
	}


}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
