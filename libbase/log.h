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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
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
	    //      flush();
	    //      state = CLOSED;
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
    LogFile& operator << (std::string &s);
#ifdef HAVE_LIBXML
    LogFile& operator << (const xmlChar *c);
#endif
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
    bool		 _stamp;
    bool		 _write;
    std::string		 _filespec;
    std::string		 _logentry;
    friend std::ostream & operator << (std::ostream &os, LogFile& e);
};

// Printf-style interfaces.
void log_msg(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_error(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void log_warning(const char* fmt, ...) __attribute__((format (printf, 1, 2)));

extern LogFile dbglogfile;

struct __Host_Function_Report__ {
    const char *func;

    __Host_Function_Report__(void) {
        dbglogfile << "TRACE: enter" << std::endl;
    }

    __Host_Function_Report__(char *_func) {
        func = _func;
        dbglogfile << func << " enter" << std::endl;
    }

    __Host_Function_Report__(const char *_func) {
        func = _func;
        dbglogfile << "TRACE: " << func << " enter" << std::endl;
    }

    ~__Host_Function_Report__(void) {
        dbglogfile << "TRACE: " << func << " return" << std::endl;
    }
};

#ifdef __cplusplus
#define GNASH_REPORT_FUNCTION   \
    __Host_Function_Report__ __host_function_report__( __PRETTY_FUNCTION__)
#define GNASH_REPORT_RETURN
#else
#define GNASH_REPORT_FUNCTION \
    log_msg"TRACE: %s entering at %d\n", __PRETTY_FUNCTION__, __LINE__)

#define GNASH_REPORT_RETURN \
    log_msg"TRACE: %s returning at %d\n", __PRETTY_FUNCTION__, __LINE__)
#endif

}


#endif // GNASH_LOG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
