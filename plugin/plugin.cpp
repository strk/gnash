// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include <cstdlib> // getenv
#include <stdlib.h> // putenv

#define MIME_TYPES_HANDLED  "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME    "Shockwave Flash"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":swf:"PLUGIN_NAME

// Some javascript plugin detectors use the description
// to decide the flash version to display. They expect the
// form (major version).(minor version) r(revision).
// e.g. "8.0 r99."
#define FLASH_VERSION DEFAULT_FLASH_MAJOR_VERSION"."\
    DEFAULT_FLASH_MINOR_VERSION" r"DEFAULT_FLASH_REV_NUMBER"."

#define PLUGIN_DESCRIPTION \
  "Shockwave Flash "FLASH_VERSION" Gnash "VERSION", the GNU SWF Player. \
  Copyright (C) 2006, 2007, 2008, 2009, 2010 <a href=\"http://www.fsf.org\">Free \
  Software Foundation</a>, Inc. <br> \
  Gnash comes with NO WARRANTY, to the extent permitted by law. \
  You may redistribute copies of Gnash under the terms of the \
  <a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public \
  License</a>. For more information about Gnash, see <a \
  href=\"http://www.gnu.org/software/gnash/\"> \
  http://www.gnu.org/software/gnash</a>. \
  Compatible Shockwave Flash "FLASH_VERSION

// Define the following to make the plugin verbose
// WARNING: will write to .xsession_errors !
// Values:
//  1: fatal errors (errors preventing the plugin from working as it should)
//  2: informational messages
//
#define GNASH_PLUGIN_DEBUG 2
//#define WRITE_FILE

#include "plugin.h" 
#include "GnashSystemIOHeaders.h"
#include "StringPredicates.h"

#include <boost/tokenizer.hpp>
#include <sys/param.h>
#include <csignal>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>
#include <climits>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

// Mozilla SDK headers
#include "prinit.h"
#include "prlock.h"
#include "prcvar.h"
#include "prerr.h"
#include "prerror.h"
#include "prthread.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

extern NPNetscapeFuncs NPNFuncs;

NPBool plugInitialized = FALSE;

static bool waitforgdb = false;
static bool createSaLauncher = false;

static const char* getPluginDescription();
static void logDebug(const std::string& msg);
static void logError(const std::string& msg);


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
    return const_cast<char *>(MIME_TYPES_DESCRIPTION);
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
    if ( plugInitialized ) {
        logDebug("NS_PluginInitialize called, but ignored (we already initialized)");
        return NPERR_NO_ERROR;
    }

    logDebug("NS_PluginInitialize call ---------------------------------------------------");

    /* Browser Functionality Checks */

    NPError err = NPERR_NO_ERROR;
    PRBool supportsXEmbed = PR_TRUE;

    /* 
    First, check for XEmbed support. The NPAPI Gnash plugin
    only works with XEmbed, so tell the plugin API to fail if
    XEmbed is not found.
    */    
    
    err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
                NPNVSupportsXEmbedBool,
                (void *)&supportsXEmbed);


    if (err != NPERR_NO_ERROR || !supportsXEmbed) {
        logError("NPAPI ERROR: No xEmbed support in this browser!");
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    } else {
        logDebug("xEmbed supported in this browser");
    }

#if 0 // Gtk is no longer required in the browser
    NPNToolkitType toolkit;
    err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
                NPNVToolkit,
                (void *)&toolkit);

    /*
    GTK2 support is currently also necessary. Fail if not
    present.
    */
    if (err != NPERR_NO_ERROR || toolkit != NPNVGtk2) {
#ifdef GNASH_PLUGIN_DEBUG
        std::cout << "NPAPI ERROR: No GTK2 support in this browser!"
            " Have version " << (int)toolkit << std::endl;
#endif

        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    } else {
#if GNASH_PLUGIN_DEBUG > 1
        std::cout << "GTK2 supported in this browser" << std::endl;
#endif
    }
#endif

    /*
    Check for environment variables.
    */
    char* opts = std::getenv("GNASH_OPTIONS");
    if (opts != NULL) {
        logDebug(std::string("GNASH_OPTIONS : ") + std::string(opts));
        
        // Should the plugin wait for gdb to be attached?
        if ( strstr(opts, "waitforgdb") ) {
            waitforgdb = true;
        }

        // Should the plugin write a script to invoke
        // the standalone player for debugging ?
        if ( strstr(opts, "writelauncher") ) {
            createSaLauncher = true;
        }

    }

    // Append SYSCONFDIR/gnashpluginrc and ~/.gnashpluginrc to GNASHRC

    std::string newGnashRc("GNASHRC=");

    newGnashRc.append(SYSCONFDIR);
    newGnashRc.append("/gnashpluginrc");

    char *home = std::getenv("HOME");
    if ( home ) {
        newGnashRc.append(":");
        newGnashRc.append(home);
        newGnashRc.append("/.gnashpluginrc");
    } else {
        std::cout << "WARNING: NPAPI plugin could not find user home dir" << std::endl;
    }

    char *gnashrc = std::getenv("GNASHRC");
    if ( gnashrc ) {
        newGnashRc.append(":");
        newGnashRc.append(gnashrc);
    }

    // putenv doesn't copy the string in standards-conforming implementations
    gnashrc = new char[PATH_MAX];
    std::strncpy(gnashrc, newGnashRc.c_str(), PATH_MAX);
    gnashrc[PATH_MAX-1] = '\0';

    if ( putenv(gnashrc) ) {
        std::cout << "WARNING: NPAPI plugin could not append to the GNASHRC env variable" << std::endl;
    }
    else logDebug(std::string("NOTE: NPAPI plugin set GNASHRC to ") + newGnashRc);

    /* Success */

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
#if 0
    if (!plugInitialized) {
#if GNASH_PLUGIN_DEBUG > 1
        std::cout << "Plugin already shut down" << std::endl;
#endif
        return;
    }

    plugInitialized = FALSE;
#endif
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
            *static_cast<const char **> (aValue) = PLUGIN_NAME;
            break;

        // This becomes the description field you see below the opening
        // text when you type about:plugins and in
        // navigator.plugins["Shockwave Flash"].description, used in
        // many flash version detection scripts.
        case NPPVpluginDescriptionString:
            *static_cast<const char **>(aValue) =
                        getPluginDescription();
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
    if(!aCreateDataStruct) return NULL;

    return new nsPluginInstance(aCreateDataStruct);
}

/// \brief destroy our plugin instance object
///
/// This destroys our instantiated object via a C++ function used by the
/// browser.
void
NS_DestroyPluginInstance(nsPluginInstanceBase* aPlugin)
{
    delete static_cast<nsPluginInstance *> (aPlugin);
}

//
// nsPluginInstance class implementation
//

/// \brief Constructor
nsPluginInstance::nsPluginInstance(nsPluginCreateData* data)
    :
    nsPluginInstanceBase(),
    _instance(data->instance),
    _window(0),
    _width(0),
    _height(0),
    _streamfd(-1),
    _ichan(0),
    _ichanWatchId(0),
    _controlfd(-1),
    _childpid(0),
    _filefd(-1),
    _name()
{
    for (size_t i=0, n=data->argc; i<n; ++i)
    {
        std::string name, val;
        gnash::StringNoCaseEqual noCaseCompare;

        if (data->argn[i]) {
            name = data->argn[i];
        }

        if (data->argv[i]) {
            val = data->argv[i];
        }

        if (noCaseCompare(name, "name")) {
            _name = val;
        }

        _params[name] = val;
    }

}

/// \brief Destructor
nsPluginInstance::~nsPluginInstance()
{
    logDebug("plugin instance destruction");

    if (_ichan) {
        logDebug("shutting down input chan");

        GError *error = NULL;
        g_io_channel_shutdown (_ichan, TRUE, &error);
        g_io_channel_unref (_ichan);
        _ichan = 0;
    }

    if ( _ichanWatchId ) {
        g_source_remove(_ichanWatchId);
        _ichanWatchId = 0;
    }

    if (_childpid > 0) {
        // When the child has terminated (signaled by _controlfd), it remains
        // as a defunct process and we remove it from the kernel table now.
        int status;
        waitpid(_childpid, &status, 0);
        logDebug("Child process exited with status " + status);
    }
    _childpid = 0;
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
        logError(std::string(__PRETTY_FUNCTION__) + " ERROR: Window handle was bogus!");
        return FALSE;
    } else {
#if GNASH_PLUGIN_DEBUG > 1
        std::cout << "X origin: = " << aWindow->x 
            << ", Y Origin = " << aWindow->y
            << ", Width = " << aWindow->width
            << ", Height = " << aWindow->height
            << ", WindowID = " << aWindow->window
            << ", this = " << static_cast<void*>(this) << std::endl;
#endif
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
    logDebug("Gnash plugin shutting down");

    int ret = close(_controlfd);
    if (ret != 0) {
        logDebug("Gnash plugin failed to close the control socket!");
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
    if(!aWindow) {
        logError(std::string(__FUNCTION__) + ": ERROR: Window handle was bogus!");
        return NPERR_INVALID_PARAM;
#if 0
    } else {
        log_debug("%s: X origin = %d, Y Origin = %d, Width = %d,"
            " Height = %d, WindowID = %p, this = %p",
            __FUNCTION__,
            aWindow->x, aWindow->y, aWindow->width, aWindow->height,
            aWindow->window, this);
#endif
    }

    if (_window) {
        return NPERR_GENERIC_ERROR;
    }

    _width = aWindow->width;
    _height = aWindow->height;

    _window = reinterpret_cast<Window> (aWindow->window);

    if (!_childpid && !_swf_url.empty()) {
        startProc();
    }

    return NPERR_NO_ERROR;
}


NPError
nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
    return NS_PluginGetValue(aVariable, aValue);
}

/// \brief Write a status message
///
/// This writes a status message to the status line at the bottom of
/// the browser window and the console firefox was started from.
NPError
nsPluginInstance::WriteStatus(char *msg) const
{
    NPN_Status(_instance, msg);
    std::cout << msg << std::endl;

    return NPERR_NO_ERROR;
}

NPError
nsPluginInstance::WriteStatus(std::string msg) const
{
    return WriteStatus(const_cast<char*>(msg.c_str()));
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
nsPluginInstance::NewStream(NPMIMEType /*type*/, NPStream* stream,
                            NPBool /*seekable*/, uint16_t* /*stype*/)
{
    if (_childpid) {
        // Apparently the child process has already been started for this
        // plugin instance. It is puzzling that this method gets called
        // again. Starting a new process for the same movie will cause
        // problems later, so we'll stop here.
        return NPERR_GENERIC_ERROR;
    }
    _swf_url = stream->url;

#if GNASH_PLUGIN_DEBUG > 1
    std::cout << __FUNCTION__ << ": The full URL is " << _swf_url << std::endl;
#endif

#ifdef WRITE_FILE
    size_t start, end;
    std::string fname;
    end   = _swf_url.find(".swf", 0) + 4;
    start = _swf_url.rfind("/", end) + 1;
    fname = "/tmp/";
    fname += _swf_url.substr(start, end - start);

    logDebug("The Flash movie name is: " + fname);

    _filefd = open(fname.c_str(),
            O_CREAT | O_WRONLY,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    
    if (_filefd < 0) {
        _filefd = open(fname.c_str(),
                O_TRUNC | O_WRONLY,
                S_IRUSR | S_IRGRP | S_IROTH);
    }
#endif
    
    if (!_swf_url.empty() && _window) {
        startProc();
    }

    return NPERR_NO_ERROR;
}

/// \brief Destroy the data stream we've been reading.
NPError
nsPluginInstance::DestroyStream(NPStream* /*stream*/, NPError /*reason*/)
{

    if (_streamfd != -1) {
        if (close(_streamfd) == -1) {
            perror("closing _streamfd");
        } else {
            _streamfd = -1;
        }
    }

#ifdef WRITE_FILE
    if (_filefd != -1) {
        if (close(_filefd) == -1) {
            perror("closing _filefd");
        } else {
            _filefd = -1;
        }
    }
#endif

    return NPERR_NO_ERROR;
}

/// \brief Return how many bytes we can read into the buffer
int32_t
nsPluginInstance::WriteReady(NPStream* /* stream */ )
{
#if GNASH_PLUGIN_DEBUG > 1
    //std::cout << "Stream for " << stream->url << " is ready" << std::endl;
#endif
    if ( _streamfd != -1 ) {
	return 1024;
    } else {
	return 0;
    }
}

/// \brief Read the data stream from Mozilla/Firefox
///
/// For now we read the bytes and write them to a disk file.
int32_t
nsPluginInstance::Write(NPStream* /*stream*/, int32_t /*offset*/, int32_t len,
        void* buffer)
{
#ifdef WRITE_FILE
    write(_filefd, buffer, len);
#endif
    int written = write(_streamfd, buffer, len);
    return written;
}

bool
nsPluginInstance::handlePlayerRequestsWrapper(GIOChannel* iochan,
        GIOCondition cond, nsPluginInstance* plugin)
{
    return plugin->handlePlayerRequests(iochan, cond);
}

bool
nsPluginInstance::handlePlayerRequests(GIOChannel* iochan, GIOCondition cond)
{
    if ( cond & G_IO_HUP ) {
        logDebug("Player request channel hang up");
        // Returning false here will cause the "watch" to be removed. This watch
        // is the only reference held to the GIOChannel, so it will be
        // destroyed. We must make sure we don't attempt to destroy it again.
        _ichan = 0;
        _ichanWatchId = 0;
        return false;
    }

    assert(cond & G_IO_IN);

#if GNASH_PLUGIN_DEBUG > 1
    int inputfd = g_io_channel_unix_get_fd(iochan);
    std::cout << "Checking player requests on fd " << inputfd << std::endl;
#endif

    do {
        GError* error=NULL;
        gchar* request;
        gsize requestSize=0;
        GIOStatus status = g_io_channel_read_line(iochan, &request,
                &requestSize, NULL, &error);

        switch ( status ) {
            case G_IO_STATUS_ERROR:
                logError(std::string("Error reading request line: ") + error->message);

                g_error_free(error);
                return false;
            case G_IO_STATUS_EOF:
                logError(std::string("EOF (error: ") + error->message);
                return false;
            case G_IO_STATUS_AGAIN:
                logError(std::string("Read again(error: ") + error->message);
                break;
            case G_IO_STATUS_NORMAL:
                // process request
                logDebug("Normal read: " + std::string(request));
                break;
            default:
                logError("Abnormal status!");
                return false;
            
        }

        // process request..
        processPlayerRequest(request, requestSize);
        g_free(request);

    } while (g_io_channel_get_buffer_condition(iochan) & G_IO_IN);

    return true;

}

bool
nsPluginInstance::processPlayerRequest(gchar* buf, gsize linelen)
{
    if ( linelen < 4 ) {
        logError(std::string("Invalid player request (too short): ") +  buf);
        return false;
    }

    if ( ! std::strncmp(buf, "GET ", 4) ) {
        char* target = buf + 4;
        if ( ! *target )
        {
            logError("No target found after GET request");
            return false;
        }
        char* url = target;
        while (*url && *url != ':') ++url;
        if ( *url ) {
            *url='\0';
            ++url;
        } else {
            logError("No colon found after GETURL target string");
            return false;
        }

#if GNASH_PLUGIN_DEBUG > 1
        std::cout << "Asked to get URL '" << url << "' in target '" 
            << target << "'" << std::endl;
#endif
        NPN_GetURL(_instance, url, target);
        return true;

    } else if ( ! std::strncmp(buf, "INVOKE ", 7) ) {
        char* command = buf + 7;
        if ( ! *command ) {
            logError("No command found after INVOKE request");
            return false;
        }
        char* arg = command;
        while (*arg && *arg != ':') ++arg;
        if ( *arg ) {
            *arg='\0';
            ++arg;
        } else {
            logError("No colon found after INVOKE command string");
            return false;
        }

        std::string name = _name; 

        std::stringstream jsurl;
        jsurl << "javascript:" << name << "_DoFSCommand('" << command << "','" << arg <<"')";

        // TODO: check if _self is a good target for this
        static const char* tgt = "_self";

        logDebug("Calling NPN_GetURL(" + jsurl.str() + ", '" + std::string(tgt) + "');");

        NPN_GetURL(_instance, jsurl.str().c_str(), tgt);
        return true;
    }  else if ( ! strncmp(buf, "POST ", 5)) {
        char* target = buf + 5;
        if (! *target) return false;
        
        char* postdata = target;
        while (*postdata && *postdata != ':') ++postdata;
        if ( *postdata ) {
            *postdata='\0';
            ++postdata;
        }
        else
        {
            logError("No colon found after getURL postdata string");
            return false;
        }
        
        char* url = postdata;
        while (*url && *url != '$') ++url;
        if (*url) {
            *url='\0';
            ++url;
        } else {
            logError("No $ character found after getURL target string");
            return false;
        }
        

        NPN_PostURL(_instance, url, target, std::strlen(postdata),
                postdata, false);

        return true;
    } else {
        logError("Unknown player request: " + std::string(buf));
        return false;
    }
}

void
logDebug(const std::string& msg)
{
#if GNASH_PLUGIN_DEBUG > 1
    std::cout << msg << std::endl;
#else
    (void) msg; // suppress warning
#endif
}

void
logError(const std::string& msg)
{
#ifdef GNASH_PLUGIN_DEBUG
    std::cout << msg << std::endl;
#endif
}

std::string
getGnashExecutable()
{
    std::string procname;
    bool process_found = false;
    struct stat procstats;

    char *gnash_env = std::getenv("GNASH_PLAYER");

    if (gnash_env) {
        procname = gnash_env;
        process_found = (0 == stat(procname.c_str(), &procstats));
        if (!process_found) {
            logError("Invalid path to gnash executable: ");
            return "";
        }
    }

    if (!process_found) {
        procname = GNASHBINDIR "/gtk-gnash";
        process_found = (0 == stat(procname.c_str(), &procstats));
    }
    if (!process_found) {
        procname = GNASHBINDIR "/kde4-gnash";
        process_found = (0 == stat(procname.c_str(), &procstats));
    }

    if (!process_found) {
        logError(std::string("Unable to find Gnash in ") + GNASHBINDIR);
        return "";
    }


    return procname;
}

void
create_standalone_launcher(const std::string& page_url, const std::string& swf_url)
{
#ifdef CREATE_STANDALONE_GNASH_LAUNCHER
    if (!createSaLauncher) {
        return;
    }

    std::ofstream saLauncher;

    std::stringstream ss;
    static int debugno = 0;
    debugno = (debugno + 1) % 10;
    ss << "/tmp/gnash-debug-" << debugno << ".sh";
    saLauncher.open(ss.str().c_str(), std::ios::out | std::ios::trunc);

    if (!saLauncher) {
        logError("Failed to open new file for standalone launcher: " + ss.str());
        return;
    }

    saLauncher << "#!/bin/sh" << std::endl
               << getGnashExecutable() << " ";

    if (!page_url.empty()) {
        saLauncher << "-U '" << page_url << "' ";
    }

    saLauncher << "'" << swf_url << "' "
               << "$@"       // allow caller to pass any additional argument
               << std::endl;

    saLauncher.close();
#endif
}

std::vector<std::string>
nsPluginInstance::getCmdLine(int hostfd, int controlfd)
{
    std::vector<std::string> arg_vec;

    std::string cmd = getGnashExecutable();
    if (cmd.empty()) {
        logError("Failed to locate the Gnash executable!");
        return arg_vec;
    }
    arg_vec.push_back(cmd);

    arg_vec.push_back("-u");
    arg_vec.push_back(_swf_url);

    const char* pageurl = getCurrentPageURL();
    if (!pageurl) {
        logError("Could not get current page URL!");
    } else {
        arg_vec.push_back("-U");
        arg_vec.push_back(pageurl);
    }

    std::stringstream pars;
    pars << "-x "  <<  _window           // X window ID to render into
         << " -j " << _width             // Width of window
         << " -k " << _height            // Height of window
         << " -F " << hostfd             // Socket to send commands to
         << " -G " << controlfd;         // Socket determining lifespan
    {
        std::string pars_str = pars.str();
        typedef boost::char_separator<char> char_sep;
        boost::tokenizer<char_sep> tok(pars_str, char_sep(" "));
        arg_vec.insert(arg_vec.end(), tok.begin(), tok.end());
    }

    for (std::map<std::string,std::string>::const_iterator it = _params.begin(),
        itEnd = _params.end(); it != itEnd; ++it) {
        const std::string& nam = it->first; 
        const std::string& val = it->second;
        arg_vec.push_back("-P");
        arg_vec.push_back(nam + "=" + val);
    }
    arg_vec.push_back("-");

    create_standalone_launcher(pageurl, _swf_url);

    return arg_vec;
}

void
nsPluginInstance::startProc()
{
    // 0 For reading, 1 for writing.
    int p2c_pipe[2];
    int c2p_pipe[2];
    int p2c_controlpipe[2];
    
    int ret = pipe(p2c_pipe);
    if (ret == -1) {
        logError("ERROR: parent to child pipe() failed: " +
                 std::string(std::strerror(errno)));
    }
    _streamfd = p2c_pipe[1];

    ret = pipe(c2p_pipe);
    if (ret == -1) {
        logError("ERROR: child to parent pipe() failed: " +
                 std::string(std::strerror(errno)));
    }

    ret = pipe(p2c_controlpipe);
    if (ret == -1) {
        logError("ERROR: parent to child pipe() failed: " +
                 std::string(std::strerror(errno)));
    }

    _controlfd = p2c_controlpipe[1];

    /*
    Setup the command line for starting Gnash
    */

    std::vector<std::string> arg_vec = getCmdLine(c2p_pipe[1], p2c_controlpipe[0]);
    if (arg_vec.empty()) {
        logError("Failed to obtain command line parameters.");
        return;
    }

    std::vector<const char*> args;

    std::transform(arg_vec.begin(), arg_vec.end(), std::back_inserter(args),
                   std::mem_fun_ref(&std::string::c_str));
    args.push_back(0);

    /*
      Argument List prepared, now fork(), close file descriptors and execv()
     */

    _childpid = fork();

    // If the fork failed, childpid is -1. So print out an error message.
    if (_childpid == -1) {
        logError("ERROR: dup2() failed: " + std::string(strerror(errno)));
        return;
    }

    // If we are the parent and fork() worked, childpid is a positive integer.
    if (_childpid > 0) {
        
        // we want to write to p2c pipe, so close read-fd0
        ret = close (p2c_pipe[0]);
        if (ret == -1) {
// this is not really a fatal error...
            logError("ERROR: p2c_pipe[0] close() failed: " +
                     std::string(strerror(errno)));
        }

        // we want to read from c2p pipe, so close read-fd1
        ret = close (c2p_pipe[1]);
        if (ret == -1)
        {
// this is not really a fatal error...
            logError("ERROR: c2p_pipe[1] close() failed: " + 
                     std::string(strerror(errno)));
        }

        ret = close (p2c_controlpipe[0]); // close read descriptor
    
#if GNASH_PLUGIN_DEBUG > 1
        std::cout << "Forked successfully, child process PID is " 
                                << _childpid
                                << std::endl;
#endif

        _ichan = g_io_channel_unix_new(c2p_pipe[0]);
        g_io_channel_set_close_on_unref(_ichan, true);
        _ichanWatchId = g_io_add_watch(_ichan, 
                (GIOCondition)(G_IO_IN|G_IO_HUP), 
                (GIOFunc)handlePlayerRequestsWrapper, this);
        g_io_channel_unref(_ichan);
        return;
    }

    // This is the child scope.
    //FF3 uses jemalloc and it has problems after the fork(), do NOT
    //use memory functions (malloc()/free()/new/delete) after the fork()
    //in the child thread process

    // We want to read parent to child, so close write-fd1
    ret = close (p2c_pipe[1]); 
    if (ret == -1) {
// not really a fatal error
        logError("ERROR: close() failed: " + std::string(strerror(errno)));
    }

    ret = close(p2c_controlpipe[1]);

    // close standard input and direct read-fd1 to standard input
    ret = dup2 (p2c_pipe[0], fileno(stdin));
    
    if (ret == -1) {
        logError("ERROR: dup2() failed: " + std::string(strerror(errno)));
    }

    // Close all of the browser's file descriptors that we just 
    // inherited (including p2c_pipe[0] that we just dup'd to fd 0).
    // Experiments show seventy or eighty file descriptors open in
    // typical cases.  Rather than close all the thousands of possible file
    // descriptors, we start after stderr and keep closing higher numbers
    // until we encounter ten fd's in a row that
    // aren't open. This will tend to close most fd's in most programs.
    int numfailed = 0, closed = 0;
    int anfd = fileno(stderr)+1;
    for ( ; numfailed < 10; anfd++) {
        if ( anfd == c2p_pipe[1] ) continue; // don't close this
        if ( anfd == c2p_pipe[0] ) continue; // don't close this either (correct?)
        if ( anfd == p2c_controlpipe[0] ) continue; // don't close this either (correct?)
        if ( anfd == p2c_controlpipe[1] ) continue; // don't close this either (correct?)
        ret = close (anfd);
        if (ret < 0) {
	    numfailed++;
	} else {
            numfailed = 0;
            closed++;
        }
    }

#if GNASH_PLUGIN_DEBUG > 1
    std::cout << "Closed " << closed << " files." << std::endl;
#endif


    /*
    Start the desired executable and go away.
    */

    
#if GNASH_PLUGIN_DEBUG > 1
    std::cout << "Starting process: ";
    for (int i = 0; args[i] != 0; ++i) {
        std::cout << args[i] << " ";
    }
    std::cout << std::endl;
#endif

    /*
    For debugging the plugin (GNASH_OPTIONS=waitforgdb)
    Block here until gdb is attached and sets waitforgdb to
    false.
    */

    if (waitforgdb) {

        std::cout << std::endl << "  Attach GDB to PID " << getpid()
                << " to debug!" << std::endl;
        std::cout << "  This thread will block until then!" << std::endl;
        std::cout << "  Once blocked here, you can set other breakpoints."
                << std::endl;
        std::cout << "  Do a \"set variable waitforgdb=$false\" to continue"
                << std::endl << std::endl;
        
        while (waitforgdb)
        {
            sleep(1);
        }
    }

    execv(args[0], const_cast<char**>(&args.front()));

    // if execv returns, an error has occurred.
    perror("executing standalone gnash");
    
    exit (-1);
}

const char*
nsPluginInstance::getCurrentPageURL() const
{
    NPP npp = _instance;

    NPIdentifier sDocument = NPN_GetStringIdentifier("document");

    NPObject *window;
    NPN_GetValue(npp, NPNVWindowNPObject, &window);

    NPVariant vDoc;
    NPN_GetProperty(npp, window, sDocument, &vDoc);
    NPN_ReleaseObject(window);

    if (!NPVARIANT_IS_OBJECT(vDoc)) {
        logError("Can't get window object");
        return NULL;
    }
    
    NPObject* npDoc = NPVARIANT_TO_OBJECT(vDoc);

    NPIdentifier sLocation = NPN_GetStringIdentifier("location");
    NPVariant vLoc;
    NPN_GetProperty(npp, npDoc, sLocation, &vLoc);
    NPN_ReleaseObject(npDoc);

    if (!NPVARIANT_IS_OBJECT(vLoc)) {
        logError("Can't get window.location object");
        return NULL;
    }

    NPObject* npLoc = NPVARIANT_TO_OBJECT(vLoc);

    NPIdentifier sProperty = NPN_GetStringIdentifier("href");
    NPVariant vProp;
    NPN_GetProperty(npp, npLoc, sProperty, &vProp);
    NPN_ReleaseObject(npLoc);

    if (!NPVARIANT_IS_STRING(vProp)) {
        logError("Can't get window.location.href object");
        return NULL;
    }

    const NPString& propValue = NPVARIANT_TO_STRING(vProp);

    return propValue.utf8characters; // const char *
}

static const char*
getPluginDescription() 
{
    static const char* desc = NULL;
    if (!desc) {
        desc = std::getenv("GNASH_PLUGIN_DESCRIPTION");
        if (desc == NULL) desc = PLUGIN_DESCRIPTION;
    }
    return desc;
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
