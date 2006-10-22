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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#ifdef __STDC_HOSTED__
#include <sstream>
#else
#include <strstream>
#endif
#include <cstdio>

#include "log.h"
#include "rc.h"
#include "shm.h"

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
        shm_unlink("/lc_test");
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
            cout << "Sorry, we can only list the files on systems with";
            cout << "with disk based shared memory" << endl;
        }
        exit(0);
    }

    //Destroy shared memory segments
    if (nuke) {
        if (filespec.size() == 0) {
            cout << "No name specified, nuking everything..." << endl;
            for (i=0; entry>0; i++) {
                entry = readdir(library_dir);
                if (entry != NULL) {
                    tmpname = "/"; // prefix a / so shm_unlink can
                                   // use the correct path
                    tmpname += entry->d_name;
                    cout << "Removing segment: " << tmpname << endl;
                    shm_unlink(tmpname.c_str());
                }
            }
            exit(0);
        } else {
            cout << "Nuking the shared memory segment " << filespec << endl;
            shm_unlink(filespec.c_str());
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

#ifdef __STDC_HOSTED__
    in.open(realname.c_str(), ios::binary|ios::in);
#else
    in.open(realname.c_str(), ios::binary|ios::in|ios::nocreate);
#endif
    
    if (!in.eof()) {
         if (!in.read(reinterpret_cast<char*>(&shmptr), sizeof(Shm))) {
             cerr << "ERROR: couldn't read!" << endl;
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
    cerr << "This program dumps the internal data of a shared memory segment"
         << endl;
    cerr << "Usage: dumpmem [hdsanlif] filename" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-d\tDump data" << endl;
    cerr << "-n [optional name]\tNuke everything" << endl;
    cerr << "-l\tLength of segment" << endl;
    cerr << "-i\tList segments" << endl;
    cerr << "-f\tForce to use builtin names for nuke" << endl;
    exit (-1);
}

/// \brief Dumps the internal data of the found ShmControl
///              block. We do our own dumping, rather than letting
///              ShmControl::dump() do it, cause that's for debugging,
///              and this is for user display purposes.
void dump_ctrl(void *inmem)
{
    Shm *ptr = static_cast<Shm *>(inmem);
    
    cerr << "\tBase address of this segment: "
         << static_cast<void *>(ptr->getAddr()) << endl;
    cerr << "\tFilespec: " << ptr->getName() << endl;
    cerr << "\t# Bytes allocated: " << ptr->getAllocated() << endl;
    cerr << "\tTotal # of bytes: " << ptr->getSize() << endl;
}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
