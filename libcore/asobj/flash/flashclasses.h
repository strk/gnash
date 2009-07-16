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

#ifndef GNASH_ASOBJ3_FLASH_H
#define GNASH_ASOBJ3_FLASH_H 1

// These headers include the prototypes of all the *_init functions
// for each class.
#include "display/display_pkg.h"
#include "external/external_pkg.h"
#include "filters/filters_pkg.h"
#include "geom/geom_pkg.h"
#include "net/net_pkg.h"
#include "text/text_pkg.h"

#include "sharedlib.h"

namespace gnash {

static gnash::SharedLib::initentry* as2classes[] = {
    0
};

} // end of gnash namespace

#endif // end of GNASH_ASOBJ3_FLASH_H

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
