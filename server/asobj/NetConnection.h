// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

#ifndef __NETCONNECTION_H__
#define __NETCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

// TODO: port to new AS architecture
//
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "network.h"

namespace gnash {
    
class NetConnection : public Network {
public:
    NetConnection();
    ~NetConnection();
    bool connect(const char *arg);
 private:
    std::string _app;
    std::string _flashVer;
    std::string _swfUrl;
    std::string _tcUrl;
};

class netconnection_as_object : public as_object
{
public:
    NetConnection obj;
};

DSOEXPORT void netconnection_new(const fn_call& fn);
DSOEXPORT void netconnection_connect(const fn_call& fn);

#ifdef ENABLE_TESTING 

DSOEXPORT void netconnection_geturl(const fn_call& fn);
DSOEXPORT void netconnection_getprotocol(const fn_call& fn);
DSOEXPORT void netconnection_gethost(const fn_call& fn);
DSOEXPORT void netconnection_getport(const fn_call& fn);
DSOEXPORT void netconnection_getpath(const fn_call& fn);
DSOEXPORT void netconnection_connected(const fn_call& fn);

DSOEXPORT void netconnection_getfilefd(const fn_call& fn);
DSOEXPORT void netconnection_getlistenfd(const fn_call& fn);
#endif

} // end of gnash namespace

// __NETCONNECTION_H__
#endif
