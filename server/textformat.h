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

#ifndef __TEXTFORMAT_H__
#define __TEXTFORMAT_H__


#include "log.h"
#include "action.h"
#include "impl.h"


namespace gnash {  

class text_format
{
public:
  // new text_format([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,
  //                [leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
  
  text_format();
  // tulrich: TODO need to take const ref!
  text_format(tu_string font);
  text_format(tu_string font, int size);
  text_format(tu_string font, int size, int color);
  ~text_format();
  
  bool underlined()  { return _underline; }
  bool italiced()    { return _italic; }
  bool bold()        { return _bold; }
  bool bullet()      { return _bullet; }
  uint32 color() const { return _color; }
  float indent() const { return _indent; }
  const tu_string& align() const { return _align; }
  float blockIndent() { return _block_indent; }
  float leading()     { return _leading; }
  float leftMargin()  { return _left_margin; }
  float RightMargin() { return _right_margin; }
  float size()        { return _point_size; }

  void underlinedSet(bool x)   { _underline = x; }
  void italicedSet(bool x)     { _italic = x; }
  void boldSet(bool x)         { _bold = x; }
  void bulletSet(bool x)       { _bullet = x; }
  void colorSet(uint32 x)      { _color = x; }
  void indentSet(float x)      { _indent = x; }
  void alignSet(tu_string x)  { _align = x; }
  void blockIndentSet(float x)   { _block_indent = x; }
  void leadingSet(float x)     { _leading = x; }
  void leftMarginSet(float x)  { _left_margin = x; }
  void rightMarginSet(float x) { _right_margin = x; }
  void sizeSet(float x)        { _point_size = x; }

  // In a paragraph, change the format of a range of characters.
  void setTextFormat (text_format &format);
  void setTextFormat (int index, text_format &format);
  void setTextFormat (int start, int end, text_format &format);

  text_format &getTextFormat ();
  text_format &getTextFormat (int index);
  text_format &getTextFormat (int start, int end);

  int getTextExtant();
  text_format *operator = (text_format &format);
  
 private:
  bool          _underline;	// A Boolean value that indicates whether the text is underlined.
  bool          _bold;		// A Boolean value that indicates whether the text is boldface.
  bool          _italic;	// A Boolean value that indicates whether the text is italicized.
  bool          _bullet;	// 
  
  tu_string	 _align;	// The alignment of the paragraph, represented as a string.
                                // If "left", the paragraph is left-aligned. If "center", the
                                // paragraph is centered. If "right", the paragraph is
                                // right-aligned.
  float		_block_indent;	// 
  uint32	_color;		// The color of text using this text format. A number
                                // containing three 8-bit RGB components; for example,
                                // 0xFF0000 is red, 0x00FF00 is green.
  tu_string _font;		// The name of a font for text as a string.
  float		_indent;	// An integer that indicates the indentation from the left
                                // margin to the first character in the paragraph
  float		_leading;	// A number that indicates the amount of leading vertical
                                // space between lines.
  float		_left_margin;	// Indicates the left margin of the paragraph, in points.
  float		_right_margin;	// Indicates the right margin of the paragraph, in points.
  float		_point_size;	// An integer that indicates the point size.
  int		_tab_stops;	// 
  int		_target;	// The target window where the hyperlink is displayed. If the
                                // target window is an empty string, the text is displayed in
                                // the default target window _self. If the url parameter is
                                // set to an empty string or to the value null, you can get
                                // or set this property, but the property will have no effect.
  tu_string	 _url;		// The URL to which the text in this text format hyperlinks.
                                // If url is an empty string, the text does not have a hyperlink
};
 
struct textformat_as_object : public gnash::as_object
{
	text_format obj;
};

void textformat_new(const fn_call& fn);

void textformat_setformat(const fn_call& fn);

void textformat_getformat(const fn_call& fn);

} // end of gnash namespace

#endif	// __TEXTFORMAT_H__
