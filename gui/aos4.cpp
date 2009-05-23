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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdio>
#include <proto/keymap.h>
#include <libraries/keymap.h>

#include "gnash.h"
#include "log.h"
#include "aos4sup.h"

#include <getopt.h>


// Use 3MB of stack.. just to be safe
__attribute__ ((used)) static const char *stackcookie = "$STACK: 3000000";
__attribute__ ((used)) static const char *version     = "$VER: Gnash 0.8.5 for AmigaOS4 (" __DATE__ ")";

using namespace std;

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{
	void throw_exception(std::exception const &e)
	{
		gnash::log_error (_("Exception: %s"), e.what() );
		//std::abort();
	}
}
#endif

namespace gnash
{

AOS4Gui::AOS4Gui(unsigned long xid, float scale, bool loop, unsigned int depth)
 : Gui(xid, scale, loop, depth),
   _timeout(0),
   _core_trap(true)
{
	if (TimerInit() == FALSE) throw std::runtime_error("cannot initialize timer device");
}

AOS4Gui::~AOS4Gui()
{
	TimerExit();
}


void
AOS4Gui::TimerExit(void)
{
	if (_port)
	{
		IExec->FreeSysObject(ASOT_PORT, _port);
		_port = 0;
	}

	if (_timerio)
	{
		if (!IExec->CheckIO((struct IORequest *)_timerio))
		{
			IExec->AbortIO((struct IORequest *)_timerio);
			IExec->WaitIO((struct IORequest *)_timerio);
		}
	}

	if (_timerio && _timerio->Request.io_Device)
	{
		IExec->CloseDevice((struct IORequest *)_timerio);
		IExec->FreeSysObject(ASOT_IOREQUEST, _timerio);
		_timerio = 0;
	}

	if (ITimer)
	{
		IExec->DropInterface((struct Interface *)ITimer);
		ITimer = 0;
	}
}

bool
AOS4Gui::TimerInit(void)
{
	_port = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	if (!_port) return FALSE;

	_timerSig = 1L << _port->mp_SigBit;

	_timerio = (struct TimeRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
		ASOIOR_Size,		sizeof(struct TimeRequest),
		ASOIOR_ReplyPort,	_port,
	TAG_DONE);

	if (!_timerio) return FALSE;

	if (!IExec->OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)
		_timerio, 0))
	{
		ITimer = (struct TimerIFace *)IExec->GetInterface((struct Library *)
			_timerio->Request.io_Device, "main", 1, NULL);
		if (ITimer) return TRUE;
	}

	return FALSE;
}

void
AOS4Gui::TimerReset(uint32 microDelay)
{
	_timerio->Request.io_Command = TR_ADDREQUEST;
	_timerio->Time.Seconds = 0;
	_timerio->Time.Microseconds = microDelay;
	IExec->SendIO((struct IORequest *)_timerio);
}



bool
AOS4Gui::run()
{
    GNASH_REPORT_FUNCTION;
    int x_old = -1;
    int y_old = -1;
    int button_state_old = -1;
	struct IntuiMessage *imsg = NULL;
	uint32 sigMask;
    uint32_t movie_time = 0;
	int mod = 0;
	key::code code;
	uint32 new_width = _width, new_height = _height;

	TimerReset(10 * 100);

    while (true)
    {
		struct Window *_window = _glue.getWindow();
		if (!_window)
			return false;
			
	    if (_timeout && OS4_GetTicks() >= _timeout)
    	{
        	break;
	    }

		sigMask = SIGBREAKF_CTRL_C | (1L << _window->UserPort->mp_SigBit);
		sigMask |= _timerSig;

		uint32 sigGot = IExec->Wait(sigMask);

		if (sigGot & _timerSig)
    	{
	        // Wait until real time catches up with movie time.

			int delay = movie_time - OS4_GetTicks();
			if (delay > 0)
			{
				IDOS->Delay(delay/10);
			}

			Gui::advance_movie(this);
			movie_time += _interval;        // Time next frame should be displayed
			TimerReset(10 * 100);
	    }
	    else if (sigGot & SIGBREAKF_CTRL_C)
	    {
    	    return true;
		}
		else
		if (sigGot)
  		{
			while ( (imsg = (struct IntuiMessage *)IExec->GetMsg(_window->UserPort) ) )
			{
				switch(imsg->Class)
				{
					case IDCMP_CLOSEWINDOW:
						if (imsg->IDCMPWindow == _window) IExec->ReplyMsg ((struct Message *)imsg);
        	    	    return true;
					break;
					case IDCMP_MOUSEMOVE:
						if (imsg->IDCMPWindow == _window)
						{
							IExec->ReplyMsg ((struct Message *)imsg);
    	   	        		if ( _window->GZZMouseX == x_old && _window->GZZMouseY == y_old) { break; }
							x_old = _window->GZZMouseX;
							y_old = _window->GZZMouseY;
   		        	    	notify_mouse_moved(x_old, y_old);
   		        	    }
		            break;
		            case IDCMP_MOUSEBUTTONS:
						if (imsg->IDCMPWindow == _window)
						{
							IExec->ReplyMsg ((struct Message *)imsg);
							switch (imsg->Code)
   							{
							    case SELECTDOWN:
		        	        	    if (imsg->Code == button_state_old) { break; }
   	    		    	        	notify_mouse_clicked(true, 1);
	                			    button_state_old = imsg->Code;
					    		break;
							    case SELECTUP:
		    	    	            notify_mouse_clicked(false, 1);
       					            button_state_old = -1;
							    break;
							}
						}
					break;
					case IDCMP_RAWKEY:
						if (imsg->IDCMPWindow == _window) 
						{
							IExec->ReplyMsg ((struct Message *)imsg);
							if ( (imsg->Code  & ~IECODE_UP_PREFIX) == RAWKEY_ESC) //ESC
    	    		    	    return true;
							code = os4_to_gnash_key(imsg);
					    	mod  = os4_to_gnash_modifier(imsg->Qualifier);
						   	key_event(code, mod, true);
						}
	    			break;
					case IDCMP_SIZEVERIFY:
						if (imsg->IDCMPWindow == _window) IExec->ReplyMsg ((struct Message *)imsg);
						IGraphics->WaitBlit();
					break;
					case IDCMP_NEWSIZE:
						if (imsg->IDCMPWindow == _window)
						{
							IExec->ReplyMsg ((struct Message *)imsg);
							if (_window)
							{
								IIntuition->GetWindowAttr(_window, WA_InnerWidth,  &new_width,  sizeof(new_width));
								IIntuition->GetWindowAttr(_window, WA_InnerHeight, &new_height, sizeof(new_height));

			        	        _glue.resize(new_width, new_height);
								resize_view(new_width, new_height);
							}
						}
					break;
				}
			}
		}
	}

    return false;
}


void
AOS4Gui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
}

bool
AOS4Gui::init(int argc, char **argv[])
{
    GNASH_REPORT_FUNCTION;

	struct ExecBase *sysbase = (struct ExecBase*) IExec->Data.LibBase;

	struct Library *timer = (struct Library *)IExec->FindName(&sysbase->DeviceList, "timer.device");
	ITimer = (struct TimerIFace *)IExec->GetInterface(timer, "main", 1, 0);

	/* Set up reference time. */
	ITimer->GetSysTime(&os4timer_starttime);

    int c;
    while ((c = getopt (argc, *argv, "m:c")) != -1) {
        switch (c) {
        case 'c':
            disableCoreTrap();
        }
    }

    _glue.init(argc, argv);

    _renderer = _glue.createRenderHandler(_depth);
    if ( ! _renderer )
	{
		log_error (_("error creating RenderHandler!\n"));
    	return false;
	}

    return true;
}


bool
AOS4Gui::createWindow(const char *title, int width, int height)
{
    GNASH_REPORT_FUNCTION;
    _width = width;
    _height = height;

    //_glue.saveOrigiginalDimension(width,height);
	_orig_width  = width;
	_orig_height = height;

    _glue.prepDrawingArea(_width, _height);

    set_render_handler(_renderer);

	struct Window *_window = _glue.getWindow();

	_window_title = (char*)malloc(strlen(title)+1);
	memset(_window_title,0x0,strlen(title)+1);
	strcpy(_window_title,title);
	
	IIntuition->SetWindowTitles(_window,title,title);

    return true;
}

void
AOS4Gui::disableCoreTrap()
{
    _core_trap = false;
}

void
AOS4Gui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _glue.setInvalidatedRegions(ranges);
}

void
AOS4Gui::renderBuffer()
{
    //GNASH_REPORT_FUNCTION;

    _glue.render();
}

void
AOS4Gui::setInterval(unsigned int interval)
{
    _interval = interval;
}

bool
AOS4Gui::createMenu()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
AOS4Gui::setupEvents()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

void
AOS4Gui::setFullscreen()
{
    if (_fullscreen) return;

	_glue.setFullscreen();
	struct Window *_window = _glue.getWindow();
	resize_view(_window->Width, _window->Height);

    _fullscreen = true;
}

void
AOS4Gui::unsetFullscreen()
{
    if (!_fullscreen) return;

	_glue.unsetFullscreen();
	struct Window *_window = _glue.getWindow();
	resize_view(_window->Width, _window->Height);
	IIntuition->SetWindowTitles(_window,_window_title,_window_title);

    _fullscreen = false;
}

bool 
AOS4Gui::showMouse(bool show)
{
    bool state = _mouseShown;
	struct Window *_window = _glue.getWindow();

    if (show == _mouseShown) return state;

    if (!show)
    {
		UWORD *EmptyPointer = NULL;

		EmptyPointer = (UWORD *)IExec->AllocVec(16, MEMF_PUBLIC | MEMF_CLEAR);
		IIntuition->SetPointer(_window, EmptyPointer, 1, 16, 0, 0);
		_mouseShown = false;
	}
	else
    {
		_mouseShown = true;
		IIntuition->ClearPointer(_window);
	}

    return state;
	
}

key::code
AOS4Gui::os4_to_gnash_key(struct IntuiMessage *imsg)
{
	unsigned char code[10];
	struct InputEvent ie;
	int actual = 0;
	gnash::key::code c(gnash::key::INVALID);

	// TODO: take care of CAPS_LOCK and NUM_LOCK and SHIFT
	// int mod = key->keysym.mod;
	//int code = key & ~IECODE_UP_PREFIX;

	switch(imsg->Code & ~IECODE_UP_PREFIX)
	{
		case RAWKEY_CRSRUP:		c = gnash::key::UP;       	break;
  		case RAWKEY_CRSRDOWN:	c = gnash::key::DOWN;     	break;
   		case RAWKEY_CRSRRIGHT:	c = gnash::key::RIGHT;    	break;
   		case RAWKEY_CRSRLEFT:	c = gnash::key::LEFT;     	break;
   		case RAWKEY_INSERT:		c = gnash::key::INSERT;   	break;
   		case RAWKEY_HOME:		c = gnash::key::HOME;     	break;
   		case RAWKEY_END:		c = gnash::key::END;      	break;
   		case RAWKEY_PAGEUP:		c = gnash::key::PGUP;     	break;
   		case RAWKEY_PAGEDOWN:	c = gnash::key::PGDN;     	break;
   		case RAWKEY_LSHIFT:
   		case RAWKEY_RSHIFT:		c = gnash::key::SHIFT;    	break;
   		case RAWKEY_LCTRL:		c = gnash::key::CONTROL;  	break;
   		case RAWKEY_LALT:
   		case RAWKEY_RALT:		c = gnash::key::ALT;      	break;
		case RAWKEY_F1: 		c = gnash::key::F1; 		break;
		case RAWKEY_F2: 		c = gnash::key::F2; 		break;
		case RAWKEY_F3: 		c = gnash::key::F3; 		break;
		case RAWKEY_F4: 		c = gnash::key::F4; 		break;
		case RAWKEY_F5: 		c = gnash::key::F5; 		break;
		case RAWKEY_F6: 		c = gnash::key::F6; 		break;
		case RAWKEY_F7: 		c = gnash::key::F7; 		break;
		case RAWKEY_F8: 		c = gnash::key::F8; 		break;
		case RAWKEY_F9: 		c = gnash::key::F9; 		break;
		case RAWKEY_F10: 		c = gnash::key::F10; 		break;
		case RAWKEY_F11: 		c = gnash::key::F11; 		break;
		case RAWKEY_F12: 		c = gnash::key::F12; 		break;

   		default: 				c = gnash::key::INVALID; 	break;
	}

	if (c == gnash::key::INVALID)
	{
		ie.ie_NextEvent    = NULL;
		ie.ie_Class 	   = IECLASS_RAWKEY;
		ie.ie_SubClass 	   = 0;
		ie.ie_Code         = imsg->Code;
		ie.ie_Qualifier    = imsg->Qualifier;
		/* recover dead key codes & qualifiers */
		ie.ie_EventAddress=(APTR *) *((ULONG *)imsg->IAddress);
		actual = IKeymap->MapRawKey(&ie, (STRPTR)code, 10, NULL);
		if (actual == 1)
		{
			c = (gnash::key::code)(int)code[0];
		}
	}
	return c;
}

int
AOS4Gui::os4_to_gnash_modifier(int state)
{
  int modifier = gnash::key::GNASH_MOD_NONE;

  if (state & IEQUALIFIER_LSHIFT) {
      modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
    }
    if (state & IEQUALIFIER_CONTROL) {
      modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
    }
    if ((state & IEQUALIFIER_LALT) || (state & IEQUALIFIER_RALT)) {
      modifier = modifier | gnash::key::GNASH_MOD_ALT;
    }

    return modifier;
}

void
AOS4Gui::key_event(gnash::key::code key, int state, bool down)
{
    if (key != gnash::key::INVALID)
    {
        // 0 should be any modifier instead..
        // see Gui::notify_key_event in gui.h
        notify_key_event(key, state, down);
    }
}

double
AOS4Gui::OS4_GetTicks()
{
	struct TimeVal cur;

	ITimer->GetSysTime(&cur);
	ITimer->SubTime(&cur, &os4timer_starttime);

	return cur.Seconds * 1000 + cur.Microseconds / 1000;
}

} // namespace gnash

