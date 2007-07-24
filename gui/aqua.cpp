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

/* $Id: aqua.cpp,v 1.19 2007/07/24 13:04:42 nihilus Exp $ */

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

ToolboxObjectClassRef customWindow;
WindowRef myWindow;
WindowDefSpec myCustomWindowSpec;
EventHandlerUPP myCustomWindowUPP;
Rect theBounds = {200,200,400,400};
 
EventTypeSpec eventList[] = {{kEventClassWindow, kEventWindowDrawFrame},
                            {kEventClassWindow, kEventWindowHitTest}};

static pascal OSStatus MyCustomWindowEventHandler (
                                EventHandlerCallRef myHandler,
                                EventRef theEvent, void* userData)
{
 
    #pragma unused (myHandler,userData)
 
    OSStatus result = eventNotHandledErr;
 
    UInt32 whatHappened;
    WindowDefPartCode where;
 
    GrafPtr thePort;
    Rect windBounds;
 
    whatHappened = GetEventKind (theEvent);
 
    switch (whatHappened)
    {
        case kEventWindowInit:
 
            GetEventParameter (theEvent, kEventParamDirectObject,
            					typeWindowRef, NULL, sizeof(WindowRef),
           						NULL, &myWindow);
           						
            SetThemeWindowBackground (myWindow, kThemeBrushMovableModalBackground, true);	// 1
            result = noErr;
            break;
 
        case kEventWindowDrawFrame:	// 2
 
            GetPort(&thePort);		// 3
            GetPortBounds(thePort, &windBounds);

            PenNormal();			// 4
            PenSize (10,10);
            FrameRect(&windBounds);	// 5
 
            result = noErr;
            break;
 
        case kEventWindowHitTest:	// 6
 
            /* determine what part of the window the user hit */
            where = wInDrag;
            SetEventParameter (theEvent, kEventParamWindowDefPart,	// 7
                                typeWindowDefPartCode,
                                sizeof(WindowDefPartCode), &where);
 
            result = noErr;
            break;
    }
 
    return (result);
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
 	
 	myCustomWindowUPP = NewEventHandlerUPP(MyCustomWindowEventHandler);
 
	RegisterToolboxObjectClass(CFSTR("com.myCompany.myApp.customWindow"),	// 2
								NULL, GetEventTypeCount(eventList), eventList,
                        		myCustomWindowUPP, NULL, &customWindow);
 
	myCustomWindowSpec.defType = kWindowDefObjectClass; // 3
	myCustomWindowSpec.u.classRef = customWindow; // 4
 
	CreateCustomWindow (&myCustomWindowSpec,kMovableModalWindowClass,	// 5
                    	kWindowStandardHandlerAttribute,
                    	&theBounds,
                    	&myWindow);
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
	GNASH_REPORT_FUNCTION;
	theBounds.top = height;
	theBounds.right = width;
	_glue.prepDrawingArea(_width, _height);
    set_render_handler(_renderer);
    return true;
}

bool AquaGui::createMenu()
{ 
	return true;
}

bool AquaGui::setupEvents()
{
	return true;
}

}