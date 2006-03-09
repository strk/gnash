// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//
//

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"

namespace gnash {
  
class System {
public:
    System();
    ~System();
   void security_allowDomain();
   void security_allowInsecureDomain();
   void security_loadPolicyFile();
   void setClipboard();
   void showSettings();
private:
    bool _capabilities;
    bool _object;
    bool _capabilities_avHardwareDisable;
    bool _capabilities_hasAccessibility;
    bool _capabilities_hasAudio;
    bool _capabilities_hasAudioEncoder;
    bool _capabilities_hasEmbeddedVideo;
    bool _capabilities_hasMP3;
    bool _capabilities_hasPrinting;
    bool _capabilities_hasScreenBroadcast;
    bool _capabilities_hasScreenPlayback;
    bool _capabilities_hasStreamingAudio;
    bool _capabilities_hasStreamingVideo;
    bool _capabilities_hasVideoEncoder;
    bool _capabilities_isDebugger;
    bool _capabilities_language;
    bool _capabilities_localFileReadDisable;
    bool _capabilities_manufacturer;
    bool _capabilities_os;
    bool _capabilities_pixelAspectRatio;
    bool _capabilities_playerType;
    bool _capabilities_screenColor;
    bool _capabilities_screenDPI;
    bool _capabilities_screenResolutionX;
    bool _capabilities_screenResolutionY;
    bool _capabilities_serverString;
    bool _capabilities_version;
    bool _security;
    bool _exactSettings;
    bool _onStatus;
    bool _useCodepage;
};

struct system_as_object : public as_object
{
    System obj;
};

void system_new(const fn_call& fn);
void system_security_allowdomain(const fn_call& fn);
void system_security_allowinsecuredomain(const fn_call& fn);
void system_security_loadpolicyfile(const fn_call& fn);
void system_setclipboard(const fn_call& fn);
void system_showsettings(const fn_call& fn);

} // end of gnash namespace

// __SYSTEM_H__
#endif

