// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#ifdef HAVE_WINSOCK2_H
# include <io.h>
#endif

// Support compilation with (or without) native language support
#include "gettext.h"	// for internationalization (GNU gettext)
#define	_(String) gettext (String)
#define N_(String) gettext_noop (String)

#include "rc.h" // for IF_VERBOSE_* implementation
//#include "tu_config.h"

#include <fstream>
#include <sstream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

// the default name for the debug log
#define DEFAULT_LOGFILE "gnash-dbg.log"
#define TIMESTAMP_LENGTH 24             // timestamp length
#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S     " // timestamp format

namespace gnash {

extern std::ostream& stampon(std::ostream& x);
extern std::ostream& stampoff(std::ostream& x);
extern std::ostream& timestamp(std::ostream& x);
extern std::ostream& datetimestamp(std::ostream& x);
#define TRACELEVEL 2

class DSOLOCAL Verbose {
    int level;
public:
    Verbose(int l) { level = l; }
    friend std::ostream& operator<<(std::ostream&, Verbose&);
};

// This is a basic file logging class
class DSOEXPORT LogFile {
public:

    static LogFile& getDefaultInstance();

    ~LogFile(void) {
	if (_state == OPEN) {
	    closeLog();
	}
    }

    enum file_state {
	CLOSED,
	OPEN,
	INPROGRESS,
	IDLE
    } _state;

    /// Intended for use by log_*(). Thread-safe (locks _ioMutex)
    //
    /// @param label
    ///		The label string ie: "ERROR" for "ERROR: <msg>"
    ///
    /// @param msg
    ///		The message string ie: "bah" for "ERROR: bah"
    ///
    void log(const char* label, const char* msg);

    /// Intended for use by log_*(). Thread-safe (locks _ioMutex)
    //
    /// @param msg
    ///		The message to print
    ///
    void log(const char* msg);
    
    file_state GetState (void) { return _state; }

    const char *getEntry(void);
    
    /// Open the specified file to write logs on disk
    //
    /// Locks _ioMutex to prevent race conditions accessing _outstream
    ///
    /// @return true on success, false on failure
    ///
    bool openLog(const std::string& filespec)
    {
        return openLog(filespec.c_str());
    }

    /// See openLog(const std::string&) 
    bool openLog(const char *filespec);

    /// Remove the log file
    //
    /// Does NOT lock _ioMutex (should it?)
    ///
    bool removeLog(void);

    /// Close the log file
    //
    /// Locks _ioMutex to prevent race conditions accessing _outstream
    ///
    bool closeLog(void);

    // accessors for the verbose level
    void setVerbosity (void) {
	_verbose++;
    }
    void setVerbosity (int x) {
	_verbose = x;
    }
    int getVerbosity (void) {
	return _verbose;
    }
    
    void setActionDump (int x) {
	_actiondump = x;
    }
    int getActionDump (void) {
	return _actiondump;
    }
    
    void setParserDump (int x) {
	_parserdump = x;
    }
    int getParserDump (void) {
	return _parserdump;
    }
    
    void setStamp (bool b) {
	_stamp = b;
    }
    bool getStamp (void) {
	return _stamp;
    }
    void setWriteDisk (bool b) {
	_write = b;
    }
    bool getWriteDisk (void) {
	return _write;
    }
    
private:

    // Use getDefaultInstance for getting the singleton
    LogFile (void);

    // Use getDefaultInstance for getting the singleton
    LogFile (const char *);

    boost::mutex _ioMutex;

    static std::ofstream _console;
    std::ofstream	 _outstream;
    static int		 _verbose;
    static bool		 _actiondump;
    static bool		 _parserdump;
    bool		 _stamp;
    bool		 _write;
    bool		 _trace;
    std::string		 _filespec;
    std::string		 _logentry;
    friend std::ostream & operator << (std::ostream &os, LogFile& e);

    LogFile& operator << (char x);
    LogFile& operator << (int x);
    LogFile& operator << (long x);
    LogFile& operator << (unsigned int x);
    LogFile& operator << (unsigned long x);
    // These both resolve to an unsigned int.
    // LogFile& operator << (size_t x);
    // LogFile& operator << (time_t x);
    LogFile& operator << (float x);
    LogFile& operator << (double &x);
    LogFile& operator << (bool x);
    LogFile& operator << (void *);
    LogFile& operator << (const char *);
    LogFile& operator << (unsigned char const *);
    LogFile& operator << (const std::string &s);
//     LogFile& operator << (const xmlChar *c);
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

/// Log a generic message
//
/// This is usually used for debugging, so most
/// such calls should be in comments, unless you're actively debugging
/// that piece of code right now.
///
DSOEXPORT void log_msg(const char* fmt, ...) GNUC_LOG_ATTRS;

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
	if (LogFile::getDefaultInstance().getVerbosity() >= TRACELEVEL+1) {
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
