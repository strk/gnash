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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <string>

#ifndef HAVE_WINSOCK_H
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# include <winsock2.h>
# include <windows.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <io.h>
#endif

#ifdef ENABLE_TESTING 
#include "impl.h"
#endif

namespace gnash {

// Define the ports for the RTMP protocols
const short RTMP = 1935;
const short RTMPT = 80;
const short RTMPTS = 443;

#ifdef HAVE_WINSOCK_H
	typedef long   in_addr_t;
#	define inet_lnaof(x) inet_addr(inet_ntoa(x))
	typedef int    socklen_t;
#endif
 
class DSOEXPORT Network {
public:
    Network();
    ~Network();
    
    // Create a new server. After creating it, then you have to wait
    // for an incoming connection.
    bool createServer(void);
    bool createServer(short port);
    
    // Accept a client connection for the current server.
    bool newConnection(void);
    bool newConnection(bool block);
    
    // Create a client connection to a tcp/ip server
    bool createClient(void);
    bool createClient(short port);
    bool createClient(const char *hostname);
    bool createClient(const char *hostname, short port);

    // Read from the connection
    int readNet(char *buffer, int nbytes);
    int readNet(char *buffer, int nbytes, int timeout);
    int readNet(int fd, char *buffer, int nbytes);
    int readNet(int fd, char *buffer, int nbytes, int timeout);
    
    // Write to the connection
    int writeNet(std::string buffer);
    int writeNet(char const *buffer);
    int writeNet(char const *buffer, int nbytes);
    int writeNet(const unsigned char *buffer, int nbytes);
    int writeNet(int fd, char const *buffer);
    int writeNet(int fd, char const *buffer, int nbytes);
    int writeNet(int fd, char const *buffer, int nbytes, int timeout);
    
    // Close the connection
    bool closeNet();
    bool closeNet(int fd);
    bool closeConnection();
    bool closeConnection(int fd);

    // Change the debug flag
    void toggleDebug(bool val);
    
    bool send(const char *str);
    //#ifdef ENABLE_TESTING 
    // Accessors for testing
    bool connected()            { return _connected; };
    int getFileFd()             { return _sockfd; };
    int getListenFd()           { return _listenfd; };
    short getPort()             { return _port; };
    std::string getURL()        { return _url; }
    std::string getProtocol()   { return _protocol; }
    std::string getHost()       { return _host; }
    std::string getPortStr()    { return _portstr; }
    std::string getPath()       { return _path; }
    int getTimeout()            { return _timeout; }
    //#endif
    Network &operator = (Network &net);
 protected:
    in_addr_t   _ipaddr;
    int         _sockfd;
    int         _listenfd;
    short       _port;
    std::string _portstr;
    std::string _url;
    std::string _protocol;
    std::string _host;
    std::string _path;
    bool        _connected;
    bool        _debug;
    int         _timeout;
};

} // end of gnash namespace

// __NETWORK_H__
#endif

