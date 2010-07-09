// flash_pkg.cpp:  ActionScript top level "flash" package.
// 
//   Copyright (C)  2009, 2010 Free Software Foundation, Inc.
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

#include "string_table.h"
#include "VM.h"
#include "fn_call.h"
#include "namedStrings.h"
#include "flash_pkg.h"
#include "display/display_pkg.h"
#include "external/external_pkg.h"
#include "filters/filters_pkg.h"
#include "geom/geom_pkg.h"
#include "net/net_pkg.h"
#include "text/text_pkg.h"
#include "Global_as.h"

namespace gnash {

class as_value;
class as_object;

static as_value
get_flash_package(const fn_call& fn)
{
    Global_as& gl = getGlobal(fn);

    as_object* pkg = gl.createObject();
    
    string_table& st = getStringTable(fn);

    flash_text_package_init(*pkg, st.find("text"));
    flash_display_package_init(*pkg, st.find("display"));
    flash_filters_package_init(*pkg, st.find("filters"));
    flash_geom_package_init(*pkg, st.find("geom"));
    flash_net_package_init(*pkg, st.find("net"));
    flash_external_package_init(*pkg, st.find("external"));

    return pkg;
}

void
flash_package_init(as_object& where, const ObjectURI& uri)
{
    const int flags = PropFlags::dontEnum | PropFlags::onlySWF8Up;
    where.init_destructive_property(uri, get_flash_package, flags);
}

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
