// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "plugin.h"
#define MIME_TYPES_HANDLED  "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME     "Shockwave Flash 8.0"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":swf:"PLUGIN_NAME
#define PLUGIN_DESCRIPTION "Gnash, a GPL\'d Flash Player. More details at http://www.gnu.org/software/gnash/"

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <X11/extensions/xf86vmode.h>
#ifdef HAVE_GTK_GTKGL_H
#include <gtk/gtkgl.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Sunkeysym.h>
// #include <SDL.h>
#include <SDL_thread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

#include "tu_file.h"
#include "tu_types.h"
//#include "Thread.h"
#include "player.h"
#include "xmlsocket.h"

using namespace std;
using namespace gnash;

extern bool processing;

// Static members. We have to share this data amongst all
NPBool       nsPluginInstance::_plugInitialized = FALSE;
Display     *nsPluginInstance::_xDisplay = NULL;
SDL_mutex   *nsPluginInstance::_glMutex = NULL;
SDL_cond    *nsPluginInstance::_gCond = NULL;
SDL_mutex   *nsPluginInstance::_playerMutex = NULL;

// These aren't static members of the class because we have to
// call these from the C callback for the Mozilla SDK.
Display     *gxDisplay = NULL;
SDL_mutex   *glMutex = NULL;
SDL_cond    *gCond = NULL;
SDL_mutex   *playerMutex = NULL;

// static int   streamfd = -1;
// static float s_scale = 1.0f;
// static bool  s_verbose = false;
// static int   doneYet = 0;
static bool  waitforgdb = false;

const int INBUFSIZE = 1024;
static void xt_event_handler(Widget xtwidget, nsPluginInstance *plugin,
		 XEvent *xevent, Boolean *b);

#if 0
static int attributeList_noFSAA[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None };
#else
static int attributeList_noFSAA[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
#endif
static int attributeList_FSAA[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, GLX_SAMPLE_BUFFERS_ARB, 1,GLX_SAMPLES_ARB, 1, None };

#ifdef HAVE_LIBXML
extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

/// \brief Return the MIME Type description for this plugin.
char*
NPP_GetMIMEDescription(void)
{
    return(MIME_TYPES_DESCRIPTION);
}

//
// general initialization and shutdown
//

/// \brief Initialize the plugin
///
/// This C function gets called once when the plugin is loaded,
/// regardless of how many instantiations there is actually playing
/// movies. So this is where all the one time only initialization
/// stuff goes.
NPError
NS_PluginInitialize()
{
    printf("%s: Initializing the Plugin\n", __PRETTY_FUNCTION__);
    glMutex = SDL_CreateMutex();
    playerMutex = SDL_CreateMutex();
    gCond = SDL_CreateCond();

    gxDisplay = XOpenDisplay(NULL);
    
    return NPERR_NO_ERROR;
}

/// \brief Shutdown the plugin
///
/// This C function gets called once when the plugin is being
/// shutdown, regardless of how many instantiations actually are
/// playing movies. So this is where all the one time only
/// shutdown stuff goes.
void
NS_PluginShutdown()
{
    printf("%s(%d): Shutting down the plugin\n", __PRETTY_FUNCTION__, __LINE__);
    XCloseDisplay(gxDisplay);
    gxDisplay = NULL;
    SDL_DestroyMutex(glMutex);
    glMutex = NULL;
    SDL_DestroyMutex(playerMutex);
    playerMutex = NULL;
    SDL_DestroyCond(gCond);
    gCond = NULL;
//    SDL_Quit();
}

/// \brief Retrieve values from the plugin for the Browser
///
/// This C function is called by the browser to get certain
/// information is needs from the plugin. This information is the
/// plugin name, a description, etc...
NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
    char tmp[1024];
    NPError err = NPERR_NO_ERROR;
    
    switch (aVariable) {
      case NPPVpluginNameString:
          *((char **)aValue) = PLUGIN_NAME;
          break;
      // This becomes the description field you see below the opening
      // text when you type about:plugins
      case NPPVpluginDescriptionString:
	  snprintf(tmp, 1024,
		  "Gnash, a GPL\'d Flash Player. More details at "
		  "<a href=http://www.gnu.org/software/gnash/>"
		  "http://www.gnu.org/software/gnash</a>"
	      );
          *((char **)aValue) = tmp;
          break;
      case NPPVpluginTimerInterval:
      case NPPVpluginNeedsXEmbed:
      case NPPVpluginKeepLibraryInMemory:
      default:
          err = NPERR_INVALID_PARAM;
          break;
    }
    return err;
}

/// \brief construct our plugin instance object
///
/// This instantiates a new object via a C function used by the
/// browser.
nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    if(!aCreateDataStruct)
        return NULL;

    nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
    
    return plugin;
}

/// \brief destroy our plugin instance object
///
/// This destroys our instantiated object via a C function used by the
/// browser.
void
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    if (aPlugin) {
        delete (nsPluginInstance *)aPlugin;
    }
}

//
// nsPluginInstance class implementation
//

/// \brief Construct a new nsPluginInstance object
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
                                                    mInstance(aInstance),
                                                    mWindow(0),
                                                    mXtwidget(0),
                                                    mFontInfo(0),
						    mContext(NULL),
						    _glInitialized(FALSE)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
//    _plugInitialized(FALSE);
    bShutting = FALSE;
    mThread = NULL;
//    mMutex = SDL_CreateMutex();
}

/// \brief Destroy a nsPluginInstance object
nsPluginInstance::~nsPluginInstance()
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
//     if (mThread != NULL) {
// 	SDL_KillThread(mThread);
//     }
//    if (gCond) {
//        SDL_DestroyCond(gCond);
//    }
}

/// \brief Initialize an instance of the plugin object
/// 
/// This methods initializes the plugin object, and is called for
/// every movie that gets played. This is where the movie playing
/// specific initialization goes.
NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
    if(aWindow == NULL) {
	printf("%s: ERROR: Window handle was bogus!\n", __PRETTY_FUNCTION__);
        return FALSE;
    } else {
	printf("%s: X origin = %d, Y Origin = %d, Width = %d,"
	       " Height = %d, WindowID = %p, this = %p\n",
	       __PRETTY_FUNCTION__,
	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
	       aWindow->window, this);
    }

    // Only for developers. Make the plugin block here so we can
    // attach GDB to it.
    bool gdb = false;
    while (gdb) {
	printf ("Waiting for GDB for pid %d\n", getpid());
	sleep(5);
    }    

    if (_plugInitialized) {
	printf("%s Already initialized...\n", __PRETTY_FUNCTION__);
 	return TRUE;
    }

    initGL();

    _plugInitialized = TRUE;

//    mThread = SDL_CreateThread(playerThread3, this);
    
//     char SDL_windowhack[32];
//     sprintf (SDL_windowhack,"SDL_WINDOWID=%d", aWindow->window);
//     putenv (SDL_windowhack);
    
//    _plugInitialized = TRUE;

    return TRUE;
}

/// \brief Shutdown an instantiated object
///
/// This shuts down an object, and is called for every movie that gets
/// played. This is where the movie playing specific shutdown code
/// goes.
void
nsPluginInstance::shut()
{
    printf("%s(%d): Entering. \n", __PRETTY_FUNCTION__, __LINE__);

    destroyContext();
    
    if (mThread) {
	SDL_KillThread(mThread);
	mThread = NULL;
    }
}

/// \brief Set the window to be used to render in
///
/// This sets up the window the plugin is supposed to render
/// into. This calls passes in various information used by the plugin
/// to setup the window. This may get called multiple times by each
/// instantiated object, so it can't do much but window specific
/// setup here.
NPError
nsPluginInstance::SetWindow(NPWindow* aWindow)
{
    if(aWindow == NULL) {
	printf("%s: ERROR: Window handle was bogus!\n", __PRETTY_FUNCTION__);
        return FALSE;
    } else {
	printf("%s: X origin = %d, Y Origin = %d, Width = %d,"
	       " Height = %d, WindowID = %p, this = %p\n",
	       __PRETTY_FUNCTION__,
	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
	       aWindow->window, this);
    }    
    
//     if (_glInitialized) {
// 	printf("%s Already initialized...\n", __PRETTY_FUNCTION__);
// 	return TRUE;
//     }    
    
    if (aWindow->x == mX && aWindow->y == mY
	&& aWindow->width == mWidth
	&& aWindow->height == mHeight
	&& (unsigned long)(aWindow->window) == mWindow) {
	return TRUE;
    }

    lockGL();
    lockX();
    
    mX = aWindow->x;
    mY = aWindow->y;
    mWidth = aWindow->width;
    mHeight = aWindow->height;
    
    if (mWindow == (Window) aWindow->window) {
        // The page with the plugin is being resized.
        // Save any UI information because the next time
        // around expect a SetWindow with a new window id.
	printf("Error: Setwindow() called with same window handle - but resizing plugin unhandled!\n");
    } else {
        mWindow = (Window) aWindow->window;
        NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct *)aWindow->ws_info;
	mVisual = ws_info->visual;
        mDepth = ws_info->depth;
        mColormap = ws_info->colormap;
//        _xDisplay = ws_info->display;

        if (!mFontInfo) {
            if (!(mFontInfo = XLoadQueryFont(gxDisplay, "9x15")))
                printf("Cannot open 9X15 font\n");
        }
	
	XVisualInfo *vi = glXChooseVisual(gxDisplay, DefaultScreen(gxDisplay), attributeList_FSAA);
	if (vi == NULL) {
	    vi = glXChooseVisual(gxDisplay, DefaultScreen(gxDisplay), attributeList_noFSAA);
	} else {
	    vi->visual = mVisual;
	}
	
	mContext = glXCreateContext(gxDisplay, vi, 0, GL_TRUE);
	if (mContext) {
	    printf("%s: Got new glxContext %p\n", __PRETTY_FUNCTION__, mContext);
	    _glInitialized = TRUE;
	    setGL();
	} else {
	    printf("%s: ERROR: Couldn't get new glxContext!\n", __PRETTY_FUNCTION__);
	}

        // add xt event handler#
        long event_mask = ExposureMask|KeyPress|KeyRelease|ButtonPress|ButtonRelease;        Widget xtwidget;
	
//         xtwidget =  XtWindowToWidget((Display *) gxdisplay,
//                                      (Window) aWindow->window);
//         XtAddEventHandler(xtwidget, event_mask, FALSE,
//                           (XtEventHandler) xt_event_handler, this);
    }
    freeX();
    
    resizeWindow(mWidth,mHeight);
    
    return NPERR_NO_ERROR;
}

const char *
nsPluginInstance::getVersion()
{
    printf("%s(%d): Entering. \n", __PRETTY_FUNCTION__, __LINE__);
    return NPN_UserAgent(mInstance);
}

NPError
nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
    NPError err = NPERR_NO_ERROR;
    switch (aVariable) {
      case NPPVpluginNameString:
      case NPPVpluginDescriptionString:
          return NS_PluginGetValue(aVariable, aValue) ;
          break;
      default:
          err = NPERR_INVALID_PARAM;
          break;
    }
    return err;

}

/// \brief Write a status message
///
/// This writes a status message to the status line at the bottom of
/// the browser window and the console firefox was started from.
NPError
nsPluginInstance::WriteStatus(char *msg) const
{
    NPN_Status(mInstance, msg);
    printf("%s\n", msg);

    return NPERR_NO_ERROR;
}

/// \brief Open a new data stream
///
/// Opens a new incoming data stream, which is the flash movie we want
/// to play.
/// A URL can be pretty ugly, like in this example:
/// http://www.shockwave.com/swf/navbar/navbar_sw.swf?atomfilms=http%3a//www.atomfilms.com/af/home/&shockwave=http%3a//www.shockwave.com&gameblast=http%3a//gameblast.shockwave.com/gb/gbHome.jsp&known=0
/// ../flash/gui.swf?ip_addr=foobar.com&ip_port=3660&show_cursor=true&path_prefix=../flash/&trapallkeys=true"
///
/// So this is where we parse the URL to get all the options passed in
/// when invoking the plugin.
NPError
nsPluginInstance::NewStream(NPMIMEType type, NPStream * stream,
                            NPBool seekable, uint16 * stype)
{
    char tmp[300];
    memset(tmp, 0, 300);
    string url = stream->url;
    string fname, opts;
    unsigned int start, end, eq;
    bool dumpopts = false;

    printf("%s: this = %p, URL is %s\n", __PRETTY_FUNCTION__,
	   (void *)this, stream->url);

    end   = url.find(".swf", 0) + 4;
    start = url.rfind("/", end) + 1;
    fname = "/tmp/";
    fname += url.substr(start, end - start);

    // extract the parameters from the URL
    start = url.find("?", end);
    end = url.size();
    if (start != string::npos) {
	opts = url.substr(start+1, end);
    }

    printf("The full URL is %s\n", url.c_str());
    while (opts.size() > 0) {
	start = 0;
	eq = opts.find("=", 0);
 	if (opts[0] == '&') {
	    start++;
	}
	end = opts.find("&", start);
 	if (end <= 0) {
 	    end = opts.size();
	}
 	if (eq == string::npos) {
 	    eq = opts.size();
 	}
	string name = opts.substr(start, eq);
	string value = opts.substr(eq+1, end-eq-1);
	if (dumpopts) {
	    printf("Option %s = %s\n", name.c_str(), value.c_str());
	}
	// Look for our special debug flags
	if (name == "debug") {
 	    printf("Debug flag is %s\n", value.c_str());
	    if (value == "waitforgdb") {
		waitforgdb = true;
	    }
	    if (value == "dumpopts") {
		dumpopts = true;
	    }
	} else {
	    _options[name] = value;
	}
	if (opts[end] == '&') {
		end++;
	}
	opts.erase(start, end);
    }
    
    //  printf("%s: URL is %s\n", __PRETTY_FUNCTION__, url.c_str());
    printf("%s: Open stream for %s, this = %p\n", __PRETTY_FUNCTION__,
	   fname.c_str(), (void *)this);

    sprintf(tmp, "Loading Flash movie %s", fname.c_str());
    WriteStatus(tmp);
  
    _streamfd = open(fname.c_str(), O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if (_streamfd < 0) {
        sprintf(tmp,"%s can't be opened, check your permissions!\n", fname.c_str());
        WriteStatus(tmp);
        _streamfd = open(fname.c_str(), O_TRUNC | O_WRONLY, S_IRUSR|S_IRGRP|S_IROTH);
        if (_streamfd < 0) {
            sprintf(tmp,"%s can't be created, check your permissions!\n", fname.c_str());
            WriteStatus(tmp);
        }
    }

    swf_file = fname;
    processing = true;

    return NPERR_NO_ERROR;
}

/// \brief Destroy the data stream we've been reading.
NPError
nsPluginInstance::DestroyStream(NPStream * stream, NPError reason)
{
    char tmp[300];
    memset(tmp, 0, 300);
    nsPluginInstance *arg = (nsPluginInstance *)this;
    sprintf(tmp, "Done Flash movie %s", swf_file.c_str());
    WriteStatus(tmp);

    printf("%s: this = %p, URL is %s\n", __PRETTY_FUNCTION__,
	   (void *)arg, stream->url);
    processing = false;

    if (_streamfd) {
        close(_streamfd);
        _streamfd = -1;
    }
    
    // Wait for GDB
    if (waitforgdb) {
	printf("Attach GDB to PID %d to debug!\n", getpid());
	printf("This thread will block until then!...\n");
	printf("Once blocked here, you can set other breakpoints.\n");
	printf("do a \"set variable waitforgdb=false\" to continue\n");
	while (waitforgdb) {
	    sleep(1);
	}
    }

    printf("%s: Starting player Thread for this = %p\n",
	   __PRETTY_FUNCTION__, (void *)this);
    mThread = SDL_CreateThread(playerThread, this);
    
    SDL_mutexP(playerMutex);
    SDL_CondBroadcast(gCond);    

    sprintf(tmp, "Started thread for Flash movie %s", swf_file.c_str());
    WriteStatus(tmp);

    return NPERR_NO_ERROR;
}

void
nsPluginInstance::URLNotify(const char *url, NPReason reason,
                            void *notifyData)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    printf("URL: %s\nReason %i\n", url, reason);
}

/// \brief Return how many bytes we can read into the buffer
int32
nsPluginInstance::WriteReady(NPStream * stream)
{
//   printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
//   printf("Stream for %s is ready\n", stream->url);

    return INBUFSIZE;
}

/// \brief Read the data stream from Mozilla/Firefox
///
/// For now we read the bytes and write them to a disk file.
int32
nsPluginInstance::Write(NPStream * stream, int32 offset, int32 len,
                        void *buffer)
{
//   printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
//   printf("Reading Stream %s, offset is %d, length = %d \n",
//          stream->url, offset, len);

    return write(_streamfd, buffer, len);
}

/// \brief Initialize OpenGL
///
void
nsPluginInstance::initGL()
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    
    if (_glInitialized) {
	printf("%s: OpenGL already initialized...\n", __PRETTY_FUNCTION__);
	return;
    }
    
    // Grab control of the display
    lockGL();
    lockX();
    setGL();
    
    printf("%s: Initializing OpenGL...\n", __PRETTY_FUNCTION__);

    // Enable smooth shading 
    glShadeModel(GL_SMOOTH);
  
    // Set the background black 
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
    // Depth buffer setup 
    glClearDepth(1.0f);
  
    // Enables Depth Testing 
    glEnable(GL_DEPTH_TEST);
  
    // The Type Of Depth Test To Do 
    glDepthFunc(GL_LEQUAL);

    // Really Nice Perspective Calculations 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Release control of the display
    unsetGL();
    freeX();
    freeGL();
}

/// \brief Shutdown OpenGL
void
nsPluginInstance::destroyContext()
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);

    if (!_glInitialized) {
	printf("%s: OpenGL already killed...\n", __PRETTY_FUNCTION__);
	return;
    }

    if (gxDisplay && mContext) {
	// Grab control of the display
	lockGL();
	lockX();    
	setGL();
	
	printf("%s: Destroying GLX Context %p...\n", __PRETTY_FUNCTION__,
	       mContext);
	glXDestroyContext(gxDisplay, mContext);
	_glInitialized = FALSE;
	mContext = NULL;
	
	// Release control of the display
	unsetGL();
	freeX();
	freeGL();
    }
}

/// \brief Resize our viewport after a window resize event
int
nsPluginInstance::resizeWindow( int width, int height )
{
    printf("%s(%d): Width = %d, Height = %d\n",
	   __PRETTY_FUNCTION__, __LINE__, width, height);

    if (!_plugInitialized) {
	printf("%s: OpenGL not initialized...\n", __PRETTY_FUNCTION__);
	return true;
    }

    // Grab control of the display
    lockGL();
    lockX();
    setGL();
    
    printf("%s: Resizing window...\n", __PRETTY_FUNCTION__);

    // Height / width ration 
    GLfloat ratio;
  
    // Protect against a divide by zero 
    if (height == 0) {
        height = 1;
    }
  
    ratio = (GLfloat)width / (GLfloat)height;
  
    // Setup our viewport. 
    glViewport(0, 0, (GLint)width, (GLint)height);
  
    // change to the projection matrix and set our viewing volume. 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  
    // Set our perspective 
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
  
    // Make sure we're changing the model view and not the projection 
    glMatrixMode(GL_MODELVIEW);
  
    // Reset The View 
//    glLoadIdentity();
  
    // Release control of the display
    unsetGL();
    freeX();
    freeGL();

    return(true);
}

/// \brief Draw a hardcoded image
///
/// This draws a hardcoded OpenGL graphic into the window, and is only
/// used for testing by developers.
void
nsPluginInstance::drawTestScene( void )
{
    printf("%s: for instance %p\n", __PRETTY_FUNCTION__, this);

    static SDL_mutex   *mutant = NULL;

    if (!mutant) {
        mutant = SDL_CreateMutex();
    }

    SDL_mutexP(mutant);
    // Grab control of the display
    lockGL();
    lockX();
    setGL();
    
    printf("%s: Drawing graphic...\n", __PRETTY_FUNCTION__);

    // Clear The Screen And The Depth Buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Move Left 1.5 Units And Into The Screen 6.0
    glLoadIdentity();
    glTranslatef( -1.5f, 0.0f, -6.0f );

    glColor3f(1.0f,1.0f,1.0f);
    
    glBegin( GL_TRIANGLES );            // Drawing Using Triangles
      glVertex3f(  0.0f,  1.0f, 0.0f ); // Top
      glVertex3f( -1.0f, -1.0f, 0.0f ); // Botom Left
      glVertex3f(  1.0f, -1.0f, 0.0f ); // Bottom Rigt
    glEnd( );                           // Finished Drawing The Triangle
    
    /* Move Right 3 Units */
    glTranslatef( 3.0f, 0.0f, 0.0f );
    
    glBegin( GL_QUADS );                // Draw A Quad
      glVertex3f( -1.0f,  1.0f, 0.0f ); // Top Left
      glVertex3f(  1.0f,  1.0f, 0.0f ); // Top Right
      glVertex3f(  1.0f, -1.0f, 0.0f ); // Bottom Right
      glVertex3f( -1.0f, -1.0f, 0.0f ); // Bottom Left
    glEnd( );                   // Done Drawing The Quad

    swapBuffers();
    
    // Release control of the display
    unsetGL();
    freeX();
    freeGL();
    SDL_mutexP(mutant);
}

/// \brief Handle X events
///
/// This C function handles events from X, like keyboard events, or
/// Expose events that we're interested in.
static void
xt_event_handler(Widget xtwidget, nsPluginInstance *plugin,
		 XEvent *xevent, Boolean *b)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    int        keycode;
    KeySym     keysym;
#if 0
    SDL_Event  sdl_event;
    SDL_keysym sdl_keysym;

    //    handleKeyPress((SDL_keysym)keysym);
    printf("Peep Event returned %d\n", SDL_PeepEvents(&sdl_event, 1, SDL_PEEKEVENT, SDL_USEREVENT|SDL_ACTIVEEVENT|SDL_KEYDOWN|SDL_KEYUP|SDL_MOUSEBUTTONUP|SDL_MOUSEBUTTONDOWN));
  
    if (SDL_PollEvent(&sdl_event)) {
        switch(sdl_event.type) {
          case SDL_ACTIVEEVENT:
          case SDL_VIDEORESIZE:
          case SDL_KEYDOWN:
              /* handle key presses */
              handleKeyPress( &sdl_event.key.keysym );
              break;
          default:
              break;
      
        }
    }
#endif
  
    switch (xevent->type) {
      case Expose:
          // get rid of all other exposure events
          if (plugin) {
// 	      if (_glInitialized) {
// 		  plugin->setGL();
// #ifdef TEST_GRAPHIC
// 		  plugin->drawTestScene();
// 		  plugin->swapBuffers();
// 		  plugin->freeX();
// #else
// 		  gnash::movie_interface *m = gnash::get_current_root();
// 		  if (m != NULL) {
// 		      m->display();
// 		  }
// #endif
// 		  printf("Drawing GL Scene for expose event!\n");
// 	      } else {
 		  printf("GL Surface not initialized yet, ignoring expose event!\n");
// 	      }
          }
          break;
      case ButtonPress:
//     fe.type = FeButtonPress;
          printf("Button Press\n");
          break;
      case ButtonRelease:
          //     fe.type = FeButtonRelease;
          printf("Button Release\n");
          break;
      case KeyPress:
          keycode = xevent->xkey.keycode;
		plugin->lockX();
          keysym = XLookupKeysym((XKeyEvent*)xevent, 0);
          printf ("%s(%d): Keysym is %s\n", __PRETTY_FUNCTION__, __LINE__,
                  XKeysymToString(keysym));
		plugin->freeX();

          switch (keysym) {
            case XK_Up:
                printf("Key Up\n");
                break;
            case XK_Down:
                printf("Key Down\n");
                break;
            case XK_Left:
                printf("Key Left\n");
                break;
            case XK_Right:
                printf("Key Right\n");
                break;
            case XK_Return:
                printf("Key Return\n");
                break;
      
            default:
                break;
          }
    }
}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
