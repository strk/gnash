// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef USE_GTK
#include "gtk_gnash.h"
#endif

#ifdef USE_KDE
#include "kde_gnash.h"
#endif

#ifdef USE_SDL
#include "sdl_gnash.h"
#endif

#ifdef USE_FLTK
#include "fltk_gnash.h"
#endif

#include "md5.h"

using namespace std;

int
main(int argc, char *argv[])
{
    string procname;
    pid_t childpid;
    
    char *gnash_env = std::getenv("GNASH_PLAYER");
    if (!gnash_env) {
      procname = GNASHBINDIR;
      procname += "/hildon-gnash";
    } else {
      procname = gnash_env;
    }
    
    struct stat procstats;

    // See if the file actually exists, otherwise we can't spawn it
    if (stat(procname.c_str(), &procstats) == -1) {
	cout << "Invalid filename: " << procname << endl;
        exit(EXIT_FAILURE);
    }

    // Check the MD5, as a minimal security check
    string md5 = hildon_gnash;
    if (md5_filespec_check(procname, md5)) {
        cout << "MD5 matches for " << procname << endl;
    } else {
        cout << "ERROR: MD5 doesn't match! for " << procname << endl;
        exit(EXIT_FAILURE);
    }
    
    // make a copy of ourself, the child gets replaced by the file to
    // be executed, and the parent goes away.
    childpid = fork();
    
    // childpid is -1, if the fork failed, so print out an error message
    if (childpid == -1) {
      cout << "ERROR: dup2() failed: " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }

    // childpid is a positive integer, if we are the parent, and
    // fork() worked, so exit cleanly
    if (childpid > 0) {
      cout << "Forked sucessfully, child process PID is " << childpid << endl;
      exit(EXIT_SUCCESS);
    }
    
    // setup the command line for the executable we want to launch
    long win_xid;
    int win_height, win_width;
    string base_url = ".";
    
    const size_t buf_size = 30;
    char xid[buf_size], width[buf_size], height[buf_size];
    snprintf(xid, buf_size, "%ld", win_xid);
    snprintf(width, buf_size, "%d", win_width);
    snprintf(height, buf_size, "%d", win_height);

    // REMEMBER TO INCREMENT THE maxargc COUNT IF YOU
    // ADD NEW ARGUMENTS
    const size_t maxargc = 16;
    char **sub_argv = new char *[maxargc];

    size_t sub_argc = 0;
    sub_argv[sub_argc++] = const_cast<char*>( procname.c_str() );
    // don't specify rendering flags, so that the rcfile
    // will control that 
    // sub_argv[sub_argc++] = "-r";
    // sub_argv[sub_argc++] = "3";
    sub_argv[sub_argc++] = "-v";
    sub_argv[sub_argc++] = "-x";
    sub_argv[sub_argc++] = xid;
    sub_argv[sub_argc++] = "-j";
    sub_argv[sub_argc++] = width;
    sub_argv[sub_argc++] = "-k";
    sub_argv[sub_argc++] = height;
    sub_argv[sub_argc++] = "-u";
    sub_argv[sub_argc++] = const_cast<char*>( base_url.c_str() );
    sub_argv[sub_argc++] = "--version";
    sub_argv[sub_argc++] = 0;

    assert(sub_argc <= maxargc);

    // Start the desired executable and go away
    cout << "Starting process: ";

    for (int i=0; sub_argv[i] != 0; ++i) {
      cout << sub_argv[i] << " ";
    }
    cout << endl;

    execv(sub_argv[0], sub_argv);
    // if execv returns, an error has occurred.
    perror(strerror(errno));

    delete[] sub_argv;

    exit (0);    
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
