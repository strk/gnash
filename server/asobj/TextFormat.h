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

// 
//

#ifndef __TEXTFORMAT_H__
#define __TEXTFORMAT_H__

#include "as_object.h" // for inheritance of TextFormat
#include "edit_text_character_def.h" // for edit_text_character_def::alignment enum
#include <boost/cstdint.hpp> // for boost::uint32_t
#include <string>

// Forward declarations
namespace gnash {  
	//class as_object;
}

namespace gnash {  

class DSOEXPORT TextFormat : public as_object
{
public:
  
	TextFormat();
	~TextFormat() {}

	/// Return a Boolean value that indicates whether the text is underlined.
	bool underlined()  { return _underline; }

	/// Return a Boolean value that indicates whether the text is italicized.
	bool italiced()    { return _italic; }

	/// Return a Boolean value that indicates whether the text is boldface.
	bool bold()        { return _bold; }

	bool bullet()      { return _bullet; }

	/// Return the color of text using this text format.
	//
	/// A number containing three 8-bit RGB components; for example,
        /// 0xFF0000 is red, 0x00FF00 is green.
	boost::uint32_t color() const { return _color; }

	/// \brief
	/// Return ann integer that indicates the indentation from the left
        /// margin to the first character in the paragraph
	boost::uint16_t indent() const { return _indent; }

	/// Return the alignment of the paragraph, represented as a string.
	//
	/// If "left", the paragraph is left-aligned. If "center", the
	/// paragraph is centered. If "right", the paragraph is
	/// right-aligned.
	///
	/// FIXME: use an enum !
	///
	edit_text_character_def::alignment align() const { return _align; }

	/// Return the name of a font for text as a string.
	const std::string& font() const { return _font; }

	// See doc for _target member
	const std::string& target() const { return _target; }

	// See doc for _target member
	void targetSet(const std::string& s) { _target=s; }

	// See doc for _url member
	const std::string& url() const { return _url; }

	// See doc for _url member
	void urlSet(const std::string& s) { _url=s; }

	///
	boost::uint16_t blockIndent() { return _block_indent; }

	/// Return a number that indicates the amount of leading vertical
	/// space between lines.
	boost::uint16_t leading()     { return _leading; }

	/// Indicates the left margin of the paragraph, in points.
	boost::uint16_t leftMargin()  { return _left_margin; }

	/// Indicates the right margin of the paragraph, in points.
	boost::uint16_t rightMargin() { return _right_margin; }

	/// Return a float that indicates the point size in twips.
	boost::uint16_t size()        { return _point_size; }

	void underlinedSet(bool x)   { _underline = x; }
	void italicedSet(bool x)     { _italic = x; }
	void boldSet(bool x)         { _bold = x; }
	void bulletSet(bool x)       { _bullet = x; }
	void colorSet(boost::uint32_t x)      { _color = x; }
	void indentSet(boost::uint16_t x)      { _indent = x; }
	void fontSet(const std::string& font) { _font=font; }

	void alignSet(edit_text_character_def::alignment x)  { _align = x; }

	static edit_text_character_def::alignment parseAlignString(const std::string& align);

	static const char* getAlignString(edit_text_character_def::alignment a);

	void alignSet(const std::string& align) { alignSet(parseAlignString(align)); }

	void blockIndentSet(boost::uint16_t x)   { _block_indent = x; }
	void leadingSet(boost::uint16_t x)     { _leading = x; }
	void leftMarginSet(boost::uint16_t x)  { _left_margin = x; }
	void rightMarginSet(boost::uint16_t x) { _right_margin = x; }

	/// Set font point size in twips
	void sizeSet(boost::uint16_t x)        { _point_size = x; }

	static as_value display_getset(const fn_call& fn);
	static as_value bullet_getset(const fn_call& fn);
	static as_value tabStops_getset(const fn_call& fn);
	static as_value blockIndent_getset(const fn_call& fn);
	static as_value leading_getset(const fn_call& fn);
	static as_value indent_getset(const fn_call& fn);
	static as_value rightMargin_getset(const fn_call& fn);
	static as_value leftMargin_getset(const fn_call& fn);
	static as_value align_getset(const fn_call& fn);
	static as_value underline_getset(const fn_call& fn);
	static as_value italic_getset(const fn_call& fn);
	static as_value bold_getset(const fn_call& fn);
	static as_value target_getset(const fn_call& fn);
	static as_value url_getset(const fn_call& fn);
	static as_value color_getset(const fn_call& fn);
	static as_value size_getset(const fn_call& fn);
	static as_value font_getset(const fn_call& fn);
	static as_value getTextExtent_method(const fn_call& fn);

  
private:


	/// A Boolean value that indicates whether the text is underlined.
	bool          _underline;

	/// A Boolean value that indicates whether the text is boldface.
	bool          _bold;	

	/// A Boolean value that indicates whether the text is italicized.
	bool          _italic;

	// 
	bool          _bullet;
  
	/// The alignment of the paragraph, represented as a string.
	//
	/// If "left", the paragraph is left-aligned. If "center", the
	/// paragraph is centered. If "right", the paragraph is
	/// right-aligned. If "justify", the paragraph is justified.
	///
	edit_text_character_def::alignment _align;

	// 
	boost::uint16_t		_block_indent;

	/// The color of text using this text format.
	//
	/// A number containing three 8-bit RGB components; for example,
        /// 0xFF0000 is red, 0x00FF00 is green.
	boost::uint32_t	_color;	

	// The name of a font for text as a string.
	std::string _font;	

	/// An integer that indicates the indentation from the left
        /// margin to the first character in the paragraph (twips)
	boost::uint16_t _indent;

	/// A number that indicates the amount of leading vertical
	/// space between lines (twips)
	boost::uint16_t _leading;

	/// Indicates the left margin of the paragraph, in points (twips)
	boost::uint16_t _left_margin;

	/// Indicates the right margin of the paragraph, in points (twips).
	boost::uint16_t _right_margin;

	/// Point size in twips.
	boost::uint16_t	_point_size;

	///
	int _tab_stops;

	/// The target window where the hyperlink is displayed. 
        /// If the target window is an empty string, the text is displayed in
        /// the default target window _self. If the url parameter is
        /// set to an empty string or to the value null, you can get
        /// or set this property, but the property will have no effect.
	std::string	_target;

	/// The URL to which the text in this text format hyperlinks.
	/// If url is an empty string, the text does not have a hyperlink
	std::string	 _url;	

};
 

/// Initialize the global Color class
void textformat_class_init(as_object& global);

} // end of gnash namespace

#endif	// __TEXTFORMAT_H__
