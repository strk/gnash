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
#ifdef USE_GTK_PLUG
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Sunkeysym.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include "tu_file.h"
#include "tu_types.h"
#include "player.h"
#include "xmlsocket.h"

// Mozilla SDK headers
#include "prinit.h"
#include "prlock.h"
#include "prcvar.h"
#include "prerr.h"
#include "prerror.h"
#include "prthread.h"

using namespace std;
using namespace gnash;

extern bool processing;

extern NPNetscapeFuncs NPNFuncs;

NPBool      plugInitialized = FALSE;
Display     *gxDisplay = NULL;
PRLock      *glMutex = NULL;
PRLock      *playerMutex = NULL;
PRCondVar   *playerCond = NULL;

// Static members. We have to share this data amongst all
//Display     *nsPluginInstance::gxDisplay = NULL;
//PRLock      *nsPluginInstance::_playerMutex = NULL;
//PRCondVar   *nsPluginInstance::_playerCond = NULL;
//SDL_cond    *nsPluginInstance::_gCond = NULL;
//SDL_mutex   *nsPluginInstance::_playerMutex = NULL;
//PRLock      *nsPluginInstance::_prlock = NULL;

GtkWidget   *gtkplug = NULL;
GtkMenu     *popup_menu = NULL;
GtkMenuItem *menuitem_play = NULL;
GtkMenuItem *menuitem_pause = NULL;

// static int   streamfd = -1;
// static float s_scale = 1.0f;
// static bool  s_verbose = false;
// static int   doneYet = 0;
static bool  waitforgdb = false;

const int INBUFSIZE = 1024;
// static void xt_event_handler(Widget xtwidget, nsPluginInstance *plugin,
// 		 XEvent *xevent, Boolean *b);

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

#ifdef USE_GTK_PLUG
// destroy a GtkWidget. This is only used if using an external window
// was used to render the movie in.
gboolean
destroy_callback(GtkWidget * widget, GdkEvent * event,
	       nsPluginInstance * instance)
{
    gtk_widget_destroy(widget);
}

gboolean
fixme_callback(GtkWidget * widget, GdkEvent * event,
	       nsPluginInstance * instance)
{
    GNASH_REPORT_FUNCTION;

    dbglogfile << "Got a Callback! " << event->type << endl;
    
    switch (event->type) {
      case GDK_DESTROY:
	  dbglogfile << "Got a DESTROY event" << endl;
	  break;
      case GDK_EXPOSE:
//	  dbglogfile << "Got an EXPOSE event" << endl;
	    break;
      case GDK_KEY_PRESS:
	  dbglogfile << "Got a KEY PRESS event" << endl;
	  break;
      case GDK_KEY_RELEASE:
	  dbglogfile << "Got a KEY RELEASE event" << endl;	    
	  break;
      case GDK_MAP:
//	  dbglogfile << "Got a MAP event" << endl;
	  break;
      case GDK_FOCUS_CHANGE:
//	  dbglogfile << "Got a FOCUS CHANGE event" << endl;
	  break;
      case GDK_BUTTON_PRESS:
	  dbglogfile << "Got a BUTTON PRESS event" << endl;
	  break;
      case GDK_ENTER_NOTIFY:
//	  dbglogfile << "Got a ENTER event" << endl;
	  break;
      case GDK_LEAVE_NOTIFY:
//	  dbglogfile << "Got a LEAVE event" << endl;
	  break;
      case GDK_NOTHING:
      case GDK_DELETE:
      case GDK_MOTION_NOTIFY:
      case GDK_2BUTTON_PRESS:
      case GDK_3BUTTON_PRESS:
      case GDK_BUTTON_RELEASE:
      case GDK_CONFIGURE:
      case GDK_UNMAP:
      case GDK_PROPERTY_NOTIFY:
      case GDK_SELECTION_CLEAR:
      case GDK_SELECTION_NOTIFY:
      case GDK_SELECTION_REQUEST:	  
      case GDK_PROXIMITY_IN:
      case GDK_PROXIMITY_OUT:
      case GDK_DRAG_ENTER:
      case GDK_DRAG_LEAVE:
      case GDK_DRAG_MOTION:
      case GDK_DRAG_STATUS:
      case GDK_DROP_START:
      case GDK_DROP_FINISHED:
      case GDK_CLIENT_EVENT:
      case GDK_VISIBILITY_NOTIFY:
      case GDK_NO_EXPOSE:
      case GDK_SCROLL:
      case GDK_WINDOW_STATE:
      case GDK_SETTING:
      case GDK_OWNER_CHANGE:
      case GDK_GRAB_BROKEN:
	  break;
    }
    
    
//     gtk_widget_hide(GTK_WIDGET(instance->gtkwidget));
//     instance->Quit();
    return TRUE;
}

#endif

void
PR_CALLBACK Destructor(void *data)
{
    GNASH_REPORT_FUNCTION;

    /*
     * We don't actually free the storage since it's actually allocated
     * on the stack. Normally, this would not be the case and this is
     * the opportunity to free whatever.
    PR_Free(data);
     */
}  /* Destructor */


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
    GNASH_REPORT_FUNCTION;

    NPError err = NPERR_NO_ERROR;
    PRBool supportsXEmbed = PR_FALSE;
    NPNToolkitType toolkit;

    // This mutex is to lock the display before doing any OpenGL or
    // X11 function calls.
    glMutex = PR_NewLock();
    if (glMutex) {
	dbglogfile << "Allocated new GL Mutex" << endl;
    } else {
	dbglogfile << "ERROR: Couldn't allocate new GL Mutex!" << endl;
    }
    

    // This mutex is only used with the condition variable.
    playerMutex = PR_NewLock();
    if (playerMutex) {
	dbglogfile << "Allocated new X11 Mutex" << endl;
    } else {
	dbglogfile << "ERROR: Couldn't allocate new Player Mutex!" << endl;
    }

    // This is used to signal the player when it should start playing
    // a movie.
    playerCond = PR_NewCondVar(playerMutex);
    if (playerCond) {
	dbglogfile << "Allocated new condition variable" << endl;
    } else {
	dbglogfile << "ERROR: Couldn't allocate new Condition Variable!" << endl;
    }

    // Open a connection to the X11 server so we can lock the Display
    // when swapping GLX contexts.
    gxDisplay = XOpenDisplay(NULL);
    if (gxDisplay) {
	dbglogfile << "Opened connection to X11 server" << endl;
    } else {
	dbglogfile << "ERROR: Couldn't open a connection to the X11 server!" << endl;
    }
    
    dbglogfile.setVerbosity(1);

    // Make sure that the browser supports functionality we need
    err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
                               NPNVSupportsXEmbedBool,
                               (void *)&supportsXEmbed);

    if (err != NPERR_NO_ERROR || supportsXEmbed != PR_TRUE) {
	log_warning("No xEmbed support in this Mozilla version!");
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    } else {
	dbglogfile << "xEmbed supported in this Mozilla version" << endl;
    }

    err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
                               NPNVToolkit,
                               (void *)&toolkit);
    
    if (err != NPERR_NO_ERROR || toolkit != NPNVGtk2) {
	log_warning("No GTK2 support in this Mozilla version! Have %d",
		    (int)toolkit);
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    } else {
	dbglogfile << "Gtk2+ supported in this Mozilla version" << endl;
    }

//     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)) {
// 	fprintf(stderr, "Unable to init SDL: %s", SDL_GetError());
// 	exit(1);
//     }
    
    plugInitialized = TRUE;

    GNASH_REPORT_RETURN;
    
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
    GNASH_REPORT_FUNCTION;

    if (!plugInitialized) {
	dbglogfile << "Plugin already shut down" << endl;
	return;
    }
    
    if (playerMutex) {
	PR_DestroyLock(playerMutex);
	playerMutex = NULL;
	dbglogfile << "Destroyed Player Mutex" << endl;
    }

    if (glMutex) {
	PR_DestroyLock(glMutex);
	glMutex = NULL;
	dbglogfile << "Destroyed GL Mutex" << endl;
    }

    if (playerCond) {
	PR_DestroyCondVar(playerCond);
	playerCond = NULL;
	dbglogfile << "Destroyed Player condition variable" << endl;
    }

    if (gxDisplay) {
 	XCloseDisplay(gxDisplay);
 	gxDisplay = NULL;
	dbglogfile << "Closed connection to X11 server" << endl;
    }

    GNASH_REPORT_RETURN;
    plugInitialized = FALSE;
}

/// \brief Retrieve values from the plugin for the Browser
///
/// This C function is called by the browser to get certain
/// information is needs from the plugin. This information is the
/// plugin name, a description, etc...
NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
    GNASH_REPORT_FUNCTION;
    
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
      case NPPVpluginNeedsXEmbed:
#ifdef USE_GTK_PLUG
//	  *((PRBool *)aValue) = PR_TRUE;
	  break;
#endif
      case NPPVpluginTimerInterval:
      case NPPVpluginKeepLibraryInMemory:
      default:
          err = NPERR_INVALID_PARAM;
          break;
    }
    GNASH_REPORT_RETURN;
    return err;
}

/// \brief construct our plugin instance object
///
/// This instantiates a new object via a C function used by the
/// browser.
nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
    GNASH_REPORT_FUNCTION;
    
    if(!aCreateDataStruct)
        return NULL;

    nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
    
    GNASH_REPORT_RETURN;
    return plugin;
}

/// \brief destroy our plugin instance object
///
/// This destroys our instantiated object via a C function used by the
/// browser.
void
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
    GNASH_REPORT_FUNCTION;

    if (aPlugin) {
        delete (nsPluginInstance *)aPlugin;
    }
    GNASH_REPORT_RETURN;
}

//
// nsPluginInstance class implementation
//

/// \brief Construct a new nsPluginInstance object
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
                                                    mInstance(aInstance),
                                                    _window(0),
                                                    mXtwidget(0),
                                                    mFontInfo(0),
						    _glxContext(NULL),
						    _shutdown(FALSE),
						    _glInitialized(FALSE),
						    _thread(NULL),
						    _newwin(FALSE)
{
    GNASH_REPORT_FUNCTION;
}

/// \brief Destroy a nsPluginInstance object
nsPluginInstance::~nsPluginInstance()
{
    GNASH_REPORT_FUNCTION;
}

/// \brief Initialize an instance of the plugin object
/// 
/// This methods initializes the plugin object, and is called for
/// every movie that gets played. This is where the movie playing
/// specific initialization goes.
NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);
    
    if(aWindow == NULL) {
	log_msg("%s: ERROR: Window handle was bogus!", __PRETTY_FUNCTION__);
        return FALSE;
    } else {
	log_msg("%s: X origin = %d, Y Origin = %d, Width = %d,"
	       " Height = %d, WindowID = %p, this = %p",
		__FUNCTION__,
	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
	       aWindow->window, this);
    }

    // Only for developers. Make the plugin block here so we can
    // attach GDB to it.
    bool gdb = false;
    while (gdb) {
	dbglogfile << "Waiting for GDB for pid " << getpid() << endl;
	sleep(5);
    }
    
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
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    

    if (_thread) {
	dbglogfile << "Waiting for the thread to terminate..." << endl;
	PRStatus rv = PR_SetThreadPrivate(_thread_key, (void *)"stop");
	_shutdown = TRUE;
// 	PR_Interrupt(_thread);
// 	if (PR_PENDING_INTERRUPT_ERROR == PR_GetError()) {
// 	    dbglogfile << "ERROR: Couldn't interupt thread!" << endl;
// 	}
	
	PR_JoinThread(_thread);
	_thread = NULL;
    }
    destroyContext();
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
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    if(aWindow == NULL) {
	dbglogfile << __FUNCTION__ << ": ERROR: Window handle was bogus!" << endl;
        return FALSE;
    } else {
	log_msg("%s: X origin = %d, Y Origin = %d, Width = %d,"
	       " Height = %d, WindowID = %p, this = %p",
	       __FUNCTION__,
	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
	       aWindow->window, this);
    }    
    
//     if (_glInitialized) {
// 	log_msg("%s Already initialized...", __FUNCTION__);
// 	return TRUE;
//     }    
    
    if (aWindow->x == mX && aWindow->y == mY
	&& aWindow->width == mWidth
	&& aWindow->height == mHeight
	&& (unsigned long)(aWindow->window) == _window) {
	return TRUE;
    }

    lockGL();
    lockX();
    mX = aWindow->x;
    mY = aWindow->y;
    mWidth = aWindow->width;
    mHeight = aWindow->height;
    
    if (_window == (Window) aWindow->window) {
        // The page with the plugin is being resized.
        // Save any UI information because the next time
        // around expect a SetWindow with a new window id.
	dbglogfile << __FUNCTION__ << "Error: Setwindow() called with same window handle - but resizing plugin unhandled!" << endl;
    } else {
        _window = (Window) aWindow->window;
        NPSetWindowCallbackStruct *ws_info =
	    (NPSetWindowCallbackStruct *)aWindow->ws_info;
	mVisual = ws_info->visual;
        mDepth = ws_info->depth;
        mColormap = ws_info->colormap;
//        gxDisplay = ws_info->display;

        if (!mFontInfo) {
            if (!(mFontInfo = XLoadQueryFont(gxDisplay, "9x15"))) {
                dbglogfile << "ERROR: Cannot open 9X15 font!" << endl;
	    }
        }
	
	XVisualInfo *vi = glXChooseVisual(gxDisplay, DefaultScreen(gxDisplay),
					  attributeList_FSAA);
	if (vi == NULL) {
	    vi = glXChooseVisual(gxDisplay, DefaultScreen(gxDisplay),
				 attributeList_noFSAA);
	} else {
	    vi->visual = mVisual;
	}
	
	_glxContext = glXCreateContext(gxDisplay, vi, 0, GL_TRUE);
	if (_glxContext) {
	    dbglogfile << __FUNCTION__ << ": Got new glxContext "
		       << (void *)_glxContext << endl;
	    setGL();
	    initGL();
	    _glInitialized = TRUE;
	} else {
	    dbglogfile << __FUNCTION__ << ": ERROR: Couldn't get new glxContext!" << endl;
	}

        // add xt event handler#
        long event_mask = ExposureMask|KeyPress|KeyRelease|ButtonPress|ButtonRelease;        Widget xtwidget;
	
//         xtwidget =  XtWindowToWidget((Display *) gxdisplay,
//                                      (Window) aWindow->window);
//         XtAddEventHandler(xtwidget, event_mask, FALSE,
//                           (XtEventHandler) xt_event_handler, this);
    }


#ifdef USE_GTK_PLUG_XXXXXXX
    gtkplug = gtk_plug_new(_window);
//     if (_newwin) {
// 	_gtkwidget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//     } else {
// 	_gtkwidget = gtk_window_new(GTK_WINDOW_POPUP);
//     }
    gtk_window_set_title(GTK_WINDOW(gtkplug), "Gnash player");
    
    gtk_widget_add_events(gtkplug, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(gtkplug, GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(GTK_OBJECT(gtkplug), "button_press_event",
		     GTK_SIGNAL_FUNC(fixme_callback), this);
    
    gtk_signal_connect(GTK_OBJECT(gtkplug), "delete_event",
		       GTK_SIGNAL_FUNC(destroy_callback), this);
    
    g_signal_connect(GTK_OBJECT(gtkplug), "expose_event",
		     GTK_SIGNAL_FUNC(fixme_callback), this);
    
    g_signal_connect(GTK_OBJECT(gtkplug), "key_press_event",
		     GTK_SIGNAL_FUNC(fixme_callback), this);
    
    g_signal_connect(GTK_OBJECT(gtkplug), "focus_in_event",
		     GTK_SIGNAL_FUNC(fixme_callback), this);
    
    g_signal_connect(GTK_OBJECT(gtkplug), "focus_out_event",
		     GTK_SIGNAL_FUNC(fixme_callback), this);
    
    g_signal_connect(GTK_OBJECT(gtkplug), "map_event",
		     GTK_SIGNAL_FUNC(fixme_callback), this);

//    gtk_widget_realize(gtkplug);
//     popup_menu = GTK_MENU(gtk_menu_new());
//     menuitem_play =
// 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Play"));
//     gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_play));
//     gtk_widget_show(GTK_WIDGET(menuitem_play));
    
    gtk_widget_show(gtkplug);
//    gtk_widget_set_usize(_gtkwidget,mWidth, mHeight);    
#endif
    resizeWindow(mWidth,mHeight);

    unsetGL();
    freeX();
    freeGL();
    
    return NPERR_NO_ERROR;
}

const char *
nsPluginInstance::getVersion()
{
    GNASH_REPORT_FUNCTION;
    
    return NPN_UserAgent(mInstance);
}

NPError
nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
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
    log_msg("%s", msg);

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
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    char tmp[300];
    memset(tmp, 0, 300);
    string url = stream->url;
    string fname, opts;
    unsigned int start, end, eq;
    bool dumpopts = false;

    log_msg("%s: this = %p, URL is %s", __FUNCTION__,
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

    dbglogfile << __FUNCTION__ << ": The full URL is " << url << endl;
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
	    dbglogfile << __FUNCTION__ << "Option " << name << " = "
		       << value << endl;
	}
	// Look for our special debug flags
	if (name == "debug") {
 	    dbglogfile << __FUNCTION__ << "Debug flag is " << value << endl;
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
    
    //  log_msg("%s: URL is %s", __PRETTY_FUNCTION__, url.c_str());
    log_msg("%s: Open stream for %s, this = %p", __FUNCTION__,
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
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    char tmp[300];
    memset(tmp, 0, 300);
    nsPluginInstance *arg = (nsPluginInstance *)this;
    sprintf(tmp, "Done Flash movie %s", swf_file.c_str());
    WriteStatus(tmp);

    log_msg("%s: this = %p, URL is %s", __PRETTY_FUNCTION__,
	   (void *)arg, stream->url);
    processing = false;

    if (_streamfd) {
        close(_streamfd);
        _streamfd = -1;
    }
    
    // Wait for GDB
    if (waitforgdb) {
	log_msg("Attach GDB to PID %d to debug!", getpid());
	log_msg("This thread will block until then!...");
	log_msg("Once blocked here, you can set other breakpoints.");
	log_msg("do a \"set variable waitforgdb=false\" to continue");
	while (waitforgdb) {
	    sleep(1);
	}
    }

    log_msg("%s: Starting player Thread for this = %p",
	   __PRETTY_FUNCTION__, (void *)this);


//     PRStatus rv;
//     rv = PR_NewThreadPrivateIndex(&_thread_key, Destructor);
//     rv = PR_SetThreadPrivate(_thread_key, (void *)"run");

    // PR_USER_THREAD -	PR_Cleanup blocks until the last thread of
    //			type PR_USER_THREAD terminates.
    // PR_SYSTEM_THREAD - NSPR ignores threads of type
    // PR_SYSTEM_THREAD when determining when a call to PR_Cleanup
    // should return.
    //
    // PR_LOCAL_THREAD - A local thread, scheduled locally by NSPR
    // within the process.
    // PR_GLOBAL_THREAD - A global thread, scheduled by the host OS.
    // PR_GLOBAL_BOUND_THREAD -	A global bound (kernel) thread,
    // scheduled by the host OS 
    _thread = PR_CreateThread(PR_USER_THREAD, playerThread, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);
    
//     PR_Lock(playerMutex);
//     PR_NotifyCondVar(playerCond);

    sprintf(tmp, "Started thread for Flash movie %s", swf_file.c_str());
    WriteStatus(tmp);

    return NPERR_NO_ERROR;
}

void
nsPluginInstance::URLNotify(const char *url, NPReason reason,
                            void *notifyData)
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    log_msg("URL: %s\nReason %i", url, reason);
}

/// \brief Return how many bytes we can read into the buffer
int32
nsPluginInstance::WriteReady(NPStream * stream)
{
//   log_msg("%s(%d): Entering", __PRETTY_FUNCTION__, __LINE__);
//   log_msg("Stream for %s is ready", stream->url);

    return INBUFSIZE;
}

/// \brief Read the data stream from Mozilla/Firefox
///
/// For now we read the bytes and write them to a disk file.
int32
nsPluginInstance::Write(NPStream * stream, int32 offset, int32 len,
                        void *buffer)
{
//   log_msg("%s(%d): Entering", __PRETTY_FUNCTION__, __LINE__);
//   log_msg("Reading Stream %s, offset is %d, length = %d",
//          stream->url, offset, len);

    return write(_streamfd, buffer, len);
}

/// \brief Initialize OpenGL
///
void
nsPluginInstance::initGL()
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    if (_glInitialized) {
	dbglogfile << __FUNCTION__ << ": OpenGL already initialized..."  << endl;
	return;
    }

    // Grab control of the display
//    lockDisplay();
    
    dbglogfile << __FUNCTION__ << ": Initializing OpenGL..." << endl;

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

    if (_glxContext) {
	_glInitialized = TRUE;
    }
	
    // Release control of the display
//    freeDisplay();
}

/// \brief Shutdown OpenGL
void
nsPluginInstance::destroyContext()
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    

    if (!_glInitialized) {
	dbglogfile << __FUNCTION__ << ": OpenGL already killed..." << endl;
	return;
    }

    if (gxDisplay && _glxContext) {
	// Grab control of the display
//	lockDisplay();
 	lockGL();
 	lockX();
	setGL();
	
	dbglogfile << __FUNCTION__ << ": Destroying GLX Context "
		   << (void *)_glxContext << endl;
	glXDestroyContext(gxDisplay, _glxContext);
	_glxContext = NULL;

//	freeDisplay();
	// Release control of the display
 	unsetGL();
 	freeX();
 	freeGL();
    }
    _glInitialized = FALSE;
}

/// \brief Resize our viewport after a window resize event
int
nsPluginInstance::resizeWindow( int width, int height )
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    

    log_msg("%s: Width = %d, Height = %d",  __FUNCTION__, width, height);

    if (!plugInitialized || !_glxContext) {
	dbglogfile << __FUNCTION__ << ": OpenGL not initialized..." << endl;
	return true;
    }

    // Grab control of the display
//     lockGL();
//     lockX();
//     setGL();
    
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
//     unsetGL();
//     freeX();
//     freeGL();

    return(true);
}

/// \brief Draw a hardcoded image
///
/// This draws a hardcoded OpenGL graphic into the window, and is only
/// used for testing by developers.
void
nsPluginInstance::drawTestScene( void )
{
    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    

    // Grab control of the display
    lockDisplay();
    
    dbglogfile << __FUNCTION__ << ": Drawing graphic..." << endl;

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
    glEnd();                   // Done Drawing The Quad

    swapBuffers();
    
    // Release control of the display
    freeDisplay();
//    SDL_mutexP(mutant);
}

#if 0
/// \brief Handle X events
///
/// This C function handles events from X, like keyboard events, or
/// Expose events that we're interested in.
static void
xt_event_handler(Widget xtwidget, nsPluginInstance *plugin,
		 XEvent *xevent, Boolean *b)
{
    GNASH_REPORT_FUNCTION;

    int        keycode;
    KeySym     keysym;
#if 0
    SDL_Event  sdl_event;
    SDL_keysym sdl_keysym;

    //    handleKeyPress((SDL_keysym)keysym);
    log_msg("Peep Event returned %d", SDL_PeepEvents(&sdl_event, 1, SDL_PEEKEVENT, SDL_USEREVENT|SDL_ACTIVEEVENT|SDL_KEYDOWN|SDL_KEYUP|SDL_MOUSEBUTTONUP|SDL_MOUSEBUTTONDOWN));
  
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
// 		  log_msg("Drawing GL Scene for expose event!");
// 	      } else {
 		  log_msg("GL Surface not initialized yet, ignoring expose event!");
// 	      }
          }
          break;
      case ButtonPress:
//     fe.type = FeButtonPress;
          log_msg("Button Press");
          break;
      case ButtonRelease:
          //     fe.type = FeButtonRelease;
          log_msg("Button Release");
          break;
      case KeyPress:
          keycode = xevent->xkey.keycode;
		plugin->lockX();
          keysym = XLookupKeysym((XKeyEvent*)xevent, 0);
          log_msg ("%s(%d): Keysym is %s", __PRETTY_FUNCTION__, __LINE__,
                  XKeysymToString(keysym));
		plugin->freeX();

          switch (keysym) {
            case XK_Up:
                log_msg("Key Up");
                break;
            case XK_Down:
                log_msg("Key Down");
                break;
            case XK_Left:
                log_msg("Key Left");
                break;
            case XK_Right:
                log_msg("Key Right");
                break;
            case XK_Return:
                log_msg("Key Return");
                break;
      
            default:
                break;
          }
    }
}
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
