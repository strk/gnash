// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//


#ifndef _GNASH_EDIT_TEXT_CHARACTER_DEF_H_
#define _GNASH_EDIT_TEXT_CHARACTER_DEF_H_

#include "character_def.h" // for inheritance
#include "rect.h" // for composition
#include "textformat.h" // for composition

namespace gnash {

// Forward declarations
class movie_definition;

/// \brief
/// A definition for a text display character, whose text can
/// be changed at runtime (by script or host).
/// This object is defined by SWF tag 37 (SWF::DEFINEEDITTEXT)
///
class edit_text_character_def : public character_def
{
public:

	/// Text alignment values
	enum alignment
	{
		ALIGN_LEFT = 0,
		ALIGN_RIGHT,
		ALIGN_CENTER,
		/// probably don't need to implement...
		ALIGN_JUSTIFY
	};

	edit_text_character_def(movie_definition* root_def)
		:
		m_root_def(root_def),
		m_format(),
		m_word_wrap(false),
		m_multiline(false),
		m_password(false),
		m_readonly(false),
		m_auto_size(false),
		m_no_select(false),
		m_border(false),
		m_html(false),
		m_use_outlines(false), // For an SWF-defined textfield we'll read
		                       // this from the tag. Dynamic textfields should
		                       // use device fonts by default (so not use outline ones)
		m_font_id(-1),
		m_font(NULL),
		m_text_height(1), // TODO: initialize to a meaningful value (see sprite_instance::add_textfield)
		                  //       and make sure get_font_height is not called for rendering purposes
		                  //       (instead call a method of edit_text_character_def)
		m_max_length(0),
		m_alignment(ALIGN_LEFT),
		m_left_margin(0),
		m_right_margin(0),
		m_indent(0),
		m_leading(0)
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

	/// Get width of this definition in twips (by definition)
	float width() const { return m_rect.width(); }

	/// Get height of this definition in twips (by definition)
	float height() const { return m_rect.height(); }

	/// Create an instance of this character
	character* create_character_instance(character* parent, int id);

	/// Initialize from SWF input stream (tag 37)
	void read(stream* in, int tag_type, movie_definition* m);

	/// Return a reference to the default text associated
	/// with this EditText definition.
	const std::string& get_default_text() const {
		return m_default_text;
	}

	/// Return a reference to the "VariableName" associated
	/// with this EditText definition. The variable name
	/// is allowed to contain path information and should
	/// be used to provide an 'alias' to the 'text' member
	/// of instances.
	const std::string& get_variable_name() const {
		return m_variable_name;
	}

	/// Return the maximum length of text this widget can hold.
	//
	/// If zero, the text length is unlimited.
	///
	unsigned int get_max_length() const {
		return m_max_length;
	}

	/// Get boundaries of this movie
	//
	/// Return a reference to private space, copy
	/// it if you need it  for longer then this
	/// object's lifetime.
	///
	const rect& get_bounds() const {
		return m_rect;
	}

	/// Set boundaries of this textfield
	//
	/// This method is used for dynamic textfields
	/// (actionscript created)
	///
	void set_bounds(const rect& bounds)
	{
		m_rect = bounds;
	}

	/// Get right margin in twips
	uint16_t get_right_margin() const {
		return m_right_margin;
	}

	/// Get left margin in twips
	uint16_t get_left_margin() const {
		return m_left_margin;
	}

	/// Get indentation in  twips
	uint16_t get_indent() const {
		return m_indent;
	}

	/// Get height of font  in twips.
	// @@ what if has_font is false ??
	uint16_t get_font_height() const {
		return m_text_height;
	}

	/// Set height of font  in twips.
	// 
	/// Used by dynamically created textfields.
	///
	void set_font_height(uint16_t h) {
		m_text_height = h;
	}

	/// Get font.
	//
	/// Note: use add_ref() on the return if you need to keep
	/// it alive after this definition gets destructed.
	///
	const font* get_font();

	/// Get color of the text
	const rgba& get_text_color() const {
		return m_color;
	}

	/// Set color of the text
	void set_text_color(const rgba& col) {
		m_color = col;
	}

	/// \brief
	/// Get extra space between lines (in twips).
	//
	/// This is in addition to default font line spacing.
	uint16_t get_leading() const {
		return m_leading;
	}

	/// Get text alignment
	alignment get_alignment() const {
		return m_alignment;
	}

	/// Is border requested ?
	bool has_border() const {
		return m_border;
	}

	/// Word wrap requested ?
	bool do_word_wrap() const {
		return m_word_wrap;
	}

	/// Get root movie definition
	movie_definition* get_root_def() {
		return m_root_def;
	}

	bool get_readonly() const
	{
		return m_readonly;
	}
	
	bool get_no_select() const 
	{
	  return m_no_select;
	}
	
	const rect&	get_bound() const
	{
		// I know it's stupid to have an alias that's nearly the same name but
		// get_bound() is required by the base class and get_bounds() was already
		// there. Should be fixed (remove get_bounds). 
		return get_bounds(); 
	}

	/// Return true if HTML was allowed by definition
	bool htmlAllowed() const { return m_html; }

	/// Return true if this character definition requested use of device fonts
	// 
	/// Used by edit_text_character constructor to set it's default.
	///
	bool getUseEmbeddedGlyphs() const 
	{
		return m_use_outlines;
	}
	
protected:

#ifdef GNASH_USE_GC
	/// Mark all reachable resources (for GC)
	//
	/// Reachable resources are:
	///  - The root movie definition (m_root_def) @@ what do we use this for ?
	///  - The font being used (m_font) 
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:

	/// Root movie_definition
	movie_definition*	m_root_def;

	rect			m_rect;
	std::string		m_variable_name;
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
	/// When true, use specified SWF internal font (embed fonts)
	/// Otherwise, use specified device font (or a default one if m_font_id is -1)
	///
	/// Also known as USE_GLYPH (from Ming)
	///
	bool	m_use_outlines;

	int	m_font_id;
	font*	m_font;

	/// height of font text, in twips
	uint16_t m_text_height;

	/// Text color
	rgba	m_color;

	/// Maximum length of text this widget can display (number of chars?)
	//
	/// If zero, the text length is unlimited.
	///
	unsigned int m_max_length;

	alignment m_alignment;
	
	/// extra space between box's left border and text (in twips)
	uint16_t m_left_margin;

	/// extra space between box's right border and text (in twips)
	uint16_t m_right_margin;

	/// how much to indent the first line of multiline text (in twips)
	uint16_t	m_indent;

	/// \brief
	/// Extra space between lines
	/// (in addition to default font line spacing)
	uint16_t	m_leading;

	/// The default text to be displayed
	std::string	m_default_text;

};

} // namespace gnash

#endif // _GNASH_EDIT_TEXT_CHARACTER_DEF_H_
