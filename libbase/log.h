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

// Support compilation with (or without) native language support
#include "gettext.h"	// for internationalization (GNU gettext)
#define	_(String) gettext (String)
#define N_(String) gettext_noop (String)

#include "rc.h" // for IF_VERBOSE_* implementation

#include <fstream>
#include <sstream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
//#include <boost/format.hpp>

// the default name for the debug log
#define DEFAULT_LOGFILE "gnash-dbg.log"
#define TIMESTAMP_LENGTH 24             // timestamp length
#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S     " // timestamp format

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

    enum fileState {
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
    static int		 _verbose;

    /// Whether to dump all SWF actions
    static bool		 _actiondump;

    /// Whether to dump parser output
    static bool		 _parserdump;

    /// The state of the log file.
    fileState _state;

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


DSOEXPORT unsigned char *hexify(unsigned char *p, const unsigned char *s, int length, bool ascii);

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
//DSOEXPORT void logError(const boost::format& fmt);

/// Log a message about unimplemented features.
//
/// This function must be used to warn user about missing Gnash features.
/// We expect all calls to this function to disappear over time, as we
/// implement those features of Flash.
///
DSOEXPORT void log_unimpl(const char* fmt, ...) GNUC_LOG_ATTRS;
//DSOEXPORT void logUnimpl(const boost::format& fmt);

/// Use only for explicit user traces
//
/// Current users are Global.cpp for _global.trace() and
/// ASHandlers.cpp for ActionTrace
///
DSOEXPORT void log_trace(const char* fmt, ...) GNUC_LOG_ATTRS;
//DSOEXPORT void logTrace(const boost::format& fmt);

/// Log debug info
//
/// Used for function entry/exit tracing.
///
DSOEXPORT void log_debug(const char* fmt, ...) GNUC_LOG_ATTRS;
//DSOEXPORT void logDebug(const boost::format& fmt);

/// Log action execution info
//
/// Wrap all calls to this function (and other related statements)
/// into an IF_VERBOSE_ACTION macro, so to allow completely
/// removing all the overhead at compile time and reduce it
/// at runtime.
///
DSOEXPORT void log_action(const char* fmt, ...) GNUC_LOG_ATTRS;
//DSOEXPORT void logAction(const boost::format& fmt);

/// Log parsing information
//
/// Wrap all calls to this function (and other related statements)
/// into an IF_VERBOSE_PARSE macro, so to allow completely
/// removing all the overhead at compile time and reduce it
/// at runtime.
///
DSOEXPORT void log_parse(const char* fmt, ...) GNUC_LOG_ATTRS;
//DSOEXPORT void logParse(const boost::format& fmt);

/// Log security information
DSOEXPORT void log_security(const char* fmt, ...) GNUC_LOG_ATTRS;
//DSOEXPORT void logSecurity(const boost::format& fmt);

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
//DSOEXPORT void logSWFError(const boost::format& fmt);

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
//DSOEXPORT void logASError(const boost::format& fmt);


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
	if (LogFile::getDefaultInstance().getVerbosity() >= DEBUGLEVEL+1) {
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
