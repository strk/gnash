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

#ifndef GNASH_SHM_H
#define GNASH_SHM_H

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include <cstdint>

#if defined (WIN32)
// Include for HANDLE
#include <windows.h>
#else
// key_t
#include <sys/types.h>
#endif

#include "dsodefs.h" //For DSOEXPORT

// Forward declarations
namespace gnash {
	class fn_call;
}

namespace gnash {

class SharedMem
{
public:

    typedef std::uint8_t* iterator;

    /// The beginning of the SharedMem section.
    //
    /// This is only valid after attach() has returned true. You can check
    /// with the function attached().
    iterator begin() const {
        return _addr;
    }

    /// The end of the SharedMem section.
    //
    /// This is only valid after attach() has returned true.
    iterator end() const {
        return _addr + _size;
    }

    /// Construct a SharedMem with the requested size.
    //
    /// @param size     The size of the shared memory section. If successfully
    ///                 created, the segment will be exactly this size and
    ///                 is not resizable.
    DSOEXPORT SharedMem(size_t size);

    /// Destructor.
    DSOEXPORT ~SharedMem();
    
    /// Initialize the shared memory segment
    //
    /// This is called by LocalConnection when either connect() or send()
    /// is called.
    DSOEXPORT bool attach();

    /// Use to get a scoped semaphore lock on the shared memory.
    class Lock
    {
    public:
        Lock(const SharedMem& s) : _s(s), _locked(s.lock()) {}
        ~Lock() { if (_locked) _s.unlock(); }
        bool locked() const {
            return _locked;
        }
    private:
        const SharedMem& _s;
        bool _locked;
    };

private:
    
    /// Get a semaphore lock if possible
    //
    /// @return     true if successful, false if not.
    DSOEXPORT bool lock() const;
    
    /// Release a semaphore lock if possible
    //
    /// @return     true if successful, false if not.
    DSOEXPORT bool unlock() const;

    /// Obtain a semaphore.
    /// @return true on success; false otherwise.
    bool getSemaphore();

    iterator _addr;

    const size_t _size;

    // Semaphore ID.
    int _semid;

    // Shared memory ID.
    int _shmid;

#if !defined(WIN32) 
    key_t _shmkey;
#else
    long _shmkey;
    HANDLE _shmhandle;
#endif
};

/// Check if the SharedMem has been attached.
//
/// This only checks whether the attach operation was successful, not whether
/// the shared memory still exists and is still attached where it was 
/// initially. It is always possible for other processes to remove it while
/// Gnash is using it, but there is nothing we can do about this.
inline bool
attached(const SharedMem& mem) {
    return (mem.begin());
}

} // end of gnash namespace

#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
