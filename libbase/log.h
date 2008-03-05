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

#ifndef GNASH_LOG_H
#define GNASH_LOG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_WINSOCK2_H
# include <io.h>
#endif

#include "rc.h" // for IF_VERBOSE_* implementation

#include <fstream>
#include <sstream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/format.hpp>

// the default name for the debug log
#define DEFAULT_LOGFILE "gnash-dbg.log"
#define TIMESTAMP_LENGTH 24             // timestamp length
#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S     " // timestamp format

// Support compilation with (or without) native language support
#include "gettext.h"
#define	_(String) gettext (String)
#define N_(String) gettext_noop (String)

// This macro should be used to add both boost formatting and
// internationalization to log messages. The po directory
// Makefile must also look for the BF macro for gettext
// processing, otherwise they will not appear in the
// translation (.po) files.
//#define BF(x) logFormat(_(x))

// Define to switch between printf-style log formatting
// and boost::format
#define USE_BOOST_FORMAT_TEMPLATES 1

#ifdef USE_BOOST_FORMAT_TEMPLATES
# include <boost/preprocessor/arithmetic/inc.hpp>
# include <boost/preprocessor/repetition/enum_params.hpp>
# include <boost/preprocessor/repetition/repeat.hpp>
# include <boost/preprocessor/repetition/repeat_from_to.hpp>
# include <boost/preprocessor/seq/for_each.hpp>
#endif

namespace gnash {

extern std::ostream& stampon(std::ostream& x);
extern std::ostream& stampoff(std::ostream& x);
extern std::ostream& timestamp(std::ostream& x);
extern std::ostream& datetimestamp(std::ostream& x);
#define DEBUGLEVEL 2

// This is a basic file logging class
class DSOEXPORT LogFile {
public:

    static LogFile& getDefaultInstance();

    ~LogFile();

    enum FileState {
        CLOSED,
        OPEN,
        INPROGRESS,
        IDLE
    };

    /// Intended for use by log_*(). Thread-safe (locks _ioMutex)
    //
    /// @param label
    ///		The label string ie: "ERROR" for "ERROR: <msg>"
    ///
    /// @param msg
    ///		The message string ie: "bah" for "ERROR: bah"
    ///
    void log(const std::string& label, const std::string& msg);

    /// Intended for use by log_*(). Thread-safe (locks _ioMutex)
    //
    /// @param msg
    ///		The message to print
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

    // accessors for the verbose level
    void setVerbosity () {
        _verbose++;
    }

    void setVerbosity (int x) {
        _verbose = x;
    }

    int getVerbosity () {
        return _verbose;
    }
    
    void setActionDump (int x) {
        _actiondump = x;
    }

    int getActionDump () {
        return _actiondump;
    }
    
    void setParserDump (int x) {
        _parserdump = x;
    }

    int getParserDump () {
        return _parserdump;
    }
    
    void setStamp (bool b) {
        _stamp = b;
    }

    bool getStamp () {
        return _stamp;
    }

    void setWriteDisk (bool b) {
        _write = b;
    }

    bool getWriteDisk () {
        return _write;
    }

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
    std::ofstream	 _outstream;

    /// How much output is required: 2 or more gives debug output.
    int		 _verbose;

    /// Whether to dump all SWF actions
    bool		 _actiondump;

    /// Whether to dump parser output
    bool		 _parserdump;

    /// The state of the log file.
    FileState _state;

    bool		 _stamp;

    /// Whether to write the log file to disk.
    bool		 _write;

    std::string		 _filespec;

    /// For the ostream << operator
    friend std::ostream & operator << (std::ostream &os, LogFile& e);

    LogFile& operator << (const std::string &s);
    LogFile& operator << (std::ostream & (&)(std::ostream &));

    /// Print anything that can be printed on a stringstream
    //
    /// This template function could replace ALL but
    /// operator << (const std::string&) members of
    /// LogFile class.
    ///
    template <class T>
    LogFile& operator << (const T& any)
    {
	    std::stringstream ss;
	    ss << any;
	    return *this << ss.str();
    }

};


#ifdef USE_BOOST_FORMAT_TEMPLATES
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
#define LOG_TYPES (error) (debug) (unimpl) (aserror) (swferror) (security) (action) (parse) (trace)

/// This actually creates the template functions using the TOKENIZE
/// functions above. The templates look like this:
//
/// template< typename T0 , typename T1 , typename T2 , typename T3 > 
/// void
/// log_security (const T0& t0, const T1& t1, const T2& t2, const T3& t3)
/// {
///     if (_verbosity == 0) return;
///     processLog_security(myFormat(t0) % t1 % t2 % t3);
/// }
//
/// Only not as nicely indented.
///
/// Use "g++ -E log.h" or "gcc log.h" to check.
#define LOG_TEMPLATES(z, n, data)\
    template< \
         BOOST_PP_ENUM_PARAMS(\
         BOOST_PP_INC(n), typename T)\
     >\
    DSOEXPORT void log_##data (\
        BOOST_PP_REPEAT(\
        BOOST_PP_INC(n), \
        TOKENIZE_ARGS, t)\
    ) { \
    if (LogFile::getDefaultInstance().getVerbosity() == 0) return; \
    processLog_##data(logFormat(t0) \
    BOOST_PP_REPEAT_FROM_TO(1, \
        BOOST_PP_INC(n), \
        TOKENIZE_FORMAT, t));\
    }\

/// Defines the maximum number of template arguments
//
/// The preprocessor generates templates with 1..ARG_NUMBER
/// arguments.
#define ARG_NUMBER 16

/// Calls the macro LOG_TEMPLATES an ARG_NUMBER number
/// of times, each time adding an extra typename argument to the
/// template.
#define GENERATE_LOG_TYPES(r, _, t) \
    BOOST_PP_REPEAT(ARG_NUMBER, LOG_TEMPLATES, t)

/// Calls the template generator for each log type in the
/// sequence LOG_TYPES.
BOOST_PP_SEQ_FOR_EACH(GENERATE_LOG_TYPES, _, LOG_TYPES)

#undef TOKENIZE_FORMAT
#undef GENERATE_LOG_TYPES
#undef LOG_TEMPLATES
#undef ARG_NUMBER

DSOEXPORT void processLog_error(const boost::format& fmt);
DSOEXPORT void processLog_unimpl(const boost::format& fmt);
DSOEXPORT void processLog_trace(const boost::format& fmt);
DSOEXPORT void processLog_debug(const boost::format& fmt);
DSOEXPORT void processLog_action(const boost::format& fmt);
DSOEXPORT void processLog_parse(const boost::format& fmt);
DSOEXPORT void processLog_security(const boost::format& fmt);
DSOEXPORT void processLog_swferror(const boost::format& fmt);
DSOEXPORT void processLog_aserror(const boost::format& fmt);

#else

#ifdef __GNUC__
#define GNUC_LOG_ATTRS __attribute__((format (printf, 1, 2)))
#else
#define GNUC_LOG_ATTRS
#endif

/// Log a runtime error
//
/// Runtime errors, such as un-openable files, un-allocatable memory,
/// etc, are logged (for convenience of the user or debugger) this way.
/// This function is not used to report coding errors; use log_aserror
/// or log_swferror for that.
///
DSOEXPORT void log_error(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log a message about unimplemented features.
//
/// This function must be used to warn user about missing Gnash features.
/// We expect all calls to this function to disappear over time, as we
/// implement those features of Flash.
///
DSOEXPORT void log_unimpl(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Use only for explicit user traces
//
/// Current users are Global.cpp for _global.trace() and
/// ASHandlers.cpp for ActionTrace
///
DSOEXPORT void log_trace(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log debug info
//
/// Used for function entry/exit tracing.
///
DSOEXPORT void log_debug(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log action execution info
//
/// Wrap all calls to this function (and other related statements)
/// into an IF_VERBOSE_ACTION macro, so to allow completely
/// removing all the overhead at compile time and reduce it
/// at runtime.
///
DSOEXPORT void log_action(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log parsing information
//
/// Wrap all calls to this function (and other related statements)
/// into an IF_VERBOSE_PARSE macro, so to allow completely
/// removing all the overhead at compile time and reduce it
/// at runtime.
///
DSOEXPORT void log_parse(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log security information
DSOEXPORT void log_security(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log a malformed SWF error
//
/// This indicates an error in how the binary SWF file was constructed, i.e.
/// probably a bug in the tools used to build the SWF file.
///
/// Wrap all calls to this function (and other related statements)
/// into an IF_VERBOSE_MALFORMED_SWF macro, so to allow completely
/// removing all the overhead at compile time and reduce it
/// at runtime.
///
DSOEXPORT void log_swferror(const char* fmt, ...) GNUC_LOG_ATTRS;

/// Log an ActionScript error
//
/// This indicates an error by the programmer who wrote the ActionScript
/// code, such as too few or too many arguments to a function.
///
/// Wrap all calls to this function (and other related statements)
/// into an IF_VERBOSE_ASCODING_ERRORS macro, so to allow completely
/// removing all the overhead at compile time and reduce it
/// at runtime.
///
DSOEXPORT void log_aserror(const char* fmt, ...) GNUC_LOG_ATTRS;


#endif // USE_BOOST_FORMAT_TEMPLATES

/// A fault-tolerant boost::format object for logging
//
/// Generally to be used in the LogFile macro BF(), which will also
/// be recognized by gettext for internationalization and is less
/// effort to type.
DSOEXPORT boost::format logFormat(const std::string &str);

/// Convert a sequence of bytes to hex or ascii format.
//
/// @param bytes    the array of bytes to process
/// @param length   the number of bytes to read. Callers are responsible
///                 for checking that length does not exceed the array size.
/// @param ascii    whether to return in ascii or space-separated hex format.
/// @return         a string representation of the byte sequence.
DSOEXPORT std::string hexify(const unsigned char *bytes, size_t length, bool ascii);

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

class DSOEXPORT __Host_Function_Report__ {
public:
    const char *func;

    // Only print function tracing messages when multiple -v
    // options have been supplied. 
    __Host_Function_Report__(void) {
	log_debug("entering");
    }

    __Host_Function_Report__(char *_func) {
	func = _func;
	log_debug("%s enter", func);
    }

    __Host_Function_Report__(const char *_func) {
	func = _func;
	log_debug("%s enter", func);
    }

    ~__Host_Function_Report__(void) {
	if (LogFile::getDefaultInstance().getVerbosity() >= DEBUGLEVEL + 1) {
	    log_debug("returning");
	}
    }
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
	gnash::__Host_Function_Report__ __host_function_report__( __PRETTY_FUNCTION__)
#define GNASH_REPORT_RETURN
#else
#define GNASH_REPORT_FUNCTION \
    log_debug("entering")

#define GNASH_REPORT_RETURN \
    log_debug("returning")
#endif

}


#endif // GNASH_LOG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
