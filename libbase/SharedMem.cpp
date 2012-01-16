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

#include "SharedMem.h"

#include <string>
#include <vector>
#include <cerrno>
#include <cstring>

#if !defined(__riscos__) && !defined(__OS2__) 
# include <sys/types.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/ipc.h>
#endif

#include "log.h"

#if (defined(HAVE_SHMGET)) 
# define ENABLE_SHARED_MEM 1
#else
# undef ENABLE_SHARED_MEM
#endif

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
    // Nothing to do if we were never attached.
    if (!_addr) return;

    if (::shmdt(_addr) < 0) {
        const int err = errno;
        log_error(_("Error detaching shared memory: %s"), std::strerror(err));
    }

    // We can still try to shut it down.
    struct ::shmid_ds ds;
    if (::shmctl(_shmid, IPC_STAT, &ds) < 0) {
        const int err = errno;
        log_error(_("Error during stat of shared memory segment: %s"),
                std::strerror(err));
    }

#ifndef __amigaos4__
    else {
        // Note that this isn't completely reliable.
        if (!ds.shm_nattch) {
            log_debug(_("No shared memory users left. Removing segment and semaphore."));
            ::shmctl(_shmid, IPC_RMID, 0);
            ::semctl(_semid, IPC_RMID, 0);
        }
    }
#endif
}

bool
SharedMem::lock() const
{
    struct sembuf sb = { 0, -1, SEM_UNDO };
    const int ret = ::semop(_semid, &sb, 1);
    return ret >= 0;
}

bool
SharedMem::unlock() const
{
    struct sembuf sb = { 0, 1, SEM_UNDO };
    const int ret = ::semop(_semid, &sb, 1);
    return ret >= 0;
}

bool
SharedMem::attach()
{
#if !ENABLE_SHARED_MEM
# error "You need SYSV Shared memory support to use this option"
#endif
    
    // Don't try to attach twice.
    if (_addr) return true;
    
    _shmkey = rcfile.getLCShmKey();

    // Check rcfile for key; if there isn't one, use the Adobe key.
    if (_shmkey == 0) {
        log_debug("No shared memory key specified in rcfile. Using default for communication with other players");
        _shmkey = 0xdd3adabd;
    }
    
    log_debug("Using shared memory key %s",
            boost::io::group(std::hex, std::showbase, _shmkey));

    // First get semaphore.
    
    // Check if it exists already.
    _semid = ::semget(_shmkey, 1, 0600);

#ifndef __amigaos4__
    // Struct for semctl
    union semun {
        int val;
        struct semi_ds* buf;
        unsigned short* array;
    };
#endif

    semun s;

    // If it does not exist, create it and set its value to 1.
    if (_semid < 0) {

        _semid = ::semget(_shmkey, 1, IPC_CREAT | 0600);
        
        if (_semid < 0) {
            log_error(_("Failed to get semaphore for shared memory!"));
            return false;
        }    

        s.val = 1;
        const int ret = ::semctl(_semid, 0, SETVAL, s);
        if (ret < 0) {
            log_error(_("Failed to set semaphore value"));
            return false;
        }
    }
    
    // The 4th argument is neither necessary nor used, but we pass it
    // anyway for fun.
    const int semval = ::semctl(_semid, 0, GETVAL, s);

    if (semval != 1) {
        log_error(_("Need semaphore value of 1 for locking. Cannot attach shared memory!"));
        return false;
    }

    Lock lock(*this);

    // Then attach shared memory. See if it exists.
    _shmid = ::shmget(_shmkey, _size, 0600);

    // If not create it.
    if (_shmid < 0) {
        _shmid = ::shmget(_shmkey, _size, IPC_CREAT | 0660);
    }

    if (_shmid < 0) {
        log_error(_("Unable to get shared memory segment!"));
        return false;
    }

    _addr = static_cast<iterator>(::shmat(_shmid, 0, 0));

    if (!_addr) {
        log_error(_("Unable to attach shared memory: %s"),
                std::strerror(errno));
        return false;
    }

    assert(_addr);
    return true;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
