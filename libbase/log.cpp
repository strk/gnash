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

/* $Id: log.cpp,v 1.61 2008/02/19 00:47:17 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#if defined(_WIN32) && defined(WIN32)
// Required for SYSTEMTIME definitions
# include <windows.h>
# include <sys/types.h>
# include <unistd.h> 
#else
# include <unistd.h>
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <ctime>
/* #define BOOST_HAS_THREADS 1
   This should be set by boost itself.
*/


#include "log.h"

using namespace std;


namespace gnash {

// static data to be shared amongst all classes.
ofstream LogFile::_console;
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
ostream&
timestamp(ostream& x) {
    time_t t;
    char buf[10];

    memset (buf, '0', 10);        // this terminates the string
    time (&t);                    // get the current time
    strftime (buf, sizeof(buf), "%H:%M:%S", localtime (&t));

    return x << buf << ": ";
}


string
timestamp() {

    time_t t;
    char buf[10];

    memset (buf, '0', 10);        // this terminates the string
    time (&t);                    // get the current time
    strftime (buf, sizeof(buf), "%H:%M:%S", localtime (&t));

    std::stringstream ss;
    ss << getpid() << "] " << buf;
    return ss.str();
    //string sbuf = buf;
    //return sbuf;
}

ostream& datetimestamp(ostream& x) {
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

// Printf-style log for debugging info.
void
log_msg(const char* fmt, ...)
{
    va_list ap;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    dbglogfile.log(tmp);

    va_end (ap);
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

void
log_debug(const char* fmt, ...)
{

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

const char *
LogFile::getEntry(void)
{
    return _logentry.c_str();
}

void
LogFile::log(const char* msg)
{
    boost::mutex::scoped_lock lock(_ioMutex);

    dbglogfile << msg << endl;

}

void
LogFile::log(const char* label, const char* msg)
{
    boost::mutex::scoped_lock lock(_ioMutex);

    dbglogfile << label << ": " << msg << endl;

}

// Default constructor
LogFile::LogFile (void)
	:
	_state(CLOSED),
	_stamp(true),
	_write(true),
	_trace(false)
{
    string loadfile;


    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    //rcfile.dump();
    _write = rcfile.useWriteLog();
    loadfile = rcfile.getDebugLog();
    if ( loadfile.empty() ) loadfile = DEFAULT_LOGFILE;

    // TODO: expand ~ to getenv("HOME") !!

    openLog(loadfile);
}

bool
LogFile::openLog (const std::string& filespec)
{
    boost::mutex::scoped_lock lock(_ioMutex);
    if (_state == OPEN) {
	_outstream.close ();
	_state = CLOSED;
    }

    // Append, don't truncate, the log file
    _outstream.open (filespec.c_str(), ios::app); // ios::out
    if( ! _outstream ) {
	// Can't use log_error here...
        std::cerr << "ERROR: can't open debug log file " << filespec << " for appending." << std::endl;
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
    _logentry.clear();

    return true;
}

/// \brief print a char
LogFile&
LogFile::operator << (char x)
{
    if (_verbose) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }

    _state = INPROGRESS;

  return *this;
}

/// \brief print a long
LogFile&
LogFile::operator << (long x)
{
    if (_verbose) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }

    _state = INPROGRESS;

  return *this;
}

/// \brief print an unsigned integer
LogFile&
LogFile::operator << (unsigned int x)
{
    if (_verbose) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }

    _state = INPROGRESS;

    return *this;
}

/// \brief print an unsigned long
LogFile&
LogFile::operator << (unsigned long x)
{
    if (_verbose) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }

    _state = INPROGRESS;

    return *this;
}

/// \brief print a float
LogFile&
LogFile::operator << (float x)
{
    if (_verbose > 0) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }

    _state = INPROGRESS;

  return *this;
}

/// \brief print a double
LogFile&
LogFile::operator << (double &x)
{
    if (_verbose) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }
    _state = INPROGRESS;

    return *this;
}

/// \brief print an integer
LogFile&
LogFile::operator << (int x)
{

    if (_verbose) {
	cout << x;
    }

    if (_write) {
	_outstream << x;
    }

    _state = INPROGRESS;

    return *this;
}

/// \brief print a pointer
LogFile&
LogFile::operator << (void *ptr)
{
    if (_verbose) {
	cout << ptr;
    }

    if (_write) {
	_outstream << ptr;
    }

    _state = INPROGRESS;

    return *this;
}

/// \brief print an STL string
LogFile&
LogFile::operator << (const std::string &s)
{
    if (_verbose) {
	cout << s;
    }

    if (_write) {
	_outstream << s;
    }

    _state = INPROGRESS;

    return *this;
}

/// \brief print a const char * string
///
/// This function is more complex than the other logging functions
/// because it has hooks to look for the TRACE: keyword for logging
/// function calls. We always want these to be logged to the disk
/// file, but optionally printed to the terminal window.
///
/// Since the tracing functions use varargs, they always become a
/// const char * string after processing the arguments. That means we
/// can look for the keyword to determine what to do.
LogFile&
LogFile::operator << (const char *str)
{
    string c(str);

    _logentry = timestamp();
    _logentry += ": ";

    // See if we have the TRACE keyword
    if (strstr(str, N_("DEBUG: "))) {
	_trace = true;
    }

    if (_stamp == true && (_state == IDLE || _state == OPEN)) {
	_state = INPROGRESS;
	if (_trace) {
	    if (_verbose >= TRACELEVEL) {
		cout << _logentry  << c;
	    }
	} else {
	    if (_verbose) {
		cout << _logentry  << c;
	    }
	}
	if (_write) {
	    _outstream << _logentry << c;
	}
    } else {
	if (_trace) {
	    if (_verbose >= TRACELEVEL) {
		cout << c;
	    }
	} else {
	    if (_verbose) {
		cout << c;
	    }
	}
	if (_write) {
	    _outstream << c;
	}
    }
    _logentry += c;

    return *this;
}

LogFile&
LogFile::operator << (unsigned char const *c)
{
    _logentry = timestamp();
    _logentry += ": ";

    if (c == -0) {
      return *this;
    }

    if (_stamp == true && (_state == IDLE || _state == OPEN)) {
	_state = INPROGRESS;
	if (_verbose) {
	    cout << _logentry  << c;
	}
	if (_write) {
	    _outstream << _logentry << c;
	}
    } else {
	if (_verbose) {
	    cout << c;
	}
	if (_write) {
	    _outstream << c;
	}
    }
    _logentry += (const char*)c;

    return *this;
}

/// \brief Grab the endl operator.
LogFile&
LogFile::operator << (std::ostream & (&)(std::ostream &))
{
    if (_trace) {
	if (_verbose) {
	    cout << endl;
	}
    } else {
	if (_verbose) {
	    cout << endl;
	}
    }

    if (_write) {
	_outstream << endl;;
	_outstream.flush();
    }

    _state = IDLE;
    _trace = false;

    return *this;
}

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

