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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "plugin.h"
#define MIME_TYPES_HANDLED  "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME     "Shockwave Flash"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":swf:"PLUGIN_NAME
// PLUGIN_DESCRIPTION is inline in a function below, since it got very
// long, including copyright info and URLs and such.
#define PLUGIN_DESCRIPTION  SEE-BELOW-SEARCH-FOR-PLUGIN_DESCRIPTION

#include <sys/param.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Sunkeysym.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <string>

#include "tu_file.h"
#include "tu_types.h"
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

bool processing;

extern NPNetscapeFuncs NPNFuncs;

NPBool      plugInitialized = FALSE;
PRLock      *playerMutex = NULL;
PRCondVar   *playerCond = NULL;


// static int   streamfd = -1;
// static float s_scale = 1.0f;
// static bool  s_verbose = false;
// static int   doneYet = 0;
static bool  waitforgdb = false;

const int INBUFSIZE = 1024;

#ifdef HAVE_LIBXML
extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

void
PR_CALLBACK Destructor(void *data)
{
//    GNASH_REPORT_FUNCTION;

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
//    GNASH_REPORT_FUNCTION;

    NPError err = NPERR_NO_ERROR;
    PRBool supportsXEmbed = PR_TRUE;
    NPNToolkitType toolkit;

#ifndef USE_FORK
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
#endif // end of USE_FORK

    dbglogfile.setVerbosity(2);

    // Make sure that the browser supports functionality we need
    err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
                               NPNVSupportsXEmbedBool,
                               (void *)&supportsXEmbed);

    if (err != NPERR_NO_ERROR || !supportsXEmbed) {
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

    plugInitialized = TRUE;

//    GNASH_REPORT_RETURN;
    
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
//    GNASH_REPORT_FUNCTION;

    if (!plugInitialized) {
	dbglogfile << "Plugin already shut down" << endl;
	return;
    }

#ifndef USE_FORK
    if (playerMutex) {
	PR_DestroyLock(playerMutex);
	playerMutex = NULL;
	dbglogfile << "Destroyed Player Mutex" << endl;
    }

    if (playerCond) {
	PR_DestroyCondVar(playerCond);
	playerCond = NULL;
	dbglogfile << "Destroyed Player condition variable" << endl;
    }
#endif // end of USE_FORK

//    GNASH_REPORT_RETURN;
    plugInitialized = FALSE;
}

// HTML description of Gnash, for display in URL "about:plugins" in the browser.
// PLUGIN_DESCRIPTION used to feed in here, but now it's just literal.
static const char description[] = 
"Shockwave Flash 8.0 - Gnash " VERSION ", the GNU Flash Player.  "
"Copyright &copy; 2006 "
"<a href=\"http://www.fsf.org\">Free Software Foundation</a>, Inc.<br>"
"Gnash comes with NO WARRANTY, to the extent permitted by law.  "
"You may redistribute copies of Gnash under the terms of the "
"<a href=\"http://www.gnu.org/licenses/gpl.html\">GNU "
"General Public License</a>, with an additional special exception allowing "
"linking with Mozilla, or any variant of Mozilla (such as Firefox), "
"so long as the linking is "
"through its standard plug-in interface.  For more information about Gnash, "
"see <a href=\"http://www.gnu.org/software/gnash/\">"
"http://www.gnu.org/software/gnash</a>."
	    ;

/// \brief Retrieve values from the plugin for the Browser
///
/// This C function is called by the browser to get certain
/// information is needs from the plugin. This information is the
/// plugin name, a description, etc...
NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
//    GNASH_REPORT_FUNCTION;
    
    NPError err = NPERR_NO_ERROR;
    
    switch (aVariable) {
      case NPPVpluginNameString:
          *((char **)aValue) = PLUGIN_NAME;
          break;

      // This becomes the description field you see below the opening
      // text when you type about:plugins
      case NPPVpluginDescriptionString:
          *((char **)aValue) = (char *)description;
          break;

      case NPPVpluginNeedsXEmbed:
#ifdef HAVE_GTK2
	  *((PRBool *)aValue) = PR_TRUE;
#else
	  *((PRBool *)aValue) = PR_FALSE;
#endif
	  break;
      case NPPVpluginTimerInterval:
      case NPPVpluginKeepLibraryInMemory:
      default:
          err = NPERR_INVALID_PARAM;
          break;
    }
//    GNASH_REPORT_RETURN;
    return err;
}

/// \brief construct our plugin instance object
///
/// This instantiates a new object via a C function used by the
/// browser.
nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
//    GNASH_REPORT_FUNCTION;
    
    if(!aCreateDataStruct)
        return NULL;

    nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
    
//    GNASH_REPORT_RETURN;
    return plugin;
}

/// \brief destroy our plugin instance object
///
/// This destroys our instantiated object via a C function used by the
/// browser.
void
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
//    GNASH_REPORT_FUNCTION;

    if (aPlugin) {
        delete (nsPluginInstance *)aPlugin;
    }
//    GNASH_REPORT_RETURN;
}

//
// nsPluginInstance class implementation
//

/// \brief Construct a new nsPluginInstance object
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
                                                    mInstance(aInstance),
                                                    _window(0),
                                                    mXtwidget(0),
						    _shutdown(FALSE),
						    _thread(NULL),
						    _thread_key(0),
						    _childpid(0)
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Destroy a nsPluginInstance object
nsPluginInstance::~nsPluginInstance()
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Initialize an instance of the plugin object
/// 
/// This methods initializes the plugin object, and is called for
/// every movie that gets played. This is where the movie playing
/// specific initialization goes.
NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);
    
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
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    

#ifndef USE_FORK
    if (_thread) {
	dbglogfile << "Waiting for the thread to terminate..." << endl;
//	PRStatus rv = PR_SetThreadPrivate(_thread_key, (void *)"stop");
	_shutdown = TRUE;
// 	PR_Interrupt(_thread);
// 	if (PR_PENDING_INTERRUPT_ERROR == PR_GetError()) {
// 	    dbglogfile << "ERROR: Couldn't interupt thread!" << endl;
// 	}
	
	PR_JoinThread(_thread);
	_thread = NULL;
    }
// end of USE_FORK
#endif
    if (_childpid) {
	kill(_childpid, SIGINT);
    }
    
    _childpid = 0;
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
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    if(aWindow == NULL) {
	dbglogfile << __FUNCTION__ << ": ERROR: Window handle was bogus!" << endl;
        return FALSE;
//     } else {
// 	log_msg("%s: X origin = %d, Y Origin = %d, Width = %d,"
// 	       " Height = %d, WindowID = %p, this = %p",
// 	       __FUNCTION__,
// 	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
// 	       aWindow->window, this);
    }
    
    if (aWindow->x == mX && aWindow->y == mY
	&& aWindow->width == mWidth
	&& aWindow->height == mHeight
	&& (unsigned long)(aWindow->window) == _window) {
	return TRUE;
    }

    mX = aWindow->x;
    mY = aWindow->y;
    mWidth = aWindow->width;
    mHeight = aWindow->height;
    
    if (_window == (Window) aWindow->window) {
        // The page with the plugin is being resized.
        // Save any UI information because the next time
        // around expect a SetWindow with a new window id.
//	dbglogfile << __FUNCTION__ << "Error: Setwindow() called with same window handle - but resizing plugin unhandled!" << endl;
    } else {
        _window = (Window) aWindow->window;
        NPSetWindowCallbackStruct *ws_info =
	    (NPSetWindowCallbackStruct *)aWindow->ws_info;
        mDepth = ws_info->depth;
        mColormap = ws_info->colormap;
    }

    resizeWindow(mWidth,mHeight);

    return NPERR_NO_ERROR;
}

const char *
nsPluginInstance::getVersion()
{
//    GNASH_REPORT_FUNCTION;
    
    return NPN_UserAgent(mInstance);
}

NPError
nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
    return NS_PluginGetValue(aVariable, aValue) ;
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
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    int len = strlen(stream->url)+1;
    char tmp[len];
    memset(tmp, 0, len);
    string url = stream->url;
    string fname, opts;
    size_t start, end, eq;
    bool dumpopts = false;

//     log_msg("%s: this = %p, URL is %s", __FUNCTION__,
// 	   (void *)this, stream->url);

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

    string name;
    string value;
    
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
	// Some URLs we can't parse for odd reasons, so this prevents
	// Firefox from crashing in those cases.
 	if (end == string::npos) {
	    dbglogfile << "ERROR: Can't parse URL!" << endl;
	    return NPERR_INVALID_PARAM;
 	} else {
	    name = opts.substr(start, eq);
	    value = opts.substr(eq+1, end-eq-1);
	}
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
	if ((opts.size() > end) && (opts[end] == '&')) {
		end++;
	}
	if (end != string::npos) {
	    opts.erase(start, end);
	}
    }
    
    //  log_msg("%s: URL is %s", __PRETTY_FUNCTION__, url.c_str());
//     log_msg("%s: Open stream for %s, this = %p", __FUNCTION__,
// 	   fname.c_str(), (void *)this);

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

    _swf_file = fname;
    processing = true;

    return NPERR_NO_ERROR;
}

/// \brief Destroy the data stream we've been reading.
NPError
nsPluginInstance::DestroyStream(NPStream * stream, NPError reason)
{
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
//    nsPluginInstance *arg = (nsPluginInstance *)this;
    char tmp[300];
    memset(tmp, 0, 300);
    sprintf(tmp, "Done downloading Flash movie %s", _swf_file.c_str());
    WriteStatus(tmp);

//     log_msg("%s: this = %p, URL is %s", __PRETTY_FUNCTION__,
// 	   (void *)arg, stream->url);
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

#ifndef USE_FORK
    _thread = PR_CreateThread(PR_USER_THREAD, playerThread, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);
#else
    Window window = _window;
    _childpid = startProc(_swf_file.c_str(), window);
#endif // !USE_FORK

//     sprintf(tmp, "Started thread for Flash movie %s", _swf_file.c_str());
//     WriteStatus(tmp);

    return NPERR_NO_ERROR;
}

void
nsPluginInstance::URLNotify(const char *url, NPReason reason,
                            void *notifyData)
{
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    
    
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

/// \brief Resize our viewport after a window resize event
int
nsPluginInstance::resizeWindow( int width, int height )
{
//    log_trace("%s: enter for instance %p", __PRETTY_FUNCTION__, this);    

    log_msg("%s: Width = %d, Height = %d",  __FUNCTION__, width, height);

    return(true);
}

int
nsPluginInstance::startProc(string filespec)
{
    return startProc(filespec, _window);
}

int
nsPluginInstance::startProc(string filespec, Window win)
{
//    GNASH_REPORT_FUNCTION;
    
    struct stat procstats;
    int ret = 0;
    
    char *gnash_env = getenv("GNASH_PLAYER");
    if (!gnash_env) {
	_procname = PREFIX;
	_procname += "/bin/gnash"; 
    } else {
	_procname = gnash_env; 
    }
    
    // See if the file actually exists, otherwise we can't spawn it
    if (stat(_procname.c_str(), &procstats) == -1) {
        dbglogfile << "Invalid filename: " << _procname << endl;
        return -1;
    }
    
    _childpid = fork();
    // childpid is -1, if the fork failed, so print out an error message
    if (_childpid == -1) {
        perror(strerror(errno));
        return -1;
    }
    // childpid is a positive integer, if we are the parent, and
    // fork() worked
    if (_childpid > 0) {
        dbglogfile << "Forked sucessfully, child process PID is " << _childpid << endl;
        return _childpid;
    }
        
    // setup the command line
    char num[30];
    memset(num, 0, 30);
    sprintf(num, "%ld", win);
    char num2[30];
    memset(num2, 0, 30);
    sprintf(num2, "%d", mWidth);
    char num3[30];
    memset(num3, 0, 30);
    sprintf(num3, "%d", mHeight);
    
//     cmd_line[0] = new char(procname.size()+1);
//     strcpy(cmd_line[0], procname.c_str());
//     cmd_line[1] = new char(50);
//     sprintf(cmd_line[1], "-x %d", (int)win);
//     cmd_line[2] = new char(50);
//     sprintf(cmd_line[2], "-v");
//     cmd_line[3] = new char(filespec.size()+1);
//     sprintf(cmd_line[3], "%s", filespec.c_str());
    // This option tells the child process to wait for GDB to connect.

    // This option tells the child process to wait for GDB to connect.
    if (waitforgdb) {
//         cmd_line[4] = new char(4);
//         strcpy(cmd_line[4], "-s");
    }
    char *argv[] = {
	(char *)_procname.c_str(),
	"-x", num,
	(char *)filespec.c_str(),
	"-j", num2,
	"-k", num3,
	0
    };
    
    // If we are the child, exec the new process, then go away
    if (_childpid == 0) {
        // Start the desired executable
        dbglogfile << "Starting " << _procname << " with -x "
		   << (int)win << " " << filespec << endl;
	ret = execv(argv[0], argv);
        perror(strerror(ret));
        exit(ret);
    }
    
    return _childpid;
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
