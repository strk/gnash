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
#include "Stage.h"

namespace gnash {

Stage::Stage() {
}

Stage::~Stage() {
}


void
Stage::addListener()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Stage::removeListener()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
stage_new(const fn_call& fn)
{
    stage_as_object *stage_obj = new stage_as_object;

    stage_obj->set_member("addlistener", &stage_addlistener);
    stage_obj->set_member("removelistener", &stage_removelistener);

    fn.result->set_as_object(stage_obj);
}
void stage_addlistener(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void stage_removelistener(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

