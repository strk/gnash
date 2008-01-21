// System.cpp:  ActionScript "System" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "System.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface
#include "URL.h" // for encoding serverString

#define TF(x) (x ? "t" : "f")

namespace gnash {

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

	// "LNX 9,0,22,0", "MAC 8,0,99,0"
	// Override in gnashrc
	const std::string version = VM::get().getPlayerVersion();

	// Flash 7: "StandAlone", "External", "PlugIn", "ActiveX"
	// TODO: Implement properly
	const std::string playerType = "StandAlone";

	// "Windows XP", "Windows 2000", "Windows NT", "Windows 98/ME", "Windows 95", "Windows CE", "Linux", "MacOS"
	// Override in gnashrc
	const std::string os = VM::get().getOSName();

	// "Macromedia Windows", "Macromedia Linux", "Macromedia MacOS"
	// Override in gnashrc
	const std::string manufacturer = rcfile.getFlashSystemManufacturer();

	/* Human Interface */
	
	// Two-letter language code ('en', 'de') corresponding to ISO 639-1
	// TODO: Chinese should be either zh-CN or zh-TW
	// TODO: Are there more than the 20 officially documented? 
	// TODO: Other / unknown should return 'xu'. 
	const std::string language = VM::get().getSystemLanguage();

	/* Media */
		
	// Is audio available?
	const bool hasAudio = (get_sound_handler() != NULL);

	/* A URL-encoded string to send system info to a server.*/
	/* Boolean values are represented as t or f.		*/
	/* Privacy concerns should probably be addressed by 	*/
	/* allowing this string to be sent or not; individual	*/
	/* values that might affect privacy can be overridden	*/
	/* in gnashrc.						*/

	std::string serverString =
		 	+ "OS=" + URL::encode(os) 
			+ "&A=" + TF(hasAudio)
			+ "&V=" + URL::encode(version)
			+ "&PT=" + playerType
			+ "&L=" + language
			+ "&AVD="	// avHardwareDisable (bool)
			+ "&ACC="	// hasAccessibility (bool)
			+ "&AE="	// hasAudioEncoder (bool)
			+ "&EV="	// hasEmbeddedVideo (bool)
			+ "&IME="	// hasIME (bool)
			+ "&MP3="	// hasMP3 (bool)
			+ "&PR="	// hasPrinting (bool)
			+ "&SB="	// hasScreenBroadcast (bool)
			+ "&SP="	// hasScreenPlayback (bool)
			+ "&SA="	// hasStreamingAudio (bool)
			+ "&SV="	// hasStreamingVideo (bool)
			+ "&VE="	// hasVideoEncoder (bool)
			+ "&DEB="	// isDebugger (bool)
			+ "&LFD="	// localFileReadDisable (bool)
			+ "&M=" + URL::encode(manufacturer)
			+ "&AR="	// pixelAspectRatio (double)
			+ "&COL="	// screenColor (?)
			+ "&DP="	// screenDPI (int?)
			+ "&R="	// + screenResolutionX + "x" + screenResolutionY
			;
		
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
		proto->init_member("serverString", serverString, flags);
		
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

} // end of gnash namespace
