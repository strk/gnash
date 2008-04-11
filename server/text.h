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

// Code for the text tags.


#ifndef GNASH_TEXT_H
#define GNASH_TEXT_H

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
		rgba	m_color;
		float	m_x_offset;
		float	m_y_offset;
		float	m_text_height;
		bool	m_has_x_offset;
		bool	m_has_y_offset;


		text_style()
			:
			m_x_offset(0),
			m_y_offset(0),
			m_text_height(1.0f),
			m_has_x_offset(false),
			m_has_y_offset(false),
			m_font(NULL)
		{
		}

		/// Should text be underlined ?
		bool isUnderlined() const { return _underlined; }

		/// Specify whether text should be underlined or not
		void setUnderlined(bool v) { _underlined=v; }

		/// Set an X offset
		void setXOffset(float o)
		{
			// TODO: is this really needed ?
			m_has_x_offset=true;
			m_x_offset=o;
		}

		/// Shift X offset by given amount
		void shiftXOffset(float xo)
		{
			//assert(m_has_x_offset)
			m_x_offset+=xo;
		}

		/// Return true if text has an X offset
		//
		// TODO: is this really needed ?
		//
		bool hasXOffset() const
		{
			return m_has_x_offset;
		}

		/// Return the X offset
		float getXOffset() const
		{
			return m_x_offset;
		}

		/// Set an Y offset
		void setYOffset(float o)
		{
			m_has_y_offset=true;
			m_y_offset=o;
		}

		/// Shift Y offset by given amount
		void shiftYOffset(float yo)
		{
			//assert(m_has_y_offset)
			m_y_offset+=yo;
		}

		/// Return true if text has an Y offset
		//
		// TODO: is this really needed ?
		//
		bool hasYOffset() const
		{
			return m_has_y_offset;
		}

		/// Return the Y offset
		float getYOffset() const
		{
			return m_y_offset;
		}

		/// Set font by id and movie_definition
		//
		/// This method will perform a lookup from the movie_definition
		/// and appropriately set the m_font member.
		///
		/// @param id
		///	The font id.
		///
		/// @param root_def
		///	The movie_definition used for looking up font by id
		///
		/// @return true on success, false on error (unknown font id)
		///
		bool setFont(int id, movie_definition& def);

		/// Set font by font pointer.
		//
		/// @param fnt
		///	The font pointer.
		///	Must not be NULL or an assertion will fail.
		///
		void setFont(const font* fnt)
		{
			assert(fnt);
			m_font = fnt;
		}

		/// Return the associated font (possibly NULL).
		//
		/// @return 
		///	The font associated with this style. 
		///	NOTE: it may be NULL if a font set by id/movie_definition
		///	      could not be resolved.
		///
		const font* getFont() const
		{
			return m_font;
		}

	private:

		// TODO: turn _underlined, has_x_offset and has_y_offset
		//       into a single bitwise flag.
		//       Also, check if the has_{x,y}_offset are needed
		//       at all !
		bool	_underlined;

		const font* m_font;

		/// Set m_font based on m_font_id.
		//
		/// @param root_def
		///	The movie_definition used for looking up font by id
		///
		/// @return true on success, false on error
		///	(unknown font id, would print an swferror about it)
		///
		bool	resolve_font(int id, const movie_definition& root_def);
	};


	/// A vector of glyphs sharing the same text_style
	//
	/// For each glyph, this class stores index and advance values
	///
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
	//
	/// @param useEmbeddedGlyphs
	///	If true, the font will be queried for embedded glyphs.
	///	Otherwise, the font will be queried for device fonts.
	///
	void display_glyph_records(
		const matrix& this_mat,
		character* inst,
		const std::vector<text_glyph_record>& records,
		movie_definition* root_def, bool useEmbeddedGlyphs);

} // namespace gnash

#endif // GNASH_TEXT_H
