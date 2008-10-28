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

#include <sstream>

#include "movie_root.h" // interface callback
#include "log.h"
#include "System_as.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface

inline std::string
trueFalse(bool x)
{
    return x ? "t" : "f";
}

namespace gnash {

static const std::string& systemLanguage(as_object& proto);

static as_value system_security_allowdomain(const fn_call& fn);
static as_value system_security_allowinsecuredomain(const fn_call& fn);
static as_value system_security_loadpolicyfile(const fn_call& fn);
static as_value system_setclipboard(const fn_call& fn);
static as_value system_showsettings(const fn_call& fn);
static as_value system_exactsettings(const fn_call& fn);
static as_value system_usecodepage(const fn_call& fn);

void registerSystemNative(as_object& global)
{
    VM& vm = global.getVM();
    
    vm.registerNative(system_security_allowdomain, 12, 0);
    vm.registerNative(system_showsettings, 2107, 0);
    
    // From http://osflash.org/flashcoders/undocumented/asnative
    
    // Run once in startup script then deleted...
    // System.Capabilities.Query 11, 0    
    
    // System.Product.isRunning 2201, 0
    // System.Product.isInstalled 2201, 1
    // System.Product.launch 2201, 2
    // System.Product.download 2201, 3    
}

static as_object*
getSystemSecurityInterface(as_object& o)
{
    VM& vm = o.getVM();

	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object(getObjectInterface());
		proto->init_member("allowDomain", vm.getNative(12, 0));

		// TODO: only available when SWF >= 7 
		proto->init_member("allowInsecureDomain", new builtin_function(system_security_allowinsecuredomain));

		proto->init_member("loadPolicyFile", new builtin_function(system_security_loadpolicyfile));
	}
	return proto.get();
}

static as_object*
getSystemCapabilitiesInterface(as_object& o)
{
	RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    //
    // Filesystem, access, miscellaneous hardware information
    //

    // "Windows XP", "Windows 2000", "Windows NT", "Windows 98/ME",
    // "Windows 95", "Windows CE", "Linux", "MacOS"
    // Override in gnashrc
    VM& vm = o.getVM();
    
    const std::string os = vm.getOSName();

    const std::string language = systemLanguage(o);

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
    
    std::string screenColor;
    
    int screenDPI = 0;

    int screenResolutionX = 0;
    int screenResolutionY = 0;

    std::istringstream ss;

    const movie_root& m = vm.getRoot();

    ss.str(m.callInterface("System.capabilities.screenResolutionX", ""));
    ss >> screenResolutionX;
        
    ss.clear();
    ss.str(m.callInterface("System.capabilities.screenResolutionY", ""));
    ss >> screenResolutionY;

    ss.clear();
    ss.str(m.callInterface("System.capabilities.screenDPI", ""));
    ss >> screenDPI;
        
    pixelAspectRatio = m.callInterface("System.capabilities.pixelAspectRatio", "");
    playerType = m.callInterface("System.capabilities.playerType", "");
    screenColor = m.callInterface("System.capabilities.screenColor", "");

    //
    // Media
    //
        
    // Is audio available?
    const bool hasAudio = (vm.getRoot().runInfo().soundHandler() != NULL);

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
    const std::string version = vm.getPlayerVersion();

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
	serverString << "A="    << trueFalse(hasAudio)
			<< "&SA="	<< trueFalse(hasStreamingAudio)
			<< "&SV="	<< trueFalse(hasStreamingVideo)
			<< "&EV="	<< trueFalse(hasEmbeddedVideo)
			<< "&MP3="	<< trueFalse(hasMP3)						
			<< "&AE="	<< trueFalse(hasAudioEncoder)
			<< "&VE="	<< trueFalse(hasVideoEncoder)
			<< "&ACC="	<< trueFalse(hasAccessibility)
			<< "&PR="	<< trueFalse(hasPrinting)
			<< "&SP="	<< trueFalse(hasScreenPlayback) 
			<< "&SB="	<< trueFalse(hasScreenBroadcast) 
			<< "&DEB="	<< trueFalse(isDebugger)
			<< "&V="    << URL::encode(version)
			<< "&M="    << URL::encode(manufacturer)
			<< "&R="    << screenResolutionX << "x" << screenResolutionY
			<< "&DP="	<< screenDPI
			<< "&COL="	<< screenColor					
			<< "&AR="   << pixelAspectRatio
			<< "&OS="   << URL::encode(os)
			<< "&L="    << language			
			<< "&PT="   << playerType
			<< "&AVD="	<< trueFalse(avHardwareDisable) 
			<< "&LFD="	<< trueFalse(localFileReadDisable)
			<< "&WD="   << trueFalse(windowlessDisable)
			<< "&TLS="	<< trueFalse(hasTLS);
	
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		const int flags = as_prop_flags::dontDelete
		                | as_prop_flags::dontEnum
		                | as_prop_flags::readOnly;

		proto = new as_object(getObjectInterface());

		proto->init_member("version", version, flags);
		proto->init_member("playerType", playerType, flags);
		proto->init_member("os", os, flags);
		proto->init_member("manufacturer", manufacturer, flags);
		proto->init_member("language", language, flags);
		proto->init_member("hasAudio", hasAudio, flags);
		proto->init_member("screenResolutionX", screenResolutionX, flags);
		proto->init_member("screenResolutionY", screenResolutionY, flags);
		proto->init_member("screenColor", screenColor, flags);
		proto->init_member("screenDPI", screenDPI, flags);
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
	VM& vm = proto.getVM();
    const int version = vm.getSWFVersion();

	proto.init_member("security", getSystemSecurityInterface(proto));
	proto.init_member("capabilities", getSystemCapabilitiesInterface(proto));
	proto.init_member("setClipboard", new builtin_function(system_setclipboard));
	proto.init_member("showSettings", vm.getNative(2107, 0));

	proto.init_property("useCodepage", &system_usecodepage, &system_usecodepage);

    if (version < 6) return;

    proto.init_property("exactSettings", &system_exactsettings, &system_exactsettings);

}


as_value
system_security_allowdomain(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl ("System.security.allowDomain") );
    return as_value();
}


as_value
system_security_allowinsecuredomain(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl ("System.security.allowInsecureDomain") );
    return as_value();
}


as_value
system_security_loadpolicyfile(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl ("System.security.loadPolicyFile") );
    return as_value();
}


as_value
system_setclipboard(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl ("System.setClipboard") );
    return as_value();
}


as_value
system_showsettings(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl ("System.showSettings") );
    return as_value();
}


// FIXME: should return true if shared object files
// are stored under an exact domain name (www.gnashdev.org or
// gnashdev.org); false if both are stored under gnashdev.org.
// Can be set.
as_value
system_exactsettings(const fn_call& fn)
{
	static boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    // Getter
    if (fn.nargs == 0)
    {
        // Is always true until we implement it.
        return as_value(true);   
    }
    
    // Setter
    else 
    {
        LOG_ONCE(log_unimpl ("System.exactSettings") );
        return as_value();
    }
}


// FIXME: if true, SWF6+ should treat characters as Latin
// charset variants. If false (default), as UtrueFalse-8.
// Can be set.
as_value
system_usecodepage(const fn_call& fn)
{
	static boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    // Getter
    if (fn.nargs == 0)
    {
        // Is always false until we implement it.
        return as_value(false);   
    }
    
    // Setter
    else 
    {
        LOG_ONCE(log_unimpl ("System.useCodepage") );
        return as_value();
    }
}


void
system_class_init(as_object& global)
{
	// _global.System is NOT a class, but a simple object, see System.as

	static boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
	attachSystemInterface(*obj);
	global.init_member("System", obj.get());
}


const std::string&
systemLanguage(as_object& proto)
{
	// Two-letter language code ('en', 'de') corresponding to ISO 639-1
	// Chinese can be either zh-CN or zh-TW. English used to have a 
	// country (GB, US) qualifier, but that was dropped in version 7 of the player.
 	// This method relies on getting a POSIX-style language code of the form
	// "zh_TW.utf8", "zh_CN" or "it" from the VM.
	// It is obviously very easy to extend support to all language codes, but
	// some scripts rely on there being only 20 possible languages. It could
	// be a run time option if it's important enough to care.

	static std::string lang = proto.getVM().getSystemLanguage();
	
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
