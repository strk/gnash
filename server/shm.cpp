// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <errno.h>

#include "log.h"
#include "shm.h"

using namespace std;

namespace gnash {

const int DEFAULT_SHM_SIZE = 10240;
const int MAX_FILESPEC_SIZE = 20;

#define FLAT_ADDR_SPACE 1

Shm::Shm() :_addr(0), _alloced(0), _size(0)
{
}

Shm::~Shm()
{
}
    
// Initialize the shared memory segment
bool
Shm::attach(char const *filespec, bool nuke)
{
    int  fd;
    bool exists = false;
    long addr;
#ifdef FLAT_ADDR_SPACE
    off_t off;
#endif
    Shm *sc;
    string absfilespec;

    _size = DEFAULT_SHM_SIZE;

#ifdef darwin
    absfilespec = "/tmp";
#else
    absfilespec = "/";
#endif
    absfilespec += filespec;
    _filespec = absfilespec;
    filespec = absfilespec.c_str();
    
    
//     log_msg("%s: Initializing %d bytes of memory for \"%s\"\n",
//             __PRETTY_FUNCTION__, DEFAULT_SHM_SIZE, absfilespec.c_str());

    // Adjust the allocated amount of memory to be on a page boundary.
    long pageSize = sysconf(_SC_PAGESIZE);
    if (_size % pageSize) {
        _size += pageSize - _size % pageSize;
    }
    
    errno = 0;
    
    // Create the shared memory segment
    fd = shm_open(filespec, O_RDWR|O_CREAT|O_EXCL|O_TRUNC,
        S_IRUSR|S_IWUSR);

    // If it already exists, then just attach to it.
    if (fd < 0 && errno == EEXIST) {
        exists = true;
        log_msg("Shared Memory segment \"%s\" already exists\n",
                filespec);
//        fd = shm_open(filespec, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        return false;
    }
    
    // MacOSX returns this when you use O_EXCL for shm_open() instead
    // of EEXIST
    if (fd < 0 && errno == EINVAL) {
        exists = true;
        log_msg("WARNING: shm_open failed, retrying: %s\n",
                strerror(errno));
//        fd = shm_open(filespec, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        return false;
    }
    
    // We got the file descriptor, now map it into our process.
    if (fd >= 0) {
        if (!exists) {
            // Set the size so we can write to new segment
            ftruncate(fd, _size);
        }

#ifdef darwin
        _addr = static_cast<char *>(mmap(0, _size,
            PROT_READ|PROT_WRITE,
            MAP_SHARED|MAP_HASSEMAPHORE, fd, 0));
#else
        _addr = static_cast<char *>(mmap(0, _size,
            PROT_READ|PROT_WRITE|PROT_EXEC,
            MAP_SHARED|MAP_INHERIT|MAP_HASSEMAPHORE, fd, 0));
#endif
        if (_addr == MAP_FAILED) {
            log_msg("WARNING: mmap() failed: %s\n", strerror(errno));
            return false;
        }
        
        if (exists && !nuke) {
            // If there is an existing memory segment that we don't
            // want to trash, we just want to attach to it. We know
            // that a ShmControl data class has been instantiated in
            // the base of memory, and the first field is the address
            // used for the previous mmap(), so we grab that value,
            // unmap the old address, and map the original address
            // into this process. This is done so that memory
            // allocations between processes all have the same
            // addresses. Otherwise, one can't do globally shared data
            // among processes that requires any dynamic memory
            // allocation. All of this is so our custom memory
            // allocator for STL containers will work.
            addr = *(reinterpret_cast<long *>(_addr));
            if (addr == 0)
            {
                log_msg("WARNING: No address found in memory segment!\n");
                nuke = true;
            } else {
#ifdef FLAT_ADDR_SPACE
                log_msg("Adjusting address to 0x%lx\n", addr);
                munmap(_addr, _size);
                log_msg("Unmapped address %p\n", _addr);
#if 0
                _addr = static_cast<char *>(mmap(reinterpret_cast<char *>(addr),
                                                 _size, PROT_READ|PROT_WRITE,
                    MAP_SHARED|MAP_FIXED|MAP_INHERIT|MAP_HASSEMAPHORE,
                    fd, static_cast<off_t>(0)));
#else
                off = (off_t)((long)addr - (long)_addr);
#ifdef darwin
                _addr = static_cast<char *>(mmap((char *)addr,
                         _shmSize, PROT_READ|PROT_WRITE,
                         MAP_FIXED|MAP_SHARED, fd, 0));
#else
                _addr = static_cast<char *>(mmap((char *)addr,
                         _size, PROT_READ|PROT_WRITE|PROT_EXEC,
                         MAP_FIXED|MAP_SHARED, fd, 0));
#endif
                
#endif
                if (_addr == MAP_FAILED) {
                    log_msg("WARNING: MMAP failed: %s\n", strerror(errno));
                    return static_cast<Shm *>(0);
                }
#endif
            }
        }
        
        log_msg(
            "Opened Shared Memory segment \"%s\": %d bytes at %p.\n",
            filespec, _size, _addr);
        
        if (nuke) {
//            log_msg("Zeroing %d bytes at %p.\n", _size, _addr);
            // Nuke all the segment, so we don't have any problems
            // with leftover data.
            memset(_addr, 0, _size);
            sc = cloneSelf();
        } else {
            sc = reinterpret_cast<Shm *>(_addr);
        }
    } else {
        log_msg("ERROR: Couldn't open the Shared Memory segment \"%s\"! %s\n",
                filespec, strerror(errno));
        return false;
    }

    // don't close it on an error
    if (fd) {
        ::close(fd);
    }
    
    return true; 
}

Shm *
Shm::cloneSelf(void)
{

    if (_addr > 0) {
//         log_msg("Cloning ShmControl, %d bytes to %p\n",
//                 sizeof(Shm), _addr);
        // set the allocated bytes before we copy so the value is
        // correct in both instantiations of this object.
        _alloced = sizeof(Shm);
        memcpy(_addr, this, sizeof(Shm));
        return reinterpret_cast<Shm *>(_addr);
    }
    
    log_msg("WARNING: Can't clone Self, address 0x0\n");

    return static_cast<Shm *>(0);
}

// Resize the allocated memory segment
bool
Shm::resize()
{
    // Increase the size by 10 %
    return resize(DEFAULT_SHM_SIZE + (DEFAULT_SHM_SIZE/10)); 
}

bool
Shm::resize(int bytes)
    
{
# ifdef HAVE_MREMAP
    _addr = mremap(_shmAddr, _shmSize, _shmSize + bytes, MREMAP_MAYMOVE);
    if (_addr != 0) {
        return true;
    }
#else
    // FIXME: alloc a whole new segment, and copy this one
    // into it. Yeuch...
#endif
    return false;
}

// Allocate a memory from the shared memory segment
void *
Shm::brk(int bytes)
{
    int wordsize = sizeof(long);
    
    // Adjust the allocated amount of memory to be on a word boundary.
    if (bytes % wordsize) {
        int wordsize = sizeof(long);
        
        // Adjust the allocated amount of memory to be on a word boundary.
        if (bytes % wordsize) {
            bytes += wordsize - bytes % wordsize;
        }
        
        void *addr = (static_cast<char *>(_addr)) + _alloced;
        
        log_msg("%s: Allocating %d bytes at %p\n",
                __PRETTY_FUNCTION__, bytes, addr);
        // Zero out the block before returning it
        memset(addr, 0, bytes);
        
        // Increment the counter
        _alloced += bytes;
        
        // Return a pointer to the beginning of the block
        return addr;
        bytes += wordsize - bytes % wordsize;
    }
    
    void *addr = (static_cast<char *>(_addr)) + _alloced;
    
    log_msg("%s: Allocating %d bytes at %p\n",
            __PRETTY_FUNCTION__, bytes, addr);

    // Zero out the block before returning it
    memset(addr, 0, bytes);
    
    
    // Increment the counter
    _alloced += bytes;
    
    // Return a pointer to the beginning of the block
    return addr; 
}

// Close the memory segment. This removes it from the system.
bool
Shm::closeMem()
{
    // Only nuke the shared memory segement if we're the last one.
    if (_filespec.size() != 0) {
        shm_unlink(_filespec.c_str());
    }
    
     // flush the shared memory to disk
     if (_addr > 0) {
         // detach memory
         munmap(_addr, _size);
     }
    
    _addr = 0;
    _alloced = 0;

    return true;    
}

#ifdef ENABLE_TESTING
bool
Shm::exists()
{
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
    for (int i=0; i<dirlist.size(); i++)
    {
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
    
    realname += _filespec;
    
    if (stat(realname.c_str(), &stats) == 0) {
        return true;
    }
    return false;
}

// These are the callbacks used to define custom methods for our AS
// classes. This way we can examine the private data after calling a
// method to see if it worked correctly.
void shm_getname(const fn_call& fn)
{
    shm_as_object *ptr = (shm_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.getName().c_str());
}
void shm_getsize(const fn_call& fn)
{
    shm_as_object *ptr = (shm_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_int(ptr->obj.getSize());
}
void shm_getallocated(const fn_call& fn)
{
    shm_as_object *ptr = (shm_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_int(ptr->obj.getAllocated());
}
void shm_exists(const fn_call& fn)
{
    shm_as_object *ptr = (shm_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_bool(ptr->obj.exists());
}
#endif

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
