// TextSnapshot_as.cpp:  ActionScript "TextSnapshot" class, for Gnash.
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

#include "TextSnapshot_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "generic_character.h"
#include "DisplayList.h"
#include "MovieClip.h"

#include <boost/algorithm/string/compare.hpp>
#include <algorithm>

namespace gnash {

// Forward declarations
namespace {

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

    void setTextReachable( const TextSnapshot_as::TextFields::value_type& vt);

}

namespace {

class TextFinder
{
public:
    TextFinder(TextSnapshot_as::TextFields& fields) : _fields(fields) {}

    void operator()(character* ch) {
        if (ch->isUnloaded()) return;
        std::string text;
        generic_character* tf;
        if ((tf = ch->getStaticText(text))) {
            _fields.push_back(std::make_pair(tf, text));
        }
    }

private:
        TextSnapshot_as::TextFields& _fields;
};

} // anonymous namespace


TextSnapshot_as::TextSnapshot_as(const MovieClip* mc)
    :
    as_object(getTextSnapshotInterface()),
    _valid(mc)
{
    if (mc) {
        const DisplayList& dl = mc->getDisplayList();

        TextFinder finder(_textFields);
        dl.visitAll(finder);
    }
}

void
TextSnapshot_as::markReachableResources() const
{
    std::for_each(_textFields.begin(), _textFields.end(), setTextReachable);
}

void
TextSnapshot_as::makeString(std::string& to, bool newline) const
{
    for (TextFields::const_iterator it = _textFields.begin(),
            e = _textFields.end(); it != e; ++it)
    {
        if (newline && it != _textFields.begin()) to += '\n';
        to += it->second;
    }
}

const std::string
TextSnapshot_as::getText(boost::int32_t start, boost::int32_t end, bool nl)
    const
{
    std::string snapshot;
    makeString(snapshot, nl);

    const std::string::size_type len = snapshot.size();

    if (len == 0) return std::string();

    // Start is always moved to between 0 and len - 1.
    start = std::max<boost::int32_t>(start, 0);
    start = std::min<std::string::size_type>(len - 1, start);

    // End is always moved to between start and end. We don't really care
    // about the end of the string.
    end = std::max(start + 1, end);

    return snapshot.substr(start, end - start);

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

void
TextSnapshot_as::init(as_object& global)
{
	// This is going to be the global TextSnapshot "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&textsnapshot_ctor, getTextSnapshotInterface());
	}

	global.init_member("TextSnapshot", cl.get());
}


namespace {

void
attachTextSnapshotInterface(as_object& o)
{

    const int flags = as_prop_flags::onlySWF6Up;

	o.init_member("findText", new builtin_function(textsnapshot_findText),
            flags);
	o.init_member("getCount", new builtin_function(textsnapshot_getCount),
            flags);
	o.init_member("getTextRunInfo",
            new builtin_function(textsnapshot_getTextRunInfo), flags);
	o.init_member("getSelected",
            new builtin_function(textsnapshot_getSelected), flags);
	o.init_member("getSelectedText",
            new builtin_function(textsnapshot_getSelectedText), flags);
	o.init_member("getText",
            new builtin_function(textsnapshot_getText), flags);
	o.init_member("hitTestTextNearPos",
            new builtin_function(textsnapshot_hitTestTextNearPos), flags);
	o.init_member("setSelectColor",
            new builtin_function(textsnapshot_setSelectColor), flags);
	o.init_member("setSelected",
            new builtin_function(textsnapshot_setSelected), flags);
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

as_value textsnapshot_getTextRunInfo(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
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

    log_unimpl (__FUNCTION__);
    return as_value();
}

/// Returns a string, or undefined if not valid.
as_value
textsnapshot_getSelectedText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (!ts->valid()) return as_value();

    log_unimpl (__FUNCTION__);
    return as_value();
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

    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_ctor(const fn_call& fn)
{
    MovieClip* mc = (fn.nargs == 1) ? fn.arg(0).to_sprite() : 0;
    return as_value(new TextSnapshot_as(mc));
}

void
setTextReachable(const TextSnapshot_as::TextFields::value_type& vt)
{
    vt.first->setReachable();
}

} // anonymous namespace
} // gnash namespace
