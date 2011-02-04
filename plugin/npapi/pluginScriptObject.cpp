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
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <unistd.h>
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include <ws2tcpip.h>
#else
# include <sys/un.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
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

/// The HostFD is the file descriptor for the socket connection
/// to the standalone player. This is used by this plugin when reading
/// messages from the standalone player.
static int hostfd = -1;

/// The ControlFD is the file descriptor for the socket connection
/// to the standalone player. This is used when writing to the
/// standalone player from this plugin.
static int controlfd = -1;

bool
testfunc (NPObject */* npobj */, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t /* argCount */, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    DOUBLE_TO_NPVARIANT(122333.4444, *result);
    
    return true;
}

void
printNPVariant(const NPVariant *value)
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

//    NPN_Status(_nppinstance, __FUNCTION__);
    
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
    : _nppinstance (0)
{
//    log_debug(__PRETTY_FUNCTION__);
    
    initializeIdentifiers();
    
    _sockfds[READFD] = 0;
    _sockfds[WRITEFD] = 0;
}

// Constructor
GnashPluginScriptObject::GnashPluginScriptObject(NPP npp)
    : _nppinstance (npp)
{
//    log_debug(__PRETTY_FUNCTION__);
    
    initializeIdentifiers();

    _sockfds[READFD] = 0;
    _sockfds[WRITEFD] = 0;
}

// Destructor
GnashPluginScriptObject::~GnashPluginScriptObject() 
{
//    log_debug(__PRETTY_FUNCTION__);
// Should be automatically shutdown by GIO
//    closePipe();
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
//    log_debug(__PRETTY_FUNCTION__);
#if 0
    NPN_MemFree(reinterpret_cast<void *>(npobj));
#else
    delete (GnashPluginScriptObject *)npobj;
#endif
}

void 
GnashPluginScriptObject::marshalInvalidate (NPObject */* npobj */)
{
//    log_debug(__PRETTY_FUNCTION__);

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
        return func(NULL, name, args, argCount, result);
    } else {
        log_error("Couldn't find Method \"%s\"", NPN_UTF8FromIdentifier(name));
    }

    return false;
    
//    return NPN_Invoke(_nppinstance, this, name, args, argCount, result);
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

    GnashNPVariant parsed = plugin::ExternalInterface::parseXML(data);

    printNPVariant(&parsed.get());
    
    return parsed;
}

void
GnashPluginScriptObject::setControlFD(int x)
{
//    log_debug("%s: %d", __FUNCTION__, x);
#if 0
    if (_iochan == 0) {
        _iochan[WRITEFD] = g_io_channel_unix_new(x);
    }
#endif
    controlfd = x;              // FIXME: this should go away
}

int
GnashPluginScriptObject::getControlFD()
{
// log_debug("getControlFD: %d", controlfd);

#if 0
    return g_io_channel_unix_get_fd (_iochan);
#endif    
    return controlfd;
};

void
GnashPluginScriptObject::setHostFD(int x)
{
//    log_debug("%s: %d", __FUNCTION__, x);
#if 0
    if (_iochan == 0) {
        _iochan[WRITEFD] = g_io_channel_unix_new(x);
    }
#endif
    hostfd = x;              // FIXME: this should go away
}

int
GnashPluginScriptObject::getHostFD()
{
// log_debug("getControlFD: %d", controlfd);

#if 0
    return g_io_channel_unix_get_fd (_iochan);
#endif    
    return hostfd;
};


// Write to the standalone player over the control socket
int
GnashPluginScriptObject::writePlayer(const std::string &data)
{
    return writePlayer(controlfd, data);
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
    return readPlayer(hostfd);
}

std::string
GnashPluginScriptObject::readPlayer(int fd)
{
//    log_debug(__PRETTY_FUNCTION__);

    std::string empty;

    if (fd <= 0) {
        log_error("Invalid fd passed");
        return empty;
    }

    // Wait for some data from the player
    int bytes = 0;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    struct timeval tval;
    tval.tv_sec = 2;
    tval.tv_usec = 0;
    // log_debug("Waiting for data... ");
    if (select(fd+1, &fdset, NULL, NULL, &tval)) {
        // log_debug("There is data in the network");
#ifndef _WIN32
        ioctl(fd, FIONREAD, &bytes);
#else
        ioctlSocket(fd, FIONREAD, &bytes);
#endif
    }

    // No data yet
    if (bytes == 0) {
        return empty;
    }

    log_debug("There are %d bytes in the network buffer", bytes);

    std::string buf(bytes, '\0');

    int ret = ::read(fd, &buf[0], bytes);
    if (ret <= 0) {
        return empty;
    }

    if (ret < bytes) {
        buf.resize(ret);
    }

    return buf;
}


// Close the socket
bool
GnashPluginScriptObject::closePipe(int fd)
{
//     log_debug(__FUNCTION__);
    
    if (fd > 0) {
        // Send a Quit message to the player before closing the pipe.
        std::vector<std::string> args;
        std::string str = plugin::ExternalInterface::makeInvoke("Quit", args);
        writePlayer(fd, str);
    
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
    }

    return true;
}

// Create a socket so we can talk to the player.
bool
GnashPluginScriptObject::createPipe()
{
    log_debug(__PRETTY_FUNCTION__);
#if 0    
    int p2c_pipe[2];
    int c2p_pipe[2];
    int p2c_controlpipe[2];

    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, p2c_pipe);
    if (ret == -1) {
        gnash::log_error("ERROR: socketpair(p2c) failed: %s", strerror(errno));
        return;
    }
    _streamfd = p2c_pipe[1];

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, c2p_pipe);
    if (ret == -1) {
        gnash::log_error("ERROR: socketpair(c2p) failed: %s", strerror(errno));
        return;
    }

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, p2c_controlpipe);
    if (ret == -1) {
        gnash::log_error("ERROR: socketpair(control) failed: %s", strerror(errno));
        return;
    }
    _controlfd = p2c_controlpipe[1];
#endif

#if 0
    if ((_sockfds[READFD] == 0) && (_sockfds[WRITEFD] == 0)) {
        int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, _sockfds);
    
        if (ret == 0) {
            // Set up the Glib IO Channel for reading from the plugin
//            log_debug("Read fd for socketpair is: %d", _sockfds[READFD]);
            
            _iochan[READFD]  = g_io_channel_unix_new(_sockfds[READFD]);
            g_io_channel_set_close_on_unref(_iochan[READFD], true);
            _watchid = g_io_add_watch(_iochan[READFD], 
                                      (GIOCondition)(G_IO_IN|G_IO_HUP), 
                                      (GIOFunc)handleInvokeWrapper, this);
            
            // Set up the Glib IO Channel for writing to the plugin
//            log_debug("Write fd for socketpair is: %d", _sockfds[WRITEFD]);
            _iochan[WRITEFD] = g_io_channel_unix_new(_sockfds[WRITEFD]);
            g_io_channel_set_close_on_unref(_iochan[WRITEFD], true);
            return true;
        }
    }
#endif
    
#if 0
    std::stringstream ss;
    static int count = 0;
    ss << "/tmp/gnash-" << getpid() << count++;

    return createPipe(ss.str());
#endif

    return false;
}

#if 0
bool
GnashPluginScriptObject::createPipe(const std::string &name)
{
    log_debug(__PRETTY_FUNCTION__);

    mode_t mode = S_IRUSR|S_IWUSR;
    if (!name.empty()) {
        int ret = mkfifo(name.c_str(), mode);
        if (ret == 0) {
            _sockfd = ::open(name.c_str(), O_RDWR|O_NONBLOCK, mode);
            if (_sockfd < 0) {
                log_error("Couldn't open the pipe: \"%s\"", strerror(errno));
            }
        } else {
            log_error("Couldn't create fifo: s\n", strerror(errno));
        }
    }

    _pipename = name;
    
    return false;
}
#endif


// Close the socketpair
bool
GnashPluginScriptObject::closePipe()
{
//     log_debug(__FUNCTION__);

    bool ret = closePipe(_sockfds[READFD]);
    if (ret) {
        ret = closePipe(_sockfds[WRITEFD]);
    } else {
        return false;
    }

    GError *error;
    GIOStatus rstatus = g_io_channel_shutdown(_iochan[READFD], true, &error);
    GIOStatus wstatus = g_io_channel_shutdown(_iochan[WRITEFD], true, &error);
    if ((rstatus == G_IO_STATUS_NORMAL)
        && (wstatus == G_IO_STATUS_NORMAL)) {
        return true;
    } else {
        return false;
    }

    return false;
}

// Check the pipe to see if it's ready, ie... is gnash connected yet ?
bool
GnashPluginScriptObject::checkPipe()
{
    return checkPipe(_sockfds[WRITEFD]);
}

bool
GnashPluginScriptObject::checkPipe(int fd)
{
//    log_debug(__PRETTY_FUNCTION__);
    
    fd_set fdset;
        
    if (fd > 2) {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
	struct timeval tval;
        tval.tv_sec = 0;
        tval.tv_usec = 100;
        errno = 0;
        int ret = select(fd+1, &fdset, NULL, NULL, &tval);
        if (ret == 0) {
            log_debug ("The pipe for #fd %d timed out waiting to read", fd);
            return false;
        } else if (ret == 1) {
            log_debug ("The pipe for #fd is ready", fd);
            controlfd = fd;
            return true;
        } else {
            log_error("The pipe has this error: %s", strerror(errno));
        }
    }

    return false;
}

bool
GnashPluginScriptObject::handleInvoke(GIOChannel *iochan, GIOCondition cond)
{
    log_debug(__PRETTY_FUNCTION__);
    
    if ( cond & G_IO_HUP ) {
        log_debug("Player control channel hang up");
        // Returning false here will cause the "watch" to be removed. This watch
        // is the only reference held to the GIOChannel, so it will be
        // destroyed. We must make sure we don't attempt to destroy it again.
        _watchid = 0;
        return false;
    }

    assert(cond & G_IO_IN);

    log_debug("Checking player requests on fd #%d",
              g_io_channel_unix_get_fd(iochan));

    do {
        GError* error=NULL;
        gchar* request;
        gsize requestSize=0;
        GIOStatus status = g_io_channel_read_line(iochan, &request,
                &requestSize, NULL, &error);

        switch ( status ) {
          case G_IO_STATUS_ERROR:
                log_error("Error reading request line: %s", error->message);

                g_error_free(error);
                return false;
            case G_IO_STATUS_EOF:
                log_error("EOF (error: %s", error->message);
                return false;
            case G_IO_STATUS_AGAIN:
                log_error("Read again(error: %s", error->message);
                break;
            case G_IO_STATUS_NORMAL:
                // process request
                log_debug("Normal read: %s" + std::string(request));
                break;
            default:
                log_error("Abnormal status!");
                return false;
            
        }

        // process request..
        processPlayerRequest(request, requestSize);
        g_free(request);

    } while (g_io_channel_get_buffer_condition(iochan) & G_IO_IN);

    return true;
}

bool
GnashPluginScriptObject::processPlayerRequest(gchar */* buf */, gsize /* len */)
{
    log_debug(__PRETTY_FUNCTION__);

    return false;
}

bool
GnashPluginScriptObject::handleInvokeWrapper(GIOChannel *iochan,
                                             GIOCondition cond,
                                             GnashPluginScriptObject* plugin)
{
    log_debug(__PRETTY_FUNCTION__);
    
    return plugin->handleInvoke(iochan, cond);
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
