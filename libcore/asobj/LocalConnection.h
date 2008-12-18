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

#ifndef GNASH_ASOBJ_LOCALCONNECTION_H
#define GNASH_ASOBJ_LOCALCONNECTION_H

#include <string>
#include <map>
#include <boost/cstdint.hpp> 

#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "lcshm.h"

namespace gnash {
  
  class LocalConnection : public as_object, amf::LcShm {
public:
    LocalConnection();
    ~LocalConnection();
    void close(void);
    bool connect();
    bool connect(const std::string& name);
    const std::string& domain() {
        return _domain;
    }


    void send();
    std::string &getName() { return _name; };
    bool connected() { return _connected; };
    
private:
    
    std::string getDomain();
    
    bool _connected;
    std::string _name;
    std::map<const char *, short> _allocated;
    const std::string _domain;
    
};

void localconnection_class_init(as_object& glob);

} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
