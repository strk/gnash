// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

//

#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// FIXME: HAVE_PTHREAD_H is defined in config.h, but for some reason
// refuses to be defined for this file.
//#ifdef HAVE_PTHREAD_H
#include <pthread.h>
// #else
// #error "You must have POSIX threads installed to compile this file!"
// #endif
// #include <sys/types.h>

namespace gnash
{

typedef void *(*ThreadPtr_t)(void *);
typedef void (*ThreadCleanupPtr_t)(void *);

class Thread {
public:
    Thread(void);
    ~Thread(void);

    void debugOn()      { _debug = true; };
    void debugOff()     { _debug = false; };

    // Create a new thread
    pthread_t *createThread(void *arg, ThreadPtr_t funcptr);
    
    // Destroy the thread
    void destroyThread(void *retval);
    void destroyThread();
    
    // Setup a cleanup function
    void cleanupThread(ThreadCleanupPtr_t funcptr, void *arg);
    
    // Dump private data
    void dump();

private:
    bool               _debug;  // print debug or not
    pthread_attr_t     _tattr;  // holds thread's attributes
protected:
    pthread_t          _thread; // pointer to the thread info
};

// EOF namespace gnash
}
// EOF __THREAD_H__
#endif

