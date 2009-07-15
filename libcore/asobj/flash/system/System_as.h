// System_as.h:  ActionScript 3 "System" class, for Gnash.
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
//

#ifndef GNASH_ASOBJ3_SYSTEM_H
#define GNASH_ASOBJ3_SYSTEM_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>


namespace gnash {

// Forward declarations
class as_object;

/// Initialize the global System class
void system_class_init(as_object& global);

void registerSystemNative(as_object& global);


/// Get the vector aof allowed domains to access
const std::vector<std::string>& getAllowDataAccess();

/// add a url string to the vector of allowed domains
bool addAllowDataAccess( const std::string& url );

} // gnash namespace

// GNASH_ASOBJ3_SYSTEM_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

