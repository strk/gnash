// LoadableObject.h: abstraction of network-loadable AS object functions.
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
//

#ifndef GNASH_LOADABLE_OBJECT_H
#define GNASH_LOADABLE_OBJECT_H

#include <boost/noncopyable.hpp>
#include "as_object.h"
#include "smart_ptr.h"
#include "LoadThread.h"

namespace gnash {


/// Abstract class for loadable AS objects' interface
//
/// This is used for XML_as and LoadVars_as to abstract their identical
/// network loading functions.
///
/// It is a virtual base class because XML_as also inherits from XMLNode.
//
/// It may not be copied.
class LoadableObject
{
public:


    /// Register methods as native for use by XML_as and LoadVars_as
    static void registerNative(as_object& global);

    /// Shared AS methods for XML and LoadVars, which can be used
    /// interchangeably with each object in ActionScript.
    static as_value loadableobject_addRequestHeader(const fn_call& fn);
    
    /// These functions return the value of _bytesTotal and _bytesLoaded
    static as_value loadableobject_getBytesLoaded(const fn_call& fn);
    static as_value loadableobject_getBytesTotal(const fn_call& fn);

};


}

#endif
