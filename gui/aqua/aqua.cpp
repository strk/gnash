//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

/* $Id: aqua.cpp,v 1.33 2008/05/26 22:13:33 ann Exp $ */

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

extern "C"{
# ifdef HAVE_GETOPT_H
#  include <getopt.h>
# endif
# ifndef __GNUC__
  extern int getopt(int, char *const *, const char *);
# endif
}

#include "aquasup.h"
#include "log.h"
#include "movie_root.h"
#include "RunResources.h"

#include "Renderer.h"

#include <Carbon/Carbon.h>

using namespace std;

namespace gnash {

/* Main window widget */
WindowRef myWindow = NULL;

pascal OSStatus DoWindowClose (EventHandlerCallRef  nextHandler,
                               EventRef             theEvent,
                               void*                userData)
{
	QuitApplicationEventLoop();
        return noErr;
}

void DoAdvanceMovie ( EventLoopTimerRef inTimer, void* data)
{
   AquaGui* gui = static_cast<AquaGui*>(data);
   Gui::advance_movie(gui);
}
  
	
AquaGui::AquaGui(unsigned long xid, float scale, bool loop, RunResources& r)
	: Gui(xid, scale, loop, r),
          _advance_timer(NULL)
{
}

AquaGui::~AquaGui()
{
    if (_advance_timer) {
      RemoveEventLoopTimer(*_advance_timer);
    }
}


bool AquaGui::run()
{
    double interval = _interval / 1000.0;


    OSStatus ret = InstallEventLoopTimer (GetMainEventLoop(), 0, interval, 
      DoAdvanceMovie, this, _advance_timer);
    if (ret != noErr) {
      return false;
    }

    RepositionWindow(myWindow, NULL, kWindowCascadeOnMainScreen);
    SelectWindow(myWindow);
    ShowWindow(myWindow);
	SetWindowModified(myWindow, false);
    RunApplicationEventLoop();
    return true;
}

void AquaGui::renderBuffer()
{
    _glue.render();

      Rect rectPort;
      GetWindowPortBounds (myWindow, &rectPort);
      InvalWindowRect (myWindow,  &rectPort); // force redrow
}

bool AquaGui::init(int argc, char **argv[]) /* Self-explainatory */
{

	OSErr err;
	long response;
	Str255 text = " OS X version lower than 10.4 is not supported!", tmp = "";
  
	/* Version check */
	err = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((err == noErr) && (response >= 0x00001040));
	
   	if (!ok)
      {
      StandardAlert(kAlertStopAlert, text, tmp, NULL, NULL);
      ExitToShell();
      }
      	
  	_glue.init(argc, argv);

    _renderer.reset(_glue.createRenderHandler());
    if(!_renderer)return false;

	_runResources.setRenderer(boost::shared_ptr<Renderer>(_renderer));
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

	  switch(newcursor) {
	  	case gnash::CURSOR_HAND:	  		
				SetThemeCursor(kThemePointingHandCursor);
        	break;
      	case gnash::CURSOR_INPUT:
      			SetThemeCursor(kThemeCrossCursor);
      		break;
      	default:
      	        SetThemeCursor(kThemeArrowCursor);
        }        
}


bool AquaGui::createWindow(const char* title, int width, int height,
                           int xPosition, int yPosition)
{
	CFStringRef	windowTitle = NULL;
	OSStatus	result;
	Rect		theBounds = {0, 0, 0, 0};

	EventTypeSpec     eventType;                 // Specifier for event type
	EventHandlerUPP   handlerUPP;                // Pointer to event handler routine
 
        _width = width;
        _height = height;

	SetRect(&theBounds, 0, 0, width, height);
	OSStatus status = CreateNewWindow ( kDocumentWindowClass,
                    	 kWindowStandardDocumentAttributes 
                       | kWindowStandardHandlerAttribute,
                    	&theBounds,
                    	&myWindow);

	windowTitle = CFStringCreateWithCString(NULL, title, NULL);
	result = SetWindowTitleWithCFString(myWindow, windowTitle);
	if(windowTitle != NULL)CFRelease(windowTitle);

	createMenu();
	
	eventType.eventClass = kEventClassWindow;          // Set event class
	eventType.eventKind  = kEventWindowClose;          // Set event kind
	handlerUPP = NewEventHandlerUPP(DoWindowClose);    // Point to handler
	InstallWindowEventHandler (myWindow, handlerUPP,  // Install handler
                                 1, &eventType,
                                 NULL, NULL);
 	assert(_glue.prepDrawingArea(_width, _height, GetWindowPort(myWindow)));

    return true;
}

bool AquaGui::createMenu()
{ 	
	MenuRef rApplicationMenu;
	MenuItemIndex outIndex[1];	  
	
	
	/* Enable 'Prefereces...' */
	EnableMenuCommand(NULL, kHICommandPreferences);

	GetIndMenuItemWithCommandID(NULL, kHICommandPreferences, 1, &rApplicationMenu, outIndex);

	/* Enable 'About' */
	InsertMenuItemTextWithCFString(rApplicationMenu, CFSTR("About Gnash"), (short) 0, 0, kHICommandAbout);

	return true;
}

bool AquaGui::setupEvents()
{	

	return true;
}

} // namespace gnash

