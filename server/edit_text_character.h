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
//


#ifndef _GNASH_EDIT_TEXT_CHARACTER_H_
#define _GNASH_EDIT_TEXT_CHARACTER_H_

#include "character.h" // for inheritance
#include "edit_text_character_def.h" // for inlines
#include "styles.h" // for fill_style and line_style
#include "text.h" // for text_glyph_record
#include "Range2d.h"
#include "rect.h" // for inlines
#include "edit_text_character_def.h" // for ::alignment typedef


// Forward declarations
namespace gnash {
	class text_character_def; 
	class text_glyph_record; 
	//class sprite_instance;
}

namespace gnash {

/// An instance of an edit_text_character_def 
class edit_text_character : public character
{

public:

	/// Possible autoSize values
	enum AutoSizeValue {

		/// Do not automatically resize TextField as text grow/shrink
		autoSizeNone,

		/// Expand TextField, anchor the top-left side
		autoSizeLeft,

		/// Expand TextField, anchor the horizontal center
		autoSizeCenter,

		/// Expand TextField, anchor the top-right side
		autoSizeRight
	};

	edit_text_character(
			character* parent,
			edit_text_character_def* def,
			int id);

	~edit_text_character();

	// TODO: should this return isSelectable() ?
	bool can_handle_mouse_event() const { return true; }

	character* get_topmost_mouse_entity(float x, float y);
	
	bool wantsInstanceName()
	{
		return true; // text fields can be referenced 
	}	
		
	bool on_event(const event_id& id);	

	const char* get_variable_name() const
	{
		return _variable_name.c_str();
	}

	/// Set the name of a variable associated to this
	/// TextField's displayed text.
	//
	/// Calling this function will override any previous
	/// setting for the variable name.
	/// 
	void set_variable_name(const std::string& newname);

	/// Set our text to the given string.
	//
	/// This function will also update any registered variable
	///
	void	set_text_value(const char* new_text);

	/// Set our text to the given string by effect of an update of a registered variable name
	//
	/// This cal only updates the text and is only meant to be called by ourselves
	/// or by sprite_instance when a registered TextVariable is updated.
	///
	void updateText(const std::string& s);

 	/// Return value of our text.
	std::string get_text_value() const;

	/// We have a "text" member.
	void set_member(string_table::key name, const as_value& val, 
		string_table::key nsname = 0);

	bool get_member(string_table::key name, as_value* val, 
		string_table::key nsname = 0);

	/// Draw the dynamic string.
	void	display();

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	geometry::Range2d<float> getBounds() const
	{
		return _bounds;
	}

	// See dox in character.h
	bool pointInShape(float x, float y) const;

	/// See dox in character::unload (character.h)
	//
	/// NOTE: edit_text_character (TextField) never has
	///       an onUnload event, so we always return false
	///	  here. (TODO: verify this)
	///
	bool unload();

	/// Return true if the 'background' should be drawn
	bool getDrawBackground() const;

	/// Specify wheter to draw the background
	void setDrawBackground(bool draw);

	/// Return color of the background
	rgba getBackgroundColor() const;

	/// Set color of the background
	//
	/// Use setDrawBackground to actually use this value.
	///
	void setBackgroundColor(const rgba& col);

	/// Return true if this TextField should have it's border visible
	bool getDrawBorder() const;

	/// Specify wheter to draw the border
	void setDrawBorder(bool draw);

	/// Return color of the border
	rgba getBorderColor() const;

	/// Set color of the border
	//
	/// Use setDrawBorder to actually use this value.
	///
	void setBorderColor(const rgba& col);

	/// Return color of the text
	const rgba& getTextColor() const 
	{
		return _textColor;
	}

	/// Set color of the text
	void setTextColor(const rgba& col);

	/// \brief
	/// Return true if this TextField should use embedded font glyphs,
	/// false if it should use device font glyphs
	bool getEmbedFonts() const {
		return _embedFonts;
	}

	/// \brief
	/// Set whether this TextField should use embedded font glyphs,
	/// or use device font glyphs
	//
	/// @param use
	void setEmbedFonts(bool use);

	/// Get autoSize value 
	AutoSizeValue getAutoSize() const
	{
		return _autoSize;
	}

	/// Return text alignment
	edit_text_character_def::alignment getTextAlignment();

	/// Set autoSize value 
	//
	/// @param val
	/// 	The AutoSizeValue to use
	///
	void setAutoSize(AutoSizeValue val);

	/// Parse autoSize string value
	//
	/// @param val
	/// 	Auto size value as a string (one of none, left, center, right)
	///
	/// @return an AutoSizeValue identifier. autoSizeNone if invalid
	///
	static AutoSizeValue parseAutoSizeValue(const string& val);

	/// Return autoSize value as a string
	//
	/// @param val
	/// 	Auto size value 
	///
	/// @return a C-string representation of the autoSize value.
	///	The returns is *never* NULL.
	///
	static const char* autoSizeValueName(AutoSizeValue val);

	/// \brief
	/// Return true if text should continue to next available line
	/// when hitting end of bounding box.
	///
	bool doWordWrap() const {
		return _wordWrap;
	}

	/// Set wordWrap parameter 
	//
	/// @param on
	///	If true text hitting bounding box limits will continue
	///	to next line.
	///	If false, either text will be truncated or bounding box
	///	expanded, depending on autoSize (see getAutoSize)
	///
	void setWordWrap(bool on);

	/// \brief
	/// Return true if HTML markup in text should be rendered.
	///
	bool doHtml() const {
		return _html;
	}

	/// Set html parameter
	//
	/// @param on
	///	If true HTML tags in the text will be parsed and rendered
	void setHtml(bool on) {
		_html = on;
	}

	/// Return true if the TextField text is selectable
	bool isSelectable() const
	{
		return _selectable;
	}

	/// Set 'selectable' parameter
	void setSelectable(bool v) 
	{
		_selectable = v;
	}

	// See character::isActiveTextField
	bool virtual isSelectableTextField() const
	{
		return isSelectable();
	}

private:

	/// Call this function when willing to invoke the onChanged event handler
	void onChanged();

	/// The actual text
	std::string _text;

	/// immutable definition of this object, as read
	/// from the SWF stream. Assured to be not-NULL
	/// by constructor. This might change in the future
	boost::intrusive_ptr<edit_text_character_def>	m_def;

	/// bounds of dynamic text, as laid out
	geometry::Range2d<float> m_text_bounding_box;

	/// Reset our text bounding box to the given point.
	void	reset_bounding_box(float x, float y)
	{
		m_text_bounding_box.setTo(x,y);
	}

	typedef std::vector<text_glyph_record> TextGlyphRecords;
	TextGlyphRecords m_text_glyph_records;

	/// used to pass a color on to shape_character::display()
	std::vector<fill_style>	m_dummy_style;

	std::vector<line_style>	m_dummy_line_style;

	/// Convert the characters in _text into a series of
	/// text_glyph_records to be rendered.
	void	format_text();

	/// Does LEFT/CENTER/RIGHT alignment on the records in
	/// m_text_glyph_records[], starting with
	/// last_line_start_record and going through the end of
	/// m_text_glyph_records.
	float align_line(edit_text_character_def::alignment align,
			int last_line_start_record, float x);

	/// Set our font, return previously set one.
	/// This is private for now, but might eventally
	/// be public, for setting fonts from ActionScript.
	const font* set_font(const font* newfont);

	const font* _font;

	bool m_has_focus;
	size_t m_cursor;
	void show_cursor();
	float m_xcursor;
	float m_ycursor;

	/// Associate a variable to the text of this character
	//
	/// Setting the associated variable actually changes the
	/// displayed text. Getting the variable would return the
	/// displayed text.
	///
	/// If the given variable already exist use it to set
	/// current text before overriding it.
	///
	/// Since the variable target may be undefined at time
	/// of instantiation of this EditText character, the
	/// class keeps track of wheter it succeeded registering
	/// the variable and this function will do nothing in this
	/// case. Thus it is safe to call it multiple time, using
	/// an as-needed policy (will be called from get_text_value and
	/// display)
	///
	void registerTextVariable();

	typedef std::pair<sprite_instance*, string_table::key> VariableRef;

	/// \brief
	/// Parse the given variable name
	/// into sprite and a string_table::key components
	///
	VariableRef parseTextVariableRef(const std::string& variableName) const;
	
	// Text fields need to handle cxform specially 
	cxform	get_world_cxform() const;

	/// The flag keeping status of TextVariable registration
	//
	/// It will be set to true if there's no need to register
	/// a text variable (ie. non-specified in the SWF)
	///
	bool _text_variable_registered;

	/// The text variable name
	//
	/// This is stored here, and not just in the definition,
	/// because it can be changed programmatically, by setting
	/// 'TextFields.variable'
	std::string _variable_name;

	bool _drawBackground;

	rgba _backgroundColor;

	bool _drawBorder;

	rgba _borderColor;

	rgba _textColor;

	bool _embedFonts;

	bool _wordWrap;

	bool _html;

	bool _selectable;

	AutoSizeValue _autoSize;

	/// Area in which the text is drawn. 
	//
	/// This area encloses all the text, can be automatically
	/// extended to fit text or hide text overflowing it.
	/// See the setAutoSize() method to change that.
	///
	geometry::Range2d<float> _bounds;

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable reosurces (for GC)
	//
	/// Reachable resource is currenlty just our definition,
	/// plus common character resources
	///
	void markReachableResources() const
	{
		if ( m_def.get() ) m_def->setReachable();

		// recurse to parent...
		markCharacterReachable();
	}
#endif
};

/// Initialize the global TextField class
void textfield_class_init(as_object& global);

} // namespace gnash

#endif // _GNASH_EDIT_TEXT_CHARACTER_H_
