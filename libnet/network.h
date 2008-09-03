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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#if !defined(HAVE_WINSOCK_H) || defined(__OS2__)
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# include <winsock2.h>
# include <windows.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <io.h>
#endif

#include "dsodefs.h" //For DSOEXPORT.
#include <boost/cstdint.hpp>
#include <cassert>
#include <string>

namespace amf {
class Buffer;
}

namespace gnash {

// Define the ports for the RTMP protocols
const short ADMIN_PORT = 1111;
const short RTMP_PORT = 1935;
const short RTMPE_PORT = 1935;
const short RTMPT_PORT = 80;
const short RTMPTE_PORT = 80;
const short RTMPTS_PORT = 443;

#ifdef __OS2__
 typedef int    socklen_t;
 #define SHUT_RDWR 0x2
#endif

#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
  typedef long   in_addr_t;
#  define inet_lnaof(x) inet_addr(inet_ntoa(x))
  typedef int    socklen_t;
#endif

// Adjust for the constant size
const size_t NETBUFSIZE = 1448;	// 1500 appears to be the default size as used by FMS
//const size_t NETBUFSIZE = 1357*2;	// 1500 appears to be the default size as used by FMS

class DSOEXPORT Network {
public:
    typedef boost::uint8_t byte_t;

    Network();
    ~Network();
    
    // Create a new server. After creating it, then you have to wait
    // for an incoming connection.
    int createServer(void);
    int createServer(short port);
    
    // Accept a client connection for the current server.
    int newConnection(void);
    int newConnection(int fd);
    int newConnection(bool block, int fd);
    int newConnection(bool block);

    // Connect to a named pipe
    bool connectSocket(const std::string &sock);

    // Create a client connection to a tcp/ip server
    bool createClient(void);
    bool createClient(short port);
    bool createClient(const std::string &hostname);
    bool createClient(const std::string &hostname, short port);

    // Read from the connection
    amf::Buffer *readNet();
    int readNet(amf::Buffer *buffer);
    int readNet(amf::Buffer *buffer, int timeout);
    int readNet(byte_t *buffer, int nbytes);
    int readNet(byte_t *buffer, int nbytes, int timeout);
    int readNet(int fd, byte_t *buffer, int nbytes);
    int readNet(int fd, byte_t *buffer, int nbytes, int timeout);
    
    // Write to the connection
    int writeNet(amf::Buffer *buffer);
    int writeNet(const std::string &buffer);
    int writeNet(const byte_t *buffer, int nbytes);
//    int writeNet(int fd, const byte_t *buffer);
    int writeNet(int fd, const byte_t *buffer, int nbytes);
    int writeNet(int fd, const byte_t *buffer, int nbytes, int timeout);
    
    // Close the connection
    bool closeNet();
    bool closeNet(int fd);
    bool closeConnection();
    bool closeConnection(int fd);

    // Change the debug flag
    void toggleDebug(bool val);
    bool netDebug() { return _debug; };
    
    bool send(const char *str);

    // Accessors for testing
    bool connected()           
    {
        assert ( ( _connected && _sockfd > 0 ) || ( ! _connected && _sockfd <= 0 ) );
        return _connected;
    };

    void setPort(short x) { _port = x; };
    short getPort() const { return _port; };
    void setFileFd(int x) { _sockfd = x; };
    int getFileFd() const { return _sockfd; };
    int getListenFd() const { return _listenfd; };
    void setListenFd(int x) { _listenfd = x; };
    const std::string& getURL() const { return _url; }
    const std::string& getProtocol() const  { return _protocol; }
    const std::string& getHost() const { return _host; }
    const std::string& getPortStr() const { return _portstr; }
    const std::string& getPath() const { return _path; }
    int getTimeout() const { return _timeout; }

    // Network is not copiable !
    //Network &operator = (Network &net) {}

 protected:
    in_addr_t   _ipaddr;
    int         _sockfd;	// the file descriptor used for reading and writing
    int         _listenfd;	// the file descriptor used to listen for new connections
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

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
