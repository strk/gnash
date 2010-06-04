// ExternalInterface_as.h:  ActionScript "ExternalInterface" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_ASOBJ_EXTERNALINTERFACE_H
#define GNASH_ASOBJ_EXTERNALINTERFACE_H

#include <string>
#include <vector>
#include <map>

namespace gnash {

class as_object;
class as_value;
class ObjectURI;
class Global_as;
class movie_root;
}

namespace gnash {

class ExternalInterface_as
{
public:
    ExternalInterface_as(as_object* owner);
    ~ExternalInterface_as();
    
private:
    int                 _fd;
    std::map<std::string, as_object *> _methods;
};

/// Initialize the global ExternalInterface class
void externalinterface_class_init(gnash::as_object& where,
                                  const gnash::ObjectURI& uri);

void registerExternalInterfaceNative(as_object& global);

} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
