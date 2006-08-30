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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

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

