// edit_text_character_def.cpp:  Text tags, for Gnash.
//
//   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: edit_text_character_def.cpp,v 1.10 2007/07/01 10:54:34 bjacques Exp $ */

// Based on the public domain text.cpp of Thatcher Ulrich <tu@tulrich.com> 2003

#include "stream.h"
#include "log.h"
#include "movie_definition.h" // for m_root_def use

#include "edit_text_character_def.h"
#include "edit_text_character.h"
#include "font.h" // for setReachable call

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

	in->read_string(m_variable_name);

	if (has_text)
	{
		in->read_string(m_default_text);
	}

	IF_VERBOSE_PARSE (
		log_parse("edit_text_char:\n"
			" varname = %s\n"
			" text = ``%s''\n"
			" font_id: %d\n"
			" text_height: %d",
			m_variable_name.c_str(),
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
			log_error(_("error: text style with undefined font; font_id = %d"), m_font_id);
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

	// This gives an "instance name" to the TextField, but
	// it is not really what we need.
	//
	// First of all the VariableName ("m_variable_name") is
	// NOT the default name of an instance, rather it is
	// a variable associated with it and can contain path
	// information (ie. we can associate a variable in a different
	// timeline)
	//
	// We actually need to set that variable to an object which
	// is a TextField dinamic variable. The object should take
	// care of updating this TextField text when assigned to
	// and to retrive this TextField text when extracted value from.
	//
	// The DefineEditTextVariableNameTest.swf file under
	// testsuite/misc-ming.all gives an idea of the problem
	// (in particular it shows a case in which VariableName is
	// outside of TextField timeline/scope)
	//
	//ch->set_name(m_variable_name.c_str());

	return ch;
}

#ifdef GNASH_USE_GC
void
edit_text_character_def::markReachableResources() const
{
	if ( m_root_def ) m_root_def->setReachable();
	if ( m_font ) m_font->setReachable();
}
#endif // GNASH_USE_GC

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
