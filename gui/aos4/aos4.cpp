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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdio>
#include "rc.h"

#include <proto/asl.h>
#include <proto/keymap.h>
#include <proto/requester.h>

#include <libraries/keymap.h>

#include <classes/window.h>
#include <classes/requester.h>

#include "log.h"
#include "aos4sup.h"
#include "aos4_gnash_prefs.h"
#undef CUSTOM
#include "RunResources.h"

#include <getopt.h>
#include <signal.h>

#include "GnashSleep.h" // for gnashSleep


// Use 4MB of stack.. just to be safe
__attribute__ ((used)) static const char *stackcookie = "$STACK: 4000000";
__attribute__ ((used)) static const char *version     = "$VER: Gnash " VERSION " for AmigaOS4 (" __DATE__ ")";

extern struct NewMenu nm[];
extern Object *win;
extern struct MsgPort *AppPort;
extern Object *Objects[OBJ_NUM];
extern int audioTaskID;

#define GAD(x) (struct Gadget *)Objects[x]
#define OBJ(x) Objects[x]

#define RESET_TIME 30 * 1000 

using namespace std;

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{
	void throw_exception(std::exception const &e)
	{
		gnash::log_error (_("Exception: %s on file %s line %d"), e.what(), __FILE__, __LINE__ );
		//std::abort();
	}
}
#endif

namespace gnash
{

AOS4Gui::AOS4Gui(unsigned long xid, float scale, bool loop, RunResources& r)
 : Gui(xid, scale, loop, r),
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
	if (_timerio)
	{
		if (!IExec->CheckIO((struct IORequest *)_timerio))
		{
			IExec->AbortIO((struct IORequest *)_timerio);
			IExec->WaitIO((struct IORequest *)_timerio);
		}
	}

	if (_port)
	{
		IExec->FreeSysObject(ASOT_PORT, _port);
		_port = 0;
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

	if (!IExec->OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)
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

void
AOS4Gui::killAudioTask()
{
	if (audioTaskID) // This is a Global variable defined in sound_handler_ahi that point to the audioTask
	{
		IExec->Signal((struct Task*)audioTaskID,SIGBREAKF_CTRL_C);
		IDOS->Delay(50); //Wait to be sure that audio task has finished its work..
	}
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
    uint32 movie_time = OS4_GetTicks();
	int mod = 0;
	key::code code;
	uint32 new_width = _width, new_height = _height;
    ULONG menucode;
	struct FileRequester *AmigaOS_FileRequester = NULL;

	TimerReset(RESET_TIME);

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
			IExec->GetMsg(_port);

			int delay = movie_time - OS4_GetTicks();
			if (delay > 0)
			{
				gnashSleep(delay);
			}

			movie_time += _interval;        // Time next frame should be displayed
			Gui::advance_movie(this);
			TimerReset(RESET_TIME); 			// 20fps
	    }
		else if (sigGot)
  		{
			while ( (imsg = (struct IntuiMessage *)IExec->GetMsg(_window->UserPort) ) )
			{
				switch(imsg->Class)
				{
	                case IDCMP_MENUPICK:
						if (imsg->IDCMPWindow == _window) IExec->ReplyMsg ((struct Message *)imsg);
    	                menucode = imsg->Code;
        	            if (menucode != MENUNULL)
            	        {
                	        while (menucode != MENUNULL)
                    	    {
								struct Menu* _local_menu = _glue.getMenu();
								if (_local_menu)
								{
	                        	    struct MenuItem  *item = IIntuition->ItemAddress(_local_menu,menucode);

    	            	            switch (MENUNUM(menucode))
        	            	        {
            	            	        case 0:  /* First menu */
    	        	                        switch (ITEMNUM(menucode))
        	        	                    {
            	        	                    /* Other cases here */
												case 0:
													AmigaOS_FileRequester = (struct FileRequester *)IAsl->AllocAslRequest(ASL_FileRequest, NULL);
													if (AmigaOS_FileRequester)
													{
														char SWFFile[1024];
														BOOL result;

														result = IAsl->AslRequestTags( AmigaOS_FileRequester,
															ASLFR_TitleText,        "Choose a video",
															ASLFR_DoMultiSelect,    FALSE,
															ASLFR_RejectIcons,      TRUE,
															ASLFR_DoPatterns,		TRUE,
															ASLFR_InitialPattern,	(ULONG)"(#?.swf)",
												    		ASLFR_InitialDrawer,    "PROGDIR:",
															TAG_DONE);

														if (result == TRUE)
														{
															strcpy(SWFFile, AmigaOS_FileRequester->fr_Drawer);
															if (strlen(SWFFile) && !(SWFFile[strlen(SWFFile) - 1] == ':' || SWFFile[strlen(SWFFile) - 1] == '/'))
															    strcat (SWFFile, "/");
															strcat (SWFFile, AmigaOS_FileRequester->fr_File);

														    log_error (_("Attempting to open file %s.\n"
															               "NOTE: the file open functionality is not yet implemented!"),
																	       SWFFile);

														}
														IAsl->FreeAslRequest(AmigaOS_FileRequester);
													}
													else
														log_error (_("Cannot open File Requester!\n"));
												break;
    		                                    case 6:  /* Quit */
    		                                    	killAudioTask();
        		                                    return true;
            		                            break;
                	    	                }
                    		            break;
                    		            case 1:
    	        	                        switch (ITEMNUM(menucode))
        	        	                    {
        	        	                    	case 0:	/* Show Preferences */
													struct Window 		*PrefsWindow;
													struct GnashPrefs	*stored_prefs;

													if ((AppPort = IExec->CreateMsgPort()) )
													{
														stored_prefs = ReadPrefs();
														win = make_window(stored_prefs);

														if ((PrefsWindow = RA_OpenWindow(win)))
														{
															uint32
																sigmask     = 0,
																siggot      = 0,
																result      = 0;
															uint16
																code        = 0;
															BOOL
																done        = FALSE;
															ULONG iValue;
															ULONG sValue;
															std::string tmp;
															
														    RcInitFile& _rcfile = RcInitFile::getDefaultInstance();
															
															/* Due an undiscovered bug in my code (?) the string values must be set AFTER the window is created and opened */
															/* Don't ask me why.. */
															IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_LOGFILENAME_VALUE),	PrefsWindow, NULL, STRINGA_TextVal,stored_prefs->logfilename,	TAG_DONE);
															IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_SHAREDOBJDIR_VALUE),	PrefsWindow, NULL, STRINGA_TextVal,stored_prefs->sharedobjdir,	TAG_DONE);
															IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_MEDIASAVEDIR_VALUE),	PrefsWindow, NULL, STRINGA_TextVal,stored_prefs->savemediadir,	TAG_DONE);
															IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_PLAYERVERSION_VALUE),	PrefsWindow, NULL, STRINGA_TextVal,stored_prefs->playerversion,	TAG_DONE);
															IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_OS_VALUE),			PrefsWindow, NULL, STRINGA_TextVal,stored_prefs->detectedos,	TAG_DONE);
															IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_URLOPENER_VALUE),		PrefsWindow, NULL, STRINGA_TextVal,stored_prefs->urlopener,		TAG_DONE);

															IIntuition->GetAttr(WINDOW_SigMask, win, &sigmask);
															while (!done)
															{
																siggot = IExec->Wait(sigmask | SIGBREAKF_CTRL_C);
																if (siggot & SIGBREAKF_CTRL_C) done = TRUE;
																while ((result = RA_HandleInput(win, &code)))
																{
																	switch(result & WMHI_CLASSMASK)
																	{
																		case WMHI_CLOSEWINDOW:
																			done = TRUE;
																			break;
																		case WMHI_GADGETUP:
																			switch (result & WMHI_GADGETMASK)
																			{
																				case OBJ_SCROLLER:
																					char value[5];
																					sprintf(value, "%d", code);
																					IIntuition->RefreshSetGadgetAttrs(GAD(OBJ_SCROLLER_VALUE), PrefsWindow, NULL, GA_Text, value,TAG_DONE);
																				break;
																				case OBJ_CANCEL:
																					done=TRUE;
																				break;
																				case OBJ_OK:
																					IIntuition->GetAttr(SCROLLER_Top, OBJ(OBJ_SCROLLER), &iValue);
																					_rcfile.verbosityLevel(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_LOGTOFILE), &iValue);
																					_rcfile.useWriteLog(iValue);
																					
																					IIntuition->GetAttr(STRINGA_TextVal, OBJ(OBJ_LOGFILENAME_VALUE), &sValue);
																			        tmp.assign((const char *)sValue);
																					_rcfile.setDebugLog(tmp);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_LOGPARSER), &iValue);
																					_rcfile.useParserDump(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_LOGSWF), &iValue);
																					_rcfile.useActionDump(iValue);
																					
																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_LOGMALFORMEDSWF), &iValue);
																					_rcfile.showMalformedSWFErrors(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_LOGACTIONSCRIPT), &iValue);
																					_rcfile.showASCodingErrors(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_LOGLOCALCONNECTION), &iValue);
																					_rcfile.setLCTrace(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_CONNECTLOCALHOST), &iValue);
																					_rcfile.useLocalHost(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_CONNECTLOCALDOMAIN), &iValue);
																					_rcfile.useLocalDomain(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_DISABLESSL), &iValue);
																					_rcfile.insecureSSL(iValue);

																					IIntuition->GetAttr(STRINGA_TextVal, OBJ(OBJ_SHAREDOBJDIR_VALUE), &sValue);
																			        tmp.assign((const char *)sValue);
																					_rcfile.setSOLSafeDir(tmp);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_DONTWRITESHAREDOBJ), &iValue);
																					_rcfile.setSOLReadOnly(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_ONLYLOCALSHAREDOBJ), &iValue);
																					_rcfile.setSOLLocalDomain(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_DISABLELOCALCONNOBJ), &iValue);
																					_rcfile.setLocalConnection(iValue);

																					IIntuition->GetAttr(INTEGER_Number, OBJ(OBJ_NETWORKTIMEOUT), &iValue);
																					_rcfile.setStreamsTimeout(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_USESOUNDHANDLER), &iValue);
																					_rcfile.useSound(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_SAVEMEDIASTREAMS), &iValue);
																					_rcfile.saveStreamingMedia(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_SEVEDYNAMICSTREAMS), &iValue);
																					_rcfile.saveLoadedMedia(iValue);

																					IIntuition->GetAttr(STRINGA_TextVal, OBJ(OBJ_MEDIASAVEDIR_VALUE), &sValue);
																			        tmp.assign((const char *)sValue);
																					_rcfile.setMediaDir(tmp);

																					IIntuition->GetAttr(STRINGA_TextVal, OBJ(OBJ_PLAYERVERSION_VALUE), &sValue);
																			        tmp.assign((const char *)sValue);
																					_rcfile.setFlashVersionString(tmp);

																					IIntuition->GetAttr(STRINGA_TextVal, OBJ(OBJ_OS_VALUE), &sValue);
																			        tmp.assign((const char *)sValue);
																					_rcfile.setFlashSystemOS(tmp);

																					IIntuition->GetAttr(STRINGA_TextVal, OBJ(OBJ_URLOPENER_VALUE), &sValue);
																			        tmp.assign((const char *)sValue);
																					_rcfile.setURLOpenerFormat(tmp);

																					IIntuition->GetAttr(INTEGER_Number, OBJ(OBJ_SIZEMOVIELIB), &iValue);
																					_rcfile.setMovieLibraryLimit(iValue);

																					IIntuition->GetAttr(CHECKBOX_Checked, OBJ(OBJ_STARTINPAUSE), &iValue);
																					_rcfile.startStopped(iValue);

																			    	_rcfile.updateFile();
																			    	done = TRUE;
																				break;
																			}
																			break;
																		case WMHI_ICONIFY:
																			if (RA_Iconify(win)) PrefsWindow = NULL;
																			break;
																		case WMHI_UNICONIFY:
																			PrefsWindow = RA_OpenWindow(win);
																			break;
																	}
																}
															}
															IIntuition->DisposeObject(win);
														}
														IExec->DeleteMsgPort(AppPort);
													}
		                    		            break;
											}
                    		            break;
                    		            case 2:
    	        	                        switch (ITEMNUM(menucode))
        	        	                    {
        	        	                    	case 0:	/* Force redraw */
													refreshView();
												break;
												case 1:	/* Toggle Fullscreen */
													toggleFullscreen();
												break;
												case 2:	/* Show updated ranges */
													showUpdatedRegions(!showUpdatedRegions());

													if (showUpdatedRegions())
														nm[13].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
													else
														nm[13].nm_Flags = MENUTOGGLE|CHECKIT;

													// refresh to clear the remaining red lines...
													if (!showUpdatedRegions()) refreshView();
												break;
											}
                    		            break;
                    		            case 3:
    	        	                        switch (ITEMNUM(menucode))
        	        	                    {
    		                                    case 0:  /* Play */
													play();
            		                            break;
    		                                    case 1:  /* Pause */
													pause();
            		                            break;
    		                                    case 2:  /* Stop */
													stop();
            		                            break;
    		                                    case 4:  /* Restart */
													restart();
            		                            break;
                	    	                }
                    		            break;
                    		            case 4:
    	        	                        switch (ITEMNUM(menucode))
        	        	                    {
    		                                    case 0:  /* About Dialog */
													showAboutDialog();
											}
                    		            break;
                    	    	    }

		                            /* Handle multiple selection */
    		                        menucode = item->NextSelect;
	                    	    }
	                    	    else
	                    	    	break; /* No menu no party! */
    	                	}
    	                }
                    break;
					case IDCMP_CLOSEWINDOW:
						if (imsg->IDCMPWindow == _window) 
							IExec->ReplyMsg ((struct Message *)imsg);
                       	killAudioTask();
        	    	    return true;
					break;
					case IDCMP_MOUSEMOVE:
						if (imsg->IDCMPWindow == _window)
						{
							IExec->ReplyMsg ((struct Message *)imsg);
    	   	        		if ( _window->GZZMouseX == x_old && _window->GZZMouseY == y_old) { break; }
							x_old = _window->GZZMouseX;
							y_old = _window->GZZMouseY;
   		        	    	notifyMouseMove(x_old, y_old);
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
   	    		    	        	notifyMouseClick(true);
	                			    button_state_old = imsg->Code;
					    		break;
							    case SELECTUP:
		    	    	            notifyMouseClick(false);
       					            button_state_old = -1;
							    break;
							}
						}
					break;
					case IDCMP_RAWKEY:
						if (imsg->IDCMPWindow == _window)
						{
							if (!(imsg->Code & IECODE_UP_PREFIX))
							{
								code = os4_to_gnash_key(imsg);
						    	mod  = os4_to_gnash_modifier(imsg->Qualifier);
							   	key_event(code, mod, true);
							}
							else
							{
								imsg->Code &= ~IECODE_UP_PREFIX;
								code = os4_to_gnash_key(imsg);
						    	mod  = os4_to_gnash_modifier(imsg->Qualifier);
							   	key_event(code, mod, false);
							}								
						}
	    			break;
					case IDCMP_SIZEVERIFY:
						if (imsg->IDCMPWindow == _window) 
							IExec->ReplyMsg ((struct Message *)imsg);
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
	    else if (sigGot & SIGBREAKF_CTRL_C)
	    {
    	    return true;
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
	int _depth = 24;
	struct ExecBase *sysbase = (struct ExecBase*) IExec->Data.LibBase;

	struct Library *timer = (struct Library *)IExec->FindName(&sysbase->DeviceList, "timer.device");
	ITimer = (struct TimerIFace *)IExec->GetInterface(timer, "main", 1, 0);

	/* Set up reference time. */
	ITimer->GetSysTime(&os4timer_starttime);

    _glue.init(argc, argv);

    _renderer.reset(_glue.createRenderHandler(_depth));
    //_renderer = _glue.createRenderHandler(_depth);
    if ( ! _renderer )
	{
		log_error (_("error creating RenderHandler!\n"));
    	return false;
	}
	signal(SIGINT, SIG_IGN);

    return true;
}


bool
AOS4Gui::createWindow(const char *title, int width, int height, int xPosition, int yPosition)
{
    _width = width;
    _height = height;
	
    _glue.saveOrigiginalDimension(width,height,xPosition,yPosition);
	_orig_width  = width;
	_orig_height = height;
	_orig_xPosition = xPosition;
	_orig_yPosition = yPosition;

    _glue.prepDrawingArea(_width, _height);

    //set_Renderer(_renderer);
	_runResources.setRenderer(boost::shared_ptr<Renderer>(_renderer));

	struct Window *_window = _glue.getWindow();

	_window_title = (char*)malloc(strlen(title)+1);
	memset(_window_title,0x0,strlen(title)+1);
	strcpy(_window_title,title);

	IIntuition->SetWindowTitles(_window,title,title);
	IIntuition->ChangeWindowBox(_window,xPosition,yPosition,width,height);
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
    return true;
}

bool
AOS4Gui::setupEvents()
{
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
		case RAWKEY_SPACE:		c = gnash::key::SPACE;		break;
		case RAWKEY_TAB:		c = gnash::key::TAB;		break;
		case RAWKEY_BACKSPACE:	c = gnash::key::BACKSPACE;	break;
		case RAWKEY_ENTER:
		case RAWKEY_RETURN:		c = gnash::key::ENTER;		break;
		
   		default: 				
   			c = gnash::key::INVALID; 	
   		break;
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

uint32
AOS4Gui::OS4_GetTicks()
{
	struct TimeVal cur;
	uint32 result = 0;
	
	ITimer->GetSysTime(&cur);
	ITimer->SubTime(&cur, &os4timer_starttime);
	result = cur.Seconds * 1000 + cur.Microseconds / 1000;

	return result;
}

void
AOS4Gui::PrintMsg( CONST_STRPTR text )
{
	Object *reqobj;

	reqobj = (Object *)IIntuition->NewObject( IRequester->REQUESTER_GetClass(), NULL,
            REQ_Type,       REQTYPE_INFO,
			REQ_TitleText,  "Gnash",
            REQ_BodyText,   text,
            REQ_Image,      REQIMAGE_INFO,
            REQ_GadgetText, "Abort",
            TAG_END
        );

	if ( reqobj )
	{
		IIntuition->IDoMethod( reqobj, RM_OPENREQ, NULL, NULL, NULL, TAG_END );
		IIntuition->DisposeObject( reqobj );
	}
}

void
AOS4Gui::showAboutDialog(void)
{
	CONST_STRPTR about = "Gnash is the GNU SWF Player based on GameSWF.\n"
    					"\nRenderer: "
    					RENDERER_CONFIG
    					"\nGUI: "
					    "AmigaOS4"
					    "\nMedia: "
    					MEDIA_CONFIG" "
#ifdef HAVE_FFMPEG_AVCODEC_H
    					"\nBuilt against ffmpeg version: "
    					LIBAVCODEC_IDENT
#endif
						"\n\nCopyright (C) 2005, 2006, 2007, "
            			"2008, 2009, 2010 The Free Software Foundation"
						"\n\nAmigaOS4 Version by Andrea Palmate' - http://www.amigasoft.net";

	PrintMsg(about);
}

struct GnashPrefs *
AOS4Gui::ReadPrefs(void)
{
    RcInitFile& _rcfile = RcInitFile::getDefaultInstance();
	struct GnashPrefs *localprefs;

	localprefs = (struct GnashPrefs *) malloc(sizeof(GnashPrefs));
	memset(localprefs, 0, sizeof(GnashPrefs));

	localprefs->verbosity 			= _rcfile.verbosityLevel();
	localprefs->logtofile 			= _rcfile.useWriteLog();
	strncpy(localprefs->logfilename, 	_rcfile.getDebugLog().c_str(), 			 254);
	localprefs->logparser 			= _rcfile.useParserDump();
	localprefs->logswf    			= _rcfile.useActionDump();
	localprefs->logmalformedswf 	= _rcfile.showMalformedSWFErrors();
	localprefs->logactionscript		= _rcfile.showASCodingErrors();
#if 0
	localprefs->loglocalconn		= _rcfile.getLCTrace();
#endif
	localprefs->connectlocalhost	= _rcfile.useLocalHost();
	localprefs->connectlocaldomain	= _rcfile.useLocalDomain();
	localprefs->disablessl			= _rcfile.insecureSSL();
	strncpy(localprefs->sharedobjdir, 	_rcfile.getSOLSafeDir().c_str(), 		 254);
	localprefs->dontwriteso			= _rcfile.getSOLReadOnly();
	localprefs->onlylocalso			= _rcfile.getSOLLocalDomain();
	localprefs->disablelocal		= _rcfile.getLocalConnection();
	localprefs->nettimeout			= _rcfile.getStreamsTimeout();
	localprefs->usesound			= _rcfile.useSound();
	localprefs->savemedia			= _rcfile.saveStreamingMedia();
	localprefs->savedynamic			= _rcfile.saveLoadedMedia();
	strncpy(localprefs->savemediadir, 	_rcfile.getMediaDir().c_str(), 			 254);
	strncpy(localprefs->playerversion, 	_rcfile.getFlashVersionString().c_str(), 31);
	strncpy(localprefs->detectedos, 	_rcfile.getFlashSystemOS().c_str(), 	 31);
	strncpy(localprefs->urlopener, 		_rcfile.getURLOpenerFormat().c_str(),	 254);
	localprefs->maxsizemovielib		= _rcfile.getMovieLibraryLimit();
	localprefs->startpaused			= _rcfile.startStopped();
	return localprefs;
}

} // namespace gnash

