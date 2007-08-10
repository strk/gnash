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

// 
//
//


/* $Id: dumpshm.cpp,v 1.13 2007/08/10 14:06:36 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <sys/stat.h>

extern "C"{
	#include <unistd.h>
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
#ifndef __GNUC__
	extern int optind, getopt(int, char *const *, const char *);
	extern char *optarg;
#endif
}
#include <dirent.h>
#include <sys/types.h>
#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__)
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#elif !defined(__riscos__) && !defined(__OS2__)
#include <windows.h>
#include <process.h>
#include <io.h>
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#if defined(__STDC_HOSTED__) || !defined(__GNUC__)
#include <sstream>
#else
#include <strstream>
#endif
#include <cstdio>

#include "log.h"
#include "rc.h"
#include "shm.h"
#include "gnash.h"

using namespace std;
using namespace gnash;

static void usage (void);
void dump_ctrl(void *ptr);

const int PIDSTART = 20000;
const int PIDEND   = 23000;
const int LINELEN  = 80;
const unsigned int LOOPCNT  = 5;
const int DEFAULT_SHM_SIZE = 1024;

int
main(int argc, char *argv[])
{
    unsigned int          i;
    int                   c;
    bool                  dump  = false;
    bool                  nuke  = false;
    bool                  listfiles  = false;
    bool                  force = false;
    int                   size  = 0;
    string                filespec, realname, tmpname;
    struct dirent         *entry;
    vector<const char *>  dirlist;
    
// #ifdef __STDC_HOSTED__
//     ostringstream         *shmnames;
// #else
//     strstream             *shmnames;
// #endif

    // Enable native language support, i.e. internationalization
    setlocale (LC_MESSAGES, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);

    /* This initializes the DBG_MSG macros */ 
    while ((c = getopt (argc, argv, "hdnl:if")) != -1) {
        switch (c) {
          case 'h':
            usage ();
            break;
            
          case 'f':
            force = true;
            break;
            
          case 'i':
            listfiles = true;
            break;
            
          case 'd':
            dump = true;
            break;
            
          case 'n':
            nuke = true;
            break;

          case 'l':
            size = atoi(optarg);
            break;
            
          default:
            usage ();
            break;
        }
    }
    
    
    // If no command line arguments have been supplied, do nothing but
    // print the  usage message.
    if (argc < 2) {
        usage();
        exit(0);
    }
    
    
    if (size == 0) {
        size = DEFAULT_SHM_SIZE;
    }
    
    // get the file name from the command line
    if (optind < argc) {
        filespec = argv[optind];
        cout << "Will use \"" << filespec << "\" for memory segment file"
             << endl;
    }
    
    DIR *library_dir = NULL;

    // Solaris stores shared memory segments in /var/tmp/.SHMD and
    // /tmp/.SHMD. Linux stores them in /dev/shm.
    dirlist.push_back("/dev/shm");
    dirlist.push_back("/var/tmp/.SHMD");
    dirlist.push_back("/tmp/.SHMD");

    // Open the directory where the raw POSIX shared memory files are
    for (i=0; i<dirlist.size(); i++) {
        library_dir = opendir (dirlist[i]);
        if (library_dir != NULL) {
            realname = dirlist[i];
            
            // By convention, the first two entries in each directory are
            // for . and .. (``dot'' and ``dot dot''), so we ignore those. The
            // next directory read will get a real file, if any exists.
            entry = readdir(library_dir);
            entry = readdir(library_dir);
            break;
        }
    }

    // MacOSX, unlike Solaris and Linux doesn't appear to use a file
    // based scheme for shared memory segments. In this case, we can't
    // analyze the files directly, and are forced to explicitly create
    // all the probable names to be deleted.
    if (library_dir == NULL || force && nuke) {
        // These are the names left by the various test cases. Just
        // blindly delete them, because they may not even exist, so
        // the errors can be ignored.
#ifdef HAVE_SHM_UNLINK        
	shm_unlink("/lc_test");
#endif
        exit(0);
    }

    // Just list the shared memory segments
    if (listfiles) {
        if (library_dir != NULL)
        {
            for (i=0; entry>0; i++) {
                entry = readdir(library_dir);
                if (entry != NULL) {
                    cout << "Found segment: " << entry->d_name << endl;
                }
            }
        } else {
            cout << _("Sorry, we can only list the files on systems with"
		      " disk based shared memory") << endl;
        }
        exit(0);
    }

    //Destroy shared memory segments
    if (nuke) {
        if (filespec.size() == 0) {
            cout << _("No name specified, nuking everything...") << endl;
            for (i=0; entry>0; i++) {
                entry = readdir(library_dir);
                if (entry != NULL) {
                    tmpname = "/"; // prefix a / so shm_unlink can
                                   // use the correct path
                    tmpname += entry->d_name;
                    cout << _("Removing segment: ") << tmpname << endl;
#ifdef HAVE_SHM_UNLINK
                    shm_unlink(tmpname.c_str());
#endif
                }
            }
            exit(0);
        } else {
            cout << _("Nuking the shared memory segment ") << filespec << endl;
#ifdef HAVE_SHM_UNLINK 
           shm_unlink(filespec.c_str());
#endif
        }
        
        exit(0);
    }
    
    // Because POSIX shared memory is is file system based, we can
    // open the raw file, which lets us analyze it better without the
    // potential hassle of memory faults.
    ifstream in;
//    int offset;
    Shm shmptr;

    realname += filespec;

#if defined(__STDC_HOSTED__) || !defined(__GNUC__)
    in.open(realname.c_str(), ios::binary|ios::in);
#else
    in.open(realname.c_str(), ios::binary|ios::in|ios::nocreate);
#endif
    
    if (!in.eof()) {
         if (!in.read(reinterpret_cast<char*>(&shmptr), sizeof(Shm))) {
             cerr << _("ERROR: couldn't read!") << endl;
             exit(1);
         }
         dump_ctrl(&shmptr);
         
//        in.read(reinterpret_cast<char*>(mmptr), sizeof(MemManager));
        // This is pretty gross. Because we aren't mapping this segment
        // into memory, we splice in the data we first read to the
        // proper field in the Memory Manager structure, since it
        // references this same data anyway.
//        long *tmpptr = reinterpret_cast<long *>(mmptr);
//        tmpptr[2] = reinterpret_cast<long>(shmptr);
//        mmptr->memControlSet(shmptr);

//        memblks  = new MemBlock [mmptr->blockCountGet()];
        // The Memory Block data is a big array of contiguous memory,
        // so we figure out it's offset from the front of the file and
        // then read from there.
//        offset = reinterpret_cast<char*>(mmptr->memBlocksGet()) -
//            reinterpret_cast<char*>(shmptr->memAddrGet());
//        in.seekg(offset, ios::beg);
//        in.read(reinterpret_cast<char*>(memblks), mmptr->blockCountGet());
//        tmpptr[1] = reinterpret_cast<long>(memblks);        
//        mmptr->memBlocksSet(memblks);
    }
}

/// \brief  Display the command line arguments
static void
usage (void)
{
    cerr << _("This program dumps the internal data of a shared memory segment")
         << endl;
    cerr << _("Usage: dumpmem [hdsanlif] filename") << endl;
    cerr << _("-h\tHelp") << endl;
    cerr << _("-d\tDump data") << endl;
    cerr << _("-n [optional name]\tNuke everything") << endl;
    cerr << _("-l\tLength of segment") << endl;
    cerr << _("-i\tList segments") << endl;
    cerr << _("-f\tForce to use builtin names for nuke") << endl;
    exit (-1);
}

/// \brief Dumps the internal data of the found ShmControl
///              block. We do our own dumping, rather than letting
///              ShmControl::dump() do it, cause that's for debugging,
///              and this is for user display purposes.
void dump_ctrl(void *inmem)
{
    Shm *ptr = static_cast<Shm *>(inmem);
    
    cerr << _("\tBase address of this segment: ")
         << static_cast<void *>(ptr->getAddr()) << endl;
    cerr << _("\tFilespec: ") << ptr->getName() << endl;
    cerr << _("\t# Bytes allocated: ") << ptr->getAllocated() << endl;
    cerr << _("\tTotal # of bytes: ") << ptr->getSize() << endl;
}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
