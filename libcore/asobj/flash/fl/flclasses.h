// 
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_FL_H
#define GNASH_ASOBJ3_FL_H 1

#include "accessibility/fl_accessibility_pkg.h"
#include "containers/containers_pkg.h"
#include "controls/controls_pkg.h"
#include "core/core_pkg.h"
#include "managers/managers_pkg.h"
#include "motion/motion_pkg.h"
#include "transitions/transitions_pkg.h"
#include "video/video_pkg.h"

#include <sharedlib.h>

static gnash::SharedLib::initentry *asclasses[] = {
    gnash::flash_accessibility_package_init,
    gnash::flash_containers_package_init,
    gnash::flash_controls_package_init,
    gnash::flash_core_package_init,
    gnash::flash_managers_package_init,
    gnash::flash_motion_package_init,
    gnash::flash_transitions_package_init,
    gnash::flash_video_package_init,
    0
};

#endif // end of GNASH_ASOBJ3_FL_H

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
