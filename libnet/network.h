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
# include <sys/select.h>
#ifdef HAVE_POLL
# include <poll.h>
#else 
# ifdef HAVE_EPOLL
#  include <epoll.h>
# endif
#endif
#else
# include <winsock2.h>
# include <windows.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <io.h>
#endif

#include "dsodefs.h" //For DSOEXPORT.
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <vector>
#include <cassert>
#include <string>
#include <map>

namespace amf {
class Buffer;
}

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
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

/// \class Network
///	This is a low level network class for Gnash and Cygnal. This
///	handles the grunt work on both the client side and the server
///	side of a network connection.
class DSOEXPORT Network {
public:
    typedef boost::uint8_t byte_t;

    Network();
    ~Network();
    
    /// \brief Create a new server.
    ///		After creating it, then you have to wait for an
    ///		incoming connection.
    ///
    /// @param port The optional port number to wait on for
    ///		connections.
    ///
    /// @return The file descritor to wait for connections on.
    int createServer(void);
    int createServer(short port);
    
    /// \brief Accept a client connection for the current server.
    ///
    /// @param fd The optional file descriptor to wait on for
    ///		connections.
    ///
    /// @param block True if this should be a blocking wait.
    ///
    /// @return The file descriptor of the new connection.
    int newConnection(void);
    int newConnection(int fd);
    int newConnection(bool block, int fd);
    int newConnection(bool block);

    /// \brief Connect to a named pipe
    ///
    ///
    /// @param sock The name of the named pipe to connect to.
    ///
    /// @return True if the connect suceeded, false if it failed.
    bool connectSocket(const std::string &sock);

    /// \brief Create a client connection to a tcp/ip server.
    ///
    /// @param port The tcp/ip port to use for contacting the server.
    ///
    /// @param hostname The name of the host to connect to. The
    ///		default is localhost.
    ///
    /// @return True if the connect suceeded, false if it failed.
    bool createClient(void);
    bool createClient(short port);
    bool createClient(const std::string &hostname);
    bool createClient(const std::string &hostname, short port);

    /// \brief Read from the opened connection.
    ///
    /// @param buffer A Buffer class to hold the data.
    ///
    /// @param timeout An optional timeout for reading data, in seconds.
    ///
    /// @param fd The optional file descriptor to read data from.
    ///
    /// @param nbytes The number of bytes to try to read.
    ///
    /// @return The number of bytes read.
    boost::shared_ptr<amf::Buffer> readNet();
    int readNet(boost::shared_ptr<amf::Buffer> buffer);
    int readNet(int fd, boost::shared_ptr<amf::Buffer> buffer);
    int readNet(boost::shared_ptr<amf::Buffer> buffer, int timeout);
    int readNet(int fd, boost::shared_ptr<amf::Buffer> buffer, int timeout);
    int readNet(byte_t *data, int nbytes);
    int readNet(byte_t *data, int nbytes, int timeout);
    int readNet(int fd, byte_t *data, int nbytes);
    int readNet(int fd, byte_t *data, int nbytes, int timeout);
    
    /// \brief Write to the opened connection.
    ///
    /// @param buffer A Buffer class holding the data.
    ///
    /// @param timeout An optional timeout for writing data, in seconds.
    ///
    /// @param fd The optional file descriptor to write data to from.
    ///
    /// @param nbytes The number of bytes to try to write..
    ///
    /// @return The number of bytes read.
    int writeNet(amf::Buffer *buffer);
    int writeNet(const std::string &data);
    int writeNet(const byte_t *data, int nbytes);
//    int writeNet(int fd, const byte_t *buffer);
    int writeNet(int fd, const byte_t *buffer, int nbytes);
    int writeNet(int fd, const byte_t *buffer, int nbytes, int timeout);

#ifdef HAVE_POLL
    boost::shared_ptr<std::vector<struct pollfd> > waitForNetData(int limit, struct pollfd *fds);
#endif
    fd_set waitForNetData(int limit, fd_set data);
    fd_set waitForNetData(std::vector<int> &data);
	
    /// \brief Close the connection
    ///
    /// @param fd The file descritor of the open connection to close.
    ///
    /// @return True if the connection was closed, false if it failed.
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
    void setTimeout(int x) { _timeout = x; }
    int getTimeout() const { return _timeout; }

    Network &operator = (Network &net);

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
