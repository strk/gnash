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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//
//

#ifndef __LOCALCONNECTION_H__
#define __LOCALCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <map>

#include "impl.h"
#include "log.h"
#ifdef NETWORK_CONN
#include "network.h"
#else
#include "shm.h"
#endif

namespace gnash {
  
#ifdef NETWORK_CONN
class LocalConnection : public Network {
#else
class LocalConnection : public Shm {
#endif
public:
    LocalConnection();
    ~LocalConnection();
    void close(void);
    bool connect(const char *name);
    std::string domain(void);
    void send();
// FIXME: these should be callbacks
//     bool        _allowDomain;
//     bool        _allowInsecureDomain;
//     bool        _onStatus;
private:
    std::string _name;
    std::map<const char *, short> _allocated;
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

