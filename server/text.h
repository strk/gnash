// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Code for the text tags.


#ifndef GNASH_TEXT_H
#define GNASH_TEXT_H

namespace gnash {

	// Forward declarations
	struct text_character_def; 
	struct text_glyph_record; 
	struct text_style; 

	// Helper struct.
	// @@ text_character_def friend ?
	struct text_style
	{
		int	m_font_id;
		mutable font*	m_font;
		rgba	m_color;
		float	m_x_offset;
		float	m_y_offset;
		float	m_text_height;
		bool	m_has_x_offset;
		bool	m_has_y_offset;

		text_style()
			:
			m_font_id(-1),
			m_font(NULL),
			m_x_offset(0),
			m_y_offset(0),
			m_text_height(1.0f),
			m_has_x_offset(false),
			m_has_y_offset(false)
		{
		}

		void	resolve_font(movie_definition_sub* root_def) const;
	};


	// Helper struct.
	// @@ text_character_def friend ?
	struct text_glyph_record
	{
		struct glyph_entry
		{
			int	m_glyph_index;
			float	m_glyph_advance;
		};
		text_style	m_style;
		array<glyph_entry>	m_glyphs;

		void read(stream* in, int glyph_count,
			int glyph_bits, int advance_bits);
	};

	/// Text character 
	//
	/// This is either read from SWF stream 
	/// or (hopefully) created with scripting
	///
	struct text_character_def : public character_def
	{
		movie_definition_sub*	m_root_def;
		rect	m_rect;
		matrix	m_matrix;
		array<text_glyph_record>	m_text_glyph_records;

		text_character_def(movie_definition_sub* root_def)
			:
			m_root_def(root_def)
		{
			assert(m_root_def);
		}

		void read(stream* in, int tag_type, movie_definition_sub* m);

		/// Draw the string.
		void display(character* inst);

	};

} // namespace gnash

#endif // GNASH_TEXT_H
