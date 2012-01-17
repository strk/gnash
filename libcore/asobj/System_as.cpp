// System.cpp:  ActionScript "System" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "System_as.h"

#include <sstream>
#include <string>
#include <iomanip>
#include <boost/tuple/tuple.hpp>

#include "movie_root.h" // interface callback
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "VM.h" // for getPlayerVersion() 
#include "GnashAlgorithm.h"
#include "RunResources.h"
#include "HostInterface.h"

namespace gnash {


// Forward declarations.
namespace {

    inline char trueFalse(bool x) { return x ? 't' : 'f'; }

    std::string systemLanguage(as_object& proto);

    as_value system_security_allowdomain(const fn_call& fn);
    as_value system_security_allowinsecuredomain(const fn_call& fn);
    as_value system_security_loadpolicyfile(const fn_call& fn);
    as_value system_setClipboard(const fn_call& fn);
    as_value system_showsettings(const fn_call& fn);
    as_value system_exactsettings(const fn_call& fn);
    as_value system_usecodepage(const fn_call& fn);
    void attachSystemSecurityInterface(as_object& o);
    void attachSystemCapabilitiesInterface(as_object& o);
    void attachSystemInterface(as_object& proto);
}

void
system_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinObject(where, attachSystemInterface, uri);
}


void
registerSystemNative(as_object& where)
{
    VM& vm = getVM(where);
    
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

namespace {

void
attachSystemSecurityInterface(as_object& o)
{
    VM& vm = getVM(o);
    o.init_member("allowDomain", vm.getNative(12, 0));

    Global_as& gl = getGlobal(o);
    o.init_member("allowInsecureDomain",
                  gl.createFunction(system_security_allowinsecuredomain));
    o.init_member("loadPolicyFile",
                  gl.createFunction(system_security_loadpolicyfile));
}

void
attachSystemCapabilitiesInterface(as_object& o)
{
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    //
    // Filesystem, access, miscellaneous hardware information
    //

    // "Windows XP", "Windows 2000", "Windows NT", "Windows 98/ME",
    // "Windows 95", "Windows CE", "Linux", "MacOS"
    // Override in gnashrc
    VM& vm = getVM(o);
    
    const std::string os = vm.getOSName();
    
    const std::string language = systemLanguage(o);
    
    // FIXME: these need to be implemented properly 
    // Does the NetStream object natively support SSL?
    const bool hasTLS = true;
    
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

    const movie_root& m = vm.getRoot();

    int resX;
    int resY;

    boost::tie(resX, resY) = m.callInterface<std::pair<int, int> >(
            HostMessage(HostMessage::SCREEN_RESOLUTION));

    const double screenDPI = m.callInterface<double>(HostMessage(
                HostMessage::SCREEN_DPI));

    const double aspectRatio = m.callInterface<double>(HostMessage(
                HostMessage::PIXEL_ASPECT_RATIO));

    // Note that the pp uses the current locale to display the
    // ratio (for the decimal separator).
    std::ostringstream s;
    s << std::setprecision(7) << aspectRatio;
    const std::string pixelAspectRatio = s.str();

    // "StandAlone", "External", "PlugIn", "ActiveX" (get from GUI)
    const std::string playerType = m.callInterface<std::string>(HostMessage(
                HostMessage::PLAYER_TYPE));

    const std::string screenColor = m.callInterface<std::string>(HostMessage(
                HostMessage::SCREEN_COLOR));

    //
    // Media
    //
        
    // Is audio available?
    const bool hasAudio = (vm.getRoot().runResources().soundHandler());

    // FIXME: these need to be implemented properly. They are mostly
    // self-explanatory.
    const bool hasAudioEncoder = true;
    const bool hasEmbeddedVideo = true;
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
                 << "&V="   << URL::encode(version)
                 << "&M="   << URL::encode(manufacturer)
                 << "&R="   << resX << "x" << resY
                 << "&DP="	<< screenDPI
                 << "&COL="	<< screenColor					
                 << "&AR="  << pixelAspectRatio
                 << "&OS="  << URL::encode(os)
                 << "&L="   << language			
                 << "&PT="  << playerType
                 << "&AVD="	<< trueFalse(avHardwareDisable) 
                 << "&LFD="	<< trueFalse(localFileReadDisable)
                 << "&WD="  << trueFalse(windowlessDisable)
                 << "&TLS="	<< trueFalse(hasTLS);
    
    const int flags = PropFlags::dontDelete
        | PropFlags::dontEnum | PropFlags::readOnly;

    o.init_member("hasAccessibility", hasAccessibility, flags);
    o.init_member("pixelAspectRatio", pixelAspectRatio, flags);
    o.init_member("screenColor", screenColor, flags);
    o.init_member("screenDPI", screenDPI, flags);
    o.init_member("screenResolutionY", resY, flags);
    o.init_member("screenResolutionX", resX, flags);
    o.init_member("hasTLS", hasTLS, flags);
    o.init_member("hasVideoEncoder", hasVideoEncoder, flags);
    o.init_member("hasAudioEncoder", hasAudioEncoder, flags);
    o.init_member("hasMP3", hasMP3, flags);
    o.init_member("hasAudio", hasAudio, flags);
    o.init_member("serverString", serverString.str(), flags);
    o.init_member("version", version, flags);
    o.init_member("hasStreamingAudio", hasStreamingAudio, flags);
    o.init_member("hasStreamingVideo", hasStreamingVideo, flags);
    o.init_member("hasEmbeddedVideo", hasEmbeddedVideo, flags);
    o.init_member("hasPrinting", hasPrinting, flags);
    o.init_member("hasScreenPlayback", hasScreenPlayback, flags);
    o.init_member("hasScreenBroadcast", hasScreenBroadcast, flags);
    o.init_member("isDebugger", isDebugger, flags);
    o.init_member("playerType", playerType, flags);
    o.init_member("avHardwareDisable", avHardwareDisable, flags);
    o.init_member("localFileReadDisable", localFileReadDisable, flags);
    o.init_member("windowlessDisable", windowlessDisable, flags);
    o.init_member("os", os, flags);
    o.init_member("manufacturer", manufacturer, flags);
    o.init_member("language", language, flags);
}

void
attachSystemInterface(as_object& proto)
{
    Global_as& gl = getGlobal(proto);
    
    VM& vm = getVM(proto);
    registerBuiltinObject(proto, attachSystemSecurityInterface,
                          getURI(vm, "security"));
    registerBuiltinObject(proto, attachSystemCapabilitiesInterface,
                          getURI(vm, "capabilities"));
    
    proto.init_member("setClipboard", 
                      gl.createFunction(system_setClipboard));
    
    proto.init_member("showSettings", vm.getNative(2107, 0));
    proto.init_property("useCodepage", &system_usecodepage,
                        &system_usecodepage);
    
    const int flags = PropFlags::dontDelete
        | PropFlags::dontEnum
        | PropFlags::readOnly
        | PropFlags::onlySWF6Up;
    
    proto.init_property("exactSettings", &system_exactsettings,
                        &system_exactsettings, flags);
    
}

// This function returns false if no arguments were passed, true if any
// arguments were passed at all, even if they are not strings. There is
// currently no known way of accessing the list of allowed domains.
as_value
system_security_allowdomain(const fn_call& fn)
{
    LOG_ONCE(log_unimpl(_("System.security.allowDomain")));
    if (!fn.nargs) {
        return as_value(false);
    }
    return as_value(true);
}


as_value
system_security_allowinsecuredomain(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("System.security.allowInsecureDomain")));
    return as_value();
}


as_value
system_security_loadpolicyfile(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("System.security.loadPolicyFile")));
    return as_value();
}

as_value
system_setClipboard(const fn_call& fn)
{
    if (!fn.nargs) {
        return as_value();
    }

    const std::string& s = fn.arg(0).to_string();
    movie_root& m = getRoot(fn);
    m.callInterface(HostMessage(HostMessage::SET_CLIPBOARD, s));

    return as_value();
}

as_value
system_showsettings(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("System.showSettings")));
    return as_value();
}

// FIXME: should return true if shared object files
// are stored under an exact domain name (www.gnashdev.org or
// gnashdev.org); false if both are stored under gnashdev.org.
// Can be set.
as_value
system_exactsettings(const fn_call& fn)
{
    // Getter
    if (fn.nargs == 0) {
        // Is always true until we implement it.
        return as_value(true);   
    }
    else {
        LOG_ONCE(log_unimpl(_("System.exactSettings")));
        return as_value();
    }
}


// FIXME: if true, SWF6+ should treat DisplayObjects as Latin
// charset variants. If false (default), as UtrueFalse-8.
// Can be set.
as_value
system_usecodepage(const fn_call& fn)
{
    // Getter
    if (!fn.nargs) {
        // Is always false until we implement it.
        return as_value(false);   
    }
    else {
        LOG_ONCE(log_unimpl(_("System.useCodepage")));
        return as_value();
    }
}

std::string
systemLanguage(as_object& proto)
{
	// Two-letter language code ('en', 'de') corresponding to ISO 639-1
	// Chinese can be either zh-CN or zh-TW. English used to have a 
	// country (GB, US) qualifier, but that was dropped in version 7 of
    // the player.
 	// This method relies on getting a POSIX-style language code of the form
	// "zh_TW.utf8", "zh_CN" or "it" from the VM.
	// It is obviously very easy to extend support to all language codes, but
	// some scripts rely on there being only 20 possible languages. It could
	// be a run time option if it's important enough to care.

	std::string lang = getVM(proto).getSystemLanguage();
	
	const char* languages[] = {"en", "fr", "ko", "ja", "sv",
				"de", "es", "it", "zh", "pt",
				"pl", "hu", "cs", "tr", "fi",
				"da", "nl", "no", "ru"};
	
	const size_t size = arraySize(languages);
	
	if (std::find(languages, languages + size, lang.substr(0, 2)) !=
            languages + size) {
		if (lang.substr(0, 2) == "zh") {
			// Chinese is the only language since the pp version 7
			// to need an additional qualifier.
			if (lang.substr(2, 3) == "_TW") lang = "zh-TW";
			else if (lang.substr(2, 3) == "_CN") lang = "zh-CN";
			else lang = "xu";
		}
		else {
			// All other matching cases: retain just the first
			// two DisplayObjects.
			lang.erase(2);
		}
	}
	else {
		// Unknown language. We also return this if
		// getSystemLanguage() returns something unexpected. 
		lang = "xu";
	}

	return lang;
}

} // anonymous namespace
} // gnash namespace
