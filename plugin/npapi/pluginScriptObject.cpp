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

#include <iostream>
#include "plugin.h" 
#include "pluginScriptObject.h"

extern NPNetscapeFuncs NPNFuncs;
// Debugging utilities
extern void GnashLogDebug(const std::string& msg);
extern void GnashLogError(const std::string& msg);

// NPClass of GnashPluginScriptObject
static NPClass GnashPluginScriptObjectClass = {
    NP_CLASS_STRUCT_VERSION,
    GnashPluginScriptObject::marshalAllocate,
    GnashPluginScriptObject::marshalDeallocate,
    GnashPluginScriptObject::marshalInvalidate,
    GnashPluginScriptObject::marshalHasMethod,
    GnashPluginScriptObject::marshalInvoke,
    GnashPluginScriptObject::marshalInvokeDefault,
    GnashPluginScriptObject::marshalHasProperty,
    GnashPluginScriptObject::marshalGetProperty,
    GnashPluginScriptObject::marshalSetProperty,
    GnashPluginScriptObject::marshalRemoveProperty
};

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject()
    : m_npp (0)
{
    // GnashLogDebug("GnashPluginScriptObject constructor");
}

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject(NPP npp)
    : m_npp (npp)
{
    // GnashLogDebug("GnashPluginScriptObject constructor");
}

// Destructor
GnashPluginScriptObject::~GnashPluginScriptObject() 
{
    // do nothing
}

// Marshal Functions
NPClass *
GnashPluginScriptObject::marshalGetNPClass() 
{
    return &GnashPluginScriptObjectClass;
}

NPObject *
GnashPluginScriptObject::marshalAllocate (NPP npp, NPClass *aClass)
{
    // GnashLogDebug("marshalAllocate");
    return new GnashPluginScriptObject(npp);
}

void 
GnashPluginScriptObject::marshalDeallocate (NPObject *npobj)
{
    
    // GnashLogDebug("marshalDeallocate");
    delete (GnashPluginScriptObject *)npobj;
}

void 
GnashPluginScriptObject::marshalInvalidate (NPObject *npobj)
{
    // GnashLogDebug("marshalInvalidate");
}

bool 
GnashPluginScriptObject::marshalHasMethod (NPObject *npobj, NPIdentifier name)
{
    // GnashLogDebug("marshalHasMethod");
    return false;
}

bool 
GnashPluginScriptObject::marshalInvoke (NPObject *npobj, NPIdentifier name,
                                        const NPVariant *args, uint32_t argCount,
                                        NPVariant *result)
{
    // GnashLogDebug("marshalInvoke");
    return false;
}

bool 
GnashPluginScriptObject::marshalInvokeDefault (NPObject *npobj,
                                               const NPVariant *args,
                                               uint32_t argCount,
                                               NPVariant *result)
{
    // GnashLogDebug("marshalInvokeDefault");
    return false;
}

bool 
GnashPluginScriptObject::marshalHasProperty (NPObject *npobj, NPIdentifier name)
{
    // GnashLogDebug("marshalHasProperty");
    if (name == NPNFuncs.getstringidentifier("version")) {
        return true;
    }
    return false;
}

bool 
GnashPluginScriptObject::marshalGetProperty (NPObject *npobj, NPIdentifier name,
                                             NPVariant *result)
{
    // GnashLogDebug("marshalGetProperty");
    if (name == NPNFuncs.getstringidentifier("version")) {
        DOUBLE_TO_NPVARIANT(1.23, *result);
        return true;
    }
    return false;
}

bool 
GnashPluginScriptObject::marshalSetProperty (NPObject *npobj, NPIdentifier name,
                                             const NPVariant *value)
{
    // GnashLogDebug("marshalSetProperty");
    return false;
}

bool 
GnashPluginScriptObject::marshalRemoveProperty (NPObject *npobj, NPIdentifier name)
{
    // GnashLogDebug("marshalRemoveProperty");
    return false;
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
