// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// Todo:
//    Support the methods listed in
//    http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_03.html

#ifndef GNASH_PLUGIN_CALLBACKS_H
#define GNASH_PLUGIN_CALLBACKS_H

extern bool GetVariableCallback (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool SetVariableCallback (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool GotoFrame (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool IsPlaying (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool LoadMovie (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool Pan (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool PercentLoaded (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool Play (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool Rewind (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool SetZoomRect (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool StopPlay (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool Zoom (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool TotalFrames (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

#endif // GNASH_PLUGIN_CALLBACKS_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
