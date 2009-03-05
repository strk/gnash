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

TextSnapshot_as::TextSnapshot_as()
    :
    as_object(getTextSnapshotInterface())
{
}

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



TextSnapshot_as::TextSnapshot_as(const MovieClip& mc)
    :
    as_object(getTextSnapshotInterface())
{
    const DisplayList& dl = mc.getDisplayList();

    TextFinder finder(_textFields);
    dl.visitAll(finder);
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
    start = std::max(start, 0);
    start = std::min<std::string::size_type>(len - 1, start);

    // End is always moved to between start and end. We don't really care
    // about end.
    end = std::max(start + 1, end);

    return snapshot.substr(start, end - start);

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

	// Register _global.TextSnapshot
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
as_value textsnapshot_findText(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_getCount(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    
    if (fn.nargs)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("TextSnapshot.getCount takes no arguments");
        );
        return as_value();
    }


    return ts->getCount();
}

as_value textsnapshot_getSelected(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value textsnapshot_getSelectedText(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_getText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ts =
        ensureType<TextSnapshot_as>(fn.this_ptr);

    if (fn.nargs < 2 || fn.nargs > 3)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("TextSnapshot.getText requires exactly 2 arguments");
        );
        return as_value();
    }

    boost::int32_t start = fn.arg(0).to_int();
    boost::int32_t end = fn.arg(1).to_int();

    const bool newline = fn.nargs == 3 ? fn.arg(2).to_bool() : false;

    return ts->getText(start, end, newline);

}


as_value textsnapshot_hitTestTextNearPos(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value textsnapshot_setSelectColor(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value textsnapshot_setSelected(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new TextSnapshot_as;

	return as_value(obj.get()); // will keep alive
}

void
setTextReachable(const TextSnapshot_as::TextFields::value_type& vt)
{
    vt.first->setReachable();
}

} // anonymous namespace
} // end of gnash namespace
