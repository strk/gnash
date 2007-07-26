// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

/* $Id: aqua.cpp,v 1.24 2007/07/26 07:27:29 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"{
#include <unistd.h>
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
//	extern int getopt(int, char *const *, const char *);
}

#include "gnash.h"
#include "gui.h"
#include "aquasup.h"
#include "log.h"
#include "movie_root.h"

#include "render_handler.h"

#include <Carbon/Carbon.h>

WindowRef	myWindow;

pascal OSStatus DoWindowClose (EventHandlerCallRef  nextHandler,
                               EventRef             theEvent,
                               void*                userData){
	QuitApplicationEventLoop();
}

namespace gnash {
	
AquaGui::AquaGui(unsigned long xid, float scale, bool loop, unsigned int depth)
	: Gui(xid, scale, loop, depth)
{
}

AquaGui::~AquaGui()
{
	
}

void AquaGui::setInterval(unsigned int interval)
{
    _interval = interval;
}

bool AquaGui::run()
{
  	GNASH_REPORT_FUNCTION;
  	
	RepositionWindow(myWindow, NULL, kWindowCenterOnMainScreen);
    ShowWindow(myWindow);
    RunApplicationEventLoop();
    return true;
}

void AquaGui::renderBuffer()
{
    GNASH_REPORT_FUNCTION;
    _glue.render();
}

void
AquaGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
	GNASH_REPORT_FUNCTION;
#if 0	
    _glue.setInvalidatedRegions(ranges);
#endif
}

bool AquaGui::init(int argc, char ***argv) /* Self-explainatory */
{
  
	GNASH_REPORT_FUNCTION;
	InitCursor();
#if 0                    
  	_glue.init(argc, argv);

    _renderer = _glue.createRenderHandler();
    if(!_renderer)return false;
#endif    
    return true;

}

void AquaGui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
}

void AquaGui::key_event(int key, bool down)
{
}

void AquaGui::setCursor(gnash_cursor_type newcursor)
{
	  GNASH_REPORT_FUNCTION;
#if 0	  
	  switch(newcursor) {
	  	case gnash::CURSOR_HAND:	  		
			setCursor();
        	break;
      	case gnash::CURSOR_INPUT:
      		setCursor()
        	break;
      	default:
      	setCursor();
        }
#endif        
}


bool AquaGui::createWindow(const char* title, int width, int height)
{
	CFStringRef	windowTitle;
	OSStatus	result;
	Rect		theBounds;
	EventTypeSpec     eventType;                 // Specifier for event type
	EventHandlerUPP   handlerUPP;                // Pointer to event handler routine
 
	GNASH_REPORT_FUNCTION;

	SetRect(&theBounds, 0, 0, width, height);
	CreateNewWindow (kDocumentWindowClass,
                    	 kWindowStandardDocumentAttributes 
                       | kWindowStandardHandlerAttribute
                       | kWindowMetalAttribute
                       | kWindowCompositingAttribute
                       | kWindowInWindowMenuAttribute,
                    	&theBounds,
                    	&myWindow);
                    	
	windowTitle = CFStringCreateWithCString(NULL, title, NULL);
	result = SetWindowTitleWithCFString(myWindow, windowTitle);
	CFRelease (windowTitle);                    	

	createMenu();
	
	eventType.eventClass = kEventClassWindow;          // Set event class
	eventType.eventKind  = kEventWindowClose;          // Set event kind
	handlerUPP = NewEventHandlerUPP(DoWindowClose);    // Point to handler
	InstallWindowEventHandler (myWindow, handlerUPP,  // Install handler
                                 1, &eventType,
                                 NULL, NULL);
	_glue.prepDrawingArea(_width, _height);
    set_render_handler(_renderer);
    return true;
}

bool AquaGui::createMenu()
{ 	
	MenuRef	specialmenuRef;

	GNASH_REPORT_FUNCTION;
	
	CreateNewMenu(202, 0, &specialmenuRef); // 202 is an arbitrary ID
	SetMenuTitleWithCFString(specialmenuRef, CFSTR("Special"));	
	InsertMenu(specialmenuRef, 0);
	
	return true;
}

bool AquaGui::setupEvents()
{	
	GNASH_REPORT_FUNCTION;

	return true;
}

}