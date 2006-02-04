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

#ifndef __NETCONNECTION_H__
#define __NETCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "impl.h"
#include "log.h"

namespace gnash {

// Define the ports for the RTMP protocols
const int RTMP = 1935;
const int RTMPT = 80;

class NetConnection {
public:
    NetConnection();
    ~NetConnection();
    void connect(const char *arg);

    // Accessors for testing, not supported by Flash
    bool connected() { return _connected; }
    bool connected(bool state) { _connected = state; }

#ifdef ENABLE_TESTING
    std::string getURL()        { return _url; }
    std::string getProtocol()   { return _protocol; }
    std::string getHost()       { return _host; }
    std::string getPort()       { return _port; }
    std::string getPath()       { return _path; }
    
    void setURL(std::string url) { _url = url; };
    void setProtocol(std::string proto) { _protocol = proto; };
    void setHost(std::string host) { _host = host; };
    void setPort(std::string port) { _port = port; };
    void setPath(std::string path) { _path = path; };
#endif
private:
    bool        _connected;
    std::string _url;
    std::string _protocol;
    std::string _host;
    std::string _port;
    std::string _path;
};

struct netconnection_as_object : public as_object
{
    NetConnection obj;
};
void netconnection_new(const fn_call& fn);
void netconnection_connect(const fn_call& fn);

#ifdef ENABLE_TESTING
void netconnection_geturl(const fn_call& fn);
void netconnection_getprotocol(const fn_call& fn);
void netconnection_gethost(const fn_call& fn);
void netconnection_getport(const fn_call& fn);
void netconnection_getpath(const fn_call& fn);
#endif
} // end of gnash namespace

// __NETCONNECTION_H__
#endif
