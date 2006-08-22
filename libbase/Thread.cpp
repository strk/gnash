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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <cstring>

#include "log.h"
#include "Thread.h"

using namespace std;

namespace gnash {

Thread::Thread(void)

{
    _debug = false;
    printf("%s: Entered\n", __PRETTY_FUNCTION__);
}

Thread::~Thread(void)
{ 
    printf("%s: Entered\n", __PRETTY_FUNCTION__);
    _debug = false;    
//    destroyThread((void *)this);
}

/// \brief Create a POSIX thread
///
/// The new thread is created in a detached state
pthread_t *
Thread::createThread(void *arg, ThreadPtr_t funcptr)
{
    pthread_attr_init(&_tattr);

    if (pthread_attr_setscope(&_tattr, PTHREAD_SCOPE_SYSTEM)) {
        log_msg("Couldn't set thread scope, %s\n", strerror(errno));
    }

#if 0
    // on non-Solaris OS, default stack may be too small. This seems
    // to only effect HPUX versions before 11.i (11.11) on the PARISC.
    if (pthread_attr_setstacksize(&_type, 1048576)) {
        log_msg("Couldn't set thread stacksize, %s\n", strerror(errno));
    }
#endif
#if 0
    if (pthread_attr_setschedparam(&_tattr, SCHED_FIFO))
//    if (pthread_attr_setdetachstate(&_tattr, PTHREAD_CREATE_JOINABLE))
    {
        DBG_MSG(DBG_EROR, "Couldn't set thread scheduler state, %s",
            strerror(errno));
    }
#endif

//    if (pthread_attr_setdetachstate(&_tattr, PTHREAD_CREATE_JOINABLE)) {
    if (pthread_attr_setdetachstate(&_tattr, PTHREAD_CREATE_DETACHED)) {
        log_msg("Couldn't set thread detach state, %s\n", strerror(errno));
    }
    
    if (pthread_create(&_thread, &_tattr, funcptr, arg)) {
        log_msg("Couldn't create the thread, %s\n", strerror(errno));
    }
    
//     // guarantee thread is independent and parallel
//     if (pthread_detach(_thread)) {
//         log_msg("Couldn't detach the thread, %s\n", strerror(errno));
//     }
    
    // free attribute object
    pthread_attr_destroy(&_tattr);

    return &_thread;
}

/// \brief Destroy a POSIX thread
void
Thread::destroyThread(void *retval)
{
    log_msg("%s: Entered\n", __PRETTY_FUNCTION__);
    pthread_exit(retval);
}

void
Thread::dump(void)
{
    int val;
    struct sched_param param;
    
    log_msg("Debugging flag is ");
    if (_debug) {
        log_msg("\t\tON\n");
    } else {
        log_msg("\t\tOFF\n");
    }
    
//    _tattr

    log_msg("The detached thread state is \t");
    pthread_attr_getdetachstate(&_tattr, &val);
    switch (val) {
      case PTHREAD_CREATE_JOINABLE:
        log_msg("PTHREAD_CREATE_JOINABLE\n");
        break;
      case PTHREAD_CREATE_DETACHED:
        log_msg("PTHREAD_CREATE_DETACHED\n");
        break;
      default:
        log_msg("NONE SPECIFIED\n");
        break;
    }                                                                                
    log_msg("The thread schedule policy is \t");
    pthread_attr_getschedpolicy(&_tattr, &val);
    switch (val) {
      case SCHED_OTHER:
        log_msg("SCHED_OTHER\n");
        break;
      case SCHED_RR:
        log_msg("SCHED_RR\n");
        break;
      case SCHED_FIFO:
        log_msg("SCHED_FIFO\n");
        break;
      default:
        log_msg("NONE SPECIFIED\n");
        break;
    }
    
    pthread_attr_getschedparam(&_tattr, &param);
//    log_msg("The schedule parameter is " << param);
        
    log_msg("The inherit scheduler is \t");
    pthread_attr_getinheritsched(&_tattr, &val);
    switch (val) {
      case PTHREAD_EXPLICIT_SCHED:
        log_msg("PTHREAD_EXPLICIT_SCHED\n");
        break;
      case PTHREAD_INHERIT_SCHED:
        log_msg("PTHREAD_INHERIT_SCHED\n");
        break;
      default:
        log_msg("NONE SPECIFIED\n");
        break;
    }
    
    log_msg("The scope is \t\t\t");
    pthread_attr_getscope(&_tattr, &val);
    switch (val) {
      case PTHREAD_SCOPE_SYSTEM:
        log_msg("PTHREAD_SCOPE_SYSTEM\n");
        break;
      case PTHREAD_SCOPE_PROCESS:
        log_msg("PTHREAD_SCOPE_PROCESS\n");
        break;
      default:
        log_msg("NONE SPECIFIE\n");
        break;
    }    
}

/// \brief Setup callback for thread exit

/// This installs a function pointer that gets called when
/// this thread exists.
void
Thread::cleanupThread(ThreadCleanupPtr_t funcptr, void* /*arg*/)
{
    // Install the handler into the list
    pthread_cleanup_push(funcptr, NULL);

    // Execute the handler, which waits till the thread exits
    pthread_cleanup_pop(1);
}

// EOF namespace gnash
}
