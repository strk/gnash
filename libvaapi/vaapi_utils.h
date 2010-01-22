// vaapi_utils.h: VA API utilities
// 
// Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#ifndef GNASH_VAAPI_UTILS_H
#define GNASH_VAAPI_UTILS_H

#include "vaapi_common.h"

namespace gnash {

/// Debug output
void DSOEXPORT vaapi_dprintf(const char *format, ...);

/// Check VA status for success or print out an error
bool DSOEXPORT vaapi_check_status(VAStatus status, const char *msg);

} // gnash namespace

#endif /* GNASH_VAAPI_UTILS_H */
