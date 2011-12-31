// TextField.cpp:  User-editable text regions, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
// Things implemented:
//	- setTextFormat does not discard target, url, tabStops, display or
//	  bullets
//	- Above five fields are now implemented (except for target != blank)
//		- Call movie_root getURL function to properly open url and target

// Things to work on:
//	- For the url cases (url property and anchor tag in html) we should
//	  change the mouse cursor to the hand cursor standard for linkable 
//    text

#include "TextField.h"

#include <algorithm> 
#include <string>
#include <cstdlib>
#include <cctype>
#include <utility>
#include <map>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include "utf8.h"
#include "log.h"
#include "swf/DefineEditTextTag.h"
#include "MovieClip.h"
#include "movie_root.h"     // for killing focus
#include "as_environment.h" 
#include "Font.h" 
#include "fontlib.h" 
#include "namedStrings.h"
#include "StringPredicates.h"
#include "TextFormat_as.h"
#include "GnashKey.h"
#include "TextRecord.h"
#include "Point2d.h"
#include "GnashNumeric.h"
#include "MouseButtonState.h"
#include "Global_as.h"
#include "Renderer.h"
#include "Transform.h"
#include "ObjectURI.h"

// Text fields have a fixed 2 pixel padding for each side (regardless of border)
#define PADDING_TWIPS 40 

// Define the following to get detailed log information about
// textfield bounds and HTML tags:
//#define GNASH_DEBUG_TEXTFIELDS 1

// Define this to get debugging info about text formatting
//#define GNASH_DEBUG_TEXT_FORMATTING 1

namespace gnash {

TextField::TextField(as_object* object, DisplayObject* parent,
        const SWF::DefineEditTextTag& def)
    :
    InteractiveObject(object, parent),
    _tag(&def),
    _url(""),
    _target(""),
    _display(),
    _tabStops(),
    _variable_name(def.variableName()),
    _backgroundColor(255,255,255,255),
    _borderColor(0,0,0,255),
    _textColor(def.color()),
    _alignment(def.alignment()),
    _font(0),
    m_cursor(0u),
    _glyphcount(0u),
    _scroll(0u),
    _maxScroll(1u),
    _hScroll(0u),
    _maxHScroll(0u),
    _bottomScroll(0u),
    _linesindisplay(0u),
    _maxChars(def.maxChars()),
    _autoSize(def.autoSize() ? AUTOSIZE_LEFT : AUTOSIZE_NONE),
    _type(def.readOnly() ? typeDynamic : typeInput),
    _bounds(def.bounds()),
    _selection(0, 0),
    _leading(def.leading()),
    _indent(def.indent()), 
    _blockIndent(0),
    _leftMargin(def.leftMargin()), 
    _rightMargin(def.rightMargin()), 
    _fontHeight(def.textHeight()), 
    _textDefined(def.hasText()),
    _restrictDefined(false),
    _underlined(false),
    _bullet(false),
    m_has_focus(false),
    _multiline(def.multiline()),
    _password(def.password()),
    _text_variable_registered(false),
    _drawBackground(def.border()),
    _drawBorder(def.border()),
    _embedFonts(def.getUseEmbeddedGlyphs()),
    _wordWrap(def.wordWrap()),
    _html(def.html()),
    _selectable(!def.noSelect())
{
    assert(object);

    // WARNING! remember to set the font *before* setting text value!
    boost::intrusive_ptr<const Font> f = def.getFont();
    if (!f) f = fontlib::get_default_font(); 
    setFont(f);

    const int version = getSWFVersion(*object);
    
    // set default text *before* calling registerTextVariable
    // (if the textvariable already exist and has a value
    // the text will be replaced with it)
    if (_textDefined) {
        setTextValue(utf8::decodeCanonicalString(def.defaultText(), version));
    }

    init();

}

TextField::TextField(as_object* object, DisplayObject* parent,
        const SWFRect& bounds)
    :
    InteractiveObject(object, parent),
    _url(""),
    _target(""),
    _display(),
    _tabStops(),
    _backgroundColor(255,255,255,255),
    _borderColor(0, 0, 0, 255),
    _textColor(0, 0, 0, 255),
    _alignment(ALIGN_LEFT),
    _font(0),
    m_cursor(0u),
    _glyphcount(0u),
    _scroll(0u),
    _maxScroll(1u),
    _hScroll(0u),
    _maxHScroll(0u),
    _bottomScroll(0u),
    _linesindisplay(0u),
    _maxChars(0),
    _autoSize(AUTOSIZE_NONE),
    _type(typeDynamic),
    _bounds(bounds),
    _selection(0, 0),
    _leading(0),
    _indent(0), 
    _blockIndent(0),
    _leftMargin(0), 
    _rightMargin(0), 
    _fontHeight(12 * 20), 
    _textDefined(false),
    _restrictDefined(false),
    _underlined(false),
    _bullet(false),
    m_has_focus(false),
    _multiline(false),
    _password(false),
    _text_variable_registered(false),
    _drawBackground(false),
    _drawBorder(false),
    _embedFonts(false),
    _wordWrap(false),
    _html(false),
    _selectable(true)
{
    assert(object);
    // Use the default font (Times New Roman for Windows, Times for Mac
    // according to docs. They don't say what it is for Linux.
    boost::intrusive_ptr<const Font> f = fontlib::get_default_font(); 
    setFont(f);

    init();
}

void
TextField::init()
{
    registerTextVariable();

    reset_bounding_box(0, 0);
}


TextField::~TextField()
{
}

void
TextField::removeTextField()
{
    int depth = get_depth();
    if ( depth < 0 || depth > 1048575 )
    {
        //IF_VERBOSE_ASCODING_ERRORS(
        log_debug("CHECKME: removeTextField(%s): TextField depth (%d) "
            "out of the 'dynamic' zone [0..1048575], won't remove",
            getTarget(), depth);
        //);
        return;
    }

    DisplayObject* p = parent();
    assert(p); // every TextField must have a parent, right ?

    MovieClip* parentSprite = p->to_movie();

    if (!parentSprite) {
        log_error(_("FIXME: attempt to remove a TextField being a child of a %s"),
                typeName(*p));
        return;
    }

    // second argument is arbitrary, see comments above
    // the function declaration in MovieClip.h
    parentSprite->remove_display_object(depth, 0);
}

void
TextField::show_cursor(Renderer& renderer, const SWFMatrix& mat)
{
    if (_textRecords.empty()) {
        return;
    }
    boost::uint16_t x;
    boost::uint16_t y;
    boost::uint16_t h;
    size_t i = cursorRecord();
    SWF::TextRecord record = _textRecords[i];

    x = record.xOffset();
    y = record.yOffset() - record.textHeight() + getLeading();
    h = record.textHeight();
	
	if (!record.glyphs().empty()) {
        for (unsigned int p = 0 ; p < (m_cursor - _recordStarts[i]); ++p) {
            x += record.glyphs()[p].advance;
        }
    }

    const std::vector<point> line = boost::assign::list_of
        (point(x, y))
        (point(x, y + h));
    
    renderer.drawLine(line, rgba(0, 0, 0, 255), mat);
}

size_t
TextField::cursorRecord()
{
    if (_textRecords.empty()) return 0;

    size_t i = 0;

    while (i < _textRecords.size() && m_cursor >= _recordStarts[i]) {
        ++i;
    }
    // TODO: it seems like this could return (size_t) -1, but there's no
    // evidence this is allowed or handled.
    return i - 1;
}

void
TextField::display(Renderer& renderer, const Transform& base)
{
    const DisplayObject::MaskRenderer mr(renderer, *this);

    registerTextVariable();

    const bool drawBorder = getDrawBorder();
    const bool drawBackground = getDrawBackground();

    Transform xform = base * transform();

    // This is a hack to handle device fonts, which are not affected by
    // color transform.
    if (!getEmbedFonts()) xform.colorTransform = SWFCxForm();

    if ((drawBorder || drawBackground) && !_bounds.is_null()) {

        boost::int32_t xmin = _bounds.get_x_min();
        boost::int32_t xmax = _bounds.get_x_max();
        boost::int32_t ymin = _bounds.get_y_min();
        boost::int32_t ymax = _bounds.get_y_max();

        const std::vector<point> coords = boost::assign::list_of
            (point(xmin, ymin))
            (point(xmax, ymin))
            (point(xmax, ymax))
            (point(xmin, ymax));

        rgba borderColor = drawBorder ? getBorderColor() : rgba(0,0,0,0);
        rgba backgroundColor = drawBackground ? getBackgroundColor() :
                                                rgba(0,0,0,0);

        SWFCxForm cx = xform.colorTransform;
            
        if (drawBorder) borderColor = cx.transform(borderColor);
         
        if (drawBackground) backgroundColor = cx.transform(backgroundColor);
        
#ifdef GNASH_DEBUG_TEXTFIELDS
	log_debug("rendering a Pol composed by corners %s", _bounds);
#endif

        renderer.draw_poly(coords, backgroundColor, 
                borderColor, xform.matrix, true);
        
    }

    // Draw our actual text.
    // Using a SWFMatrix to translate to def bounds seems an hack to me.
    // A cleaner implementation is likely correctly setting the
    // _xOffset and _yOffset memebers in glyph records.
    // Anyway, see bug #17954 for a testcase.
    if (!_bounds.is_null()) {
        xform.matrix.concatenate_translation(_bounds.get_x_min(),
                _bounds.get_y_min()); 
    }

    _displayRecords.clear();
    // TODO: work out how leading should be implemented.
    const float fontLeading = 0;

    //offset the lines
    int yoffset = (getFontHeight() + fontLeading) + PADDING_TWIPS;
    size_t recordline;
    for (size_t i = 0; i < _textRecords.size(); ++i) {
        recordline = 0;
        //find the line the record is on
        while (recordline < _line_starts.size() && 
                _line_starts[recordline] <= _recordStarts[i]) {
            ++recordline;
        }
        //offset the line
        _textRecords[i].setYOffset((recordline-_scroll)*yoffset);
        //add the lines we want to the display record
        if (_textRecords[i].yOffset() > 0 &&
            _textRecords[i].yOffset() < _bounds.height()) {
            _displayRecords.push_back(_textRecords[i]);
        }
    }
        
    SWF::TextRecord::displayRecords(renderer, xform, _displayRecords,
            _embedFonts);

    if (m_has_focus && !isReadOnly()) show_cursor(renderer, xform.matrix);
    
    clear_invalidated();
}


void
TextField::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    if (!force && !invalidated()) return; // no need to redraw
    
    ranges.add(m_old_invalidated_ranges);

    const SWFMatrix& wm = getWorldMatrix(*this);

    SWFRect bounds = getBounds();
    bounds.expand_to_rect(m_text_bounding_box); 
    wm.transform(bounds);
    ranges.add(bounds.getRange());            
}

void
TextField::setRestrict(const std::string& restrict)
{
    _restrictDefined = true;
    
    std::string::const_iterator rit = restrict.begin();
    std::string::const_iterator re = restrict.end();
    std::set<wchar_t>::const_iterator locate;

    if (*rit == '^') { //then this is a true RESTRICT pattern, add all chars to _restrictedchars
        for (unsigned int i = 0; i <= 255; ++i) {
            _restrictedchars.insert(char(i));
        }
    } else { //then this is an ALLOW pattern, _restrictedchars should remain empty
        _restrictedchars.clear();
    }
        
    while (rit != re) {
        while (rit != re && *rit != '^') { //This loop allows chars
            if (*rit == '-') {
                log_error(_("invalid restrict string"));
                return;
            } else if (*(rit+1) == '-') {
                if (re - (rit+2) != 0) {
                    unsigned int q = *(rit+2);
                    for (unsigned int p = *rit; p <= q; (++p)){
                        _restrictedchars.insert(char(p));
                    }
                    rit += 3;
                } else {
                    log_error(_("invalid restrict string"));
                    return;
                }
            } else if (*rit == '\\') {
                ++rit;
                _restrictedchars.insert(*rit);
                ++rit;
            } else {
                _restrictedchars.insert(*rit);
                ++rit;
            }
        }
        if (rit != re) {
            ++rit;
        }
        while (rit != re && *rit != '^') { //This loop restricts chars
            locate = _restrictedchars.find(*rit);
            if (*rit == '-') {
                log_error(_("invalid restrict string"));
                return;
            } else if (*(rit+1) == '-') {
                if (re - (rit+2) != 0) {
                    unsigned int q = *(rit+2);
                    for (unsigned int p = *rit; p <= q; ++p){
                        locate = _restrictedchars.find(p);
                        if(locate != _restrictedchars.end()) {
                            _restrictedchars.erase(locate);
                        }
                    }
                    ++rit;
                    ++rit;
                    ++rit;
                } else {
                    log_error(_("invalid restrict string"));
                    return;
                }
            } else if (*rit == '\\') {
                ++rit;
                locate = _restrictedchars.find(*rit);
                if(locate != _restrictedchars.end()) {
                    _restrictedchars.erase(locate);
                }
                ++rit;
            } else {
                if(locate != _restrictedchars.end()) {
                    _restrictedchars.erase(locate);
                }
                ++rit;
            }
        }
        if (rit != re) {
            ++rit;
        }
    }
    _restrict = restrict;
}

void
TextField::replaceSelection(const std::string& replace)
{
    const int version = getSWFVersion(*getObject(this));
    const std::wstring& wstr = utf8::decodeCanonicalString(replace, version);
    
    assert(_selection.second >= _selection.first);
    assert(_selection.second <= _text.size());
    assert(_selection.first <= _text.size());

    // If the text has changed but the selection hasn't, make sure we
    // don't access it out of bounds.
    const size_t start = _selection.first;
    const size_t end = _selection.second;

    const size_t replaceLength = wstr.size();

    _text.replace(start, end - start, wstr);
    _selection = std::make_pair(start + replaceLength, start + replaceLength);
}

void
TextField::setSelection(int start, int end)
{
    if (_text.empty()) {
        _selection = std::make_pair(0, 0);
        return;
    }

    const size_t textLength = _text.size();

    if (start < 0) start = 0;
    else start = std::min<size_t>(start, textLength);

    if (end < 0) end = 0;
    else end = std::min<size_t>(end, textLength);

    // The cursor position is always set to the end value, even if the
    // two values are swapped to obtain the selection. Equal values are
    // fine.
    m_cursor = end;
    if (start > end) std::swap(start, end);

    _selection = std::make_pair(start, end);
}

void
TextField::keyInput(key::code c)
{
    // c is the unique gnash::key::code for a DisplayObject/key.
    // The maximum value is about 265, including function keys.
    // It seems that typing in DisplayObjects outside the Latin-1 set
    // (256 DisplayObject codes, identical to the first 256 of UTF-8)
    // is not supported, though a much greater number UTF-8 codes can be
    // stored and displayed. See utf.h for more information.
    // This is a limit on the number of key codes, not on the
    // capacity of strings.


    setHtml(false); //editable html fields are not yet implemented
    std::wstring s = _text;

    // maybe _text is changed in ActionScript
    m_cursor = std::min<size_t>(m_cursor, _text.size());
    
    size_t cur_cursor = m_cursor;
    size_t previouslinesize = 0;
    size_t nextlinesize = 0;
    size_t manylines = _line_starts.size();
    LineStarts::iterator linestartit = _line_starts.begin();
    LineStarts::const_iterator linestartend = _line_starts.end();

    switch (c) {
        case key::BACKSPACE:
            if (isReadOnly()) return;
            if (m_cursor > 0)
            {
                s.erase(m_cursor - 1, 1);
                m_cursor--;
                setTextValue(s);
            }
            break;

        case key::DELETEKEY:
            if (isReadOnly()) return;
            if (_glyphcount > m_cursor)
            {
                s.erase(m_cursor, 1);
                setTextValue(s);
            }
            break;

        case key::INSERT:        // TODO
            if (isReadOnly()) return;
            break;

        case key::HOME:
            while ( linestartit < linestartend && *linestartit <= m_cursor ) {
                cur_cursor = *linestartit;
                ++linestartit;
            }
            m_cursor = cur_cursor;
            break;
            
        case key::PGUP:
            // if going a page up is too far...
            if(_scroll < _linesindisplay) {
                _scroll = 0;
                m_cursor = 0;
            } else { // go a page up
                _scroll -= _linesindisplay;
                m_cursor = _line_starts[_scroll];
            }
            scrollLines();
            break;
            
        case key::UP:
            while ( linestartit < linestartend && *linestartit <= m_cursor ) {
                cur_cursor = *linestartit;
                ++linestartit;
            }
            //if there is no previous line
            if ( linestartit-_line_starts.begin() - 2 < 0 ) {
                m_cursor = 0;
                break;
            }
            previouslinesize = _textRecords[linestartit-_line_starts.begin() - 2].glyphs().size();
            //if the previous line is smaller
            if (m_cursor - cur_cursor > previouslinesize) {
                m_cursor = *(--(--linestartit)) + previouslinesize;
            } else {
                m_cursor = *(--(--linestartit)) + (m_cursor - cur_cursor);
            }
            if (m_cursor < _line_starts[_scroll] && _line_starts[_scroll] != 0) {
                --_scroll;
            }
            scrollLines();
            break;
            
        case key::END:
            while ( linestartit < linestartend && *linestartit <= m_cursor ) {
                ++linestartit;
            }
            m_cursor = linestartit != linestartend ? *linestartit - 1 : _text.size();
            break;
            
        case key::PGDN:
            //if going another page down is too far...
            if(_scroll + _linesindisplay >= manylines) {
                if(manylines - _linesindisplay <= 0) {
                    _scroll = 0;
                } else {
                    _scroll = manylines - _linesindisplay;
                }
                if(m_cursor < _line_starts[_scroll-1]) {
                    m_cursor = _line_starts[_scroll-1];
                } else {
                    m_cursor = _text.size();
                }
            } else { //go a page down
                _scroll += _linesindisplay;
                m_cursor = _line_starts[_scroll];
            }
            scrollLines();
            break;
            
        case key::DOWN:
        {
            while (linestartit < linestartend &&
                    *linestartit <= m_cursor ) {
                cur_cursor = *linestartit;
                ++linestartit;
            }

            // linestartit should never be before _line_starts.begin()
            const size_t currentLine = linestartit -
                _line_starts.begin();
            
            //if there is no next line
            if (currentLine >= manylines ) {
                m_cursor = _text.size();
                break;
            }
            nextlinesize = _textRecords[currentLine].glyphs().size();
            
            //if the next line is smaller
            if (m_cursor - cur_cursor > nextlinesize) {
                m_cursor = *linestartit + nextlinesize;
            } else { 
                //put the cursor at the same character distance
                m_cursor = *(linestartit) + (m_cursor - cur_cursor);
            }
            if (_line_starts.size() > _linesindisplay &&
                m_cursor >= _line_starts[_scroll+_linesindisplay]) {
                ++_scroll;
            }
            scrollLines();
            break;
        }

        case key::LEFT:
            m_cursor = m_cursor > 0 ? m_cursor - 1 : 0;
            break;

        case key::RIGHT:
            m_cursor = m_cursor < _glyphcount ? m_cursor + 1 :
                                                _glyphcount;
            break;
            
        case key::ENTER:
            if (isReadOnly()) return;
            if (!multiline()) break;

        default:
        
            if (maxChars() != 0) {
                if (_maxChars <= _glyphcount) {
                    break;
                }
            }
            
            if (isReadOnly()) return;
            wchar_t t = static_cast<wchar_t>(
                    gnash::key::codeMap[c][key::ASCII]);
            if (t != 0) {
                
                if (!_restrictDefined) {
                    // Insert one copy of the character
                    // at the cursor position.
                    s.insert(m_cursor, 1, t);
                    m_cursor++;
                } else if (_restrictedchars.count(t)) {
                    // Insert one copy of the character
                    // at the cursor position.
                    s.insert(m_cursor, 1, t);
                    m_cursor++;
                } else if (_restrictedchars.count(tolower(t))) {
                    // restrict substitutes the opposite case
                    s.insert(m_cursor, 1, tolower(t));
                    m_cursor++;
                } else if (_restrictedchars.count(toupper(t))) {
                    // restrict substitutes the opposite case
                    s.insert(m_cursor, 1, toupper(t));
                    m_cursor++;
                }
            }
            setTextValue(s);
    }
    onChanged();
    set_invalidated();
}

void
TextField::mouseEvent(const event_id& ev)
{    
    switch (ev.id())
    {
		case event_id::PRESS:
		{
			movie_root& root = stage();
            boost::int32_t x_mouse, y_mouse;
            boost::tie(x_mouse, y_mouse) = root.mousePosition();
			
			SWFMatrix m = getMatrix(*this);
			
			x_mouse -= m.get_x_translation();
			y_mouse -= m.get_y_translation();
			
			SWF::TextRecord rec;
			
			for (size_t i=0; i < _textRecords.size(); ++i) {
				if 	((x_mouse >  _textRecords[i].xOffset()) && 
					(x_mouse < _textRecords[i].xOffset()+_textRecords[i].recordWidth()) &&
					(y_mouse > _textRecords[i].yOffset()-_textRecords[i].textHeight()) &&
					(y_mouse < _textRecords[i].yOffset())) {
						rec = _textRecords[i];
						break;
					}
			}
			
			if (!rec.getURL().empty()) {
				   root.getURL(rec.getURL(), rec.getTarget(), "",
								MovieClip::METHOD_NONE);
			}

			break;
		}
        default:
            return;
    };
}

InteractiveObject*
TextField::topmostMouseEntity(boost::int32_t x, boost::int32_t y)
{
    if (!visible()) return 0;
    
    // Not selectable, so don't catch mouse events!
    if (!_selectable) return 0;

    SWFMatrix m = getMatrix(*this);
    point p(x, y);
    m.invert().transform(p);

    if (_bounds.point_test(p.x, p.y)) return this;

    return 0;
}

void
TextField::updateText(const std::string& str)
{
    const int version = getSWFVersion(*getObject(this));
    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);
    updateText(wstr);
}

void
TextField::updateText(const std::wstring& wstr)
{
    _textDefined = true;
    if (_text == wstr) return;

    set_invalidated();

    _text = wstr;

    _selection.first = std::min(_selection.first, _text.size());
    _selection.second = std::min(_selection.second, _text.size());

    format_text();
}

void
TextField::updateHtmlText(const std::wstring& wstr)
{
    if (_htmlText == wstr) return;

    set_invalidated();

    _htmlText = wstr;
    format_text();
}

void
TextField::setTextValue(const std::wstring& wstr)
{
    updateHtmlText(wstr);
    updateText(wstr);

    if (!_variable_name.empty() && _text_variable_registered) {
        // TODO: notify MovieClip if we have a variable name !
        VariableRef ref = parseTextVariableRef(_variable_name);
        as_object* tgt = ref.first;
        if (tgt) {
            const int version = getSWFVersion(*getObject(this));
            // we shouldn't truncate, right?
            tgt->set_member(ref.second, utf8::encodeCanonicalString(wstr,
                        version)); 
        }
        else {
            // nothing to do (too early ?)
            log_debug("setTextValue: variable name %s points to a non-existent"
		      "target, I guess we would not be registered if this was"
		      "true, or the sprite we've registered our variable name"
		      "has been unloaded", _variable_name);
        }
    }
}

std::string
TextField::get_text_value() const
{
    // we need the const_cast here because registerTextVariable
    // *might* change our text value, calling the non-const
    // setTextValue().
    // This happens if the TextVariable has not been already registered
    // and during registration comes out to name an existing variable
    // with a pre-existing value.
    const_cast<TextField*>(this)->registerTextVariable();

    const int version = getSWFVersion(*getObject(this));

    return utf8::encodeCanonicalString(_text, version);
}

std::string
TextField::get_htmltext_value() const
{
    const_cast<TextField*>(this)->registerTextVariable();
    const int version = getSWFVersion(*getObject(this));
    return utf8::encodeCanonicalString(_htmlText, version);
}

void
TextField::setTextFormat(TextFormat_as& tf)
{
    //TODO: this is lazy. we should set all the TextFormat variables HERE, i think
    //This is just so we can set individual variables without having to call format_text()
    //This calls format_text() at the end of setting TextFormat
    if (tf.align()) setAlignment(*tf.align());
    if (tf.size()) setFontHeight(*tf.size()); // keep twips
    if (tf.indent()) setIndent(*tf.indent());
    if (tf.blockIndent()) setBlockIndent(*tf.blockIndent());
    if (tf.leading()) setLeading(*tf.leading());
    if (tf.leftMargin()) setLeftMargin(*tf.leftMargin());
    if (tf.rightMargin()) setRightMargin(*tf.rightMargin());
    if (tf.color()) setTextColor(*tf.color());
    if (tf.underlined()) setUnderlined(*tf.underlined());
    if (tf.bullet()) setBullet(*tf.bullet());
    setDisplay(tf.display());
	if (tf.tabStops()) setTabStops(*tf.tabStops());
	
	// NEED TO IMPLEMENT THESE TWO
	if (tf.url()) setURL(*tf.url());
	if (tf.target()) setTarget(*tf.target());
    
    format_text();
}

float
TextField::align_line(TextAlignment align, int last_line_start_record, float x)
{
    float width = _bounds.width(); 
    float right_margin = getRightMargin();

    float extra_space = (width - right_margin) - x - PADDING_TWIPS;

    if (extra_space <= 0.0f) {
#ifdef GNASH_DEBUG_TEXTFIELDS
        log_debug("TextField text doesn't fit in its boundaries: "
                "width %g, margin %g - nothing to align",
                width, right_margin);
#endif
        return 0.0f;
    }

    float shift_right = 0.0f;

    switch (align) {
        case ALIGN_LEFT:
            // Nothing to do; already aligned left.
            return 0.0f;
        case ALIGN_CENTER:
            // Distribute the space evenly on both sides.
            shift_right = extra_space / 2;
            break;
        case ALIGN_RIGHT:
            // Shift all the way to the right.
            shift_right = extra_space;
            break;
        case ALIGN_JUSTIFY:
            // What should we do here?
            break;
    }

    // Shift the beginnings of the records on this line.
    for (size_t i = last_line_start_record; i < _textRecords.size(); ++i) {
        SWF::TextRecord& rec = _textRecords[i];
        rec.setXOffset(rec.xOffset() + shift_right); 
    }
    return shift_right;
}

boost::intrusive_ptr<const Font>
TextField::setFont(boost::intrusive_ptr<const Font> newfont)
{
    if (newfont == _font) return _font;

    boost::intrusive_ptr<const Font> oldfont = _font;
    set_invalidated();
    _font = newfont; 
    format_text();
    return oldfont;  
}


void
TextField::insertTab(SWF::TextRecord& rec, boost::int32_t& x, float scale)
{
     // tab (ASCII HT)
    const int space = 32;
    int index = rec.getFont()->get_glyph_index(space, _embedFonts); 
    if (index == -1) {
        IF_VERBOSE_MALFORMED_SWF (
          log_error(_("TextField: missing glyph for space char (needed "
                  "for TAB). Make sure DisplayObject shapes for font "
                  "%s are being exported into your SWF file."),
                rec.getFont()->name());
        );
    }
    else {
        // TODO: why is there a copy of the vector?
		std::vector<int> tabStops = _tabStops;
        
        std::sort(_tabStops.begin(), _tabStops.end()); 

        if (!_tabStops.empty()) {
            int tab = _tabStops.back() + 1;
            
            for (size_t i = 0; i < tabStops.size(); ++i) {        
                if (tabStops[i] > x) {
                    if((tabStops[i] - x) < tab) {
                        tab = tabStops[i] - x;
                    }
				}

            }

			// This is necessary in case the number of tabs in the text
			// are more than the actual number of tabStops inside the 
			// vector
			if (tab != _tabStops.back() + 1) {
				SWF::TextRecord::GlyphEntry ge;
				ge.index = rec.getFont()->get_glyph_index(32, _embedFonts);
				ge.advance = tab;
				rec.addGlyph(ge);
				x+=ge.advance;
			}
        }
        else {
            SWF::TextRecord::GlyphEntry ge;
            ge.index = index;
            ge.advance = scale * rec.getFont()->get_advance(index, 
                    _embedFonts);

            const int tabstop = 4;
            rec.addGlyph(ge, tabstop);
            x += ge.advance * tabstop;
        }
    }
}

void
TextField::format_text()
{
    _textRecords.clear();
    _line_starts.clear();
    _recordStarts.clear();
    _glyphcount = 0;

    _recordStarts.push_back(0);
		
    // nothing more to do if text is empty
    if (_text.empty()) {
        // TODO: should we still reset _bounds if autoSize != AUTOSIZE_NONE ?
        //       not sure we should...
        reset_bounding_box(0, 0);
        return;
    }
    
    AutoSize autoSize = getAutoSize();
    if (autoSize != AUTOSIZE_NONE) {
        // When doing WordWrap we don't want to change
        // the boundaries. See bug #24348
        if (!doWordWrap()) {
            _bounds.set_to_rect(0, 0, 0, 0); // this is correct for 'true'
        }
    }

    // FIXME: I don't think we should query the definition
    // to find the appropriate font to use, as ActionScript
    // code should be able to change the font of a TextField
    if (!_font) {
        log_error(_("No font for TextField!"));
        return;
    }

    boost::uint16_t fontHeight = getFontHeight();
    const float scale = fontHeight /
        static_cast<float>(_font->unitsPerEM(_embedFonts));

    // TODO: work out how leading affects things.
    const float fontLeading = 0;

    const boost::uint16_t leftMargin = getLeftMargin();
    const boost::uint16_t indent = getIndent();
    const boost::uint16_t blockIndent = getBlockIndent();
    const bool underlined = getUnderlined();

    /// Remember the current bounds for autosize.
    SWFRect oldBounds(_bounds);

    SWF::TextRecord rec;    // one to work on
    rec.setFont(_font.get());
    rec.setUnderline(underlined);
    rec.setColor(getTextColor()); 
    rec.setXOffset(PADDING_TWIPS + 
            std::max(0, leftMargin + indent + blockIndent));
    rec.setYOffset(PADDING_TWIPS + fontHeight + fontLeading);
    rec.setTextHeight(fontHeight);
	
	// create in textrecord.h
	rec.setURL(_url);
	rec.setTarget(_target);
    
    // BULLET CASE:
                
    // First, we indent 10 spaces, and then place the bullet
    // character (in this case, an asterisk), then we pad it
    // again with 10 spaces
    // Note: this works only for additional lines of a 
    // bulleted list, so that is why there is a bullet format
    // in the beginning of format_text()
    if (_bullet) {
        int space = rec.getFont()->get_glyph_index(32, _embedFonts);

        SWF::TextRecord::GlyphEntry ge;
        ge.index = space;
        ge.advance = scale * rec.getFont()->get_advance(space, _embedFonts);
        rec.addGlyph(ge, 5);

        // We use an asterisk instead of a bullet
        int bullet = rec.getFont()->get_glyph_index(42, _embedFonts);
        ge.index = bullet;
        ge.advance = scale * rec.getFont()->get_advance(bullet, _embedFonts);
        rec.addGlyph(ge);
        
        space = rec.getFont()->get_glyph_index(32, _embedFonts);
        ge.index = space;
        ge.advance = scale * rec.getFont()->get_advance(space, _embedFonts);
        rec.addGlyph(ge, 4);
    }

    boost::int32_t x = static_cast<boost::int32_t>(rec.xOffset());
    boost::int32_t y = static_cast<boost::int32_t>(rec.yOffset());

    // Start the bbox at the upper-left corner of the first glyph.
    //reset_bounding_box(x, y + fontHeight); 
    
    int last_code = -1; // only used if _embedFonts
    int last_space_glyph = -1;
    size_t last_line_start_record = 0;

    _line_starts.push_back(0);
    
    // String iterators are very sensitive to 
    // potential changes to the string (to allow for copy-on-write).
    // So there must be no external changes to the string or
    // calls to most non-const member functions during this loop.
    // Especially not c_str() or data().
    std::wstring::const_iterator it = _text.begin();
    const std::wstring::const_iterator e = _text.end();

    ///handleChar takes care of placing the glyphs    
    handleChar(it, e, x, y, rec, last_code, last_space_glyph,
            last_line_start_record);
                
    // Expand bounding box to include the whole text (if autoSize and wordWrap
    // is not in operation.
    if (_autoSize != AUTOSIZE_NONE && !doWordWrap())
    {
        _bounds.expand_to_point(x + PADDING_TWIPS, y + PADDING_TWIPS);

        if (_autoSize == AUTOSIZE_RIGHT) {
            /// Autosize right expands from the previous right margin.
            SWFMatrix m;

            m.set_x_translation(oldBounds.get_x_max() - _bounds.width());
            m.transform(_bounds);
        }
        else if (_autoSize == AUTOSIZE_CENTER) {
            // Autosize center expands from the previous center.
            SWFMatrix m;
            m.set_x_translation(oldBounds.get_x_min() + oldBounds.width() / 2.0 - 
                    _bounds.width() / 2.0);
            m.transform(_bounds);
        }
    }

    // Add the last line to our output.
    _textRecords.push_back(rec);
	
    // align the last (or single) line
    align_line(getTextAlignment(), last_line_start_record, x);

    scrollLines();
	
    set_invalidated(); //redraw
    
}

void
TextField::scrollLines()
{
    boost::uint16_t fontHeight = getFontHeight();
    const float fontLeading = 0;

    _linesindisplay = _bounds.height() / (fontHeight + fontLeading + PADDING_TWIPS);
    if (_linesindisplay > 0) { //no need to place lines if we can't fit any
        size_t manylines = _line_starts.size();
        size_t lastvisibleline = _scroll + _linesindisplay;
        size_t line = 0;

        // If there aren't as many lines as we have scrolled, display the
        // end of the text.
        if (manylines < _scroll) {
            _scroll = manylines - _linesindisplay;
            return;
        }

        // which line is the cursor on?
        while (line < manylines && _line_starts[line] <= m_cursor) {
            ++line;
        }

        if (manylines - _scroll <= _linesindisplay) {
            // This is for if we delete a line
            if (manylines < _linesindisplay) _scroll = 0;
            else {
                _scroll = manylines - _linesindisplay;
            }
        } else if (line < _scroll) {
            //if we are at a higher position, scroll the lines down
            _scroll -= _scroll - line;
        } else if (manylines > _scroll + _linesindisplay) {
            //if we are at a lower position, scroll the lines up
            if (line >= (_scroll+_linesindisplay)) {
                _scroll += line - (lastvisibleline);
            }
        }
    }
}

void
TextField::newLine(boost::int32_t& x, boost::int32_t& y, 
				   SWF::TextRecord& rec, int& last_space_glyph,
				LineStarts::value_type& last_line_start_record, float div)
{
    // newline.
    LineStarts::iterator linestartit = _line_starts.begin();
    LineStarts::const_iterator linestartend = _line_starts.end();
    
    // TODO: work out how leading affects things.
    const float leading = 0;
    
    // Close out this stretch of glyphs.
    ++_glyphcount;
    _textRecords.push_back(rec);
    _recordStarts.push_back(_glyphcount);
    align_line(getTextAlignment(), last_line_start_record, x);

    // Expand bounding box to include last column of text ...
    if (!doWordWrap() && _autoSize != AUTOSIZE_NONE) {
        _bounds.expand_to_point(x + PADDING_TWIPS, y + PADDING_TWIPS);
    }

    // new paragraphs get the indent.
    x = std::max(0, getLeftMargin() + getIndent() + getBlockIndent()) +
        PADDING_TWIPS;
    y += div * (getFontHeight() + leading);
    if (y >= _bounds.height()) {
        ++_maxScroll;
    }        
            
    // Start a new record on the next line. Other properties of the
    // TextRecord should be left unchanged.
    rec.clearGlyphs();
    rec.setXOffset(x);
    rec.setYOffset(y);

    last_space_glyph = -1;
    last_line_start_record = _textRecords.size();
                         
    linestartit = _line_starts.begin();
    linestartend = _line_starts.end();
    //Fit a line_start in the correct place
    const size_t currentPos = _glyphcount;

    while (linestartit < linestartend && *linestartit < currentPos)
    {
        ++linestartit;
    }
    _line_starts.insert(linestartit, currentPos);

    // BULLET CASE:
                
    // First, we indent 10 spaces, and then place the bullet
    // character (in this case, an asterisk), then we pad it
    // again with 10 spaces
    // Note: this works only for additional lines of a 
    // bulleted list, so that is why there is a bullet format
    // in the beginning of format_text()
    if (_bullet)
    {
        int space = rec.getFont()->get_glyph_index(32, _embedFonts);
        SWF::TextRecord::GlyphEntry ge;
        ge.index = space;

        const float scale = getFontHeight() /
            static_cast<float>(_font->unitsPerEM(_embedFonts));

        ge.advance = scale * rec.getFont()->get_advance(space, _embedFonts);
                  
        rec.addGlyph(ge,5);
        _glyphcount += 5;
                    
        int bullet = rec.getFont()->get_glyph_index(42, _embedFonts);
        ge.index = bullet;
        ge.advance = scale * rec.getFont()->get_advance(bullet, _embedFonts);
        rec.addGlyph(ge);
        ++_glyphcount;

        ge.index = space;
        ge.advance = scale * rec.getFont()->get_advance(space, _embedFonts);
        
        rec.addGlyph(ge,4);
        _glyphcount += 4;
    }
}

void
TextField::handleChar(std::wstring::const_iterator& it,
        const std::wstring::const_iterator& e, boost::int32_t& x,
        boost::int32_t& y, SWF::TextRecord& rec, int& last_code,
        int& last_space_glyph, LineStarts::value_type& last_line_start_record)
{
    LineStarts::iterator linestartit = _line_starts.begin();
    LineStarts::const_iterator linestartend = _line_starts.end();
    
    float scale = _fontHeight /
        static_cast<float>(_font->unitsPerEM(_embedFonts)); 
    float fontDescent = _font->descent(_embedFonts) * scale; 

    // TODO: work out how leading should be implemented.
    const float leading = 0;
    const float fontLeading = 0;
    
    boost::uint32_t code = 0;
    while (it != e)
    {
        code = *it++;
        if (!code) break;

        if ( _embedFonts )
        {
            x += rec.getFont()->get_kerning_adjustment(last_code, 
                    static_cast<int>(code)) * scale;
            last_code = static_cast<int>(code);
        }

        // Expand the bounding-box to the lower-right corner of each glyph as
        // we generate it.
        m_text_bounding_box.expand_to_point(x, y + fontDescent);
        switch (code)
        {
            case 27:
                // Ignore escape
                break;
            case 9:
                insertTab(rec, x, scale);
                break;
            case 8:
                // Backspace 

                // This is a limited hack to enable overstrike effects.
                // It backs the cursor up by one DisplayObject and then continues
                // the layout.  E.g. you can use this to display an underline
                // cursor inside a simulated text-entry box.
                //
                // ActionScript understands the '\b' escape sequence
                // for inserting a BS DisplayObject.
                //
                // ONLY WORKS FOR BACKSPACING OVER ONE CHARACTER, WON'T BS
                // OVER NEWLINES, ETC.

                if (!rec.glyphs().empty())
                {
                    // Peek at the previous glyph, and zero out its advance
                    // value, so the next char overwrites it.
                    float advance = rec.glyphs().back().advance;
                    x -= advance; 
                    // Remove one glyph
                    rec.clearGlyphs(1);
                }
                continue;
            case 13:
            case 10:
            {
                newLine(x,y,rec,last_space_glyph,last_line_start_record,1.0);
                break;
            }
            case '<':
                if (doHtml())
                {
                    //close out this stretch of glyphs
                    _textRecords.push_back(rec);
                    rec.clearGlyphs();
                    _recordStarts.push_back(_glyphcount);
                    if (*it == '/') {
                        while (it != e && *it != '>') {
                            ++it;
                        }
                        ++it;
                        return;
                    }
                    LOG_ONCE(log_debug("HTML in a text field is unsupported, "
                                         "gnash will just ignore the tags and "
                                         "print their content"));
            
                    std::wstring discard;
                    std::map<std::string,std::string> attributes;
                    SWF::TextRecord newrec;
                    newrec.setFont(rec.getFont());
                    newrec.setUnderline(rec.underline());
                    newrec.setColor(rec.color()); 
                    newrec.setTextHeight(rec.textHeight());
                    newrec.setXOffset(x);
                    newrec.setYOffset(y);
                    bool selfclosing = false;
                    bool complete = parseHTML(discard, attributes, it, e, selfclosing);
                    std::string s(discard.begin(), discard.end());

                    std::map<std::string,std::string>::const_iterator attloc;
                    
                    if (!complete) {
                        //parsing went wrong
                        continue;
                    } else {
                        // Don't think this is the best way to match with
                        // tags...
                        // TODO: assumes tags are properly nested. This isn't
                        // correct.
                        if (s == "U") {
                            //underline
                            newrec.setUnderline(true);
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                        }
                        else if (s == "A") {
                            // anchor (blue text).
							rgba color(0, 0, 0xff, 0xff);
							newrec.setColor(color);
							newrec.setUnderline(true);
							attloc = attributes.find("HREF");
							if (attloc != attributes.end()) {
								newrec.setURL(attloc->second);
							}
							attloc = attributes.find("TARGET");
							if (attloc !=attributes.end()) {
								newrec.setTarget(attloc->second);
							}
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                        }
                        else if (s == "B") {
                            //bold
                            Font* boldfont =
                                fontlib::get_font(rec.getFont()->name(),
                                    true, rec.getFont()->isItalic());
                            newrec.setFont(boldfont);
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                        }
                        else if (s == "FONT") {
                            //font
                            boost::uint16_t originalsize = _fontHeight;
                            attloc = attributes.find("COLOR");
                            if (attloc != attributes.end()) {
                                std::string hexval(attloc->second);
                                if (hexval.empty() || hexval[0] != '#') {
                                    // FIXME: should this be a log_aserror
                                    //        or log_unimpl ? It is triggered
                                    //        by TextFieldHTML.as
                                    log_error(_("Unexpected value '%s' in TextField font color attribute"),
                                        hexval);
                                }
                                else {
                                    hexval.erase(0, 1);
                                    // font COLOR attribute
                                    const rgba color =
                                        colorFromHexString(hexval);
                                    newrec.setColor(color);
                                }
                            }
                            attloc = attributes.find("FACE");
                            if (attloc != attributes.end()) {
                                if (attloc->second.empty()) {
                                    IF_VERBOSE_ASCODING_ERRORS(
                                         log_aserror(_("Expected a font name in FACE attribute."))
                                    );
                                } else {
                                    //font FACE attribute
                                    Font* newfont = 
                                        fontlib::get_font(attloc->second,
                                        rec.getFont()->isBold(),
                                        rec.getFont()->isItalic());
                                    newrec.setFont(newfont);
                                }
                            }
                            attloc = attributes.find("SIZE");
                            if (attloc != attributes.end()) {
                                //font SIZE attribute
                                std::string firstchar = attloc->second.substr(0,1);
                                if (firstchar == "+") {
                                    newrec.setTextHeight(rec.textHeight() +
                                        
										(pixelsToTwips(std::strtol(
                                        attloc->second.substr(1,attloc->second.length()-1).data(),
                                        NULL,10))));
                                    newrec.setYOffset(PADDING_TWIPS +
                                        newrec.textHeight() +
                                        (fontLeading - fontDescent));
                                    _fontHeight += pixelsToTwips(std::strtol(
                                        attloc->second.substr(1,attloc->second.length()-1).data(),
                                        NULL,10));
                                } else if (firstchar == "-") {
                                    newrec.setTextHeight(rec.textHeight() -
                                        (pixelsToTwips(std::strtol(
                                        attloc->second.substr(1,attloc->second.length()-1).data(),
                                        NULL,10))));
                                    newrec.setYOffset(PADDING_TWIPS +
                                        newrec.textHeight() +
                                        (fontLeading - fontDescent));
                                    _fontHeight -= pixelsToTwips(std::strtol(
                                        attloc->second.substr(1,attloc->second.length()-1).data(),
                                        NULL,10));
                                } else {
                                    newrec.setTextHeight(pixelsToTwips(std::strtol(
                                        attloc->second.data(), NULL, 10)));
                                    newrec.setYOffset(PADDING_TWIPS + newrec.textHeight() +
                                        (fontLeading - fontDescent));
                                    _fontHeight = pixelsToTwips(std::strtol(
                                        attloc->second.data(), NULL, 10));
                                }
                            }
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                            _fontHeight = originalsize;
                            y = newrec.yOffset();
                        }
                        else if (s == "IMG") {
                            //image
                            log_unimpl(_("<img> html tag in TextField"));
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                        }
                        else if (s == "I") {
                            //italic
                            Font* italicfont = 
                                fontlib::get_font(rec.getFont()->name(),
                                    rec.getFont()->isBold(), true);
                            newrec.setFont(italicfont);
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                        } else if (s == "LI") {
                            //list item (bullet)	
							int space = newrec.getFont()->get_glyph_index(32, _embedFonts);
							SWF::TextRecord::GlyphEntry ge;
							ge.index = space;
							ge.advance = scale * newrec.getFont()->get_advance(space, _embedFonts);
							newrec.addGlyph(ge, 5);

							// We use an asterisk instead of a bullet
							int bullet = newrec.getFont()->get_glyph_index(42, _embedFonts);
							ge.index = bullet;
							ge.advance = scale * newrec.getFont()->get_advance(bullet, _embedFonts);
							newrec.addGlyph(ge);
							
							space = newrec.getFont()->get_glyph_index(32, _embedFonts);
							ge.index = space;
							ge.advance = scale * newrec.getFont()->get_advance(space, _embedFonts);
							newrec.addGlyph(ge, 4);

							handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
							newLine(x, y, newrec, last_space_glyph,
                                    last_line_start_record, 1.0);
                        }
                        else if (s == "SPAN") {
                            //span
                            log_unimpl(_("<span> html tag in TextField"));
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                        }
                        else if (s == "TEXTFORMAT") {
                            log_debug("in textformat");
                            //textformat
                            boost::uint16_t originalblockindent = getBlockIndent();
                            boost::uint16_t originalindent = getIndent();
                            boost::uint16_t originalleading = getLeading();
                            boost::uint16_t originalleftmargin = getLeftMargin();
                            boost::uint16_t originalrightmargin = getRightMargin();
                            std::vector<int> originaltabstops = getTabStops();
                            attloc = attributes.find("BLOCKINDENT");
                            if (attloc != attributes.end()) {
                                //textformat BLOCKINDENT attribute
                                setBlockIndent(pixelsToTwips(std::strtol(
                                        attloc->second.data(), NULL, 10)));
                                if (newrec.xOffset() == std::max(0, originalleftmargin +
                                    originalindent + originalblockindent) + PADDING_TWIPS) {
                                    //if beginning of line, indent
                                    x = std::max(0, getLeftMargin() +
                                        getIndent() + getBlockIndent())
                                         + PADDING_TWIPS;
                                    newrec.setXOffset(x);
                                }
                            }
                            attloc = attributes.find("INDENT");
                            if (attloc != attributes.end()) {
                                //textformat INDENT attribute
                                setIndent(pixelsToTwips(std::strtol(
                                    attloc->second.data(), NULL, 10)));
                                if (newrec.xOffset() == std::max(0, originalleftmargin +
                                    originalindent + getBlockIndent()) + PADDING_TWIPS) {
                                    //if beginning of line, indent
                                    x = std::max(0, getLeftMargin() +
                                        getIndent() + getBlockIndent())
                                         + PADDING_TWIPS;
                                    newrec.setXOffset(x);
                                }
                            }
                            attloc = attributes.find("LEADING");
                            if (attloc != attributes.end()) {
                                //textformat LEADING attribute
                                setLeading(pixelsToTwips(std::strtol(
                                        attloc->second.data(), NULL, 10)));
                            }
                            attloc = attributes.find("LEFTMARGIN");
                            if (attloc != attributes.end()) {
                                //textformat LEFTMARGIN attribute
                                setLeftMargin(pixelsToTwips(std::strtol(
                                        attloc->second.data(), NULL, 10)));
                                if (newrec.xOffset() == std::max(0, originalleftmargin +
                                    getIndent() + getBlockIndent()) + PADDING_TWIPS) {
                                    //if beginning of line, indent
                                    x = std::max(0, getLeftMargin() +
                                        getIndent() + getBlockIndent())
                                         + PADDING_TWIPS;
                                    newrec.setXOffset(x);
                                }
                            }
                            attloc = attributes.find("RIGHTMARGIN");
                            if (attloc != attributes.end()) {
                                //textformat RIGHTMARGIN attribute
                                setRightMargin(pixelsToTwips(std::strtol(
                                        attloc->second.data(), NULL, 10)));
                                //FIXME:Should not apply this to this line if we are not at
                                //beginning of line. Not sure how to do that.
                            }
                            attloc = attributes.find("TABSTOPS");
                            if (attloc != attributes.end()) {
                                //textformat TABSTOPS attribute
                                log_unimpl(_("html <textformat> tag tabstops attribute"));
                            }
                            handleChar(it, e, x, y, newrec, last_code,
                                    last_space_glyph, last_line_start_record);
                            setBlockIndent(originalblockindent);
                            setIndent(originalindent);
                            setLeading(originalleading);
                            setLeftMargin(originalleftmargin);
                            setRightMargin(originalrightmargin);
                            setTabStops(originaltabstops);
                        }
                        else if (s == "P") {
                            //paragraph
                            if (_display == TEXTFORMAT_BLOCK) {
                                handleChar(it, e, x, y, newrec, last_code,
                                        last_space_glyph,
                                        last_line_start_record);
                                newLine(x, y, rec, last_space_glyph,
                                        last_line_start_record, 1.0);
								newLine(x, y, rec, last_space_glyph,
                                        last_line_start_record, 1.5);
                            }
                            else {
                                handleChar(it, e, x, y, newrec, last_code,
                                        last_space_glyph,
                                        last_line_start_record);
                            }
                        }
                        else if (s == "BR" || s == "SBR") {
                            //line break
							newLine(x, y, rec, last_space_glyph,
										last_line_start_record, 1.0);
                        }
                        else {
                            log_debug("<%s> tag is unsupported", s);
                            if (!selfclosing) { //then recurse, look for closing tag
                            handleChar(it, e, x, y, newrec, last_code,
                                                last_space_glyph, last_line_start_record);
                            }
                        }
                    }
                    rec.setXOffset(x);
                    rec.setYOffset(y);
                    continue;
                }
                // If HTML isn't enabled, carry on and insert the glyph.
                // FIXME: do we also want to be changing last_space_glyph?
                //        ...because we are...
            case 32:
                last_space_glyph = rec.glyphs().size();
                // Don't break, as we still need to insert the space glyph.

            default:
            {
                if ( password() )
                {    
                    SWF::TextRecord::GlyphEntry ge;
                    int bullet = rec.getFont()->get_glyph_index(42, _embedFonts);
                    ge.index = bullet;
                    ge.advance = scale * rec.getFont()->get_advance(bullet, 
                        _embedFonts);
                    rec.addGlyph(ge);
                    ++_glyphcount;
                    break;
                }
                // The font table holds up to 65535 glyphs. Casting
                // from uint32_t would, in the event that the code
                // is higher than 65535, result in the wrong DisplayObject
                // being chosen. Flash can currently only handle 16-bit
                // values.
                int index = rec.getFont()->get_glyph_index(
                        static_cast<boost::uint16_t>(code), _embedFonts);

                IF_VERBOSE_MALFORMED_SWF (
                    if (index == -1)
                    {
                        // Missing glyph! Log the first few errors.
                        static int s_log_count = 0;
                        if (s_log_count < 10)
                        {
                            s_log_count++;
                            if (_embedFonts)
                            {
                                log_swferror(_("TextField: missing embedded "
                                    "glyph for char %d. Make sure DisplayObject "
                                    "shapes for font %s are being exported "
                                    "into your SWF file"),
                                    code, _font->name());
                            }
                            else
                            {
                                log_swferror(_("TextField: missing device "
                                    "glyph for char %d. Maybe you don't have "
                                    "font '%s' installed in your system."),
                                    code, _font->name());
                            }
                        }

                        // Drop through and use index == -1; this will display
                        // using the empty-box glyph
                    }
                );
				
                SWF::TextRecord::GlyphEntry ge;
                ge.index = index;
                ge.advance = scale * rec.getFont()->get_advance(index, 
                        _embedFonts);

                rec.addGlyph(ge);

                x += ge.advance;
                ++_glyphcount;
            }
        }

        float width = _bounds.width();
        if (x >= width - getRightMargin() - PADDING_TWIPS)
        {
#ifdef GNASH_DEBUG_TEXT_FORMATTING
            log_debug("Text in TextField %s exceeds width [ _bounds %s ]",
                    getTarget(), _bounds);
#endif

            // No wrap and no resize: truncate
            if (!doWordWrap() && getAutoSize() == AUTOSIZE_NONE)
            {
#ifdef GNASH_DEBUG_TEXT_FORMATTING
                log_debug(" wordWrap=false, autoSize=none");
#endif 
                // Truncate long line, but keep expanding text box
                bool newlinefound = false;
                while (it != e)
                {
                    code = *it++;
                    if (_embedFonts)
                    {
                        x += rec.getFont()->get_kerning_adjustment(last_code,
                                static_cast<int>(code)) * scale;
                        last_code = code;
                    }
                    // Expand the bounding-box to the lower-right corner
                    // of each glyph, even if we don't display it 
                    m_text_bounding_box.expand_to_point(x, y + fontDescent);
#ifdef GNASH_DEBUG_TEXT_FORMATTING
                    log_debug("Text bbox expanded to %s (width: %f)",
                            m_text_bounding_box, m_text_bounding_box.width());
#endif

                    if (code == 13 || code == 10)
                    {
                        newlinefound = true;
                        break;
                    }

                    int index = rec.getFont()->get_glyph_index(
                            static_cast<boost::uint16_t>(code), _embedFonts);
                    x += scale * rec.getFont()->get_advance(index, _embedFonts);

                }
                if (!newlinefound) break;
            }
            else if (doWordWrap()) {

#ifdef GNASH_DEBUG_TEXT_FORMATTING
                log_debug(" wordWrap=true");
#endif

                // Insert newline if there's space or autosize != none

                // Close out this stretch of glyphs.
                _textRecords.push_back(rec);

                float previous_x = x;
                x = std::max(0, getLeftMargin() + getBlockIndent()) + PADDING_TWIPS;
                y += _fontHeight + leading;
                if (y >= _bounds.height()) {
                    ++_maxScroll;
                }

                // Start a new record on the next line.
                rec.clearGlyphs();
                rec.setXOffset(x);
                rec.setYOffset(y);

                // TODO : what if m_text_glyph_records is empty ?
                // Is it possible ?
                assert(!_textRecords.empty());
                SWF::TextRecord& last_line = _textRecords.back();
                
                linestartit = _line_starts.begin();
                linestartend = _line_starts.end();
                if (last_space_glyph == -1)
                {
                    // Pull the previous glyph down onto the
                    // new line.
                    if (!last_line.glyphs().empty())
                    {
                        rec.addGlyph(last_line.glyphs().back());
                        x += last_line.glyphs().back().advance;
                        previous_x -= last_line.glyphs().back().advance;
                        last_line.clearGlyphs(1);
                        //record the new line start
                        //
                        const size_t currentPos = _glyphcount;
                        while (linestartit != linestartend &&
                                *linestartit + 1 <= currentPos)
                        {
                            ++linestartit;
                        }
                        _line_starts.insert(linestartit, currentPos);
                        _recordStarts.push_back(currentPos);
                    }
                } else {
                    // Move the previous word down onto the next line.

                    previous_x -= last_line.glyphs()[last_space_glyph].advance;

                    const SWF::TextRecord::Glyphs::size_type lineSize =
                        last_line.glyphs().size();
                    for (unsigned int i = last_space_glyph + 1; i < lineSize;
                            ++i)
                    {
                        rec.addGlyph(last_line.glyphs()[i]);
                        x += last_line.glyphs()[i].advance;
                        previous_x -= last_line.glyphs()[i].advance;
                    }
                    last_line.clearGlyphs(lineSize - last_space_glyph);
                    
                    // record the position at the start of this line as
                    // a line_start
                    const size_t linestartpos = _glyphcount -
                            rec.glyphs().size();

                    while (linestartit < linestartend &&
                            *linestartit < linestartpos)
                    {
                        ++linestartit;
                    }
                    _line_starts.insert(linestartit, linestartpos);
                    _recordStarts.push_back(linestartpos);
                }

                align_line(getTextAlignment(), last_line_start_record, previous_x);

                last_space_glyph = -1;
                last_line_start_record = _textRecords.size();
                
            }
            else
            {
#ifdef GNASH_DEBUG_TEXT_FORMATTING
                log_debug(" wordWrap=%d, autoSize=%d", _wordWrap, _autoSize);
#endif 
            }
        }
    }
}

int
TextField::getDefinitionVersion() const
{
    // TODO: work out if this correct.
    return get_root()->getDefinitionVersion();
}


TextField::VariableRef
TextField::parseTextVariableRef(const std::string& variableName) const
{
    VariableRef ret;
    ret.first = 0;

#ifdef DEBUG_DYNTEXT_VARIABLES
    log_debug("VariableName: %s", variableName);
#endif

    /// Why isn't get_environment const again ?
    const as_environment& env = const_cast<TextField*>(this)->get_environment();

    as_object* target = getObject(env.target());
    if (!target) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Current environment has no target, "
                "can't bind VariableName (%s) associated to "
                "text field. Gnash will try to register "
                "again on next access."), variableName);
        );
        return ret;
    }

    // If the variable string contains a path, we extract
    // the appropriate target from it and update the variable
    // name. We copy the string so we can assign to it if necessary.
    std::string parsedName = variableName;
    std::string path, var;
    if (parsePath(variableName, path, var)) {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug("Variable text Path: %s, Var: %s", path, var);
#endif
        // find target for the path component
        // we use our parent's environment for this
        target = findObject(env, path);

        parsedName = var;
    }

    if (!target) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("VariableName associated to text field refers "
                    "to an unknown target (%s). It is possible that the "
                    "DisplayObject will be instantiated later in the SWF "
                    "stream. Gnash will try to register again on next "
                    "access."), path);
        );
        return ret;
    }

    ret.first = target;
    ret.second = getURI(getVM(*object()), parsedName);

    return ret;
}

void
TextField::registerTextVariable()
{
//#define DEBUG_DYNTEXT_VARIABLES 1

#ifdef DEBUG_DYNTEXT_VARIABLES
    log_debug("registerTextVariable() called");
#endif

    if (_text_variable_registered) {
        return;
    }

    if (_variable_name.empty()) {
        _text_variable_registered = true;
        return;
    }

    VariableRef varRef = parseTextVariableRef(_variable_name);
    as_object* target = varRef.first;
    if (!target) {
        log_debug("VariableName associated to text field (%s) refer to "
                    "an unknown target. It is possible that the DisplayObject "
                    "will be instantiated later in the SWF stream. "
                    "Gnash will try to register again on next access.",
                _variable_name);
        return;
    }

    const ObjectURI& key = varRef.second;
    as_object* obj = getObject(this);
    const int version = getSWFVersion(*obj);
    
    // check if the VariableName already has a value,
    // in that case update text value
    as_value val;
    if (target->get_member(key, &val)) {
        // TODO: pass environment to to_string ?
        setTextValue(utf8::decodeCanonicalString(val.to_string(), version));
    }
    else if (_textDefined) {
        as_value newVal = as_value(utf8::encodeCanonicalString(_text, version));
        target->set_member(key, newVal);
    }

    MovieClip* sprite = get<MovieClip>(target);

    if (sprite) {
        // add the textfield variable to the target sprite
        // TODO: have set_textfield_variable take a string_table::key instead ?
        sprite->set_textfield_variable(key, this);

    }
    _text_variable_registered = true;
}

/// Parses an HTML tag (between < and >) and puts
/// the contents into tag. Returns false if the
/// tag was incomplete. The iterator is moved to after
/// the closing tag or the end of the string.
bool
TextField::parseHTML(std::wstring& tag,
        std::map<std::string, std::string>& attributes,
        std::wstring::const_iterator& it,
        const std::wstring::const_iterator& e,
        bool& selfclosing) const
{
    while (it != e && *it != ' ') {
        if (*it == '/') {
            ++it;
            if (*it == '>') {
                ++it;
                selfclosing = true;
                return true;
            } else {
                while (it != e) {
                    ++it;
                }
                log_error(_("invalid html tag"));
                return false;
            }
        }
        if (*it == '>') {
            ++it;
            return true;
        }
        
        // Check for NULL character
        if (*it == 0) {
            log_error(_("found NULL character in htmlText"));
            return false;
        }
        tag.push_back(std::toupper(*it));
        ++it;
    }
    while (it != e && *it == ' ') {
        ++it; //skip over spaces
    }
    if (*it == '>') {
        ++it;
        return true;
    }
    if (*it == '/') {
        ++it;
        if (*it == '>') {
            ++it;
            selfclosing = true;
            return true;
        } else {
            while (it != e) {
                ++it;
            }
            log_error(_("invalid html tag"));
            return false;
        }
    }

    std::string attname;
    std::string attvalue;

    //attributes
    while (it != e && *it != '>') {
        while (it != e && *it != '=' && *it != ' ') {
            
            if (*it == 0) {
                log_error(_("found NULL character in htmlText"));
                return false;
            }
            if (*it == '>') {
                log_error(_("malformed HTML tag, invalid attribute name"));
                while (it != e) {
                    ++it;
                }
                return false;
            }
            
            attname.push_back(std::toupper(*it));
            ++it;
        }
        while (it != e && (*it == ' ' || *it == '=')) {
            ++it; //skip over spaces and '='
        }

        if (it == e) return false;
        const char q = *it;
        if (q != '"' && q != '\'') { 
            // This is not an attribute.
            while (it != e) ++it;
            return false;
        }

        // Advance past attribute opener    
        ++it; 
        while (it != e && *it != q) {

            if (*it == 0) {
                log_error(_("found NULL character in htmlText"));
                return false;
            }

            attvalue.push_back(std::toupper(*it));
            ++it;
        }
        
        if (it == e) return false;
    
        if (*it != q) { 
            while (it != e) ++it;
            return false;
        } 

        // Skip attribute closer.
        ++it;

        attributes.insert(std::make_pair(attname, attvalue));
        attname.clear();
        attvalue.clear();

        if ((*it != ' ') && (*it != '/') && (*it != '>')) {
            log_error(_("malformed HTML tag, invalid attribute value"));
            while (it != e) {
                ++it;
            }
            return false;
        }
        if (*it == ' ') {
            while (it != e && *it == ' ') {
                ++it; //skip over spaces
            }
        }
        if (*it == '>') {
            ++it;
            return true;
        } else if (*it == '/') {
            ++it;
            if (*it == '>') {
                ++it;
                selfclosing = true;
                return true;
            } else {
                while (it != e) {
                    ++it;
                }
                log_error(_("invalid html tag"));
                return false;
            }
        }
    }
    
#ifdef GNASH_DEBUG_TEXTFIELDS
    log_debug("HTML tag: %s", utf8::encodeCanonicalString(tag, 7));
#endif
    log_error(_("I declare this a HTML syntax error"));
    return false; //since we did not return already, must be malformed...?
}

void
TextField::set_variable_name(const std::string& newname)
{
    if (newname != _variable_name) {
        _variable_name = newname;

        // The name was empty or undefined, so there's nothing more to do.
        if (_variable_name.empty()) return;

        _text_variable_registered = false;

#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug("Calling updateText after change of variable name");
#endif

        // Use the original definition text if this isn't dynamically
        // created.
        if (_tag) updateText(_tag->defaultText());

#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug("Calling registerTextVariable after change of variable "
		    "name and updateText call");
#endif
        registerTextVariable();
    }
}

bool
TextField::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    const SWFMatrix wm = getWorldMatrix(*this).invert();
    point lp(x, y);
    wm.transform(lp);
    return _bounds.point_test(lp.x, lp.y);
}

bool
TextField::getDrawBorder() const
{
    return _drawBorder;
}

void
TextField::setDrawBorder(bool val) 
{
    if (_drawBorder != val) {
        set_invalidated();
        _drawBorder = val;
    }
}

rgba
TextField::getBorderColor() const
{
    return _borderColor;
}

void
TextField::setBorderColor(const rgba& col)
{
    if (_borderColor != col) {
        set_invalidated();
        _borderColor = col;
    }
}

bool
TextField::getDrawBackground() const
{
    return _drawBackground;
}

void
TextField::setDrawBackground(bool val) 
{
    if (_drawBackground != val) {
        set_invalidated();
        _drawBackground = val;
    }
}

rgba
TextField::getBackgroundColor() const
{
    return _backgroundColor;
}

void
TextField::setBackgroundColor(const rgba& col)
{
    if (_backgroundColor != col) {
        set_invalidated();
        _backgroundColor = col;
    }
}

void
TextField::setTextColor(const rgba& col)
{
    if (_textColor != col) {

        set_invalidated();
        _textColor = col;
        std::for_each(_displayRecords.begin(), _displayRecords.end(),
                boost::bind(&SWF::TextRecord::setColor, _1, _textColor));
    }
}

void
TextField::setEmbedFonts(bool use)
{
    if (_embedFonts != use) {
        set_invalidated();
        _embedFonts=use;
        format_text();
    }
}

void
TextField::setWordWrap(bool wrap)
{
    if (_wordWrap != wrap) {
        set_invalidated();
        _wordWrap = wrap;
        format_text();
    }
}

void
TextField::setLeading(boost::int16_t h)
{
    if (_leading != h) {
        set_invalidated();
        _leading = h;
    }
}

void
TextField::setUnderlined(bool v)
{
    if (_underlined != v) {
        set_invalidated();
        _underlined = v;
    }
}

void          
TextField::setBullet(bool b)
{              
    if (_bullet != b) {
        _bullet = b;
    }
}

void 
TextField::setTabStops(const std::vector<int>& tabStops)
{
	_tabStops.resize(tabStops.size());
	
	for (size_t i = 0; i < tabStops.size(); i ++) {
		_tabStops[i] = pixelsToTwips(tabStops[i]);
	}
	
    set_invalidated();
}

void 
TextField::setURL(std::string url)
{ 
    if (_url != url) {
        set_invalidated();
        _url = url;
    }
}

void
TextField::setTarget(std::string target)
{
    if (_target != target) {
        set_invalidated();
        _target = target;
    }
}

void
TextField::setDisplay(TextFormatDisplay display)
{
    if (_display != display) {
        set_invalidated();
        _display = display;
    }
}

void
TextField::setAlignment(TextAlignment h)
{
    if (_alignment != h) {
        set_invalidated();
        _alignment = h;
    }
}

void
TextField::setIndent(boost::uint16_t h)
{
    if (_indent != h) {
        set_invalidated();
        _indent = h;
    }
}

void
TextField::setBlockIndent(boost::uint16_t h)
{
    if (_blockIndent != h) {
        set_invalidated();
        _blockIndent = h;
    }
}

void
TextField::setRightMargin(boost::uint16_t h)
{
    if (_rightMargin != h) {
        set_invalidated();
        _rightMargin = h;
    }
}

void
TextField::setLeftMargin(boost::uint16_t h)
{
    if (_leftMargin != h) {
        set_invalidated();
        _leftMargin = h;
    }
}

void
TextField::setFontHeight(boost::uint16_t h)
{
    if (_fontHeight != h) {
        set_invalidated();
        _fontHeight = h;
    }
}


TextField::TypeValue
TextField::parseTypeValue(const std::string& val)
{
    StringNoCaseEqual cmp;

    if (cmp(val, "input")) return typeInput;
    if (cmp(val, "dynamic")) return typeDynamic;
    return typeInvalid;
}


const char*
TextField::typeValueName(TypeValue val)
{
    switch (val) {
        case typeInput:
            //log_debug("typeInput returned as 'input'");
            return "input";
        case typeDynamic:
            //log_debug("typeDynamic returned as 'dynamic'");
            return "dynamic";
        default:
            //log_debug("invalid type %d returned as 'invalid'", (int)val);
            return "invalid";
    }

}

void
TextField::setAutoSize(AutoSize val)
{
    if (val == _autoSize) return; 

    set_invalidated();
    _autoSize = val; 
    format_text();
}

TextField::TextAlignment
TextField::getTextAlignment()
{
    TextAlignment textAlignment = getAlignment(); 

    switch (_autoSize) {
        case AUTOSIZE_CENTER:
            textAlignment = ALIGN_CENTER;
            break;
        case AUTOSIZE_LEFT:
            textAlignment = ALIGN_LEFT;
            break;
        case AUTOSIZE_RIGHT:
            textAlignment = ALIGN_RIGHT;
            break;
        default:
            // Leave it as it was.
            break;
    }

    return textAlignment;
}
    
void
TextField::onChanged()
{
    as_object* obj = getObject(this);
    callMethod(obj, NSV::PROP_BROADCAST_MESSAGE, "onChanged", obj);
}

/// This is called by movie_root when focus is applied to this TextField.
//
/// The return value is true if the TextField can receive focus.
/// The swfdec testsuite suggests that version 5 textfields cannot ever
/// handle focus.
bool
TextField::handleFocus()
{
    set_invalidated();

    /// Select the entire text on focus.
    setSelection(0, _text.length());

    m_has_focus = true;

    m_cursor = _text.size();
    format_text();
    return true;
}

/// This is called by movie_root when focus is removed from the
/// current TextField.
void
TextField::killFocus()
{
    if (!m_has_focus) return; 
    set_invalidated();
    m_has_focus = false;
    format_text(); // is this needed ?
}

void
TextField::setWidth(double newwidth)
{
	const SWFRect& bounds = getBounds();
    _bounds.set_to_rect(bounds.get_x_min(),
            bounds.get_y_min(),
            bounds.get_x_min() + newwidth,
            bounds.get_y_max());
}

void
TextField::setHeight(double newheight)
{
	const SWFRect& bounds = getBounds();
    _bounds.set_to_rect(bounds.get_x_min(),
            bounds.get_y_min(),
            bounds.get_x_max(),
            bounds.get_y_min() + newheight);
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

