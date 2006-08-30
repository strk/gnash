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
#include "log.h"
#include "tu_types.h"

#define MIME_TYPES_HANDLED  "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME     "Shockwave Flash"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":swf:"PLUGIN_NAME

#define PLUGIN_DESCRIPTION \
  "Shockwave Flash 8.0 - Gnash " VERSION ", the GNU Flash Player. Copyright   \
  &copy; 2006 <a href=\"http://www.fsf.org\">Free Software Foundation</a>,    \
  Inc.<br> Gnash comes with NO WARRANTY, to the extent permitted by law.  You \
  may redistribute copies of Gnash under the terms of the                     \
  <a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public License \
  </a>, with an additional special exception allowing linking with Mozilla,   \
  or any variant of Mozilla (such as Firefox), so long as the linking is      \
  through its standard plug-in interface.  For more information about Gnash,  \
  see <a href=\"http://www.gnu.org/software/gnash/\">                         \
  http://www.gnu.org/software/gnash</a>."


#include <sys/param.h>
#include <csignal>
#include <unistd.h>
#include <cstdio>
#include <cstddef>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <string>

// Mozilla SDK headers
#include "prinit.h"
#include "prlock.h"
#include "prcvar.h"
#include "prerr.h"
#include "prerror.h"
#include "prthread.h"

using namespace std;
using namespace gnash;

extern NPNetscapeFuncs NPNFuncs;

NPBool      plugInitialized = FALSE;

static bool  waitforgdb = false;

void
PR_CALLBACK Destructor(void * /* data */)
{
#if 0
    /*
     * We don't actually free the storage since it's actually allocated
     * on the stack. Normally, this would not be the case and this is
     * the opportunity to free whatever.
     */
    PR_Free(data);
#endif
}

/// \brief Return the MIME Type description for this plugin.
char*
NPP_GetMIMEDescription(void)
{
    return MIME_TYPES_DESCRIPTION;
}

//
// general initialization and shutdown
//

/// \brief Initialize the plugin
///
/// This C++ function gets called once when the plugin is loaded,
/// regardless of how many instantiations there is actually playing
/// movies. So this is where all the one time only initialization
/// stuff goes.
NPError
NS_PluginInitialize()
{
    NPError err = NPERR_NO_ERROR;
    PRBool supportsXEmbed = PR_TRUE;
    NPNToolkitType toolkit;

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

    return NPERR_NO_ERROR;
}

/// \brief Shutdown the plugin
///
/// This C++ function gets called once when the plugin is being
/// shutdown, regardless of how many instantiations actually are
/// playing movies. So this is where all the one time only
/// shutdown stuff goes.
void
NS_PluginShutdown()
{
    if (!plugInitialized) {
	dbglogfile << "Plugin already shut down" << endl;
	return;
    }

    plugInitialized = FALSE;
}



/// \brief Retrieve values from the plugin for the Browser
///
/// This C++ function is called by the browser to get certain
/// information is needs from the plugin. This information is the
/// plugin name, a description, etc...
NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
    NPError err = NPERR_NO_ERROR;

    switch (aVariable) {
      case NPPVpluginNameString:
          *static_cast<char **> (aValue) = PLUGIN_NAME;
          break;

      // This becomes the description field you see below the opening
      // text when you type about:plugins
      case NPPVpluginDescriptionString:
          *static_cast<char **>(aValue) = PLUGIN_DESCRIPTION;
          break;

      case NPPVpluginNeedsXEmbed:
#ifdef HAVE_GTK2
	  *static_cast<PRBool *>(aValue) = PR_TRUE;
#else
	  *static_cast<PRBool *>(aValue) = PR_FALSE;
#endif
	  break;
      case NPPVpluginTimerInterval:
      case NPPVpluginKeepLibraryInMemory:
      default:
          err = NPERR_INVALID_PARAM;
          break;
    }
    return err;
}

/// \brief construct our plugin instance object
///
/// This instantiates a new object via a C++ function used by the
/// browser.
nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
    if(!aCreateDataStruct)
      return NULL;

    return new nsPluginInstance(aCreateDataStruct->instance);
}

/// \brief destroy our plugin instance object
///
/// This destroys our instantiated object via a C++ function used by the
/// browser.
void
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
    delete static_cast<nsPluginInstance *> (aPlugin);
}

//
// nsPluginInstance class implementation
//

/// \brief Constructor
nsPluginInstance::nsPluginInstance(NPP aInstance)
  : nsPluginInstanceBase(),
    _instance(aInstance),
    _window(0),
    _childpid(0)
{
}

/// \brief Destructor
nsPluginInstance::~nsPluginInstance()
{
}

/// \brief Initialize an instance of the plugin object
/// 
/// This methods initializes the plugin object, and is called for
/// every movie that gets played. This is where the movie playing
/// specific initialization goes.
NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
    if(!aWindow) {
	log_msg("%s: ERROR: Window handle was bogus!", __PRETTY_FUNCTION__);
        return FALSE;
    } else {
	log_msg("%s: X origin = %d, Y Origin = %d, Width = %d,"
	       " Height = %d, WindowID = %p, this = %p",
		__FUNCTION__,
	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
	       aWindow->window, static_cast<void*>(this));
    }

#if 0
    // Only for developers. Make the plugin block here so we can
    // attach GDB to it.

    bool gdb = true;
    while (gdb) {
	dbglogfile << "Waiting for GDB for pid " << getpid() << endl;
	sleep(5);
    }
#endif
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
    if(!aWindow) {
	dbglogfile << __FUNCTION__ << ": ERROR: Window handle was bogus!" << endl;
        return NPERR_INVALID_PARAM;
#if 0
    } else {
 	log_msg("%s: X origin = %d, Y Origin = %d, Width = %d,"
 	       " Height = %d, WindowID = %p, this = %p",
 	       __FUNCTION__,
 	       aWindow->x, aWindow->y, aWindow->width, aWindow->height,
 	       aWindow->window, this);
#endif
    }

    _width = aWindow->width;
    _height = aWindow->height;

    _window = reinterpret_cast<Window> (aWindow->window);

    return NPERR_NO_ERROR;
}


NPError
nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
    return NS_PluginGetValue(aVariable, aValue) ;
}

/// \brief Write a status message
///
/// This writes a status message to the status line at the bottom of
/// the browser window and the console firefox was started from.
NPError
nsPluginInstance::WriteStatus(char *msg) const
{
    NPN_Status(_instance, msg);
    log_msg("%s", msg);

    return NPERR_NO_ERROR;
}

NPError
nsPluginInstance::WriteStatus(string msg) const
{
  return WriteStatus( const_cast<char*>(msg.c_str()) );
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
nsPluginInstance::NewStream(NPMIMEType /* type */, NPStream * stream,
                            NPBool /* seekable */, uint16_t * /* stype */)
{
    string url = stream->url;
    string fname, opts;
    size_t start, end, eq;
    bool dumpopts = false;

#if 0
    log_msg("%s: this = %p, URL is %s", __FUNCTION__,
      (void *)this, stream->url);
#endif

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
	start = 0; // TODO: An empty name seems useless to me. If this is 
	           //       indeed the case, we should set `start' to one.

	eq = opts.find("=", start);
	if (eq == string::npos) {
	    dbglogfile << "INFO: Ignoring URL appendix without name." << endl;
	    goto process;
	} 

 	if (opts[0] == '&') {
	    // A (technically invalid) URL like movie.swf?&option=value.
	    start++;
	}

	end = opts.find("&", start);
 	if (end == string::npos) {
	    // We have only one name=value pair remaining.
	    end = opts.size();
 	}
	
	name = opts.substr(start, eq);
	value = opts.substr(eq+1, end-eq-1);
	
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

#if 0
    log_msg("%s: URL is %s", __PRETTY_FUNCTION__, url.c_str());
    log_msg("%s: Open stream for %s, this = %p", __FUNCTION__,
    fname.c_str(), (void *)this);
#endif

process:
    WriteStatus("Loading Flash movie " + fname);

    _streamfd = open(fname.c_str(), O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if (_streamfd < 0) {
        WriteStatus(fname + " can't be opened, check your permissions!\n");
        _streamfd = open(fname.c_str(), O_TRUNC | O_WRONLY, S_IRUSR|S_IRGRP|S_IROTH);
        if (_streamfd < 0) {
            WriteStatus(fname + " can't be created, check your permissions!\n");
        }
    }

    _swf_file = fname;

    return NPERR_NO_ERROR;
}

/// \brief Destroy the data stream we've been reading.
NPError
nsPluginInstance::DestroyStream(NPStream * /* stream */, NPError /* reason */)
{
    WriteStatus("Finished downloading Flash movie " + _swf_file +
                ". Playing...");

#if 0
    nsPluginInstance *arg = (nsPluginInstance *)this;
    log_msg("%s: this = %p, URL is %s", __PRETTY_FUNCTION__,
      (void *)arg, stream->url);
#endif

    if (_streamfd != -1) {
        if (close(_streamfd) == -1) {
            perror(strerror(errno));
        } else {
            _streamfd = -1;
        }
    }

    if (waitforgdb) {
	log_msg("Attach GDB to PID %d to debug!", getpid());
	log_msg("This thread will block until then!...");
	log_msg("Once blocked here, you can set other breakpoints.");
	log_msg("do a \"set variable waitforgdb=false\" to continue");
	while (waitforgdb) {
	    sleep(1);
	}
    }

    _childpid = startProc(_swf_file.c_str(), _window);

    return NPERR_NO_ERROR;
}

/// \brief Return how many bytes we can read into the buffer
int32
nsPluginInstance::WriteReady(NPStream * /* stream */ )
{
#if 0
    log_msg("Stream for %s is ready", stream->url);
#endif
    return 1024;
}

/// \brief Read the data stream from Mozilla/Firefox
///
/// For now we read the bytes and write them to a disk file.
int32
nsPluginInstance::Write(NPStream * /* stream */, int32 /* offset */, int32 len,
                        void * buffer)
{
#if 0
    log_msg("Reading Stream %s, offset is %d, length = %d",
      stream->url, offset, len);
#endif
    return write(_streamfd, buffer, len);
}

int
nsPluginInstance::startProc(string filespec, Window win)
{
    string procname;
    char *gnash_env = getenv("GNASH_PLAYER");
    if (!gnash_env) {
      procname = PREFIX;
      procname += "/bin/gnash";
    } else {
      procname = gnash_env;
    }

    struct stat procstats;

    // See if the file actually exists, otherwise we can't spawn it
    if (stat(procname.c_str(), &procstats) == -1) {
        dbglogfile << "Invalid filename: " << procname << endl;
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

    // We are the child

    // setup the command line
    const size_t buf_size = 30;
    char xid[buf_size], width[buf_size], height[buf_size];
    snprintf(xid, buf_size, "%ld", win);
    snprintf(width, buf_size, "%d", _width);
    snprintf(height, buf_size, "%d", _height);

    char * const argv[] = {
      const_cast<char*>( procname.c_str() ),
      "-x", xid,
      "-j", width,
      "-k", height,
      const_cast<char*>( filespec.c_str() ),
      0
    };

    // Start the desired executable and go away
    dbglogfile << "Starting process: ";

    for (int i=0; argv[i] != 0; ++i) {
      dbglogfile << argv[i] << " ";
    }
    dbglogfile << endl;

    execv(argv[0], argv);
    // if execv returns, an error has occurred.
    perror(strerror(errno));

    exit (-1);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
