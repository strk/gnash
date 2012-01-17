// TextField_as.h:  ActionScript 3 "TextField" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_TEXTFIELD_AS_H
#define GNASH_ASOBJ3_TEXTFIELD_AS_H

namespace gnash {

class as_object;
struct ObjectURI;
class Global_as;

/// Native function to create a plain object with TextField properties
//
/// This function calls the TextField constructor.
as_object* createTextFieldObject(Global_as& gl);

/// Initialize the global TextField class
void textfield_class_init(as_object& global, const ObjectURI& uri);

void registerTextFieldNative(as_object& global);

} // namespace gnash

// end of GNASH_ASOBJ3_TEXTFIELD_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

