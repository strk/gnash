// StyleSheet_as.cpp:  ActionScript "StyleSheet" class, for Gnash.
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

#include "text/StyleSheet_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value stylesheet_getStyle(const fn_call& fn);
    as_value stylesheet_parseCSS(const fn_call& fn);
    as_value stylesheet_setStyle(const fn_call& fn);
    as_value stylesheet_transform(const fn_call& fn);
    as_value stylesheet_ctor(const fn_call& fn);
    void attachStyleSheetInterface(as_object& o);
    void attachStyleSheetStaticInterface(as_object& o);
    as_object* getStyleSheetInterface();

}

class StyleSheet_as : public as_object
{

public:

    StyleSheet_as()
        :
        as_object(getStyleSheetInterface())
    {}
};

// extern (used by Global.cpp)
void stylesheet_class_init(as_object& global, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&stylesheet_ctor, getStyleSheetInterface());
        attachStyleSheetStaticInterface(*cl);
    }

    // Register _global.StyleSheet
    global.init_member("StyleSheet", cl.get());
}

namespace {

void
attachStyleSheetInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("getStyle", gl->createFunction(stylesheet_getStyle));
    o.init_member("parseCSS", gl->createFunction(stylesheet_parseCSS));
    o.init_member("setStyle", gl->createFunction(stylesheet_setStyle));
    o.init_member("transform", gl->createFunction(stylesheet_transform));
}

void
attachStyleSheetStaticInterface(as_object& /*o*/)
{
}

as_object*
getStyleSheetInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachStyleSheetInterface(*o);
    }
    return o.get();
}

as_value
stylesheet_getStyle(const fn_call& fn)
{
    boost::intrusive_ptr<StyleSheet_as> ptr =
        ensureType<StyleSheet_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stylesheet_parseCSS(const fn_call& fn)
{
    boost::intrusive_ptr<StyleSheet_as> ptr =
        ensureType<StyleSheet_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stylesheet_setStyle(const fn_call& fn)
{
    boost::intrusive_ptr<StyleSheet_as> ptr =
        ensureType<StyleSheet_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stylesheet_transform(const fn_call& fn)
{
    boost::intrusive_ptr<StyleSheet_as> ptr =
        ensureType<StyleSheet_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stylesheet_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new StyleSheet_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

