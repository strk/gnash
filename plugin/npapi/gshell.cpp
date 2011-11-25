// 
//   Copyright (C) 2010, 2011 Free Software Foundation, Inc
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
// You should hxave received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>

#include "npapi.h"
#include "npruntime.h"
#include "npfunctions.h"
#include "plugin.h"
#include "pluginbase.h"
#include <regex.h>

#include "external.h"
#include "dsodefs.h" // DSOEXPORT

#ifdef _WIN32
#undef DLL_EXPORT
#define LIBLTDL_DLL_IMPORT 1
#endif

#include <sys/param.h>
#include <ltdl.h>

using namespace gnash; 
using namespace std; 

typedef NPError NP_InitializePtr (NPNetscapeFuncs* aNPNFuncs,
                                  NPPluginFuncs* aNPPFuncs);

extern NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue);

// This is a global variable commonly used by plugins to access
// the browser's function table. Since we aren't using a browser
// we have to populate this ourselves.
NPNetscapeFuncs *
populateNPFuncs(NPNetscapeFuncs *aNPNFuncs)
{
    aNPNFuncs->size = 45;       // this is the count of entries
    aNPNFuncs->version = 1.9;

    aNPNFuncs->geturl = NPN_GetURL;
    aNPNFuncs->posturl = NPN_PostURL;
    aNPNFuncs->requestread = NPN_RequestRead;
    aNPNFuncs->newstream = NPN_NewStream;
    aNPNFuncs->write = NPN_Write;
    aNPNFuncs->destroystream = NPN_DestroyStream;
    aNPNFuncs->status = NPN_Status;
    aNPNFuncs->uagent = NPN_UserAgent;
    aNPNFuncs->memalloc = NPN_MemAlloc;
    aNPNFuncs->memfree = NPN_MemFree;
    aNPNFuncs->memflush = NPN_MemFlush;
    aNPNFuncs->reloadplugins = NPN_ReloadPlugins;
    aNPNFuncs->getJavaEnv = 0;    // NPN_GetJavaEnv;
    aNPNFuncs->getJavaPeer = 0;   //NPN_GetJavaPeer;
    aNPNFuncs->geturlnotify = NPN_GetURLNotify;
    aNPNFuncs->posturlnotify = NPN_PostURLNotify;
    aNPNFuncs->getvalue = NPN_GetValue;
    aNPNFuncs->setvalue = NPN_SetValue;
    aNPNFuncs->invalidaterect = NPN_InvalidateRect;
    aNPNFuncs->invalidateregion = NPN_InvalidateRegion;
    aNPNFuncs->forceredraw = 0;    // NPN_ForceDraw;
    aNPNFuncs->getstringidentifier = NPN_GetStringIdentifier;
    aNPNFuncs->getstringidentifiers = NPN_GetStringIdentifiers;
    aNPNFuncs->getintidentifier = NPN_GetIntIdentifier;
    aNPNFuncs->identifierisstring = NPN_IdentifierIsString;
    aNPNFuncs->utf8fromidentifier = NPN_UTF8FromIdentifier;
    aNPNFuncs->intfromidentifier = NPN_IntFromIdentifier;
    aNPNFuncs->createobject = NPN_CreateObject;
    aNPNFuncs->retainobject = NPN_RetainObject;
    aNPNFuncs->releaseobject = NPN_ReleaseObject;
    aNPNFuncs->invoke = NPN_Invoke;
    aNPNFuncs->invokeDefault = NPN_InvokeDefault;
    aNPNFuncs->evaluate = NPN_Evaluate;
    aNPNFuncs->getproperty = NPN_GetProperty;
    aNPNFuncs->setproperty = NPN_SetProperty;
    aNPNFuncs->removeproperty = NPN_RemoveProperty;
    aNPNFuncs->hasproperty = NPN_HasProperty;
    aNPNFuncs->hasmethod = NPN_HasMethod;
    aNPNFuncs->releasevariantvalue = NPN_ReleaseVariantValue;
    aNPNFuncs->setexception = NPN_SetException;
    // aaNPNFuncs->pushpopupsenabledstate = NPN_PushPopupsEnabledState;
    // aaNPNFuncs->poppopupsenabledstate = NPN_PopPopupsEnabledState;
    aNPNFuncs->enumerate = NPN_Enumerate;
    aNPNFuncs->pluginthreadasynccall = NPN_PluginThreadAsyncCall;
    aNPNFuncs->construct = NPN_Construct;
    // aNPNFuncs->getvalueforurl = NPN_GetValueForURL;
    // aNPNFuncs->setvalueforurl = NPN_SetValueForURL;

    return aNPNFuncs;
}

int
main(int argc, char *argv[])
{
    // Initialize libtool's dynamic library loader
    int errors = lt_dlinit ();
    if (errors) {
        cerr << "ERROR: Couldn't initialize ltdl: " << lt_dlerror() << endl;
    }    

    // load a plugin in the current directory
    string plugindir = ".";

    // 
    char dir[MAXPATHLEN];
    getcwd(dir, MAXPATHLEN);
    plugindir += ":";
    plugindir += dir;
    plugindir += "/.libs/";
//    plugindir += "/plugin/npapo/.libs";
    
    lt_dlsetsearchpath(plugindir.c_str());

    string filespec = "libgnashplugin.so";
    lt_dlhandle handle;
    handle = lt_dlopenext (filespec.c_str());
    
    if (handle == NULL) {
        cerr << lt_dlerror() << endl;
        return false;
    }
    // Make this module unloadable
    lt_dlmakeresident(handle);    

    // Get the init function
    lt_ptr run = NULL;
    string symbol = "NP_Initialize"; // NS_PluginInitialize instead ?
    run  = lt_dlsym (handle, symbol.c_str());
    
    if (run == NULL) {
        cout << "Couldn't find symbol: " << symbol << endl;
        return NULL;
    } else {
        cout << "Found symbol " << symbol << " @ " << hex << run << endl;
    }
    
    // this gets populated by the browser
    NPNetscapeFuncs aNPNFuncs;
    populateNPFuncs(&aNPNFuncs);

    // this gets populated by the plugin
    //NPPluginFuncs *aNPPFuncs;
    nsPluginCreateData ds;

    // NPPFuncs.newp(NPMIMEType, 0, 0, 1, 0, "foo", 0);
    // nsPluginInstanceBase * plugin = NS_NewPluginInstance(&ds);
    
    NPPluginFuncs *aNPPFuncs = 0;
    // Execute the plugin's initialization function
    NP_InitializePtr *ptr = (NP_InitializePtr *)run;
    NPError error = ptr(&aNPNFuncs, aNPPFuncs);
    //    cout << getPluginDescription() << endl;

    char *str = 0;
    NPP_GetValue(0, NPPVpluginDescriptionString, str);
    if (str) {
        cerr << str << endl;
    }

    bool bo = false;
    NPN_GetValue(0, NPNVSupportsXEmbedBool, &bo);
    cerr << bo << endl;    
}

// We have to implement these two memory allocation functions as
// they're used in the code we're testing.
void *
NPN_MemAlloc(uint32_t size)
{
  void * rv = NULL;
  rv = malloc(size);
  return rv;
}

void
NPN_MemFree(void* ptr)
{
  assert(ptr);
  free(ptr);
}

uint32_t
NPN_MemFlush(uint32_t size)
{
    return size;
}

NPUTF8 *
NPN_UTF8FromIdentifier(NPIdentifier identifier)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

int32_t
NPN_IntFromIdentifier(NPIdentifier identifier)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

// These are just stubs to get the test case to link standalone.
NPIdentifier
NPN_GetStringIdentifier(const NPUTF8 *name)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPP_GetValue(NPP instance, NPPVariable aVariable, void *aValue)
{
    if (NPPFuncs.getvalue) {
        return NPPFuncs.getvalue(instance, aVariable, aValue);
    }
    return NPERR_GENERIC_ERROR;
}

NPError
NPN_GetURLNotify(NPP instance, const char* url,
                 const char* target, void* notifyData)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPN_PostURLNotify(NPP instance, const char* url,
                  const char* target, uint32_t len,
                  const char* buf, NPBool file,
                  void* notifyData)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
           const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args,
                  uint32_t argCount, NPVariant *result)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_Evaluate(NPP npp, NPObject *npobj, NPString *script,
             NPVariant *result)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                NPVariant *result)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPN_GetValue(NPP instance, NPNVariable aVariable, void *aValue)
{
    if (NPPFuncs.getvalue) {
        return NPNFuncs.getvalue(instance, aVariable, aValue);
    }
    return NPERR_GENERIC_ERROR;
}

NPError
NPN_SetValue(NPP instance, NPPVariable aVariable, void *aValue)
{
    if (NPPFuncs.setvalue) {
        return NPNFuncs.setvalue(instance, aVariable, aValue);
    }
    return NPERR_GENERIC_ERROR;
}

void
NPN_InvalidateRect(NPP instance, NPRect *invalidRect)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NPN_ReloadPlugins(NPBool reloadPages)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPN_GetURL(NPP instance, const char* url, const char* target)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NP_LOADDS NPN_PostURL(NPP instance, const char* url,
                      const char* target, uint32_t len,
                      const char* buf, NPBool file)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPN_NewStream(NPP instance, NPMIMEType type, const char* target,
              NPStream** stream)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NPN_Status(NPP instance, const char* message)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

const char *
NPN_UserAgent(NPP instance)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

int32_t
NPN_Write(NPP instance, NPStream* stream, int32_t len, void* buffer)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPError
NPN_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NS_PluginShutdown()
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

#ifdef NPAPI_CONST
const
#endif
char*
NPP_GetMIMEDescription(void)
{
    char *x = 0;
    return x;
}

void
NS_DestroyPluginInstance(nsPluginInstanceBase *aPlugin)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

std::map<NPIdentifier, NPVariant *> _properties;
std::map<NPIdentifier,  NPInvokeFunctionPtr> _methods;

// Implement minimal properties handling
bool
NPN_SetProperty(NPP npp, NPObject* obj, NPIdentifier name,
                     const NPVariant *value)
{
    _properties[name] = const_cast<NPVariant *>(value);
}

bool
NPN_GetProperty(NPP npp, NPObject* obj, NPIdentifier name,
                     const NPVariant *value)
{
    return _properties[name];
}

bool
NPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_HasProperty(NPP npp, NPObject* obj, NPIdentifier name,
                     const NPVariant *value)
{
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        return true;
    }
}


void
NPN_SetException(NPObject *npobj, const NPUTF8 *message)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NPN_PluginThreadAsyncCall(NPP plugin, void (*func)(void *), void *userData)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
              uint32_t *count)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_Construct(NPP npp, NPObject *npobj, const NPVariant *args,
              uint32_t argCount, NPVariant *result)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NPN_ReleaseVariantValue(NPVariant *variant)
{
    switch(variant->type) {
        case NPVariantType_String:
        {
            NPN_MemFree(const_cast<NPUTF8*>(NPVARIANT_TO_STRING(*variant).UTF8Characters));
            break;
        }
        case NPVariantType_Object:
        {
            NPObject* obj = NPVARIANT_TO_OBJECT(*variant);
            if (obj) {
                NPN_ReleaseObject(obj);
            }
            break;
        }
        default:
        {}
    }
 
    NULL_TO_NPVARIANT(*variant);
}

NPObject *
NPN_CreateObject(NPP npp, NPClass *aClass)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

void
NPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount,
                         NPIdentifier *identifiers)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPIdentifier
NPN_GetIntIdentifier(int32_t intid)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

bool
NPN_IdentifierIsString(NPIdentifier identifier)
{
    cerr << "UNIMPLEMENTED: " << __PRETTY_FUNCTION__ << endl;
}

NPObject*
NPN_RetainObject(NPObject *obj)
{
    assert(obj); ++obj->referenceCount; return obj;
}

void
NPN_ReleaseObject(NPObject *npobj)
{
    assert(npobj);
    --npobj->referenceCount;
    if (npobj->referenceCount == 0) {
        NPN_MemFree(npobj);
    }
}
// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
