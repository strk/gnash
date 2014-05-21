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
#include "GnashSleep.h"

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
    _addr(nullptr),
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
            ::shmctl(_shmid, IPC_RMID, nullptr);
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
SharedMem::getSemaphore()
{
#ifndef __amigaos4__
    // Struct for semctl
    union semun {
        int val;
        struct semid_ds* buf;
        unsigned short* array;
    };
#endif

    semun s;

    // We employ the textbook solution to the race condition during creation
    // and initialization of the semaphore: Create the semaphore with IPC_EXCL
    // so that for the first process, creation will succeed, but for any
    // concurrent process it will fail with EEXIST. Then, once the first
    // process is finished initializing the semaphore, semop is called (in our
    // case by calling lock()) which modifies semid_ds::otime, and the
    // concurrent process knows it can proceed with using the semaphore
    // (provided it obtains a lock.)
    _semid = ::semget(_shmkey, 1, IPC_CREAT | IPC_EXCL | 0600);
        
    if (_semid >= 0) {
        // Initialize semval.
        s.val = 1;
        int ret = ::semctl(_semid, 0, SETVAL, s);
        if (ret < 0) {
            log_error(_("Failed to set semaphore value: %1%"), strerror(errno));
            return false;
        }
        // The first semop is executed immediately after this function returns.
    } else if (errno == EEXIST) {
        _semid = semget(_shmkey, 1, 0600);
        if (_semid < 0) {
            log_error(_("Failed to obtain nonexclusive semaphore for shared "
                        "memory: %1%"), strerror(errno));
            return false;
        }

        // Wait until semid_ds::sem_otime changes.
        semid_ds buf = semid_ds();
        s.buf = &buf;
        const int maxRetries = 10;
        time_t uSecondsWait = 100; // 0.1ms
        bool ok = false;

        for(int i = 0; i < maxRetries; i++) {
            semctl(_semid, 0, IPC_STAT, s);
            if (buf.sem_otime != 0) {
                ok = true;
                break;
            } else {
                gnashSleep(uSecondsWait);
            }
        }

        if (!ok) {
            log_error(_("Timed out waiting for semaphore initialization."));
            return false;
        }
    } else {
        log_error(_("Failed creating semaphore: %1%"), strerror(errno));
        return false;
    }

    return true;
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

    if (!getSemaphore()) {
        return false;
    }

    // Note that at this point semval is of indeterminate value: for the
    // process that created the semaphore, the value is 1, but for another
    // process the value would depend on whether there is an existing lock.
    
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

    _addr = static_cast<iterator>(::shmat(_shmid, nullptr, 0));

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
