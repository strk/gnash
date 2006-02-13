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
#include "LoadVars.h"

namespace gnash {

LoadVars::LoadVars() {
}

LoadVars::~LoadVars() {
}


void
LoadVars::addRequestHeader()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::decode()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::getBytesLoaded()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::getBytesTotal()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::load()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::send()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::sendAndLoad()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
LoadVars::toString()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
loadvars_new(const fn_call& fn)
{
    loadvars_as_object *loadvars_obj = new loadvars_as_object;

    loadvars_obj->set_member("addrequestheader", &loadvars_addrequestheader);
    loadvars_obj->set_member("decode", &loadvars_decode);
    loadvars_obj->set_member("getbytesloaded", &loadvars_getbytesloaded);
    loadvars_obj->set_member("getbytestotal", &loadvars_getbytestotal);
    loadvars_obj->set_member("load", &loadvars_load);
    loadvars_obj->set_member("send", &loadvars_send);
    loadvars_obj->set_member("sendandload", &loadvars_sendandload);
    loadvars_obj->set_member("tostring", &loadvars_tostring);

    fn.result->set_as_object(loadvars_obj);
}
void loadvars_addrequestheader(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_decode(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_getbytesloaded(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_getbytestotal(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_load(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_send(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_sendandload(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void loadvars_tostring(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

