// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Code for the text tags.


#ifndef GNASH_TEXT_H
#define GNASH_TEXT_H

#include "textformat.h" // maybe we should include it here
#include "styles.h" 

namespace gnash {

	// Forward declarations
	struct text_character_def; 
	struct text_glyph_record; 
	struct text_style; 

	// Helper struct.
	// @@ text_character_def friend ?
	struct text_style
	{
		int	m_font_id;
		mutable font*	m_font;
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

		void	resolve_font(movie_definition_sub* root_def) const;
	};


	// Helper struct.
	// @@ text_character_def friend ?
	struct text_glyph_record
	{
		struct glyph_entry
		{
			int	m_glyph_index;
			float	m_glyph_advance;
		};
		text_style	m_style;
		array<glyph_entry>	m_glyphs;

		void read(stream* in, int glyph_count,
			int glyph_bits, int advance_bits);
	};

	/// Text character 
	//
	/// This is either read from SWF stream 
	/// or (hopefully) created with scripting
	///
	struct text_character_def : public character_def
	{
		movie_definition_sub*	m_root_def;
		rect	m_rect;
		matrix	m_matrix;
		array<text_glyph_record>	m_text_glyph_records;

		text_character_def(movie_definition_sub* root_def)
			:
			m_root_def(root_def)
		{
			assert(m_root_def);
		}

		void read(stream* in, int tag_type, movie_definition_sub* m);

		/// Draw the string.
		void display(character* inst);

	};

	/// \brief
	/// A definition for a text display character, whose text can
	/// be changed at runtime (by script or host).
	/// This object is defined by SWF tag 37.
	///
	struct edit_text_character_def : public character_def
	{
		movie_definition_sub*	m_root_def;
		rect			m_rect;
		tu_string		m_default_name;
		text_format		m_format;
		bool			m_word_wrap;
		bool			m_multiline;
		/// show asterisks instead of actual characters
		bool			m_password;
		bool			m_readonly;
		/// resize our bound to fit the text
		bool			m_auto_size;
		bool			m_no_select;

		/// forces white background and black border.
		/// silly, but sometimes used
		bool			m_border;

		/// Allowed HTML (from Alexi's SWF Reference).
		//
		/// <a href=url target=targ>...</a> -- hyperlink
		/// <b>...</b> -- bold
		/// <br> -- line break
		/// <font face=name size=[+|-][0-9]+ color=#RRGGBB>...</font>  -- font change; size in TWIPS
		/// <i>...</i> -- italic
		/// <li>...</li> -- list item
		/// <p>...</p> -- paragraph
		/// <tab> -- insert tab
		/// <TEXTFORMAT>  </TEXTFORMAT>
		///   [ BLOCKINDENT=[0-9]+ ]
		///   [ INDENT=[0-9]+ ]
		///   [ LEADING=[0-9]+ ]
		///   [ LEFTMARGIN=[0-9]+ ]
		///   [ RIGHTMARGIN=[0-9]+ ]
		///   [ TABSTOPS=[0-9]+{,[0-9]+} ]
		///
		/// Change the different parameters as indicated. The
		/// sizes are all in TWIPs. There can be multiple
		/// positions for the tab stops. These are seperated by
		/// commas.
		/// <U>...</U> -- underline
		///
		bool			m_html;



		/// \brief
		/// When true, use specified SWF internal font. 
		/// Otherwise, renderer picks a default font
		bool	m_use_outlines;

		int	m_font_id;
		font*	m_font;
		float	m_text_height;

		rgba	m_color;
		int	m_max_length;

		enum alignment
		{
			ALIGN_LEFT = 0,
			ALIGN_RIGHT,
			ALIGN_CENTER,
			/// probably don't need to implement...
			ALIGN_JUSTIFY
		};
		alignment	m_alignment;
		
		/// extra space between box border and text
		float	m_left_margin;

		float	m_right_margin;

		/// how much to indent the first line of multiline text
		float	m_indent;

		/// \brief
		/// Extra space between lines
		/// (in addition to default font line spacing)
		float	m_leading;
		tu_string	m_default_text;

		edit_text_character_def(movie_definition_sub* root_def)
			:
			m_root_def(root_def),
			m_word_wrap(false),
			m_multiline(false),
			m_password(false),
			m_readonly(false),
			m_auto_size(false),
			m_no_select(false),
			m_border(false),
			m_html(false),
			m_use_outlines(false),
			m_font_id(-1),
			m_font(NULL),
			m_text_height(1.0f),
			m_max_length(0),
			m_alignment(ALIGN_LEFT),
			m_left_margin(0.0f),
			m_right_margin(0.0f),
			m_indent(0.0f),
			m_leading(0.0f)
		{
			assert(m_root_def);

			m_color.set(0, 0, 0, 255);
		}

		/// Set the format of the text
		void	set_format(text_format &format)
		{
			m_format = format;
		}
		
		~edit_text_character_def()
		{
		}


		character* create_character_instance(movie* parent, int id);


		/// Initialize from SWF input stream (tag 37)
		void read(stream* in, int tag_type, movie_definition_sub* m);
	};

	/// ...
	struct edit_text_character : public character
	{
		edit_text_character_def*	m_def;
		array<text_glyph_record>	m_text_glyph_records;

		/// used to pass a color on to shape_character::display()
		array<fill_style>	m_dummy_style;

		array<line_style>	m_dummy_line_style;

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

#endif // GNASH_TEXT_H
