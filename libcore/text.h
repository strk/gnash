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
#include "font.h"

namespace gnash {

	// Forward declarations
	class text_character_def; 
	class text_glyph_record; 
	class text_style; 
	class SWFStream;

	// Helper class.
	// @@ text_character_def friend ?
	class text_style
	{
	public:
		rgba	m_color;
		float	m_text_height;

		text_style()
			:
			m_text_height(1.0f),
			_hasXOffset(false),
			_hasYOffset(false),
			_underlined(false),
			_xOffset(0.0f),
			_yOffset(0.0f),
			_font(NULL)
		{
		}

		/// Should text be underlined ?
		bool isUnderlined() const
		{
			return _underlined;
		}

		/// Specify whether text should be underlined or not
		void setUnderlined(bool v) { _underlined=v; }

		/// Set an X offset
		void setXOffset(float o)
		{
			_hasXOffset=true;
			_xOffset=o;
		}

		/// Drop X offset
		void dropXOffset()
		{
			_hasXOffset=false;
			_xOffset=0; // we shouldn't need this..
		}

		/// Shift X offset by given amount
		void shiftXOffset(float xo)
		{
			//assert(_hasXOffset)
			_xOffset+=xo;
		}

		/// Return true if text has an X offset
		//
		// TODO: is this really needed ?
		//
		bool hasXOffset() const
		{
			return _hasXOffset;
		}

		/// Return the X offset
		float getXOffset() const
		{
			return _xOffset;
		}

		/// Set an Y offset
		void setYOffset(float o)
		{
			_hasYOffset = true;
			_yOffset=o;
		}

		/// Drop X offset
		void dropYOffset()
		{
			_hasYOffset = false; 
			_yOffset=0; // we shouldn't need this..
		}

		/// Shift Y offset by given amount
		void shiftYOffset(float yo)
		{
			//assert(_hasYOffset)
			_yOffset+=yo;
		}

		/// Return true if text has an Y offset
		bool hasYOffset() const
		{
			return _hasYOffset;
		}

		/// Return the Y offset
		float getYOffset() const
		{
			return _yOffset;
		}

		/// Set font by id and movie_definition
		//
		/// This method will perform a lookup from the movie_definition
		/// and appropriately set the _font member.
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
			_font = fnt;
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
			return _font;
		}

	private:

		bool _hasXOffset;
		bool _hasYOffset;
		bool _underlined;

		float	_xOffset;
		float	_yOffset;

		const font* _font;

		/// Set font based on id
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

		void read(SWFStream& in, int glyph_count,
			int glyph_bits, int advance_bits);

	};

	/// Render the given glyph records.
	//
	/// @param useEmbeddedGlyphs
	///	If true, the font will be queried for embedded glyphs.
	///	Otherwise, the font will be queried for device fonts.
	///
	void display_glyph_records(
		const SWFMatrix& this_mat,
		character* inst,
		const std::vector<text_glyph_record>& records,
		bool useEmbeddedGlyphs);

} // namespace gnash

#endif // GNASH_TEXT_H
