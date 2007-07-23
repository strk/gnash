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

/* $Id: aqua.cpp,v 1.15 2007/07/23 01:01:31 nihilus Exp $ */

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

struct GlobalAppInfo  
{
  CFBundleRef		mainBundle;
  IBNibRef			mainNib;
  WindowGroupRef	windowGroups[3];
};
typedef struct GlobalAppInfo GlobalAppInfo;

GlobalAppInfo  g;

static pascal OSStatus AppEventHandlerProc(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData)
{
  #pragma unused (inCallRef)
  HICommand        		command;
  WindowGroupRef		windowGroup;
  WindowGroupAttributes windowGroupAttributes;
  UInt32	eventKind 	= GetEventKind(inEvent);
  UInt32	eventClass	= GetEventClass(inEvent);
  WindowRef	window 		= (WindowRef) inUserData;
  OSStatus	err 		= eventNotHandledErr;

  GNASH_REPORT_FUNCTION;
  return err;	
}
static pascal OSStatus SimpleWindowEventHandlerProc(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData);

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
	OSErr 						err;
  	static const EventTypeSpec	sApplicationEvents[] =  {  {kEventClassCommand, kEventCommandProcess}  };
  
	GNASH_REPORT_FUNCTION;
	BlockZero(&g, sizeof(g));
    
  	g.mainBundle = CFBundleGetMainBundle();
  	if (g.mainBundle == NULL) 
  	{
  		err = false;
  	 	goto Bail;
  	}
  
	err = CreateNibReferenceWithCFBundle(g.mainBundle, CFSTR("aqua"), &g.mainNib);
	if (err != noErr)goto Bail;
	if (g.mainNib == NULL)
	{
  		err = false;
  		goto Bail;
  	}
  		
  	err = SetMenuBarFromNib(g.mainNib, CFSTR("aqua"));
  	if (err != noErr)goto Bail;

  	InstallApplicationEventHandler(NewEventHandlerUPP(AppEventHandlerProc), GetEventTypeCount(sApplicationEvents), sApplicationEvents, 0, NULL);

  	//  Force the document group to be created first, so we can position our groups between the floating and document groups
  	(void) GetWindowGroupOfClass(kDocumentWindowClass);
  
  	//  Create our default WindowGroups and set their z-order
  	err = CreateWindowGroup(0, &g.windowGroups[0]);
  	err = CreateWindowGroup(0, &g.windowGroups[1]);
  	err = CreateWindowGroup(0, &g.windowGroups[2]);

  	//  Position our groups behind the floating group and in front of the document group
  	SendWindowGroupBehind(g.windowGroups[2], GetWindowGroupOfClass(kDocumentWindowClass));
  	SendWindowGroupBehind(g.windowGroups[1], g.windowGroups[2]);
  	SendWindowGroupBehind(g.windowGroups[0], g.windowGroups[1]);
    
	RunApplicationEventLoop();
    return true;

Bail:
	return err;    
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
    _glue.setInvalidatedRegions(ranges);
}

bool AquaGui::init(int argc, char ***argv) /* Self-explainatory */
{
  GNASH_REPORT_FUNCTION;
  
  	_glue.init (argc, argv);

    _renderer = _glue.createRenderHandler();
    if(!_renderer)return false;    
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