// vaapi.c: VA API wrapper
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

#include "vaapi.h"
#include <stdlib.h>
#include <string.h>

namespace gnash {

/// Parse ENV environment variable expecting "yes" | "no" values
static bool
getenv_yesno(const char *env, int *pval)
{
    const char *env_str = getenv(env);
    if (!env_str)
        return false;

    int val;
    if (strcmp(env_str, "1") == 0 || strcmp(env_str, "yes") == 0)
        val = 1;
    else if (strcmp(env_str, "0") == 0 || strcmp(env_str, "no") == 0)
        val = 0;
    else
        return false;

    if (pval)
        *pval = val;
    return true;
}

static int g_vaapi_is_enabled = -1;

// Enable video acceleration (with VA API)
void vaapi_enable()
{
    g_vaapi_is_enabled = 1;
}

// Disable video acceleration (with VA API)
void vaapi_disable()
{
    g_vaapi_is_enabled = 0;
}

// Check whether video acceleration is enabled
bool vaapi_is_enabled()
{
    if (g_vaapi_is_enabled < 0) {
        if (!getenv_yesno("GNASH_VAAPI", &g_vaapi_is_enabled))
            g_vaapi_is_enabled = 1;
    }
    return g_vaapi_is_enabled;
}

} // gnash namespace
