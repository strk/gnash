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

#include "FileFilter_as3.h"
#include "FileReferenceList_as3.h"
#include "FileReference_as3.h"
#include "IDynamicPropertyOutput_as3.h"
#include "IDynamicPropertyWriter_as3.h"
#include "LocalConnection_as3.h"
#include "NetConnection_as3.h"
#include "NetStream_as3.h"
#include "ObjectEncoding_as3.h"
#include "Responder_as3.h"
#include "SharedObjectFlushStatus_as3.h"
#include "SharedObject_as3.h"
#include "Socket_as3.h"
#include "URLLoaderDataFormat_as3.h"
#include "URLLoader_as3.h"
#include "URLRequestDefaults_as3.h"
#include "URLRequestHeader_as3.h"
#include "URLRequestMethod_as3.h"
#include "URLRequest_as3.h"
#include "URLStream_as3.h"
#include "URLVariables_as3.h"
#include "XMLSocket_as3.h"

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
	where.init_destructive_property(st.find("net"), get_flash_net_package);
}


} // end of gnash namespace
