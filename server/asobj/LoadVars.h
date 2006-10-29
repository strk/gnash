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

#ifndef __LOADVARS_H__
#define __LOADVARS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"

namespace gnash {
  
class LoadVars {
public:
    LoadVars();
    ~LoadVars();
   void addRequestHeader();
   void decode();
   void getBytesLoaded();
   void getBytesTotal();
   void load();
   void send();
   void sendAndLoad();
   void toString();
private:
    bool _contentType;
    bool _loaded;
    bool _onData;
    bool _onLoad;
};

class loadvars_as_object : public as_object
{
public:
    LoadVars obj;
};

void loadvars_new(const fn_call& fn);
void loadvars_addrequestheader(const fn_call& fn);
void loadvars_decode(const fn_call& fn);
void loadvars_getbytesloaded(const fn_call& fn);
void loadvars_getbytestotal(const fn_call& fn);
void loadvars_load(const fn_call& fn);
void loadvars_send(const fn_call& fn);
void loadvars_sendandload(const fn_call& fn);
void loadvars_tostring(const fn_call& fn);

} // end of gnash namespace

// __LOADVARS_H__
#endif

