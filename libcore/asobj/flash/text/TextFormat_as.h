// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_TEXTFORMAT_H
#define GNASH_TEXTFORMAT_H

#include "TextField.h"
#include "RGBA.h" 

#include <vector>
#include <boost/cstdint.hpp> 
#include <string>

namespace gnash {
    class as_object;
}

namespace gnash {  

/// A type to hold a simple value but with an additional 'unset' state.
//
/// Most TextFormat values can be 'uninitialized', meaning they are not
/// taken into account when applying formatting. These values return null
/// in ActionScript.
template<typename T>
class
Optional
{
public:

    /// Construct an Optional<T> with no value set.
    Optional()
        :
        _t(),
        _set(false)
    {}

    /// Construct an Optional<T> with an initial value.
    //
    /// Implicit construction is allowed.
    Optional(const T& t)
        :
        _t(t),
        _set(true)
    {}

    /// Unset this value.
    void unset() {
        _set = false;
    }

    /// Relatively safe conversion to bool via void*.
    operator const void*() const {
        return _set ? this : 0;
    }

    /// Access to value using operator->().
    const T* operator->() const {
        assert(_set);
        return &_t;
    }

    /// Assign a T to this Optional<T>.
    Optional<T>& operator=(const T& t) {
        _t = t;
        _set = true;
        return *this;
    }

    /// Retrieve type only if set (converts to true).
    const T& operator*() const {
        assert(_set);
        return _t;
    }

private:
    T _t;
    bool _set;
};


/// The TextFormat_as Relay type stores text properties.
//
/// Most properties can either have a value or be null.
//
/// TODO: SWF8 has two additional members: kerning and letterSpacing.
class TextFormat_as : public Relay
{
public:
  
    TextFormat_as();

    ~TextFormat_as() {}

    /// Return a Boolean value that indicates whether the text is underlined.
    const Optional<bool>& underlined() const { return _underline; }
    
    /// Return a Boolean value that indicates whether the text is boldface.
    const Optional<bool>& bold() const { return _bold; }

    /// Return a Boolean value that indicates whether the text is italicized.
    const Optional<bool>& italic() const { return _italic; }

    /// Return the color of text using this text format.
    const Optional<rgba>& color() const { return _color; }

    /// Whether the text should have a bullet.
    const Optional<bool>& bullet() const { return _bullet; }

    /// The display type (block or inline).
    //
    /// Note this is not an optional parameter.
    TextField::TextFormatDisplay display() const {
        return _display;
    }

    const Optional<std::vector<int> >& tabStops() const {
        return _tabStops;
    }

    /// Indentation from left margin to the first character in the paragraph
    const Optional<boost::uint16_t>& indent() const { return _indent; }
    
    /// Paragraph alignment
    const Optional<TextField::TextAlignment>& align() const { return _align; }

    /// Font name.
    const Optional<std::string>& font() const { return _font; }

    // See doc for _target member
    const Optional<std::string>& target() const { return _target; }

    // See doc for _url member
    const Optional<std::string>& url() const { return _url; }

    /// The block indent.
    const Optional<boost::uint32_t>& blockIndent() const {
        return _blockIndent;
    }

    /// Return a number that indicates the amount of leading vertical
    /// space between lines.
    const Optional<boost::uint16_t>& leading() const { return _leading; }

    /// Indicates the left margin of the paragraph, in points.
    const Optional<boost::uint16_t>& leftMargin() const { return _leftMargin; }

    /// Indicates the right margin of the paragraph in twips
    const Optional<boost::uint16_t>& rightMargin() const {
        return _rightMargin;
    }

    /// Return a float that indicates the point size in twips.
    const Optional<boost::uint16_t>& size() const { return _pointSize; }

    /// Setters

    void targetSet(const Optional<std::string>& s) { _target=s; }

    void urlSet(const Optional<std::string>& s) { _url=s; }

    void underlinedSet(const Optional<bool>& x) { _underline = x; }

    void italicSet(const Optional<bool>& x) { _italic = x; }

    void boldSet(const Optional<bool>& x) { _bold = x; }

    void bulletSet(const Optional<bool>& x) { _bullet = x; }

    void colorSet(const Optional<rgba>& x) { _color = x; }

    void indentSet(const Optional<boost::uint16_t>& x) { _indent = x; }

    void fontSet(const Optional<std::string>& font) { _font=font; }
    
    void alignSet(const Optional<TextField::TextAlignment>& x) { _align = x; }
    
    void alignSet(const std::string& align);
    
    void blockIndentSet(const Optional<boost::uint32_t>& x) {
        _blockIndent = x;
    }
    
    void leadingSet(const Optional<boost::uint16_t>& x) { _leading = x; }

    void leftMarginSet(const Optional<boost::uint16_t>& x) { _leftMargin = x; }

    void rightMarginSet(const Optional<boost::uint16_t>& x) {
        _rightMargin = x;
    }

    void sizeSet(const Optional<boost::uint16_t>& x) { _pointSize = x; }

    void tabStopsSet(const std::vector<int>& tabStops) { _tabStops = tabStops; }

    /// These are not optional!
    void displaySet(TextField::TextFormatDisplay x) { _display = x; }
    void displaySet(const std::string& display);
private:

    /// A Boolean value that indicates whether the text is underlined.
    Optional<bool> _underline;

    /// A Boolean value that indicates whether the text is boldface.
    Optional<bool> _bold;

    /// A Boolean value that indicates whether the text is italicized.
    Optional<bool> _italic;

    // 
    Optional<bool> _bullet;
    
    TextField::TextFormatDisplay _display;
  
    /// The alignment of the paragraph, represented as a string.
    //
    /// If "left", the paragraph is left-aligned. If "center", the
    /// paragraph is centered. If "right", the paragraph is
    /// right-aligned. If "justify", the paragraph is justified.
    ///
    Optional<TextField::TextAlignment> _align;

    // 
    Optional<boost::uint32_t> _blockIndent;

    /// The color of text using this text format.
    //
    /// A number containing three 8-bit RGB components; for example,
        /// 0xFF0000 is red, 0x00FF00 is green.
    Optional<rgba> _color;    

    // The name of a font for text as a string.
    Optional<std::string> _font;    

    /// An integer that indicates the indentation from the left
    /// margin to the first DisplayObject in the paragraph (twips)
    Optional<boost::uint16_t> _indent;

    /// A number that indicates the amount of leading vertical
    /// space between lines (twips)
    Optional<boost::uint16_t> _leading;

    /// Indicates the left margin of the paragraph, in points (twips)
    Optional<boost::uint16_t> _leftMargin;

    /// Indicates the right margin of the paragraph, in points (twips).
    Optional<boost::uint16_t> _rightMargin;

    /// Point size in twips.
    Optional<boost::uint16_t> _pointSize;

    ///
    Optional<std::vector<int> > _tabStops;

    /// The target window where the hyperlink is displayed. 
        /// If the target window is an empty string, the text is displayed in
        /// the default target window _self. If the url parameter is
        /// set to an empty string or to the value null, you can get
        /// or set this property, but the property will have no effect.
    Optional<std::string> _target;

    /// The URL to which the text in this text format hyperlinks.
    /// If url is an empty string, the text does not have a hyperlink
    Optional<std::string> _url;    
};

void textformat_class_init(as_object& global, const ObjectURI& uri);

void registerTextFormatNative(as_object& global);

} // end of gnash namespace

#endif

