// log.cpp:  Message logging functions, for gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "log.h"

#include <ctime> 
#include <cctype> 
#include <cstring> 
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip> 
#include <string>
#include <thread>
#include <mutex>
#include <boost/format.hpp>

#include <unistd.h> // for getpid

#include "GnashAlgorithm.h"
#include "ClockTime.h"

using std::cout;
using std::endl;

namespace gnash {

// Convert each byte into its hex representation
std::string
hexify(const unsigned char *p, size_t length, bool ascii)
{

    const std::vector<unsigned char> bytes(p, p + length);

    std::ostringstream ss;
    
    // For hex output, fill single-digit numbers with a leading 0.
    if (!ascii) ss << std::hex << std::setfill('0');
    
    for (const unsigned char& byte : bytes)
        {
        if (ascii) {
            if (std::isprint(byte) || byte == 0xd) {
                ss << byte;
            }
            else ss << ".";
        }
        else  {
            // Not ascii
            ss << std::setw(2) << static_cast<int>(byte) << " ";    
        }
    }    
    
    return ss.str();

}

LogFile&
LogFile::getDefaultInstance()
{
    static LogFile o;
    return o;
}


namespace {

    LogFile& dbglogfile = LogFile::getDefaultInstance();

    struct Timestamp {
        std::uint64_t startTicks;
        std::map<std::thread::id, int> threadMap;
        Timestamp() : startTicks(clocktime::getTicks()) {}
    };

    std::ostream& operator<< (std::ostream& o, Timestamp& t)
    {
        std::thread::id tid = std::this_thread::get_id();
        int& htid = t.threadMap[tid];
        if (!htid) {
            htid = t.threadMap.size();
            // TODO: notify actual thread id for index
        }

        std::uint64_t diff = clocktime::getTicks() - t.startTicks;
        // should we split in seconds/ms ?
        o << "[" << getpid() << ":" << htid << "] " << diff;

        return o;

    }

    Timestamp timestamp;

}

// boost format functions to process the objects
// created by our hundreds of templates 

void
processLog_trace(const boost::format& fmt)
{
    dbglogfile.log(N_("TRACE"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_debug(const boost::format& fmt)
{
    if (dbglogfile.getVerbosity() < LogFile::LOG_DEBUG) return;
    dbglogfile.log(N_("DEBUG"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_abc(const boost::format& fmt)
{
    if (dbglogfile.getVerbosity() < LogFile::LOG_EXTRA) return;
    dbglogfile.log(N_("ABC"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_VERBOSE, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_parse(const boost::format& fmt)
{
    dbglogfile.log(fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_VERBOSE, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_network(const boost::format& fmt)
{
    dbglogfile.log(N_("NETWORK"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_error(const boost::format& fmt)
{
    dbglogfile.log(N_("ERROR"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_ERROR, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_unimpl(const boost::format& fmt)
{
    dbglogfile.log(N_("UNIMPLEMENTED"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_security(const boost::format& fmt)
{
    dbglogfile.log(N_("SECURITY"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_swferror(const boost::format& fmt)
{
    dbglogfile.log(N_("MALFORMED SWF"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_aserror(const boost::format& fmt)
{
    dbglogfile.log(N_("ACTIONSCRIPT ERROR"), fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, "Gnash", fmt.str().c_str());
#endif    
}

void
processLog_action(const boost::format& fmt)
{
    bool stamp = dbglogfile.getStamp();
    dbglogfile.setStamp(false);
    dbglogfile.log(fmt.str());
    dbglogfile.setStamp(stamp);
}

void
LogFile::log(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(_ioMutex);

    if ( !_verbose ) return; // nothing to do if not verbose

    if (openLogIfNeeded()) {
        if (_stamp) {
            _outstream << timestamp << ": " << msg << "\n";
        } else {
            _outstream << msg << "\n";
        }
    }
    else {
        // log to stdout
        if (_stamp) {
            cout << timestamp << " " << msg << endl;
        } else {
            cout << msg << endl;
        }
    }
    
    if (_listener) {
        (*_listener)(msg);
    }
}

inline void
LogFile::log(const std::string& label, const std::string& msg)
{
    log(label + ": " + msg);
}

void
LogFile::setLogFilename(const std::string& fname)
{
    closeLog();
    _logFilename = fname;
}

void
LogFile::setWriteDisk(bool use)
{
    if (!use) closeLog();
    _write = use;
}

// Default constructor
LogFile::LogFile()
    :
    _verbose(0),
    _actiondump(false),
    _network(false),
    _parserdump(false),
    _state(CLOSED),
    _stamp(true),
    _write(false),
    _listener(nullptr)
{
}

LogFile::~LogFile()
{
    if (_state == OPEN) closeLog();
}

bool
LogFile::openLogIfNeeded()
{
    if (_state != CLOSED) return true;
    if (!_write) return false;

    if (_logFilename.empty()) _logFilename = DEFAULT_LOGFILE;

    // TODO: expand ~ to getenv("HOME") !!

    return openLog(_logFilename);
}

bool
LogFile::openLog(const std::string& filespec)
{

    // NOTE:
    // don't need to lock the mutex here, as this method
    // is intended to be called only by openLogIfNeeded,
    // which in turn is called by operator<< which is called
    // by the public log_xxx functions that log themselves

    if (_state != CLOSED) {
    cout << "Closing previously opened stream" << endl;
        _outstream.close();
        _state = CLOSED;
    }

    // Append, don't truncate, the log file
    _outstream.open(filespec.c_str(), std::ios::app|std::ios::out); // ios::out
    if( _outstream.fail() ) {
    // Can't use log_error here...
        cout << "ERROR: can't open debug log file " << filespec << 
            " for appending." << endl;
        return false;
    }       

    _filespec = filespec;
    _state = OPEN;

    return true;
}

bool
LogFile::closeLog()
{
    std::lock_guard<std::mutex> lock(_ioMutex);

    if (_state == OPEN) {
        _outstream.flush();
        _outstream.close();
    }
    _state = CLOSED;

    return true;
}

bool
LogFile::removeLog()
{
    if (_state == OPEN) {
        _outstream.close();
    }

    // Ignore the error, we don't care
    unlink(_filespec.c_str());
    _filespec.clear();

    return true;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

