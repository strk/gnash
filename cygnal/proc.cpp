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

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <csignal>
#include <iostream>
#include <cstdlib>

#include "log.h"
#include "crc.h"
#include "proc.h"
#include "network.h"

using namespace std;
using namespace gnash;

namespace cygnal
{

LogFile& dbglogfile = LogFile::getDefaultInstance();
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

Proc::Proc (void)
{
//    GNASH_REPORT_FUNCTION;
}

Proc::~Proc (void)
{
//    GNASH_REPORT_FUNCTION;
}

bool
Proc::startCGI(void)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return false;
}

Proc&
Proc::getDefaultInstance()
{
//    GNASH_REPORT_FUNCTION;
    static Proc c;
    return c;
}


bool
Proc::startCGI(const string &filespec, std::uint16_t port)
{
//    GNASH_REPORT_FUNCTION;
    return startCGI(filespec, false, port);
}

bool
Proc::startCGI(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    return startCGI(filespec, false, 0);
}

bool
Proc::startCGI(const string &filespec, bool outflag)
{

    return startCGI(filespec, outflag, 0);
}

bool
Proc::startCGI(const string &filespec, bool outflag, std::uint16_t port)
{
//    GNASH_REPORT_FUNCTION;
    struct stat procstats;
    pid_t childpid;
    char *cmd_line[20];
    
    _output[filespec] = outflag;

    string path;
    if (crcfile.getCgiRoot().size() > 0) {
        path = crcfile.getCgiRoot().c_str();
        log_debug("Document Root for CGI files is: %s", path);
    } else {
        // Yes, I know this is a hack.
        path = "/var/www/html/cygnal/cgi-bin";
    }
//    string path = filespec;
    path += filespec;
        
    // simple debug junk
    log_debug("Starting \"%s\"", path);

    // See if the file actually exists, otherwise we can't spawn it
    if (stat(path.c_str(), &procstats) == -1) {
        log_error(_("Invalid filespec for CGI: \"%s\""), path);
//        perror(filespec.c_str());
	return (false);
    }

    // setup a command line. By default, argv[0] is the name of the process
    cmd_line[0] = new char[filespec.size()+1];
    strcpy(cmd_line[0], filespec.c_str());

    // If the parent has verbosity on, chances are the child should too.
//     if (dbglogfile.getVerbosity() > 0) {
    cmd_line[1] = new char[3];
    strcpy(cmd_line[1], "-n");
    cmd_line[2] = new char[4];
    strcpy(cmd_line[2], "-vv");
    cmd_line[3] = nullptr;
//     }
    
    // When running multiple cgis, we prefer to specify the port it's using.
    if (port > 0) {
        cmd_line[3] = new char[3];
        strcpy(cmd_line[3], "-p");
        cmd_line[4] = new char[10];
        sprintf(cmd_line[4], "%d", port);
        cmd_line[5] = nullptr;
    }


    // fork ourselves silly
    childpid = fork();
    
//    std::lock_guard<std::mutex> lock(_mutex);
    
    // childpid is a positive integer, if we are the parent, and fork() worked
    if (childpid > 0) {
	_pids[filespec] = childpid;
        return (true);
    }
    
    // childpid is -1, if the fork failed, so print out an error message
    if (childpid == -1) {
        // fork() failed
	perror(filespec.c_str());
	return (false);
    }

    // If we are the child, exec the new process, then go away
    if (childpid == 0) {
	// Turn off all output, if requested
	if (outflag == false) {
	    close(1);
	    open("/dev/null", O_WRONLY);
	    close(2);
	    open("/dev/null", O_WRONLY);
	}
	// Start the desired executable
	execv(path.c_str(), cmd_line);
	perror(path.c_str());
	exit(EXIT_SUCCESS);
    }
    
    return (true);
}

int
Proc::findCGI(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Finding \"%s\"", filespec);
    std::lock_guard<std::mutex> lock(_mutex);

    return _pids[filespec];
}

bool
Proc::stopCGI(void)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    std::lock_guard<std::mutex> lock(_mutex);

    return false;
}
    
bool
Proc::stopCGI(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Stopping \"%s\"", filespec);

    std::lock_guard<std::mutex> lock(_mutex);
    pid_t pid = _pids[filespec];
    
    if (kill (pid, SIGQUIT) == -1) {
	return (false);
    } else {
	return (true);
    }
}
 
bool
Proc::setOutput(const string &filespec, bool outflag)
{
//    GNASH_REPORT_FUNCTION;
    std::lock_guard<std::mutex> lock(_mutex);
    _output[filespec] = outflag;
    
    return (true);
}

bool
Proc::getOutput(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    std::lock_guard<std::mutex> lock(_mutex);
    
    return _output[filespec];
}

bool
Proc::connectCGI (const string &host, std::uint16_t port)
{
//    GNASH_REPORT_FUNCTION;
    return createClient(host, port);
}


} // end of cygnal namespace
