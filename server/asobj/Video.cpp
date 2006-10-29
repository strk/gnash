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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Video.h"
#include "fn_call.h"

namespace gnash {

Video::Video()
{
    GNASH_REPORT_FUNCTION;
}

Video::~Video()
{
    GNASH_REPORT_FUNCTION;
}


void
Video::attach()
{
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
}

void
Video::clear()
{
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
}
void
video_new(const fn_call& fn)
{
    video_as_object *video_obj = new video_as_object;

    video_obj->set_member("attach", &video_attach);
    video_obj->set_member("clear", &video_clear);

    fn.result->set_as_object(video_obj);
}
void video_attach(const fn_call& /*fn*/) {
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
}
void video_clear(const fn_call& /*fn*/) {
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
}

} // end of gnaash namespace

