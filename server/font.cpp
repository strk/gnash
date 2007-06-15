// font.cpp:  ActionScript font handling, for Gnash.
// 
//   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/* $Id: font.cpp,v 1.39 2007/06/15 15:00:29 strk Exp $ */

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003

#include "font.h"
#include "stream.h"
#include "log.h"
#include "tu_file.h"
#include "movie_definition.h"
#include "shape_character_def.h"
#include "swf.h"
#include "GnashException.h"

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
		m_glyphs.resize(0); // there's no need for this !

		// Delete the name string.
		delete [] m_name;
	}

	shape_character_def*	font::get_glyph(int index) const
	{
		if (index >= 0 && index < (int) m_glyphs.size())
		{
			return m_glyphs[index].get();
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
			assert (tag == SWF::DEFINEFONT2 || tag == SWF::DEFINEFONT3);
			readDefineFont2_or_3(in, m);
		}

		if ( m_name && ! initDeviceFontProvider() )
		{
			log_error("Could not initialize device font face '%s'", m_name);
		}
	}

	// Read a DefineFont tag
	void font::readDefineFont(stream* in, movie_definition* m)
	{
		IF_VERBOSE_PARSE (
		log_parse(_("reading DefineFont"));
		);

		unsigned long table_base = in->get_position();

		// Read the glyph offsets.  Offsets
		// are measured from the start of the
		// offset table.
		std::vector<unsigned>	offsets;
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

		// TODO: use a structure to hold all of these ?
		m_glyphs.resize(count);
		m_texture_glyphs.resize(count);
		m_advance_table.resize(count);

		if (m->get_create_font_shapes() == DO_LOAD_FONT_SHAPES)
		{
			// Read the glyph shapes.
			{for (int i = 0; i < count; i++)
			{
				// Seek to the start of the shape data.
				unsigned long new_pos = table_base + offsets[i];

				if ( ! in->set_position(new_pos) )
				{
        				throw ParserException(_("Glyphs offset table corrupted in DefineFont tag"));
				}

				// Create & read the shape.
				shape_character_def* s = new shape_character_def;
				s->read(in, SWF::DEFINEFONT, false, m); 

				m_glyphs[i] = s;
			}}
		}
	}

	// Read a DefineFont2 or DefineFont3 tag
	void font::readDefineFont2_or_3(stream* in, movie_definition* m)
	{
		IF_VERBOSE_PARSE (
		log_parse(_("reading DefineFont2 or DefineFont3"));
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

		// Inhibit compiler warning.
		reserved = reserved;

		m_name = in->read_string_with_length();

		uint16_t glyph_count = in->read_u16();
		
		unsigned long table_base = in->get_position();

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
				log_parse(_("Glyph %d at offset %u"), i, off);
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
				log_parse(_("Glyph %d at offset %u"), i, off);
				);

				offsets.push_back(off);
			}
			font_code_offset = in->read_u16();
		}

		// TODO: use a structure to hold all of these ?
		m_glyphs.resize(glyph_count);
		m_texture_glyphs.resize(glyph_count);
		m_advance_table.resize(glyph_count);

		if (m->get_create_font_shapes() == DO_LOAD_FONT_SHAPES)
		{
			// Read the glyph shapes.
			{for (int i = 0; i < glyph_count; i++)
			{
				// Seek to the start of the shape data.
				unsigned long new_pos = table_base + offsets[i];

				// It seems completely possible to
				// have such seeks-back, see bug #16311
				//assert(new_pos >= in->get_position());

				if ( ! in->set_position(new_pos) )
				{
        				throw ParserException(_("Glyphs offset table corrupted in DefineFont2/3 tag"));
				}

				// Create & read the shape.
				shape_character_def* s = new shape_character_def;
				s->read(in, SWF::DEFINEFONT2, false, m); // .. or DEFINEFONT3 actually..

				m_glyphs[i] = s;
			}}

			unsigned long current_position = in->get_position();
			if (font_code_offset + table_base != current_position)
			{
				// Bad offset!  Don't try to read any more.
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Bad offset in DefineFont2"));
				);
				return;
			}
		}
		else
		{
			// Skip the shape data.
			unsigned long new_pos = table_base + font_code_offset;
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
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Repeated kerning pair found - ignoring"));
		);
	}

			}
		}
	}


	// Read additional information about this font, from a
	// DefineFontInfo tag.  The caller has already read the tag
	// type and font id.
	void	font::read_font_info(stream* in, SWF::tag_type tag,
			movie_definition* /*m*/)
	{
		assert(tag == SWF::DEFINEFONTINFO || tag == SWF::DEFINEFONTINFO2); 


		if ( tag == SWF::DEFINEFONTINFO2 )
		{
			// See: SWFalexref/SWFalexref.html#tag_definefont2
			static bool warned = false;
			if ( ! warned ) {
				log_unimpl(_("DefineFontInfo2 partially implemented"));
				warned = true;
			}
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
		log_parse(_("reading code table at offset %lu"), in->get_position());
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
		int glyph_index = -1;
		code_table::const_iterator it = m_code_table.find(code);
		if ( it != m_code_table.end() )
		{
			glyph_index = it->second;
#if 0
			log_msg(_("get_glyph_index(%u) returning %d"),
				code, glyph_index);
#endif
			return glyph_index;
		}

		// Try adding an os font, of possible
		if ( _ftProvider.get() )
		{
			glyph_index = const_cast<font*>(this)->add_os_glyph(code);
		}
#if 0
		log_msg(_("get_glyph_index(%u) returning -1"), code);
#endif
		return glyph_index;
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
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("empty advance table in font %s"), get_name());
				);
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
			shape_character_def*	s = m_glyphs[i].get();
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
			log_error(_("error reading cache file in font::input_cached_data(): "
				  "glyph count mismatch"));
			in->go_to_end();	// ensure that no more data will be read from this stream.
			return;
		}

		for (int i = 0; i < n; i++)
		{
			m_glyphs[i]->input_cached_data(in);
		}
#endif // 0
	}

	int
	font::add_os_glyph(uint16_t code)
	{
		assert ( _ftProvider.get() );
		assert(m_code_table.find(code) == m_code_table.end());

		float advance;

		// Get the vectorial glyph
		boost::intrusive_ptr<shape_character_def> sh = _ftProvider->getGlyph(code, advance);

		if ( ! sh )
		{
			log_error("Could not create shape "
					"glyph for character code %u (%c) with "
					"device font %s (%p)", code, code, m_name,
					_ftProvider.get());
			return -1;
		}

		// Find new glyph offset
		int newOffset = m_texture_glyphs.size();

		// Add the new glyph id
		m_code_table[code] = newOffset;

		// Add advance info
		m_advance_table.push_back(advance);

		// Add dummy textured glyph
		m_texture_glyphs.push_back(texture_glyph());

		// Add vector glyph
		m_glyphs.push_back(sh);

		testInvariant();

		return newOffset;
	}

	bool
	font::initDeviceFontProvider()
	{
		if ( ! m_name )
		{
			log_error("No name associated with this font, can't use device fonts (should I use a default one?)");
			return false;
		}

		_ftProvider = FreetypeGlyphsProvider::createFace(m_name, m_is_bold, m_is_italic);
		if ( ! _ftProvider.get() )
		{
			log_error("Could not create a freetype face %s", m_name);
			return false;
		}
		return true;
	}

#ifdef GNASH_USE_GC
/// Mark reachable resources (for the GC)
//
/// Reachable resources are:
///	- texture_glyphs
///	- shape_character_defs (vector glyphs)
void
font::markReachableResources() const
{
	// Mark textured glyphs
	for (TextureGlyphVect::const_iterator i=m_texture_glyphs.begin(), e=m_texture_glyphs.end(); i!=e; ++i)
	{
		i->setReachable();
	}

	// Mark vector glyphs
	for (GlyphVect::const_iterator i=m_glyphs.begin(), e=m_glyphs.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

}
#endif // GNASH_USE_GC


}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
