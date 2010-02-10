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
#include <sys/sem.h>
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

namespace gnash {

SharedMem::SharedMem(size_t size)
    :
    _addr(0),
    _size(size),
    _semid(0),
    _shmid(0),
    _shmkey(0)
{
}

SharedMem::~SharedMem()
{
    shmdt(_addr);
    struct shmid_ds ds;
    shmctl(_shmid, IPC_STAT, &ds);

    // Note that this isn't completely reliable.
    if (!ds.shm_nattch) {
        log_debug("No shared memory users left. Removing segment.");
        shmctl(_shmid, IPC_RMID, 0);
    }
}

bool
SharedMem::lock()
{
    struct sembuf sb = { 0, -1, SEM_UNDO };
    int ret = semop(_semid, &sb, 1);
    return ret >= 0;
}

bool
SharedMem::unlock()
{
    struct sembuf sb = { 0, 1, SEM_UNDO };
    int ret = semop(_semid, &sb, 1);
    return ret >= 0;
}

bool
SharedMem::attach()
{
   
    // Don't try to attach twice.
    if (_addr) return true;

#if (defined(USE_SYSV_SHM) && defined(HAVE_SHMGET)) || defined(_WIN32)
    
    // If there is no SYSV style shared memory key in the users ~/.gnashrc
    // file, warn them
    // that compatibility will be broken, and then just pick our own key so
    // things still work finer when using just Gnash.
    if (_shmkey == 0) {
        _shmkey = 0xdd3adabd;
    }
    
#ifndef _WIN32

    _semid = semget(_shmkey, 1, 0600);

    if (_semid < 0) {
        _semid = semget(_shmkey, 1, IPC_CREAT | 0600);
    }
    
    if (_semid < 0) {
        log_error("Failed to get semaphore for shared memory!");
        return false;
   }
    
    // According to POSIX.1-2001 we have to define this union (even though
    // we don't want to use it).
    union semun {
        int val;
        struct semid_ds* buf;
        ushort* array;
    };

    semun s;
    semctl(_semid, 0, GETVAL, &s);

    _shmid = shmget(_shmkey, _size, 0600);

    if (_shmid < 0) {
        _shmid = shmget(_shmkey, _size, IPC_CREAT | 0660);
    }

    if (_shmid < 0) {
        log_error("Unable to get shared memory segment!");
        return false;
    }

	_addr = static_cast<iterator>(shmat(_shmid, 0, 0));

    if (!_addr) {
	    log_debug("WARNING: shmat() failed: %s", std::strerror(errno));
	    return false;
	}


#else
    _shmhandle = CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL,
	    PAGE_READWRITE, 0, _size, NULL);
    if (_shmhandle == NULL) {
	log_debug("WARNING: CreateFileMapping failed: %ld\n", GetLastError());
        return false;
    }
    _addr = static_cast<iterator>(MapViewOfFile(_shmhandle, FILE_MAP_ALL_ACCESS,
            0, 0, _size));
    if (_addr == NULL) {
	log_debug("WARNING: MapViewOfFile() failed: %ld\n", GetLastError());
	return false;
    }
#endif

    assert(_addr);
    return true;


#else
# error "You need SYSV Shared memory support to use this option"
#endif	 // end of USE_SYSV_SHM
}	



} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
