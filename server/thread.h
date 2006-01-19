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

#ifndef __G_PTHREAD_H__
#define __G_PTHREAD_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREADS

#include <pthread.h>

namespace gnash {  
  typedef void *(*PThreadPtr)(void *);
  typedef void (*PThreadCleanupPtr)(void *);


class Thread
  {
  public:
    Thread();
    ~Thread();
    // Create a new thread
    pthread_t *createPThread(void *, PThreadPtr funcptr);
    
    // Destroy the thread
    void destroyPThread(void *retval);
    void destroyPThread();
    
    // Setup a cleanup function
    void cleanupThread(PThreadCleanupPtr funcptr, void *arg);
    pthread_attr_t     _tattr;    // holds thread's attributes
  protected:
    pthread_t          _thread;  // pointer to the thread info

  private:
} // end of gnash namespace

// end of HAVE_PTHREADS
#endif
 
// __G_PTHREAD_H__
#endif
