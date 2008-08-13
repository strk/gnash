// text.cpp:  Implementation of ActionScript text tags, for Gnash.
// 
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "utf8.h"
#include "utility.h"
#include "impl.h"
#include "shape_character_def.h"
#include "SWFStream.h"
#include "log.h"
#include "font.h"
#include "fontlib.h"
#include "render.h"
#include "text.h"
#include "movie_definition.h"

// Define the following macro to get debugging messages
// for text rendering
//#define GNASH_DEBUG_TEXT_RENDERING 1

// Define the following macro to have invalid glyphs drawn as
// empty boxes
//#define DRAW_INVALID_GLYPHS_AS_EMPTY_BOXES 1


namespace gnash {

	bool text_style::setFont(int id, movie_definition& root_def) 
	{
		return resolve_font(id, root_def);
	}

	bool text_style::resolve_font(int id, const movie_definition& root_def)
	{
		assert(id >= 0);

		_font = root_def.get_font(id);
		if (_font == NULL)
		{
			IF_VERBOSE_MALFORMED_SWF(
                log_error(_("text style references unknown font (id = %d)"), id);
			);
			return false;
		}

		return true;
	}

	void text_glyph_record::read(SWFStream& in, int glyph_count,
			int glyph_bits, int advance_bits)
	{
		// TODO: shouldn't we take unsigned for *_bits ?
		m_glyphs.resize(glyph_count);
		in.ensureBits(glyph_count * (glyph_bits+advance_bits));
		for (int i = 0; i < glyph_count; i++)
		{
			m_glyphs[i].m_glyph_index = in.read_uint(glyph_bits);
			m_glyphs[i].m_glyph_advance = (float) in.read_sint(advance_bits);
		}
	}

	// Render the given glyph records.
	void	display_glyph_records(
		const matrix& this_mat,
		character* inst,
		const std::vector<text_glyph_record>& records,
		bool useEmbeddedGlyphs)
	{
		//GNASH_REPORT_FUNCTION;
		
		static std::vector<fill_style>	s_dummy_style;	// used to pass a color on to shape_character::display()
		static std::vector<line_style>	s_dummy_line_style;
		s_dummy_style.resize(1);

		matrix	mat = inst->get_world_matrix();
		mat.concatenate(this_mat);

		cxform	cx = inst->get_world_cxform();
		matrix	base_matrix = mat;

		float x = 0.0f;
		float y = 0.0f;

		for (unsigned int i = 0; i < records.size(); i++)
		{
			// Draw the characters within the current record; i.e. consecutive
			// chars that share a particular style.
			const text_glyph_record&	rec = records[i];

			const font*	fnt = rec.m_style.getFont();
			if (fnt == NULL)
			{
#ifdef GNASH_DEBUG_TEXT_RENDERING
				log_debug("No font in style of record %u", i);
#endif
				continue;
			}

			// unitsPerEM returns an int, we cast to float to get
			// a float division
			float unitsPerEM = fnt->unitsPerEM(useEmbeddedGlyphs);
			float scale = rec.m_style.m_text_height / unitsPerEM;

#ifdef GNASH_DEBUG_TEXT_RENDERING
			log_debug("font for record %u == %p", i, (const void*)fnt);
#endif

			if ( rec.m_style.hasXOffset() ) x = rec.m_style.getXOffset();
			if ( rec.m_style.hasYOffset() ) y = rec.m_style.getYOffset();

			boost::int16_t startX = x; // for the underline, if any

			s_dummy_style[0].set_color(rec.m_style.m_color);

			rgba	transformed_color = cx.transform(rec.m_style.m_color);

			unsigned int nglyphs = rec.m_glyphs.size();
			for (unsigned int j = 0; j < nglyphs; ++j)
			{
				// the glyph entry
				const text_glyph_record::glyph_entry& ge = rec.m_glyphs[j];

				int	index = ge.m_glyph_index;
					
				mat = base_matrix;
				mat.concatenate_translation(x, y);
				mat.concatenate_scale(scale, scale);

				if (index == -1)
				{
#ifdef GNASH_DEBUG_TEXT_RENDERING
log_error(_("invalid glyph (-1)"));
#endif

#ifdef DRAW_INVALID_GLYPHS_AS_EMPTY_BOXES
					// The EM square is 1024x1024, but usually isn't filled up.
					// We'll use about half the width, and around 3/4 the height.
					// Values adjusted by eye.
					// The Y baseline is at 0; negative Y is up.
					//
					// TODO: FIXME (if we'll ever enable it back): the EM
					//       square is not hard-coded anymore but can be
					//       queried from the font class
					//
					static const boost::int16_t	s_empty_char_box[5 * 2] =
					{
						 32,   32,
						480,   32,
						480, -656,
						 32, -656,
						 32,   32
					};
					render::draw_line_strip(s_empty_char_box, 5, transformed_color, mat);  
#endif

				}
				else
				{
					shape_character_def*	glyph = fnt->get_glyph(index, useEmbeddedGlyphs);

					// Draw the character using the filled outline.
					if (glyph)
					{
#ifdef GNASH_DEBUG_TEXT_RENDERING
log_debug(_("render shape glyph using filled outline (render::draw_glyph)"));
#endif

						gnash::render::draw_glyph(glyph, mat, transformed_color);
					}
				}
				x += ge.m_glyph_advance;
			}

			bool underline = rec.m_style.isUnderlined(); 
			if ( nglyphs && underline )
			{
				// Underline should end where last displayed glyphs
				// does. 'x' here is where next glyph would be displayed
				// which is normally after some space.
				// For more precise metrics we should substract the advance
				// of last glyph and add the actual size of it.
				// This will only be known if a glyph was actually found,
				// or would be the size of the empty box (arbitrary size)
				//
				boost::int16_t endX = (int)x; // - rec.m_glyphs.back().m_glyph_advance + (480.0*scale);

				// The underline is made to be some pixels below the baseline (0)
				// and scaled so it's further as font size increases.
				//
				// 1/4 the EM square offset far from baseline 
				boost::int16_t posY = int(y+int((unitsPerEM/4)*scale));

				boost::int16_t underline[2 * 2] =
				{
					startX,   posY,
					  endX,   posY,
				};
				render::draw_line_strip(underline, 2, transformed_color, base_matrix);
			}
		}
	}

}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
