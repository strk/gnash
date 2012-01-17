// 
//   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc
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

#ifndef GNASH_PLUGIN_CALLBACKS_H
#define GNASH_PLUGIN_CALLBACKS_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "npapi.h"
#include "npruntime.h"

//    Support the methods listed in
//    http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_03.html

/// \page XMLAPI ExternalInterface XML API
/// \section whatis Whatis the External Interface
/// a swf file being played in the flash player has an interactive
/// protocol so the player and the browser can communicate. This is
/// used as an XML-RPC operation, the browser often making requests to
/// the player for information about it's status, or retrieving the
/// values of a variable like "$version".
///
/// A plugin that supports this interface is called a Scriptable
/// Plugin. There is luckily a short defined list of these remote
/// methods that can be invoked.
///
/// \section proto callback prototypes
/// As these callbacks use a generalized typedef for the signature, often some
/// of the parameters can be ignored. These are commented out in the function
/// definition to elimnate the volumnes of bogus warnings about not using them
/// in the method.
///
/// \section codes return codes
/// Although all these callbacks return a boolean value, this is not used by
/// the API at all, just internally to the plugin. All return values are
/// encoded into the "result" parameter. Many of the callbacks don't have a
/// result at all, which is good as the flash player blocks after sending a
/// request to the player while waiting for a response. So not having to wait
/// can be a good thing to avoid issues.

namespace gnash {

/// Set a variable in the flash player
///
/// @param npobj the NPObject to act on
/// @param name the name of the variable to get
/// @param args ignore for this method
/// @param argCount the count of arguments, always 1
/// @param result the value of the variable as returned by the standalone player.
///
/// @example "GetVariable method invoke"
///
/// <pre>
/// <invoke name="GetVariable" returntype="xml">
///        <arguments>
///              <string>var1</string>
///        </arguments>
/// </invoke>
/// </pre>
///
/// @return nothing
extern bool GetVariableCallback (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// Set a variable in the flash player
///
/// @param npobj the NPObject to act on
/// @param name the name of the variable to set
/// @param args the value of the variable
/// @param argCount the count of arguments, always 2
/// @param result not set for this method
///
/// @example "SetVariable method invoke"
///
/// <pre>
/// <invoke name="SetVariable" returntype="xml">
///        <arguments>
///              <string>var1</string>
///              <string>value1</string>
///        </arguments>
/// </invoke>
/// </pre>
///
/// @return nothing
extern bool SetVariableCallback (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// Goto the specified frame in the swf movie being played
///
/// @param npobj the NPObject to act on
/// @param name ignored for this method
/// @param args the frame number
/// @param argCount the count of arguments, always 1
/// @param result not set for this method
///
/// @example "GotoFrame method invoke"
///
/// <pre>
/// <invoke name="GotoFrame" returntype="xml">
///      <arguments>
///              <number>123</number>
///      </arguments>
/// </invoke>
/// </pre>
extern bool GotoFrame (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// Check the standalone player to see if it's playing.
///
/// @param npobj the NPObject to act on
/// @param name ignored for this method
/// @param args ignored for this method
/// @param argCount the count of arguments, always 0
/// @param result a boolean fron the standalone player
///
/// @example "IsPlaying method invoke"
///
/// <pre>
/// <invoke name="IsPlaying" returntype="xml">
///      <arguments></arguments>
/// </invoke>
/// </pre>
extern bool IsPlaying (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// Load a swf movie into the standalone player
///
/// @param npobj the NPObject to act on
/// @param name ignored for this method
/// @param args the name of the file
/// @param argCount the count of arguments, always 1
/// @param result a boolean fron the standalone player
///
/// @example "LoadMovie method invoke"
///
/// <pre>
/// <invoke name="LoadMovie" returntype="xml">
///      <arguments>
///              <number>2</number>
///              <string>/tmp/bogus.swf</string>
///      </arguments>
/// </invoke>
/// </pre>
extern bool LoadMovie (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// Pan the movie being played
///
/// @param npobj the NPObject to act on
/// @param name ignored for this method
/// @param args[0] The X coordinate 
/// @param args[1] The Y coordinate 
/// @param args[2] The mode, 0 for pixels, 1 for percent
/// @param argCount the count of arguments, always 3
/// @param result no result from the player
///
/// @example "Pan method invoke"
///
/// <pre>
/// <invoke name="Pan" returntype="xml">
///      <arguments>
///              <number>1</number>
///              <number>2</number>
///              <number>0</number>
///      </arguments>
/// </invoke>
/// </pre>
extern bool Pan (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// Get the percentage of the movie that's been played so far.
///
/// @param npobj the NPObject to act on
/// @param name ignored for this method
/// @param args ignored for this method
/// @param argCount the count of arguments, always 0
/// @param result an integer fron the standalone player
///
/// @example "PercentLoaded method invoke"
///
/// <pre>
/// <invoke name="PercentLoaded" returntype="xml">
///      <arguments></arguments>
/// </invoke>
///
/// <number>33</number>
/// </pre>
extern bool PercentLoaded (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// @example "Play invoke method"
///
/// <pre>
/// <invoke name="Play" returntype="xml">
///      <arguments></arguments>
/// </invoke>
/// </pre>
extern bool Play (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// @example "Rewind invoke method"
///
/// <pre>
/// <invoke name="Rewind" returntype="xml">
///      <arguments></arguments>
/// </invoke>
/// </pre>
extern bool Rewind (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// @example "SetZoomRect invoke method"
///
/// <pre>
/// <invoke name="SetZoomRect" returntype="xml">
///      <arguments>
///              <number>1</number>
///              <number>2</number>
///              <number>3</number>
///              <number>4</number>
///      </arguments>
/// </invoke>
/// </pre>
extern bool SetZoomRect (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// @example "StopPlay invoke method"
///
/// <pre>
/// <invoke name="StopPlay" returntype="xml">
///      <arguments></arguments>
/// </invoke>
/// </pre>
extern bool StopPlay (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// @example "Zoom invoke method"
///
/// <pre>
/// <invoke name="Zoom" returntype="xml">
///      <arguments>
///              <number>12</number>
///      </arguments>
/// </invoke>
/// </pre>
extern bool Zoom (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

/// @example "TotalFrames invoke method"
///
/// <pre>
/// <invoke name="TotalFrames" returntype="xml">
///      <arguments></arguments>
/// </invoke>
///
/// <number>66</number>
/// </pre>
extern bool TotalFrames (NPObject *npobj, NPIdentifier name,
                          const NPVariant *args, uint32_t argCount,
                          NPVariant *result);

extern bool remoteCallback (NPObject *npobj, NPIdentifier name,
                            const NPVariant *args, uint32_t argCount,
                            NPVariant *result);

#endif // GNASH_PLUGIN_CALLBACKS_H

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
