// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__)
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#elif !defined(__riscos__) && !defined(__OS2__)
#include <windows.h>
#include <process.h>
#include <io.h>
#endif
#include <string>
#include <vector>
#include <cerrno>

#include "log.h"
#include "SharedMem.h"

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

using namespace std;

namespace gnash {

const int DEFAULT_SHM_SIZE = 64528;

#ifdef darwin
# ifndef MAP_INHERIT
# define MAP_INHERIT 0
#endif
#ifndef PROT_EXEC
# define PROT_EXEC
# endif
#endif

#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 8
#endif

#define FLAT_ADDR_SPACE 1

Shm::Shm()
    :
    _addr(0),
    _alloced(0),
    _size(0),
    _shmkey(0),
    _shmfd(0)
{
    memset(_filespec, 0, MAX_SHM_NAME_SIZE);
}

Shm::~Shm()
{
}

bool
Shm::attach()
{
    
#if (defined(USE_SYSV_SHM) && defined(HAVE_SHMGET)) || defined(_WIN32)
    // this is the magic size of shared memory segments used by the
    // Flash player;
    _size = 64528;
    
    // If there is no SYSV style shared memory key in the users ~/.gnashrc
    // file, warn them
    // that compatibility will be broken, and then just pick our own key so
    // things still work finer when using just Gnash.
    if (_shmkey == 0) {
        _shmkey = 0xdd3adabd;
    }
    
#ifndef _WIN32
    {
	const int shmflg = 0660 | IPC_CREAT;

	_shmfd = shmget(_shmkey, _size, shmflg);
	if (_shmfd < 0 && errno == EEXIST) {
	    // Get the shared memory id for this segment
	    _shmfd = shmget(_shmkey, _size, 0);
	}
	_addr = static_cast<char *>(shmat(_shmfd, 0, 0));
	if (_addr <= 0) {
	    log_debug("WARNING: shmat() failed: %s\n", strerror(errno));
	    return false;
	}
    }
#else
    _shmhandle = CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL,
	    PAGE_READWRITE, 0, _size, NULL);
    if (_shmhandle == NULL) {
	log_debug("WARNING: CreateFileMapping failed: %ld\n", GetLastError());
        return false;
    }
    _addr = static_cast<char *>(MapViewOfFile(_shmhandle, FILE_MAP_ALL_ACCESS,
            0, 0, _size));
    if (_addr == NULL) {
	log_debug("WARNING: MapViewOfFile() failed: %ld\n", GetLastError());
	return false;
    }
#endif

    return true;
#else
#error "You need SYSV Shared memory support to use this option"
#endif	 // end of USE_SYSV_SHM
}	


// Close the memory segment. This removes it from the system.
bool
Shm::closeMem()
{
//    GNASH_REPORT_FUNCTION;
    // Only nuke the shared memory segement if we're the last one.
#ifdef USE_POSIX_SHM
#ifdef HAVE_SHM_UNLINK
    if (strlen(_filespec) != 0) {
        shm_unlink(_filespec);
    }
    
     // flush the shared memory to disk
     if (_addr > 0) {
         // detach memory
         munmap(_addr, _size);
     }
#endif
#ifdef USE_SYSV_SHM
     shmctl(_shmfd, IPC_RMID, 0);
#else
# ifdef __riscos__
     free(_addr);
# else
     CloseHandle(_shmhandle);
#endif
#endif
#endif
    
    _addr = 0;
    _alloced = 0;
    memset(_filespec, 0, MAX_SHM_NAME_SIZE);

    return true;    
}

bool
Shm::exists()
{
//    GNASH_REPORT_FUNCTION;
    struct stat           stats;
    struct dirent         *entry;
    vector<const char *>  dirlist;
    string                realname;
    DIR                   *library_dir = NULL;

    // Solaris stores shared memory segments in /var/tmp/.SHMD and
    // /tmp/.SHMD. Linux stores them in /dev/shm.
    dirlist.push_back("/dev/shm");
    dirlist.push_back("/var/tmp/.SHMD");
    dirlist.push_back("/tmp/.SHMD");

    // Open the directory where the raw POSIX shared memory files are
    for (unsigned int i=0; i<dirlist.size(); i++)
    {
        library_dir = opendir (dirlist[i]);
        if (library_dir != NULL) {
            realname = dirlist[i];
            
            // By convention, the first two entries in each directory
            // are for . and .. (``dot'' and ``dot dot''), so we
            // ignore those. The next directory read will get a real
            // file, if any exists.
            entry = readdir(library_dir);
            entry = readdir(library_dir);
            break;
        }
    }

    if (strlen(_filespec)) {
	realname += _filespec;
    
	if (stat(realname.c_str(), &stats) == 0) {
	    return true;
	}
    }
    
    return false;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
