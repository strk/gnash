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
#include "stream.h"
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

		m_font = root_def.get_font(id);
		if (m_font == NULL)
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_error(_("text style references unknown font (id = %d)"),
				id);
			);
			return false;
		}

		return true;
	}

	void text_glyph_record::read(stream* in, int glyph_count,
			int glyph_bits, int advance_bits)
	{
		// TODO: shouldn't we take unsigned for *_bits ?
		m_glyphs.resize(glyph_count);
		in->ensureBits(glyph_count * (glyph_bits+advance_bits));
		for (int i = 0; i < glyph_count; i++)
		{
			m_glyphs[i].m_glyph_index = in->read_uint(glyph_bits);
			m_glyphs[i].m_glyph_advance = (float) in->read_sint(advance_bits);
		}
	}

	// Render the given glyph records.
	void	display_glyph_records(
		const matrix& this_mat,
		character* inst,
		const std::vector<text_glyph_record>& records,
		// root_def was used to resove fonts, now done at parse time
		movie_definition* /*root_def*/, bool useEmbeddedGlyphs)
	{
		//GNASH_REPORT_FUNCTION;
		
		static std::vector<fill_style>	s_dummy_style;	// used to pass a color on to shape_character::display()
		static std::vector<line_style>	s_dummy_line_style;
		s_dummy_style.resize(1);

		matrix	mat = inst->get_world_matrix();
		mat.concatenate(this_mat);

		cxform	cx = inst->get_world_cxform();
		float	pixel_scale = inst->get_pixel_scale();

		matrix	base_matrix = mat;

		float	scale = 1.0f;
		float	x = 0.0f;
		float	y = 0.0f;

		for (unsigned int i = 0; i < records.size(); i++)
		{
			// Draw the characters within the current record; i.e. consecutive
			// chars that share a particular style.
			const text_glyph_record&	rec = records[i];

			//rec.m_style.resolve_font(root_def);

			const font*	fnt = rec.m_style.getFont();
			if (fnt == NULL)
			{
#ifdef GNASH_DEBUG_TEXT_RENDERING
				log_debug("No font in style of record %u", i);
#endif
				continue;
			}

			scale = rec.m_style.m_text_height / 1024.0f;	// the EM square is 1024 x 1024

#ifdef GNASH_DEBUG_TEXT_RENDERING
			log_debug("font for record %u == %p", i, (const void*)rec.m_style.getFont());
#endif

			if (rec.m_style.m_has_x_offset)
			{
				x = rec.m_style.m_x_offset;
			}
			if (rec.m_style.m_has_y_offset)
			{
				y = rec.m_style.m_y_offset;
			}

			s_dummy_style[0].set_color(rec.m_style.m_color);

			rgba	transformed_color = cx.transform(rec.m_style.m_color);

			for (unsigned int j = 0; j < rec.m_glyphs.size(); j++)
			{
				int	index = rec.m_glyphs[j].m_glyph_index;
					
				mat = base_matrix;
				mat.concatenate_translation(x, y);
				//mat.concatenate_scale(scale);
				if (fnt->is_subpixel_font())
					mat.concatenate_scale(0.05f * scale);
				else
					mat.concatenate_scale(scale);

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

						gnash::render::draw_glyph(glyph, mat, transformed_color, pixel_scale);
						
					}
				}
				x += rec.m_glyphs[j].m_glyph_advance;
			}
		}
	}

}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
