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
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include <ws2tcpip.h>
#else
# include <sys/ioctl.h>
# include <unistd.h>
# include <sys/select.h>
#endif
#include "npapi.h"
#include "npruntime.h"
#include "plugin.h" 
#include "callbacks.h" 
#include "pluginScriptObject.h"

extern NPNetscapeFuncs NPNFuncs;

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
    GnashPluginScriptObject::marshalRemoveProperty,
    GnashPluginScriptObject::marshalEnumerate,
    GnashPluginScriptObject::marshalConstruct
};

static int controlfd = -1;

#if 0

// http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_04.html
// Tell Target Methods. Use TGetProperty, TGetPropertyAsNumber, TSetProperty
TCallLabel
TCurrentFrame
TCurrentLabel
TGetProperty
TGetPropertyAsNumber
TGotoFrame
TGotoLabel
TPlay
TSetProperty
TStopPlay

// Target Properties
X_POS
Y_POS
X_SCALE
Y_SCALE
CURRENT_FRAME                   // read-only
TOTAL_FRAMES                    // read-only
ALPHA
VISIBLE
WIDTH                           // read-only
HEIGHT                          // read-only
ROTATE
TARGET                          // read-only
FRAMES_LOADED                   // read-only
NAME
DROP_TARGET                     // read-only
URL                             // read-only
HIGH_QUALITY
FOCUS_RECT
SOUND_BUF_TIME

// Standard Events
OnProgress
OnReadyStateChange
FSCommand
#endif

bool
testfunc (NPObject */* npobj */, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t /* argCount */, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    DOUBLE_TO_NPVARIANT(122333.4444, *result);
    
    return true;
}

void
printNPVariant(NPVariant *value)
{
    if (NPVARIANT_IS_DOUBLE(*value)) {
        double num = NPVARIANT_TO_DOUBLE(*value);
        log_debug("is double, value %g", num);
    } else if (NPVARIANT_IS_STRING(*value)) {
        std::string str(NPVARIANT_TO_STRING(*value).UTF8Characters);
        log_debug("is string, value %s", str);
    } else if (NPVARIANT_IS_BOOLEAN(*value)) {
        bool flag = NPVARIANT_TO_BOOLEAN(*value);
        log_debug("is boolean, value %d", flag);
    } else if (NPVARIANT_IS_INT32(*value)) {
        int num = NPVARIANT_TO_INT32(*value);
        log_debug("is int, value %d", num);
    } else if (NPVARIANT_IS_NULL(*value)) {
        log_debug("value is null");
    } else if (NPVARIANT_IS_VOID(*value)) {
        log_debug("value is void");
    } else if (NPVARIANT_IS_OBJECT(*value)) {
        log_debug("value is object");
    }    
}

//
// The methods for GnashPluginScriptObject start here.
//

void
GnashPluginScriptObject::AddProperty(const std::string &name,
                                     const std::string &str)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));
    int length = str.size();;
    char *bar = (char *)NPN_MemAlloc(length+1);
    std::copy(str.begin(), str.end(), bar);
    bar[length] = 0;  // terminate the new string or bad things happen
    
    // When an NPVariant becomes a string object, it *does not* make a copy.
    // Instead it stores the pointer (and length) we just allocated.
    STRINGN_TO_NPVARIANT(bar, length, *value);
    SetProperty(id, value);
}

void
GnashPluginScriptObject::AddProperty(const std::string &name, double num)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));
    DOUBLE_TO_NPVARIANT(num, *value);
    SetProperty(id, value);
}

void
GnashPluginScriptObject::AddProperty(const std::string &name, int num)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));
    INT32_TO_NPVARIANT(num, *value);
    SetProperty(id, value);
}

// Sets up the property and method identifier arrays used by the browser
// via the hasProperty and hasMethod fuction pointers
void
GnashPluginScriptObject::initializeIdentifiers()
{
//    log_debug("initializeIdentifiers");

//    NPN_Status(_nppinstance, __FUNCTION__);
    
//    http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_04.html
    
    // We maintain an internal property for our version number, rather
    // than asking the player.
    AddProperty("$version", "10,1,r999");
    // id and name appear to be the same tag, but differeing browsers access
    // one or the other, or both.
    AddProperty("name", "Hello World");
    AddProperty("id", "Hello World");

    AddProperty("src", "example");
    AddProperty("align", "middle");
    AddProperty("quality", "high");
    AddProperty("bgcolor", "#FFFFFF");
    AddProperty("allowScriptAccess", "sameDomain");
    AddProperty("type", "application/x-shockwave-flash");
    AddProperty("codebase", "http://www.getgnash.org");
    AddProperty("pluginspage", "http://www.getgnash.org");

    AddProperty("classid", "2b70f2b1-fc72-4734-bb81-4eb2a7713e49");
    AddProperty("movie", "unknown");
    AddProperty("width", 0);
    AddProperty("height", 0);
    AddProperty("vspace", 0);
    AddProperty("hspace", 0);
    AddProperty("class", "class unknown");
    AddProperty("title", "title unknown");
    AddProperty("accesskey", 0);
    AddProperty("name", "name unknown");
    AddProperty("tabindex", 8);
    AddProperty("FlashVars", "flashVars unknown");

    // Javascript and flash events
    AddProperty("onafterupdate", "unknown");
    AddProperty("onbeforeupdate", "unknown");
    AddProperty("onblur", "unknown");
    AddProperty("oncellchange", "unknown");
    AddProperty("onclick", "unknown");
    AddProperty("ondblClick", "unknown");
    AddProperty("ondrag", "unknown");
    AddProperty("ondragend", "unknown");
    AddProperty("ondragenter", "unknown");
    AddProperty("ondragleave", "unknown");
    AddProperty("ondragover", "unknown");
    AddProperty("ondrop", "unknown");
    AddProperty("onfinish", "unknown");
    AddProperty("onfocus", "unknown");
    AddProperty("onhelp", "unknown");
    AddProperty("onmousedown", "unknown");
    AddProperty("onmouseup", "unknown");
    AddProperty("onmouseover", "unknown");
    AddProperty("onmousemove", "unknown");
    AddProperty("onmouseout", "unknown");
    AddProperty("onkeypress", "unknown");
    AddProperty("onkeydown", "unknown");
    AddProperty("onkeyup", "unknown");
    AddProperty("onload", "unknown");
    AddProperty("onlosecapture", "unknown");
    AddProperty("onpropertychange", "unknown");
    AddProperty("onreadystatechange", "unknown");
    AddProperty("onrowsdelete", "unknown");
    AddProperty("onrowenter", "unknown");
    AddProperty("onrowexit", "unknown");
    AddProperty("onrowsinserted", "unknown");
    AddProperty("onstart", "");
    AddProperty("onscroll", "unknown");
    AddProperty("onbeforeeditfocus", "unknown");
    AddProperty("onactivate", "unknown");
    AddProperty("onbeforedeactivate", "unknown");
    AddProperty("ondeactivate", "unknown");
    
#if 0
   for (int i = 0; i < NUM_METHOD_IDENTIFIERS; i++) {
       SetProperty(pluginMethodIdentifiers[i], value);
   }
   
   // fill the method identifier array
   NPNFuncs.getstringidentifiers(pluginMethodIdentifierNames,
                                 NUM_METHOD_IDENTIFIERS,
                                 pluginMethodIdentifiers);
#endif
//     NPIdentifier id = NPN_GetStringIdentifier("showFoobar");
//    NPInvokeFunctionPtr func = testfunc;  
//    AddMethod(id, func);

   NPIdentifier id = NPN_GetStringIdentifier("SetVariable");
   AddMethod(id, SetVariableCallback);

   id = NPN_GetStringIdentifier("GetVariable");
   AddMethod(id, GetVariableCallback);

   id = NPN_GetStringIdentifier("GotoFrame");
   AddMethod(id, GotoFrame);

   id = NPN_GetStringIdentifier("IsPlaying");
   AddMethod(id, IsPlaying);

   id = NPN_GetStringIdentifier("LoadMovie");
   AddMethod(id, LoadMovie);

   id = NPN_GetStringIdentifier("Pan");
   AddMethod(id, Pan);

   id = NPN_GetStringIdentifier("PercentLoaded");
   AddMethod(id, PercentLoaded);

   id = NPN_GetStringIdentifier("Play");
   AddMethod(id, Play);

   id = NPN_GetStringIdentifier("Rewind");
   AddMethod(id, Rewind);

   id = NPN_GetStringIdentifier("SetZoomRect");
   AddMethod(id, SetZoomRect);

   id = NPN_GetStringIdentifier("StopPlay");
   AddMethod(id, StopPlay);

   id = NPN_GetStringIdentifier("Zoom");
   AddMethod(id, Zoom);

   id = NPN_GetStringIdentifier("TotalFrames");
   AddMethod(id, TotalFrames);
   
};

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject()
    : _nppinstance (0)
{
//    log_debug(__PRETTY_FUNCTION__);
    initializeIdentifiers();
}

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject(NPP npp)
    : _nppinstance (npp)
{
//    log_debug(__PRETTY_FUNCTION__);
    initializeIdentifiers();
}

// Destructor
GnashPluginScriptObject::~GnashPluginScriptObject() 
{
    // log_debug(__PRETTY_FUNCTION__);
    // do nothing
}

// Marshal Functions
NPClass *
GnashPluginScriptObject::marshalGetNPClass() 
{
    // log_debug(__PRETTY_FUNCTION__);
    return &GnashPluginScriptObjectClass;
}

NPObject *
GnashPluginScriptObject::marshalAllocate (NPP npp, NPClass */* aClass */)
{
    log_debug(__PRETTY_FUNCTION__);
#if 0
    GnashPluginScriptObject *npobj = reinterpret_cast<GnashPluginScriptObject *>
        (NPN_MemAlloc(sizeof(GnashPluginScriptObject)));
    npobj->setInstance(npp);
    return npobj;
#else
    return new GnashPluginScriptObject(npp);
#endif
}


void 
GnashPluginScriptObject::marshalDeallocate (NPObject *npobj)
{
    log_debug(__PRETTY_FUNCTION__);
#if 0
    NPN_MemFree(reinterpret_cast<void *>(npobj));
#else
    delete (GnashPluginScriptObject *)npobj;
#endif
}

void 
GnashPluginScriptObject::marshalInvalidate (NPObject */* npobj */)
{
    log_debug(__PRETTY_FUNCTION__);

//    gpso->Invalidate();
}

bool 
GnashPluginScriptObject::marshalHasMethod (NPObject *npobj, NPIdentifier name)
{
    // log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

#if 0
    log_debug("Checking for Method: ");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d", NPN_IntFromIdentifier(name));
    }
#endif
    
    return gpso->HasMethod(name);
}

bool 
GnashPluginScriptObject::marshalInvoke (NPObject *npobj, NPIdentifier name,
                                        const NPVariant *args, uint32_t argCount,
                                        NPVariant *result)
{
    // log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->Invoke(name, args, argCount, result);
}

bool 
GnashPluginScriptObject::marshalInvokeDefault (NPObject *npobj,
                                               const NPVariant *args,
                                               uint32_t argCount,
                                               NPVariant *result)
{
    // log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->InvokeDefault(args, argCount, result);
}

bool 
GnashPluginScriptObject::marshalHasProperty (NPObject *npobj, NPIdentifier name)
{
//    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    return gpso->HasProperty(name);
}

bool 
GnashPluginScriptObject::marshalGetProperty (NPObject *npobj, NPIdentifier name,
                                             NPVariant *result)
{
//    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->GetProperty(name, result);    
}

bool 
GnashPluginScriptObject::marshalSetProperty (NPObject *npobj, NPIdentifier name,
                                             const NPVariant *value)
{
//    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;    
    return gpso->SetProperty(name, value);
}

bool 
GnashPluginScriptObject::marshalRemoveProperty (NPObject *npobj, NPIdentifier name)
{
//    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;    
    return gpso->RemoveProperty(name);
}

bool 
GnashPluginScriptObject::marshalEnumerate (NPObject *npobj, void***identifier,
                                           uint32_t *count)
{
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    return gpso->Enumerate(identifier, count);

    return false;
}

bool 
GnashPluginScriptObject::marshalConstruct (NPObject *npobj, const NPVariant *data,
                                  uint32_t count, NPVariant *result)
{
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;    
    return gpso->Construct(data, count, result);

    return false;
}

bool
GnashPluginScriptObject::HasProperty(NPIdentifier name)
{
//    log_debug(__PRETTY_FUNCTION__);

#if 0
    log_debug("Checking for Property \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif
    
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        // log_debug(" FOUND");
        return true;
    }
    
    return false;
};

bool
GnashPluginScriptObject::GetProperty(NPIdentifier name, NPVariant *result)
{
//    log_debug(__PRETTY_FUNCTION__);

    log_debug("Getting Property \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
    
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
//        log_debug(" FOUND = ");
        // We have to copy the data we hold internally into the result
        // data object, rather than just resetting the result point to
        // our internal copy, which doesn't work.
        NPVariant *value = it->second;
        if (NPVARIANT_IS_DOUBLE(*value)) {
            double num = NPVARIANT_TO_DOUBLE(*value);
            log_debug(" %g", num);
            DOUBLE_TO_NPVARIANT(num, *result);
        } else if (NPVARIANT_IS_STRING(*value)) {
            log_debug(" %s", NPVARIANT_TO_STRING(*value).UTF8Characters);
            STRINGN_TO_NPVARIANT(NPVARIANT_TO_STRING(*value).UTF8Characters,
                                 NPVARIANT_TO_STRING(*value).UTF8Length,
                                 *result);
        } else if (NPVARIANT_IS_BOOLEAN(*value)) {
            log_debug(" %d", NPVARIANT_TO_BOOLEAN(*value));
            BOOLEAN_TO_NPVARIANT(NPVARIANT_TO_BOOLEAN(*value), *result);
        } else if (NPVARIANT_IS_INT32(*value)) {
            log_debug(" %d", NPVARIANT_TO_INT32(*value));
            INT32_TO_NPVARIANT(NPVARIANT_TO_INT32(*value), *result);
        } else if (NPVARIANT_IS_NULL(*value)) {
            log_debug(" null value");
            NULL_TO_NPVARIANT(*result);
        } else if (NPVARIANT_IS_VOID(*value)) {
            log_debug(" void value");
            VOID_TO_NPVARIANT(*result);
        } else if (NPVARIANT_IS_OBJECT(*value)) {
            log_debug(" object");
            OBJECT_TO_NPVARIANT(NPVARIANT_TO_OBJECT(*value), *result);
        }
        return true;
    }

    return false;
};

bool
GnashPluginScriptObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
    // log_debug(__PRETTY_FUNCTION__);

    _properties[name] = const_cast<NPVariant *>(value);

    return false;
}

bool
GnashPluginScriptObject::RemoveProperty(NPIdentifier name)
{
    // log_debug(__PRETTY_FUNCTION__);

    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        _properties.erase(it);
        return true;
    }    
    
    return false;
}

bool
GnashPluginScriptObject::Enumerate(NPIdentifier **/*identifier */, uint32_t */* count */)
{
    log_debug(__PRETTY_FUNCTION__);

    return false;
}

bool
GnashPluginScriptObject::Construct(const NPVariant */* args */, uint32_t /* argCount */,
                                   NPVariant */* result */)
{
    log_debug(__PRETTY_FUNCTION__);

    return false;
}

bool
GnashPluginScriptObject::HasMethod(NPIdentifier name)
{
//    log_debug(__PRETTY_FUNCTION__);

#if 0
    log_debug("Checking for Method \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    std::map<NPIdentifier, NPInvokeFunctionPtr>::iterator it;
    it = _methods.find(name);
    if (it != _methods.end()) {
        // log_debug(" FOUND");
        return true;
    }    
    
    return false;
}

bool
GnashPluginScriptObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
//    log_debug(__PRETTY_FUNCTION__);
#if 0
    log_debug("Invoking Method \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    std::map<NPIdentifier, NPInvokeFunctionPtr>::iterator it;
    it = _methods.find(name);
    if (it != _methods.end()) {
        // log_debug(" FOUND");
        NPInvokeFunctionPtr func = it->second;
        return func(NULL, name, args, argCount, result);
    }

    return false;
}

bool
GnashPluginScriptObject::InvokeDefault(const NPVariant */* args */,
                          uint32_t /* argCount */, NPVariant */* result */)
{
    log_debug(__PRETTY_FUNCTION__);
#if 0
    log_debug("Invoking Default Method \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    return false;
}

bool
GnashPluginScriptObject::AddMethod(NPIdentifier name, NPInvokeFunctionPtr func)
{
//    log_debug(__PRETTY_FUNCTION__);
    
#if 0
    log_debug("Adding Method \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    _methods[name] = func;
    
    return true;
}

// SetVariable sends a message to the player that looks like this:
// "Command Name Type value\n", ie... "SetVariable var1 string value1\n"
bool
GnashPluginScriptObject::SetVariable(const std::string &name,
                                     NPVariant *value)
{
    log_debug(__PRETTY_FUNCTION__);

    // Build the command message
    std::stringstream ss;
    ss << "SetVariable " << name;
    if (NPVARIANT_IS_DOUBLE(*value)) {
        ss << " double " << NPVARIANT_TO_DOUBLE(*value);
    } else if (NPVARIANT_IS_STRING(*value)) {
        std::string varname = NPVARIANT_TO_STRING(*value).UTF8Characters;
        ss << " string " << varname;
    } else if (NPVARIANT_IS_BOOLEAN(*value)) {
        ss << " boolean\n" << NPVARIANT_TO_BOOLEAN(*value);
    } else if (NPVARIANT_IS_INT32(*value)) {
        ss << " int32\n" << NPVARIANT_TO_INT32(*value);
    } else if (NPVARIANT_IS_NULL(*value)) {
        ss << " null";
    } else if (NPVARIANT_IS_VOID(*value)) {
        ss << " void";
    } else if (NPVARIANT_IS_OBJECT(*value)) {
        ss << " object\n";         // FIXME: add the object data
    }

    // Add the CR, so we know where the message ends when parsing.
    ss << std::endl;

    // Write the message to the Control FD.
    size_t ret = writePlayer(controlfd, ss.str());
    // Unless we wrote the same amount of data as the message contained,
    // something went wrong.
    if (ret != ss.str().size()) {
        log_error("Couldn't set the variable, network problems.");
        return false;
    }

    return true;
}

// GetVariable sends a message to the player that looks like this:
// "Command Name\n", ie... "GetVariable var1\n". Then it waits
// for the response with the type and value.
NPVariant *
GnashPluginScriptObject::GetVariable(const std::string &name)
{
    log_debug(__PRETTY_FUNCTION__);

    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));
    NULL_TO_NPVARIANT(*value);

    std::stringstream ss;
    
    ss << "GetVariable " << name << std::endl;
    size_t ret = writePlayer(controlfd, ss.str());
    if (ret != ss.str().size()) {
        log_error("Couldn't send GetVariable request, network problems.");
        return false;
    }

    // Have the read function allocate the memory
    const char *data = 0;
    char *ptr = 0;
    ptr = const_cast<char *>(data);
    ret = readPlayer(controlfd, &data, 0);
    if (ret == 0) {
        return value;
    }
    // We need a non const pointer to walk through the data.
    ptr = const_cast<char *>(data);

    // Make sure this mesasge is our response, whnich it should be,
    // but you never know...
    if (strncmp(ptr, "GetVariable ", 12) != 0) {
        log_debug("Illegal response! %s", ptr);
        return value;
    } else {
        // A legit response has CR on the end already
        log_debug("Legit response: %s", ptr);
    }    
    ptr += 12;

#if 0
    if (strncmp(ptr, name.c_str(), name.size()) == 0) {
        log_debug("Mismatched variable name! %s", ptr);
        return value;
    } else {
        log_debug("Variable names matched: %s", ptr);
    }
#endif
    ptr += name.size() + 1;     // don't forget to skip the space

    if (strncmp(ptr, "double ", 7) == 0) {
        ptr += 7;
        double num = strtod(ptr, NULL);
        DOUBLE_TO_NPVARIANT(num, *value);
    } else if (strncmp(ptr, "string ", 7) == 0) {
        ptr += 7;
        std::string str(ptr);
        int length = str.size();;
        char *bar = (char *)NPN_MemAlloc(length+1);
        std::copy(str.begin(), str.end(), bar);
        bar[length] = 0;  // terminate the new string or bad things happen
        // When an NPVariant becomes a string object, it *does not* make a copy.
        // Instead it stores the pointer (and length) we just allocated.
        STRINGN_TO_NPVARIANT(bar, length, *value);
    } else if (strncmp(ptr, "boolean ", 8) == 0) {
        ptr += 8;
        bool flag = strtol(ptr, NULL, 0);
        BOOLEAN_TO_NPVARIANT(flag, *value);
    } else if (strncmp(ptr, "int32 ", 6) == 0) {
        ptr += 6;
        int num = strtol(ptr, NULL, 0);
        INT32_TO_NPVARIANT(num, *value);
    } else if (strncmp(ptr, "null ", 5) == 0) {
        ptr += 5;
        NULL_TO_NPVARIANT(*value);
    } else if (strncmp(ptr, "void ", 5) == 0) {
        ptr += 5;
        VOID_TO_NPVARIANT(*value);
    } else if (strncmp(ptr, "object ", 7) == 0) {
        ptr += 7;
        log_debug("\tFIXME: %s = object", name);
        // OBJECT_TO_NPVARIANT(num, *value);
    }

    // free the memory used for the data, as it was allocated in readPlayer().
    NPN_MemFree(reinterpret_cast<void *>(const_cast<char *>(data)));
    
    printNPVariant(value);
    
    return value;
}

void
GnashPluginScriptObject::setControlFD(int x)
{
//    log_debug("%s: %d", __FUNCTION__, x);
    
    controlfd = x;
}

int
GnashPluginScriptObject::getControlFD()
{
    log_debug("getControlFD: %d", controlfd);
    
    return controlfd;
};


// Write to the standalone player over the control socket
int
GnashPluginScriptObject::writePlayer(int fd, const std::string &data)
{
    log_debug(__PRETTY_FUNCTION__);

//    log_debug("Writing data: %s", data);

    if (fd > 2) {
        return writePlayer(fd, data.c_str(), data.size());
    }
    
    return 0;
}

int
GnashPluginScriptObject::writePlayer(int fd, const char *data, size_t length)
{
    log_debug(__PRETTY_FUNCTION__);    

    if (fd > 2) {
        return ::write(fd, data, length);
    }

//    log_debug("Writing data: %s", data);    
    
    return 0;
}

// Read the standalone player over the control socket
int
GnashPluginScriptObject::readPlayer(int /* fd */, const std::string &/* data */)
{
//    log_debug(__PRETTY_FUNCTION__);

//    return readPlayer(fd, data.c_str(), data.size());

    return -1;
}

int
GnashPluginScriptObject::readPlayer(int fd, const char **data, size_t length)
{
//    log_debug(__PRETTY_FUNCTION__);

    if (fd > 0) {
        // Wait for some data from the player
        int bytes = 0;
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        struct timeval tval;
        tval.tv_sec = 10;
        tval.tv_usec = 0;
        log_debug("Waiting for data... ");
        if (select(fd+1, &fdset, NULL, NULL, &tval)) {
            // log_debug("There is data in the network");
#ifndef _WIN32
            ioctl(fd, FIONREAD, &bytes);
#else
            ioctlSocket(fd, FIONREAD, &bytes);
#endif
        // } else {
        //     log_debug("There is no data in thhe network");
        }
        

        log_debug("There are %d bytes in the network buffer", bytes);
        // No data yet
        if (bytes == 0) {
            return 0;
        }

        char *buf = 0;
        if (*data == 0) {
            // Since we know how bytes are in the network buffer, allocate
            // some memory to read the data.
            buf = (char *)NPN_MemAlloc(bytes+1);
            // terminate incase we want to treat the data like a string.
            buf[bytes] = 0;
            length = bytes;
        }
        int ret = ::read(fd, buf, bytes);
        if (ret == bytes) {
            *data = buf;
        }
        return ret;
    }
    
    return 0;
}   

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
