// TextSnapshot_as.cpp:  ActionScript "TextSnapshot" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "text/TextSnapshot_as.h"
#include "GnashException.h" // for ActionException

#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "StaticText.h"
#include "DisplayList.h"
#include "MovieClip.h"
#include "Font.h"
#include "swf/TextRecord.h"
#include "Array_as.h"
#include "RGBA.h"
#include "GnashNumeric.h"

#include <boost/algorithm/string/compare.hpp>
#include <boost/dynamic_bitset.hpp>
#include <algorithm>

namespace gnash {

// Forward declarations
namespace {
    void attachTextSnapshotStaticInterface(as_object& o);
    
    as_value textsnapshot_findText(const fn_call& fn);
    as_value textsnapshot_getCount(const fn_call& fn);
    as_value textsnapshot_getSelected(const fn_call& fn);
    as_value textsnapshot_getSelectedText(const fn_call& fn);
    as_value textsnapshot_getTextRunInfo(const fn_call& fn);
    as_value textsnapshot_getText(const fn_call& fn);
    as_value textsnapshot_hitTestTextNearPos(const fn_call& fn);
    as_value textsnapshot_setSelectColor(const fn_call& fn);
    as_value textsnapshot_setSelected(const fn_call& fn);
    as_value textsnapshot_ctor(const fn_call& fn);

    void attachTextSnapshotInterface(as_object& o);
    as_object* getTextSnapshotInterface();

    size_t getTextFields(const MovieClip* mc,
            TextSnapshot_as::TextFields& fields);

    void setTextReachable(const TextSnapshot_as::TextFields::value_type& vt);

}

// extern (used by Global.cpp)
void TextSnapshot_as::init(as_object& global)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&textsnapshot_ctor, getTextSnapshotInterface());;
        attachTextSnapshotStaticInterface(*cl);
    }

    // Register _global.TextSnapshot
    global.init_member("TextSnapshot", cl.get());
}

/// The member _textFields is initialized here unnecessarily to show
/// that it is constructed before it is used.
TextSnapshot_as::TextSnapshot_as(const MovieClip* mc)
    :
    as_object(getTextSnapshotInterface()),
    _textFields(),
    _valid(mc),
    _count(getTextFields(mc, _textFields))
{
}

void
TextSnapshot_as::setSelected(size_t start, size_t end, bool selected)
{
    /// If there are no TextFields, there is nothing to do.
    if (_textFields.empty()) return;

    start = std::min(start, _count);
    end = std::min(end, _count);

    TextFields::const_iterator field = _textFields.begin();

    size_t totalChars = field->first->getSelected().size();
    size_t fieldStartIndex = 0;

    for (size_t i = start; i < end; ++i) {

        /// Find the field containing the start index.
        while (totalChars <= i) {
            fieldStartIndex = totalChars;
            ++field;

            if (field == _textFields.end()) return;
            
            const boost::dynamic_bitset<>& sel = field->first->getSelected();
            totalChars += sel.size();
            continue;
        }
        field->first->setSelected(i - fieldStartIndex, selected);
    }
}

bool
TextSnapshot_as::getSelected(size_t start, size_t end) const
{

    if (_textFields.empty()) return false;

    start = std::min(start, _count);
    end = std::min(end, _count);

    TextFields::const_iterator field = _textFields.begin();

    size_t totalChars = field->first->getSelected().size();
    size_t fieldStartIndex = 0;

    for (size_t i = start; i < end; ++i) {

        /// Find the field containing the start index.
        while (totalChars <= i) {
            fieldStartIndex = totalChars;
            ++field;
            if (field == _textFields.end()) return false;

            const boost::dynamic_bitset<>& sel = field->first->getSelected();
            totalChars += sel.size();
            continue;
        }

        if (field->first->getSelected().test(i - fieldStartIndex)) return true;
    }
    
    return false;
}

void
TextSnapshot_as::markReachableResources() const
{
    std::for_each(_textFields.begin(), _textFields.end(), setTextReachable);
    markAsObjectReachable();
}

void
TextSnapshot_as::getTextRunInfo(size_t start, size_t end, Array_as& ri) const
{
    std::string::size_type pos = 0;

    std::string::size_type len = end - start;

    for (TextFields::const_iterator field = _textFields.begin(),
            e = _textFields.end(); field != e; ++field) {

        const Records& rec = field->second;
        const SWFMatrix& mat = field->first->getMatrix();
        const boost::dynamic_bitset<>& selected = field->first->getSelected();

        const std::string::size_type fieldStartIndex = pos;

        for (Records::const_iterator j = rec.begin(), end = rec.end();
                j != end; ++j) {
        
            const SWF::TextRecord* tr = *j;
            assert(tr);

            const SWF::TextRecord::Glyphs& glyphs = tr->glyphs();
            const SWF::TextRecord::Glyphs::size_type numGlyphs = glyphs.size();

            if (pos + numGlyphs < start) {
                pos += numGlyphs;
                continue;
            }

            const Font* font = tr->getFont();
            assert(font);

            double x = tr->xOffset();
            for (SWF::TextRecord::Glyphs::const_iterator k = glyphs.begin(),
                    e = glyphs.end(); k != e; ++k) {
                
                if (pos < start) {
                    x += k->advance;
                    ++pos;
                    continue;
                }
                
                as_object* el = new as_object;

                el->init_member("indexInRun", pos);
                el->init_member("selected",
                        selected.test(pos - fieldStartIndex));
                el->init_member("font", font->name());
                el->init_member("color", tr->color().toRGBA());
                el->init_member("height", twipsToPixels(tr->textHeight()));

                const double factor = 65536.0;
                el->init_member("matrix_a", mat.sx / factor);
                el->init_member("matrix_b", mat.shx / factor);
                el->init_member("matrix_c", mat.shy / factor);
                el->init_member("matrix_d", mat.sy / factor);

                const double xpos = twipsToPixels(mat.tx + x);
                const double ypos = twipsToPixels(mat.ty + tr->yOffset());
                el->init_member("matrix_tx", xpos);
                el->init_member("matrix_ty", ypos);

                ri.push(el);

                ++pos;
                x += k->advance;
                if (pos - start > len) return;
            }
        }
    }
    
}

void
TextSnapshot_as::makeString(std::string& to, bool newline, bool selectedOnly,
        std::string::size_type start, std::string::size_type len) const
{

    std::string::size_type pos = 0;

    for (TextFields::const_iterator field = _textFields.begin(),
            e = _textFields.end(); field != e; ++field)
    {
        // When newlines are requested, insert one after each individual
        // text field is processed.
        if (newline && pos > start) to += '\n';

        const Records& records = field->second;
        const boost::dynamic_bitset<>& selected = field->first->getSelected();

        /// Remember the position at the beginning of the StaticText.
        const std::string::size_type fieldStartIndex = pos;

        for (Records::const_iterator j = records.begin(), end = records.end();
                j != end; ++j) {
        
            const SWF::TextRecord* tr = *j;
            assert(tr);

            const SWF::TextRecord::Glyphs& glyphs = tr->glyphs();
            const SWF::TextRecord::Glyphs::size_type numGlyphs = glyphs.size();

            if (pos + numGlyphs < start) {
                pos += numGlyphs;
                continue;
            }

            const Font* font = tr->getFont();
            assert(font);

            for (SWF::TextRecord::Glyphs::const_iterator k = glyphs.begin(),
                    e = glyphs.end(); k != e; ++k) {
                
                if (pos < start) {
                    ++pos;
                    continue;
                }
                
                if (!selectedOnly || selected.test(pos - fieldStartIndex)) {
                    to += font->codeTableLookup(k->index, true);
                }
                ++pos;
                if (pos - start == len) return;
            }
        }
    }
}

std::string
TextSnapshot_as::getText(boost::int32_t start, boost::int32_t end, bool nl)
    const
{

    // Start is always moved to between 0 and len - 1.
    start = std::max<boost::int32_t>(start, 0);
    start = std::min<boost::int32_t>(start, _count - 1);

    // End is always moved to between start and end. We don't really care
    // about the end of the string.
    end = std::max(start + 1, end);

    std::string snapshot;
    makeString(snapshot, nl, false, start, end - start);

    return snapshot;

}

std::string
TextSnapshot_as::getSelectedText(bool newline) const
{
    std::string sel;
    
    makeString(sel, newline, true);
    return sel;
}

boost::int32_t
TextSnapshot_as::findText(boost::int32_t start, const std::string& text,
        bool ignoreCase) const
{

    if (start < 0 || text.empty()) return -1;

    std::string snapshot;
    makeString(snapshot);

    const std::string::size_type len = snapshot.size();

    // Don't try to search if start is past the end of the string.
    if (len < static_cast<size_t>(start)) return -1;

    if (ignoreCase) {
        std::string::const_iterator it = std::search(snapshot.begin() + start,
                snapshot.end(), text.begin(), text.end(), boost::is_iequal());
        return (it == snapshot.end()) ? -1 : it - snapshot.begin();
    }

    std::string::size_type pos = snapshot.find(text, start);
    return (pos == std::string::npos) ? -1 : pos;

}

namespace {

class TextFinder
{
public:
    TextFinder(TextSnapshot_as::TextFields& fields)
        :
        _fields(fields),
        _count(0)
    {}

    void operator()(DisplayObject* ch) {

        /// This is not tested.
        if (ch->unloaded()) return;

        TextSnapshot_as::Records text;
        StaticText* tf;
        size_t numChars;

        if ((tf = ch->getStaticText(text, numChars))) {
            _fields.push_back(std::make_pair(tf, text));
            _count += numChars;
        }
    }

    size_t getCount() const { return _count; }

private:
    TextSnapshot_as::TextFields& _fields;
    size_t _count;
};

void
attachTextSnapshotStaticInterface(as_object& o)
{

}

void
attachTextSnapshotInterface(as_object& o)
{

    const int flags = as_prop_flags::onlySWF6Up;

    Global_as* gl = getGlobal(o);
	o.init_member("findText", gl->createFunction(textsnapshot_findText),
            flags);
	o.init_member("getCount", gl->createFunction(textsnapshot_getCount),
            flags);
	o.init_member("getTextRunInfo",
            gl->createFunction(textsnapshot_getTextRunInfo), flags);
	o.init_member("getSelected",
            gl->createFunction(textsnapshot_getSelected), flags);
	o.init_member("getSelectedText",
            gl->createFunction(textsnapshot_getSelectedText), flags);
	o.init_member("getText",
            gl->createFunction(textsnapshot_getText), flags);
	o.init_member("hitTestTextNearPos",
            gl->createFunction(textsnapshot_hitTestTextNearPos), flags);
	o.init_member("setSelectColor",
            gl->createFunction(textsnapshot_setSelectColor), flags);
	o.init_member("setSelected",
            gl->createFunction(textsnapshot_setSelected), flags);
}

as_object*
getTextSnapshotInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachTextSnapshotInterface(*o);
	}
	return o.get();
}

as_value
textsnapshot_getTextRunInfo(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    
    if (!ts->valid()) return as_value();

    if (fn.nargs != 2) {
        return as_value();
    }

    size_t start = std::max<boost::int32_t>(0, fn.arg(0).to_int());
    size_t end = std::max<boost::int32_t>(start + 1, fn.arg(1).to_int());

    Array_as* ri = new Array_as;

    ts->getTextRunInfo(start, end, *ri);
    
    return as_value(ri);
}

as_value
textsnapshot_findText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    
    if (!ts->valid()) return as_value();

    if (fn.nargs != 3) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("TextSnapshot.findText() requires 3 arguments");
        );
        return as_value();
    }

    boost::int32_t start = fn.arg(0).to_int();
    const std::string& text = fn.arg(1).to_string();

    /// Yes, the pp is case-insensitive by default. We don't write
    /// functions like that here.
    bool ignoreCase = !fn.arg(2).to_bool();

    return ts->findText(start, text, ignoreCase);
}

as_value
textsnapshot_getCount(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    
    if (!ts->valid()) return as_value();

    if (fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("TextSnapshot.getCount() takes no arguments");
        );
        return as_value();
    }

    return ts->getCount();
}

/// Returns a boolean value, or undefined if not valid.
as_value
textsnapshot_getSelected(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (!ts->valid()) return as_value();

    if (fn.nargs != 2) {
        return as_value();
    }

    size_t start = std::max<boost::int32_t>(0, fn.arg(0).to_int());
    size_t end = std::max<boost::int32_t>(start + 1, fn.arg(1).to_int());

    return as_value(ts->getSelected(start, end));
}

/// Returns a string, or undefined if not valid.
as_value
textsnapshot_getSelectedText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (!ts->valid()) return as_value();

    if (fn.nargs > 1) {
        return as_value();
    }

    bool newlines = fn.nargs ? fn.arg(0).to_bool() : false;

    return as_value(ts->getSelectedText(newlines));
}

/// Returns a string, or undefined if not valid.
as_value
textsnapshot_getText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (!ts->valid()) return as_value();
    
    if (fn.nargs < 2 || fn.nargs > 3)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("TextSnapshot.getText requires exactly 2 arguments");
        );
        return as_value();
    }

    boost::int32_t start = fn.arg(0).to_int();
    boost::int32_t end = fn.arg(1).to_int();

    const bool newline = (fn.nargs > 2) ? fn.arg(2).to_bool() : false;

    return ts->getText(start, end, newline);

}


/// Returns bool, or undefined if not valid.
as_value
textsnapshot_hitTestTextNearPos(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (!ts->valid()) return as_value();

    log_unimpl (__FUNCTION__);
    return as_value();
}

/// Returns void.
as_value
textsnapshot_setSelectColor(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    log_unimpl (__FUNCTION__);
    return as_value();
}


/// Returns void.
as_value
textsnapshot_setSelected(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (fn.nargs < 2 || fn.nargs > 3) {
        return as_value();
    }

    size_t start = std::max<boost::int32_t>(0, fn.arg(0).to_int());
    size_t end = std::max<boost::int32_t>(start, fn.arg(1).to_int());

    bool selected = (fn.nargs > 2) ? fn.arg(2).to_bool() : true;

    ts->setSelected(start, end, selected);

    return as_value();
}

as_value
textsnapshot_ctor(const fn_call& fn)
{
    MovieClip* mc = (fn.nargs == 1) ? fn.arg(0).to_sprite() : 0;
    return as_value(new TextSnapshot_as(mc));
}

size_t
getTextFields(const MovieClip* mc, TextSnapshot_as::TextFields& fields)
{
    if (mc) {
        const DisplayList& dl = mc->getDisplayList();

        TextFinder finder(fields);
        dl.visitAll(finder);
        return finder.getCount();
    }
    return 0;
}

void
setTextReachable(const TextSnapshot_as::TextFields::value_type& vt)
{
    vt.first->setReachable();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

