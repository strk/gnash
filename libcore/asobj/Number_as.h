// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

// Implementation for ActionScript Number object.

#ifndef GNASH_NUMBER_H
#define GNASH_NUMBER_H



namespace gnash {

class as_object;
struct ObjectURI;
class Global_as;

/// Initialize the global Number class
void number_class_init(as_object& global, const ObjectURI& uri);

void registerNumberNative(as_object& global);

}

#endif // GNASH_NUMBER_H
