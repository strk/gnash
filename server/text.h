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
//
//

// Code for the text tags.


#ifndef GNASH_TEXT_H
#define GNASH_TEXT_H

#include "textformat.h" // maybe we should include it here
#include "styles.h" 

namespace gnash {

	// Forward declarations
	class text_character_def; 
	class text_glyph_record; 
	class text_style; 

	// Helper class.
	// @@ text_character_def friend ?
	class text_style
	{
	public:
		int	m_font_id;
		mutable const font*	m_font;
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

		void	resolve_font(movie_definition* root_def) const;
	};


	// Helper class.
	// @@ text_character_def friend ?
	class text_glyph_record
	{
	public:
		struct glyph_entry
		{
			int	m_glyph_index;
			float	m_glyph_advance;
		};
		text_style	m_style;
		std::vector<glyph_entry>	m_glyphs;

		void read(stream* in, int glyph_count,
			int glyph_bits, int advance_bits);

	};

	/// Render the given glyph records.
	void display_glyph_records(
		const matrix& this_mat,
		character* inst,
		const std::vector<text_glyph_record>& records,
		movie_definition* root_def);

} // namespace gnash

#endif // GNASH_TEXT_H
