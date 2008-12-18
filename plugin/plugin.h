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


#ifndef GNASH_PLUGIN_H
#define GNASH_PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifndef HAVE_FUNCTION
# ifndef HAVE_func
#  define dummystr(x) # x
#  define dummyestr(x) dummystr(x)
#  define __FUNCTION__ __FILE__":"dummyestr(__LINE__)
# else
#  define __FUNCTION__ __func__	
# endif
#endif

#ifndef HAVE_PRETTY_FUNCTION
	#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

/* Xlib/Xt stuff */
#include <X11/Xlib.h>
//#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#ifdef HAVE_GTK2
#include <gtk/gtk.h>
#endif
#include <string>
#include <map>
#include "pluginbase.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"

extern NPBool      plugInitialized;
extern PRLock      *playerMutex;
extern PRCondVar   *playerCond;

class nsPluginInstance : public nsPluginInstanceBase
{
public:
    nsPluginInstance(nsPluginCreateData* );
    virtual ~nsPluginInstance();

    // We are required to implement these three methods.
    NPBool init(NPWindow *aWindow);
    NPBool isInitialized() { return plugInitialized; }
    void shut();

    NPError GetValue(NPPVariable variable, void *value);
    NPError SetWindow(NPWindow *aWindow);

    NPError NewStream(NPMIMEType type, NPStream *stream, NPBool seekable,
                      uint16_t *stype);
    NPError DestroyStream(NPStream * stream, NPError reason);

    int32_t WriteReady(NPStream *stream);
    int32_t Write(NPStream *stream, int32_t offset, int32_t len, void *buffer);

    NPError WriteStatus(char *msg) const;
    NPError WriteStatus(std::string msg) const;

    void startProc(Window win);

private:

    static bool handlePlayerRequestsWrapper(GIOChannel* iochan, GIOCondition cond, nsPluginInstance* plugin);

    bool handlePlayerRequests(GIOChannel* iochan, GIOCondition cond);

    /// Process a null-terminated request line
    //
    /// @param buf
    ///	  The single request.
    ///   Caller is responsible for memory management, but give us
    ///   permission to modify the string.
    ///
    /// @param len
    ///	  Lenght of buffer.
    ///
    /// @return true if the request was processed, false otherwise (bogus request..)
    ///
    bool processPlayerRequest(gchar* buf, gsize len);

    // EMBED or OBJECT attributes / parameters
    // @@ this should likely replace the _options element below
    std::map<std::string, std::string> _params;

    /// Dump current session cookies to a file,
    /// setting _cookieFile to its name (or clear
    /// it on failure).
    void dumpCookies();

    NPP                                _instance;
    Window                             _window;
    std::string                        _swf_url;
    std::string                        _swf_file;
    unsigned int                       _width;
    unsigned int                       _height;
    std::map<std::string, std::string> _options;
    int                                _streamfd;
    GIOChannel*                        _ichan;
    int                                _ichanWatchId;
    pid_t                              _childpid;
    int                                _filefd;

    /// Name of the plugin instance element in the dom 
    std::string                        _name;

    /// Cookie file
    std::string                        _cookieFile;

    const char* getCurrentPageURL() const;
};

// end of __PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// End:
