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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "System.h"
#include "fn_call.h"
#include "builtin_function.h"

namespace gnash {

System::System() {
}

System::~System() {
}


void
System::security_allowDomain()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
System::security_allowInsecureDomain()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
System::security_loadPolicyFile()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
System::setClipboard()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
System::showSettings()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

static as_object*
getSystemSecurityInterface()
{
	static as_object* proto = NULL;
	if ( proto == NULL )
	{
		proto = new as_object();
		proto->set_member("allowdomain", &system_security_allowdomain);
		proto->set_member("allowinsecuredomain", &system_security_allowinsecuredomain);
		proto->set_member("loadpolicyfile", &system_security_loadpolicyfile);
	}
	return proto;
}

static as_object*
getSystemCapabilitiesInterface()
{
	static as_object* proto = NULL;
	if ( proto == NULL )
	{
		proto = new as_object();
		proto->set_member("version", "Gnash-" VERSION);
	}
	return proto;
}

static void
attachSystemInterface(as_object* proto)
{
	// Initialize Function prototype
	proto->set_member("security", getSystemSecurityInterface());
	proto->set_member("capabilities", getSystemCapabilitiesInterface());
	proto->set_member("setclipboard", &system_setclipboard);
	proto->set_member("showsettings", &system_showsettings);
}

static as_object*
getSystemInterface()
{
	static as_object* proto = NULL;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachSystemInterface(proto);
		proto->set_member("constructor", &system_new); 
		proto->set_member_flags("constructor", 1);
	}
	return proto;
}

system_as_object::system_as_object()
	:
	as_object(getSystemInterface()) // pass System inheritence
{
}

void
system_new(const fn_call& fn)
{
    system_as_object *system_obj = new system_as_object();

    fn.result->set_as_object(system_obj);
}

void system_security_allowdomain(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void system_security_allowinsecuredomain(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void system_security_loadpolicyfile(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void system_setclipboard(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void system_showsettings(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
system_init(as_object* glob)
{
	// This is going to be the global System "class"/"function"
	static as_function* sys=NULL;

	if ( sys == NULL )
	{
		sys = new builtin_function(NULL, getSystemInterface());

		// We replicate interface to the System class itself
		attachSystemInterface(sys);
	}

	// Register _global.System
	glob->set_member("System", sys);

}


} // end of gnaash namespace

