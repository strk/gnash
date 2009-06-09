// LocalConnection_as.h:  ActionScript 3 "LocalConnection" class, for Gnash.
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

#ifndef GNASH_ASOBJ3_LOCALCONNECTION_H
#define GNASH_ASOBJ3_LOCALCONNECTION_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <map>
#include <boost/cstdint.hpp>

#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "lcshm.h"

#include "VM.h"
#include "builtin_function.h" // need builtin_functionin

namespace gnash {

// Forward declarations
class as_object;


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

    std::string &getName() { return _name; };

    bool connected() { return _connected; };
    
    static void init(as_object& global);

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

/// Initialize the global LocalConnection class

as_object* getLocalConnectionInterface();
as_value localconnection_ctor(const fn_call& fn);
//static void init(as_object& global);

/*
#ifndef GNASH_ASOBJ3_LOCALCONNECTION_INIT
#define GNASH_ASOBJ3_LOCALCONNECTION_INIT
// extern (used by Global.cpp)
void LocalConnection_as::init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&localconnection_ctor, getLocalConnectionInterface());
        attachLocalConnectionStaticInterface(*cl);
    }

    // Register _global.LocalConnection
    global.init_member("LocalConnection", cl.get());
}
#endif
*/

} // gnash namespace

// GNASH_ASOBJ3_LOCALCONNECTION_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

