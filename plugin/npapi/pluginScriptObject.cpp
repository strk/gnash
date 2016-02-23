// 
//   Copyright (C) 2010, 2011, 2012, 2014, 2016 Free Software Foundation, Inc
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

#include <map>
#include <string>
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include <ws2tcpip.h>
#else
# include <sys/ioctl.h>
# include <unistd.h>
# include <poll.h>
#endif
#include "npapi.h"
#include "npruntime.h"
#include "plugin.h" 
#include "callbacks.h" 
#include "external.h" 
#include "pluginScriptObject.h"

extern NPNetscapeFuncs NPNFuncs;

namespace gnash {

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

void
printNPVariant(const NPVariant *value)
{
    if (NPVARIANT_IS_DOUBLE(*value)) {
        double num = NPVARIANT_TO_DOUBLE(*value);
        log_debug("is double, value %g", num);
    } else if (NPVARIANT_IS_STRING(*value)) {
        std::string str = NPStringToString(NPVARIANT_TO_STRING(*value));
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
                                     const std::string &val)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());

    NPVariant strvar;
    STRINGN_TO_NPVARIANT(val.c_str(), static_cast<int>(val.size()), strvar);
    SetProperty(id, strvar);
}

void
GnashPluginScriptObject::AddProperty(const std::string &name, double num)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant value;
    DOUBLE_TO_NPVARIANT(num, value);
    SetProperty(id, value);
}

void
GnashPluginScriptObject::AddProperty(const std::string &name, int num)
{
    NPIdentifier id = NPN_GetStringIdentifier(name.c_str());
    NPVariant value;
    INT32_TO_NPVARIANT(num, value);
    SetProperty(id, value);
}

// Sets up the property and method identifier arrays used by the browser
// via the hasProperty and hasMethod fuction pointers
void
GnashPluginScriptObject::initializeIdentifiers()
{
    // log_debug("initializeIdentifiers");

//    NPN_Status(nppinstance, __FUNCTION__);
    
//    http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_04.html
    
    // We maintain an internal property for our version number, rather
    // than asking the player.
    AddProperty("$version", "10,1,r999");
    // id and name appear to be the same tag, but differeing browsers access
    // one or the other, or both.
    // name=send_this_page_swf
    AddProperty("name", "Hello World");
    // id=send_this_page_swf
    AddProperty("id", "Hello World");

    // http://s.ytimg.com/yt/swf/watch-vfl161193.swf
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
    
    // Add the default methods
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
    
    // id = NPN_GetStringIdentifier("TestASMethod");
    // AddMethod(id, remoteCallback);
};

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject()
    : nppinstance (nullptr),
      _controlfd(-1),
      _hostfd(-1)
{
//    log_debug(__PRETTY_FUNCTION__);
    
    initializeIdentifiers();
    
}

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject(NPP npp)
    : nppinstance (npp),
      _controlfd(-1),
      _hostfd(-1)
{
//    log_debug(__PRETTY_FUNCTION__);
    
    initializeIdentifiers();

}

// Destructor
GnashPluginScriptObject::~GnashPluginScriptObject() 
{
//    log_debug(__PRETTY_FUNCTION__);
}

// Marshal Functions
NPClass *
GnashPluginScriptObject::marshalGetNPClass() 
{
//    log_debug(__PRETTY_FUNCTION__);
    return &GnashPluginScriptObjectClass;
}

NPObject *
GnashPluginScriptObject::marshalAllocate (NPP npp, NPClass */* aClass */)
{
//    log_debug(__PRETTY_FUNCTION__);
    return new GnashPluginScriptObject(npp);
}


void 
GnashPluginScriptObject::marshalDeallocate (NPObject *npobj)
{
//    log_debug(__PRETTY_FUNCTION__);
    delete (GnashPluginScriptObject *)npobj;
}

void 
GnashPluginScriptObject::marshalInvalidate (NPObject */* npobj */)
{
//    log_debug(__PRETTY_FUNCTION__);

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
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->Invoke(npobj, name, args, argCount, result);
}

bool 
GnashPluginScriptObject::marshalInvokeDefault (NPObject *npobj,
                                               const NPVariant *args,
                                               uint32_t argCount,
                                               NPVariant *result)
{
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->InvokeDefault(args, argCount, result);
}

bool 
GnashPluginScriptObject::marshalHasProperty (NPObject *npobj, NPIdentifier name)
{
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    return gpso->HasProperty(name);
}

bool 
GnashPluginScriptObject::marshalGetProperty (NPObject *npobj, NPIdentifier name,
                                             NPVariant *result)
{
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    return gpso->GetProperty(name, result);    
}

bool 
GnashPluginScriptObject::marshalSetProperty (NPObject *npobj, NPIdentifier name,
                                             const NPVariant *value)
{
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;    
    return gpso->SetProperty(name, *value);
}

bool 
GnashPluginScriptObject::marshalRemoveProperty (NPObject *npobj, NPIdentifier name)
{
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
#if 0
    log_debug("Checking for Property \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif
    
    return _properties.find(name) != _properties.end();
};

bool
GnashPluginScriptObject::GetProperty(NPIdentifier name, NPVariant *result)
{
    if (NPN_IdentifierIsString(name)) {
        log_debug("Getting Property \"%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("Getting Property \"%d\"...", NPN_IntFromIdentifier(name));
    }

    std::map<NPIdentifier, GnashNPVariant>::const_iterator it;
    it = _properties.find(name);
    if (it == _properties.end()) {
        return false;
    }

    const GnashNPVariant& val = it->second;
    val.copy(*result);

    return true;
};

bool
GnashPluginScriptObject::SetProperty(NPIdentifier name, const NPVariant& value)
{
    _properties[name] = value;

    return false;
}

bool
GnashPluginScriptObject::RemoveProperty(NPIdentifier name)
{
    std::map<NPIdentifier, GnashNPVariant>::iterator it;
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
#if 0
    log_debug("Checking for Method \"");
    if (NPN_IdentifierIsString(name)) {
        log_debug("%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    return _methods.find(name) != _methods.end();
}

bool
GnashPluginScriptObject::Invoke(NPObject */* npobj */, NPIdentifier name,
                                const NPVariant *args, uint32_t argCount,
                                NPVariant *result)
{
//    log_debug(__PRETTY_FUNCTION__);
    
#if 1
    if (NPN_IdentifierIsString(name)) {
        log_debug("Invoking Method \"%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("Invoking Method: \"%d\"...", NPN_IntFromIdentifier(name));
    }
    // log_debug("SCRIPT OBJECT invoke %s: %x", NPN_UTF8FromIdentifier(name),
    //           (void *)npobj);    
#endif

    std::map<NPIdentifier, NPInvokeFunctionPtr>::iterator it;
    it = _methods.find(name);
    if (it != _methods.end()) {
        // log_debug("FOUND Method \"%s\"!", NPN_UTF8FromIdentifier(name));
        NPInvokeFunctionPtr func = it->second;
        return func(this, name, args, argCount, result);
    } else {
        log_error("Couldn't find Method \"%s\"", NPN_UTF8FromIdentifier(name));
    }

    return false;
    
//    return NPN_Invoke(nppinstance, this, name, args, argCount, result);
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
    if (NPN_IdentifierIsString(name)) {
        log_debug("Adding Method \"%s\"...", NPN_UTF8FromIdentifier(name));
    } else {
        log_debug("Adding Method \"%d\"...", NPN_IntFromIdentifier(name));
    }
#endif

    _methods[name] = func;
    
    return true;
}

// SetVariable sends a message to the player that looks like this:
// "Command Name Type value\n", ie... "SetVariable var1 string value1\n"
bool
GnashPluginScriptObject::SetVariable(const std::string &name,
                                     const NPVariant& value)
{
    std::vector<std::string> iargs;
    std::string str = plugin::ExternalInterface::makeString(name);
    iargs.push_back(str);
    str = plugin::ExternalInterface::convertNPVariant(&value);
    iargs.push_back(str);
    str = plugin::ExternalInterface::makeInvoke("SetVariable", iargs);
    
    log_debug("Trying to set a value for %s.", name);

    // Write the message to the Control FD.
    size_t ret = writePlayer(str);
    // Unless we wrote the same amount of data as the message contained,
    // something went wrong.
    if (ret != str.size()) {
        log_error("Couldn't set the variable, network problems.");
        return false;
    }
    
    return true;
}

// GetVariable sends a message to the player that looks like this:
// "Command Name\n", ie... "GetVariable var1\n". Then it waits
// for the response with the type and value.
GnashNPVariant
GnashPluginScriptObject::GetVariable(const std::string &name)
{
    std::vector<std::string> iargs;
    std::string str = plugin::ExternalInterface::makeString(name);
    iargs.push_back(str);
    str = plugin::ExternalInterface::makeInvoke("GetVariable", iargs);

    log_debug("Trying to get a value for %s.", name);
    
    size_t ret = writePlayer(str);
    if (ret != str.size()) {
        // If all the browser wants is the version, we don't need to
        // ask the standalone player for this value. YouTube at
        // least depends on this for some pages which want this to
        // be greater than 8.0.0. This appears to potentially be
        // Google's way of trying to revent downloaders, as this requires
        // plugin support.
        NPVariant value;
        if (name == "$version") {
            STRINGN_TO_NPVARIANT("LNX 10,0,r999", 13, value);
        } else {
            log_error("Couldn't send GetVariable request, network problems.");
            NULL_TO_NPVARIANT(value);
        }
        return value;
    }

    // Have the read function allocate the memory
    std::string data = readPlayer();
    if (data.empty()) {
        return GnashNPVariant();
    }

    GnashNPVariant parsed = plugin::ExternalInterface::parseXML(this, data);

    printNPVariant(&parsed.get());
    
    return parsed;
}

void
GnashPluginScriptObject::setControlFD(int x)
{
//    log_debug("%s: %d", __FUNCTION__, x);
    _controlfd = x;              // FIXME: this should go away
}

int
GnashPluginScriptObject::getControlFD()
{
// log_debug("getControlFD: %d", controlfd);

    return _controlfd;
};

void
GnashPluginScriptObject::setHostFD(int x)
{
//    log_debug("%s: %d", __FUNCTION__, x);
    _hostfd = x;              // FIXME: this should go away
}

int
GnashPluginScriptObject::getHostFD()
{
// log_debug("getControlFD: %d", controlfd);

    return _hostfd;
};


// Write to the standalone player over the control socket
int
GnashPluginScriptObject::writePlayer(const std::string &data)
{
    return writePlayer(_controlfd, data);
}

int
GnashPluginScriptObject::writePlayer(int fd, const std::string &data)
{
//    log_debug(__PRETTY_FUNCTION__);

//    log_debug("Writing data to fd #%d:\n %s", fd, data);

    if (fd > 2) {
        return ::write(fd, data.c_str(), data.size());
    }
    
    return 0;
}

std::string
GnashPluginScriptObject::readPlayer()
{
    return readPlayer(_hostfd);
}

std::string
GnashPluginScriptObject::readPlayer(int fd)
{
    std::string empty;

    if (fd <= 0) {
        log_error("Invalid fd passed");
        return empty;
    }

    // Wait for some data from the player
    int bytes = 0;

    pollfd pfds[1] = { pollfd() };
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;

    int rv = poll(pfds, 1 /* arraySize */, 2000 /* ms */);

    // No data yet
    if (rv <= 0) {
        return empty;
    }

#ifndef _WIN32
    rv = ioctl(fd, FIONREAD, &bytes);
#else
    rv = ioctlSocket(fd, FIONREAD, &bytes);
#endif
    if (rv < 0) {
        log_error("FIONREAD ioctl failed, unable to get network buffer length");
        return empty;
    }
    log_debug("There are %d bytes in the network buffer", bytes);

    if (bytes <= 0) {
        return empty;
    }

    char buf[bytes];

    int ret = ::read(fd, buf, bytes);
    if (ret <= 0 || ret > bytes) {
        return empty;
    }

    return std::string(buf, ret);
}




} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
