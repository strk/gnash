//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
//xs

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "utility.h"
#include "log.h"
#include "network.h"

#include <sys/types.h>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include <ws2tcpip.h>
#else
# include <sys/ioctl.h>
# include <sys/time.h>
# include <unistd.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netdb.h>
# include <sys/param.h>
# include <sys/select.h>
#include <csignal>

// This is for non-standard signal functions such as sigemptyset.
#include <signal.h>

#ifdef HAVE_POLL_H
# include <poll.h>
#else 
# ifdef HAVE_EPOLL_H
#  include <epoll.h>
# endif
#endif
#endif

#include "buffer.h"
#include "GnashException.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

#ifndef FIONREAD
#define FIONREAD 0
#endif

using std::string;
using std::vector;

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

static const char *DEFAULTPROTO = "tcp";
static const short DEFAULTPORT  = RTMP_PORT;

#ifndef INADDR_NONE
#define INADDR_NONE  0xffffffff
#endif

static void cntrlc_handler(int sig);
// this is set when we get a signal during a pselect() or ppoll()
static int  sig_number = 0;

Network::Network()
	:
	_ipaddr(INADDR_ANY),
	_sockfd(0),
	_listenfd(0),
	_port(0),
	_connected(false),
	_debug(false),
	_timeout(0)
{
//    GNASH_REPORT_FUNCTION;
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(1, 1);		// Windows Sockets 1.1
    if (WSAStartup( wVersionRequested, &wsaData ) != 0) {
        log_error(_("Could not find a usable WinSock DLL"));
        exit(EXIT_FAILURE);
    }
#endif

}

Network::~Network()
{
//    GNASH_REPORT_FUNCTION;
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
    WSACleanup();
#else
    closeNet();
#endif
}

// Description: Create a tcp/ip network server. This creates a server
//              that listens for incoming socket connections. This
//              supports IP aliasing on the host, and will sequntially
//              look for IP address to bind this port to.
int
Network::createServer(void)
{
//    GNASH_REPORT_FUNCTION;

    short port;

    if (_port) {
	port = _port;
    } else {
	port = DEFAULTPORT;
    }
    return createServer(port);
}

// FIXME: Should also support IPv6 (AF_INET6)
int
Network::createServer(short port)
{
//    GNASH_REPORT_FUNCTION;

    struct protoent *ppe;
    struct sockaddr_in sock_in;
    int             on, type;
    int             retries = 0;

    if (_listenfd >= 2) {
	log_debug(_("already connected to port %hd"), port);
	return _listenfd;
    }
    
    const struct hostent *host = gethostbyname("localhost");
    struct in_addr *thisaddr = reinterpret_cast<struct in_addr *>(host->h_addr_list[0]);
    _ipaddr = thisaddr->s_addr;
    memset(&sock_in, 0, sizeof(sock_in));

#if 0
    // Accept incoming connections only on our IP number
    sock_in.sin_addr.s_addr = thisaddr->s_addr;
#else
    // Accept incoming connections on any IP number
    sock_in.sin_addr.s_addr = INADDR_ANY;
#endif

    _ipaddr = sock_in.sin_addr.s_addr;
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(port);

    if ((ppe = getprotobyname(DEFAULTPROTO)) == 0) {
        log_error(_("unable to get protocol entry for %s"),
                DEFAULTPROTO);
        return -1;
    }

    // set protocol type
    if ( strcmp(DEFAULTPROTO, "udp") == 0) {
        type = SOCK_DGRAM;
    } else {
        type = SOCK_STREAM;
    }

    // Get a file descriptor for this socket connection
    _listenfd = socket(PF_INET, type, ppe->p_proto);

    // error, wasn't able to create a socket
    if (_listenfd < 0) {
        log_error(_("unable to create socket: %s"), strerror(errno));
        return -1;
    }

    on = 1;
    if (setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&on, sizeof(on)) < 0) {
        log_error(_("setsockopt SO_REUSEADDR failed"));
        return -1;
    }

    retries = 0;

//     in_addr_t       nodeaddr;
//     nodeaddr = inet_lnaof(*thisaddr);
    while (retries < 5) {
        if (bind(_listenfd, reinterpret_cast<struct sockaddr *>(&sock_in),
                 sizeof(sock_in)) == -1) {
            log_error(_("unable to bind to port %hd: %s"),
                    port, strerror(errno));
//                    inet_ntoa(sock_in.sin_addr), strerror(errno));
            retries++;
        }

	if (_debug) {
//		char  ascip[INET_ADDRSTRLEN];
//		inet_ntop(sock_in.sin_family, &_ipaddr, ascip, INET_ADDRSTRLEN);
		char *ascip = ::inet_ntoa(sock_in.sin_addr);
		log_debug(_("Server bound to service on %s, port %hd, using fd #%d"),
		    ascip, ntohs(sock_in.sin_port),
		    _listenfd);
	}

        if (type == SOCK_STREAM && listen(_listenfd, 5) < 0) {
            log_error(_("unable to listen on port: %hd: %s "),
                port, strerror(errno));
            return -1;
        }

	// We have a socket created
        _port = port;
        return _listenfd;
    }
    return -1;
}

// Description: Accept a new network connection for the port we have
//              created a server for.
// The default is to block.
int
Network::newConnection(void)
{
//    GNASH_REPORT_FUNCTION;

    return newConnection(true, _listenfd);
}

int
Network::newConnection(int fd)
{
//    GNASH_REPORT_FUNCTION;

    return newConnection(true, fd);
}

int
Network::newConnection(bool block)
{
//    GNASH_REPORT_FUNCTION;

    return newConnection(block, _listenfd);
}

int
Network::newConnection(bool block, int fd)
{
//    GNASH_REPORT_FUNCTION;

    struct sockaddr	newfsin;
    socklen_t		alen;
    int			ret;
    fd_set                fdset;
    int                   retries = 3;

    alen = sizeof(struct sockaddr_in);

    if (fd <= 2) {
        return -1;
    }
    if (_debug) {
	log_debug(_("Waiting to accept net traffic on fd #%d for port %d"), fd, _port);
    }

#ifdef HAVE_PSELECT
	struct timespec tval;
	sigset_t sigset, blockset, pending;
	sigemptyset(&blockset);        
//        sigaddset(&blockset, SIGINT); /* Block SIGINT */
        sigaddset(&blockset, SIGPIPE); /* Block SIGPIPE */
	sigprocmask(SIG_BLOCK, &blockset, &sigset);
#else
	struct timeval tval;
#endif
	
    while (retries--) {
        // We use select to wait for the read file descriptor to be
        // active, which means there is a client waiting to connect.
        FD_ZERO(&fdset);
        // also return on any input from stdin
//         if (_console) {
//             FD_SET(fileno(stdin), &fdset);
//         }
        FD_SET(fd, &fdset);

        // Reset the timeout value, since select modifies it on return. To
        // block, set the timeout to zero.
#ifdef HAVE_PSELECT
	tval.tv_sec = _timeout;
	tval.tv_nsec = 0;
        if (block) {
	    ret = pselect(fd+1, &fdset, NULL, NULL, NULL, &blockset);
	} else {
	    ret = pselect(fd+1, &fdset, NULL, NULL, &tval, &blockset);
	}
	if (sig_number) {
	    log_debug(_("Have a SIGINT interupt waiting!"));
	}
	sigpending(&pending);
	if (sigismember(&pending, SIGINT)) {
	    log_debug(_("Have a pending SIGINT interupt waiting!"));
	    int sig;
	    sigwait(&blockset, &sig);
	}
	if (sigismember(&pending, SIGPIPE)) {
	    log_debug(_("Have a pending SIGPIPE interupt waiting!"));
	    int sig;
	    sigwait(&blockset, &sig);
	}
#else
        tval.tv_sec = 1;
        tval.tv_usec = 0;
        if (block) {
            ret = select(fd+1, &fdset, NULL, NULL, NULL);
        } else {
            ret = select(fd+1, &fdset, NULL, NULL, &tval);
        }
#endif

        if (FD_ISSET(0, &fdset)) {
	    if (_debug) {
		log_debug(_("There is a new network connection request."));
	    }
            return 1;
        }
	
        // If interrupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
            log_debug(_("The accept() socket for fd #%d was interrupted by a system call"), fd);
        }

        if (ret == -1) {
            log_debug(_("The accept() socket for fd #%d never was available"), fd);
            return -1;
        }

        if (ret == 0) {
            if (_debug) {
                log_debug(_("The accept() socket for fd #%d timed out waitingfor data"), fd);
		return 0;
            }
        }
	
    }

#ifndef HAVE_WINSOCK_H
    fcntl(_listenfd, F_SETFL, O_NONBLOCK); // Don't let accept() block
#endif
    _sockfd = accept(fd, &newfsin, &alen);

    if (_sockfd < 0) {
        log_error(_("unable to accept: %s"), strerror(errno));
        return -1;
    }

    if (_debug) {
	log_debug(_("Accepting tcp/ip connection on fd #%d for port %d"), _sockfd, _port);
    }

    return _sockfd;
}

#if defined(_WIN32) || defined(__amigaos4__)
/* from sys/socket.h */
typedef unsigned short      sa_family_t;

/* from sys/un.h */
#define UNIX_PATH_MAX   108

struct sockaddr_un {
    sa_family_t sun_family; /* AF_UNIX */
    char sun_path[UNIX_PATH_MAX];   /* pathname */
};

#endif /* _WIN32 */

// Connect to a named pipe
bool
Network::connectSocket(const string &sockname)
{
//    GNASH_REPORT_FUNCTION;

    struct sockaddr_un  addr;
    fd_set              fdset;
    struct timeval      tval;
    int                 ret;
    int                 retries;

    addr.sun_family = AF_UNIX;
    // socket names must be 108 bytes or less as specifiec in sys/un.h.
    strncpy(addr.sun_path, sockname.c_str(), 100);

    _sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (_sockfd < 0) {
	log_error(_("unable to create socket: %s"), strerror(errno));
	_sockfd = -1;
	return false;
    }
    
    retries = 2;
    while (retries-- > 0) {
        // We use select to wait for the read file descriptor to be
        // active, which means there is a client waiting to connect.
        FD_ZERO(&fdset);
        FD_SET(_sockfd, &fdset);

        // Reset the timeout value, since select modifies it on return. To
        // block, set the timeout to zero.
        tval.tv_sec = 5;
        tval.tv_usec = 0;

        ret = ::select(_sockfd+1, &fdset, NULL, NULL, &tval);

        // If interrupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
	    log_debug(_("The connect() socket for fd %d was interrupted by a system call"),
		      _sockfd);
	    continue;
	}

        if (ret == -1) {
	    log_debug(_("The connect() socket for fd %d never was available for writing"),
		      _sockfd);
#ifdef HAVE_WINSOCK_H
	    ::shutdown(_sockfd, 0); // FIXME: was SHUT_BOTH
#else
	    ::shutdown(_sockfd, SHUT_RDWR);
#endif
	    _sockfd = -1;
	    return false;
	}
        if (ret == 0) {
            log_error(_("The connect() socket for fd %d timed out waiting to write"),
                      _sockfd);
            continue;
        }

        if (ret > 0) {
            ret = ::connect(_sockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
            if (ret == 0) {
                log_debug(_("\tsocket name %s for fd %d"), sockname, _sockfd);
                _connected = true;
                assert(_sockfd > 0);
                return true;
            }
            if (ret == -1) {
                log_error(_("The connect() socket for fd %d never was available for writing"),
                        _sockfd);
                _sockfd = -1;
                assert(!_connected);
                return false;
            }
        }
    }
    

#ifndef HAVE_WINSOCK_H
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
#endif

    _connected = true;
    assert(_sockfd > 0);
    return true;    
}

// Create a client connection to a tcp/ip based service
bool
Network::createClient(void)
{
//    GNASH_REPORT_FUNCTION;

    short port;

    if (_port) {
	port = _port;
    } else {
	port = RTMP_PORT;
    }
    return createClient("localhost", port);
}
bool
Network::createClient(short /* port */)
{
//    GNASH_REPORT_FUNCTION;

    return false;
}

bool
Network::createClient(const string &hostname)
{
//    GNASH_REPORT_FUNCTION;

    short port;

    if (_port) {
	port = _port;
    } else {
	port = RTMP_PORT;
    }
    return createClient(hostname, port);
}

bool
Network::createClient(const string &hostname, short port)
{
//    GNASH_REPORT_FUNCTION;

    struct sockaddr_in  sock_in;
    fd_set              fdset;
    struct timeval      tval;
    int                 ret;
    int                 retries;
    char                thishostname[MAXHOSTNAMELEN];
    struct protoent     *proto;

//    assert( ! connected() );
    if (connected()) {
        return true;
    }

    _port = port;    
    log_debug(_("%s: to host %s at port %d"), __FUNCTION__, hostname, port);

    memset(&sock_in, 0, sizeof(struct sockaddr_in));
    memset(&thishostname, 0, MAXHOSTNAMELEN);
    if (hostname.size() == 0) {
        if (gethostname(thishostname, MAXHOSTNAMELEN) == 0) {
            log_debug(_("The hostname for this machine is %s"), thishostname);
        } else {
            log_debug(_("Couldn't get the hostname for this machine"));
            return false;
        }
    }

    const struct hostent *hent = ::gethostbyname(hostname.c_str());
    if (hent > 0) {
        ::memcpy(&sock_in.sin_addr, hent->h_addr, hent->h_length);
    }
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = ntohs(static_cast<short>(port));

#if 0
    char ascip[INET_ADDRSTRLEN];
    inet_ntop(sock_in.sin_family, &sock_in.sin_addr.s_addr, ascip, INET_ADDRSTRLEN);
    log_debug(_("The IP address for this client socket is %s"), ascip);
#endif

    proto = ::getprotobyname("TCP");

    _sockfd = ::socket(PF_INET, SOCK_STREAM, proto->p_proto);
    if (_sockfd < 0) {
        log_error(_("unable to create socket: %s"), strerror(errno));
        _sockfd = -1;
        return false;
    }

    retries = 2;
    while (retries-- > 0) {
        // We use select to wait for the read file descriptor to be
        // active, which means there is a client waiting to connect.
        FD_ZERO(&fdset);
        FD_SET(_sockfd, &fdset);

        // Reset the timeout value, since select modifies it on return. To
        // block, set the timeout to zero.
        tval.tv_sec = 5;
        tval.tv_usec = 0;

        ret = ::select(_sockfd+1, &fdset, NULL, NULL, &tval);

        // If interrupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
            log_debug(_("The connect() socket for fd %d was interrupted "
                        "by a system call"), _sockfd);
            continue;
        }

        if (ret == -1) {
            log_debug(_("The connect() socket for fd %d never was "
                        "available for writing"), _sockfd);
#ifdef HAVE_WINSOCK_H
            ::shutdown(_sockfd, 0); // FIXME: was SHUT_BOTH
#else
            ::shutdown(_sockfd, SHUT_RDWR);
#endif
	    ::close(_sockfd);
            _sockfd = -1;
            return false;
        }

        if (ret == 0) {
#ifdef HAVE_WINSOCK_H
            ::shutdown(_sockfd, 0); // FIXME: was SHUT_BOTH
#else
            ::shutdown(_sockfd, SHUT_RDWR);
#endif
            log_error(_("The connect() socket for fd %d timed out waiting "
                        "to write"), _sockfd);
	    ::close(_sockfd);
            continue;
        }

        if (ret > 0) {
            ret = ::connect(_sockfd, 
                    reinterpret_cast<struct sockaddr *>(&sock_in),
                    sizeof(sock_in));

            if (ret == 0) {
                char *ascip = ::inet_ntoa(sock_in.sin_addr);
        // 		char ascip[INET_ADDRSTRLEN];
        // 		inet_ntop(sock_in.sin_family, &sock_in.sin_addr.s_addr, ascip, INET_ADDRSTRLEN);
                log_debug(_("\tport %d at IP %s for fd %d"), port,
                        ascip, _sockfd);
                _connected = true;
                assert(_sockfd > 0);
                return true;
            }
            if (ret == -1) {
                log_error(_("The connect() socket for fd %d never was "
                            "available for writing"), _sockfd);
#ifdef HAVE_WINSOCK_H
		::shutdown(_sockfd, 0); // FIXME: was SHUT_BOTH
#else
		::shutdown(_sockfd, SHUT_RDWR);
#endif
		::close(_sockfd);
                _sockfd = -1;
                assert(!_connected);
                return false;
            }
        }
    }
    //  ::close(_sockfd);
    //  return false;

    printf("\tConnected at port %d on IP %s for fd #%d", port,
           ::inet_ntoa(sock_in.sin_addr), _sockfd);

#ifndef HAVE_WINSOCK_H
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
#endif

    _connected = true;
    _port = port;
    assert(_sockfd > 0);
    return true;
}

bool
Network::closeNet()
{
//    GNASH_REPORT_FUNCTION;

    if ((_sockfd > 0) && (_connected)) {
        closeNet(_sockfd);
        _sockfd = 0;
        _connected = false;
    }

    return false;
}

bool
Network::closeNet(int sockfd)
{
//    GNASH_REPORT_FUNCTION;

    int retries = 0;

    // If we can't close the socket, other processes must be
    // locked on it, so we wait a second, and try again. After a
    // few tries, we give up, cause there must be something
    // wrong.

    if (sockfd <= 0) {
        return true;
    }

    while (retries < 3) {
        if (sockfd) {
            // Shutdown the socket connection
#if 0
            if (shutdown(sockfd, SHUT_RDWR) < 0) {
                if (errno != ENOTCONN) {
                    cerr << "WARNING: Unable to shutdown socket for fd #"
                         << sockfd << strerror(errno) << endl;
                } else {
                    cerr << "The socket using fd #" << sockfd
                         << " has been shut down successfully." << endl;
                    return true;
                }
            }
#endif
            if (::close(sockfd) < 0) {
		// If we have a bad file descriptor, it's because
		// this got closed already, usually by another
		// thread being paranoid.
		if (errno != EBADF) {
		    log_error(_("Unable to close the socket for fd #%d: %s"),
			      sockfd, strerror(errno));
		}
#ifndef HAVE_WINSOCK_H
                sleep(1);
#endif
                retries++;
            } else {
		log_debug(_("Closed the socket on fd #%d"), sockfd);
                return true;
            }
        }
    }
    return false;
}
// Description: Close an open socket connection.
bool
Network::closeConnection(void)
{
//    GNASH_REPORT_FUNCTION;

    closeConnection(_sockfd);
    _sockfd = 0;
    closeConnection(_listenfd);
    _listenfd = 0;
    _connected = false;

    return false;
}

bool
Network::closeConnection(int fd)
{
//    GNASH_REPORT_FUNCTION;

    if (fd > 0) {
        ::close(fd);
	log_debug(_("%s: Closed fd #%d"), __FUNCTION__, fd);
//        closeNet(fd);
    }

    return false;
}

boost::shared_ptr<cygnal::Buffer>
Network::readNet()
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Buffer> buffer(new cygnal::Buffer);
    int ret = readNet(*buffer);
    if (ret > 0) {
	buffer->resize(ret);
    }
    return buffer;
}

// Read from the connection
int
Network::readNet(int fd, cygnal::Buffer &buffer)
{
//    GNASH_REPORT_FUNCTION;
    int ret = readNet(fd, buffer.reference(), buffer.size(), _timeout);
    if (ret > 0) {
	buffer.resize(ret);
    }

    return ret;
}

int
Network::readNet(int fd, cygnal::Buffer *buffer)
{
//    GNASH_REPORT_FUNCTION;
    int ret = readNet(fd, buffer->reference(), buffer->size(), _timeout);
    if (ret > 0) {
	buffer->setSeekPointer(buffer->reference() + ret);
    }

    return ret;
}

int
Network::readNet(cygnal::Buffer &buffer)
{
//    GNASH_REPORT_FUNCTION;
    int ret = readNet(_sockfd, buffer, _timeout);

    return ret;
}

int
Network::readNet(cygnal::Buffer &buffer, int timeout)
{
//    GNASH_REPORT_FUNCTION;
    int ret = readNet(_sockfd, buffer.reference(), buffer.size(), timeout);
    if (ret > 0) {
	buffer.resize(ret);	// FIXME: why does this corrupt
    }

    return ret;
}

int
Network::readNet(int fd, cygnal::Buffer &buffer, int timeout)
{
    // GNASH_REPORT_FUNCTION;
    int ret = readNet(fd, buffer.reference(), buffer.size(), timeout);
    buffer.setSeekPointer(ret);
#if 0
    if (ret > 0) {
	buffer.resize(ret);	// FIXME: why does this corrupt
    }
#endif
    
    return ret;
}

int
Network::readNet(byte_t *data, int nbytes)
{
//    GNASH_REPORT_FUNCTION;
    return readNet(_sockfd, data, nbytes, _timeout);
}

int
Network::readNet(byte_t *data, int nbytes, int timeout)
{
//    GNASH_REPORT_FUNCTION;
    return readNet(_sockfd, data, nbytes, timeout);
}

int
Network::readNet(int fd, byte_t *data, int nbytes)
{
//    GNASH_REPORT_FUNCTION;
    return readNet(fd, data, nbytes, _timeout);
}

int
Network::readNet(int fd, byte_t *buffer, int nbytes, int timeout)
{
//     GNASH_REPORT_FUNCTION;

    fd_set              fdset;
    int                 ret = -1;

//     boost::mutex::scoped_lock lock(_net_mutex);

    if (_debug) {
	log_debug(_("Trying to read %d bytes from fd #%d"), nbytes, fd);
    }
#ifdef NET_TIMING
    if (_timing_debug)
    {
        gettimeofday(&tp, NULL);
        read_start_time = static_cast<double>(tp.tv_sec)
            + static_cast<double>(tp.tv_usec*1e-6);
    }
#endif
    if (fd > 2) {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

#ifdef HAVE_PSELECT
	struct timespec tval;
	sigset_t pending, blockset;
	sigemptyset(&blockset);        
        // sigaddset(&blockset, SIGINT); /* Block SIGINT */
//        sigaddset(&blockset, SIGPIPE); /* Block SIGPIPE */
        sigprocmask(SIG_BLOCK, &blockset, NULL);

	// Trap ^C (SIGINT) so we can kill all the threads
// 	struct sigaction  act;
// 	act.sa_handler = cntrlc_handler;
// 	act.sa_flags = 0;
// 	sigemptyset(&act.sa_mask);
// 	sigaction (SIGINT, &act, NULL);
#else
	struct timeval tval;
#endif
        if (timeout == 0) {
#ifdef HAVE_PSELECT
	    ret = pselect(fd+1, &fdset, NULL, NULL, NULL, &blockset);
#else
	    ret = select(fd+1, &fdset, NULL, NULL, NULL);
#endif
	} else {	
#ifdef HAVE_PSELECT
	    tval.tv_sec = timeout;
	    tval.tv_nsec = 0;
	    ret = pselect(fd+1, &fdset, NULL, NULL, &tval, &blockset);
	    sigpending(&pending);
	    if (sigismember(&pending, SIGINT)) {
		log_debug(_("Have a pending SIGINT interupt waiting!"));
		int sig;
		sigwait(&blockset, &sig);
		cntrlc_handler(SIGINT);
	    }
	    if (sigismember(&pending, SIGPIPE)) {
		log_debug(_("Have a pending SIGPIPE interupt waiting!"));
		int sig;
		sigwait(&blockset, &sig);
		cntrlc_handler(SIGINT);
	    }
#else
	    tval.tv_sec = timeout;
	    tval.tv_usec = 0;
	    ret = select(fd+1, &fdset, NULL, NULL, &tval);
#endif
	}

        // If interrupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
            log_error(_("The socket for fd #%d was interrupted by a system call"), fd);
        }

        if (ret == -1) {
            log_error(_("The socket for fd #%d was never available for reading"), fd);
            return -1;
        }

        if (ret == 0) {
	    if (_debug) {
		log_debug(_("The socket for #fd %d timed out waiting to read"), fd);
	    }
            return 0;
        }

#ifdef USE_SSL
	if (_ssl) {
	    ret = _ssl->sslRead(buffer, nbytes);
	} else {
	    ret = read(fd, buffer, nbytes);
	}
#else
	ret = read(fd, buffer, nbytes);
#endif

	// If we read zero bytes, the network may be closed, as we returned from the select()
        if (ret == -1) {
            log_error(_("The socket for fd #%d was never available for reading data"), fd);
            return -1;
        }

        if (ret == 0) {
	    if (_debug) {
		log_debug (_("The socket for #fd %d timed out waiting to read data"), fd);
	    }
            return 0;
        }
	
	if (_debug) {
	    log_debug (_("read %d bytes from fd #%d from port %d"), ret, fd, _port);
	}
#if 0
	if (ret) {
	    log_debug (_("%s: Read packet data from fd #%d (%d bytes): \n%s"),
		       __FUNCTION__, fd, ret, hexify(buffer, ret, true));
	}
#endif    
    }

    return ret;

}

// Write to the connection
int
Network::writeNet(cygnal::Buffer *buffer)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(buffer->reference(), buffer->allocated());
}

int
Network::writeNet(int fd, cygnal::Buffer *buffer)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(fd, buffer->reference(), buffer->allocated());
}

// Write to the connection
int
Network::writeNet(cygnal::Buffer &buffer)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(buffer.reference(), buffer.allocated());
}

// Write to the connection
int
Network::writeNet(int fd, cygnal::Buffer &buffer)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(fd, buffer.reference(), buffer.allocated());
}

int
Network::writeNet(const std::string& data)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(reinterpret_cast<const byte_t *>(data.c_str()), data.size());
}

int
Network::writeNet(const byte_t *data, int nbytes)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(_sockfd, data, nbytes, _timeout);
}

// int
// Network::writeNet(const byte_t *buffer, int nbytes)
// {
//     return writeNet(_sockfd, buffer, nbytes, _timeout);
// }

// int
// Network::writeNet(int fd, const byte_t *buffer)
// {
//     return writeNet(fd, buffer, strlen(buffer), _timeout);
// }

int
Network::writeNet(int fd, const byte_t *data, int nbytes)
{
//     GNASH_REPORT_FUNCTION;
    return writeNet(fd, data, nbytes, _timeout);
}

int
Network::writeNet(int fd, const byte_t *buffer, int nbytes, int timeout)
{
//     GNASH_REPORT_FUNCTION;

    fd_set              fdset;
    int                 ret = -1;

    boost::mutex::scoped_lock lock(_net_mutex);
    
    // We need a writable, and not const point for byte arithmetic.
    byte_t *bufptr = const_cast<byte_t *>(buffer);

#ifdef NET_TIMING
    // If we are debugging the tcp/ip timings, get the initial time.
    if (_timing_debug)
    {
        gettimeofday(&starttime, 0);
    }
#endif
    if (fd > 2) {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

#ifdef HAVE_PSELECT
	struct timespec tval;
	sigset_t pending, blockset; //, emptyset;
	sigemptyset(&blockset);        
        // sigaddset(&blockset, SIGINT); /* Block SIGINT */
        sigaddset(&blockset, SIGPIPE);
        sigprocmask(SIG_BLOCK, &blockset, NULL);
#else
	struct timeval tval;
#endif
        // Reset the timeout value, since select modifies it on return
        if (timeout <= 0) {
            timeout = 5;
        }
#ifdef HAVE_PSELECT
	tval.tv_sec = timeout;
	tval.tv_nsec = 0;
	ret = pselect(fd+1, NULL, &fdset, NULL, &tval, &blockset);
	sigpending(&pending);
	if (sigismember(&pending, SIGINT)) {
	    log_debug(_("Have a pending SIGINT interupt waiting!"));
	    int sig;
	    sigwait(&blockset, &sig);
	    cntrlc_handler(SIGINT);
	}
#else
	tval.tv_sec = timeout;
        tval.tv_usec = 0;
        ret = select(fd+1, NULL, &fdset, NULL, &tval);
#endif
	
        // If interrupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
            log_error (_("The socket for fd #%d was interrupted by a system call"), fd);
        }

        if (ret == -1) {
            log_error (_("The socket for fd #%d was never available for writing"), fd);
        }

        if (ret == 0) {
            log_debug (_("The socket for fd #%d timed out waiting to write"), fd);
	    return 0;
        }

#ifdef USE_SSL
	if (_ssl) {
	    ret = _ssl->sslWrite(buffer, nbytes);
	} else {
	    ret = write(fd, bufptr, nbytes);
	}
#else
	ret = write(fd, bufptr, nbytes);
#endif
        if (ret == 0) {
            log_error (_("Wrote zero out of %d bytes to fd #%d: %s"), 
		nbytes, fd, strerror(errno));
            return ret;
        }
        if (ret < 0) {
            log_error (_("Couldn't write %d bytes to fd #%d: %s"), 
		nbytes, fd, strerror(errno));
            return ret;
        }
        if (ret > 0) {
            bufptr += ret;
            if (ret != nbytes) {
		if (_debug) {
		    log_debug (_("wrote %d bytes to fd #%d, expected %d"),
			       ret, fd, nbytes);
		}
            } else {
		if (_debug) {
		    log_debug (_("wrote %d bytes to fd #%d for port %d"),
			       ret, fd, _port);
		}
//                return ret;
            }
        }
#if 0
	if (ret) {
	    log_debug (_("%s: Wrote packet data to fd #%d: \n%s"),
		       __FUNCTION__, fd, hexify(buffer, ret, true));
	}
#endif    
    }

#ifdef NET_TIMING
    if (_timing_debug)
    {
        gettimeofday(&endtime, 0);

        if ((endtime.tv_sec - starttime.tv_sec) &&
            endtime.tv_usec - starttime.tv_usec)
        {
            log_debug (_("took %d usec to write (%d bytes)"),
		endtime.tv_usec - starttime.tv_usec, bytes_written);
        }
    }
#endif


    return ret;
}

void
Network::addPollFD(struct pollfd &fd, Network::entry_t *func)
{
//    GNASH_REPORT_FUNCTION;

    log_debug(_("%s: adding fd #%d to pollfds"), __PRETTY_FUNCTION__, fd.fd);
    boost::mutex::scoped_lock lock(_poll_mutex);
    _handlers[fd.fd] = func;
     _pollfds.push_back(fd);
//     notify();
}

void
Network::addPollFD(struct pollfd &fd)
{
//    GNASH_REPORT_FUNCTION;
    log_debug(_("%s: adding fd #%d to pollfds"), __PRETTY_FUNCTION__, fd.fd);
    boost::mutex::scoped_lock lock(_poll_mutex);
     _pollfds.push_back(fd);
//     notify();
}

struct pollfd
&Network::getPollFD(int index)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_poll_mutex);
    return _pollfds[index];
}

struct pollfd *
Network::getPollFDPtr()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_poll_mutex);
    return &_pollfds[0];
}

void
Network::erasePollFD(int fd)
{
//    GNASH_REPORT_FUNCTION;
    log_debug(_("%s: erasing fd #%d from pollfds"), __PRETTY_FUNCTION__, fd);
    boost::mutex::scoped_lock lock(_poll_mutex);
    if (_pollfds.size() > 0) {
	vector<struct pollfd>::iterator it;
	for (it=_pollfds.begin(); it<_pollfds.end(); it++) {
	    if ((*it).fd == fd) {
		_pollfds.erase(it);
		break;
	    }
	}
    }
}

void
Network::erasePollFD(vector<struct pollfd>::iterator &itt)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_poll_mutex);
    if (_pollfds.size() == 1) {
 	_pollfds.clear();
     } else {
	_pollfds.erase(itt);
    }
}

void
Network::addEntry(int fd, Network::entry_t *func)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_poll_mutex);
    _handlers[fd] = func;
}

Network::entry_t *
Network::getEntry(int fd)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_poll_mutex);
    return _handlers[fd];
}

boost::shared_ptr<std::vector<struct pollfd> >
Network::waitForNetData(int limit, struct pollfd *fds)
{
//    GNASH_REPORT_FUNCTION;

    boost::shared_ptr<vector<struct pollfd> > hits(new vector<struct pollfd>);

    log_debug(_("%s: waiting for %d fds"), __FUNCTION__, limit);

    if ((fds == 0) || (limit == 0)) {
	return hits;
    }
    
    int timeout = _timeout;
    if (timeout <= 0) {
	timeout = 5;
    }
#ifdef HAVE_PPOLL
	struct timespec tval;
	sigset_t pending, blockset;
	sigemptyset(&blockset);         /* Block SIGINT */
//        sigaddset(&blockset, SIGINT);
//        sigaddset(&blockset, SIGPIPE);
        sigprocmask(SIG_BLOCK, &blockset, NULL);

	tval.tv_sec = 5; // FIXME: was _timeout;
	tval.tv_nsec = 0;
	int ret = ppoll(fds, limit, &tval, &blockset);
	sigpending(&pending);
	if (sigismember(&pending, SIGINT)) {
	    log_debug(_("Have a pending SIGINT interupt waiting!"));
	    int sig;
	    sigwait(&blockset, &sig);
	}
#else
#ifdef HAVE_POLL_H
	int ret = poll(fds, limit, _timeout);
#else
	fd_set fdset;
	struct timeval        tval;
	tval.tv_sec = timeout;
	tval.tv_usec = 0;
	int ret = select(limit+1, &fdset, NULL, NULL, &tval);
#endif
#endif

	log_debug(_("Poll returned: %d, timeout is: %d"), ret, _timeout);

    while (ret--) {
	for (int i = 0; i<limit; i++) {
	    // If we get this event, the other end of the connection has been shut down
#if 0
	    if (fds[i].revents &POLLPRI ) {
		log_debug("%s: Revents has a POLLPRI  set 0x%x for fd #%d",
			  __FUNCTION__, fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLRDNORM) {
		log_debug("%s: Revents has a POLLRDNORM set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLHUP) {
		log_debug("%s: Revents has a POLLHUP set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }


	    if (fds[i].revents & POLLERR) {
		log_debug("%s: Revents has a POLLERR set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLHUP) {
		log_debug("%s: Revents has a POLLHUP set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLNVAL) {
		log_debug("%s: Revents has a POLLNVAL set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
//		throw GnashException("Polling an invalid file descritor");
	    }
	    if (fds[i].revents & POLLIN) {
		log_debug("%s: Revents has a POLLIN set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLMSG) {
		log_debug("%s: Revents has a POLLMSG set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLREMOVE) {
		log_debug("%s: Revents has a POLLREMOVE set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
	    }
	    if (fds[i].revents & POLLRDHUP) {
		log_debug("%s: Revents has a POLLRDHUP set 0x%x for fd #%d",
			  __FUNCTION__,  fds[i].revents, fds[i].fd);
//		throw GnashException("Connection dropped from client side.");
	    }
#endif    
// 	    if ((fds[i].revents & POLLIN) || (fds[i].revents & POLLRDHUP))  {
		hits->push_back(fds[i]);
// 		// If we got as many matches as were seen by poll(), then
// 		// stop searching the rest of the items in the array.
// 		if (hits->size() == ret) {
// 		    break;
// 		}
// 	    } else {
// 		log_debug("No data on fd #%d, revents is 0x%x", fds[i].fd, fds[i].revents);
// 	    }
	}
    }
    
    return hits;
}

fd_set
Network::waitForNetData(vector<int> &data)
{
    // GNASH_REPORT_FUNCTION;

    fd_set fdset;
    FD_ZERO(&fdset);
	
    if (data.size()) {
	int max = 0;
	
	for (size_t i = 0; i<data.size(); i++) {
	    FD_SET(data[i], &fdset);
	    if (data[i] > max) {
		max = data[i];
	    }
	}
	return waitForNetData(max+1, fdset);
    }

    return fdset;
}

fd_set
Network::waitForNetData(int limit, fd_set files)
{
    // GNASH_REPORT_FUNCTION;

    // select modifies the set of file descriptors, and we don't
    // want to modify the one passed as an argument, so we make a copy.
    fd_set fdset = files;

    // Reset the timeout value, since select modifies it on return
    int timeout = _timeout;
    if (timeout <= 0) {
	timeout = 30;
    }
#ifdef HAVE_PSELECT_XX
    struct timespec tval;
    sigset_t pending, sigmask;
    sigprocmask(SIG_BLOCK, &sigmask, NULL);

    tval.tv_sec = 0;
    tval.tv_nsec = timeout * 1000000000;
    int ret = pselect(limit+1, &fdset, NULL, NULL, &tval, &sigmask);
    sigpending(&pending);
    if (sigismember(&pending, SIGINT)) {
	log_debug(_("Have a pending SIGINT interupt waiting!"));
	int sig;
	sigwait(&sigmask, &sig);
    }
    if (sigismember(&pending, SIGPIPE)) {
	log_debug(_("Have a pending SIGPIPE interupt waiting!"));
	int sig;
	sigwait(&sigmask, &sig);
    }
#else
    struct timeval tval;
    tval.tv_sec = 0;
    tval.tv_usec = timeout * 1000; // was 1000000
    int ret = select(limit+1, &fdset, NULL, NULL, &tval);
#endif
    // If interrupted by a system call, try again
    if (ret == -1 && errno == EINTR) {
	log_error (_("Waiting for data was interrupted by a system call"));
    }
    
    if (ret == -1) {
	log_error (_("Waiting for data for fdset, was never available for reading"));
	FD_ZERO(&fdset);
	FD_SET(0, &fdset);
	return fdset;
    }
    
    if (ret == 0) {
	// log_debug (_("Waiting for data for fdset, timed out waiting for data"));
	FD_ZERO(&fdset);
	FD_SET(0, &fdset);
	return fdset;
    }

    if (ret < 0) {
	log_error(_("select() got an error: %s."), strerror(errno));
	FD_ZERO(&fdset);
	FD_SET(0, &fdset);
    } else {
	log_network(_("select() saw activity on %d file descriptors."), ret);
    }

    return fdset;
}

Network &
Network::operator = (Network &net)
{
    GNASH_REPORT_FUNCTION;
    
    // the file descriptor used for reading and writing
    _sockfd= net.getFileFd();
    // the file descriptor used to listen for new connections
    _listenfd = net.getListenFd();
    _port = net.getPort();
    _portstr = net.getPortStr();
    _url = net.getURL();
    _protocol = net.getProtocol();
    _host = net.getHost();
    _path = net.getPath();
    _connected = net.connected();
    _debug = net.netDebug();
    _timeout = net.getTimeout();
    return *this;
}

void
Network::toggleDebug(bool val)
{
    // Turn on our own debugging
    _debug = val;

    // Turn on debugging for the utility methods
    // recursive on all control paths,
    // toggleDebug(true);
}

#ifdef USE_SSL
bool
Network::initSSL(std::string &hostname)
{
    GNASH_REPORT_FUNCTION;
    string nothing;
    initSSL(hostname, nothing);
}

bool
Network::initSSL(std::string &hostname, std::string &password)
{
    GNASH_REPORT_FUNCTION;
    string nothing;

    initSSL(hostname, password, nothing, nothing, nothing, true);
}

bool
Network::initSSL(std::string &hostname, std::string &password, bool auth)
{
    GNASH_REPORT_FUNCTION;

    string nothing;
    initSSL(hostname, password, nothing, nothing, nothing, auth);
}

bool
Network::initSSL(std::string &hostname, std::string &passwd, 
		 std::string &keyfile, std::string &calist,
		 std::string &rootpath, bool auth)
{
    GNASH_REPORT_FUNCTION;

    // FIXME: make sure we have a connection

    if (_sockfd == 0) {
	if ((_sockfd = createClient(hostname, SSL_PORT) == false)) {
	    log_error(_("Can't connect to server %s"), hostname);
	    return false;
	}
    }

    if (!_ssl) {
	_ssl.reset(new SSLClient);
    }

    if (!hostname.empty()) {
	_ssl->setHostname(hostname);
    } else {
	log_debug(_("Using default hostname: \"%s\""), _host);
    }
    if (!keyfile.empty()) {
	_ssl->setKeyfile(keyfile);
    } else {
	log_debug(_("Using default keyfile: \"%s\""), _ssl->getKeyfile());
    }
    if (!calist.empty()) {
	_ssl->setCAlist(calist);
    } else {
	log_debug(_("Using default CA List: \"%s\""), _ssl->getCAlist());
    }

    if (!passwd.empty()) {
	_ssl->setPassword(passwd);
    } else {
	log_debug(_("Using default Password: \"%s\""), _ssl->getPassword());
    }
    if (!rootpath.empty()) {
	_ssl->setRootPath(rootpath);
    } else {
	log_debug(_("Using default Root Path to PEM files: \"%s\""),
		  _ssl->getRootPath());
    }

    if (_ssl->sslConnect(_sockfd)) {
        log_debug(_("Connected to SSL server"));
    } else {
        log_error(_("Couldn't connect to SSL server"));
	return false;
    }

    // If we got this far, everthing worked
    return true;
}
#endif

// Use an ioctl() to see how many bytes are in the network buffers.
size_t
Network::sniffBytesReady(int fd)
{
    // GNASH_REPORT_FUNCTION;

    int bytes = 0;
    fd_set fdset;

    FD_SET(fd, &fdset);

    struct timeval tval;
    tval.tv_sec = 0;
    tval.tv_usec = 10;
    if (select(fd+1, &fdset, NULL, NULL, &tval)) {
 	if (FD_ISSET(fd, &fdset)) {
#ifndef _WIN32
	    ioctl(fd, FIONREAD, &bytes);
#else
	    ioctlSocket(fd, FIONREAD, &bytes);
#endif
	}
    }

    log_network(_("#%d bytes waiting in kernel network buffer."), bytes);
    
    return bytes;
}

// Trap Control-C so we can cleanly exit
static void
cntrlc_handler (int sig)
{
    GNASH_REPORT_FUNCTION;
    sig_number = sig;
    log_debug(_("Got an %d interrupt while blocked on pselect()"), sig);
    exit(EXIT_FAILURE);
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
