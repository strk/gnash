// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
//

/*
** File:		prlock.h
** Description:	API to basic locking functions of NSPR.
**
**
** NSPR provides basic locking mechanisms for thread synchronization.  Locks 
** are lightweight resource contention controls that prevent multiple threads 
** from accessing something (code/data) simultaneously.
**/

#ifndef prlock_h___
#define prlock_h___

#include "prtypes.h"

PR_BEGIN_EXTERN_C

/**********************************************************************/
/************************* TYPES AND CONSTANTS ************************/
/**********************************************************************/

/*
 * PRLock --
 *
 *     NSPR represents the lock as an opaque entity to the client of the
 *	   API.  All routines operate on a pointer to this opaque entity.
 */

typedef struct PRLock PRLock;

/**********************************************************************/
/****************************** FUNCTIONS *****************************/
/**********************************************************************/

/***********************************************************************
** FUNCTION:    PR_NewLock
** DESCRIPTION:
**  Returns a pointer to a newly created opaque lock object.
** INPUTS:      void
** OUTPUTS:     void
** RETURN:      PRLock*
**   If the lock can not be created because of resource constraints, NULL
**   is returned.
**  
***********************************************************************/
NSPR_API(PRLock*) PR_NewLock(void);

/***********************************************************************
** FUNCTION:    PR_DestroyLock
** DESCRIPTION:
**  Destroys a given opaque lock object.
** INPUTS:      PRLock *lock
**              Lock to be freed.
** OUTPUTS:     void
** RETURN:      None
***********************************************************************/
NSPR_API(void) PR_DestroyLock(PRLock *lock);

/***********************************************************************
** FUNCTION:    PR_Lock
** DESCRIPTION:
**  Lock a lock.
** INPUTS:      PRLock *lock
**              Lock to locked.
** OUTPUTS:     void
** RETURN:      None
***********************************************************************/
NSPR_API(void) PR_Lock(PRLock *lock);

/***********************************************************************
** FUNCTION:    PR_Unlock
** DESCRIPTION:
**  Unlock a lock.  Unlocking an unlocked lock has undefined results.
** INPUTS:      PRLock *lock
**              Lock to unlocked.
** OUTPUTS:     void
** RETURN:      PR_STATUS
**              Returns PR_FAILURE if the caller does not own the lock.
***********************************************************************/
NSPR_API(PRStatus) PR_Unlock(PRLock *lock);

PR_END_EXTERN_C

#endif /* prlock_h___ */
