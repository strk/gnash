// TextFormatAlign_as.cpp:  ActionScript "TextFormatAlign" class, for Gnash.
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

#include "text/TextFormatAlign_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h"

/// TextFormatAlign is an AVM2-only class containing constants.

namespace gnash {

// Forward declarations
namespace {
    void attachTextFormatAlignStaticInterface(as_object& o);
}

// extern (used by Global.cpp)
void textformatalign_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> obj =
        new as_object(getObjectInterface());

    attachTextFormatAlignStaticInterface(*obj);

    // Register _global.TextFormatAlign
    where.init_member(getName(uri), obj.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachTextFormatAlignStaticInterface(as_object& o)
{
    o.init_member("CENTER", "center");
    o.init_member("JUSTIFY", "justify");
    o.init_member("LEFT", "left");
    o.init_member("RIGHT", "right");
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

