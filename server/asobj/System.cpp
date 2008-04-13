// System.cpp:  ActionScript "System" class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <sstream>

#include "movie_root.h" // interface callback
#include "log.h"
#include "System.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface
#include "URL.h" // for encoding serverString

#define TF(x) (x ? "t" : "f")

namespace gnash {

const std::string& systemLanguage();

static as_value system_security_allowdomain(const fn_call& fn);
static as_value system_security_allowinsecuredomain(const fn_call& fn);
static as_value system_security_loadpolicyfile(const fn_call& fn);
static as_value system_setclipboard(const fn_call& fn);
static as_value system_showsettings(const fn_call& fn);
static as_value system_showsettings(const fn_call& fn);

static as_object*
getSystemSecurityInterface()
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object(getObjectInterface());
		proto->init_member("allowDomain", new builtin_function(system_security_allowdomain));

		// TODO: only available when SWF >= 7 
		proto->init_member("allowInsecureDomain", new builtin_function(system_security_allowinsecuredomain));

		proto->init_member("loadPolicyFile", new builtin_function(system_security_loadpolicyfile));
	}
	return proto.get();
}

static as_object*
getSystemCapabilitiesInterface()
{
	RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    //
    // Filesystem, access, miscellaneous hardware information
    //

    // "Windows XP", "Windows 2000", "Windows NT", "Windows 98/ME",
    // "Windows 95", "Windows CE", "Linux", "MacOS"
    // Override in gnashrc
    const std::string os = VM::get().getOSName();

    const std::string language = systemLanguage();

    // FIXME: these need to be implemented properly 
    // Does the NetStream object natively support SSL?
	const bool hasTLS = false;

    // Microphone and camera access disabled
	const bool avHardwareDisable = false;
	
	// Not sure: seems to be whether the movie can 'float' above web pages,
	// and is useful for disabling certain annoying adverts.
	const bool windowlessDisable = false;

	const bool hasPrinting = true;
	const bool hasAccessibility = true;
	const bool isDebugger = false;
	const bool localFileReadDisable = false;

    //
    // Display information (needs active GUI)
    //

    // Documented to be a number, but is in fact a string.
    std::string pixelAspectRatio;

    // "StandAlone", "External", "PlugIn", "ActiveX" (get from GUI)
    std::string playerType;

    int screenResolutionX = 0;
    int screenResolutionY = 0;

    std::istringstream ss;

    if (movie_root::interfaceHandle) {
        ss.str((*movie_root::interfaceHandle)("System.capabilities.screenResolutionX", ""));
        ss >> screenResolutionX;
        
        ss.clear();
        ss.str((*movie_root::interfaceHandle)("System.capabilities.screenResolutionY", ""));
        ss >> screenResolutionY;
        
        pixelAspectRatio = (*movie_root::interfaceHandle)("System.capabilities.pixelAspectRatio", "");
        playerType = (*movie_root::interfaceHandle)("System.capabilities.playerType", "");
    }

    //
	// Media
	//
		
	// Is audio available?
	const bool hasAudio = (get_sound_handler() != NULL);

    // FIXME: these need to be implemented properly. They are mostly
    // self-explanatory.
    const bool hasAudioEncoder = true;
    const bool hasEmbeddedVideo = true;
    const bool hasIME = true;
    const bool hasMP3 = true;
    const bool hasScreenBroadcast = true;
    const bool hasScreenPlayback = true;
    const bool hasStreamingAudio = true;
    const bool hasStreamingVideo = true;
    const bool hasVideoEncoder = true;

    //
    // Player version
    //

    // "LNX 9,0,22,0", "MAC 8,0,99,0"
    // Override in gnashrc
    const std::string version = VM::get().getPlayerVersion();

    // "Macromedia Windows", "Macromedia Linux", "Macromedia MacOS"
    // Override in gnashrc
    const std::string manufacturer = rcfile.getFlashSystemManufacturer();
    
    // serverString
	// A URL-encoded string to send system info to a server.
	// Boolean values are represented as t or f.		
	// Privacy concerns should probably be addressed by 	
	// allowing this string to be sent or not; individual	
	// values that might affect privacy can be overridden	
	// in gnashrc.
	
	// hasIME seems not to be included in the server string, though
	// it is documented to have a server string of IME.
	// Linux player version 9 has no hasIME property (but no need
	// to emulate that.)
	
	// TLS and hasTLS are documented for AS3, player version 9.
	//
	// WD is included in the server string for player version 9,
	// but not documented. It corresponds to the equally undocumented
	// windowlessDisable.
	
	// This should be the standard order of parameters in the server
	// string.
	std::ostringstream serverString;
	serverString << "&A="    << TF(hasAudio)
			<< "&SA="	<< TF(hasStreamingAudio)
			<< "&SV="	<< TF(hasStreamingVideo)
			<< "&EV="	<< TF(hasEmbeddedVideo)
			<< "&MP3="	<< TF(hasMP3)						
			<< "&AE="	<< TF(hasAudioEncoder)
			<< "&VE="	<< TF(hasVideoEncoder)
			<< "&ACC="	<< TF(hasAccessibility)
			<< "&PR="	<< TF(hasPrinting)
			<< "&SP="	<< TF(hasScreenPlayback) 
			<< "&SB="	<< TF(hasScreenBroadcast) 
			<< "&DEB="	<< TF(isDebugger)
			<< "&V="    << URL::encode(version)
			<< "&M="    << URL::encode(manufacturer)
			<< "&R="    << screenResolutionX << "x" << screenResolutionY
			<< "&DP="	// screenDPI (int?
			<< "&COL="	// screenColor (?)						
			<< "&AR="   << pixelAspectRatio
			<< "OS="    << URL::encode(os)
			<< "&L="    << language			
			<< "&PT="   << playerType
			<< "&AVD="	<< TF(avHardwareDisable) 
			<< "&LFD="	<< TF(localFileReadDisable)
			<< "&WD="   << TF(windowlessDisable)
			<< "&TLS="	<< TF(hasTLS);
	
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		int flags  = as_prop_flags::dontDelete;
		    flags |= as_prop_flags::dontEnum;
		    flags |= as_prop_flags::readOnly;

		proto = new as_object(getObjectInterface());

		proto->init_member("version", version, flags);
		proto->init_member("playerType", playerType, flags);
		proto->init_member("os", os, flags);
		proto->init_member("manufacturer", manufacturer, flags);
		proto->init_member("language", language, flags);
		proto->init_member("hasAudio", hasAudio, flags);
		proto->init_member("screenResolutionX", screenResolutionX, flags);
		proto->init_member("screenResolutionY", screenResolutionY, flags);
		proto->init_member("pixelAspectRatio", pixelAspectRatio, flags);
		proto->init_member("serverString", serverString.str(), flags);
		proto->init_member("avHardwareDisable", avHardwareDisable, flags);
		proto->init_member("hasAudioEncoder", hasAudioEncoder, flags);
		proto->init_member("hasEmbeddedVideo", hasEmbeddedVideo, flags);
		proto->init_member("hasIME", hasIME, flags);
		proto->init_member("hasMP3", hasMP3, flags);
		proto->init_member("hasPrinting", hasPrinting, flags);
		proto->init_member("hasScreenBroadcast", hasScreenBroadcast, flags);
		proto->init_member("hasScreenPlayback", hasScreenPlayback, flags);
		proto->init_member("hasStreamingAudio", hasStreamingAudio, flags);
		proto->init_member("hasStreamingVideo", hasStreamingVideo, flags);
		proto->init_member("hasVideoEncoder", hasVideoEncoder, flags);
		proto->init_member("hasAccessibility", hasAccessibility, flags);
		proto->init_member("isDebugger", isDebugger, flags);
		proto->init_member("localFileReadDisable", localFileReadDisable, flags);
		proto->init_member("hasTLS", hasTLS, flags);
		proto->init_member("windowlessDisable", windowlessDisable, flags);
	}
	return proto.get();
}

static void
attachSystemInterface(as_object& proto)
{
	// Initialize Function prototype
	proto.init_member("security", getSystemSecurityInterface());
	proto.init_member("capabilities", getSystemCapabilitiesInterface());
	proto.init_member("setClipboard", new builtin_function(system_setclipboard));
	proto.init_member("showSettings", new builtin_function(system_showsettings));
}

static as_object*
getSystemInterface()
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object(getObjectInterface());
		attachSystemInterface(*proto);
	}
	return proto.get();
}

system_as_object::system_as_object()
	:
	as_object(getSystemInterface()) // pass System inheritence
{
}

as_value system_security_allowdomain(const fn_call& /*fn*/) {
    log_unimpl (__PRETTY_FUNCTION__);
    return as_value();
}

as_value system_security_allowinsecuredomain(const fn_call& /*fn*/) {
    log_unimpl (__PRETTY_FUNCTION__);
    return as_value();
}

as_value system_security_loadpolicyfile(const fn_call& /*fn*/) {
    log_unimpl (__PRETTY_FUNCTION__);
    return as_value();
}

as_value system_setclipboard(const fn_call& /*fn*/) {
    log_unimpl (__PRETTY_FUNCTION__);
    return as_value();
}

as_value system_showsettings(const fn_call& /*fn*/) {
    log_unimpl (__PRETTY_FUNCTION__);
    return as_value();
}

void
system_class_init(as_object& global)
{
	// _global.System is NOT a class, but a simple object, see System.as

	static boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
	attachSystemInterface(*obj);
	global.init_member("System", obj.get());
}

const std::string& systemLanguage()
{
	// Two-letter language code ('en', 'de') corresponding to ISO 639-1
	// Chinese can be either zh-CN or zh-TW. English used to have a 
	// country (GB, US) qualifier, but that was dropped in version 7 of the player.
 	// This method relies on getting a POSIX-style language code of the form
	// "zh_TW.utf8", "zh_CN" or "it" from the VM.
	// It is obviously very easy to extend support to all language codes, but
	// some scripts rely on there being only 20 possible languages. It could
	// be a run time option if it's important enough to care.

	static std::string lang = VM::get().getSystemLanguage();
	
	const char* languages[] = {"en", "fr", "ko", "ja", "sv",
				"de", "es", "it", "zh", "pt",
				"pl", "hu", "cs", "tr", "fi",
				"da", "nl", "no", "ru"};
	
	const unsigned int size = sizeof (languages) / sizeof (*languages);
	
	if (std::find(languages, languages + size, lang.substr(0,2)) != languages + size)
	{
		if (lang.substr(0,2) == "zh")
		{
			// Chinese is the only language since the pp version 7
			// to need an additional qualifier.
			if (lang.substr(2, 3) == "_TW") lang = "zh-TW";
			else if (lang.substr(2, 3) == "_CN") lang = "zh-CN";
			else lang = "xu";
		}
		else
		{
			// All other matching cases: retain just the first
			// two characters.
			lang.erase(2);
		}
	}
	else
	{
		// Unknown language. We also return this if
		// getSystemLanguage() returns something unexpected. 
		lang = "xu";
	}

	return lang;

}

} // end of gnash namespace
