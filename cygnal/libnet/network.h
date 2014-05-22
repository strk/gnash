// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
# include <sys/socket.h>
# include <netdb.h>
#ifdef HAVE_POLL_H
# include <poll.h>
#else 
# ifdef HAVE_EPOLL_H
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

#include <memory>
#include <cstdint>
#include <mutex>
#include <vector>
#include <cassert>
#include <string>
#include <map>

#ifdef USE_SSH
# include "sshclient.h"
#endif

#ifdef USE_SSL
# include "sslclient.h"
#endif

#include "dsodefs.h" //For DSOEXPORT.

namespace cygnal {
class Buffer;
}

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

// forward declare the encryption protocols
class SSLClient;
class SSHClient;

// Define the default ports
const short SSL_PORT    = 443;
const short SSH_PORT    = 22;
const short HTTP_PORT   = 80;

// Delay Tolerant Networking Research Group, http://www.dtnrg.org
const short DTN1_PORT   = 2445;
const short DTN2_PORT   = 4556;

// Define the ports for the RTMP protocols
const short ADMIN_PORT  = 1111;
const short RTMP_PORT   = 1935;
const short RTMPE_PORT  = 1935;
const short RTMPT_PORT  = HTTP_PORT;
const short RTMPTE_PORT = HTTP_PORT;
const short RTMPTS_PORT = SSL_PORT;
const short CGIBIN_PORT = 1234;

#ifdef __OS2__
 typedef int    socklen_t;
 #define SHUT_RDWR 0x2
#endif

#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
  typedef long   in_addr_t;
#  define inet_lnaof(x) inet_addr(inet_ntoa(x))
  typedef int    socklen_t;
#endif

#if defined(HAVE_POLL_H) || defined(HAVE_PPOLL)
#include <poll.h>
#else
struct pollfd {
    int   fd; 
    short events;
    short revents;
};
#endif

/// \class Network
///	This is a low level network class for Gnash and Cygnal. This
///	handles the grunt work on both the client side and the server
///	side of a network connection.
class DSOEXPORT Network {
public:
    /// This enum contains the list of all supported protocols.
    typedef enum {
	NONE,
	HTTP,
	HTTPS,
	RTMP,
	RTMPT,
	RTMPTS,
	RTMPE,
	RTMPS,
	DTN
    } protocols_supported_e;
    // This is used to pass parameters to a thread using std::bind
    typedef struct {
	int tid;
	int port;
	int netfd;
	void *entry;
	void *handler;
	cygnal::Buffer *buffer;
	std::string filespec;
        std::string hostname;
	protocols_supported_e protocol;
    } thread_params_t;
    typedef std::uint8_t byte_t;
    typedef bool entry_t (thread_params_t *);

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
    int createServer(std::string hostname, short port);
    
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
    std::shared_ptr<cygnal::Buffer> readNet();
    int readNet(cygnal::Buffer &buffer);
    int readNet(int fd, cygnal::Buffer &buffer);
    int readNet(int fd, cygnal::Buffer *buffer);
    int readNet(cygnal::Buffer &buffer, int timeout);
    int readNet(int fd, cygnal::Buffer &buffer, int timeout);
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
    int writeNet(cygnal::Buffer *buffer);
    int writeNet(cygnal::Buffer &buffer);
    int writeNet(int fd, cygnal::Buffer *buffer);
    int writeNet(int fd, cygnal::Buffer &buffer);
    int writeNet(const std::string &data);
    int writeNet(const byte_t *data, int nbytes);
//    int writeNet(int fd, const byte_t *buffer);
    int writeNet(int fd, const byte_t *buffer, int nbytes);
    int writeNet(int fd, const byte_t *buffer, int nbytes, int timeout);
    
    /// \brief Wait for sries of file descriptors for data.
    ///
    /// @param limit The max number of file descriptors to wait for.
    ///
    /// @return A vector of the file descriptors that have activity.
    std::shared_ptr<std::vector<struct pollfd> > waitForNetData(int limit, struct pollfd *fds);
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
    bool connected() const
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
    void setURL(const std::string& url) { _url = url; }

    const std::string& getProtocol() const  { return _protocol; }
    void setProtocol(const std::string& proto) { _protocol = proto; }

    const std::string& getHost() const { return _host; }
    void setHost(const std::string& host) { _host = host; }

    const std::string& getPortStr() const { return _portstr; }
    void setPortStr(const std::string& port) { _portstr = port; }

    const std::string& getPath() const { return _path; }
    void setPath(const std::string& path) { _path = path; }

    void setTimeout(int x) { _timeout = x; }
    int getTimeout() const { return _timeout; }

    Network &operator = (Network &net);

    // The pollfd are an array of data structures used by the poll()
    // system call. We have to keep track of these as network
    // connections get added and disconnected.
    void addPollFD(struct pollfd &fd, entry_t *ptr);
    void addPollFD(struct pollfd &fd);
    void erasePollFD(int fd);
    void erasePollFD(std::vector<struct pollfd>::iterator &itt);
    struct pollfd &getPollFD(int fd);
    struct pollfd *getPollFDPtr();
#ifdef HAVE_POLL_H
    size_t getPollFDSize() { return _pollfds.size(); };
    void clearPollFD() { _pollfds.clear(); };
#endif

    // The entry point is an function pointer, which is the event
    // handler when there is data on a file descriptor.
    void addEntry(int fd, entry_t *func);
    entry_t *getEntry(int fd);
    
//    void executePollFD(int index) { _handler[index](); ];

#ifdef USE_SSL
    bool initSSL(std::string &hostname);
    bool initSSL(std::string &hostname, std::string &password);
    bool initSSL(std::string &hostname, std::string &password, bool auth);
    bool initSSL(std::string &hostname, std::string &password, 
		 std::string &keyfile, std::string &calist,
		 std::string &rootpath, bool auth);
#endif

    // Use an ioctl() to see how many bytes are in the network buffers.
    size_t sniffBytesReady(int fd);
    
 protected:
    // Return the string representation of the IPV4 or IPV6 number
    std::shared_ptr<char> getIPString(struct addrinfo *ai);

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
    size_t	_bytes_loaded;
    /// \var Handler::_handlers
    ///		Keep a list of all active network connections
    std::map<int, entry_t *> _handlers;
    std::vector<struct pollfd> _pollfds;
    // This is the mutex that controls access to the que.
    std::mutex	_poll_mutex;
    std::mutex	_net_mutex;
#ifdef USE_SSL
    std::unique_ptr<SSLClient> _ssl;
#endif
#ifdef USE_SSH
    std::unique_ptr<SSHClient> _ssh;
#endif
};

} // end of gnash namespace

// __NETWORK_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
