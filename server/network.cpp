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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "utility.h"
#include "log.h"
#include "xml.h"
#include "network.h"

#include <sys/types.h>
#ifdef HAVE_WINSOCK
# include <WinSock2.h>
# include <windows.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <io.h>
#else
# include <sys/time.h>
# include <sys/fcntl.h>
# include <unistd.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netdb.h>
# include <errno.h>
# include <sys/param.h>
# include <sys/select.h>
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

namespace gnash {

const int SOCKET_DATA   = 1;  
const int INBUF         = 10000;
  
Network::Network() : _sockfd(0), _port(0), _connected(false)
{
  //log_msg("%s: \n", __FUNCTION__);
}

Network::~Network() 
{
  //log_msg("%s: \n", __FUNCTION__);
}

bool
Network::clientConnect(const char *host, short port)
{
  struct sockaddr_in  sock_in;
  fd_set              fdset;
  struct timeval      tval;
  int                 ret;
  int                 retries;
  char                thishostname[MAXHOSTNAMELEN];
  struct protoent     *proto;

  if (port < 1024) {
    log_error("Can't connect to priviledged port #%hd!\n", port);
    _connected = false;
    return false;
  }

  log_msg("%s: to host %s at port %d\n", __FUNCTION__, host, port);
  
  memset(&sock_in, 0, sizeof(struct sockaddr_in));
  memset(&thishostname, 0, MAXHOSTNAMELEN);
  if (strlen(host) == 0) {
    if (gethostname(thishostname, MAXHOSTNAMELEN) == 0) {
      log_msg("The hostname for this machine is %s.\n", thishostname);
    } else {
      log_msg("Couldn't get the hostname for this machine!\n");
      return false;
    }   
  }
  const struct hostent *hent = ::gethostbyname(host);
  if (hent > 0) {
    ::memcpy(&sock_in.sin_addr, hent->h_addr, hent->h_length);
  }
  sock_in.sin_family = AF_INET;
  sock_in.sin_port = ntohs(static_cast<short>(port));

#if 0
    char ascip[32];
    inet_ntop(AF_INET, &sock_in.sin_addr.s_addr, ascip, INET_ADDRSTRLEN);
      log_msg("The IP address for this client socket is %s\n", ascip);
#endif

  proto = ::getprotobyname("TCP");

  _sockfd = ::socket(PF_INET, SOCK_STREAM, proto->p_proto);
  if (_sockfd < 0)
    {
      log_error("unable to create socket : %s\n", strerror(errno));
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

    // If interupted by a system call, try again
    if (ret == -1 && errno == EINTR)
      {
        log_msg("The connect() socket for fd #%d was interupted by a system call!\n",
                _sockfd);
        continue;
      }
    
    if (ret == -1)
      {
        log_msg("The connect() socket for fd #%d never was available for writing!\n",
                _sockfd);
#ifdef HAVE_WINSOCK
        ::shutdown(_sockfd, SHUT_BOTH);
#else
        ::shutdown(_sockfd, SHUT_RDWR);
#endif
        _sockfd = -1;      
        return false;
      }
    if (ret == 0) {
      log_error("The connect() socket for fd #%d timed out waiting to write!\n",
                _sockfd);
      continue;
    }

    if (ret > 0) {
      ret = ::connect(_sockfd, reinterpret_cast<struct sockaddr *>(&sock_in), sizeof(sock_in));
      if (ret == 0) {
        log_msg("\tport %d at IP %s for fd #%d\n", port,
                ::inet_ntoa(sock_in.sin_addr), _sockfd);
        _connected = true;
        return true;
      }
      if (ret == -1) {
        log_msg("The connect() socket for fd #%d never was available for writing!\n",
                _sockfd);
        _sockfd = -1;      
        return false;
      }
    }
  }
  //  ::close(_sockfd);
  //  return false;

  printf("\tConnected at port %d on IP %s for fd #%d\n", port,
          ::inet_ntoa(sock_in.sin_addr), _sockfd);
  
#ifndef HAVE_WINSOCK
  fcntl(_sockfd, F_SETFL, O_NONBLOCK);
#endif

  _connected = true;
  return true;
}

void
Network::close()
{
  log_msg("%s: \n", __FUNCTION__);
  // Since the return code from close() doesn't get used by Shockwave,
  // we don't care either.
  if (_sockfd > 0) {
    ::close(_sockfd);
  }
}

bool
Network::send(const char *data)
{
  log_msg("%s: \n", __FUNCTION__);
  int length = strlen(data);
  
  int ret = write(_sockfd, data, length);

  log_msg("%s: sent %d bytes, data was %s\n",
          __FUNCTION__, ret, data);
  if (ret == length) {
    return true;
  } else {
    return false;
  }
}

void network_geturl(const fn_call& fn)
{
    network_as_object *ptr = (network_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.getURL().c_str());
}

void network_getprotocol(const fn_call& fn)
{
    network_as_object *ptr = (network_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.getProtocol().c_str());
}

void network_gethost(const fn_call& fn)
{
    network_as_object *ptr = (network_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.getHost().c_str());
}

void network_getport(const fn_call& fn)
{
    network_as_object *ptr = (network_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.getPortStr().c_str());
}

void network_getpath(const fn_call& fn)
{
    network_as_object *ptr = (network_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.getPath().c_str());
}

} // end of gnaash namespace
