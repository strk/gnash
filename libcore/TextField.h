// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include <boost/intrusive_ptr.hpp>
#include <map>
#include <string>
#include <vector>

#include "InteractiveObject.h" // for inheritance
#include "LineStyle.h" // for LineStyle
#include "snappingrange.h"
#include "SWFRect.h" // for inlines
#include "GnashKey.h"

// Forward declarations
namespace gnash {
    namespace SWF {
        class DefineEditTextTag;
        class TextRecord;
    }
    class TextFormat_as;
    class Font;
}

#ifdef __ANDROID__
namespace std {
typedef basic_string
   <wchar_t
   ,std::char_traits<wchar_t>
   ,std::allocator<wchar_t> >
wstring;
}
#endif

namespace gnash {

/// An instance of a DefineEditTextTag 
class DSOTEXPORT TextField : public InteractiveObject
{

public:
    
    typedef std::vector<size_t> LineStarts;

    /// Text alignment values
	enum TextAlignment
	{
		ALIGN_LEFT = 0,
		ALIGN_RIGHT,
		ALIGN_CENTER,
		ALIGN_JUSTIFY
	};

	/// Text format display values
	enum TextFormatDisplay
	{
		TEXTFORMAT_BLOCK = 0,
		TEXTFORMAT_INLINE = 1
	};
	
	/// Possible autoSize values
	enum AutoSize {

		/// Do not automatically resize TextField as text grow/shrink
		AUTOSIZE_NONE,

		/// Expand TextField, anchor the top-left side
		AUTOSIZE_LEFT,

		/// Expand TextField, anchor the horizontal center
		AUTOSIZE_CENTER,

		/// Expand TextField, anchor the top-right side
		AUTOSIZE_RIGHT
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

    /// Constructs a TextField as specified in a DefineEditText tag.
	TextField(as_object* object, DisplayObject* parent,
            const SWF::DefineEditTextTag& def);

    /// Constructs a TextField with default values and the specified bounds.
    //
    /// Notably, the default textHeight is 12pt (240 twips).
	/// @param parent A pointer to the DisplayObject parent of this TextField
	/// @param bounds A SWFRect specifying the bounds of this TextField
    TextField(as_object* object, DisplayObject* parent, const SWFRect& bounds);


	~TextField();

	// TODO: should this return isSelectable() ?
	/// Returns true for now, TextField is always "Mouse-Enabled"
	bool mouseEnabled() const { return true; }

	/// Returns a pointer to the topmost InteractiveObject at (x,y)
	//
	/// @param x x-coordinate
	/// @param y y-coordinate
	InteractiveObject* topmostMouseEntity(boost::int32_t x, boost::int32_t y);

    /// Return the version of the SWF this was parsed from.
    //
    /// TODO: work out what this means for dynamic TextFields.
    virtual int getDefinitionVersion() const;

	/// This function is called as a user-input handler
	void mouseEvent(const event_id& id);	

    /// Handle user input from a key press.
    void keyInput(key::code k);

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

	/// Return value of our htmlText.
	std::string get_htmltext_value() const;

	/// Return true if text is defined
	bool getTextDefined() const { return _textDefined; }

    size_t getCaretIndex() const {
        return m_cursor;
    }

	/// Get a std::pair of size_t with start/end of selection
    //
    /// Both start and end should invariably be within the
    /// range of the text.
    const std::pair<size_t, size_t>& getSelection() const {
        return _selection;
    }

    /// Replace the current selection with the new text.
	//
	/// @param replace String to replace the current selection
    void replaceSelection(const std::string& replace);

    /// Set the current selection
    //
    /// @param start    The index of the beginning of the selection.
    /// @param end      The index of the end of the selection.
    //
    /// If start is greater than end, the values are swapped, ensuring
    /// end is never less than start.
    void setSelection(int start, int end);

    /// Override of DisplayObject::setWidth
    //
    /// TextField width currently behaves differently from MovieClip width
    virtual void setWidth(double width);
    
    /// Override of DisplayObject::setHeight
    //
    /// TextField height currently behaves differently from MovieClip height
    virtual void setHeight(double height);

	/// Draw the dynamic string.
	virtual void display(Renderer& renderer, const Transform& xform);

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	/// Get bounding SWFRect of this TextField
	virtual SWFRect getBounds() const
	{
		return _bounds;
	}

	// See dox in DisplayObject.h
	bool pointInShape(boost::int32_t x, boost::int32_t y) const;

	/// Return true if the 'background' should be drawn
	bool getDrawBackground() const;

	/// Specify whether to draw the background
	//
	/// @param draw If true the background of this TextField will be drawn
	void setDrawBackground(bool draw);

	/// \brief Return color of the background
	rgba getBackgroundColor() const;

	/// Set color of the background
	//
	/// Use setDrawBackground to actually use this value.
	/// @param col RGBA Object with color information. TextField
	/// 	background will be drawn in this color
	void setBackgroundColor(const rgba& col);

	/// Return true if this TextField should have its border visible
	bool getDrawBorder() const;

	/// Specify whether to draw the border
	//
	/// @param draw If true the border of this TextField will be drawn
	void setDrawBorder(bool draw);

	/// Return color of the border
	rgba getBorderColor() const;

	/// Set color of the border
	//
	/// Use setDrawBorder to actually use this value.
	/// @param col RGBA Object with color information. TextField border
	/// 	will be drawn in this color.
	void setBorderColor(const rgba& col);

	/// Return color of the text
	const rgba& getTextColor() const 
	{
		return _textColor;
	}

	/// Set color of the text
	//
	/// @param col RGBA Object with color information. Text in this TextField
	/// 	will be displayed in this color.
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
	//
	/// @param max The maximum number of characters that can be
	/// 	input by the user (Does not restrict Scripts)
    void maxChars(boost::int32_t max) {
        _maxChars = max;
    }

    /// Get the current multiline setting of the TextField
    bool multiline() const {
        return _multiline;
    }

    /// Set the current multiline setting of the TextField
	//
	/// @param b If true "Enter" key will be recognized (Does not
	/// 	restrict Scripts)
    void multiline(bool b) {
        _multiline = b;
    }
	
    /// Get the current password setting of the TextField
    bool password() const {
        return _password;
    }

    /// Set the current password setting of the TextField
	//
	/// @param b If true characters in the TextField will be displayed
	/// 	as (*)
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
	AutoSize getAutoSize() const
	{
		return _autoSize;
	}

	/// Return text TextAlignment
    TextAlignment getTextAlignment();

	/// Set autoSize value 
	//
	/// @param val
	/// 	The AutoSize to use
	///
	void setAutoSize(AutoSize val);

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

	/// Return true if HTML markup in text should be rendered.
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
	//
	/// @param v
	///	If true text in this TextField will be selectable
	void setSelectable(bool v) 
	{
		_selectable = v;
	}

	// See DisplayObject::isActiveTextField
	/// Return true if the TextField text is selectable
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
	boost::intrusive_ptr<const Font> setFont(
            boost::intrusive_ptr<const Font> newfont);

	const Font* getFont() { return _font.get(); }

	
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

	boost::int16_t getLeading() const
	{
		return _leading;
	}

	void setLeading(boost::int16_t h);

	bool getUnderlined() const
	{
		return _underlined;
	}
	
	TextFormatDisplay getDisplay() const
	{ 
		return _display;
	}
	
	bool getBullet() const
	{
		return _bullet;
	}
	
	const std::vector<int>& getTabStops() const
	{
		return _tabStops;
	}

	bool isRestrict() const
	{
		return _restrictDefined;
	}
	
	const std::string& getRestrict() const
	{
		return _restrict;
	}

	size_t getScroll() const
	{
		return _scroll;
	}

	size_t getMaxScroll() const
	{
		return _maxScroll;
	}

	size_t getHScroll() const
	{
		return _hScroll;
	}

	size_t getMaxHScroll() const
	{
		return _maxHScroll;
	}

	size_t getBottomScroll() const
	{
		return _bottomScroll;
	}

	void setUnderlined(bool v);
	void setTabStops(const std::vector<int>& tabStops);
	void setBullet(bool b);
	void setURL(std::string url);
	void setTarget(std::string target);
	void setRestrict(const std::string& restrict);
	void setDisplay(TextFormatDisplay display);
	void setScroll(size_t scroll) {
		_scroll = scroll;
		format_text();
	}
	void setMaxScroll(size_t maxScroll) {
		_maxScroll = maxScroll;
		format_text();
	}
	void setHScroll(size_t hScroll) {
		_hScroll = hScroll;
		format_text();
	}
	void setMaxHScroll(size_t maxHScroll) {
		_maxHScroll = maxHScroll;
		format_text();
	}
	void setbottomScroll(size_t bottomScroll) {
		_bottomScroll = bottomScroll;
		format_text();
	}

	/// Returns the number of the record that the cursor is in
	//
	size_t cursorRecord();

	void setTextFormat(TextFormat_as& tf);

	const SWFRect& getTextBoundingBox() const {
		return m_text_bounding_box;
	}

	/// Set our text to the given string.
	//
	/// This function will also update any registered variable
	///
	void setTextValue(const std::wstring& wstr);

private:

    void init();

	/// \brief Set our text to the given string by effect of an update of a
    /// registered variable name
	//
	/// This call only updates the text and is only meant to be called
    /// by ourselves or by MovieClip when a registered TextVariable is
    /// updated.
	void updateText(const std::wstring& s);

	void updateHtmlText(const std::wstring& s);

    void insertTab(SWF::TextRecord& rec, boost::int32_t& x, float scale);

	/// What happens when setFocus() is called on this TextField.
    //
    /// @return true if focus was set. A TextField can always receive focus,
    /// so this always returns true.
	virtual bool handleFocus();

	/// Kill focus 
	virtual void killFocus();

	/// Call this function when willing to invoke the onChanged event handler
	void onChanged();

	/// Reset our text bounding box to the given point.
	void reset_bounding_box(boost::int32_t x, boost::int32_t y)
	{
		m_text_bounding_box.set_to_point(x, y);
	}

	/// Convert the DisplayObjects in _text into a series of
	/// text_glyph_records to be rendered.
	void format_text();
	
	/// Move viewable lines based on m_cursor
	void scrollLines();
	
	/// Handles a new line, this will be called several times, so this
	/// will hopefully make code cleaner
	void newLine(boost::int32_t& x, boost::int32_t& y, 
				 SWF::TextRecord& rec, int& last_space_glyph,
				 LineStarts::value_type& last_line_start_record, float div);
					
	/// De-reference and do appropriate action for character iterator
	void handleChar(std::wstring::const_iterator& it,
            const std::wstring::const_iterator& e, boost::int32_t& x,
            boost::int32_t& y, SWF::TextRecord& rec, int& last_code,
		    int& last_space_glyph,
            LineStarts::value_type& last_line_start_record);
	
	/// Extracts an HTML tag.
	///
	/// @param tag  This string is filled with the extracted HTML tag.
	/// @param attributes This is a map of attribute names and values
	/// @param it   An iterator pointing to the first DisplayObject of the
	///             HTML tag. It is left pointing to the DisplayObject after the
	///             closing tag or the end of the string.
	/// @param e    An iterator pointing to the end of the string.
	/// @return     Whether the tag is complete or not (i.e. whether a '>'
	///             was found).
	bool parseHTML(std::wstring& tag,
            std::map<std::string, std::string>& attributes,
            std::wstring::const_iterator& it,
            const std::wstring::const_iterator& e,
			bool& selfclosing) const;

	/// Does LEFT/CENTER/RIGHT alignment on the records in
	/// m_text_glyph_records[], starting with
	/// last_line_start_record and going through the end of
	/// m_text_glyph_records.
	float align_line(TextAlignment align, int last_line_start_record, float x);

	/// Associate a variable to the text of this DisplayObject
	//
	/// Setting the associated variable actually changes the
	/// displayed text. Getting the variable would return the
	/// displayed text.
	///
	/// If the given variable already exist use it to set
	/// current text before overriding it.
	///
	/// Since the variable target may be undefined at time
	/// of instantiation of this EditText DisplayObject, the
	/// class keeps track of wheter it succeeded registering
	/// the variable and this function will do nothing in this
	/// case. Thus it is safe to call it multiple time, using
	/// an as-needed policy (will be called from get_text_value and
	/// display)
	///
	void registerTextVariable();

	typedef std::pair<as_object*, ObjectURI> VariableRef;

	/// \brief
	/// Parse the given variable name
	/// into sprite and a string_table::key components
	///
	VariableRef parseTextVariableRef(const std::string& variableName) const;

	/// Called in display(), sets the cursor using m_cursor and _textRecords
	//
	/// @param renderer
	/// @param mat
	void show_cursor(Renderer& renderer, const SWFMatrix& mat);

    /// The immutable definition of our TextField
    //
    /// This is NULL for dynamic TextFields.
    boost::intrusive_ptr<const SWF::DefineEditTextTag> _tag;

	/// The actual text.
    //
    /// Because we have to deal with non-ascii DisplayObjects (129-255), this
    /// is a wide string; the cursor position and the position within the
    /// string are then the same, which makes manipulating the string much
    /// easier.
	std::wstring _text;

	/// The html representation of our text
	//
	std::wstring _htmlText;

	/// bounds of dynamic text, as laid out
	SWFRect m_text_bounding_box;

	typedef std::vector<SWF::TextRecord> TextRecords;
	TextRecords _textRecords;

	std::vector<size_t> _recordStarts;

	TextRecords _displayRecords;

	std::string _url;
	std::string _target;
	std::string _restrict;
	std::set<wchar_t> _restrictedchars;
	TextFormatDisplay _display;
	std::vector<int> _tabStops;
	LineStarts _line_starts;

	/// The text variable name
	//
	/// This is stored here, and not just in the definition,
	/// because it can be changed programmatically, by setting
	/// 'TextFields.variable'
	std::string _variable_name;

	rgba _backgroundColor;

	rgba _borderColor;

	rgba _textColor;
	
	TextAlignment _alignment;

	boost::intrusive_ptr<const Font> _font;
	size_t m_cursor;
	size_t _glyphcount;
	size_t _scroll;
	size_t _maxScroll;
	size_t _hScroll;
	size_t _maxHScroll;
	size_t _bottomScroll;
	size_t _linesindisplay;

    /// Corresponds to the maxChars property.
    size_t _maxChars;
	
    AutoSize _autoSize;

	TypeValue _type;

	/// Area in which the text is drawn. 
	//
	/// This area encloses all the text, can be automatically
	/// extended to fit text or hide text overflowing it.
	/// See the setAutoSize() method to change that.
	///
	SWFRect _bounds;

    /// Represents the selected part of the text. The second element must
    /// never be less than the first.
    std::pair<size_t, size_t> _selection;

    boost::int16_t _leading;
	boost::uint16_t _indent;

	/// Indentation for every line (including the ones created by
	/// effect of a word-wrap.
	boost::uint16_t _blockIndent;

	boost::uint16_t _leftMargin;

	boost::uint16_t _rightMargin;

	boost::uint16_t _fontHeight;

	/// This flag will be true as soon as the TextField
	/// is assigned a text value. Only way to be false is
	/// when definition has the hasText flag set to false
	/// and no actionscript added text.
	bool _textDefined;

	bool _restrictDefined;
	bool _underlined;
	bool _bullet;

	bool m_has_focus;
	

    /// Corresponds to the multiline property.
    bool _multiline;

    /// Corresponds to the password property.
    bool _password;
	
	/// The flag keeping status of TextVariable registration
	//
	/// It will be set to true if there's no need to register
	/// a text variable (ie. non-specified in the SWF)
	///
	bool _text_variable_registered;

	bool _drawBackground;

	bool _drawBorder;

	bool _embedFonts;

	bool _wordWrap;

	bool _html;

	bool _selectable;
	
};

} // namespace gnash

#endif 
