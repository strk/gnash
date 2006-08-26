// text.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code for the text tags.


#include "stream.h"
#include "log.h"
#include "movie_definition.h" // for m_root_def use

#include "edit_text_character_def.h"
#include "edit_text_character.h"

namespace gnash {

// Forward declarations
class movie_definition;

void
edit_text_character_def::read(stream* in, int tag_type,
		movie_definition* m)
{
	assert(m != NULL);
	assert(tag_type == SWF::DEFINEEDITTEXT); // 37

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
		m_text_height = in->read_u16();
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
		//m_left_margin = (float) in->read_u16();
		m_left_margin = in->read_u16();
		//m_right_margin = (float) in->read_u16();
		m_right_margin = in->read_u16();
		m_indent = in->read_s16();
		m_leading = in->read_s16();
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

	IF_VERBOSE_PARSE (
		log_parse("edit_text_char:\n"
			" default varname = %s\n"
			" text = ``%s''\n"
			" font_id: %d\n"
			" text_height: %d",
			m_default_name.c_str(),
			m_default_text.c_str(),
			m_font_id,
			m_text_height);
	);
}

const font*
edit_text_character_def::get_font() 
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

	return m_font;
}

character*
edit_text_character_def::create_character_instance(character* parent,
		int id)
{
	// Resolve the font, if possible
	get_font();
	edit_text_character* ch = new edit_text_character(parent, this, id);

	ch->set_name(m_default_name.c_str());

	return ch;
}


} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

