// text.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code for the text tags.


#include "utf8.h"
#include "utility.h"
#include "impl.h"
#include "shape.h"
#include "stream.h"
#include "log.h"
#include "font.h"
#include "fontlib.h"
#include "render.h"
#include "textformat.h"
#include "text.h"
#include "Movie.h"

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
	static void	display_glyph_records(
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

			font*	fnt = rec.m_style.m_font;
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
					static const Sint16	s_empty_char_box[5 * 2] =
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

		IF_VERBOSE_PARSE(log_msg("begin text records\n"));

		bool	last_record_was_style_change = false;

		text_style	style;
		for (;;)
		{
			int	first_byte = in->read_u8();
			
			if (first_byte == 0)
			{
				// This is the end of the text records.
				IF_VERBOSE_PARSE(log_msg("end text records\n"));
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

				IF_VERBOSE_PARSE(log_msg("  text style change\n"));

				if (has_font)
				{
					Uint16	font_id = in->read_u16();
					style.m_font_id = font_id;
					IF_VERBOSE_PARSE(log_msg("  has_font: font id = %d\n", font_id));
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
					IF_VERBOSE_PARSE(log_msg("  has_color\n"));
				}
				if (has_x_offset)
				{
					style.m_has_x_offset = true;
					style.m_x_offset = in->read_s16();
					IF_VERBOSE_PARSE(log_msg("  has_x_offset = %g\n", style.m_x_offset));
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
					IF_VERBOSE_PARSE(log_msg("  has_y_offset = %g\n", style.m_y_offset));
				}
				else
				{
					style.m_has_y_offset = false;
					style.m_y_offset = 0.0f;
				}
				if (has_font)
				{
					style.m_text_height = in->read_u16();
					IF_VERBOSE_PARSE(log_msg("  text_height = %g\n", style.m_text_height));
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

				IF_VERBOSE_PARSE(log_msg("  glyph_records: count = %d\n", glyph_count));
			}
		}
	}

	void text_character_def::display(character* inst)
	{
// 	        GNASH_REPORT_FUNCTION;
		display_glyph_records(m_matrix, inst,
			m_text_glyph_records, m_root_def);
	}


	/// Read a DefineText tag.
	void	define_text_loader(stream* in, int tag_type, movie_definition* m)
	{
		assert(tag_type == 11 || tag_type == 33);

		Uint16	character_id = in->read_u16();
		
		text_character_def*	ch = new text_character_def(m);
		IF_VERBOSE_PARSE(log_msg("text_character, id = %d\n", character_id));
		ch->read(in, tag_type, m);

		// IF_VERBOSE_PARSE(print some stuff);

		m->add_character(character_id, ch);
	}


	//
	// edit_text_character_def
	//

	void
	edit_text_character_def::read(stream* in, int tag_type,
			movie_definition* m)
	{
		assert(m != NULL);
		assert(tag_type == 37);

		m_rect.read(in);

		in->align();
		bool	has_text = in->read_uint(1) ? true : false;
		m_word_wrap = in->read_uint(1) ? true : false;
		m_multiline = in->read_uint(1) ? true : false;
		m_password = in->read_uint(1) ? true : false;
		m_readonly = in->read_uint(1) ? true : false;
		bool	has_color = in->read_uint(1) ? true : false;
		bool	has_max_length = in->read_uint(1) ? true : false;
		bool	has_font = in->read_uint(1) ? true : false;

		in->read_uint(1);	// reserved
		m_auto_size = in->read_uint(1) ? true : false;
		bool	has_layout = in->read_uint(1) ? true : false;
		m_no_select = in->read_uint(1) ? true : false;
		m_border = in->read_uint(1) ? true : false;
		in->read_uint(1);	// reserved
		m_html = in->read_uint(1) ? true : false;
		m_use_outlines = in->read_uint(1) ? true : false;

		if (has_font)
		{
			m_font_id = in->read_u16();
			m_text_height = (float) in->read_u16();
		}

		if (has_color)
		{
			m_color.read_rgba(in);
		}

		if (has_max_length)
		{
			m_max_length = in->read_u16();
		}

		if (has_layout)
		{
			m_alignment = (alignment) in->read_u8();
			m_left_margin = (float) in->read_u16();
			m_right_margin = (float) in->read_u16();
			m_indent = (float) in->read_s16();
			m_leading = (float) in->read_s16();
		}

		char*	name = in->read_string();
		m_default_name = name;
		delete [] name;

		if (has_text)
		{
			char*	str = in->read_string();
			m_default_text = str;
			delete [] str;
		}

		IF_VERBOSE_PARSE(log_msg("edit_text_char, varname = %s, text = %s\n",
					 m_default_name.c_str(), m_default_text.c_str()));
	}



	//
	// edit_text_character
	//

	void
	edit_text_character::set_text_value(const char* new_text)
	{
		if (m_text == new_text)
		{
			return;
		}

		m_text = new_text;
		if (m_def->m_max_length > 0
		    && m_text.length() > m_def->m_max_length)
		{
			m_text.resize(m_def->m_max_length);
		}

		format_text();
	}

	void
	edit_text_character::set_member(const tu_stringi& name,
			const as_value& val)
	{
		// @@ TODO need to inherit basic stuff like _x, _y, _xscale, _yscale etc

		as_standard_member	std_member = get_standard_member(name);
		switch (std_member)
		{
		default:
		case M_INVALID_MEMBER:
			break;
		case M_TEXT:
			//if (name == "text")
		{
			int version = get_parent()->get_movie_definition()->get_version();
			set_text_value(val.to_tu_string_versioned(version).c_str());
			return;
		}
		case M_X:
			//else if (name == "_x")
		{
			matrix	m = get_matrix();
			m.m_[0][2] = (float) PIXELS_TO_TWIPS(val.to_number());
			set_matrix(m);

			// m_accept_anim_moves = false;
			
			return;
		}
		case M_Y:
			//else if (name == "_y")
		{
			matrix	m = get_matrix();
			m.m_[1][2] = (float) PIXELS_TO_TWIPS(val.to_number());
			set_matrix(m);

			// m_accept_anim_moves = false;
			
			return;
		}
		case M_VISIBLE:
			//else if (name == "_visible")
		{
			set_visible(val.to_bool());
			return;
		}
		case M_ALPHA:
			//else if (name == "_alpha")
		{
			// @@ TODO this should be generic to struct character!
			// Arg is in percent.
			cxform	cx = get_cxform();
			cx.m_[3][0] = fclamp(float(val.to_number()) / 100.f, 0, 1);
			set_cxform(cx);
			return;
		}
		case M_TEXTCOLOR:
			//else if (name == "textColor")
		{	
			// The arg is 0xRRGGBB format.
			Uint32	rgb = (Uint32) val.to_number();

			cxform	cx = get_cxform();
			cx.m_[0][0] = fclamp(((rgb >> 16) & 255) / 255.0f, 0, 1);
			cx.m_[1][0] = fclamp(((rgb >>  8) & 255) / 255.0f, 0, 1);
			cx.m_[2][0] = fclamp(((rgb      ) & 255) / 255.0f, 0, 1);
			set_cxform(cx);

			return;
		}
		// @@ TODO see TextField members in Flash MX docs
		}	// end switch
	}

	bool
	edit_text_character::get_member(const tu_stringi& name, as_value* val)
	{
		as_standard_member	std_member = get_standard_member(name);
		switch (std_member)
		{
		default:
		case M_INVALID_MEMBER:
			break;
		case M_TEXT:
			//if (name == "text")
		{
			val->set_tu_string(m_text);
			return true;
		}
		case M_VISIBLE:
			//else if (name == "_visible")
		{
			val->set_bool(get_visible());
			return true;
		}
		case M_ALPHA:
			//else if (name == "_alpha")
		{
			// @@ TODO this should be generic to struct character!
			const cxform&	cx = get_cxform();
			val->set_double(cx.m_[3][0] * 100.f);
			return true;
		}
		case M_TEXTCOLOR:
			//else if (name == "textColor")
		{
			// Return color in 0xRRGGBB format
			const cxform&	cx = get_cxform();
			int	r = iclamp(int(cx.m_[0][0] * 255), 0, 255);
			int	g = iclamp(int(cx.m_[0][0] * 255), 0, 255);
			int	b = iclamp(int(cx.m_[0][0] * 255), 0, 255);
			val->set_int((r << 16) + (g << 8) + b);
			return true;
		}
		case M_X:
			//else if (name == "_x")
		{
			matrix	m = get_matrix();	// @@ get_world_matrix()???
			val->set_double(TWIPS_TO_PIXELS(m.m_[0][2]));
			return true;
		}
		case M_Y:
			//else if (name == "_y")
		{
			matrix	m = get_matrix();	// @@ get_world_matrix()???
			val->set_double(TWIPS_TO_PIXELS(m.m_[1][2]));
			return true;
		}
		case M_WIDTH:
			//else if (name == "_width")
		{
			// @@ TODO should implement this in
			// character and inherit into both here and sprite_instance
			rect	transformed_rect;
			transformed_rect.enclose_transformed_rect(get_world_matrix(), m_def->m_rect);
			val->set_double(TWIPS_TO_PIXELS(transformed_rect.width()));
			return true;
		}
		case M_HEIGHT:
			//else if (name == "_height")
		{
			// @@ TODO should implement this in
			// character and inherit into both here and sprite_instance
			rect	transformed_rect;
			transformed_rect.enclose_transformed_rect(get_world_matrix(), m_def->m_rect);
			val->set_double(TWIPS_TO_PIXELS(transformed_rect.height()));
			return true;
		}
		case M_TEXTWIDTH:
			//else if (name == "textWidth")
		{
			// Return the width, in pixels, of the text as laid out.
			// (I.e. the actual text content, not our defined
			// bounding box.)
			//
			// In local coords.  Verified against Macromedia Flash.
			val->set_double(TWIPS_TO_PIXELS(m_text_bounding_box.width()));

			return true;
		}
		}	// end switch

		return false;
	}
		
	// @@ WIDTH_FUDGE is a total fudge to make it match the Flash player!  Maybe
	// we have a bug?
	#define WIDTH_FUDGE 80.0f


	void
	edit_text_character::align_line(
			edit_text_character_def::alignment align,
			int last_line_start_record, float x)
	{
		float	extra_space = (m_def->m_rect.width() -
				m_def->m_right_margin) - x - WIDTH_FUDGE;
		assert(extra_space >= 0.0f);

		float	shift_right = 0.0f;

		if (align == edit_text_character_def::ALIGN_LEFT)
		{
			// Nothing to do; already aligned left.
			return;
		}
		else if (align == edit_text_character_def::ALIGN_CENTER)
		{
			// Distribute the space evenly on both sides.
			shift_right = extra_space / 2;
		}
		else if (align == edit_text_character_def::ALIGN_RIGHT)
		{
			// Shift all the way to the right.
			shift_right = extra_space;
		}

		// Shift the beginnings of the records on this line.
		for (unsigned int i = last_line_start_record; i < m_text_glyph_records.size(); i++)
		{
			text_glyph_record&	rec = m_text_glyph_records[i];

			if (rec.m_style.m_has_x_offset)
			{
				rec.m_style.m_x_offset += shift_right;
			}
		}
	}

	void
	edit_text_character::format_text()
	{
		m_text_glyph_records.resize(0);

		if (m_def->m_font == NULL)
		{
			return;
		}

		// @@ mostly for debugging
		// Font substitution -- if the font has no
		// glyphs, try some other defined font!
		if (m_def->m_font->get_glyph_count() == 0)
		{
			// Find a better font.
			font*	newfont = m_def->m_font;
			for (int i = 0, n = fontlib::get_font_count(); i < n; i++)
			{
				font*	f = fontlib::get_font(i);
				assert(f);

				if (f->get_glyph_count() > 0)
				{
					// This one looks good.
					newfont = f;
					break;
				}
			}

			if (m_def->m_font != newfont)
			{
				log_error("error: substituting font!  font '%s' has no glyphs, using font '%s'\n",
					  fontlib::get_font_name(m_def->m_font),
					  fontlib::get_font_name(newfont));

				m_def->m_font = newfont;
			}
		}


		float	scale = m_def->m_text_height / 1024.0f;	// the EM square is 1024 x 1024

		text_glyph_record	rec;	// one to work on
		rec.m_style.m_font = m_def->m_font;
		rec.m_style.m_color = m_def->m_color;
		rec.m_style.m_x_offset = fmax(0, m_def->m_left_margin + m_def->m_indent);
		rec.m_style.m_y_offset = m_def->m_text_height
			+ (m_def->m_font->get_leading() - m_def->m_font->get_descent()) * scale;
		rec.m_style.m_text_height = m_def->m_text_height;
		rec.m_style.m_has_x_offset = true;
		rec.m_style.m_has_y_offset = true;

		float	x = rec.m_style.m_x_offset;
		float	y = rec.m_style.m_y_offset;

		// Start the bbox at the upper-left corner of the first glyph.
		reset_bounding_box(x, y - m_def->m_font->get_descent() * scale + m_def->m_text_height);

		float	leading = m_def->m_leading;
		leading += m_def->m_font->get_leading() * scale;

		int	last_code = -1;
		int	last_space_glyph = -1;
		int	last_line_start_record = 0;

		const char*	text = &m_text[0];
		while (Uint32 code = utf8::decode_next_unicode_character(&text))
		{
// @@ try to truncate overflow text??
#if 0
			if (y + m_def->m_font->get_descent() * scale > m_def->m_rect.height())
			{
				// Text goes below the bottom of our bounding box.
				rec.m_glyphs.resize(0);
				break;
			}
#endif // 0

			//Uint16	code = m_text[j];

			x += m_def->m_font->get_kerning_adjustment(last_code, (int) code) * scale;
			last_code = (int) code;

			// Expand the bounding-box to the lower-right corner of each glyph as
			// we generate it.
			m_text_bounding_box.expand_to_point(x, y + m_def->m_font->get_descent() * scale);

			if (code == 13 || code == 10)
			{
				// newline.

				// Frigging Flash seems to use '\r' (13) as its
				// default newline character.  If we get DOS-style \r\n
				// sequences, it'll show up as double newlines, so maybe we
				// need to detect \r\n and treat it as one newline.

				// Close out this stretch of glyphs.
				m_text_glyph_records.push_back(rec);
				align_line(m_def->m_alignment, last_line_start_record, x);

				x = fmax(0, m_def->m_left_margin + m_def->m_indent);	// new paragraphs get the indent.
				y += m_def->m_text_height + leading;

				// Start a new record on the next line.
				rec.m_glyphs.resize(0);
				rec.m_style.m_font = m_def->m_font;
				rec.m_style.m_color = m_def->m_color;
				rec.m_style.m_x_offset = x;
				rec.m_style.m_y_offset = y;
				rec.m_style.m_text_height = m_def->m_text_height;
				rec.m_style.m_has_x_offset = true;
				rec.m_style.m_has_y_offset = true;

				last_space_glyph = -1;
				last_line_start_record = m_text_glyph_records.size();

				continue;
			}

			if (code == 8)
			{
				// backspace (ASCII BS).

				// This is a limited hack to enable overstrike effects.
				// It backs the cursor up by one character and then continues
				// the layout.  E.g. you can use this to display an underline
				// cursor inside a simulated text-entry box.
				//
				// ActionScript understands the '\b' escape sequence
				// for inserting a BS character.
				//
				// ONLY WORKS FOR BACKSPACING OVER ONE CHARACTER, WON'T BS
				// OVER NEWLINES, ETC.

				if (rec.m_glyphs.size() > 0)
				{
					// Peek at the previous glyph, and zero out its advance
					// value, so the next char overwrites it.
					float	advance = rec.m_glyphs.back().m_glyph_advance;
					x -= advance;	// maintain formatting
					rec.m_glyphs.back().m_glyph_advance = 0;	// do the BS effect
				}
				continue;
			}

			// Remember where word breaks occur.
			if (code == 32)
			{
				last_space_glyph = rec.m_glyphs.size();
			}

			int	index = m_def->m_font->get_glyph_index((Uint16) code);
			if (index == -1)
			{
				// error -- missing glyph!
				
				// Log an error, but don't log too many times.
				static int	s_log_count = 0;
				if (s_log_count < 10)
				{
					s_log_count++;
					log_warning("%s\n\t -- missing glyph for char %d\n"
						    "-- make sure character shapes for font %s are being exported "
						  "into your SWF file!\n",
						    __PRETTY_FUNCTION__,
						    code,
						    m_def->m_font->get_name());
				}

				// Drop through and use index == -1; this will display
				// using the empty-box glyph
			}
			text_glyph_record::glyph_entry	ge;
			ge.m_glyph_index = index;
			ge.m_glyph_advance = scale * m_def->m_font->get_advance(index);

			rec.m_glyphs.push_back(ge);

			x += ge.m_glyph_advance;

			
			if (x >= m_def->m_rect.width() - m_def->m_right_margin - WIDTH_FUDGE)
			{
				// Whoops, we just exceeded the box width.  Do word-wrap.

				// Insert newline.

				// Close out this stretch of glyphs.
				m_text_glyph_records.push_back(rec);
				float	previous_x = x;

				x = m_def->m_left_margin;
				y += m_def->m_text_height + leading;

				// Start a new record on the next line.
				rec.m_glyphs.resize(0);
				rec.m_style.m_font = m_def->m_font;
				rec.m_style.m_color = m_def->m_color;
				rec.m_style.m_x_offset = x;
				rec.m_style.m_y_offset = y;
				rec.m_style.m_text_height = m_def->m_text_height;
				rec.m_style.m_has_x_offset = true;
				rec.m_style.m_has_y_offset = true;
				
				text_glyph_record&	last_line = m_text_glyph_records.back();
				if (last_space_glyph == -1)
				{
					// Pull the previous glyph down onto the
					// new line.
					if (last_line.m_glyphs.size() > 0)
					{
						rec.m_glyphs.push_back(last_line.m_glyphs.back());
						x += last_line.m_glyphs.back().m_glyph_advance;
						previous_x -= last_line.m_glyphs.back().m_glyph_advance;
						last_line.m_glyphs.resize(last_line.m_glyphs.size() - 1);
					}
				}
				else
				{
					// Move the previous word down onto the next line.

					previous_x -= last_line.m_glyphs[last_space_glyph].m_glyph_advance;

					for (unsigned int i = last_space_glyph + 1; i < last_line.m_glyphs.size(); i++)
					{
						rec.m_glyphs.push_back(last_line.m_glyphs[i]);
						x += last_line.m_glyphs[i].m_glyph_advance;
						previous_x -= last_line.m_glyphs[i].m_glyph_advance;
					}
					last_line.m_glyphs.resize(last_space_glyph);
				}

				align_line(m_def->m_alignment, last_line_start_record, previous_x);

				last_space_glyph = -1;
				last_line_start_record = m_text_glyph_records.size();
			}

			// TODO: HTML markup
		}

		// Add this line to our output.
		m_text_glyph_records.push_back(rec);
		align_line(m_def->m_alignment, last_line_start_record, x);
	}

	void
	edit_text_character::display()
	{
		GNASH_REPORT_FUNCTION;

		if (m_def->m_border)
		{
			matrix	mat = get_world_matrix();
			
			// @@ hm, should we apply the color xform?  It seems logical; need to test.
			// cxform	cx = get_world_cxform();

			// Show white background + black bounding box.
			render::set_matrix(mat);

			point	coords[4];
			coords[0] = m_def->m_rect.get_corner(0);
			coords[1] = m_def->m_rect.get_corner(1);
			coords[2] = m_def->m_rect.get_corner(3);
			coords[3] = m_def->m_rect.get_corner(2);

			Sint16	icoords[18] = 
			{
				// strip (fill in)
				(Sint16) coords[0].m_x, (Sint16) coords[0].m_y,
				(Sint16) coords[1].m_x, (Sint16) coords[1].m_y,
				(Sint16) coords[2].m_x, (Sint16) coords[2].m_y,
				(Sint16) coords[3].m_x, (Sint16) coords[3].m_y,

				// outline
				(Sint16) coords[0].m_x, (Sint16) coords[0].m_y,
				(Sint16) coords[1].m_x, (Sint16) coords[1].m_y,
				(Sint16) coords[3].m_x, (Sint16) coords[3].m_y,
				(Sint16) coords[2].m_x, (Sint16) coords[2].m_y,
				(Sint16) coords[0].m_x, (Sint16) coords[0].m_y,
			};
			
			render::fill_style_color(0, rgba(255, 255, 255, 255));
			render::draw_mesh_strip(&icoords[0], 4);

			render::line_style_color(rgba(0,0,0,255));
			render::draw_line_strip(&icoords[8], 5);
		}

		// Draw our actual text.
		display_glyph_records(matrix::identity, this, m_text_glyph_records, m_def->m_root_def);

		do_display_callback();
	}

	//
	// edit_text_character_def
	//

	character*	edit_text_character_def::create_character_instance(movie* parent, int id)
	{
		if (m_font == NULL)
		{
			// Resolve the font, if possible.
			m_font = m_root_def->get_font(m_font_id);
			if (m_font == NULL)
			{
				log_error("error: text style with undefined font; font_id = %d\n", m_font_id);
			}
		}

		edit_text_character*	ch = new edit_text_character(parent, this, id);
		ch->set_name(m_default_name.c_str());
		return ch;
	}


	void	define_edit_text_loader(stream* in, int tag_type, movie_definition* m)
	// Read a DefineText tag.
	{
		assert(tag_type == 37);

		Uint16	character_id = in->read_u16();

		edit_text_character_def*	ch = new edit_text_character_def(m);
		IF_VERBOSE_PARSE(log_msg("edit_text_char, id = %d\n", character_id));
		ch->read(in, tag_type, m);

		m->add_character(character_id, ch);
	}

}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

