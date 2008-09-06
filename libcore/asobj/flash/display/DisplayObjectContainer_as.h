// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
//
//

// Implementation for ActionScript DisplayObjectContainer object.

#ifndef GNASH_DISPLAYOBJECTCONTAINER_H
#define GNASH_DISPLAYOBJECTCONTAINER_H

#include <memory> // for auto_ptr

namespace gnash {

class as_object;

/// Initialize the DisplayObjectContainer class
void display_object_container_class_init(as_object& where);

/// Return a DisplayObjectContainer instance
std::auto_ptr<as_object> init_display_object_container_instance();

as_object* getDisplayObjectContainerInterface();


}

#endif // GNASH_DISPLAYOBJECTCONTAINER_H
