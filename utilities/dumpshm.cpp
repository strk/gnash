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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "GnashSystemIOHeaders.h" // write()
#include <cstdarg>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cerrno>

extern "C"{
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
#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__) && !defined(__amigaos4__)
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#elif !defined(__riscos__) && !defined(__OS2__) && !defined(__amigaos4__)
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
#include <cerrno>

#ifdef ENABLE_NLS
# include <clocale>
#endif

#include "log.h"
#include "rc.h"
#include "shm.h"
#include "amf.h"
#include "lcshm.h"

using namespace std;
using namespace gnash;

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{

void throw_exception(std::exception const & e)
{
    std::abort();
}
}
#endif

// #error "No supported shared memory type for this platform"

static void usage (void);
void dump_ctrl(void *ptr);
void dump_shm(bool convert, bool out);
key_t list_lcs();

const int PIDSTART = 20000;
const int PIDEND   = 23000;
const int LINELEN  = 80;
const unsigned int LOOPCNT  = 5;
const int DEFAULT_SHM_SIZE = 64528;

#ifndef SHM_STAT
const int SHM_STAT = 13;
const int SHM_INFO = 14;
#endif

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

const char *DUMPSHM_VERSION = "0.5";

int
main(int argc, char *argv[])
{
    int                   c;
    bool                  listfiles  = false;
    bool          sysv = false;
    bool          convert = false;
    int                   size  = 0;
    string                filespec, realname, tmpname;
    vector<const char *>  dirlist;
    
    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    std::setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    // scan for the two main standard GNU options
    for (c = 0; c < argc; c++) {
      if (strcmp("--help", argv[c]) == 0) {
      usage();
      exit(EXIT_SUCCESS);
      }
      if (strcmp("--version", argv[c]) == 0) {
      printf (_("Gnash dumpshm version: %s, Gnash version: %s\n"),
          DUMPSHM_VERSION, VERSION);
      exit(EXIT_SUCCESS);
      }
    }
    
    while ((c = getopt (argc, argv, "hircv")) != -1) {
        switch (c) {
          case 'h':
          usage ();
          break;
          
          case 'r':
          sysv = true;
          convert = false;
          break;
          
          case 'c':
          sysv = true;
          convert = true;
          break;
          
          case 'i':
          sysv = true;
          listfiles = true;
          break;
          
          case 'v':
          // turn on verbosity for the libraries
          dbglogfile.setVerbosity();
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
        exit(EXIT_SUCCESS);
    }
    
#if defined(USE_SYSV_SHM) && defined(HAVE_IPC_INFO)
    // Just list the shared memory segments
    if (listfiles && sysv) {
    list_lcs();
        exit(EXIT_SUCCESS);
    }
#endif
    
    if (optind <= argc - 1) {
    if (*argv[optind] == '-') {
        filespec = '-';
    }
    }
    
    if (sysv) {
    if (filespec == "-") {
        dump_shm(convert, true);
    } else {
        dump_shm(convert, false);
    }
    
        exit(EXIT_SUCCESS);
    }    
    
    if (size == 0) {
        size = DEFAULT_SHM_SIZE;
    }

    // get the file name from the command line
    if (optind < argc) {
        filespec = argv[optind];
    if (!convert) {
        log_debug(_("Will use \"%s\" for memory segment file"), filespec);
    }
    }
    
}

// Dump the older style SYS V shared memory segments
void
dump_shm(bool convert, bool out)
{
// These are here for debugging purposes. It
    char *shmaddr;
    
    key_t key = rcfile.getLCShmKey();

    if (key == 0) {
    log_debug(_("No LcShmKey set in ~/.gnashrc, trying to find it ourselves"));
#if defined(USE_SYSV_SHM) && defined(HAVE_IPC_INFO)
    key = list_lcs();
#endif
    }
    
    int size = 64528;            // 1007 bytes less than unsigned

    if (key == 0) {
        log_debug(_("No shared memory segments found!"));
        return;
    }
    if (dbglogfile.getVerbosity()) {
        log_debug(_("Existing SHM Key is: %s, Size is: %s"),
                boost::io::group(hex, showbase, key), size);
    }
    
    amf::LcShm lc;
    lc.connect(key);
    lc.dump();
    
    // If the -c convert options was specified, dump the memory segment to disk.
    // This makes it easy to store them as well as to examine them in great detail.
    if (convert) {
        int fd = open("segment.raw",O_WRONLY|O_CREAT, S_IRWXU);
        if (fd == -1) {
            perror("open");
        }
        log_debug(_("Writing memory segment to disk: \"segment.raw\""));
        shmaddr = lc.getAddr();
        write(fd, shmaddr, size);
        if (out) {
#if 0
            log_debug(_("The data is: 0x%s"), hexify((uint8_t *)shmaddr, size, false));
#endif
        }
        
        close(fd);
    }
    
    exit (EXIT_SUCCESS);
}
    
#if defined(USE_SYSV_SHM) && defined(HAVE_IPC_INFO)
key_t
list_lcs()
{
    int maxid, shmid, id;
    struct shmid_ds shmseg;
    
// #ifdef USE_POSIX_SHM
//     if (library_dir != NULL) {
//     for (i=0; entry>0; i++) {
//         entry = readdir(library_dir);
//         if (entry != NULL) {
//                     cout << "Found segment: " << entry->d_name << endl;
//                 }
//             }
//         } else {
//     cout << _("Sorry, we can only list the files on systems with"
//           " disk based shared memory") << endl;
//     }
// #eendif
    
    // If we're using SYSV shared memory, we can get a list of shared memory segments.
    // By examing the size of each one, we can make a reasonable guess if it's one
    // used for flash. As permissions apply, this will only list the segments created
    // by the user running dumpshm.
//    struct shm_info shm_info;
//    maxid = shmctl(0, SHM_INFO, (struct shmid_ds *) (void *) &shm_info);
    struct shmid_ds shm_info;
    maxid = shmctl(0, SHM_INFO, &shm_info);
    if (maxid < 0) {
    log_debug(_("kernel not configured for shared memory"));
    return 0;
    }
    
//    struct shminfo shminfo;
    if ((shmctl(0, IPC_INFO, &shm_info)) < 0) {
        return 0;
    }
    for (id = 0; id <= maxid; id++) {
        shmid = shmctl(id, SHM_STAT, &shmseg);
        if (shmid < 0) {
            continue;
        }
#ifdef IPC_PERM_KEY
        if (shmseg.shm_segsz == 64528) {
            log_debug(_("Found it! \"set LCShmKey %s\" in your ~/.gnashrc"),
                        boost::io::group(hex, showbase,
                            shmseg.shm_perm.IPC_PERM_KEY));
            log_debug(_("Last changed on: %s"), ctime(&shmseg.shm_ctime));
            log_debug(_("Last attached on: %s"), ctime(&shmseg.shm_atime));
            log_debug(_("Last detached on: %s"), ctime(&shmseg.shm_dtime));
            return shmseg.shm_perm.IPC_PERM_KEY;
        }
#endif    // end of IPC_PERM_KEY
    }
// #else
// # error "No supported shared memory type for this platform"
//#endif    // end of USE_POSIX_SHM
    
    // Didn't find any segments of the right size
    return static_cast<key_t>(0);
}
#endif    // end of USE_SYSV_SHM & HAVE_IPC_INFO

/// \brief  Display the command line arguments
static void
usage (void)
{
    cerr << _("This program dumps the internal data of a shared memory segment")
         << endl;
    cerr << _("Usage: dumpshm [hdsanlif] filename") << endl;
    cerr << _("-h\tHelp") << endl;
    cerr << _("-i\tList segments") << endl;
    cerr << _("-r\tDump SYSV segments") << endl;
    cerr << _("-c\tDump SYSV segments to disk") << endl;
    cerr << _("-v\tVerbose output") << endl;
    exit (EXIT_FAILURE);
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
