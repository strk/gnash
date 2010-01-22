// vaapi.h: VA API wrapper
// 
//   Copyright (C) 2009 Splitted-Desktop Systems
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

#ifndef GNASH_VAAPI_H
#define GNASH_VAAPI_H

#include "vaapi_common.h"
#include "vaapi_utils.h"
#include "VaapiGlobalContext.h"
#include "VaapiContext.h"
#include "VaapiSurface.h"
#include "VaapiSurfaceProxy.h"
#include "VaapiImage.h"
#include "VaapiException.h"

namespace gnash {

/// Enable video acceleration (with VA API)
void DSOEXPORT vaapi_enable();

/// Disable video acceleration (with VA API)
void DSOEXPORT vaapi_disable();

/// Check whether video acceleration is enabled
bool DSOEXPORT vaapi_is_enabled();

} // gnash namespace

#endif /* GNASH_VAAPI_H */
