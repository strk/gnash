// TextRenderer_as.cpp:  ActionScript "TextRenderer" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#include "TextRenderer_as.h"

#include <sstream>

#include "as_object.h" // for inheritance
#include "as_function.h" 
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "VM.h" 

namespace gnash {

namespace {
    void attachTextRendererStaticProperties(as_object& o);
    as_value textrenderer_setAdvancedAntialiasingTable(const fn_call& fn);
    as_value textrenderer_maxLevel(const fn_call& fn);
    as_value textrenderer_ctor(const fn_call& fn);
}

// extern 
void
textrenderer_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, textrenderer_ctor, 0,
            attachTextRendererStaticProperties, uri);
}

namespace {

void
attachTextRendererStaticProperties(as_object& o)
{
   
    Global_as& gl = getGlobal(o);
    o.init_member("setAdvancedAntialiasingTable",
            gl.createFunction(textrenderer_setAdvancedAntialiasingTable));
    o.init_property("maxLevel", textrenderer_maxLevel, textrenderer_maxLevel);
}


as_value
textrenderer_setAdvancedAntialiasingTable(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
textrenderer_maxLevel(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}


as_value
textrenderer_ctor(const fn_call& /*fn*/)
{
    return as_value(); 
}

}

} // end of gnash namespace
