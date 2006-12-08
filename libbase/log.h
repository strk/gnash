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

// 
//

/* $Id: log.h,v 1.37 2006/12/08 12:56:29 strk Exp $ */

#ifndef GNASH_LOG_H
#define GNASH_LOG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <fstream>
#include <sstream>

// the default name for the debug log
#define DEFAULT_LOGFILE "gnash-dbg.log"
#define TIMESTAMP_LENGTH 24             // timestamp length
#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S     " // timestamp format

#if SIZEOF_SIZE_T == SIZEOF_INT // try "int" first
  #define SIZET_FMT "%d"
#elif SIZEOF_SIZE_T == SIZEOF_LONG
  #define SIZET_FMT "%ld"
#elif SIZEOF_SIZE_T == SIZEOF_SHORT
  #define SIZET_FMT "%hd"
#endif

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
    LogFile (void);
    LogFile (const char *);

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
    
    file_state GetState (void) { return _state; }
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
// #ifdef HAVE_LIBXML
//     LogFile& operator << (const xmlChar *c);
// #endif
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

    const char *getEntry(void);
    
    bool openLog(const char *filespec);
    bool removeLog(void);
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
};


DSOEXPORT unsigned char *hexify(unsigned char *p, const unsigned char *s, int length, bool ascii);

#ifdef __GNUC__
DSOEXPORT void log_msg(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
DSOEXPORT void log_error(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
DSOEXPORT void log_warning(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
DSOEXPORT void log_trace(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
DSOEXPORT void log_action(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
DSOEXPORT void log_parse(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
DSOEXPORT void log_security(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
#else
// Printf-style interfaces.
DSOEXPORT void log_msg(const char* fmt, ...);
DSOEXPORT void log_error(const char* fmt, ...);
DSOEXPORT void log_warning(const char* fmt, ...);
DSOEXPORT void log_trace(const char* fmt, ...);
DSOEXPORT void log_action(const char* fmt, ...);
DSOEXPORT void log_parse(const char* fmt, ...);
DSOEXPORT void log_security(const char* fmt, ...);
#endif

// Undefine this to completely remove parse debugging at compile-time
#define VERBOSE_PARSE 1

// Undefine this to completely remove action debugging at compile-time
#define VERBOSE_ACTION 1

// Undefine this to remove ActionScript errors verbosity
//#define VERBOSE_ASCODING_ERRORS 

// Undefine this to remove invalid SWF verbosity
#define VERBOSE_MALFORMED_SWF


#ifdef VERBOSE_PARSE
#define IF_VERBOSE_PARSE(x) do { if ( LogFile::getDefaultInstance().getParserDump() ) { x; } } while (0);
#else
#define IF_VERBOSE_PARSE(x)
#endif

#ifdef VERBOSE_ACTION
#define IF_VERBOSE_ACTION(x) do { if ( LogFile::getDefaultInstance().getActionDump() ) { x; } } while (0);
#else
#define IF_VERBOSE_ACTION(x)
#endif

#ifdef VERBOSE_ASCODING_ERRORS
// TODO: add a getActionDebug() method to LogFile and use it
#define IF_VERBOSE_ASCODING_ERRORS(x) {x}
#else
#define IF_VERBOSE_ASCODING_ERRORS(x)
#endif

#ifdef VERBOSE_MALFORMED_SWF
// TODO: add a getSWFDebug() method to LogFile and use it
#define IF_VERBOSE_MALFORMED_SWF(x) {x}
#else
#define IF_VERBOSE_MALFORMED_SWF(x)
#endif

class DSOEXPORT __Host_Function_Report__ {
public:
    const char *func;

    // Only print function tracing messages when tmultiplewo -v
    // options have been supplied. 
    __Host_Function_Report__(void) {
	log_trace("entering");
    }

    __Host_Function_Report__(char *_func) {
	func = _func;
	log_trace("%s enter", func);
    }

    __Host_Function_Report__(const char *_func) {
	func = _func;
	log_trace("%s enter", func);
    }

    ~__Host_Function_Report__(void) {
	if (LogFile::getDefaultInstance().getVerbosity() >= TRACELEVEL+1) {
	    log_trace("returning");
	}
    }
};

#if defined(__cplusplus) && defined(__GNUC__)
#define GNASH_REPORT_FUNCTION   \
    __Host_Function_Report__ __host_function_report__( __PRETTY_FUNCTION__)
#define GNASH_REPORT_RETURN
#else
#define GNASH_REPORT_FUNCTION \
    log_trace("entering")

#define GNASH_REPORT_RETURN \
    log_trace("returning")
#endif

}


#endif // GNASH_LOG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
