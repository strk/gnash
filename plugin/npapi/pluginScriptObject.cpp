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

#include "npapi.h"
#include "npruntime.h"
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

#if 0

#define NUM_METHOD_IDENTIFIERS       3
static NPIdentifier pluginMethodIdentifiers[NUM_METHOD_IDENTIFIERS];
static const NPUTF8 *pluginMethodIdentifierNames[NUM_METHOD_IDENTIFIERS] = {
   "setVariable",
   "getVariable",
   "showFoobar"
};
#endif

bool
testfunc (NPObject */* npobj */, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t /* argCount */, NPVariant *result)
{   
    GnashLogDebug(__PRETTY_FUNCTION__);
    
    DOUBLE_TO_NPVARIANT(122333.4444, *result);
    
    return true;
}

void
GnashPluginScriptObject::AddProperty(const std::string &name,
                                     const std::string &str)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant *value = new NPVariant;
    int length = str.size();;
    char *bar = new char[length+1];
    std::copy(str.begin(), str.end(), bar);
    STRINGN_TO_NPVARIANT(bar, length, *value);
    SetProperty(id, value);
}

void
GnashPluginScriptObject::AddProperty(const std::string &name, double num)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant *value = new NPVariant;
    DOUBLE_TO_NPVARIANT(num, *value);
    SetProperty(id, value);
}

void
GnashPluginScriptObject::AddProperty(const std::string &name, int num)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant *value = new NPVariant;
    INT32_TO_NPVARIANT(num, *value);
    SetProperty(id, value);
}

// Sets up the property and method identifier arrays used by the browser
// via the hasProperty and hasMethod fuction pointers
void
GnashPluginScriptObject::initializeIdentifiers()
{
    GnashLogDebug("initializeIdentifiers");

    AddProperty("src", "example");
    AddProperty("align", "middle");
    AddProperty("quality", "high");
    AddProperty("bgcolor", "#FFFFFF");
    AddProperty("allowScriptAccess", "sameDomain");
    AddProperty("type", "application/x-shockwave-flash");
    AddProperty("codebase", "http://www.getgnash.org");
    AddProperty("pluginspage", "http://www.getgnash.org");

    AddProperty("classid", "unneeded for free software");
    AddProperty("movie", "unknown");
    AddProperty("id", "id unknown");
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
   // We maintain an internal property for our version number, rather
   // than asking the player.
   AddProperty("version", 456.789);
   AddProperty("hello", "Hello World");
   
   NPIdentifier fid = NPN_GetStringIdentifier("showFoobar");
   NPInvokeFunctionPtr func = testfunc;
   
   AddMethod(fid, func);

};

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject()
    : _nppinstance (0)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);
    initializeIdentifiers();
}

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject(NPP npp)
    : _nppinstance (npp)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);
    initializeIdentifiers();
}

// Destructor
GnashPluginScriptObject::~GnashPluginScriptObject() 
{
    // GnashLogDebug(__PRETTY_FUNCTION__);
    // do nothing
}

// Marshal Functions
NPClass *
GnashPluginScriptObject::marshalGetNPClass() 
{
    // GnashLogDebug(__PRETTY_FUNCTION__);
    return &GnashPluginScriptObjectClass;
}

NPObject *
GnashPluginScriptObject::marshalAllocate (NPP npp, NPClass */* aClass */)
{
    GnashLogDebug(__PRETTY_FUNCTION__);
    return new GnashPluginScriptObject(npp);
}


void 
GnashPluginScriptObject::marshalDeallocate (NPObject *npobj)
{
    GnashLogDebug(__PRETTY_FUNCTION__);
//    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
//    gpso->Deallocate();
    delete (GnashPluginScriptObject *)npobj;
}

void 
GnashPluginScriptObject::marshalInvalidate (NPObject *npobj)
{
    GnashLogDebug(__PRETTY_FUNCTION__);
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

//    gpso->Invalidate();
}

bool 
GnashPluginScriptObject::marshalHasMethod (NPObject *npobj, NPIdentifier name)
{
    // GnashLogDebug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

#if 0
    printf("Checking for Method: ");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\n", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\n", NPN_IntFromIdentifier(name));
    }
#endif
    
    return gpso->HasMethod(name);
}

bool 
GnashPluginScriptObject::marshalInvoke (NPObject *npobj, NPIdentifier name,
                                        const NPVariant *args, uint32_t argCount,
                                        NPVariant *result)
{
    // GnashLogDebug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->Invoke(name, args, argCount, result);
}

bool 
GnashPluginScriptObject::marshalInvokeDefault (NPObject *npobj,
                                               const NPVariant *args,
                                               uint32_t argCount,
                                               NPVariant *result)
{
    // GnashLogDebug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->InvokeDefault(args, argCount, result);
}

bool 
GnashPluginScriptObject::marshalHasProperty (NPObject *npobj, NPIdentifier name)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    return gpso->HasProperty(name);
}

bool 
GnashPluginScriptObject::marshalGetProperty (NPObject *npobj, NPIdentifier name,
                                             NPVariant *result)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->GetProperty(name, result);    
}

bool 
GnashPluginScriptObject::marshalSetProperty (NPObject *npobj, NPIdentifier name,
                                             const NPVariant *value)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;    
    return gpso->SetProperty(name, value);
}

bool 
GnashPluginScriptObject::marshalRemoveProperty (NPObject *npobj, NPIdentifier name)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;    
    return gpso->RemoveProperty(name);
}

bool
GnashPluginScriptObject::HasProperty(NPIdentifier name)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);

#if 0
    printf("Checking for Property \"");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif
    
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        // printf(" FOUND\n");
        return true;
    }
    
    return false;
};

bool
GnashPluginScriptObject::GetProperty(NPIdentifier name, NPVariant *result)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);

    printf("Getting Property \"");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\"...", NPN_IntFromIdentifier(name));
    }
    
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        printf(" FOUND = ");
        // We have to copy the data we hold internally into the result
        // data object, rather than just resetting the result point to
        // our internal copy, which doesn't work.
        NPVariant *value = it->second;
        if (NPVARIANT_IS_DOUBLE(*value)) {
            double num = NPVARIANT_TO_DOUBLE(*value);
            printf(" %g\n", num);
            DOUBLE_TO_NPVARIANT(num, *result);
        } else if (NPVARIANT_IS_STRING(*value)) {
            printf(" %s\n", NPVARIANT_TO_STRING(*value).UTF8Characters);
            STRINGN_TO_NPVARIANT(NPVARIANT_TO_STRING(*value).UTF8Characters,
                                 NPVARIANT_TO_STRING(*value).UTF8Length,
                                 *result);
        } else if (NPVARIANT_IS_BOOLEAN(*value)) {
            printf(" %d\n", NPVARIANT_TO_BOOLEAN(*value));
            BOOLEAN_TO_NPVARIANT(NPVARIANT_TO_BOOLEAN(*value), *result);
        } else if (NPVARIANT_IS_INT32(*value)) {
            printf(" %d\n", NPVARIANT_TO_INT32(*value));
            INT32_TO_NPVARIANT(NPVARIANT_TO_INT32(*value), *result);
        } else if (NPVARIANT_IS_NULL(*value)) {
            printf(" null value\n");
            NULL_TO_NPVARIANT(*result);
        } else if (NPVARIANT_IS_VOID(*value)) {
            printf(" void value\n");
            VOID_TO_NPVARIANT(*result);
        } else if (NPVARIANT_IS_OBJECT(*value)) {
            printf(" object\n");
            OBJECT_TO_NPVARIANT(NPVARIANT_TO_OBJECT(*value), *result);
        }
        return true;
    }

    return false;
};

bool
GnashPluginScriptObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
    // GnashLogDebug(__PRETTY_FUNCTION__);

    _properties[name] = const_cast<NPVariant *>(value);

    return false;
}

bool
GnashPluginScriptObject::RemoveProperty(NPIdentifier name)
{
    // GnashLogDebug(__PRETTY_FUNCTION__);

    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        _properties.erase(it);
        return true;
    }    
    
    return false;
}

bool
GnashPluginScriptObject::HasMethod(NPIdentifier name)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);

#if 0
    printf("Checking for Method \"");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    std::map<NPIdentifier, NPInvokeFunctionPtr>::iterator it;
    it = _methods.find(name);
    if (it != _methods.end()) {
        // printf(" FOUND\n");
        return true;
    }    
    
    return false;
}

bool
GnashPluginScriptObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);
#if 1
    printf("Invoking Method \"");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    std::map<NPIdentifier, NPInvokeFunctionPtr>::iterator it;
    it = _methods.find(name);
    if (it != _methods.end()) {
        printf(" FOUND\n");
        NPInvokeFunctionPtr func = it->second;
        return func(NULL, name, args, argCount, result);
    }    

    return false;
}

bool
GnashPluginScriptObject::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    GnashLogDebug(__PRETTY_FUNCTION__);
#if 0
    printf("Invoking Default Method \"");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\"...\n", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\"...\n", NPN_IntFromIdentifier(name));
    }
#endif

    return false;
}

bool
GnashPluginScriptObject::AddMethod(NPIdentifier name, NPInvokeFunctionPtr func)
{
//    GnashLogDebug(__PRETTY_FUNCTION__);
    
#if 0
    printf("Adding Method \"");
    if (NPN_IdentifierIsString(name)) {
        printf("%s\"...\n", NPN_UTF8FromIdentifier(name));
    } else {
        printf("%d\"...\n", NPN_IntFromIdentifier(name));
    }
#endif

    _methods[name] = func;
    
    return true;
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
