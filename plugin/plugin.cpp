// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#define MIME_TYPES_HANDLED  "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME	"Shockwave Flash"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":swf:"PLUGIN_NAME

// Some javascript plugin detectors use the description
// to decide the flash version to display. They expect the
// form (major version).(minor version) r(revision).
// e.g. "8.0 r99."
#define FLASH_VERSION DEFAULT_FLASH_MAJOR_VERSION"."\
	DEFAULT_FLASH_MINOR_VERSION" r"DEFAULT_FLASH_REV_NUMBER"."

#define PLUGIN_DESCRIPTION \
  "Shockwave Flash "FLASH_VERSION" Gnash "VERSION", the GNU SWF Player. \
  Copyright &copy; 2006, 2007, 2008 <a href=\"http://www.fsf.org\">Free Software \
  Foundation</a>, Inc. <br> \
  Gnash comes with NO WARRANTY, to the extent permitted by law. \
  You may redistribute copies of Gnash under the terms of the \
  <a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public \
  License</a>. For more information about Gnash, see <a \
  href=\"http://www.gnu.org/software/gnash/\"> \
  http://www.gnu.org/software/gnash</a>. \
  Compatible Shockwave Flash "FLASH_VERSION

#include <sys/param.h>
#include "plugin.h" //Fixes Warning on redef of MIN/MAX
#include <csignal>
#include <unistd.h>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

// Mozilla SDK headers
#include "prinit.h"
#include "prlock.h"
#include "prcvar.h"
#include "prerr.h"
#include "prerror.h"
#include "prthread.h"

using namespace std;

extern NPNetscapeFuncs NPNFuncs;

NPBool plugInitialized = FALSE;

static bool waitforgdb = false;

static const char* getPluginDescription();

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
	if ( plugInitialized )
	{
		cout << "NS_PluginInitialize called, but ignored (we already initialized)" << endl;
		return NPERR_NO_ERROR;
	}

	cout << "NS_PluginInitialize call ---------------------------------------------------" << endl;


	/* Browser Functionality Checks */

	NPError err = NPERR_NO_ERROR;
	PRBool supportsXEmbed = PR_TRUE;
	NPNToolkitType toolkit;

	/* 
	First, check for XEmbed support. The NPAPI Gnash plugin
	only works with XEmbed, so tell the plugin API to fail if
	XEmbed is not found.
	*/	
	
	err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
				NPNVSupportsXEmbedBool,
				(void *)&supportsXEmbed);


	if (err != NPERR_NO_ERROR || !supportsXEmbed)
	{
		cout << "NPAPI ERROR: No xEmbed support in this browser!"
							<< endl;
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	else
	{
		cout << "xEmbed supported in this browser" << endl;
	}

	err = CallNPN_GetValueProc(NPNFuncs.getvalue, NULL,
				NPNVToolkit,
				(void *)&toolkit);

	/*
	GTK2 support is currently also necessary. Fail if not
	present.
	*/
	if (err != NPERR_NO_ERROR || toolkit != NPNVGtk2)
	{
		cout << "NPAPI ERROR: No GTK2 support in this browser!"
			" Have version " << (int)toolkit << endl;

		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	else
	{
		cout << "GTK2 supported in this browser" << endl;
	}

	/*
	Check for environment variables.
	*/
	char* opts = std::getenv("GNASH_OPTIONS");
	if (opts != NULL)
	{
		cout << "GNASH_OPTIONS : " << opts << endl;
		
		// Should the plugin wait for gdb to be attached?
		if ( strstr(opts, "waitforgdb") )
		{
			waitforgdb = true;
		}

	}

	// Append SYSCONFDIR/gnashpluginrc and ~/.gnashpluginrc to GNASHRC
	do {
		// TODO: extract content in a set, add to set
		//       and serialize back (to avoid duplicates)

		std::string newGnashRc;
		char *gnashrc = std::getenv("GNASHRC");
		if ( gnashrc )
		{
			newGnashRc.assign(gnashrc);
			newGnashRc.append(":");
		}

		newGnashRc.append(SYSCONFDIR);
		newGnashRc.append("/gnashpluginrc");

		char *home = std::getenv("HOME");
		if ( home )
		{
			newGnashRc.append(":");
			newGnashRc.append(home);
			newGnashRc.append("/.gnashpluginrc");
		}
		else
		{
			cerr << "WARNING: NPAPI plugin could not find user home dir" << endl;
		}

		if ( setenv("GNASHRC", newGnashRc.c_str(), 1) )
		{
			cerr << "WARNING: NPAPI plugin could not append to the GNASHRC env variable" << endl;
		}
		else cout << "NOTE: NPAPI plugin set GNASHRC to " << newGnashRc << endl;

	} while (0);


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
	if (!plugInitialized)
	{
		cout << "Plugin already shut down" << endl;
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

	switch (aVariable)
	{
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
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
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
	_childpid(0),
	_filefd(-1),
	_name()
{
	for (size_t i=0, n=data->argc; i<n; ++i)
	{
		string name, val;

		if (data->argn[i])
		{
			name = data->argn[i];
		}

		if (data->argv[i])
		{
			val = data->argv[i];
		}

		if ( ! strcasecmp(name.c_str(), "name") )
		{
			_name = val;
		}

		cerr << "PARAM: " << name << " = " << val << endl;
		_params[name] = val;
	}

}

/// \brief Destructor
nsPluginInstance::~nsPluginInstance()
{
	cout << "plugin instance destruction" << endl;
	if ( _ichan )
	{
		cout << "shutting down input chan " << _ichan << endl;
		GError *error = NULL;
		g_io_channel_shutdown (_ichan, TRUE, &error);
		g_io_channel_unref (_ichan);
	}
	if ( _ichanWatchId )
	{
		g_source_remove(_ichanWatchId);
	}
}

/// \brief Initialize an instance of the plugin object
/// 
/// This methods initializes the plugin object, and is called for
/// every movie that gets played. This is where the movie playing
/// specific initialization goes.
NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
	if(!aWindow)
	{
		cout <<  __PRETTY_FUNCTION__ << " ERROR: Window handle was bogus!" << endl;
		return FALSE;
	}
	else
	{
		cout << "X origin: = " << aWindow->x 
			<< ", Y Origin = " << aWindow->y
			<< ", Width = " << aWindow->width
			<< ", Height = " << aWindow->height
			<< ", WindowID = " << aWindow->window
			<< ", this = " << static_cast<void*>(this) << endl;
	}

#if 0
    // Only for developers. Make the plugin block here so we can
    // attach GDB to it.

    bool gdb = true;
    while (gdb) {
	cout << "Waiting for GDB for pid " << getpid() << endl;
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
	cout << "Shutting down" << endl;

	if (_childpid > 0)
	{
		// it seems that waiting after a SIGINT hangs firefox
		// IFF not run from the console (see bug#17082).
		// SIGTERM instead solves this problem
		kill(_childpid, SIGTERM);
		int status;
		waitpid(_childpid, &status, 0);
		cout << "Child process exited with status " << status << endl;	
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
	if(!aWindow)
	{
		cout << __FUNCTION__ << ": ERROR: Window handle was bogus!" << endl;
		return NPERR_INVALID_PARAM;
#if 0
	}
	else
	{
		log_debug("%s: X origin = %d, Y Origin = %d, Width = %d,"
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
	cout << msg << endl;

	return NPERR_NO_ERROR;
}

NPError
nsPluginInstance::WriteStatus(std::string msg) const
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
	_swf_url = stream->url;

	cout << __FUNCTION__ << ": The full URL is " << _swf_url << endl;

#ifdef WRITE_FILE
	size_t start, end;
	string fname;
	end   = _swf_url.find(".swf", 0) + 4;
	start = _swf_url.rfind("/", end) + 1;
	fname = "/tmp/";
	fname += _swf_url.substr(start, end - start);
	cout << "The Flash movie name is: " << fname << endl;

	_filefd = open(fname.c_str(), O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    
	if (_filefd < 0)
	{
		_filefd = open(fname.c_str(), O_TRUNC | O_WRONLY, S_IRUSR|S_IRGRP|S_IROTH);
	}
#endif
    
	startProc(_window);

	return NPERR_NO_ERROR;
}

/// \brief Destroy the data stream we've been reading.
NPError
nsPluginInstance::DestroyStream(NPStream * /* stream */, NPError /* reason */)
{

#if 0
    nsPluginInstance *arg = (nsPluginInstance *)this;
    log_debug("%s: this = %p, URL is %s", __PRETTY_FUNCTION__,
      (void *)arg, stream->url);
#endif

    if (_streamfd != -1)
    {
        if (close(_streamfd) == -1)
        {
            perror("closing _streamfd");
        }
        else
        {
            _streamfd = -1;
        }
    }

#ifdef WRITE_FILE
    if (_filefd != -1)
    {
        if (close(_filefd) == -1)
        {
            perror("closing _filefd");
        }
        else
        {
            _filefd = -1;
        }
    }
#endif

    return NPERR_NO_ERROR;
}

/// \brief Return how many bytes we can read into the buffer
int32_t
nsPluginInstance::WriteReady(NPStream * /* stream */ )
{
#if 0
	cout << "Stream for " << stream->url << " is ready" << endl;
#endif
	if ( _streamfd != -1 ) return 1024;
	else return 0;
}

/// \brief Read the data stream from Mozilla/Firefox
///
/// For now we read the bytes and write them to a disk file.
int32_t
nsPluginInstance::Write(NPStream * /* stream */, int32_t /* offset */, int32_t len,
				void * buffer)
{

#if 0
	log_debug("Reading Stream %s, offset is %d, length = %d",
	stream->url, offset, len);
#endif

#ifdef WRITE_FILE
	write(_filefd, buffer, len);
#endif
	return write(_streamfd, buffer, len);
}

bool
nsPluginInstance::handlePlayerRequestsWrapper(GIOChannel* iochan, GIOCondition cond, nsPluginInstance* plugin)
{
	return plugin->handlePlayerRequests(iochan, cond);
}

bool
nsPluginInstance::handlePlayerRequests(GIOChannel* iochan, GIOCondition cond)
{
	if ( cond & G_IO_HUP )
	{
		cout << "Player request channel hang up" << endl;
		g_source_remove(_ichanWatchId);
		return false;
	}

	assert(cond & G_IO_IN);
	int inputfd = g_io_channel_unix_get_fd(iochan);

	cout << "Checking player requests on fd " << inputfd << endl;

	do
	{
		GError* error=NULL;
		gchar* request;
		gsize requestSize=0;
		GIOStatus status = g_io_channel_read_line(iochan, &request, &requestSize, NULL, &error);
		switch ( status )
		{
			case G_IO_STATUS_ERROR:
				cout << "Error reading request line: " << error->message << endl; 
				g_error_free(error);
				return false;
			case G_IO_STATUS_EOF:
				cout << "EOF (error:" << error << ")" << endl;
				return false;
			case G_IO_STATUS_AGAIN:
				cout << "Read again (error:" << error << ")" << endl;
				break;
			case G_IO_STATUS_NORMAL:
				// process request
				cout << "Normal read: " << request << " (error:" << error << ")" << endl;
				break;
			default:
				cout << "Abnormal status " << status << "  (error:" << error << ")" << endl;
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
	if ( linelen < 4 )
	{
		cout << "Invalid player request (too short): " << buf << endl;
		return false;
	}

	if ( ! strncmp(buf, "GET ", 4) )
	{
		char* target = buf+4;
		if ( ! *target )
		{
			cout << "No target found after GET request" << endl;
			return false;
		}
		char* url = target;
		while (*url && *url != ':') ++url;
		if ( *url )
		{
			*url='\0';
			++url;
		}
		else
		{
			cout << "No colon found after GETURL target string" << endl;
			return false;
		}

		cout << "Asked to get URL '" << url << "' in target '" << target << "'" << endl;
		NPN_GetURL(_instance, url, target);
		return true;

	}
	else if ( ! strncmp(buf, "INVOKE ", 7) )
	{
		char* command = buf+7;
		if ( ! *command ) {
			cout << "No command found after INVOKE request" << endl;
			return false;
		}
		char* arg = command;
		while (*arg && *arg != ':') ++arg;
		if ( *arg ) {
			*arg='\0';
			++arg;
		} else {
			cout << "No colon found after INVOKE command string" << endl;
			return false;
		}

		std::string name = _name; 

		std::stringstream jsurl;
		jsurl << "javascript:" << name << "_DoFSCommand('" << command << "','" << arg <<"')";

		// TODO: check if _self is a good target for this
		static const char* tgt = "_self";

		cout << "Calling NPN_GetURL(" << jsurl.str() << ", '" << tgt << "');" << endl;
		NPN_GetURL(_instance, jsurl.str().c_str(), tgt);
		return true;
	}
	else
	{
		cout << "Unknown player request: '" << buf << "'" << endl;
		return false;
	}
}

void
nsPluginInstance::startProc(Window win)
{
	string procname;
	char *gnash_env = std::getenv("GNASH_PLAYER");
	if (gnash_env == NULL) {
		procname = GNASHBINDIR;
		procname += "/gtk-gnash";
	}
	else
	{
		procname = gnash_env;
	}

	const char* pageurl = getCurrentPageURL();
	if (!pageurl)
	{
		cout << "Could not get current page URL!" << endl;
	}

	struct stat procstats;

	// See if the file actually exists, otherwise we can't spawn it
	if (stat(procname.c_str(), &procstats) == -1)
	{
		cout << "Invalid path to standalone executable: " << procname << endl;
		return;
	}

	// 0 For reading, 1 for writing.
	int p2c_pipe[2];
	int c2p_pipe[2];
	
	int ret = pipe(p2c_pipe);
	if (ret == -1)
	{
		cout << "ERROR: parent to child pipe() failed: " << strerror(errno) << endl;
	}
	_streamfd = p2c_pipe[1];

	ret = pipe(c2p_pipe);
	if (ret == -1)
	{
		cout << "ERROR: child to parent pipe() failed: " << strerror(errno) << endl;
	}


	/*
	Setup the command line for starting Gnash
	*/

	// Prepare width, height and window ID variables
	std::ostringstream xid, width, height, hostfd;

	xid << win;
	width << _width;
	height << _height;
	hostfd << c2p_pipe[1];

	// Prepare Actionscript variables (e.g. Flashvars).
	vector<string> paramvalues;
	paramvalues.reserve(_params.size());

	for (map<string,string>::const_iterator it = _params.begin(),
		itEnd = _params.end();
		it != itEnd; ++it
		)
	{
		const string& nam = it->first; 
		const string& val = it->second;

		string param = nam;
		param += string("=");
		param += val;
		paramvalues.push_back(param);
	}

	/*
	We pass the necessary arguments to the gnash executable for
	it to run as a plugin. We do not specify rendering flags so
	they can be set in gnashrc. Gnash defaults to -r3 anyway.
	
	REMEMBER TO INCREMENT THE maxargc COUNT IF YOU
	ADD NEW ARGUMENTS
	*/ 

	const size_t maxargc = 18 + paramvalues.size() * 2;
	const char **argv = new const char *[maxargc];

	size_t argc = 0;
	argv[argc++] = procname.c_str();
	
	// Don't force verbosity, use configuration for that
	//argv[argc++] = "-v";
	
	// X window ID (necessary for gnash to function as a plugin)
	argv[argc++] = "-x";
	argv[argc++] = xid.str().c_str();
	
	// Height and width
	argv[argc++] = "-j";
	argv[argc++] = width.str().c_str();
	argv[argc++] = "-k";
	argv[argc++] = height.str().c_str();
	
	// Url of the root movie
	argv[argc++] = "-u";
	argv[argc++] = _swf_url.c_str();

	// Host FD
	argv[argc++] = "-F";
	argv[argc++] = hostfd.str().c_str();

	// Base URL is the page that the SWF is embedded in. It is 
	// by Gnash for resolving relative URLs in the movie. If the
	// embed tag "base" is specified, its value overrides the -U
	// flag later (Player.cpp).
	if ( pageurl )
	{
		argv[argc++] = "-U";
		argv[argc++] = pageurl;
	}

	// Variables for use by Actionscript.
	for ( size_t i = 0, n = paramvalues.size(); i < n; ++i)
	{
		argv[argc++] = "-P";
		argv[argc++] = paramvalues[i].c_str();
	}

	argv[argc++] = "-";
	argv[argc++] = 0;

	assert(argc <= maxargc);

	/*
	  Argument List prepared, now fork(), close file descriptors and execv()
	 */

	_childpid = fork();

	// If the fork failed, childpid is -1. So print out an error message.
	if (_childpid == -1)
	{
		cout << "ERROR: dup2() failed: " << strerror(errno) << endl;
		return;
	}

	// If we are the parent and fork() worked, childpid is a positive integer.
	if (_childpid > 0)
	{
		delete[] argv;//don't need the argument list
		
		// we want to write to p2c pipe, so close read-fd0
		ret = close (p2c_pipe[0]);
		if (ret == -1)
		{
			cout << "ERROR: p2c_pipe[0] close() failed: " << strerror(errno)
								<< endl;
		}

		// we want to read from c2p pipe, so close read-fd1
		ret = close (c2p_pipe[1]);
		if (ret == -1)
		{
			cout << "ERROR: c2p_pipe[1] close() failed: " << strerror(errno)
								<< endl;
		}
	

		cout << "Forked successfully, child process PID is " 
								<< _childpid
								<< endl;

		_ichan = g_io_channel_unix_new(c2p_pipe[0]);
		g_io_channel_set_close_on_unref(_ichan, true);
		_ichanWatchId = g_io_add_watch(_ichan, (GIOCondition)(G_IO_IN|G_IO_HUP), (GIOFunc)handlePlayerRequestsWrapper, this);

		return;
	}

	// This is the child scope.
	//FF3 uses jemalloc and it has problems after the fork(), do NOT
	//use memory functions (malloc()/free()/new/delete) after the fork()
	//in the child thread process

	// We want to read parent to child, so close write-fd1
	ret = close (p2c_pipe[1]); 
	if (ret == -1)
	{
		cout << "ERROR: close() failed: " << strerror(errno) << endl;
	}

	// close standard input and direct read-fd1 to standard input
	ret = dup2 (p2c_pipe[0], fileno(stdin));
	
	if (ret == -1)
	{
		cout << "ERROR: dup2() failed: " << strerror(errno) << endl;
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
	for ( ; numfailed < 10; anfd++)
	{
		if ( anfd == c2p_pipe[1] ) continue; // don't close this
		if ( anfd == c2p_pipe[0] ) continue; // don't close this either (correct?)
		ret = close (anfd);
		if (ret < 0) numfailed++;
		else
		{
			numfailed = 0;
			closed++;
		}
	}

	cout << "Closed " << closed << " files." << endl;


	/*
	Start the desired executable and go away.
	*/

	
	cout << "Starting process: ";

	for (int i = 0; argv[i] != 0; ++i)
	{
		cout << argv[i] << " ";
	}
	cout << endl;

	/*
	For debugging the plugin (GNASH_OPTIONS=waitforgdb)
	Block here until gdb is attached and sets waitforgdb to
	false.
	*/

	if (waitforgdb) {

		cout << endl << "  Attach GDB to PID " << getpid()
				<< " to debug!" << endl;
		cout << "  This thread will block until then!" << endl;
		cout << "  Once blocked here, you can set other breakpoints."
				<< endl;
		cout << "  Do a \"set variable waitforgdb=$false\" to continue"
				<< endl << endl;
		
		while (waitforgdb)
		{
			sleep(1);
		}
	}

	execv(argv[0], const_cast<char**>(argv));

	// if execv returns, an error has occurred.
	perror("executing standalone gnash");
	
	//could actually cause a deadlock due to jemalloc and we
	//are exiting now anyways
	//delete[] argv;

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

	if (!NPVARIANT_IS_OBJECT(vDoc))
	{
		cout << "Can't get window object" << endl;
		return NULL;
	}
	
        NPObject* npDoc = NPVARIANT_TO_OBJECT(vDoc);

        NPIdentifier sLocation = NPN_GetStringIdentifier("location");
        NPVariant vLoc;
        NPN_GetProperty(npp, npDoc, sLocation, &vLoc);
        NPN_ReleaseObject(npDoc);
        if (!NPVARIANT_IS_OBJECT(vLoc))
	{
		cout <<"Can't get window.location object" << endl;
		return NULL;
	}
        NPObject* npLoc = NPVARIANT_TO_OBJECT(vLoc);

        NPIdentifier sProperty = NPN_GetStringIdentifier("href");
        NPVariant vProp;
        NPN_GetProperty(npp, npLoc, sProperty, &vProp);
        NPN_ReleaseObject(npLoc);
        if (!NPVARIANT_IS_STRING(vProp))
	{
		cout << "Can't get window.location.href object" << endl;
		return NULL;
	}
        const NPString& propValue = NPVARIANT_TO_STRING(vProp);

        return propValue.utf8characters; // const char *
}

static const char* getPluginDescription() 
{
	static const char* desc = NULL;
	if (!desc)
	{
		desc = std::getenv("GNASH_PLUGIN_DESCRIPTION");
		if (desc == NULL) desc = PLUGIN_DESCRIPTION;
	}
	return desc;
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
