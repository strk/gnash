// desktop_pkg.cpp:  ActionScript "flash.desktop" package, for Gnash.
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

#include "ActivityEvent_as3.h"
#include "AsyncErrorEvent_as3.h"
#include "BrowserInvokeEvent_as3.h"
#include "ContextMenuEvent_as3.h"
#include "DRMAuthenticateEvent_as3.h"
#include "DRMErrorEvent_as3.h"
#include "DRMStatusEvent_as3.h"
#include "DataEvent_as3.h"
#include "ErrorEvent_as3.h"
#include "EventDispatcher_as3.h"
#include "EventPhase_as3.h"
#include "Event_as3.h"
#include "FileListEvent_as3.h"
#include "FocusEvent_as3.h"
#include "FullScreenEvent_as3.h"
#include "HTMLUncaughtScriptExceptionEvent_as3.h"
#include "HTTPStatusEvent_as3.h"
#include "IEventDispatcher_as3.h"
#include "IMEEvent_as3.h"
#include "IOErrorEvent_as3.h"
#include "InvokeEvent_as3.h"
#include "KeyboardEvent_as3.h"
#include "MouseEvent_as3.h"
#include "NativeDragEvent_as3.h"
#include "NativeWindowBoundsEvent_as3.h"
#include "NativeWindowDisplayStateEvent_as3.h"
#include "NetStatusEvent_as3.h"
#include "OutputProgressEvent_as3.h"
#include "ProgressEvent_as3.h"
#include "SQLErrorEvent_as3.h"
#include "SQLEvent_as3.h"
#include "SQLUpdateEvent_as3.h"
#include "ScreenMouseEvent_as3.h"
#include "SecurityErrorEvent_as3.h"
#include "StatusEvent_as3.h"
#include "SyncEvent_as3.h"
#include "TextEvent_as3.h"
#include "TimerEvent_as3.h"

#include "events_pkg.h"
#include "eventsclasses.h"

namespace gnash {

static as_value
get_flash_events_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.events package");
	as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;
	do {
	    asclasses[i](*pkg);
	} while (asclasses[++i] != 0);

	return pkg;
}

void
flash_events_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("events"), get_flash_events_package);
}


} // end of gnash namespace
