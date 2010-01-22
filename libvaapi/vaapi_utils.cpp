// vaapi_utils.cpp: VA API utilities
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

#include "vaapi_utils.h"
#include <stdio.h>
#include <stdarg.h>

namespace gnash {

// Debug output
void DSOEXPORT vaapi_dprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[GnashVaapi] ");
    vfprintf(stdout, format, args);
    va_end(args);
}

// Check VA status for success or print out an error
bool vaapi_check_status(VAStatus status, const char *msg)
{
    if (status != VA_STATUS_SUCCESS) {
	vaapi_dprintf("%s: %s\n", msg, vaErrorStr(status));
	return false;
    }
    return true;
}

} // gnash namespace
