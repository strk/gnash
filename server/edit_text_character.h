// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

// Code for the text tags.


#ifndef _GNASH_EDIT_TEXT_CHARACTER_H_
#define _GNASH_EDIT_TEXT_CHARACTER_H_

#include "character.h" // for inheritance
#include "edit_text_character_def.h" // for inlines
#include "styles.h" // for fill_style and line_style
#include "text.h" // for text_glyph_record

namespace gnash {

// Forward declarations
struct text_character_def; 
struct text_glyph_record; 

/// An instance of an edit_text_character_def (I presume)
struct edit_text_character : public character
{
	edit_text_character_def*	m_def;
	std::vector<text_glyph_record>	m_text_glyph_records;

	/// used to pass a color on to shape_character::display()
	std::vector<fill_style>	m_dummy_style;

	std::vector<line_style>	m_dummy_line_style;

	/// bounds of dynamic text, as laid out
	rect	m_text_bounding_box;

	tu_string	m_text;

	edit_text_character(movie* parent, edit_text_character_def* def, int id)
		:
		character(parent, id),
		m_def(def)
	{
		assert(parent);
		assert(m_def);

		set_text_value(m_def->m_default_text.c_str());

		m_dummy_style.push_back(fill_style());

		reset_bounding_box(0, 0);
	}

	~edit_text_character()
	{
	}

	virtual const char* get_text_name() const { return m_def->m_default_name.c_str(); }


	/// Reset our text bounding box to the given point.
	void	reset_bounding_box(float x, float y)
	{
		m_text_bounding_box.m_x_min = x;
		m_text_bounding_box.m_x_max = x;
		m_text_bounding_box.m_y_min = y;
		m_text_bounding_box.m_y_max = y;
	}


	/// Set our text to the given string.
	virtual void	set_text_value(const char* new_text);

	virtual const char*	get_text_value() const
	{
		return m_text.c_str();
	}


	/// We have a "text" member.
	void set_member(const tu_stringi& name, const as_value& val);


	bool get_member(const tu_stringi& name, as_value* val);


	/// Does LEFT/CENTER/RIGHT alignment on the records in
	/// m_text_glyph_records[], starting with
	/// last_line_start_record and going through the end of
	/// m_text_glyph_records.
	void align_line(edit_text_character_def::alignment align,
			int last_line_start_record, float x);


	/// Convert the characters in m_text into a series of
	/// text_glyph_records to be rendered.
	void	format_text();


	/// Draw the dynamic string.
	void	display();
};

} // namespace gnash

#endif // _GNASH_EDIT_TEXT_CHARACTER_H_
