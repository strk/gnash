// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <signal.h>
#include <iostream>
#include <cstdlib>

#include "log.h"
#include "proc.h"

using namespace std;
using namespace gnash;

Proc::Proc (void)
{
//    GNASH_REPORT_FUNCTION;
}

Proc::~Proc (void)
{
//    GNASH_REPORT_FUNCTION;
}

bool
Proc::Start (void)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return false;
}

bool
Proc::Start (string procname)
{
//    GNASH_REPORT_FUNCTION;
    return Start (procname, false);
}

bool
Proc::Start (string procname, bool b)
{
//    GNASH_REPORT_FUNCTION;
    struct stat procstats;
    pid_t childpid;
    char *cmd_line[20];
    
    _output[procname] = b;

    // simple debug junk
    log_debug("Starting \"%s\"", procname);

    // See if the file actually exists, otherwise we can't spawn it
    if (stat(procname.c_str(), &procstats) == -1) {
        log_error("Invalid filename \"%s\"", procname);
//        perror(procname.c_str());
	return (false);
    }

    // setup a command line. By default, argv[0] is the name of the process
    cmd_line[0] = new char(50);
    strcpy(cmd_line[0], procname.c_str());

    // fork ourselves silly
    childpid = fork();
    
    boost::mutex::scoped_lock lock(_mutex);
    
    // childpid is a positive integer, if we are the parent, and fork() worked
    if (childpid > 0) {
	_pids[procname] = childpid;
        return (true);
    }
    
    // childpid is -1, if the fork failed, so print out an error message
    if (childpid == -1) {
        // fork() failed
	perror(procname.c_str());
	return (false);
    }

    // If we are the child, exec the new process, then go away
    if (childpid == 0) {
	// Turn off all output, if requested
	if (b == false) {
	    close(1);
	    open("/dev/null", O_WRONLY);
	    close(2);
	    open("/dev/null", O_WRONLY);
	}
	// Start the desired executable
	execv(procname.c_str(), cmd_line);
	perror(procname.c_str());
	exit(0);
    }
    
    return (true);
}

int
Proc::Find (string procname)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Finding \"%s\"", procname);    
    boost::mutex::scoped_lock lock(_mutex);

    return _pids[procname];
}

bool
Proc::Stop (void)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    boost::mutex::scoped_lock lock(_mutex);

    return false;
}
    
bool
Proc::Stop (string procname)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Stopping \"%s\"", procname);

    boost::mutex::scoped_lock lock(_mutex);
    pid_t pid = _pids[procname];
    
    if (kill (pid, SIGQUIT) == -1) {
	return (false);
    } else {
	return (true);
    }
}
 
bool
Proc::SetOutput (string procname, bool b)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _output[procname] = b;
    
    return (true);
}

bool
Proc::GetOutput (string procname)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    
    return _output[procname];
}
