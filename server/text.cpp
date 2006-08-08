// text.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code for the text tags.


#include "utf8.h"
#include "utility.h"
#include "impl.h"
//#include "shape.h"
#include "shape_character_def.h"
#include "stream.h"
#include "log.h"
#include "font.h"
#include "fontlib.h"
#include "render.h"
#include "textformat.h"
#include "text.h"
#include "movie_definition.h"

namespace gnash {

	void text_style::resolve_font(movie_definition* root_def) const
	{
		if (m_font == NULL)
		{
			assert(m_font_id >= 0);

			m_font = root_def->get_font(m_font_id);
			if (m_font == NULL)
			{
				log_error("error: text style with undefined font; font_id = %d\n", m_font_id);
			}
		}
	}

	void text_glyph_record::read(stream* in, int glyph_count,
			int glyph_bits, int advance_bits)
	{
		m_glyphs.resize(glyph_count);
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
		movie_definition* root_def)
	{
//		GNASH_REPORT_FUNCTION;
		
		static std::vector<fill_style>	s_dummy_style;	// used to pass a color on to shape_character::display()
		static std::vector<line_style>	s_dummy_line_style;
		s_dummy_style.resize(1);

		matrix	mat = inst->get_world_matrix();
		mat.concatenate(this_mat);

		cxform	cx = inst->get_world_cxform();
		float	pixel_scale = inst->get_pixel_scale();

//		display_info	sub_di = di;
//		sub_di.m_matrix.concatenate(mat);

//		matrix	base_matrix = sub_di.m_matrix;
		matrix	base_matrix = mat;
		float	base_matrix_max_scale = base_matrix.get_max_scale();

		float	scale = 1.0f;
		float	x = 0.0f;
		float	y = 0.0f;

		for (unsigned int i = 0; i < records.size(); i++)
		{
			// Draw the characters within the current record; i.e. consecutive
			// chars that share a particular style.
			const text_glyph_record&	rec = records[i];

			rec.m_style.resolve_font(root_def);

			const font*	fnt = rec.m_style.m_font;
			if (fnt == NULL)
			{
				continue;
			}

			scale = rec.m_style.m_text_height / 1024.0f;	// the EM square is 1024 x 1024
			float	text_screen_height = base_matrix_max_scale
				* scale
				* 1024.0f
				/ 20.0f
				* pixel_scale;

			int	nominal_glyph_height = fnt->get_texture_glyph_nominal_size();
			float	max_glyph_height = fontlib::get_texture_glyph_max_height(fnt);
#ifdef GNASH_ALWAYS_USE_TEXTURES_FOR_TEXT_WHEN_POSSIBLE
			const bool	use_glyph_textures = true;
#else
			bool	use_glyph_textures =
				text_screen_height <= max_glyph_height * 1.0f;
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
				mat.concatenate_scale(scale);

				if (index == -1)
				{
					// Invalid glyph; render it as an empty box.
					render::set_matrix(mat);
					render::line_style_color(transformed_color);

					// The EM square is 1024x1024, but usually isn't filled up.
					// We'll use about half the width, and around 3/4 the height.
					// Values adjusted by eye.
					// The Y baseline is at 0; negative Y is up.
					static const int16_t	s_empty_char_box[5 * 2] =
					{
						 32,   32,
						480,   32,
						480, -656,
						 32, -656,
						 32,   32
					};
					render::draw_line_strip(s_empty_char_box, 5);
				}
				else
				{
					const texture_glyph&	tg = fnt->get_texture_glyph(index);
					shape_character_def*	glyph = fnt->get_glyph(index);

					if (tg.is_renderable()
					    && (use_glyph_textures || glyph == NULL))
					{
						fontlib::draw_glyph(mat, tg, transformed_color, nominal_glyph_height);
					}
					else
					{

						// Draw the character using the filled outline.
						if (glyph)
						{
							glyph->display(mat, cx, pixel_scale, s_dummy_style, s_dummy_line_style);
						}
					}
				}
				x += rec.m_glyphs[j].m_glyph_advance;
			}
		}
	}

	//
	// text_character_def
	// 

	void text_character_def::read(stream* in, int tag_type,
			movie_definition* m)
	{
		assert(m != NULL);
		assert(tag_type == 11 || tag_type == 33);

		m_rect.read(in);
		m_matrix.read(in);

		int	glyph_bits = in->read_u8();
		int	advance_bits = in->read_u8();

		IF_VERBOSE_PARSE(
		log_parse("begin text records\n");
		);

		bool	last_record_was_style_change = false;

		text_style	style;
		for (;;)
		{
			int	first_byte = in->read_u8();
			
			if (first_byte == 0)
			{
				// This is the end of the text records.
				IF_VERBOSE_PARSE(
			    	log_parse("end text records\n");
				);
			    	break;
			}

			// Style changes and glyph records just alternate.
			// (Contrary to what most SWF references say!)
			if (last_record_was_style_change == false)
			{
				// This is a style change.

				last_record_was_style_change = true;

				bool	has_font = (first_byte >> 3) & 1;
				bool	has_color = (first_byte >> 2) & 1;
				bool	has_y_offset = (first_byte >> 1) & 1;
				bool	has_x_offset = (first_byte >> 0) & 1;

				IF_VERBOSE_PARSE(
				log_parse("  text style change\n");
				);

				if (has_font)
				{
					uint16_t	font_id = in->read_u16();
					style.m_font_id = font_id;
					IF_VERBOSE_PARSE(
					log_parse("  has_font: font id = %d\n", font_id);
					);
				}
				if (has_color)
				{
					if (tag_type == 11)
					{
						style.m_color.read_rgb(in);
					}
					else
					{
						assert(tag_type == 33);
						style.m_color.read_rgba(in);
					}
					IF_VERBOSE_PARSE(
					log_parse("  has_color\n");
					);
				}
				if (has_x_offset)
				{
					style.m_has_x_offset = true;
					style.m_x_offset = in->read_s16();
					IF_VERBOSE_PARSE(
					log_parse("  has_x_offset = %g\n", style.m_x_offset);
					);
				}
				else
				{
					style.m_has_x_offset = false;
					style.m_x_offset = 0.0f;
				}
				if (has_y_offset)
				{
					style.m_has_y_offset = true;
					style.m_y_offset = in->read_s16();
					IF_VERBOSE_PARSE(
					log_parse("  has_y_offset = %g\n", style.m_y_offset);
					);
				}
				else
				{
					style.m_has_y_offset = false;
					style.m_y_offset = 0.0f;
				}
				if (has_font)
				{
					style.m_text_height = in->read_u16();
					IF_VERBOSE_PARSE(
					log_parse("  text_height = %g\n", style.m_text_height);
					);
				}
			}
			else
			{
				// Read the glyph record.

				last_record_was_style_change = false;

				int	glyph_count = first_byte;

// 					if (! last_record_was_style_change)
// 					{
// 						glyph_count &= 0x7F;
// 					}
// 					// else { Don't mask the top bit; the first record is allowed to have > 127 glyphs. }

				m_text_glyph_records.resize(m_text_glyph_records.size() + 1);
				m_text_glyph_records.back().m_style = style;
				m_text_glyph_records.back().read(in, glyph_count, glyph_bits, advance_bits);

				IF_VERBOSE_PARSE(
				log_parse("  glyph_records: count = %d\n", glyph_count);
				);
			}
		}
	}

	void text_character_def::display(character* inst)
	{
// 	        GNASH_REPORT_FUNCTION;
		display_glyph_records(m_matrix, inst,
			m_text_glyph_records, m_root_def);
	}


}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

