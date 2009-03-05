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
}

TextSnapshot_as::TextSnapshot_as()
    :
    as_object(getTextSnapshotInterface())
{
}

TextSnapshot_as::TextSnapshot_as(const std::string& snapshot)
    :
    as_object(getTextSnapshotInterface()),
    _snapshot(snapshot)
{
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
as_value textsnapshot_getText(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
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


} // anonymous namespace
} // end of gnash namespace
