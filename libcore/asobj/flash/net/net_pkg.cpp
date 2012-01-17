// net_pkg.cpp:  ActionScript "flash.net" package, for Gnash.
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

#include "as_object.h"
#include "VM.h"
#include "VM.h"
#include "fn_call.h"
#include "namedStrings.h"
#include "FileReferenceList_as.h"
#include "FileReference_as.h"
#include "net_pkg.h"
#include "Global_as.h"

namespace gnash {

static as_value
get_flash_net_package(const fn_call& fn)
{
    log_debug("Loading flash.net package");
 
    Global_as& gl = getGlobal(fn);

    as_object* pkg = createObject(gl);
    
    VM& vm = getVM(fn);

    filereference_class_init(*pkg, getURI(vm, "FileReference"));

    return pkg;
}

void
flash_net_package_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_net_package, flags);
}


} // end of gnash namespace
