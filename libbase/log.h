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

#ifndef GNASH_LOG_H
#define GNASH_LOG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#ifdef HAVE_LIBXML
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#endif

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

class Verbose {
    int level;
public:
    Verbose(int l) { level = l; }
    friend std::ostream& operator<<(std::ostream&, Verbose&);
};

// This is a basic file logging class
class LogFile {
public:
    LogFile (void);
    LogFile (const char *);
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
    // These both resolve to an unsigned int.
    // LogFile& operator << (size_t x);
    // LogFile& operator << (time_t x);
    LogFile& operator << (float x);
    LogFile& operator << (double &x);
    LogFile& operator << (bool x);
    LogFile& operator << (void *);
    LogFile& operator << (const char *);
    LogFile& operator << (unsigned char const *);
    LogFile& operator << (std::string &s);
// #ifdef HAVE_LIBXML
//     LogFile& operator << (const xmlChar *c);
// #endif
    std::ostream& operator << (std::ostream & (&)(std::ostream &));
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


unsigned char *hexify(unsigned char *p, const unsigned char *s, int length, bool ascii);

// Printf-style interfaces.
#if defined(_WIN32) || defined(WIN32)
void log_msg(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_warning(const char* fmt, ...);
void log_trace(const char* fmt, ...);
void log_action(const char* fmt, ...);
void log_parse(const char* fmt, ...);
#else
void log_msg(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_error(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_warning(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_trace(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_action(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_parse(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
#endif

// Undefine this to completely remove parse debugging at compile-time
#define VERBOSE_PARSE 1

// Undefine this to completely remove action debugging at compile-time
#define VERBOSE_ACTION 1

#ifdef VERBOSE_PARSE
#define IF_VERBOSE_PARSE(x) do { if ( dbglogfile.getParserDump() ) { x; } } while (0);
#else
#define IF_VERBOSE_PARSE(x)
#endif

#ifdef VERBOSE_ACTION
#define IF_VERBOSE_ACTION(x) do { if ( dbglogfile.getActionDump() ) { x; } } while (0);
#else
#define IF_VERBOSE_ACTION(x)
#endif

extern LogFile dbglogfile;

class __Host_Function_Report__ {
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
	if (dbglogfile.getVerbosity() >= TRACELEVEL+1) {
	    log_trace("returning");
	}
    }
};

#ifdef __cplusplus
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
