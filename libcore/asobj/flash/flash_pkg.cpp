// flash_pkg.cpp:  ActionScript top level "flash" package.
// 
//   Copyright (C)  2009 Free Software Foundation, Inc.
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
#include "Object.h" // for getObjectInterface
#include "flash_pkg.h"
#include "display/display_pkg.h"
#include "external/external_pkg.h"
#include "filters/filters_pkg.h"
#include "geom/geom_pkg.h"
#include "net/net_pkg.h"
#include "text/text_pkg.h"

namespace gnash {

class as_value;
class as_object;

static as_value
get_flash_package(const fn_call& fn)
{
    as_object *pkg = new as_object(getObjectInterface());
    
    string_table& st = getStringTable(fn);

    flash_text_package_init(*pkg,
            ObjectURI(st.find("text"), NSV::NS_FLASH));
    flash_display_package_init(*pkg,
            ObjectURI(st.find("display"), NSV::NS_FLASH));
    flash_filters_package_init(*pkg,
            ObjectURI(st.find("filters"), NSV::NS_FLASH));
    flash_geom_package_init(*pkg,
            ObjectURI(st.find("geom"), NSV::NS_FLASH));
    flash_net_package_init(*pkg,
            ObjectURI(st.find("net"), NSV::NS_FLASH));
    flash_external_package_init(*pkg,
            ObjectURI(st.find("external"), NSV::NS_FLASH));

    return pkg;
}

void
flash_package_init(as_object& where, const ObjectURI& uri)
{
    string_table& st = getStringTable(where);
    where.init_destructive_property(st.find("flash"), get_flash_package,
        PropFlags::dontEnum | PropFlags::onlySWF8Up);
}

}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
