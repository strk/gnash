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

#ifndef GNASH_LOG_H
#define GNASH_LOG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "rc.h" // for IF_VERBOSE_* implementation
#include "dsodefs.h" // for DSOEXPORT

#include <fstream>
#include <boost/thread/mutex.hpp>
#include <boost/format.hpp>

// the default name for the debug log
#define DEFAULT_LOGFILE "gnash-dbg.log"

// Support compilation with (or without) native language support
#include "gettext.h"
#define    _(String) gettext (String)
#define N_(String) gettext_noop (String)

// Macro to prevent repeated logging calls for the same
// event
#define LOG_ONCE(x) { \
    static bool warned = false; \
    if (!warned) { warned = true; x; } \
}

# include <boost/preprocessor/arithmetic/inc.hpp>
# include <boost/preprocessor/repetition/enum_params.hpp>
# include <boost/preprocessor/repetition/repeat.hpp>
# include <boost/preprocessor/repetition/repeat_from_to.hpp>
# include <boost/preprocessor/seq/for_each.hpp>

// Mingw32 (win32 console) doesn't use the standard GCC defines that
// Gnash used for debug messages, so make it so...
#ifndef __FUNCDNAME__
#define __FUNCDNAME__ __FUNCTION__
#endif

namespace gnash {

// This is a basic file logging class
class DSOEXPORT LogFile
{
public:

    static LogFile& getDefaultInstance();

    ~LogFile();

    enum LogLevel {
        LOG_SILENT,
        LOG_NORMAL,
        LOG_DEBUG,
        LOG_EXTRA
    };

    enum FileState {
        CLOSED,
        OPEN,
        INPROGRESS,
        IDLE
    };

    /// Intended for use by log_*(). Thread-safe (locks _ioMutex)
    //
    /// @param label
    ///        The label string ie: "ERROR" for "ERROR: <msg>"
    ///
    /// @param msg
    ///        The message string ie: "bah" for "ERROR: bah"
    ///
    void log(const std::string& label, const std::string& msg);

    /// Intended for use by log_*(). Thread-safe (locks _ioMutex)
    //
    /// @param msg
    ///        The message to print
    ///
    void log(const std::string& msg);
    
    /// Remove the log file
    //
    /// Does NOT lock _ioMutex (should it?)
    ///
    bool removeLog();

    /// Close the log file
    //
    /// Locks _ioMutex to prevent race conditions accessing _outstream
    ///
    bool closeLog();

    /// Set log filename 
    //
    /// If a log file is opened already, it will be closed
    /// by this call, and will be reopened on next use
    /// if needed.
    ///
    void setLogFilename(const std::string& fname);

    // accessors for the verbose level
    void setVerbosity() {
        ++_verbose;
    }

    void setVerbosity(int x) {
        _verbose = x;
    }

    int getVerbosity() const {
        return _verbose;
    }
    
    void setActionDump(int x) {
        _actiondump = x;
    }

    void setNetwork(int x) {
        _network = x;
    }

    int getActionDump() const {
        return _actiondump;
    }
    
    int getNetwork() const {
        return _network;
    }
    
    void setParserDump (int x) {
        _parserdump = x;
    }

    int getParserDump() const {
        return _parserdump;
    }
    
    void setStamp (bool b) {
        _stamp = b;
    }

    bool getStamp() const {
        return _stamp;
    }

    /// Set whether to write logs to file
    void setWriteDisk(bool b);

    bool getWriteDisk() const {
        return _write;
    }
    
    typedef void (*logListener)(const std::string& s);
    
    void registerLogCallback(logListener l) { _listener = l; }

private:
    
    /// Open the specified file to write logs on disk
    //
    /// Locks _ioMutex to prevent race conditions accessing _outstream
    ///
    /// @return true on success, false on failure
    ///
    bool openLog(const std::string& filespec);

    /// \brief
    /// Open the RcInitFile-specified log file if log write
    /// is requested. 
    //
    /// This method is called before any attempt to write is made.
    /// It will return true if the file was opened, false if wasn't
    /// (either not requested or error).
    ///
    /// On error, will print a message on stderr
    ///
    bool openLogIfNeeded();

    // Use getDefaultInstance for getting the singleton
    LogFile ();

    /// Mutex for locking I/O during logfile access.
    boost::mutex _ioMutex;

    /// Stream to write to stdout.
    std::ofstream _outstream;

    /// How much output is required: 2 or more gives debug output.
    int _verbose;

    /// Whether to dump all SWF actions
    bool _actiondump;

    /// Whether to dump all networking actions
    bool _network;

    /// Whether to dump parser output
    bool _parserdump;

    /// The state of the log file.
    FileState _state;

    bool _stamp;

    /// Whether to write the log file to disk.
    bool _write;

    std::string _filespec;

    std::string _logFilename;
    
    logListener _listener;

};

DSOEXPORT void processLog_network(const boost::format& fmt);
DSOEXPORT void processLog_error(const boost::format& fmt);
DSOEXPORT void processLog_unimpl(const boost::format& fmt);
DSOEXPORT void processLog_trace(const boost::format& fmt);
DSOEXPORT void processLog_debug(const boost::format& fmt);
DSOEXPORT void processLog_action(const boost::format& fmt);
DSOEXPORT void processLog_parse(const boost::format& fmt);
DSOEXPORT void processLog_security(const boost::format& fmt);
DSOEXPORT void processLog_swferror(const boost::format& fmt);
DSOEXPORT void processLog_aserror(const boost::format& fmt);
DSOEXPORT void processLog_abc(const boost::format& fmt);

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
#define LOG_TYPES (error) (debug) (unimpl) (aserror) (swferror) \
    (security) (action) (parse) (trace) (abc) (network)

/// This actually creates the template functions using the TOKENIZE
/// functions above. The templates look like this:
//
/// template<typename T0 , typename T1 , typename T2>
/// void log_error(const T0& t0 , const T1& t1 , const T2& t2)
/// {
///     if (LogFile::getDefaultInstance().getVerbosity() == 0) return;
///     boost::format f(t0);
///     using namespace boost::io;
///     f.exceptions(all_error_bits ^ (too_many_args_bit |
///                                    too_few_args_bit |
///                                    bad_format_string_bit));
///     processLog_error(f % t1 % t2);
/// }
///
/// Only not as nicely indented.
///
/// Use "g++ -E log.h" or "cpp log.h" to check.
#define LOG_TEMPLATES(z, n, data)\
template<BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(n), typename T)>\
inline void log_##data(BOOST_PP_REPEAT(BOOST_PP_INC(n), TOKENIZE_ARGS, t)) \
{\
    if (LogFile::getDefaultInstance().getVerbosity() == 0) return; \
    boost::format f(t0); \
    using namespace boost::io; \
    f.exceptions(all_error_bits ^ (too_many_args_bit | \
                                   too_few_args_bit | \
                                   bad_format_string_bit)); \
    processLog_##data(f BOOST_PP_REPEAT_FROM_TO(1, \
            BOOST_PP_INC(n), \
            TOKENIZE_FORMAT, t));\
}

/// Defines the maximum number of template arguments
//
/// The preprocessor generates templates with 1..ARG_NUMBER
/// arguments.
#define ARG_NUMBER 10

/// Calls the macro LOG_TEMPLATES an ARG_NUMBER number
/// of times, each time adding an extra typename argument to the
/// template.
#define GENERATE_LOG_TYPES(r, _, t) \
    BOOST_PP_REPEAT(ARG_NUMBER, LOG_TEMPLATES, t)

/// Calls the template generator for each log type in the
/// sequence LOG_TYPES.
BOOST_PP_SEQ_FOR_EACH(GENERATE_LOG_TYPES, _, LOG_TYPES)

#undef TOKENIZE_ARGS
#undef TOKENIZE_FORMAT
#undef GENERATE_LOG_TYPES
#undef LOG_TEMPLATES
#undef ARG_NUMBER

/// Convert a sequence of bytes to hex or ascii format.
//
/// @param bytes    the array of bytes to process
/// @param length   the number of bytes to read. Callers are responsible
///                 for checking that length does not exceed the array size.
/// @param ascii    whether to return in ascii or space-separated hex format.
/// @return         a string representation of the byte sequence.
DSOEXPORT std::string hexify(const unsigned char *bytes, size_t length,
        bool ascii);

// Define to 0 to completely remove parse debugging at compile-time
#ifndef VERBOSE_PARSE
#define VERBOSE_PARSE 1
#endif

// Define to 0 to completely remove action debugging at compile-time
#ifndef VERBOSE_ACTION
#define VERBOSE_ACTION 1
#endif

// Define to 0 to remove ActionScript errors verbosity at compile-time
#ifndef VERBOSE_ASCODING_ERRORS
#define VERBOSE_ASCODING_ERRORS  1
#endif

// Define to 0 this to remove invalid SWF verbosity at compile-time
#ifndef VERBOSE_MALFORMED_SWF
#define VERBOSE_MALFORMED_SWF 1
#endif

// Define to 0 this to remove Networking verbosity at compile-time
#ifndef VERBOSE_NETWORKING
#define VERBOSE_NETWORKING 1
#endif

#if VERBOSE_PARSE
#define IF_VERBOSE_PARSE(x) do { if ( LogFile::getDefaultInstance().getParserDump() ) { x; } } while (0);
#else
#define IF_VERBOSE_PARSE(x)
#endif

#if VERBOSE_ACTION
#define IF_VERBOSE_ACTION(x) do { if ( LogFile::getDefaultInstance().getActionDump() ) { x; } } while (0);
#else
#define IF_VERBOSE_ACTION(x)
#endif

#if VERBOSE_ACTION
#define IF_VERBOSE_NETWORK(x) do { if ( LogFile::getDefaultInstance().getNetwork() ) { x; } } while (0);
#else
#define IF_VERBOSE_NETWORK(x)
#endif

#if VERBOSE_ASCODING_ERRORS
// TODO: check if it's worth to check verbosity level too...
#define IF_VERBOSE_ASCODING_ERRORS(x) { if ( gnash::RcInitFile::getDefaultInstance().showASCodingErrors() ) { x; } }
#else
#define IF_VERBOSE_ASCODING_ERRORS(x)
#endif

#if VERBOSE_MALFORMED_SWF
// TODO: check if it's worth to check verbosity level too... 
#define IF_VERBOSE_MALFORMED_SWF(x) { if ( gnash::RcInitFile::getDefaultInstance().showMalformedSWFErrors() ) { x; } }
#else
#define IF_VERBOSE_MALFORMED_SWF(x)
#endif

class DSOEXPORT HostFunctionReport
{
public:
    // Only print function tracing messages when multiple -v
    // options have been supplied. 
    HostFunctionReport() {
        log_debug("entering");
    }

    HostFunctionReport(const char* func) : _func(func) {
        if (func) {
            log_debug("%s enter", func);
        }
        else {
            log_debug("No Function Name! enter");
        }
    }
    ~HostFunctionReport() {
        log_debug("%s returning", _func);
    }
private:
    const char* _func;
};

#ifndef HAVE_FUNCTION
    #ifndef HAVE_func
        #define dummystr(x) # x
        #define dummyestr(x) dummystr(x)
        #define __FUNCTION__ __FILE__":"dummyestr(__LINE__)
    #else
        #define __FUNCTION__ __func__    
    #endif
#endif

#ifndef HAVE_PRETTY_FUNCTION
    #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#if defined(__cplusplus) && defined(__GNUC__)
#define GNASH_REPORT_FUNCTION   \
    const gnash::HostFunctionReport hfr(__PRETTY_FUNCTION__)
#define GNASH_REPORT_RETURN
#else
#define GNASH_REPORT_FUNCTION \
    gnash::log_debug("entering")

#define GNASH_REPORT_RETURN \
    gnash::log_debug("returning")
#endif

}


#endif // GNASH_LOG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
