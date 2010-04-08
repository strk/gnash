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

// Test:
//     Use browser to open plugin/scriptable-test.html.
//
// Notes:
// 1. In order not to rewrite the whole Plugin, just be _scriptObject 
//    of nsPluginInstance.
// 2. GnashPluginScriptObject is the marshal object to response 
//    JavaScript, no function to gtk-gnash yet.
// 3. Once gnash::Player support multiple instance, it's possible to 
//    the bridge between JavaScript and ActionScript.
//
// Todo:
//    Support the methods listed in
//    http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_03.html

#ifndef GNASH_PLUGIN_SCRITP_OBJECT_H
#define GNASH_PLUGIN_SCRITP_OBJECT_H

#include "npapi.h"
#include "npruntime.h"

class GnashPluginScriptObject : public NPObject
{
public:

    GnashPluginScriptObject();
    GnashPluginScriptObject(NPP npp);

    ~GnashPluginScriptObject();

    // Marshal Functions
    static NPClass *marshalGetNPClass();

    static NPObject *marshalAllocate (NPP npp, NPClass *aClass);

    static void marshalDeallocate (NPObject *npobj);

    static void marshalInvalidate (NPObject *npobj);

    static bool marshalHasMethod (NPObject *npobj, NPIdentifier name);

    static bool marshalInvoke (NPObject *npobj, 
                               NPIdentifier name,
                               const NPVariant *args, 
                               uint32_t argCount,
                               NPVariant *result);

    static bool marshalInvokeDefault (NPObject *npobj,
                                      const NPVariant *args,
                                      uint32_t argCount,
                                      NPVariant *result);

    static bool marshalHasProperty (NPObject *npobj, NPIdentifier name);

    static bool marshalGetProperty (NPObject *npobj, NPIdentifier name,
                                    NPVariant *result);

    static bool marshalSetProperty (NPObject *npobj, NPIdentifier name,
                                    const NPVariant *value);

    static bool marshalRemoveProperty (NPObject *npobj, NPIdentifier name);

private:

    // m_npp->pdata should be the nsPluginInstance once NPP_New() is finished.
    NPP m_npp;


};

#endif /* GNASH_PLUGIN_SCRITP_OBJECT_H */

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
