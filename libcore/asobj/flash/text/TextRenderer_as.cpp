// TextRenderer_as.cpp:  ActionScript "TextRenderer" class, for Gnash.
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

#include "TextRenderer_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {


static as_value TextRenderer_setAdvancedAntialiasingTable(const fn_call& fn);
static as_value TextRenderer_maxLevel_getset(const fn_call& fn);

as_value TextRenderer_ctor(const fn_call& fn);

static void
attachTextRendererInterface(as_object& /*o*/)
{
}

static void
attachTextRendererStaticProperties(as_object& o)
{
   
    Global_as* gl = getGlobal(o);
    o.init_member("setAdvancedAntialiasingTable", gl->createFunction(TextRenderer_setAdvancedAntialiasingTable));
    o.init_property("maxLevel", TextRenderer_maxLevel_getset, TextRenderer_maxLevel_getset);
}

static as_object*
getTextRendererInterface()
{
    static boost::intrusive_ptr<as_object> o;

    if ( ! o )
    {
        // TODO: check if this class should inherit from Object
        //       or from a different class
        o = new as_object(getObjectInterface());
        VM::get().addStatic(o.get());

        attachTextRendererInterface(*o);

    }

    return o.get();
}

class TextRenderer_as: public as_object
{

public:

    TextRenderer_as()
        :
        as_object(getTextRendererInterface())
    {}

    // override from as_object ?
    //std::string get_text_value() const { return "TextRenderer"; }

    // override from as_object ?
    //double get_numeric_value() const { return 0; }
};



static as_value
TextRenderer_setAdvancedAntialiasingTable(const fn_call& fn)
{
    boost::intrusive_ptr<TextRenderer_as> ptr = ensureType<TextRenderer_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

static as_value
TextRenderer_maxLevel_getset(const fn_call& fn)
{
    boost::intrusive_ptr<TextRenderer_as> ptr = ensureType<TextRenderer_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
TextRenderer_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new TextRenderer_as;

    if ( fn.nargs )
    {
        std::stringstream ss;
        fn.dump_args(ss);
        LOG_ONCE( log_unimpl("TextRenderer(%s): %s", ss.str(), _("arguments discarded")) );
    }

    return as_value(obj.get()); // will keep alive
}

// extern 
void textrenderer_class_init(as_object& where)
{
    // This is going to be the TextRenderer "class"/"function"
    // in the 'where' package
    boost::intrusive_ptr<as_object> cl;
    Global_as* gl = getGlobal(where);
    cl = gl->createClass(&TextRenderer_ctor, getTextRendererInterface());;
    attachTextRendererStaticProperties(*cl);

    // Register _global.TextRenderer
    where.init_member("TextRenderer", cl.get());
}

} // end of gnash namespace
