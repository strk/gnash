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
#include <cstring>
#include <cstdlib>
#include "npapi.h"
#include "npruntime.h"
#include "plugin.h" 
#include "pluginScriptObject.h"

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

// Callbacks for the default methods

// As these callbacks use a generalized typedef for the signature, often some
// of the parameters can be ignored. These are commented out to elimnate the
// volumnes of bogus warnings about not using them in the method.

// SetVariable( Name, Value )
// Sends:
//      "Command Name Type value\n",
//      	ie... "SetVariable var1 string value1\n"
// Receives:
//      nothing
bool
SetVariableCallback (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    std::string varname;
    if (argCount == 2) {
        varname = NPVARIANT_TO_STRING(args[0]).UTF8Characters;
        NPVariant *value = const_cast<NPVariant *>(&args[1]);
        // log_debug("Setting Variable \"%s\"", varname);
        gpso->SetVariable(varname, value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// GetVariable( Name )
//    Sends:
// 	"Command Name\n", ie... "GetVariable var1\n"
//
//    Receives:
// 	"Command Name Type value", ie... "GetVariable var1 string value1\n"
bool
GetVariableCallback (NPObject *npobj, NPIdentifier /* name */,
                     const NPVariant *args,
                     uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    NPVariant *value = 0;
    std::string varname;
    // This method only takes one argument
    if (argCount == 1) {
        varname = NPVARIANT_TO_STRING(args[0]).UTF8Characters;
        value = gpso->GetVariable(varname);
        if (value == 0) {
            NPVARIANT_IS_NULL(*result);
        } else {
            if (NPVARIANT_IS_DOUBLE(*value)) {
                double num = NPVARIANT_TO_DOUBLE(*value);
                DOUBLE_TO_NPVARIANT(num, *result);
            } else if (NPVARIANT_IS_STRING(*value)) {
                STRINGN_TO_NPVARIANT(NPVARIANT_TO_STRING(*value).UTF8Characters,
                                     NPVARIANT_TO_STRING(*value).UTF8Length,
                                     *result);
            } else if (NPVARIANT_IS_BOOLEAN(*value)) {
                BOOLEAN_TO_NPVARIANT(NPVARIANT_TO_BOOLEAN(*value), *result);
            } else if (NPVARIANT_IS_INT32(*value)) {
                INT32_TO_NPVARIANT(NPVARIANT_TO_INT32(*value), *result);
            } else if (NPVARIANT_IS_NULL(*value)) {
                NULL_TO_NPVARIANT(*result);
            } else if (NPVARIANT_IS_VOID(*value)) {
                VOID_TO_NPVARIANT(*result);
            } else if (NPVARIANT_IS_OBJECT(*value)) {
                OBJECT_TO_NPVARIANT(NPVARIANT_TO_OBJECT(*value), *result);
            }
            NPN_MemFree(value);
            return true;
        }
    }
    
    NPN_MemFree(value);
    NPVARIANT_IS_NULL(*result);
    return false;
}

// GotoFrame( frameNumber )
//    Sends:
// 	"Command number\n", ie... "GotoFrame 77\n"
//
//    Receives:
// 	nothing
bool
GotoFrame (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    std::string varname;
    if (argCount == 1) {
        int value = NPVARIANT_TO_INT32(args[0]);
        std::stringstream ss;
        ss << "GotoFrame " << value << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't goto the specified frame, network problems.");
            return false;
        }        
        // gpso->GotoFrame(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// IsPlaying()
//    Sends:
// 	"Command\n", ie... "IsPlaying\n"

//    Receives:
// 	"Command Flag", ie... "IsPlaying true\n"
bool
IsPlaying (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        std::stringstream ss;
        ss << "IsPlaying" << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't check if the movie is playing, network problems.");
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }        
        const char *data = 0;
        char *ptr = 0;
        ret = gpso->readPlayer(gpso->getControlFD(), &data, 0);
        if (ret == 0) {
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }
        ptr = const_cast<char *>(data);
        if (strncmp(ptr, "IsPlaying ", 10) != 0) {
            log_error("Illegal response! %s", ptr);
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        } else {
            // A legit response has CR on the end already
            log_debug("Legit response: %s", ptr);
        }    
        ptr += 10;
        bool flag = false;
        if (strncmp(ptr, "true", 4) == 0) {
            flag = true;
        } else if (strncmp(ptr, "false", 5) == 0) {
            flag = false;
        }
        BOOLEAN_TO_NPVARIANT(flag, *result);
        // gpso->IsPlaying(value);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// LoadMovie( Layer, Url )
//    Sends:
// 	"Command Layer Url\n", ie... "LoadMovie 1 /foo.swf\n"
//
//    Receives:
// 	nothing
bool
LoadMovie (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 2) {
        std::stringstream ss;
        int layer = NPVARIANT_TO_INT32(args[0]);
        std::string url = NPVARIANT_TO_STRING(args[1]).UTF8Characters;
        ss << "LoadMovie " << layer << " " << url << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't load the movie, network problems.");
            return false;
        }        
        // gpso->LoadMovie();
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// Pan ( x, y, mode ) 
//    Sends:
// 	"Command Pan x y mode\n", ie... "Pan 10 10 pixels\n"
//
//    Receives:
//    	nothing
bool
Pan (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 3) {
        std::stringstream ss;
        NPVariant *value = const_cast<NPVariant *>(&args[0]);
        int x = NPVARIANT_TO_INT32(*value);
        value = const_cast<NPVariant *>(&args[1]);
        int y = NPVARIANT_TO_INT32(*value);
        value = const_cast<NPVariant *>(&args[2]);
        int mode = NPVARIANT_TO_INT32(*value);
        ss << "Pan " << x << y;
        if (mode) {
            ss << " pixels";
        } else {
            ss << " percent";
        }
        ss << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't pan the movie, network problems.");
            return false;
        }        
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// PercentLoaded()
//    Sends:
// 	"Command\n", ie... "PercentLoaded\n"
//
//    Receives:
// 	"Command number\n", ie... "PercentLoaded 23\n"
bool
PercentLoaded (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
//    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

#if 1
    static int counter = 0;
//    log_error("%s: %d ; %d\n", __FUNCTION__, gpso->getControlFD(), counter);
    INT32_TO_NPVARIANT(counter, *result);
    if (counter >= 100) {
        counter = 0;
    } else {
        counter += 20;
    }
    return true;
#else
    if (argCount == 0) {
        std::stringstream ss;
        ss << "PercentLoaded" << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't check percent loaded, network problems.");
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }        
        const char *data = 0;
        char *ptr = 0;
        ret = gpso->readPlayer(gpso->getControlFD(), &data, 0);
        if (ret == 0) {
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }
        ptr = const_cast<char *>(data);
        if (strncmp(ptr, "PercentLoaded ", 15) != 0) {
            log_error("Illegal response! %s\n", ptr);
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        } else {
            // A legit response has CR on the end already
            log_debug("Legit response: %s", ptr);
        }    
        ptr += 15;
        int percent = strtol(ptr, NULL, 0);
        if ((percent >= 0) && (percent <= 100)) {
            INT32_TO_NPVARIANT(percent, *result);
        } else {
            INT32_TO_NPVARIANT(-1, *result);
        }
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
#endif
}

// Play();
//    Sends:
// 	"Command\n", ie... "Play\n"
//
//    Receives:
// 	nothing
bool
Play (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        std::stringstream ss;
        ss << "Play" << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't play movie, network problems.");
            return false;
        }        
        // gpso->IsPlaying(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// Rewind();
//    Sends:
// 	"Command\n", ie... "Rewind\n"
//
//    Receives:
// 	nothing
bool
Rewind (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        std::stringstream ss;
        ss << "Rewind" << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't rewind movie, network problems.");
            return false;
        }        
        // gpso->Rewind(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// SetZoomRect ( left, top, right, bottom )
//    Sends:
// 	"Command left top right bottom\n", ie... "SetZoomRect 0 0 10 10\n"
//
//    Receives:
// 	nothing
bool
SetZoomRect (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 4) {
        std::stringstream ss;
        NPVariant *value = const_cast<NPVariant *>(&args[0]);
        int left = NPVARIANT_TO_INT32(*value);
        value = const_cast<NPVariant *>(&args[1]);
        int top = NPVARIANT_TO_INT32(*value);
        value = const_cast<NPVariant *>(&args[2]);
        int right = NPVARIANT_TO_INT32(*value);
        value = const_cast<NPVariant *>(&args[3]);
        int bottom = NPVARIANT_TO_INT32(*value);
        ss << "SetZoomRect " << left << " " << top << " ";
        ss << right << " " << bottom << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't Set the Zoom Rect the movie, network problems.");
            return false;
        }        
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// StopPlay()
//    Sends:
// 	"Command StopPlay\n", ie... "StopPlay\n"

//    Receives:
// 	nothing
bool
StopPlay (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        std::stringstream ss;
        ss << "StopPlay" << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't stop-play movie, network problems.");
            return false;
        }        
        // gpso->IsPlaying(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// Zoom( percent )
//    Sends:
// 	"Command number\n", ie... "Zoom 200\n"
//
//    Receives:
// 	nothing
bool
Zoom (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 1) {
        std::stringstream ss;
        NPVariant *value = const_cast<NPVariant *>(&args[1]);
        int zoom = NPVARIANT_TO_INT32(*value);
        ss << "Zoom " << zoom << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't zoom movie, network problems.");
            return false;
        }        
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// TotalFrames()
//    Sends:
// 	"Command TotalFrames\n", ie... "TotalFrames\n"
//
//    Receives:
// 	"Command Num\n", ie... "TotalFrames 1234\n"
bool
TotalFrames (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        std::stringstream ss;
        ss << "TotalFrames" << std::endl;
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), ss.str());
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != ss.str().size()) {
            log_error("Couldn't check percent loaded, network problems.");
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }        
        const char *data = 0;
        char *ptr = 0;
        ret = gpso->readPlayer(gpso->getControlFD(), &data, 0);
        if (ret == 0) {
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }
        ptr = const_cast<char *>(data);
        if (strncmp(ptr, "TotalFrames ", 13) != 0) {
            log_error("Illegal response! %s\n", ptr);
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        } else {
            // A legit response has CR on the end already
            log_debug("Legit response: %s", ptr);
        }    
        ptr += 13;
        int frames = strtol(ptr, NULL, 0);
        INT32_TO_NPVARIANT(frames, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
