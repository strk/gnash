// net_pkg.cpp:  ActionScript "flash.net" package, for Gnash.
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

#include "Object.h" // for getObjectInterface
#include "as_object.h"
#include "string_table.h"
#include "VM.h"
#include "MovieClip.h"

#include "FileFilter_as.h"
#include "FileReferenceList_as.h"
#include "FileReference_as.h"
#include "IDynamicPropertyOutput_as.h"
#include "IDynamicPropertyWriter_as.h"
#include "LocalConnection_as.h"
#include "NetConnection_as.h"
#include "NetStream_as.h"
#include "ObjectEncoding_as.h"
#include "Responder_as.h"
#include "SharedObjectFlushStatus_as.h"
#include "SharedObject_as.h"
#include "Socket_as.h"
#include "URLLoaderDataFormat_as.h"
#include "URLLoader_as.h"
#include "URLRequestHeader_as.h"
#include "URLRequestMethod_as.h"
#include "URLRequest_as.h"
#include "URLStream_as.h"
#include "URLVariables_as.h"
#include "XMLSocket_as.h"

#include "net_pkg.h"
#include "netclasses.h"

namespace gnash {

static as_value
get_flash_net_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.net package");
	as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;
	do {
	    asclasses[i](*pkg);
	} while (asclasses[++i] != 0);

	return pkg;
}

void
flash_net_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();

    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
	where.init_destructive_property(st.find("net"),
			get_flash_net_package, flags);
}


} // end of gnash namespace
