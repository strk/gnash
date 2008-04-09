// textformat.cpp:  ActionScript text formatting decorators, for Gnash.
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


#include "log.h"
#include "Object.h" // for getObjectInterface
#include "textformat.h"
#include "fn_call.h"
#include "builtin_function.h" // for getter/setter properties
#include "namedStrings.h"
#include "VM.h"

#define ONCE(x) { static bool warned=false; if (!warned) { warned=true; x; } }

namespace gnash {

static as_value textformat_new(const fn_call& fn);
static as_object* getTextFormatInterface();
static void attachTextFormatInterface(as_object& o);

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
	float indent() const { return _indent; }

	/// Return the alignment of the paragraph, represented as a string.
	//
	/// If "left", the paragraph is left-aligned. If "center", the
	/// paragraph is centered. If "right", the paragraph is
	/// right-aligned.
	///
	/// FIXME: use an enum !
	///
	const std::string& align() const { return _align; }

	/// Return the name of a font for text as a string.
	const std::string& font() const { return _font; }

	///
	float blockIndent() { return _block_indent; }

	/// Return a number that indicates the amount of leading vertical
	/// space between lines.
	float leading()     { return _leading; }

	/// Indicates the left margin of the paragraph, in points.
	float leftMargin()  { return _left_margin; }

	/// Indicates the right margin of the paragraph, in points.
	float RightMargin() { return _right_margin; }

	/// Return a float that indicates the point size.
	float size()        { return _point_size; }

	void underlinedSet(bool x)   { _underline = x; }
	void italicedSet(bool x)     { _italic = x; }
	void boldSet(bool x)         { _bold = x; }
	void bulletSet(bool x)       { _bullet = x; }
	void colorSet(boost::uint32_t x)      { _color = x; }
	void indentSet(float x)      { _indent = x; }

	void alignSet(const std::string& x)  { _align = x; }

	void blockIndentSet(float x)   { _block_indent = x; }
	void leadingSet(float x)     { _leading = x; }
	void leftMarginSet(float x)  { _left_margin = x; }
	void rightMarginSet(float x) { _right_margin = x; }
	void sizeSet(float x)        { _point_size = x; }

	// In a paragraph, change the format of a range of characters.
	void setTextFormat (TextFormat &format);
	void setTextFormat (int index, TextFormat &format);
	void setTextFormat (int start, int end, TextFormat &format);

	TextFormat &getTextFormat ();
	TextFormat &getTextFormat (int index);
	TextFormat &getTextFormat (int start, int end);

	int getTextExtant();

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
	/// right-aligned.
	///
	/// FIXME: use an enum !
	///
	std::string _align;

	// 
	float		_block_indent;

	/// The color of text using this text format.
	//
	/// A number containing three 8-bit RGB components; for example,
        /// 0xFF0000 is red, 0x00FF00 is green.
	boost::uint32_t	_color;	

	// The name of a font for text as a string.
	std::string _font;	

	/// An integer that indicates the indentation from the left
        /// margin to the first character in the paragraph
	float		_indent;

	/// A number that indicates the amount of leading vertical
	/// space between lines.
	float		_leading;

	/// Indicates the left margin of the paragraph, in points.
	float		_left_margin;

	/// Indicates the right margin of the paragraph, in points.
	float		_right_margin;

	/// An float that indicates the point size.
	float		_point_size;

	///
	int		_tab_stops;

	/// The target window where the hyperlink is displayed. 
        /// If the target window is an empty string, the text is displayed in
        /// the default target window _self. If the url parameter is
        /// set to an empty string or to the value null, you can get
        /// or set this property, but the property will have no effect.
	int		_target;

	/// The URL to which the text in this text format hyperlinks.
	/// If url is an empty string, the text does not have a hyperlink
	std::string	 _url;	

};
 

TextFormat::TextFormat()
	:
	as_object(getTextFormatInterface()),
	_underline(false),
	_bold(false),
	_italic(false),
	_bullet(false),
	_block_indent(-1),
	_color(0),
	_indent(-1),
	_leading(-1),
	_left_margin(-1),
	_right_margin(-1),
	_point_size(-1),
	_tab_stops(-1),
	_target(-1)
{
	//log_debug("%s:", __FUNCTION__);
	init_member("getTextExtent", new builtin_function(TextFormat::getTextExtent_method));
}

// In a paragraph, change the format of a range of characters.
void
TextFormat::setTextFormat (TextFormat& /*format*/)
{
  //GNASH_REPORT_FUNCTION;
}

void
TextFormat::setTextFormat (int /*index*/, TextFormat& /*format*/)
{
  //GNASH_REPORT_FUNCTION;
}

void
TextFormat::setTextFormat (int /*start*/, int /*end*/, TextFormat& /*format*/)
{
  //GNASH_REPORT_FUNCTION;
}

/// new TextFormat([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,[leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
static as_value
textformat_new(const fn_call& /* fn */)
{
  //GNASH_REPORT_FUNCTION;
  //log_debug(_("%s: args=%d"), __FUNCTION__, nargs);

  boost::intrusive_ptr<TextFormat> text_obj = new TextFormat;
  ONCE(log_unimpl("TextFormat")); // need to handle args too..
  
  return as_value(text_obj.get());
}

as_value
TextFormat::display_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.display") );
	return as_value();
}

as_value
TextFormat::bullet_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.bullet") );
	return as_value();
}

as_value
TextFormat::tabStops_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.tabStops") );
	return as_value();
}

as_value
TextFormat::blockIndent_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.blockIndent") );
	return as_value();
}

as_value
TextFormat::leading_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.leading") );
	return as_value();
}

as_value
TextFormat::indent_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.indent") );
	return as_value();
}

as_value
TextFormat::rightMargin_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.rightMargin") );
	return as_value();
}

as_value
TextFormat::leftMargin_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.leftMargin") );
	return as_value();
}

as_value
TextFormat::align_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.align") );
	return as_value();
}

as_value
TextFormat::underline_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.underline") );
	return as_value();
}

as_value
TextFormat::italic_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.italic") );
	return as_value();
}

as_value
TextFormat::bold_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.bold") );
	return as_value();
}

as_value
TextFormat::target_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.target") );
	return as_value();
}

as_value
TextFormat::url_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.url") );
	return as_value();
}

as_value
TextFormat::color_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.color") );
	return as_value();
}

as_value
TextFormat::size_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.size") );
	return as_value();
}

as_value
TextFormat::font_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.font") );
	return as_value();
}

as_value
TextFormat::getTextExtent_method(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.getTextExtent") );
	return as_value();
}

static void
attachTextFormatInterface(as_object& o)
{
	o.init_readonly_property("display", &TextFormat::display_getset);
	o.init_readonly_property("bullet", &TextFormat::bullet_getset);
	o.init_readonly_property("tabStops", &TextFormat::tabStops_getset);
	o.init_readonly_property("blockIndent", &TextFormat::blockIndent_getset);
	o.init_readonly_property("leading", &TextFormat::leading_getset);
	o.init_readonly_property("indent", &TextFormat::indent_getset);
	o.init_readonly_property("rightMargin", &TextFormat::rightMargin_getset);
	o.init_readonly_property("leftMargin", &TextFormat::leftMargin_getset);
	o.init_readonly_property("align", &TextFormat::align_getset);
	o.init_readonly_property("underline", &TextFormat::underline_getset);
	o.init_readonly_property("italic", &TextFormat::italic_getset);
	o.init_readonly_property("bold", &TextFormat::bold_getset);
	o.init_readonly_property("target", &TextFormat::target_getset);
	o.init_readonly_property("url", &TextFormat::url_getset);
	o.init_readonly_property("color", &TextFormat::color_getset);
	o.init_readonly_property("size", &TextFormat::size_getset);
	o.init_readonly_property("font", &TextFormat::font_getset);
}

static as_object*
getTextFormatInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachTextFormatInterface(*o);
	}
	return o.get();
}

// extern (used by Global.cpp)
void textformat_class_init(as_object& global)
{
	// This is going to be the global Color "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&textformat_new, getTextFormatInterface());
	}

	// Register _global.Color
	global.init_member("TextFormat", cl.get());

}

} // end of gnash namespace
