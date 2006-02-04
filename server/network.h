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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "xml.h"
#include "impl.h"
#include "log.h"

namespace gnash {

class Network {
public:
    Network();
    ~Network();
    
    bool clientConnect(const char *host, short port);
    bool serverConnect(short port);
    void close();
    bool send(const char *str);
#ifdef ENABLE_TESTING 
    // Accessors for testing
    bool connected()            { return _connected; };
    int getFileno()             { return static_cast<int>(_sockfd); };
    short getPort()             { return _port; };
    std::string getURL()        { return _url; }
    std::string getProtocol()   { return _protocol; }
    std::string getHost()       { return _host; }
    std::string getPortStr()    { return _portstr; }
    std::string getPath()       { return _path; }    
#endif
protected:
    short       _sockfd;
    short       _port;
    std::string _portstr;
    std::string _url;
    std::string _protocol;
    std::string _host;
    std::string _path;
    bool        _connected;
};

struct network_as_object : public as_object
{
    Network obj;
};

#ifdef ENABLE_TESTING 
void network_geturl(const fn_call& fn);
void network_getprotocol(const fn_call& fn);
void network_gethost(const fn_call& fn);
void network_getport(const fn_call& fn);
void network_getpath(const fn_call& fn);
#endif
} // end of gnash namespace

// __NETWORK_H__
#endif

