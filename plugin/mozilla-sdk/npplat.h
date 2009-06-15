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

#ifndef _NPPLAT_H_
#define _NPPLAT_H_

/**************************************************/
/*                                                */
/*                   Windows                      */
/*                                                */
/**************************************************/
#ifdef XP_WIN
#include "windows.h"
#endif //XP_WIN

#include "npapi.h"
#include "npupp.h"
#include <sys/types.h>

/**************************************************/
/*                                                */
/*                    Unix                        */
/*                                                */
/**************************************************/
#ifdef XP_UNIX
#include <cstdio>
#endif //XP_UNIX

/**************************************************/
/*                                                */
/*                     Mac                        */
/*                                                */
/**************************************************/
#ifdef XP_MAC

#include <Processes.h>
#include <Gestalt.h>
#include <CodeFragments.h>
#include <Timer.h>
#include <Resources.h>
#include <ToolUtils.h>

#include "jri.h"

// The Mixed Mode procInfos defined in npupp.h assume Think C-
// style calling conventions.  These conventions are used by
// Metrowerks with the exception of pointer return types, which
// in Metrowerks 68K are returned in A0, instead of the standard
// D0. Thus, since NPN_MemAlloc and NPN_UserAgent return pointers,
// Mixed Mode will return the values to a 68K plugin in D0, but 
// a 68K plugin compiled by Metrowerks will expect the result in
// A0.  The following pragma forces Metrowerks to use D0 instead.
//
#ifdef __MWERKS__
#ifndef powerc
#pragma pointers_in_D0
#endif
#endif

#ifdef __MWERKS__
#ifndef powerc
#pragma pointers_in_A0
#endif
#endif

// The following fix for static initializers (which fixes a preious
// incompatibility with some parts of PowerPlant, was submitted by 
// Jan Ulbrich.
#ifdef __MWERKS__
	#ifdef __cplusplus
	extern "C" {
	#endif
		#ifndef powerc
			extern void	__InitCode__(void);
		#else
			extern void __sinit(void);
			#define __InitCode__ __sinit
		#endif
		extern void	__destroy_global_chain(void);
	#ifdef __cplusplus
	}
	#endif // __cplusplus
#endif // __MWERKS__

// Wrapper functions for all calls from Netscape to the plugin.
// These functions let the plugin developer just create the APIs
// as documented and defined in npapi.h, without needing to 
// install those functions in the function table or worry about
// setting up globals for 68K plugins.
NPError Private_Initialize(void);
void    Private_Shutdown(void);
NPError Private_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
NPError Private_Destroy(NPP instance, NPSavedData** save);
NPError Private_SetWindow(NPP instance, NPWindow* window);
NPError Private_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
NPError Private_DestroyStream(NPP instance, NPStream* stream, NPError reason);
int32   Private_WriteReady(NPP instance, NPStream* stream);
int32   Private_Write(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
void    Private_StreamAsFile(NPP instance, NPStream* stream, const char* fname);
void    Private_Print(NPP instance, NPPrint* platformPrint);
int16   Private_HandleEvent(NPP instance, void* event);
void    Private_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData);
jref    Private_GetJavaClass(void);
NPError Private_GetValue(NPP instance, NPPVariable variable, void *result);
NPError Private_SetValue(NPP instance, NPNVariable variable, void *value);

#endif //XP_MAC

#ifndef HIBYTE
#define HIBYTE(i) (i >> 8)
#endif

#ifndef LOBYTE
#define LOBYTE(i) (i & 0xff)
#endif

#endif //_NPPLAT_H_
