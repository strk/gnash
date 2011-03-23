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

#ifndef GNASH_PLUGIN_SCRIPT_OBJECT_H
#define GNASH_PLUGIN_SCRIPT_OBJECT_H

#include <map>
#include <string>

#include <glib.h>
#include "npapi.h"
#include "npruntime.h"
#include "GnashNPVariant.h"

#define READFD 0
#define WRITEFD 1

namespace gnash {

/// Makes a deep copy of a NPVariant.
/// @param from The source NPVariant to copy values from.
/// @param to The destination NPVariant.
void
CopyVariantValue(const NPVariant& from, NPVariant& to);

class GnashPluginScriptObject : public NPObject
{
public:

    GnashPluginScriptObject();
    GnashPluginScriptObject(NPP npp);
    ~GnashPluginScriptObject();
    
    static NPClass *marshalGetNPClass();

    // NPObject static Functions. These get used by the browser, which is
    // why they have to be static.
    static NPObject *marshalAllocate (NPP npp, NPClass *aClass);
    static void marshalDeallocate (NPObject *npobj);
    static void marshalInvalidate (NPObject *npobj);
    static bool marshalHasMethod (NPObject *npobj, NPIdentifier name);
    static bool marshalInvoke (NPObject *npobj, NPIdentifier name,
                               const NPVariant *args, uint32_t argCount,
                               NPVariant *result);
    static bool marshalInvokeDefault (NPObject *npobj, const NPVariant *args,
                                      uint32_t argCount, NPVariant *result);
    static bool marshalHasProperty (NPObject *npobj, NPIdentifier name);
    static bool marshalGetProperty (NPObject *npobj, NPIdentifier name,
                                    NPVariant *result);
    static bool marshalSetProperty (NPObject *npobj, NPIdentifier name,
                                    const NPVariant *value);
    static bool marshalRemoveProperty (NPObject *npobj, NPIdentifier name);
    static bool marshalEnumerate (NPObject *npobj, void***identifier,
                                  uint32_t *count);
    static bool marshalConstruct (NPObject *npobj, const NPVariant *data,
                                  uint32_t count, NPVariant *result);
    
    static NPClass _npclass;

    /// Scripting API support. This is where all the protocol support
    /// lives.

    /// The ControlFD is the file descriptor for the socket connection
    /// to the standalone player. This is used when writing to the
    /// standalone player from this plugin.
    void setControlFD(int x);
    int getControlFD();

    /// The HostFD is the file descriptor for the socket connection
    /// to the standalone player. This is used by this plugin when reading
    /// messages from the standalone player.
    void setHostFD(int x);
    int getHostFD();

    /// Set a variable in the standalone player
    ///
    /// @param name the name of the variable to set
    ///
    /// @param value the value to set the variable to
    ///
    /// @return true or false based on the status of the invoke call
    bool SetVariable(const std::string &name, const NPVariant& value);
    
    /// Get the value of a variable from the standalone player
    ///
    /// @param name the name of the variable to set
    ///
    /// @return the value as returned by the standalone player
    GnashNPVariant GetVariable(const std::string &name);


    int getReadFD()  { return _hostfd; };
    int getWriteFD() { return _controlfd; };

    // Write to the standalone player over the control socket
    int writePlayer(const std::string &data);
    int writePlayer(int fd, const std::string &data);
    
    // Read the standalone player over the control socket
    std::string readPlayer();
    std::string readPlayer(int fd);
    
    bool Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args,
                uint32_t argCount, NPVariant *result);
    bool AddMethod(NPIdentifier name, NPInvokeFunctionPtr func);
    void AddProperty(const std::string &name, const std::string &str);
    void AddProperty(const std::string &name, double num);
    void AddProperty(const std::string &name, int num);

protected:
    // Internal functions for the API
    void Deallocate();
    void Invalidate();
    bool HasMethod(NPIdentifier name);

    bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                       NPVariant *result);
    bool HasProperty(NPIdentifier name);
    bool GetProperty(NPIdentifier name, NPVariant *result);
    bool SetProperty(NPIdentifier name, const NPVariant& value);
    bool RemoveProperty(NPIdentifier name);
    bool Enumerate(NPIdentifier **identifier, uint32_t *count);
    bool Construct(const NPVariant *data, uint32_t argCount, NPVariant *result);

private:
    void initializeIdentifiers();
    void setInstance(NPP inst) { _nppinstance = inst; };
    
    // _nppinstance->pdata should be the nsPluginInstance once NPP_New() is finished.
    NPP _nppinstance;
    
    std::map<NPIdentifier, GnashNPVariant> _properties;
    std::map<NPIdentifier, NPInvokeFunctionPtr> _methods;
    // 0 for reading, 1 for writing

    /// The ControlFD is the file descriptor for the socket connection
    /// to the standalone player. This is used when writing to the
    /// standalone player from this plugin.
    int _controlfd;
    /// The HostFD is the file descriptor for the socket connection
    /// to the standalone player. This is used by this plugin when reading
    /// messages from the standalone player.
    int _hostfd;
};

} // end of gnash namespace

#endif // GNASH_PLUGIN_SCRIPT_OBJECT_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
