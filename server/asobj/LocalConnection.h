// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifndef __LOCALCONNECTION_H__
#define __LOCALCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <map>

#include "as_object.h" // for inheritance
#include "fn_call.h"

#ifdef NETWORK_CONN
#include "network.h"
#else
#include "shm.h"
#endif

namespace gnash {
  
#ifdef NETWORK_CONN
class LocalConnection : public as_object {
#else
class LocalConnection : public as_object, Shm {
#endif

public:
    LocalConnection();
    ~LocalConnection();
    void close(void);
    bool connect(const std::string& name);
    std::string domain(int version);
    void send();
// FIXME: these should be callbacks
//     bool        _allowDomain;
//     bool        _allowInsecureDomain;
//     bool        _onStatus;
private:
    std::string _name;
    std::map<const char *, short> _allocated;
};

#if 0
class localconnection_as_object : public as_object
{
public:
    LocalConnection obj;
};
#endif

as_value localconnection_new(const fn_call& fn);
as_value localconnection_close(const fn_call& fn);
as_value localconnection_connect(const fn_call& fn);
as_value localconnection_domain(const fn_call& fn);
as_value localconnection_send(const fn_call& fn);

} // end of gnash namespace

// __LOCALCONNECTION_H__
#endif

