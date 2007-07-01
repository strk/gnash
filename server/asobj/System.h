// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//
//

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_object.h" // for inheritance
#include "fn_call.h"

namespace gnash {
  
#if 0
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
#endif

class system_as_object : public as_object
{
    //System obj;
public:
	system_as_object();
};

as_value system_new(const fn_call& fn);
as_value system_security_allowdomain(const fn_call& fn);
as_value system_security_allowinsecuredomain(const fn_call& fn);
as_value system_security_loadpolicyfile(const fn_call& fn);
as_value system_setclipboard(const fn_call& fn);
as_value system_showsettings(const fn_call& fn);

void system_class_init(as_object& global);

} // end of gnash namespace

// __SYSTEM_H__
#endif

