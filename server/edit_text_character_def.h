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


#ifndef _GNASH_EDIT_TEXT_CHARACTER_DEF_H_
#define _GNASH_EDIT_TEXT_CHARACTER_DEF_H_

#include "character_def.h" // for inheritance
#include "gnash.h" // for composition (struct rect)
#include "textformat.h" // for composition

namespace gnash {

	// Forward declarations
	class movie_definition;

	/// \brief
	/// A definition for a text display character, whose text can
	/// be changed at runtime (by script or host).
	/// This object is defined by SWF tag 37.
	///
	struct edit_text_character_def : public character_def
	{
		movie_definition*	m_root_def;
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

		edit_text_character_def(movie_definition* root_def)
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
		void read(stream* in, int tag_type, movie_definition* m);
	};

} // namespace gnash

#endif // _GNASH_EDIT_TEXT_CHARACTER_DEF_H_
