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
#include "NetStream.h"

namespace gnash {

NetStream::NetStream() {
}

NetStream::~NetStream() {
}


void
NetStream::close()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::pause()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::play()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::seek()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::setBufferTime()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
netstream_new(const fn_call& fn)
{
    netstream_as_object *netstream_obj = new netstream_as_object;

    netstream_obj->set_member("close", &netstream_close);
    netstream_obj->set_member("pause", &netstream_pause);
    netstream_obj->set_member("play", &netstream_play);
    netstream_obj->set_member("seek", &netstream_seek);
    netstream_obj->set_member("setbuffertime", &netstream_setbuffertime);

    fn.result->set_as_object(netstream_obj);
}
void netstream_close(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_pause(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_play(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_seek(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_setbuffertime(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

