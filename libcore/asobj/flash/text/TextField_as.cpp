// TextField_as.cpp:  ActionScript "TextField" class, for Gnash.
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

#include "text/TextField_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value textfield_getCharBoundaries(const fn_call& fn);
    as_value textfield_getCharIndexAtPoint(const fn_call& fn);
    as_value textfield_getFirstCharInParagraph(const fn_call& fn);
    as_value textfield_getImageReference(const fn_call& fn);
    as_value textfield_getLineIndexAtPoint(const fn_call& fn);
    as_value textfield_getLineIndexOfChar(const fn_call& fn);
    as_value textfield_getLineLength(const fn_call& fn);
    as_value textfield_getLineMetrics(const fn_call& fn);
    as_value textfield_getLineOffset(const fn_call& fn);
    as_value textfield_getLineText(const fn_call& fn);
    as_value textfield_getParagraphLength(const fn_call& fn);
    as_value textfield_getTextFormat(const fn_call& fn);
    as_value textfield_replaceSelectedText(const fn_call& fn);
    as_value textfield_replaceText(const fn_call& fn);
    as_value textfield_setSelection(const fn_call& fn);
    as_value textfield_setTextFormat(const fn_call& fn);
    as_value textfield_change(const fn_call& fn);
    as_value textfield_link(const fn_call& fn);
    as_value textfield_scroll(const fn_call& fn);
    as_value textfield_textInput(const fn_call& fn);
    as_value textfield_ctor(const fn_call& fn);
    void attachTextFieldInterface(as_object& o);
    void attachTextFieldStaticInterface(as_object& o);
    as_object* getTextFieldInterface();

}

class TextField_as : public as_object
{

public:

    TextField_as()
        :
        as_object(getTextFieldInterface())
    {}
};

// extern (used by Global.cpp)
void textfield_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&textfield_ctor, getTextFieldInterface());
        attachTextFieldStaticInterface(*cl);
    }

    // Register _global.TextField
    global.init_member("TextField", cl.get());
}

namespace {

void
attachTextFieldInterface(as_object& o)
{
    o.init_member("getCharBoundaries", gl->createFunction(textfield_getCharBoundaries));
    o.init_member("getCharIndexAtPoint", gl->createFunction(textfield_getCharIndexAtPoint));
    o.init_member("getFirstCharInParagraph", gl->createFunction(textfield_getFirstCharInParagraph));
    o.init_member("getImageReference", gl->createFunction(textfield_getImageReference));
    o.init_member("getLineIndexAtPoint", gl->createFunction(textfield_getLineIndexAtPoint));
    o.init_member("getLineIndexOfChar", gl->createFunction(textfield_getLineIndexOfChar));
    o.init_member("getLineLength", gl->createFunction(textfield_getLineLength));
    o.init_member("getLineMetrics", gl->createFunction(textfield_getLineMetrics));
    o.init_member("getLineOffset", gl->createFunction(textfield_getLineOffset));
    o.init_member("getLineText", gl->createFunction(textfield_getLineText));
    o.init_member("getParagraphLength", gl->createFunction(textfield_getParagraphLength));
    o.init_member("getTextFormat", gl->createFunction(textfield_getTextFormat));
    o.init_member("replaceSelectedText", gl->createFunction(textfield_replaceSelectedText));
    o.init_member("replaceText", gl->createFunction(textfield_replaceText));
    o.init_member("setSelection", gl->createFunction(textfield_setSelection));
    o.init_member("setTextFormat", gl->createFunction(textfield_setTextFormat));
    o.init_member("change", gl->createFunction(textfield_change));
    o.init_member("link", gl->createFunction(textfield_link));
    o.init_member("scroll", gl->createFunction(textfield_scroll));
    o.init_member("textInput", gl->createFunction(textfield_textInput));
}

void
attachTextFieldStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);

}

as_object*
getTextFieldInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachTextFieldInterface(*o);
    }
    return o.get();
}

as_value
textfield_getCharBoundaries(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getCharIndexAtPoint(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getFirstCharInParagraph(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getImageReference(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getLineIndexAtPoint(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getLineIndexOfChar(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getLineLength(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getLineMetrics(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getLineOffset(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getLineText(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getParagraphLength(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_getTextFormat(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_replaceSelectedText(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_replaceText(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_setSelection(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_setTextFormat(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_change(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_link(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_scroll(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_textInput(const fn_call& fn)
{
    boost::intrusive_ptr<TextField_as> ptr =
        ensureType<TextField_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textfield_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new TextField_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

