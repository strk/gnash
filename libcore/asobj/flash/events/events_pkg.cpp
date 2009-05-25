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

#include "ActivityEvent_as.h"
#include "AsyncErrorEvent_as.h"
#include "BrowserInvokeEvent_as.h"
#include "ContextMenuEvent_as.h"
#include "DRMAuthenticateEvent_as.h"
#include "DRMErrorEvent_as.h"
#include "DRMStatusEvent_as.h"
#include "DataEvent_as.h"
#include "ErrorEvent_as.h"
#include "EventDispatcher_as.h"
#include "EventPhase_as.h"
#include "Event_as.h"
#include "FileListEvent_as.h"
#include "FocusEvent_as.h"
#include "FullScreenEvent_as.h"
#include "HTMLUncaughtScriptExceptionEvent_as.h"
#include "HTTPStatusEvent_as.h"
#include "IEventDispatcher_as.h"
#include "IMEEvent_as.h"
#include "IOErrorEvent_as.h"
#include "InvokeEvent_as.h"
#include "KeyboardEvent_as.h"
#include "MouseEvent_as.h"
#include "NativeDragEvent_as.h"
#include "NativeWindowBoundsEvent_as.h"
#include "NativeWindowDisplayStateEvent_as.h"
#include "NetStatusEvent_as.h"
#include "OutputProgressEvent_as.h"
#include "ProgressEvent_as.h"
#include "SQLErrorEvent_as.h"
#include "SQLEvent_as.h"
#include "SQLUpdateEvent_as.h"
#include "ScreenMouseEvent_as.h"
#include "SecurityErrorEvent_as.h"
#include "StatusEvent_as.h"
#include "SyncEvent_as.h"
#include "TextEvent_as.h"
#include "TimerEvent_as.h"

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
