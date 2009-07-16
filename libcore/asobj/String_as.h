// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

// Implementation for ActionScript String object.

#ifndef GNASH_STRING_H
#define GNASH_STRING_H

#include <string>

namespace gnash {

class as_object;
class ObjectURI;
class Global_as;

// Initialize the global String class
void string_class_init(as_object& global, const ObjectURI& uri);

/// Return a String instance (possibibly NULL!)
//
/// This function will use the native String constructor in SWF5, but
/// any function registered by user as the _global.String for SWF6 and higher.
/// In the second case, not finding a proper constructor might result in
/// returning the NULL object.
///
as_object* init_string_instance(Global_as& g, const std::string& val);

void registerStringNative(as_object& global);

}

#endif // GNASH_STRING_H
