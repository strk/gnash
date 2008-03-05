// log.cpp:  Message logging functions, for gnash.
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
//


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
//#include <boost/format.hpp>

#if defined(_WIN32) && defined(WIN32)
// Required for SYSTEMTIME definitions
# include <windows.h>
# include <sys/types.h>
# include <unistd.h> 
#else
# include <unistd.h>
#endif

#include <ctime>

#include "log.h"

using std::cout;
using std::endl;

namespace gnash {

// static data to be shared amongst all classes.
//ofstream LogFile::_console;
int LogFile::_verbose = 0;
bool LogFile::_actiondump = false;
bool LogFile::_parserdump = false;

// Workspace for vsnprintf formatting.
static const int BUFFER_SIZE = 2048;

// Convert each byte into it's hex represntation
static const char hexchars[]="0123456789abcdef";
unsigned char *
hexify(unsigned char *p, const unsigned char *s, int length, bool ascii) {

    unsigned char *p1 = p;

    // convert some characters so it'll look right in the log
    for (int i=0 ; i<length; i++) {
        // use the hex value

	if (ascii) {
	    if (isprint(s[i])) {
		*p++ = s[i];
		continue;
	    } else {
 		if ((s[i] == 0xd) || (s[i] == 0xa)) {
		    *p++ = s[i];
 		    continue;
 		}
		*p++ = '^';
	    }
	} else {		// if not ascii outout requested
	    *p++ = hexchars[s[i] >> 4];
	    *p++ = hexchars[s[i] & 0xf];
	    *p++ = ' ';		// add a space between bytes
	}
	    
// 	if (isascii(s[i]) && ascii) {
// 	    *p++ = hexchars[s[i] >> 4];
// 	    *p++ = hexchars[s[i] & 0xf];
// //		    *p++ = ' ';
// 	    *p++ = '%';
// 	    continue;
// 	}
// 	*p++ = s[i];
// // 	    if (!isprint(s[i+1])) {
// // 		*p++ = hexchars[s[i] >> 4];
// // 		*p++ = hexchars[s[i] & 0xf];
// // //		*p++ = ' ';
// // 		*p++ = '$';
// // 	    }

//     } else {
// 	    if (ascii) {
// 		if (s[i] == 0xd) {
// //		    *p++ = '\r';
// 		    *p++ = '@';
// 		    continue;
// 		}
// 		if (s[i] == 0xa) {		
// //		    *p++ = '\n';
// 		    *p++ = '#';
// 		    continue;
// 		}
// 	    } else {
// 		*p++ = hexchars[s[i] >> 4];
// 		*p++ = hexchars[s[i] & 0xf];
// 	    }
// 	}
    }

    *p = '\0';

    return p1;
}

// FIXME: localize these, so they print local regional timestamps.
std::ostream&
timestamp(std::ostream& x) {
    time_t t;
    char buf[10];

    memset (buf, '0', 10);        // this terminates the string
    time (&t);                    // get the current time
    strftime (buf, sizeof(buf), "%H:%M:%S", localtime (&t));

    return x << buf << ": ";
}


std::string
timestamp() {

    time_t t;
    char buf[10];

    memset (buf, '0', 10);        // this terminates the string
    time (&t);                    // get the current time
    strftime (buf, sizeof(buf), "%H:%M:%S", localtime (&t));

    std::stringstream ss;
    ss << getpid() << "] " << buf;
    return ss.str();
}

std::ostream& datetimestamp(std::ostream& x) {
    time_t t;
    char buf[20];

    memset (buf, '0', 20);        // this terminates the string
    time (&t);                    // get the current time
    strftime (buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", localtime (&t));

    return x << buf;
}

// This is a bit of a hack. We implement wrappers for the old
// functions so we don't have to change files everywhere, but get the
// new behaviours, like logging to disk.
// THIS IS DANGEROUS AS TIME OF INITIALIZATION IS UNPREDICTABLE,
// THUS WE NOW HAVE A LogFile::getDefaultInstance() TO MAKE SURE
// INITIALIZATION OF THE GLOBAL data HAPPENS BEFORE USE
LogFile&
LogFile::getDefaultInstance()
{
	static LogFile o;
	return o;
}

namespace {
    LogFile& dbglogfile = LogFile::getDefaultInstance();
}

void
log_trace(const char* fmt, ...)
{

    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(_("TRACE"), tmp);

    va_end (ap);
}

//void
//logTrace(const boost::format& fmt)
//{
//    dbglogfile.log(N_("TRACE"), fmt.str());
//}

void
log_debug(const char* fmt, ...)
{

    if (dbglogfile.getVerbosity() < DEBUGLEVEL) return;

    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    // We don't translate DEBUG: because code below here looks for it
    // in the output of const char strings.  If we translated it, both
    // its type would change (to non-const char string) and the letters would
    // change to the local language.  Could perhaps be fixed more cleanly
    // later...
    dbglogfile.log(N_("DEBUG"), tmp);

    va_end (ap);
}

//void
//logDebug(const boost::format& fmt)
//{
//    if (dbglogfile.getVerbosity() < DEBUGLEVEL) return;
//    dbglogfile.log(N_("DEBUG"), fmt.str());
//}

void
log_action(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    bool stamp = dbglogfile.getStamp();
    dbglogfile.setStamp(false);
    dbglogfile.log(tmp);
    dbglogfile.setStamp(stamp);
}

//void
//logAction(const boost::format& fmt)
//{
//    bool stamp = dbglogfile.getStamp();
//    dbglogfile.setStamp(false);
//    dbglogfile.log(fmt.str());
//    dbglogfile.setStamp(stamp);
//}

void
log_parse(const char* fmt, ...)
{

    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(tmp);

    va_end (ap);
}

//void
//logParse(const boost::format& fmt)
//{
//    dbglogfile.log(fmt.str());
//}

// Printf-style error log.
void
log_error(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(_("ERROR"), tmp);

    va_end (ap);
}

//void
//logError(const boost::format& fmt)
//{
//    dbglogfile.log(N_("ERROR"), fmt.str());
//}

void
log_unimpl(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE-1, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(_("UNIMPLEMENTED"), tmp);

    va_end (ap);
}

//void
//logUnimpl(const boost::format& fmt)
//{
//    dbglogfile.log(N_("UNIMPLEMENTED"), fmt.str());
//}

void
log_security(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE-1, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(_("SECURITY"), tmp);

    va_end (ap);
}

//void
//logSecurity(const boost::format& fmt)
//{
//    dbglogfile.log(N_("SECURITY"), fmt.str());
//}

void
log_swferror(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE-1, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(_("MALFORMED SWF"), tmp);

    va_end (ap);
}

//void
//logSWFError(const boost::format& fmt)
//{
//    dbglogfile.log(N_("MALFORMED SWF"), fmt.str());
//}

void
log_aserror(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE-1, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(_("ACTIONSCRIPT ERROR"), tmp);

    va_end (ap);
}

//void
//logASError(const boost::format& fmt)
//{
//    dbglogfile.log(N_("ACTIONSCRIPT ERROR"), fmt.str());
//}

void
LogFile::log(const std::string& msg)
{
    boost::mutex::scoped_lock lock(_ioMutex);

    dbglogfile << msg << endl;

}

void
LogFile::log(const std::string& label, const std::string& msg)
{
    boost::mutex::scoped_lock lock(_ioMutex);

    dbglogfile << label << ": " << msg << endl;

}

// Default constructor
LogFile::LogFile ()
	:
	_state(CLOSED),
	_stamp(true),
	_write(false)
{
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    _write = rcfile.useWriteLog();

}

LogFile::~LogFile()
{
	if (_state == OPEN) closeLog();
}

bool
LogFile::openLogIfNeeded ()
{
    if (_state != CLOSED) return true;
    if (!_write) return false;

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    std::string loadfile = rcfile.getDebugLog();
    if ( loadfile.empty() ) loadfile = DEFAULT_LOGFILE;

    // TODO: expand ~ to getenv("HOME") !!

    return openLog(loadfile);
}

bool
LogFile::openLog (const std::string& filespec)
{
    // NOTE:
    // don't need to lock the mutex here, as this method
    // is intended to be called only by openLogIfNeeded,
    // which in turn is called by operator<< which is called
    // by the public log_xxx functions that log themselves

    if (_state != CLOSED) {
	cout << "Closing previously opened stream" << endl;
        _outstream.close ();
        _state = CLOSED;
    }

    // Append, don't truncate, the log file
    _outstream.open (filespec.c_str(), std::ios::app|std::ios::out); // ios::out
    if( _outstream.fail() ) {
	// Can't use log_error here...
        cout << "ERROR: can't open debug log file " << filespec << " for appending." << endl;
        return false;
    }       

    _filespec = filespec;
    _state = OPEN;

  // LogFile::outstream << "Opened " << filespec << endl;

  return true;
}

bool
LogFile::closeLog (void)
{
    boost::mutex::scoped_lock lock(_ioMutex);
    if (_state == OPEN) {
           _outstream.flush();
        _outstream.close();
    }
    _state = CLOSED;

    return true;
}

bool
LogFile::removeLog (void)
{
    if (_state == OPEN) {
        _outstream.close ();
    }

    // Ignore the error, we don't care
    unlink(_filespec.c_str());
    _filespec.clear();

    return true;
}

/// \brief print a string
///


LogFile&
LogFile::operator << (const std::string &s)
{
    // NOTE: _state will be == INPROGRESS right

    if (_stamp == true && (_state != INPROGRESS) )
    {
	std::string ts = timestamp();

        if (_verbose) cout << ts << ": " << s;
        if (openLogIfNeeded())
        {
		_outstream << ts << ": " << s;
	}
        _state = INPROGRESS;
    }
    else
    {
        if (_verbose) cout << s;
        if (openLogIfNeeded()) {
		_outstream << s;   
	}
    }

    return *this;
}

/// \brief Grab the endl operator.
LogFile&
LogFile::operator << (std::ostream & (&)(std::ostream &))
{

    if (_verbose) cout << endl;

    if (openLogIfNeeded())
    {
        _outstream << endl;;
        _outstream.flush();
    }

    _state = IDLE;

    return *this;
}

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

