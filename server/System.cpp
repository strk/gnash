// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "System.h"

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
void
system_new(const fn_call& fn)
{
    system_as_object *system_obj = new system_as_object;

    system_obj->set_member("security.allowdomain", &system_security_allowdomain);
    system_obj->set_member("security.allowinsecuredomain", &system_security_allowinsecuredomain);
    system_obj->set_member("security.loadpolicyfile", &system_security_loadpolicyfile);
    system_obj->set_member("setclipboard", &system_setclipboard);
    system_obj->set_member("showsettings", &system_showsettings);

    fn.result->set_as_object(system_obj);
}
void system_security_allowdomain(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void system_security_allowinsecuredomain(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void system_security_loadpolicyfile(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void system_setclipboard(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void system_showsettings(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

