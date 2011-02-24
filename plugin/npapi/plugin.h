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
# define __PRETTY_FUNCTION__ __FUNCTION__
#endif

/* Xlib/Xt stuff */
#include <X11/Xlib.h>
//#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <glib.h>
#include <string>
#include <map>
#include <vector>
#include <boost/format.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include "pluginbase.h"
#include "pluginScriptObject.h"

namespace gnash {

extern NPBool      plugInitialized;

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

    /// Open a new stream. THis is called every time there is swf content.
    NPError NewStream(NPMIMEType type, NPStream *stream, NPBool seekable,
                      uint16_t *stype);
    /// Destroy the stream
    NPError DestroyStream(NPStream * stream, NPError reason);

    /// Can the stream be written to yet ?
    int32_t WriteReady(NPStream *stream);
    int32_t Write(NPStream *stream, int32_t offset, int32_t len, void *buffer);
    NPObject *getScriptableObject();
    const char *getEmbedURL() const;

    GnashPluginScriptObject *getScriptObject() { return _scriptObject; }; // FIXME: debug only!!!

private:
    void startProc();
    std::vector<std::string> getCmdLine(int hostfd, int controlfd);

    void setupCookies(const std::string& pageURL);
    void setupProxy(const std::string& pageURL);

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

    NPP                                _instance;
    Window                             _window;
    std::string                        _swf_url;
    std::string                        _swf_file;
    unsigned int                       _width;
    unsigned int                       _height;
    std::map<std::string, std::string> _options;
    int                                _streamfd;
    int                                _ichanWatchId;
    pid_t                              _childpid;
    int                                _filefd;

    /// Name of the plugin instance element in the dom 
    std::string                        _name;
    GnashPluginScriptObject             *_scriptObject;
    
    std::string getCurrentPageURL() const;
};

// Define the following to make the plugin verbose
// WARNING: will write to .xsession_errors !
// Values:
//  0: no messages at all
//  1: fatal errors (errors preventing the plugin from working as it should)
//  2: informational messages
#define GNASH_PLUGIN_DEBUG 1

// This following logging code is copied from libbase/log.h, but
// duplicated here because the plugin only needs a more trimmed down
// version that doesn't require any Gnash libraires to keep the
// memory footprint down.
DSOEXPORT void processLog_error(const boost::format& fmt);
DSOEXPORT void processLog_debug(const boost::format& fmt);
DSOEXPORT void processLog_trace(const boost::format& fmt);

/// This heap of steaming preprocessor code magically converts
/// printf-style statements into boost::format messages using templates.
//
/// Macro to feed boost::format strings to the boost::format object,
/// producing code like this: "% t1 % t2 % t3 ..."
#define TOKENIZE_FORMAT(z, n, t) % t##n

/// Macro to add a number of arguments to the templated function
/// corresponding to the number of template arguments. Produces code
/// like this: "const T0& t0, const T1& t1, const T2& t2 ..."
#define TOKENIZE_ARGS(z, n, t) BOOST_PP_COMMA_IF(n) const T##n& t##n

/// This is a sequence of different log message types to be used in
/// the code. Append the name to log_ to call the function, e.g. 
/// log_error, log_unimpl.
#define LOG_TYPES (error) (debug) (trace)
/// The preprocessor generates templates with 1..ARG_NUMBER
/// arguments.
#define ARG_NUMBER 4
#define LOG_TEMPLATES(z, n, data)\
template<BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(n), typename T)>\
inline void log_##data(BOOST_PP_REPEAT(BOOST_PP_INC(n), TOKENIZE_ARGS, t)) \
{\
   if (GNASH_PLUGIN_DEBUG < 1) return; \
    boost::format f(t0);           \
    using namespace boost::io; \
    f.exceptions(all_error_bits ^ (too_many_args_bit | \
                                   too_few_args_bit | \
                                   bad_format_string_bit)); \
    processLog_##data(f BOOST_PP_REPEAT_FROM_TO(1, \
            BOOST_PP_INC(n), \
            TOKENIZE_FORMAT, t));\
}

/// Calls the macro LOG_TEMPLATES an ARG_NUMBER number
/// of times, each time adding an extra typename argument to the
/// template.
#define GENERATE_LOG_TYPES(r, _, t) \
    BOOST_PP_REPEAT(ARG_NUMBER, LOG_TEMPLATES, t)

/// Calls the template generator for each log type in the
/// sequence LOG_TYPES.
BOOST_PP_SEQ_FOR_EACH(GENERATE_LOG_TYPES, _, LOG_TYPES)

// end of __PLUGIN_H__
#endif

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
