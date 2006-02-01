// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __LOCALCONNECTION_H__
#define __LOCALCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"

namespace gnash {
  
class LocalConnection {
public:
    LocalConnection();
    ~LocalConnection();
   void close();
   void connect();
   void domain();
   void send();
private:
    bool _allowDomain;
    bool _allowInsecureDomain;
    bool _onStatus;
};

struct localconnection_as_object : public as_object
{
    LocalConnection obj;
};

void localconnection_new(const fn_call& fn);
void localconnection_close(const fn_call& fn);
void localconnection_connect(const fn_call& fn);
void localconnection_domain(const fn_call& fn);
void localconnection_send(const fn_call& fn);

} // end of gnash namespace

// __LOCALCONNECTION_H__
#endif

