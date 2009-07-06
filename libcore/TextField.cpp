// TextField.cpp:  User-editable text regions, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "utf8.h"
#include "log.h"
#include "swf/DefineEditTextTag.h"
#include "render.h"
#include "movie_definition.h" // to extract version info
#include "MovieClip.h"
#include "TextField.h"
#include "flash/ui/Keyboard_as.h" // for keyboard events
#include "movie_root.h"     // for killing focus
#include "as_environment.h" // for parse_path
#include "action.h" // for as_standard_member enum
#include "VM.h"
#include "builtin_function.h" // for getter/setter properties
#include "Font.h" // for using the _font member
#include "fontlib.h" // for searching or adding fonts the _font member
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "Array_as.h" // for _listeners construction
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "StringPredicates.h"
#include "TextFormat_as.h" // for getTextFormat/setTextFormat
#include "GnashKey.h" // key::code
#include "TextRecord.h"
#include "Point2d.h"
#include "GnashNumeric.h"

#include <algorithm> // std::min
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

// Text fields have a fixed 2 pixel padding for each side (regardless of border)
#define PADDING_TWIPS 40 

// Define the following to get detailed log information about
// textfield bounds and HTML tags:
//#define GNASH_DEBUG_TEXTFIELDS 1

// Define this to get debugging info about text formatting
//#define GNASH_DEBUG_TEXT_FORMATTING 1

namespace gnash {

// Forward declarations
namespace {
    as_object* getTextFieldInterface(VM& vm);
    void attachPrototypeProperties(as_object& proto);
    void attachTextFieldStaticMembers(as_object& o);

    as_value textfield_variable(const fn_call& fn);
    as_value textfield_setTextFormat(const fn_call& fn);
    as_value textfield_getTextFormat(const fn_call& fn);
    as_value textfield_setNewTextFormat(const fn_call& fn);
    as_value textfield_getNewTextFormat(const fn_call& fn);
    as_value textfield_getDepth(const fn_call& fn);
    as_value textfield_getFontList(const fn_call& fn);
    as_value textfield_removeTextField(const fn_call& fn);
    as_value textfield_replaceSel(const fn_call& fn);
    as_value textfield_replaceText(const fn_call& fn);

    as_value textfield_password(const fn_call& fn);
    as_value textfield_ctor(const fn_call& fn);
    as_value textfield_multiline(const fn_call& fn);
    as_value textfield_scroll(const fn_call& fn);
    as_value textfield_maxscroll(const fn_call& fn);
    as_value textfield_maxhscroll(const fn_call& fn);
    as_value textfield_maxChars(const fn_call& fn);
    as_value textfield_bottomScroll(const fn_call& fn);
    as_value textfield_hscroll(const fn_call& fn);
    as_value textfield_htmlText(const fn_call& fn);
    as_value textfield_restrict(const fn_call& fn);
    as_value textfield_background(const fn_call& fn);
    as_value textfield_border(const fn_call& fn);
    as_value textfield_backgroundColor(const fn_call& fn);
    as_value textfield_borderColor(const fn_call& fn);
    as_value textfield_text(const fn_call& fn);
    as_value textfield_textColor(const fn_call& fn);
    as_value textfield_embedFonts(const fn_call& fn);
    as_value textfield_autoSize(const fn_call& fn);
    as_value textfield_type(const fn_call& fn);
    as_value textfield_wordWrap(const fn_call& fn);
    as_value textfield_html(const fn_call& fn);
    as_value textfield_selectable(const fn_call& fn);
    as_value textfield_length(const fn_call& fn);
    as_value textfield_textWidth(const fn_call& fn);
    as_value textfield_textHeight(const fn_call& fn);
}

TextField::TextField(DisplayObject* parent, const SWF::DefineEditTextTag& def,
        int id)
    :
    InteractiveObject(parent, id),
    _tag(&def),
    _textDefined(def.hasText()),
    _underlined(false),
    _leading(def.leading()),
    _alignment(def.alignment()),
    _indent(def.indent()), 
    _blockIndent(0),
    _leftMargin(def.leftMargin()), 
    _rightMargin(def.rightMargin()), 
    _fontHeight(def.textHeight()), 
    _font(0),
    m_has_focus(false),
    m_cursor(0u),
    m_xcursor(0.0f),
    m_ycursor(0.0f),
    _multiline(def.multiline()),
    _password(def.password()),
    _maxChars(def.maxChars()),
    _text_variable_registered(false),
    _variable_name(def.variableName()),
    _drawBackground(def.border()),
    _backgroundColor(255,255,255,255),
    _drawBorder(def.border()),
    _borderColor(0,0,0,255),
    _textColor(def.color()),
    _embedFonts(def.getUseEmbeddedGlyphs()),
    _wordWrap(def.wordWrap()),
    _html(def.html()),
    _selectable(!def.noSelect()),
    _autoSize(autoSizeNone),
    _type(def.readOnly() ? typeDynamic : typeInput),
    _bounds(def.bounds()),
    _selection(0, 0)
{

    // WARNING! remember to set the font *before* setting text value!
    boost::intrusive_ptr<const Font> f = def.getFont();
    if (!f) f = fontlib::get_default_font(); 
    setFont(f);

    int version = parent->getVM().getSWFVersion();
    
    // set default text *before* calling registerTextVariable
    // (if the textvariable already exist and has a value
    // the text will be replaced with it)
    if (_textDefined) 
    {
        setTextValue(utf8::decodeCanonicalString(def.defaultText(), version));
    }

    init();

}

TextField::TextField(DisplayObject* parent, const rect& bounds)
    :
    // the id trick is to fool assertions in DisplayObject ctor
    InteractiveObject(parent, parent ? 0 : -1),
    _textDefined(false),
    _underlined(false),
    _leading(0),
    _alignment(ALIGN_LEFT),
    _indent(0), 
    _blockIndent(0),
    _leftMargin(0), 
    _rightMargin(0), 
    _fontHeight(12 * 20), 
    _font(0),
    m_has_focus(false),
    m_cursor(0u),
    m_xcursor(0.0f),
    m_ycursor(0.0f),
    _multiline(false),
    _password(false),
    _maxChars(0),
    _text_variable_registered(false),
    _drawBackground(false),
    _backgroundColor(255,255,255,255),
    _drawBorder(false),
    _borderColor(0, 0, 0, 255),
    _textColor(0, 0, 0, 255),
    _embedFonts(false), // ?
    _wordWrap(false),
    _html(false),
    _selectable(true),
    _autoSize(autoSizeNone),
    _type(typeDynamic),
    _bounds(bounds),
    _selection(0, 0)
{
    // Use the default font (Times New Roman for Windows, Times for Mac
    // according to docs. They don't say what it is for Linux.
    boost::intrusive_ptr<const Font> f = fontlib::get_default_font(); 
    setFont(f);

    init();
}

void
TextField::init()
{

    as_object* proto = getTextFieldInterface(_vm);
 
    // This is an instantiation, so attach properties to the
    // prototype.
    // TODO: is it correct to do it here, or can some TextFields
    // be constructed without attaching these?
    attachPrototypeProperties(*proto);

    set_prototype(proto);

    Array_as* ar = new Array_as();
    ar->push(this);
    set_member(NSV::PROP_uLISTENERS, ar);
    
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
        log_debug(_("CHECKME: removeTextField(%s): TextField depth (%d) "
            "out of the 'dynamic' zone [0..1048575], won't remove"),
            getTarget(), depth);
        //);
        return;
    }

    DisplayObject* parent = get_parent();
    assert(parent); // every TextField must have a parent, right ?

    MovieClip* parentSprite = parent->to_movie();

    if (!parentSprite)
    {
        log_error("FIXME: attempt to remove a TextField being a child of a %s",
                typeName(*parent));
        return;
    }

    // second argument is arbitrary, see comments above
    // the function declaration in MovieClip.h
    parentSprite->remove_display_object(depth, 0);
}

void
TextField::show_cursor(const SWFMatrix& mat)
{
    boost::uint16_t x = static_cast<boost::uint16_t>(m_xcursor);
    boost::uint16_t y = static_cast<boost::uint16_t>(m_ycursor);
    boost::uint16_t h = getFontHeight();

    const std::vector<point> box = boost::assign::list_of
        (point(x, y))
        (point(x, y + h));
    
    render::drawLine(box, rgba(0,0,0,255), mat);
}

void
TextField::display()
{

    registerTextVariable();

    const bool drawBorder = getDrawBorder();
    const bool drawBackground = getDrawBackground();

    const SWFMatrix& wmat = getWorldMatrix();

    if ((drawBorder || drawBackground) && !_bounds.is_null())
    {

        std::vector<point> coords(4);

        boost::int32_t xmin = _bounds.get_x_min();
        boost::int32_t xmax = _bounds.get_x_max();
        boost::int32_t ymin = _bounds.get_y_min();
        boost::int32_t ymax = _bounds.get_y_max();

        coords[0].setTo(xmin, ymin); 
        coords[1].setTo(xmax, ymin); 
        coords[2].setTo(xmax, ymax); 
        coords[3].setTo(xmin, ymax); 

        rgba borderColor = drawBorder ? getBorderColor() : rgba(0,0,0,0);
        rgba backgroundColor = drawBackground ? getBackgroundColor() :
                                                rgba(0,0,0,0);

        cxform cx = get_world_cxform();
            
        if (drawBorder) borderColor = cx.transform(borderColor);
         
        if (drawBackground) backgroundColor = cx.transform(backgroundColor);
        
#ifdef GNASH_DEBUG_TEXTFIELDS
    log_debug("rendering a Pol composed by corners %s", _bounds);
#endif

        render::draw_poly(&coords.front(), 4, backgroundColor, 
                borderColor, wmat, true);
        
    }

    // Draw our actual text.
    // Using a SWFMatrix to translate to def bounds seems an hack to me.
    // A cleaner implementation is likely correctly setting the
    // _xOffset and _yOffset memebers in glyph records.
    // Anyway, see bug #17954 for a testcase.
    SWFMatrix m = getWorldMatrix();

    if (!_bounds.is_null()) {
        m.concatenate_translation(_bounds.get_x_min(), _bounds.get_y_min()); 
    }
    
    SWF::TextRecord::displayRecords(m, get_world_cxform(), _textRecords,
            _embedFonts);

    if (m_has_focus) show_cursor(wmat);
    
    clear_invalidated();
}


void
TextField::add_invalidated_bounds(InvalidatedRanges& ranges, 
    bool force)
{
    if (!force && !m_invalidated) return; // no need to redraw
    
    ranges.add(m_old_invalidated_ranges);

    const SWFMatrix& wm = getWorldMatrix();

    rect bounds = getBounds();
    bounds.expand_to_rect(m_text_bounding_box); 
    wm.transform(bounds);
    ranges.add(bounds.getRange());            
}

void
TextField::replaceSelection(const std::string& replace)
{

    const int version = _vm.getSWFVersion();
    const std::wstring& wstr = utf8::decodeCanonicalString(replace, version);
    
    const size_t start = _selection.first;
    const size_t replaceLength = wstr.size();

    _text.replace(start, _selection.second - start, wstr);
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
bool
TextField::on_event(const event_id& ev)
{
    if (isReadOnly()) return false;

    switch (ev.id())
    {
        case event_id::KEY_PRESS:
        {
            if ( getType() != typeInput ) break; // not an input field
            std::wstring s = _text;

            // id.keyCode is the unique gnash::key::code for a DisplayObject/key.
            // The maximum value is about 265, including function keys.
            // It seems that typing in DisplayObjects outside the Latin-1 set
            // (256 DisplayObject codes, identical to the first 256 of UTF-8)
            // is not supported, though a much greater number UTF-8 codes can be
            // stored and displayed. See utf.h for more information.
            // This is a limit on the number of key codes, not on the
            // capacity of strings.
            gnash::key::code c = ev.keyCode();

            // maybe _text is changed in ActionScript
            m_cursor = std::min<size_t>(m_cursor, _text.size());

            switch (c)
            {
                case key::BACKSPACE:
                    if (m_cursor > 0)
                    {
                        s.erase(m_cursor - 1, 1);
                        m_cursor--;
                        setTextValue(s);
                    }
                    break;

                case key::DELETEKEY:
                    if (s.size() > m_cursor)
                    {
                        s.erase(m_cursor, 1);
                        setTextValue(s);
                    }
                    break;

                case key::INSERT:        // TODO
                    break;

                case key::HOME:
                case key::PGUP:
                case key::UP:
                    m_cursor = 0;
                    format_text();
                    break;

                case key::END:
                case key::PGDN:
                case key::DOWN:
                    m_cursor = _text.size();
                    format_text();
                    break;

                case key::LEFT:
                    m_cursor = m_cursor > 0 ? m_cursor - 1 : 0;
                    format_text();
                    break;

                case key::RIGHT:
                    m_cursor = m_cursor < _text.size() ? m_cursor + 1 :
                                                        _text.size();
                    format_text();
                    break;

                default:
                    wchar_t t = static_cast<wchar_t>(
                            gnash::key::codeMap[c][key::ASCII]);
                    if (t != 0)
                    {
                        // Insert one copy of the DisplayObject
                        // at the cursor position.
                          s.insert(m_cursor, 1, t);
                        m_cursor++;
                    }
                    setTextValue(s);
                    break;
            }
            onChanged();
        }

        default:
            return false;
    }
    return true;
}

InteractiveObject*
TextField::topmostMouseEntity(boost::int32_t x, boost::int32_t y)
{

    if (!visible()) return 0;
    
    // shouldn't this be !can_handle_mouse_event() instead ?
    // not selectable, so don't catch mouse events!
    if (!_selectable) return 0;

    SWFMatrix m = getMatrix();
    point p(x, y);
    m.invert().transform(p);

    if (_bounds.point_test(p.x, p.y)) return this;

    return 0;
}

void
TextField::updateText(const std::string& str)
{
    int version = _vm.getSWFVersion();
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
    format_text();
}

void
TextField::setTextValue(const std::wstring& wstr)
{

    updateText(wstr);

    if ( ! _variable_name.empty() && _text_variable_registered )
    {
        // TODO: notify MovieClip if we have a variable name !
        VariableRef ref = parseTextVariableRef(_variable_name);
        as_object* tgt = ref.first;
        if ( tgt )
        {
            int version = _vm.getSWFVersion();
            // we shouldn't truncate, right?
            tgt->set_member(ref.second, utf8::encodeCanonicalString(wstr,
                        version)); 
        }
        else    
        {
            // nothing to do (too early ?)
            log_debug("setTextValue: variable name %s points to a non-existent"
                    " target, I guess we would not be registered if this was "
                    "true, or the sprite we've registered our variable name "
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

    int version = _vm.getSWFVersion();

    return utf8::encodeCanonicalString(_text, version);
}

bool
TextField::set_member(string_table::key name,
        const as_value& val, string_table::key nsname, bool ifFound)
{

    // FIXME: Turn all standard members into getter/setter properties
    //        of the TextField class. See attachTextFieldInterface()
    // @@ TODO need to inherit basic stuff like _x, _y, _xscale, _yscale etc ?

    switch (name)
    {
    default:
        break;
    case NSV::PROP_uX:
    {
        SWFMatrix m = getMatrix();
        double x = infinite_to_zero( val.to_number() );
        m.tx = pixelsToTwips(x);    
        setMatrix(m); // no need to update caches when only changing translation

        // m_accept_anim_moves = false;
        return true;
    }
    case NSV::PROP_uY:
    {
        SWFMatrix m = getMatrix();
        double y = infinite_to_zero( val.to_number() );
        m.ty = pixelsToTwips(y);
        setMatrix(m); // no need to update caches when only changing translation

        // m_accept_anim_moves = false; 
        return true;
    }
    case NSV::PROP_uWIDTH:
    {
        double nw = val.to_number(); 
        if (!isFinite(nw) )
        {
            // might be our fault, see the TODO above 
            // (missing to pass as_environment out..)
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set TextField._width to %g"), nw);
            );
            return true;
        }

        if ( nw < 0 )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set TextField._width to a "
                    "negative number: %g, toggling sign"), nw);
            );
            nw = -nw;
        }

        if ( _bounds.width() == pixelsToTwips(nw) )
        {
#ifdef GNASH_DEBUG_TEXTFIELDS
            log_debug("TextField width already == %g, nothing to do to "
                    "change it", nw);
#endif
            return true; // nothing to do
        }
        if ( _bounds.is_null() )
        {
#ifdef GNASH_DEBUG_TEXTFIELDS
            log_debug("NULL TextField bounds : %s", _bounds);
#endif
            return true;
        }

#ifdef GNASH_DEBUG_TEXTFIELDS
        log_debug("Chaging TextField width to %g", nw);
#endif

        set_invalidated();

        // Modify TextField drawing rectangle
        // TODO: check which anchor point we should use !
        boost::int32_t xmin = _bounds.get_x_min();
        boost::int32_t ymin = _bounds.get_y_min();
        boost::int32_t ymax = _bounds.get_y_max();
        boost::int32_t xmax = xmin + pixelsToTwips(nw);

        assert(xmin <= xmax);
        _bounds.set_to_rect(xmin, ymin, xmax, ymax);
        assert( _bounds.width() == pixelsToTwips(nw) );

        // previously truncated text might get visible now
        // TODO: if nested masks were implemented we would 
        // not need to reformat text here
        format_text();

        return true;
    }
    case NSV::PROP_uHEIGHT:
    {
        double nh = val.to_number(); 
        if (!isFinite(nh) )
        {
            // might be our fault, see the TODO above (missing to pass
            // as_environment out..)
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set TextField._height to %g"), nh);
            );
            return true;
        }

        if ( nh < 0.0f )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set TextField._height to a negative "
                    "number: %g, toggling sign"), nh);
            );
            nh = -nh;
        }

        if ( _bounds.height() == pixelsToTwips(nh) )
        {
#ifdef GNASH_DEBUG_TEXTFIELDS
            log_debug("TextField height already == %g, nothing to do to "
                    "change it", nh);
#endif // GNASH_DEBUG_TEXTFIELDS
            return true; // nothing to do
        }
        if ( _bounds.is_null() )
        {
            return true;
        }

#ifdef GNASH_DEBUG_TEXTFIELDS
        log_debug("Changing TextField height to %g", nh);
#endif // GNASH_DEBUG_TEXTFIELDS
        set_invalidated();

        // Modify TextField drawing rectangle
        // TODO: check which anchor point we should use !
        boost::int32_t xmin = _bounds.get_x_min();
        boost::int32_t xmax = _bounds.get_x_max();
        boost::int32_t ymin = _bounds.get_y_min();
        _bounds.set_to_rect(xmin, ymin, xmax, ymin + pixelsToTwips(nh) );

        assert(_bounds.height() == pixelsToTwips(nh));

        // previously truncated text might get visible now
        // TODO: if nested masks were implemented we would 
        // not need to reformat text here
        format_text();

        return true;
    }
    case NSV::PROP_uVISIBLE:
    {
        set_visible(val.to_bool());
        return true;
    }
    case NSV::PROP_uALPHA:
    {
        // @@ TODO this should be generic to class DisplayObject!
        // Arg is in percent.
        cxform    cx = get_cxform();
        cx.aa = (boost::int16_t)(val.to_number() * 2.56);
        set_cxform(cx);
        return true;
    }
    // @@ TODO see TextField members in Flash MX docs
    }    // end switch


    return as_object::set_member(name, val, nsname, ifFound);
}

bool
TextField::get_member(string_table::key name, as_value* val,
    string_table::key nsname)
{
    //log_debug("TextField.get_member(%s)", name);

    // FIXME: Turn all standard members into getter/setter properties
    //        of the TextField class. See attachTextFieldInterface()

    switch (name)
    {
    default:
        break;
    case NSV::PROP_uVISIBLE:
    {
        val->set_bool(visible());
        return true;
    }
    case NSV::PROP_uALPHA:
    {
        // @@ TODO this should be generic to class DisplayObject!
        const cxform&    cx = get_cxform();
        val->set_double(cx.aa / 2.56);
        return true;
    }
    case NSV::PROP_uX:
    {
        SWFMatrix    m = getMatrix();    
        val->set_double(twipsToPixels(m.tx));
        return true;
    }
    case NSV::PROP_uY:
    {
        SWFMatrix    m = getMatrix();    
        val->set_double(twipsToPixels(m.ty));
        return true;
    }
    case NSV::PROP_uWIDTH:
    {
        val->set_double(twipsToPixels(get_width()));
#ifdef GNASH_DEBUG_TEXTFIELDS
        log_debug("Got TextField width == %s", *val);
#endif // GNASH_DEBUG_TEXTFIELDS
        return true;
    }
    case NSV::PROP_uHEIGHT:
    {
        val->set_double(twipsToPixels(get_height()));
#ifdef GNASH_DEBUG_TEXTFIELDS
        log_debug("Got TextField height == %s", *val);
#endif // GNASH_DEBUG_TEXTFIELDS
        return true;
    }
    }    // end switch

    return as_object::get_member(name, val, nsname);
    
}
    

float
TextField::align_line(TextAlignment align,
        int last_line_start_record, float x)
{

    float width = _bounds.width(); 
    float right_margin = getRightMargin();

    float extra_space = (width - right_margin) - x - PADDING_TWIPS;

    //assert(extra_space >= 0.0f);
    if (extra_space <= 0.0f)
    {
#ifdef GNASH_DEBUG_TEXTFIELDS
        log_debug(_("TextField text doesn't fit in its boundaries: "
                "width %g, margin %g - nothing to align"),
                width, right_margin);
#endif
        return 0.0f;
    }

    float shift_right = 0.0f;

    if (align == ALIGN_LEFT)
    {
        // Nothing to do; already aligned left.
        return 0.0f;
    }
    else if (align == ALIGN_CENTER)
    {
        // Distribute the space evenly on both sides.
        shift_right = extra_space / 2;
    }
    else if (align == ALIGN_RIGHT)
    {
        // Shift all the way to the right.
        shift_right = extra_space;
    }

    // Shift the beginnings of the records on this line.
    for (unsigned int i = last_line_start_record; i < _textRecords.size(); ++i)
    {
        SWF::TextRecord& rec = _textRecords[i];

        //if ( rec.hasXOffset() ) // why?
            rec.setXOffset(rec.xOffset() + shift_right); 
    }
    return shift_right;
}

boost::intrusive_ptr<const Font>
TextField::setFont(boost::intrusive_ptr<const Font> newfont)
{
    if ( newfont == _font ) return _font;

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
    if ( index == -1 )
    {
        IF_VERBOSE_MALFORMED_SWF (
          log_error(_("TextField: missing glyph for space char (needed "
                  "for TAB). Make sure DisplayObject shapes for font "
                  "%s are being exported into your SWF file."),
                rec.getFont()->name());
        );
    }
    else
    {
        SWF::TextRecord::GlyphEntry ge;
        ge.index = index;
        ge.advance = scale * rec.getFont()->get_advance(index, 
                _embedFonts);

        const int tabstop = 8;
        rec.addGlyph(ge, tabstop);
        x += ge.advance * tabstop;
    }
}

void
TextField::format_text()
{
    _textRecords.clear();

    // nothing more to do if text is empty
    if ( _text.empty() )
    {
        // TODO: should we still reset _bounds if autoSize != autoSizeNone ?
        //       not sure we should...
        reset_bounding_box(0, 0);
        return;
    }

    // See bug #24266
    const rect& defBounds = _bounds;

    AutoSizeValue autoSize = getAutoSize();
    if ( autoSize != autoSizeNone )
    {
        // define GNASH_DEBUG_TEXT_FORMATTING on top to get useful info
        //LOG_ONCE( log_debug(_("TextField.autoSize != 'none' TESTING")) );

        // When doing WordWrap we don't want to change
        // the boundaries. See bug #24348
        if (!  doWordWrap() )
        {
            _bounds.set_to_rect(0, 0, 0, 0); // this is correct for 'true'
        }
    }

    // Should get info from autoSize too maybe ?
    TextAlignment textAlignment = getTextAlignment();

    // FIXME: I don't think we should query the definition
    // to find the appropriate font to use, as ActionScript
    // code should be able to change the font of a TextField
    //
    if (!_font)
    {
        log_error(_("No font for TextField!"));
        return;
    }

    boost::uint16_t fontHeight = getFontHeight();
    float scale = fontHeight / (float)_font->unitsPerEM(_embedFonts); 
    float fontDescent = _font->descent() * scale; 
    float fontLeading = _font->leading() * scale;
    boost::uint16_t leftMargin = getLeftMargin();
    boost::uint16_t rightMargin = getRightMargin();
    boost::uint16_t indent = getIndent();
    boost::uint16_t blockIndent = getBlockIndent();
    bool underlined = getUnderlined();

    //log_debug("%s: fontDescent:%g, fontLeading:%g, fontHeight:%g, scale:%g",
    //  getTarget(), fontDescent, fontLeading, fontHeight, scale);

    SWF::TextRecord rec;    // one to work on
    rec.setFont(_font.get());
    rec.setUnderline(underlined);
    rec.setColor(getTextColor()); 
    rec.setXOffset(PADDING_TWIPS + 
            std::max(0, leftMargin + indent + blockIndent));
    rec.setYOffset(PADDING_TWIPS + fontHeight + (fontLeading - fontDescent));
    rec.setTextHeight(fontHeight);

    boost::int32_t x = static_cast<boost::int32_t>(rec.xOffset());
    boost::int32_t y = static_cast<boost::int32_t>(rec.yOffset());

    // Start the bbox at the upper-left corner of the first glyph.
    reset_bounding_box(x, y - fontDescent + fontHeight); 

    float leading = getLeading();
    leading += fontLeading * scale; // not sure this is correct...

    int    last_code = -1; // only used if _embedFonts
    int    last_space_glyph = -1;
    int    last_line_start_record = 0;

    unsigned int idx = 0;
    m_xcursor = x;
    m_ycursor = y;

    assert(! _text.empty() );
    
    boost::uint32_t code = 0;
    
    // String iterators are very sensitive to 
    // potential changes to the string (to allow for copy-on-write).
    // So there must be no external changes to the string or
    // calls to most non-const member functions during this loop.
    // Especially not c_str() or data().
    std::wstring::const_iterator it = _text.begin();
    const std::wstring::const_iterator e = _text.end();

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
            case 13:
            case 10:
            {
                // newline.

                // Frigging Flash seems to use '\r' (13) as its
                // default newline DisplayObject.  If we get DOS-style \r\n
                // sequences, it'll show up as double newlines, so maybe we
                // need to detect \r\n and treat it as one newline.

                // Close out this stretch of glyphs.
                _textRecords.push_back(rec);
                align_line(textAlignment, last_line_start_record, x);

                // Expand bounding box to include last column of text ...
                if ( _autoSize != autoSizeNone ) {
                    _bounds.expand_to_point(x + PADDING_TWIPS,
                            y + PADDING_TWIPS);
                }

                // new paragraphs get the indent.
                x = std::max(0, leftMargin + indent) + PADDING_TWIPS;
                y += fontHeight + leading; 

                // Start a new record on the next line. Other properties of the
                // TextRecord should be left unchanged.
                rec.clearGlyphs();
                rec.setXOffset(x);
                rec.setYOffset(y);

                last_space_glyph = -1;
                last_line_start_record = _textRecords.size();

                continue;
            }
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
            case '<':
                if (_html)
                {
                    LOG_ONCE(log_debug(_("HTML in a text field is unsupported, "
                                         "gnash will just ignore the tags and "
                                         "print their content")));
         
                    std::wstring discard;
                    bool complete = parseHTML(discard, it, e);
                    
                    if (!complete) continue;
                    else break;

                }
                // If HTML isn't enabled, carry on and insert the glyph.

            case 32:
                last_space_glyph = rec.glyphs().size();
                // Don't break, as we still need to insert the space glyph.

            default:
            {

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
            }
        }

        float width = defBounds.width();
        if (x >= width - rightMargin - PADDING_TWIPS)
        {
#ifdef GNASH_DEBUG_TEXT_FORMATTING
            log_debug("Text in TextField %s exceeds width [ _bounds %s ]", 
                    getTarget(), _bounds);
#endif

            // No wrap and no resize: truncate
            if (!doWordWrap() && autoSize == autoSizeNone)
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
            else if ( doWordWrap() )
            {
#ifdef GNASH_DEBUG_TEXT_FORMATTING
                log_debug(" wordWrap=true");
#endif 

                // Insert newline if there's space or autosize != none

                // Close out this stretch of glyphs.
                _textRecords.push_back(rec);

                float previous_x = x;
                x = leftMargin + blockIndent + PADDING_TWIPS;
                y += fontHeight + leading;

                // Start a new record on the next line.
                rec.clearGlyphs();
                rec.setXOffset(x);
                rec.setYOffset(y);

                // TODO : what if m_text_glyph_records is empty ?
                // Is it possible ?
                assert(!_textRecords.empty());
                SWF::TextRecord& last_line = _textRecords.back();
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
                    }
                }
                else
                {
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
                }

                align_line(textAlignment, last_line_start_record, previous_x);

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

        if (y > (defBounds.height() - PADDING_TWIPS) && 
                autoSize == autoSizeNone )
        {
#ifdef GNASH_DEBUG_TEXT_FORMATTING
            log_debug("Text with wordWrap exceeds height of box");
#endif
            rec.clearGlyphs();
            // TODO: should still compute m_text_bounds !
            LOG_ONCE(log_unimpl("Computing text bounds of a TextField "
                        "containing text that doesn't fit the box vertically"));
            break;
        }

        if (m_cursor > idx)
        {
            m_xcursor = x;
            m_ycursor = y;
        }
        idx++;

        // TODO: HTML markup
    }

    // Expand bounding box to include the whole text (if autoSize)
    if ( _autoSize != autoSizeNone )
    {
        _bounds.expand_to_point(x+PADDING_TWIPS, y+PADDING_TWIPS);
    }

    // Add this line to our output.
    if (!rec.glyphs().empty()) _textRecords.push_back(rec);

    float extra_space = align_line(textAlignment, last_line_start_record, x);

    m_xcursor += static_cast<int>(extra_space);
    m_ycursor -= fontHeight + (fontLeading - fontDescent);
}

TextField::VariableRef
TextField::parseTextVariableRef(const std::string& variableName) const
{
    VariableRef ret;
    ret.first = 0;

#ifdef DEBUG_DYNTEXT_VARIABLES
    log_debug(_("VariableName: %s"), variableName);
#endif

    /// Why isn't get_environment const again ?
    as_environment& env = const_cast<TextField*>(this)->get_environment();

    as_object* target = env.get_target();
    if ( ! target )
    {
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
    if (as_environment::parse_path(variableName, path, var))
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("Variable text Path: %s, Var: %s"), path, var);
#endif
        // find target for the path component
        // we use our parent's environment for this
        target = env.find_object(path);

        parsedName = var;
    }

    if ( ! target )
    {
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
    ret.second = _vm.getStringTable().find(parsedName);

    return ret;
}

void
TextField::registerTextVariable() 
{
//#define DEBUG_DYNTEXT_VARIABLES 1

#ifdef DEBUG_DYNTEXT_VARIABLES
    log_debug(_("registerTextVariable() called"));
#endif

    if ( _text_variable_registered )
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("registerTextVariable() no-op call (already registered)"));
#endif
        return;
    }

    if ( _variable_name.empty() )
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("string is empty, consider as registered"));
#endif
        _text_variable_registered=true;
        return;
    }

    VariableRef varRef = parseTextVariableRef(_variable_name);
    as_object* target = varRef.first;
    if ( ! target )
    {
        log_debug(_("VariableName associated to text field (%s) refer to "
                    "an unknown target. It is possible that the DisplayObject "
                    "will be instantiated later in the SWF stream. "
                    "Gnash will try to register again on next access."),
                _variable_name);
        return;
    }

    string_table::key key = varRef.second;

    // check if the VariableName already has a value,
    // in that case update text value
    as_value val;
    
    int version = _vm.getSWFVersion();
    
    if (target->get_member(key, &val) )
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("target object (%s @ %p) does have a member named %s"),
            typeName(*target), (void*)target, _vm.getStringTable().value(key));
#endif
        // TODO: pass environment to to_string ?
        // as_environment& env = get_environment();
        setTextValue(utf8::decodeCanonicalString(val.to_string(), version));
    }
    else if ( _textDefined )
    {
        as_value newVal = as_value(utf8::encodeCanonicalString(_text, version));
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("target sprite (%s @ %p) does NOT have a member "
                    "named %s (no problem, we'll add it with value %s)"),
                    typeName(*target), (void*)target,
                    _vm.getStringTable().value(key), newVal);
#endif
        target->set_member(key, newVal);
    }
    else
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("target sprite (%s @ %p) does NOT have a member "
                    "named %s, and we don't have text defined"),
                    typeName(*target), (void*)target,
                    _vm.getStringTable().value(key));
#endif
    }

    MovieClip* sprite = target->to_movie();

    if ( sprite )
    {
        // add the textfield variable to the target sprite
        // TODO: have set_textfield_variable take a string_table::key instead ?
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug("Calling set_textfield_variable(%s) against sprite %s",
                _vm.getStringTable().value(key), sprite->getTarget());
#endif
        sprite->set_textfield_variable(_vm.getStringTable().value(key), this);

    }
    _text_variable_registered=true;

}

/// Parses an HTML tag (between < and >) and puts
/// the contents into tag. Returns false if the
/// tag was incomplete. The iterator is moved to after
/// the closing tag or the end of the string.
bool
TextField::parseHTML(std::wstring& tag, std::wstring::const_iterator& it,
                               const std::wstring::const_iterator& e) const
{

    bool complete = false;

    while (it != e)
    {
        if (*it == '>')
        {
            ++it;
            complete = true;
            break;
        }

        // Check for NULL DisplayObject
        if (*it == 0) break;

        tag.push_back(*it++);
    }
    
#ifdef GNASH_DEBUG_TEXTFIELDS
    log_debug ("HTML tag: %s", utf8::encodeCanonicalString(tag, 7));
#endif
    
    return complete;
}

void
TextField::set_variable_name(const std::string& newname)
{
    if ( newname != _variable_name )
    {
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

/// This provides the prototype and static methods for TextField.
//
/// For SWF5 there is initially no prototype, for SWF6+ there is a 
/// limited prototype. This is changed later on instantiation of a
/// TextField.
void
textfield_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl = NULL;

    if (!cl)
    {
        VM& vm = global.getVM();

        if (vm.getSWFVersion() < 6) {
            /// Version 5 or less: no initial prototype
            cl = new builtin_function(&textfield_ctor, 0);
        }
        else {
            /// Version 6 upward: limited initial prototype
            as_object* iface = getTextFieldInterface(vm);
            cl = new builtin_function(&textfield_ctor, iface);
        }

        vm.addStatic(cl.get());

        // replicate static members to class, to be able to access
        // all methods as static functions
        attachTextFieldStaticMembers(*cl);
             
    }

    // Register _global.TextField
    global.init_member("TextField", cl.get());
}

bool
TextField::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    SWFMatrix wm = getWorldMatrix();
    point lp(x, y);
    wm.invert().transform(lp);
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
    if ( _drawBorder != val )
    {
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
    if ( _borderColor != col )
    {
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
    if ( _drawBackground != val )
    {
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
    if ( _backgroundColor != col )
    {
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
        std::for_each(_textRecords.begin(), _textRecords.end(),
                boost::bind(&SWF::TextRecord::setColor, _1, _textColor));
    }
}

void
TextField::setEmbedFonts(bool use)
{
    if ( _embedFonts != use )
    {
        set_invalidated();
        _embedFonts=use;
        format_text();
    }
}

void
TextField::setWordWrap(bool on)
{
    if ( _wordWrap != on )
    {
        set_invalidated();
        _wordWrap=on;
        format_text();
    }
}

cxform    
TextField::get_world_cxform() const
{
    // This is not automatically tested. See testsuite/samples/input-fields.swf
    // for a manual check.

    // If using a device font (PP compatibility), do not take parent cxform
    // into account.
    if (!getEmbedFonts()) return cxform();

    return DisplayObject::get_world_cxform();
}

void
TextField::setLeading(boost::uint16_t h)
{
    if ( _leading != h )
    {
        set_invalidated();
        _leading = h;
        format_text();
    }
}

void
TextField::setUnderlined(bool v)
{
    if ( _underlined != v )
    {
        set_invalidated();
        _underlined = v;
        format_text();
    }
}

void
TextField::setAlignment(TextAlignment h)
{
    if ( _alignment != h )
    {
        set_invalidated();
        _alignment = h;
        format_text();
    }
}

void
TextField::setIndent(boost::uint16_t h)
{
    if ( _indent != h )
    {
        set_invalidated();
        _indent = h;
        format_text();
    }
}

void
TextField::setBlockIndent(boost::uint16_t h)
{
    if ( _blockIndent != h )
    {
        set_invalidated();
        _blockIndent = h;
        format_text();
    }
}

void
TextField::setRightMargin(boost::uint16_t h)
{
    if ( _rightMargin != h )
    {
        set_invalidated();
        _rightMargin = h;
        format_text();
    }
}

void
TextField::setLeftMargin(boost::uint16_t h)
{
    if (_leftMargin != h)
    {
        set_invalidated();
        _leftMargin = h;
        format_text();
    }
}

void
TextField::setFontHeight(boost::uint16_t h)
{
    if ( _fontHeight != h )
    {
        set_invalidated();
        _fontHeight = h;
        format_text();
    }
}


TextField::AutoSizeValue
TextField::parseAutoSizeValue(const std::string& val)
{
    StringNoCaseEqual cmp;

    if ( cmp(val, "left") )
    {
        return autoSizeLeft;
    }
    if ( cmp(val, "right") )
    {
        return autoSizeRight;
    }
    if ( cmp(val, "center") )
    {
        return autoSizeCenter;
    }
    return autoSizeNone;

}


const char*
TextField::autoSizeValueName(AutoSizeValue val)
{
    switch (val)
    {
        case autoSizeLeft:
            return "left";
        case autoSizeRight:
            return "right";
        case autoSizeCenter:
            return "center";
        case autoSizeNone:
        default:
            return "none";
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
    switch (val)
    {
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
TextField::setAutoSize(AutoSizeValue val)
{
    if ( val == _autoSize ) return;

    set_invalidated();

    _autoSize = val; 
    format_text();
}

TextField::TextAlignment
TextField::getTextAlignment()
{
    TextAlignment textAlignment = getAlignment(); 
    if ( _autoSize == autoSizeCenter ) textAlignment = ALIGN_CENTER;
    else if ( _autoSize == autoSizeLeft ) textAlignment = ALIGN_LEFT;
    else if ( _autoSize == autoSizeRight ) textAlignment = ALIGN_RIGHT;
    return textAlignment;
}

void
TextField::onChanged()
{
    as_value met(PROPNAME("onChanged"));
    as_value targetVal(this);
    callMethod(NSV::PROP_BROADCAST_MESSAGE, met, targetVal);
}

/// This is called by movie_root when focus is applied to this TextField.
//
/// The return value is true if the TextField can recieve focus.
bool
TextField::handleFocus()
{

    set_invalidated();

    /// Select the entire text on focus.
    setSelection(0, _text.length());

    m_has_focus = true;

    // why should we add to the key listener list every time
    // we call setFocus()???
    _vm.getRoot().add_key_listener(this);

    m_cursor = _text.size();
    format_text();
    return true;
}

/// This is called by movie_root when focus is removed from the
/// current TextField.
void
TextField::killFocus()
{
    if ( ! m_has_focus ) return; // nothing to do

    set_invalidated();
    m_has_focus = false;

    movie_root& root = _vm.getRoot();
    root.remove_key_listener(this);
    format_text(); // is this needed ?

}

void
TextField::markReachableResources() const
{

    if (_tag) _tag->setReachable();

    if (_font) _font->setReachable();

    // recurse to parent...
    markDisplayObjectReachable();
}

/// TextField interface functions

namespace {

void
attachPrototypeProperties(as_object& o)
{
    // Standard flags.
    const int flags = as_prop_flags::dontDelete
        |as_prop_flags::dontEnum;

    // SWF6 or higher
    const int swf6Flags = flags | as_prop_flags::onlySWF6Up;

    boost::intrusive_ptr<builtin_function> getset;

    // The following properties should only be attached to the prototype
    // on first textfield creation.
    o.init_property(NSV::PROP_TEXT_WIDTH,
            textfield_textWidth, textfield_textWidth);
    o.init_property(NSV::PROP_TEXT_HEIGHT,
            textfield_textHeight, textfield_textHeight);

    getset = new builtin_function(textfield_variable);
    o.init_property("variable", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_background);
    o.init_property("background", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_text);
    o.init_property("text", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_backgroundColor);
    o.init_property("backgroundColor", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_border);
    o.init_property("border", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_borderColor);
    o.init_property("borderColor", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_textColor);
    o.init_property("textColor", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_embedFonts);
    o.init_property("embedFonts", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_autoSize);
    o.init_property("autoSize", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_type);
    o.init_property("type", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_wordWrap);
    o.init_property("wordWrap", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_html);
    o.init_property("html", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_selectable);
    o.init_property("selectable", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_length);
    o.init_property("length", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_maxscroll);
    o.init_property("maxscroll", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_maxhscroll);
    o.init_property("maxhscroll", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_maxChars);
    o.init_property("maxChars", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_bottomScroll);
    o.init_property("bottomScroll", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_scroll);
    o.init_property("scroll", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_hscroll);
    o.init_property("hscroll", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_restrict);
    o.init_property("restrict", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_multiline);
    o.init_property("multiline", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_password);
    o.init_property("password", *getset, *getset, swf6Flags);
    getset = new builtin_function(textfield_htmlText);
    o.init_property("htmlText", *getset, *getset, swf6Flags);
}


as_value
textfield_background(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (fn.nargs == 0) {
        return as_value(ptr->getDrawBackground());
    }
    else {
        ptr->setDrawBackground(fn.arg(0).to_bool());
    }

    return as_value();
}

as_value
textfield_border(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (fn.nargs == 0) {
        return as_value(ptr->getDrawBorder());
    }
    else {
        ptr->setDrawBorder(fn.arg(0).to_bool());
    }

    return as_value();
}

as_value
textfield_backgroundColor(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (fn.nargs == 0) {
        return as_value(ptr->getBackgroundColor().toRGB());
    }
    else {
        rgba newColor;
        newColor.parseRGB(static_cast<boost::uint32_t>(fn.arg(0).to_int()));
        ptr->setBackgroundColor(newColor);
    }

    return as_value();
}

as_value
textfield_borderColor(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (fn.nargs == 0) {
        return as_value(ptr->getBorderColor().toRGB());
    }
    else {
        rgba newColor;
        newColor.parseRGB(static_cast<boost::uint32_t>(fn.arg(0).to_number()));
        ptr->setBorderColor(newColor);
    }

    return as_value();
}

    
as_value
textfield_textColor(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (!fn.nargs) {
        // Getter
        return as_value(ptr->getTextColor().toRGB());
    }

    // Setter
    rgba newColor;
    newColor.parseRGB(static_cast<boost::uint32_t>(fn.arg(0).to_number()));
    ptr->setTextColor(newColor);

    return as_value();
}

as_value
textfield_embedFonts(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (!fn.nargs) {
        // Getter
        return as_value(ptr->getEmbedFonts());
    }

    // Setter
    ptr->setEmbedFonts(fn.arg(0).to_bool());
    return as_value();
}

as_value
textfield_wordWrap(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (fn.nargs == 0) {
        return as_value(ptr->doWordWrap());
    }
    else {
        ptr->setWordWrap(fn.arg(0).to_bool());
    }

    return as_value();
}

as_value
textfield_html(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (fn.nargs == 0) {
        return as_value(ptr->doHtml());
    }
    else {
        ptr->setHtml( fn.arg(0).to_bool() );
    }

    return as_value();
}

as_value
textfield_selectable(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->isSelectable());
    }
    else // setter
    {
        ptr->setSelectable( fn.arg(0).to_bool() );
    }

    return as_value();
}

as_value
textfield_length(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        const std::string& s = ptr->get_text_value();
        return as_value(s.length()); // TOCHECK: utf-8 ?
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set length property of TextField %s"),
            ptr->getTarget());
        );
    }

    return as_value();
}

as_value
textfield_textHeight(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // Return the height, in pixels, of the text as laid out.
        // (I.e. the actual text content, not our defined
        // bounding box.)
        //
        // In local coords.  Verified against Macromedia Flash.
        return as_value(twipsToPixels(ptr->getTextBoundingBox().height()));

    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set read-only %s property of TextField "
                "%s"), "textHeight", ptr->getTarget());
        );
    }

    return as_value();
}

as_value
textfield_textWidth(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // Return the width, in pixels, of the text as laid out.
        // (I.e. the actual text content, not our defined
        // bounding box.)
        //
        // In local coords.  Verified against Macromedia Flash.
        return as_value(twipsToPixels(ptr->getTextBoundingBox().width()));

    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set read-only %s property of TextField %s"),
            "textWidth", ptr->getTarget());
        );
    }

    return as_value();
}

as_value
textfield_autoSize(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return ptr->autoSizeValueName(ptr->getAutoSize());
    }
    else // setter
    {
        const as_value& arg = fn.arg(0);
        if ( arg.is_bool() )
        {
            if ( arg.to_bool() ) // true == left
            {
                ptr->setAutoSize( TextField::autoSizeLeft );
            }
            else
            {
                ptr->setAutoSize( TextField::autoSizeNone );
            }
        }
        else
        {
            std::string strval = arg.to_string();
            TextField::AutoSizeValue val = ptr->parseAutoSizeValue(strval);
            ptr->setAutoSize( val );
        }
    }

    return as_value();
}

as_value
textfield_type(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);

    if (!fn.nargs)
    {
        // getter
        return ptr->typeValueName(ptr->getType());
    }

    // setter
    const as_value& arg = fn.arg(0);
    std::string strval = arg.to_string();
    TextField::TypeValue val = ptr->parseTypeValue(strval);

    IF_VERBOSE_ASCODING_ERRORS(
        if ( val == TextField::typeInvalid )
        {
            log_aserror(_("Invalid value given to TextField.type: %s"), strval);
        }
    );
    ptr->setType(val);
    return as_value();
}


as_value
textfield_variable(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    if (!fn.nargs)
    {
        // Getter
        const std::string& varName = text->getVariableName();
        // An empty variable name returns null.
        if (varName.empty()) {
            as_value null;
            null.set_null();
            return null;
        }
        return as_value(varName);
    }

    // Setter
    const as_value& varName = fn.arg(0);
    if (varName.is_undefined() || varName.is_null()) {
        text->set_variable_name("");
    }
    else text->set_variable_name(varName.to_string());

    return as_value();

}


as_value
textfield_getDepth(const fn_call& fn)
{
    // TODO: make this a DisplayObject::getDepth_method function...
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    int n = text->get_depth();

    return as_value(n);

}

as_value
textfield_getFontList(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE(log_unimpl("TextField.getFontList()"));

    return as_value();
}

as_value
textfield_getNewTextFormat(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE(log_unimpl("TextField.getNewTextFormat()"));

    return as_value();
}

as_value
textfield_getTextFormat(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    boost::intrusive_ptr<TextFormat_as> tf = new TextFormat_as;
    tf->alignSet(text->getTextAlignment());
    tf->sizeSet(text->getFontHeight());
    tf->indentSet(text->getIndent());
    tf->blockIndentSet(text->getBlockIndent());
    tf->leadingSet(text->getLeading());
    tf->leftMarginSet(text->getLeftMargin());
    tf->rightMarginSet(text->getRightMargin());
    tf->colorSet(text->getTextColor());
    tf->underlinedSet(text->getUnderlined());

    const Font* font = text->getFont();
    if (font)
    {
        tf->fontSet(font->name());
        tf->italicedSet(font->isItalic());
        tf->boldSet(font->isBold());
    }

    // TODO: add font color and some more

    LOG_ONCE(
        log_unimpl("TextField.getTextFormat() discards url, target, "
            "tabStops, bullet and display")
    );

    return as_value(tf.get());
}

as_value
textfield_setTextFormat(const fn_call& fn)
{

    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("TextField.setTextFormat(%s) : %s", ss.str(),
            _("missing arg"))
        );
        return as_value();
    }
    else if ( fn.nargs > 2 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_debug("TextField.setTextFormat(%s) : args past the first are "
                "unhandled by Gnash", ss.str());
    }

    as_object* obj = fn.arg(0).to_object().get();
    if ( ! obj )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("TextField.setTextFormat(%s) : %s", ss.str(), 
            _("first argument is not an object"))
        );
        return as_value();
    }

    TextFormat_as* tf = dynamic_cast<TextFormat_as*>(obj);
    if ( ! tf )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("TextField.setTextFormat(%s) : %s", ss.str(),
            _("first argument is not a TextFormat"))
        );
        return as_value();
    }

    if ( tf->alignDefined() ) text->setAlignment(tf->align());
    if ( tf->sizeDefined() ) text->setFontHeight(tf->size()); // keep twips
    if ( tf->indentDefined() ) text->setIndent(tf->indent());
    if ( tf->blockIndentDefined() ) text->setBlockIndent(tf->blockIndent());
    if ( tf->leadingDefined() ) text->setLeading(tf->leading());
    if ( tf->leftMarginDefined() ) text->setLeftMargin(tf->leftMargin());
    if ( tf->rightMarginDefined() ) text->setRightMargin(tf->rightMargin());
    if ( tf->colorDefined() ) text->setTextColor(tf->color());
    if ( tf->underlinedDefined() ) text->setUnderlined(tf->underlined());

    if (isAS3(fn)) {
        // TODO: current font finding assumes we have a parent, which isn't
        // necessarily the case in AS3. It seems the AS2 implementation is
        // wrong anyway.
        log_unimpl("fonts in AS3 TextField.setTextFormat");
        return as_value();
    }

    if ( tf->fontDefined() )
    {
        const std::string& fontName = tf->font();
        if ( ! fontName.empty() )
        {
            bool bold = tf->bold();
            bool italic = tf->italiced();

            // NOTE: should query movie-private font lib, not global-shared one
            Movie* mi = text->get_root();
            assert(mi);
            const movie_definition* md = mi->definition();
            assert(md);
            Font* f = md->get_font(fontName, bold, italic);
            if ( ! f ) f = fontlib::get_font(fontName, bold, italic);
            text->setFont( f );
        }
    }

    // TODO: add font color and some more

    LOG_ONCE( log_unimpl("TextField.setTextFormat() discards url, target, "
                "tabStops, bullet and display") );

    return as_value();

}

as_value
textfield_setNewTextFormat(const fn_call& fn)
{
    //boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    //UNUSED(text);

    LOG_ONCE( log_unimpl("TextField.setNewTextFormat(), we'll delegate "
                "to setTextFormat") );
    return textfield_setTextFormat(fn);

    //return as_value();
}

as_value
textfield_password(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    LOG_ONCE(log_unimpl("TextField.password"));

    if (!fn.nargs)
    {
        // Getter
        return as_value(text->password());
    }
    // Setter
    text->password(fn.arg(0).to_bool());
    return as_value();
}

as_value
textfield_multiline(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    LOG_ONCE(log_unimpl("TextField.multiline"));

    if (!fn.nargs) {
        // Getter
        return as_value(text->multiline());
    }
    // Setter
    text->multiline(fn.arg(0).to_bool());
    return as_value();
}

as_value
textfield_restrict(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE (log_unimpl("TextField.restrict"));

    return as_value();
}

as_value
textfield_bottomScroll(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE (log_unimpl("TextField.bottomScroll"));

    return as_value();
}

as_value
textfield_maxhscroll(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE (log_unimpl("TextField.maxhscroll"));

    return as_value();
}

/// TextField.maxChars().
//
/// This does not limit the length of the text, but rather the
/// number of DisplayObjects that can be entered in the TextField.
//
/// Returns null when the value is 0.
as_value
textfield_maxChars(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    LOG_ONCE(log_unimpl("TextField.maxChars"));

    if (!fn.nargs)
    {
        boost::int32_t maxChars = text->maxChars();
        if (maxChars == 0)
        {
            as_value null;
            null.set_null();
            return null;
        }
        return as_value(maxChars);
    }
    // Setter
    text->maxChars(fn.arg(0).to_int());
    return as_value();
}

as_value
textfield_text(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        //
        // FIXME: should return text without HTML tags.
        return as_value(ptr->get_text_value());
    }

    // Setter
    int version = ptr->getVM().getSWFVersion();
    ptr->setTextValue(
            utf8::decodeCanonicalString(fn.arg(0).to_string(), version));

    return as_value();
}

as_value
textfield_htmlText(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> ptr = ensureType<TextField>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->get_text_value());
    }

    // Setter
    int version = ptr->getVM().getSWFVersion();
    ptr->setTextValue(
            utf8::decodeCanonicalString(fn.arg(0).to_string(), version));

    return as_value();
}

/// TextField.replaceSel(newText)
//
/// Replaces the current selection with the new text, setting both
/// begin and end of the selection to one after the inserted text.
/// If an empty string is passed, SWF8 erases the selection; SWF7 and below
/// is a no-op.
/// If no argument is passed, this is a no-op.
as_value
textfield_replaceSel(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror("TextField.replaceSel(%s) requires exactly one "
                "argument", os.str());
        );
        return as_value();
    }

    const std::string& replace = fn.arg(0).to_string();

    /// Do nothing if text is empty and version less than 8.
    const int version = text->getVM().getSWFVersion();
    if (version < 8 && replace.empty()) return as_value();

    text->replaceSelection(replace);

    return as_value();
}

as_value
textfield_scroll(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE (log_unimpl("TextField.scroll()"));

    return as_value();
}

as_value
textfield_hscroll(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE (log_unimpl("TextField.hscroll()"));

    return as_value();
}

as_value
textfield_maxscroll(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE (log_unimpl("TextField.maxscroll"));

    return as_value();
}

as_value
textfield_replaceText(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);
    UNUSED(text);

    LOG_ONCE(log_unimpl("TextField.replaceText()"));

    return as_value();
}

as_value
textfield_removeTextField(const fn_call& fn)
{
    boost::intrusive_ptr<TextField> text = ensureType<TextField>(fn.this_ptr);

    text->removeTextField();

    LOG_ONCE(log_debug("TextField.removeTextField() TESTING"));

    return as_value();
}


/// This is called for 'new TextField()' only
as_value
textfield_ctor(const fn_call& fn)
{

    VM& vm = fn.getVM();

    as_object* proto = getTextFieldInterface(vm);

    as_object* obj = 0;

    if (!isAS3(fn)) {
        // We should attach more properties to the prototype on first
        // instantiation.
        // TODO: this also attaches properties to the SWF5 prototype but makes
        // them invisible with prop flags. Is this correct?
        attachPrototypeProperties(*proto);

        obj = new as_object(proto);
    }
    else {
        rect nullRect;
        obj = new TextField(0, nullRect);
    }

    return as_value(obj);
}


void
attachTextFieldInterface(as_object& o)
{
    boost::intrusive_ptr<builtin_function> getset;

    // TextField is an AsBroadcaster
    AsBroadcaster::initialize(o);

    int propFlags = as_prop_flags::dontDelete
        |as_prop_flags::dontEnum
        |as_prop_flags::readOnly
        |as_prop_flags::isProtected;

    // Parent seems to not be a normal property
    getset = new builtin_function(&DisplayObject::parent_getset, NULL);
    o.init_property(NSV::PROP_uPARENT, *getset, *getset);

    // Target seems to not be a normal property
    getset = new builtin_function(&DisplayObject::target_getset, NULL);
    o.init_property(NSV::PROP_uTARGET, *getset, *getset);

    // _name should be a property of the instance, not the prototype
    getset = new builtin_function(&DisplayObject::name_getset, NULL);
    o.init_property(NSV::PROP_uNAME, *getset, *getset);

    o.init_property(NSV::PROP_uXMOUSE,
            DisplayObject::xmouse_get, DisplayObject::xmouse_get, propFlags);
    o.init_property(NSV::PROP_uYMOUSE,
            DisplayObject::ymouse_get, DisplayObject::ymouse_get, propFlags);
    o.init_property(NSV::PROP_uHIGHQUALITY,
            DisplayObject::highquality, DisplayObject::highquality);
    o.init_property(NSV::PROP_uQUALITY,
            DisplayObject::quality, DisplayObject::quality);
    o.init_property(NSV::PROP_uXSCALE,
            DisplayObject::xscale_getset, DisplayObject::xscale_getset);
    o.init_property(NSV::PROP_uYSCALE,
            DisplayObject::yscale_getset, DisplayObject::yscale_getset);
 
    // Standard flags.
    const int flags = as_prop_flags::dontDelete
        |as_prop_flags::dontEnum;

    // SWF6 or higher
    const int swf6Flags = flags | as_prop_flags::onlySWF6Up;

    o.init_member("setTextFormat", 
            new builtin_function(textfield_setTextFormat), swf6Flags);
    o.init_member("getTextFormat", 
            new builtin_function(textfield_getTextFormat), swf6Flags);
    o.init_member("setNewTextFormat",
            new builtin_function(textfield_setNewTextFormat), swf6Flags);
    o.init_member("getNewTextFormat",
            new builtin_function(textfield_getNewTextFormat), swf6Flags);
    o.init_member("getNewTextFormat",
            new builtin_function(textfield_getNewTextFormat), swf6Flags);
    o.init_member("getDepth",
            new builtin_function(textfield_getDepth), swf6Flags);
    o.init_member("removeTextField",
            new builtin_function(textfield_removeTextField), swf6Flags);
    o.init_member("replaceSel",
            new builtin_function(textfield_replaceSel), swf6Flags);

    // SWF7 or higher
    const int swf7Flags = flags | as_prop_flags::onlySWF7Up;

    o.init_member("replaceText",
            new builtin_function(textfield_replaceText), swf7Flags);

}

void
attachTextFieldStaticMembers(as_object& o)
{
    // Standard flags.
    const int flags = as_prop_flags::dontDelete
        |as_prop_flags::dontEnum;

    // SWF6 or higher
    const int swf6Flags = flags | as_prop_flags::onlySWF6Up;

    o.init_member("getFontList",
            new builtin_function(textfield_getFontList), swf6Flags);

}

/// This is called when a prototype should be added
//
/// @note   This is called at different times, depending on the version.
///         For SWF5 it is called only on first instantiation. For SWF6 it
///         is called at the registration of _global.TextField.
as_object*
getTextFieldInterface(VM& vm)
{
    static boost::intrusive_ptr<as_object> proto;

    if ( proto == NULL )
    {
        if (vm.getSWFVersion() < 6) {
            /// The prototype for SWF5 is a simple as_object without
            /// toString() or valueOf().
            proto = new as_object();
            vm.addStatic(proto.get());
        }
        else {
            proto = new as_object(getObjectInterface());
            vm.addStatic(proto.get());
            attachTextFieldInterface(*proto);
        }

    }
    return proto.get();
}

} // anonymous namespace

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

