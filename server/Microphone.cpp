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
#include "Microphone.h"

namespace gnash {

Microphone::Microphone() {
}

Microphone::~Microphone() {
}


void
Microphone::get()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Microphone::setGain()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Microphone::setRate()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Microphone::setSilenceLevel()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Microphone::setUseEchoSuppression()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
microphone_new(const fn_call& fn)
{
    microphone_as_object *microphone_obj = new microphone_as_object;

    microphone_obj->set_member("get", &microphone_get);
    microphone_obj->set_member("setgain", &microphone_setgain);
    microphone_obj->set_member("setrate", &microphone_setrate);
    microphone_obj->set_member("setsilencelevel", &microphone_setsilencelevel);
    microphone_obj->set_member("setuseechosuppression", &microphone_setuseechosuppression);

    fn.result->set_as_object(microphone_obj);
}
void microphone_get(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void microphone_setgain(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void microphone_setrate(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void microphone_setsilencelevel(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void microphone_setuseechosuppression(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

