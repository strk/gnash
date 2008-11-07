// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_TEXTFIELD_H
#define GNASH_TEXTFIELD_H

#include "character.h" // for inheritance
#include "styles.h" // for fill_style and line_style
#include "Range2d.h"
#include "rect.h" // for inlines
#include "font.h" // for visibility of font add_ref/drop_ref

// Forward declarations
namespace gnash {
	class text_glyph_record; 
    namespace SWF {
        class DefineEditTextTag;
    }
}

namespace gnash {

/// An instance of a DefineEditTextTag 
class TextField : public character
{

public:

    /// Text alignment values
	enum TextAlignment
	{
		ALIGN_LEFT = 0,
		ALIGN_RIGHT,
		ALIGN_CENTER,
		ALIGN_JUSTIFY
	};

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

	/// Possible type values
	enum TypeValue {

		/// Invalid value
		typeInvalid,

		/// Do not accept input, text is only changed by variable name
		/// or assigning to the .text member
		typeDynamic,

		/// Accept user input
		typeInput
	};

	TextField(character* parent, const SWF::DefineEditTextTag& def, int id);

    TextField(character* parent, const rect& bounds);

	~TextField();

	// TODO: should this return isSelectable() ?
	bool can_handle_mouse_event() const { return true; }

	character* get_topmost_mouse_entity(boost::int32_t x, boost::int32_t y);

	bool wantsInstanceName() const
	{
		return true; // text fields can be referenced 
	}	
		
	bool on_event(const event_id& id);	

	const std::string& getVariableName() const
	{
		return _variable_name;
	}

	/// Set the name of a variable associated to this
	/// TextField's displayed text.
	//
	/// Calling this function will override any previous
	/// setting for the variable name.
	/// 
	void set_variable_name(const std::string& newname);
	
	/// \brief Set our text to the given string by effect of an update of a
    /// registered variable name
	//
	/// This call only updates the text and is only meant to be called
    /// by ourselves or by MovieClip when a registered TextVariable is
    /// updated.
	void updateText(const std::string& s);

 	/// Return value of our text.
	std::string get_text_value() const;

	/// Return true if text is defined
	bool getTextDefined() const { return _textDefined; }

	/// We have a "text" member.
	bool set_member(string_table::key name, const as_value& val, 
		string_table::key nsname = 0, bool ifFound=false);

	bool get_member(string_table::key name, as_value* val, 
		string_table::key nsname = 0);

	/// Draw the dynamic string.
	void	display();

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	rect getBounds() const
	{
		return _bounds;
	}

	// See dox in character.h
	bool pointInShape(boost::int32_t x, boost::int32_t y) const;

	/// See dox in character::unload (character.h)
	//
	/// NOTE: TextField (TextField) never has
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

    /// Get the current maxChars setting of the TextField
    boost::int32_t maxChars() const {
        return _maxChars;
    }

    /// Set the current maxChars setting of the TextField
    void maxChars(boost::int32_t max) {
        _maxChars = max;
    }

    /// Get the current multiline setting of the TextField
    bool multiline() const {
        return _multiline;
    }

    /// Set the current multiline setting of the TextField
    void multiline(bool b) {
        _multiline = b;
    }

    /// Get the current password setting of the TextField
    bool password() const {
        return _password;
    }

    /// Set the current password setting of the TextField
    void password(bool b) {
        _password = b;
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

	/// Return text TextAlignment
    TextAlignment getTextAlignment();

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
	static AutoSizeValue parseAutoSizeValue(const std::string& val);

	/// Return autoSize value as a string
	//
	/// @param val
	/// 	Auto size value 
	///
	/// @return a C-string representation of the autoSize value.
	///	The returns is *never* NULL.
	///
	static const char* autoSizeValueName(AutoSizeValue val);

    /// Attaches more properties to the prototype on first instantiation.
    //
    /// @param proto    The prototype of the TextField object.
    //
    /// This is used when constructing non-AS textfields.
    static void attachTextFieldInstanceProperties(as_object& proto);

	/// Set type (input or dynamic)
	//
	/// @param val
	/// 	The TypeValue to use, no-op if typeInvalid.
	///
	void setType(TypeValue val) { if (val != typeInvalid) _type=val; }

	/// Get type (input, dynamic or invalid)
	TypeValue getType() const
	{
		return _type;
	}

	/// Return true if this TextField is read-only
	bool isReadOnly() const { return _type != typeInput; }

	/// Parse type string value
	//
	/// @param val
	/// 	Type value as a string (one of input or dynamic)
	///
	/// @return an TypeValue identifier. typeInvalid if invalid.
	///
	static TypeValue parseTypeValue(const std::string& val);

	/// Return type value as a string
	//
	/// @param val
	/// 	Type value  (enum)
	///
	/// @return a C-string representation of the type value.
	///	The returns is *never* NULL.
	///
	static const char* typeValueName(TypeValue val);

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
	virtual bool isSelectableTextField() const
	{
		return isSelectable();
	}

	/// Remove this textfield from the stage
	//
	/// This is to implement TextField.removeTextField, will
	/// basically forward the request to its parent.
	/// Eventually this and MovieClip::removeMovieClip
	/// will be merged in a single function to be later used
	/// also for AS3 removeChild().
	///
	void removeTextField();

	/// Set our font, return previously set one.
	//
	/// @param newfont
	///	Will be stored in an intrusive_ptr
	///
	boost::intrusive_ptr<const font> setFont(boost::intrusive_ptr<const font> newfont);

	const font* getFont() { return _font.get(); }

	boost::uint16_t getFontHeight() const
	{
		return _fontHeight;
	}

	void setFontHeight(boost::uint16_t h);

	boost::uint16_t getLeftMargin() const
	{
		return _leftMargin;
	}

	void setLeftMargin(boost::uint16_t h);

	boost::uint16_t getRightMargin() const
	{
		return _rightMargin;
	}

	void setRightMargin(boost::uint16_t h);

	boost::uint16_t getIndent() const
	{
		return _indent;
	}

	void setIndent(boost::uint16_t h);

	boost::uint16_t getBlockIndent() const
	{
		return _blockIndent;
	}

	void setBlockIndent(boost::uint16_t h);

	TextAlignment getAlignment() const
	{
		return _alignment;
	}

	void setAlignment(TextAlignment h);

	boost::uint16_t getLeading() const
	{
		return _leading;
	}

	void setLeading(boost::uint16_t h);

	bool getUnderlined() const
	{
		return _underlined;
	}

	void setUnderlined(bool v);

	const rect& getTextBoundingBox() const
	{
		return m_text_bounding_box;
	}

	/// Set our text to the given string.
	//
	/// This function will also update any registered variable
	///
	void setTextValue(const std::wstring& wstr);


private:

    void init();

    /// The immutable definition of our TextField
    //
    /// This is NULL for dynamic TextFields.
    boost::intrusive_ptr<const SWF::DefineEditTextTag> _tag;

	/// \brief Set our text to the given string by effect of an update of a
    /// registered variable name
	//
	/// This call only updates the text and is only meant to be called
    /// by ourselves or by MovieClip when a registered TextVariable is
    /// updated.
	void updateText(const std::wstring& s);

	/// Set focus 
	void setFocus();

	/// Kill focus 
	void killFocus();

	/// Call this function when willing to invoke the onChanged event handler
	void onChanged();

	/// Call this function when willing to invoke the onSetFocus event handler
	void onSetFocus();

	/// Call this function when willing to invoke the onKillFocus event handler
	void onKillFocus();

	/// The actual text.
    //
    /// Because we have to deal with non-ascii characters (129-255), this
    /// is a wide string; the cursor position and the position within the
    /// string are then the same, which makes manipulating the string much
    /// easier.
	std::wstring _text;

	/// This flag will be true as soon as the TextField
	/// is assigned a text value. Only way to be false is
	/// when definition has the hasText flag set to false
	/// and no actionscript added text.
	bool _textDefined;

	/// bounds of dynamic text, as laid out
	rect m_text_bounding_box;

	/// Reset our text bounding box to the given point.
	void reset_bounding_box(boost::int32_t x, boost::int32_t y)
	{
		m_text_bounding_box.set_to_point(x, y);
	}

	typedef std::vector<text_glyph_record> TextGlyphRecords;
	TextGlyphRecords m_text_glyph_records;

	/// used to pass a color on to shape_character::display()
	std::vector<fill_style>	m_dummy_style;

	std::vector<line_style>	m_dummy_line_style;

	/// Convert the characters in _text into a series of
	/// text_glyph_records to be rendered.
	void format_text();
	
	/// Extracts an HTML tag.
	///
	/// @param tag  This string is filled with the extracted HTML tag.
	/// @param it   An iterator pointing to the first character of the
	///             HTML tag. It is left pointing to the character after the
	///             closing tag or the end of the string.
	/// @param e    An iterator pointing to the end of the string.
	/// @return     Whether the tag is complete or not (i.e. whether a '>'
	///             was found).
	bool parseHTML(std::wstring& tag, std::wstring::const_iterator& it,
	                      const std::wstring::const_iterator& e) const;

	/// Does LEFT/CENTER/RIGHT alignment on the records in
	/// m_text_glyph_records[], starting with
	/// last_line_start_record and going through the end of
	/// m_text_glyph_records.
	float align_line(TextAlignment align, int last_line_start_record, float x);

	bool _underlined;

	boost::uint16_t _leading;

	TextAlignment _alignment;

	boost::uint16_t _indent;

	/// Indentation for every line (including the ones created by
	/// effect of a word-wrap.
	boost::uint16_t _blockIndent;

	boost::uint16_t _leftMargin;

	boost::uint16_t _rightMargin;

	boost::uint16_t _fontHeight;

	boost::intrusive_ptr<const font> _font;

	bool m_has_focus;
	size_t m_cursor;
	void show_cursor(const SWFMatrix& mat);
	float m_xcursor;
	float m_ycursor;

    /// Corresponds to the multiline property.
    bool _multiline;

    /// Corresponds to the password property.
    bool _password;

    /// Corresponds to the maxChars property.
    boost::int32_t _maxChars;

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

	typedef std::pair<as_object*, string_table::key> VariableRef;

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

	TypeValue _type;

	/// Area in which the text is drawn. 
	//
	/// This area encloses all the text, can be automatically
	/// extended to fit text or hide text overflowing it.
	/// See the setAutoSize() method to change that.
	///
	rect _bounds;

protected:

	/// Mark reachable reosurces (for GC)
	//
	/// Reachable resources are:
	///  - The font being used (m_font) 
	///  - Our definition
	///  - Common character resources
	///
	void markReachableResources() const;
};

/// Initialize the global TextField class
void textfield_class_init(as_object& global);

} // namespace gnash

#endif // _GNASH_EDIT_TEXT_CHARACTER_H_
