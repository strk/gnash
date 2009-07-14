// Font_as.cpp:  ActionScript "Font" class, for Gnash.
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

#include "text/Font_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value font_hasGlyphs(const fn_call& fn);
    as_value font_registerFont(const fn_call& fn);
    as_value font_ctor(const fn_call& fn);
    void attachFontInterface(as_object& o);
    void attachFontStaticInterface(as_object& o);
    as_object* getFontInterface();

}

class Font_as : public as_object
{

public:

    Font_as()
        :
        as_object(getFontInterface())
    {}
};

// extern (used by Global.cpp)
void font_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&font_ctor, getFontInterface());
        attachFontStaticInterface(*cl);
    }

    // Register _global.Font
    global.init_member("Font", cl.get());
}

namespace {

void
attachFontInterface(as_object& o)
{
    o.init_member("hasGlyphs", gl->createFunction(font_hasGlyphs));
    o.init_member("registerFont", gl->createFunction(font_registerFont));
}

void
attachFontStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);

}

as_object*
getFontInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachFontInterface(*o);
    }
    return o.get();
}

as_value
font_hasGlyphs(const fn_call& fn)
{
    boost::intrusive_ptr<Font_as> ptr =
        ensureType<Font_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
font_registerFont(const fn_call& fn)
{
    boost::intrusive_ptr<Font_as> ptr =
        ensureType<Font_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
font_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Font_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

