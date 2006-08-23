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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utility.h"
#include "log.h"
#include "xml.h"
#include "xmlsocket.h"
#include "timers.h"
#include "as_function.h"
#include "fn_call.h"
#include "sprite_instance.h"

#ifdef HAVE_LIBXML

#include <sys/types.h>
#include <fcntl.h>
#ifdef HAVE_WINSOCK
# include <WinSock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
#else
# include <sys/time.h>
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

int xml_fd = 0;                 // FIXME: This file descriptor is used by
                                // XML::checkSocket() when called from the main
                                // processing loop. 

namespace gnash {
const int SOCKET_DATA = 1;
  
const int INBUF = 10000;
  
XMLSocket::XMLSocket() {
  //log_msg("%s: \n", __FUNCTION__);
  _data = false;
  _xmldata = false;
  _closed = false;
  _connect = false;
  _processing = false;
  _port = 0;
  _sockfd = 0;
  xml_fd = 0;
}

XMLSocket::~XMLSocket()
{
  //log_msg("%s: \n", __FUNCTION__);
}

bool
XMLSocket::connect(const char *host, int port)
{
  struct sockaddr_in  sock_in;
  fd_set              fdset;
  struct timeval      tval;
  int                 ret;
  int                 retries;
  char                thishostname[MAXHOSTNAMELEN];
  struct protoent     *proto;

  if (port < 1024) {
    log_error("Can't connect to priviledged port #%d!\n", port);
    _connect = false;
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
        _connect = true;
        xml_fd = _sockfd;               // FIXME: This file descriptor is used by
                                        // XML::checkSocket() when called from
                                        // the main processing loop.
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

  _connect = true;
  return true;
}

void
XMLSocket::close()
{
  log_msg("%s: \n", __FUNCTION__);
  // Since the return code from close() doesn't get used by Shockwave,
  // we don't care either.
  if (_sockfd > 0) {
    ::close(_sockfd);
  }
}

// Return true if there is data in the socket, otherwise return false.
bool
XMLSocket::anydata(char **msgs)
{
  //printf("%s: \n", __FUNCTION__);
  return anydata(_sockfd, msgs);
}

bool XMLSocket::processingData()
{
  //printf("%s: processing flags is is %d\n", __FUNCTION__, _processing);
  return _processing;
}

void XMLSocket::processing(bool x)
{
  //printf("%s: set processing flag to %d\n", __FUNCTION__, x);
  _processing = x;
}

bool
XMLSocket::anydata(int fd, char **msgs)
{
  fd_set                fdset;
  struct timeval        tval;
  int                   ret = 0;
  char                  buf[INBUF];
  char                  *packet;
  int                   retries = 10;
  char                  *ptr, *eom;
  int                   cr, index = 0;
  static char           *leftover = 0;
  int                   adjusted_size;

  //log_msg("%s: \n", __FUNCTION__);

  if (fd <= 0) {
    return false;
  }

  //msgs = (char **)realloc(msgs, sizeof(char *));

  while (retries-- > 0) {
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    
    tval.tv_sec = 0;
    tval.tv_usec = 103;
    
    ret = ::select(fd+1, &fdset, NULL, NULL, &tval);

    // If interupted by a system call, try again
    if (ret == -1 && errno == EINTR) {
      log_msg("The socket for fd #%d was interupted by a system call!\n",
              fd);
      continue;
    }
    if (ret == -1) {
      log_error("The socket for fd #%d never was available!\n", fd);
      return false;
    }
    if (ret == 0) {
      //log_msg("There is no data in the socket for fd #%d!\n", fd);
      return false;
    }
    if (ret > 0) {
      //log_msg("There is data in the socket for fd #%d!\n", fd);        
      //break;
    }
    memset(buf, 0, INBUF);
    ret = ::read(_sockfd, buf, INBUF-2);
    cr = strlen(buf);
    //log_msg("%s: read %d bytes, first msg terminates at %d\n", __FUNCTION__, ret, cr);
    //log_msg("%s: read (%d,%d) %s\n", __FUNCTION__, buf[0], buf[1], buf);
    ptr = buf;
    // If we get a single XML message, do less work
    if (ret == cr + 1) {
      adjusted_size = memadjust(ret + 1);
      packet = new char[adjusted_size];
      //printf("Packet size is %d at %p\n", ret + 1, packet);
      memset(packet, 0, adjusted_size);
      strcpy(packet, ptr);
      eom = strrchr(packet, '\n'); // drop the CR off the end if there is one
      if (eom) {
        *eom = 0;
      }
      //data.push_back(packet);
      msgs[index] = packet;
      msgs[index+1] = 0;
      //printf("%d: Pushing Packet of size %d at %p\n", __LINE__, strlen(packet), packet);
      processing(false);
      return true;
    }

    // If we get multiple messages in a single transmission, break the buffer
    // into separate messages.
    while (strchr(ptr, '\n') > 0) {
      if (leftover) {
        processing(false);
        //printf("%s: The remainder is: \"%s\"\n", __FUNCTION__, leftover);
        //printf("%s: The rest of the message is: \"%s\"\n", __FUNCTION__, ptr);
        adjusted_size = memadjust(cr + strlen(leftover) + 1);
        packet = new char[adjusted_size];
        memset(packet, 0, adjusted_size);
        strcpy(packet, leftover);
        strcat(packet, ptr);
        eom = strrchr(packet, '\n'); // drop the CR off the end there is one
        if (eom) {
          *eom = 0;
        }
        //printf("%s: The whole message is: \"%s\"\n", __FUNCTION__, packet);
        ptr = strchr(ptr, '\n') + 2; // messages are delimited by a "\n\0"
        delete leftover;
        leftover = 0;
      } else {
        adjusted_size = memadjust(cr + 1);
        packet = new char[adjusted_size];
        memset(packet, 0, adjusted_size);
        strcpy(packet, ptr);
        ptr += cr + 1;
      } // end of if remainder
      if (*packet == '<') {
        //printf("%d: Pushing Packet #%d of size %d at %p: %s\n", __LINE__,
        //       data.size(), strlen(packet), packet, packet);
        eom = strrchr(packet, '\n'); // drop the CR off the end there is one
        if (eom) {
          *eom = 0;
        }
        //printf("Allocating new packet at %p\n", packet);
        //data.push_back(packet);
        msgs[index++] = packet;
      } else {
        log_error("Throwing out partial packet %s\n", packet);
      }
      
      //log_msg("%d messages in array now\n", data.size());
      cr = strlen(ptr);
    } // end of while (cr)
    
    if (strlen(ptr) > 0) {
      leftover = new char[strlen(ptr) + 1];
      strcpy(leftover, ptr);
      processing(true);
      //printf("%s: Adding remainder: \"%s\"\n", __FUNCTION__, leftover);
    }
    
    processing(false);
    printf("Returning %d messages\n", index);
    return true;
    
  } // end of while (retires)

  return true;
}

bool
XMLSocket::send(tu_string str)
{
  //GNASH_REPORT_FUNCTION;
  
  str += '\0';
  int ret = write(_sockfd, str.c_str(), str.size());

  //log_msg("%s: sent %d bytes, data was %s\n", __FUNCTION__, ret, str.c_str());
  if (ret == str.size()) {
    return true;
  } else {
    return false;
  }
}

// Callbacks

void
XMLSocket::onClose(tu_string /* str */)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
XMLSocket::onConnect(tu_string /* str */)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
XMLSocket::onData(tu_string /* str */)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
XMLSocket::onXML(tu_string /* str */)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
XMLSocket::push(as_object *obj)
{
  _nodes.push_back(obj);
}

void
XMLSocket::clear()
{
  unsigned int i;
  for (i=0; i< _nodes.size(); i++) {
    delete _nodes[i];
  }
}

int
XMLSocket::count()
{
  return _nodes.size();
}

int
XMLSocket::checkSockets(void)
{
  return checkSockets(_sockfd);
}

int
XMLSocket::checkSockets(int fd)
{
  fd_set                fdset;
  int                   ret = 0;
  struct timeval        tval;

  //log_msg("%s:\n", __FUNCTION__);

  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);
  
  tval.tv_sec = 2;
  tval.tv_usec = 10;
  
  ret = ::select(fd+1, &fdset, NULL, NULL, &tval); // &tval
  
  // If interupted by a system call, try again
  if (ret == -1 && errno == EINTR) {
    log_msg("The socket for fd #%d was interupted by a system call in this thread!\n",
            fd);
  }
  if (ret == -1) {
    log_error("The socket for fd #%d never was available!\n", fd);
  }
  if (ret == 0) {
    printf("There is no data in the socket for fd #%d!\n", fd);
  }
  if (ret > 0) {
    //printf("There is data in the socket for fd #%d!\n", fd);        
  }
  
  return ret;
}

void
xmlsocket_connect(const fn_call& fn)
{
  as_value	method;
  as_value	val;
  static bool first = true;     // This event handler should only be executed once.
  bool          ret;

  if (!first) {
    fn.result->set_bool(true);
    return;
  }
  
  log_msg("%s: nargs=%d\n", __FUNCTION__, fn.nargs);
  xmlsocket_as_object*	ptr = (xmlsocket_as_object*) (as_object*) fn.this_ptr;
  assert(ptr);
  const tu_string host = fn.env->bottom(fn.first_arg_bottom_index).to_string();
  tu_string port_str = fn.env->bottom(fn.first_arg_bottom_index-1).to_tu_string();
  double port = atof(port_str.c_str());
  ret = ptr->obj.connect(host.c_str(), static_cast<int>(port));

#if 0
  // Push result onto stack for onConnect
  if (ret) {
    fn.env->push(as_value(true));
  }
  else {
    fn.env->push(as_value(false));
  }
#endif
  fn.env->push(as_value(true));
  if (fn.this_ptr->get_member("onConnect", &method)) {
    //    log_msg("FIXME: Found onConnect!\n");
    as_c_function_ptr	func = method.to_c_function();
    first = false;
    //env->set_variable("success", true, 0);

    if (func) {
      // It's a C function.  Call it.
      log_msg("Calling C function for onConnect\n");
      (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
    }
    else if (as_function* as_func = method.to_as_function()) {
      // It's an ActionScript function.  Call it.
      log_msg("Calling ActionScript function for onConnect\n");
      (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 2, 2));
    } else {
      log_error("error in call_method(): method is not a function\n");
    }    
  } else {
    //ptr->set_event_handler(event_id::SOCK_CONNECT, (as_c_function_ptr)&xmlsocket_event_connect);
  }

#if 1
  movie*	mov = fn.env->get_target()->get_root_movie();
  Timer *timer = new Timer;
  as_c_function_ptr ondata_handler =
    (as_c_function_ptr)&xmlsocket_event_ondata;
  timer->setInterval(ondata_handler, 50, ptr, fn.env);
  timer->setObject(ptr);
  mov->add_interval_timer(timer);
#endif

  fn.env->pop();
  
  fn.result->set_bool(true);
}


void
xmlsocket_send(const fn_call& fn)
{
  as_value	method;
  as_value	val;
  
  xmlsocket_as_object*	ptr = (xmlsocket_as_object*) (as_object*) fn.this_ptr;
  assert(ptr);
  const tu_string object = fn.env->bottom( fn.first_arg_bottom_index).to_string();
  //  log_msg("%s: host=%s, port=%g\n", __FUNCTION__, host, port);
  fn.result->set_bool(ptr->obj.send(object));
}

void
xmlsocket_close(const fn_call& fn)
{
  as_value	method;
  as_value	val;
  
  xmlsocket_as_object*	ptr = (xmlsocket_as_object*) (as_object*) fn.this_ptr;
  assert(ptr);
  // Since the return code from close() doesn't get used by Shockwave,
  // we don't care either.
  ptr->obj.close();
}

void
xmlsocket_xml_new(const fn_call& fn)
{
  //log_msg("%s: nargs=%d\n", __FUNCTION__, nargs);
  
  xml_new(fn);
}

void
xmlsocket_new(const fn_call& fn)
{
  //log_msg("%s: nargs=%d\n", __FUNCTION__, nargs);
  
  as_object*	xmlsock_obj = new xmlsocket_as_object;
  //log_msg("\tCreated New XMLSocket object at 0x%X\n", (unsigned int)xmlsock_obj);
  xmlsock_obj->set_member("connect", &xmlsocket_connect);
  xmlsock_obj->set_member("send", &xmlsocket_send);
  xmlsock_obj->set_member("close", &xmlsocket_close);
  xmlsock_obj->set_member("Connected", true);
  // swf_event*	ev = new swf_event;
  // m_event_handlers.push_back(ev);
  // Setup event handlers
#if 0
  xmlsock_obj->set_event_handler(event_id::SOCK_DATA,
                                 (as_c_function_ptr)&xmlsocket_event_ondata);
  xmlsock_obj->set_event_handler(event_id::SOCK_CLOSE,
                                 (as_c_function_ptr)&xmlsocket_event_close);
  // 							xmlsock_obj->set_event_handler(event_id::SOCK_CONNECT,
  // 									       (as_c_function_ptr)&xmlsocket_event_connect);
  xmlsock_obj->set_event_handler(event_id::SOCK_XML,
                                 (as_c_function_ptr)&xmlsocket_event_xml);
#endif
  //periodic_events.set_event_handler(xmlsock_obj);
  
  
#if 1
  //as_c_function_ptr int_handler = (as_c_function_ptr)&timer_setinterval;
  //env->set_member("setInterval", int_handler);
  fn.env->set_member("setInterval", timer_setinterval);
  
  //as_c_function_ptr clr_handler = timer_clearinterval;
  fn.env->set_member("clearInterval", timer_clearinterval);
  //env->set_variable("setInterval", int_handler, 0);
  //xmlsock_obj->set_event_handler(event_id::TIMER,
  //       (as_c_function_ptr)&timer_expire);
#if 0
  Timer *timer = new Timer;
  as_c_function_ptr ondata_handler =
    (as_c_function_ptr)&xmlsocket_event_ondata;
  timer->setInterval(ondata_handler, 10);
  timer->setObject(xmlsock_obj);
  current_movie->add_interval_timer(timer);
#endif
  
  fn.result->set_bool(xmlsock_obj);

  // Tune malloc for the best performance
  //mallopt(M_MMAP_MAX,0);
  //mallopt(M_TRIM_THRESHOLD,-1);
  //mallopt(M_MMAP_THRESHOLD,16);

#endif
}


void
xmlsocket_event_ondata(const fn_call& fn)
{
  //log_msg("%s: nargs is %d\n", __FUNCTION__, nargs);
    
  as_value	method;
  as_value	val;
  as_value      datain;
  std::vector<const char *> msgs;
  char          *messages[200];
  int           i;
  as_c_function_ptr	func;
  as_function*       as_func;
  tu_string     data; 

  xmlsocket_as_object*	ptr = (xmlsocket_as_object*)fn.this_ptr;
  assert(ptr);
  if (ptr->obj.processingData()) {
    log_msg("Still processing data!\n");
    fn.result->set_bool(false);
    return;
  }
  
  memset(messages, 0, sizeof(char *)*200);
  
#ifndef USE_DMALLOC
  //dump_memory_stats(__FUNCTION__, __LINE__, "memory checkpoint");
#endif
  
  if (ptr->obj.anydata(messages)) {
    if (fn.this_ptr->get_member("onData", &method)) {
      func = method.to_c_function();
      as_func = method.to_as_function();
      //log_msg("Got %d messages from XMLsocket\n", msgs.size());
      //      for (i=0; i<msgs.size(); i++) {
      for (i=0; messages[i] != 0; i++) {
//          log_msg("Got message #%d, %d bytes long at %p: %s: \n", i,
//                  strlen(messages[i]), messages[i], messages[i]);
        datain = messages[i];
        //fn.env->push(datain);
#ifndef USE_DMALLOC
        //dump_memory_stats(__FUNCTION__, __LINE__, "start");
#endif
        as_environment *env = new as_environment;
        env->push(datain);
        if (func) {
          // It's a C function.  Call it.
          //log_msg("Calling C function for onData\n");
          (*func)(fn_call(&val, fn.this_ptr, env, 1, 0));
        } else if (as_func) {
          // It's an ActionScript function.  Call it.
          //log_msg("Calling ActionScript function for onData, processing msg %d\n", i);
          (*as_func)(fn_call(&val, fn.this_ptr, env, 1, 0));
        } else {
          log_error("error in call_method(): method is not a function\n");
        }
        env->pop();
        delete env;
#ifndef USE_DMALLOC
        //dump_memory_stats(__FUNCTION__, __LINE__, "end");
#endif  
        //log_msg("Deleting message #%d at %p\n", i, messages[i]);
        //delete messages[i];
        //fn.env->pop();
        datain.set_undefined();
      }
      ptr->obj.processing(false);
    } else {
      log_error("Couldn't find onData!\n");
    }
    // Delete this in a batch for now so we can track memory allocation
    for (i=0; messages[i] != 0; i++) {
      //log_msg("Deleting message #%d at %p\n", i, messages[i]);
      delete messages[i];
    }
  }

  //malloc_trim(0);
  
  //result->set(&data);
  fn.result->set_bool(true);
}

void
xmlsocket_event_close(const fn_call& /* fn */)
{
#if 0
  as_value* result = fn.result;
  as_object* this_ptr = fn.this_ptr;
  int nargs = fn.nargs;
  int first_arg = fn.first_arg_bottom_index;
#else
  log_error("%s: unimplemented!\n", __FUNCTION__);
#endif
}

void
xmlsocket_event_connect(const fn_call& fn)
{
  as_value	method;
  as_value	val;
  tu_string     data;
  static bool first = true;     // This event handler should only be executed once.

  if (!first) {
    fn.result->set_bool(true);
    return;
  }
  
  xmlsocket_as_object*	ptr = (xmlsocket_as_object*) (as_object*) fn.this_ptr;
  assert(ptr);

  log_msg("%s: connected = %d\n", __FUNCTION__, ptr->obj.connected());
  if ((ptr->obj.connected()) && (first)) {
    first = false;
    //env->set_variable("success", true, 0);
    //env->bottom(0) = true;

    if (fn.this_ptr->get_member("onConnect", &method)) {
      as_c_function_ptr	func = method.to_c_function();
      if (func)
        {
          // It's a C function.  Call it.
          //log_msg("Calling C function for onConnect\n");
          (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
      else if (as_function* as_func = method.to_as_function())
        {
          // It's an ActionScript function.  Call it.
          //log_msg("Calling ActionScript function for onConnect\n");
          (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
        }
      else
        {
          log_error("error in call_method(): method is not a function\n");
        }    
    } else {
      log_msg("FIXME: Couldn't find onConnect!\n");
    }
  }

  fn.result->set_bool(val.to_bool()); 
}
void
xmlsocket_event_xml(const fn_call& /* fn */)
{
#if 0
  as_value* result = fn.result;
  as_object* this_ptr = fn.this_ptr;
  int nargs = fn.nargs;
  int first_arg = fn.first_arg_bottom_index;
#else
  log_error("%s: unimplemented!\n", __FUNCTION__);
#endif  
}

static XMLSocket xs;

int
check_sockets(int x)
{
  if (xml_fd == 0) {
    return -1;
  }
  
  return xs.checkSockets(x);
}

} // end of gnaash namespace

// HAVE_LIBXML
#endif
