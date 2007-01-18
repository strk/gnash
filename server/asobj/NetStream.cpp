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

// 
//
//

/* $Id: NetStream.cpp,v 1.22 2007/01/18 22:53:21 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "NetStream.h"
#include "fn_call.h"

#include "movie_root.h"

#if defined(_WIN32) || defined(WIN32)
	#include <Windows.h>	// for sleep()
	#define usleep(x) Sleep(x/1000)
#else
	#include "unistd.h" // for usleep()
#endif

namespace gnash {
 
void
netstream_new(const fn_call& fn)
{
    netstream_as_object *netstream_obj = new netstream_as_object;

    // FIXME: rely on inheritance
    netstream_obj->init_member("close", &netstream_close);
    netstream_obj->init_member("pause", &netstream_pause);
    netstream_obj->init_member("play", &netstream_play);
    netstream_obj->init_member("seek", &netstream_seek);
    netstream_obj->init_member("setBufferTime", &netstream_setbuffertime);

    fn.result->set_as_object(netstream_obj);

	if (fn.nargs > 0)
	{
		as_object* nc = fn.arg(0).to_object();
		if ( nc ) netstream_obj->obj.setNetCon(nc);
		else
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_warning("First argument "
					"to NetConnection constructor "
					"doesn't cast to an Object (%s)",
					fn.arg(0).to_string());
			);
		}
	}

}

void netstream_close(const fn_call& fn)
{
	assert(dynamic_cast<netstream_as_object*>(fn.this_ptr));
	netstream_as_object* ns = static_cast<netstream_as_object*>(fn.this_ptr);
	ns->obj.close();
}

void netstream_pause(const fn_call& fn)
{
	assert(dynamic_cast<netstream_as_object*>(fn.this_ptr));
	netstream_as_object* ns = static_cast<netstream_as_object*>(fn.this_ptr);
	
	// mode: -1 ==> toogle, 0==> pause, 1==> play
	int mode = -1;
	if (fn.nargs > 0)
	{
		mode = fn.arg(0).to_bool() ? 0 : 1;
	}
	ns->obj.pause(mode);	// toggle mode
}

void netstream_play(const fn_call& fn)
{
	assert(dynamic_cast<netstream_as_object*>(fn.this_ptr));
	netstream_as_object* ns = static_cast<netstream_as_object*>(fn.this_ptr);
	
	if (fn.nargs < 1)
	{
    log_error("NetStream play needs args\n");
    return;
	}

	if (ns->obj.play(fn.arg(0).to_string()) != 0)
	{
		ns->obj.close();
	};
}

void netstream_seek(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_setbuffertime(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnash namespace

