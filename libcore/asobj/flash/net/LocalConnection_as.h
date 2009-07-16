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

#ifndef GNASH_ASOBJ_LOCALCONNECTION_H
#define GNASH_ASOBJ_LOCALCONNECTION_H

#include <string>
#include <map>
#include <boost/cstdint.hpp> 

#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "lcshm.h"

namespace gnash {
  
class LocalConnection_as : public as_object, amf::LcShm
{

public:

    LocalConnection_as();
    ~LocalConnection_as();

    void close();

    void connect(const std::string& name);

    const std::string& domain() {
        return _domain;
    }

    void send();

    const std::string& name() { return _name; };

    bool connected() { return _connected; };
    
    static void init(as_object& glob, const ObjectURI& uri);

private:
    
    /// Work out the domain.
    //
    /// Called once on construction to set _domain, though it will do
    /// no harm to call it again.
    std::string getDomain();
    
    bool _connected;
    std::string _name;

    // The immutable domain of this LocalConnection_as, based on the 
    // originating SWF's domain.
    const std::string _domain;
    
};

} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
