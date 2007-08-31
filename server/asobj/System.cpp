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

namespace gnash {

#if 0
System::System() {
}

System::~System() {
}


void
System::security_allowDomain()
{
    log_unimpl (__PRETTY_FUNCTION__);
}

void
System::security_allowInsecureDomain()
{
    log_unimpl (__PRETTY_FUNCTION__);
}

void
System::security_loadPolicyFile()
{
    log_unimpl (__PRETTY_FUNCTION__);
}

void
System::setClipboard()
{
    log_unimpl (__PRETTY_FUNCTION__);
}

void
System::showSettings()
{
    log_unimpl (__PRETTY_FUNCTION__);
}

#endif

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
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object(getObjectInterface());
		proto->init_member("version", VM::get().getPlayerVersion() );
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
		//proto->init_member("constructor", new builtin_function(system_new)); 
	}
	return proto.get();
}

system_as_object::system_as_object()
	:
	as_object(getSystemInterface()) // pass System inheritence
{
}

as_value
system_new(const fn_call& /* fn */)
{
    system_as_object *system_obj = new system_as_object();

    return as_value(system_obj);
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
