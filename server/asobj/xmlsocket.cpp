// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utility.h"
#include "xml.h"
#include "xmlsocket.h"
#include "timers.h"
#include "as_function.h"
#include "fn_call.h"
#include "sprite_instance.h"
#include "VM.h"
#include "builtin_function.h" // for setting timer, should likely avoid that..

#include "log.h"

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
# include <cerrno>
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
  
XMLSocket::XMLSocket()
{
//    GNASH_REPORT_FUNCTION;
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
//    GNASH_REPORT_FUNCTION;
}

bool
XMLSocket::connect(const char *host, short port)
{
    GNASH_REPORT_FUNCTION;
    createClient(host, port);
    _connect = true;
    return true;
}

void
XMLSocket::close()
{
    GNASH_REPORT_FUNCTION;
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
    GNASH_REPORT_FUNCTION;
    return anydata(_sockfd, msgs);
}

bool XMLSocket::processingData()
{
    GNASH_REPORT_FUNCTION;
    //printf("%s: processing flags is is %d\n", __FUNCTION__, _processing);
    return _processing;
}

void XMLSocket::processing(bool x)
{
    GNASH_REPORT_FUNCTION;
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
XMLSocket::send(std::string str)
{
    //GNASH_REPORT_FUNCTION;
    
    str += '\0';
    int ret = write(_sockfd, str.c_str(), str.size());
    
    //log_msg("%s: sent %d bytes, data was %s\n", __FUNCTION__, ret, str.c_str());
    if (ret == static_cast<signed int>(str.size())) {
        return true;
    } else {
        return false;
    }
}

// Callbacks

void
XMLSocket::onClose(std::string /* str */)
{
    GNASH_REPORT_FUNCTION;
}

void
XMLSocket::onConnect(std::string /* str */)
{
    GNASH_REPORT_FUNCTION;
}

void
XMLSocket::onData(std::string /* str */)
{
    GNASH_REPORT_FUNCTION;
}

void
XMLSocket::onXML(std::string /* str */)
{
    GNASH_REPORT_FUNCTION;
}

void
XMLSocket::push(as_object *obj)
{
    GNASH_REPORT_FUNCTION;
    _nodes.push_back(obj);
}

void
XMLSocket::clear()
{
    GNASH_REPORT_FUNCTION;
    for (unsigned int i=0; i< _nodes.size(); i++) {
        delete _nodes[i];
    }
}

int
XMLSocket::count()
{
    GNASH_REPORT_FUNCTION;
    return _nodes.size();
}

int
XMLSocket::checkSockets(void)
{
    GNASH_REPORT_FUNCTION;
    return checkSockets(_sockfd);
}

int
XMLSocket::checkSockets(int fd)
{
    GNASH_REPORT_FUNCTION;
    fd_set                fdset;
    int                   ret = 0;
    struct timeval        tval;
    

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

as_value
xmlsocket_connect(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;
    static bool first = true;     // This event handler should only be executed once.
    
    if (!first) {
        return as_value(true);
    }
    
    log_msg("%s: nargs=%d\n", __FUNCTION__, fn.nargs);
    xmlsocket_as_object* ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    const std::string host = fn.env->bottom(fn.first_arg_bottom_index).to_string();
    std::string port_str = fn.env->bottom(fn.first_arg_bottom_index-1).to_string();
    double port = atof(port_str.c_str());
    
    ptr->obj.connect(host.c_str(), static_cast<int>(port));
    
#if 0 // use connect return as result
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
        first = false; // what is this for ?
        val = call_method0(method, fn.env, fn.this_ptr);
    } else {
        //ptr->set_event_handler(event_id::SOCK_CONNECT, (as_c_function_ptr)&xmlsocket_event_connect);
    }
    
#if 1
    // TODO: don't allocate on heap!
    Timer *timer = new Timer;
    boost::intrusive_ptr<builtin_function> ondata_handler = new builtin_function(
        &xmlsocket_event_ondata, NULL);
    timer->setInterval(*ondata_handler, 50, ptr, fn.env);
    VM::get().getRoot().add_interval_timer(*timer);
#endif
    
    fn.env->pop();
    
    return as_value(true);
}


as_value
xmlsocket_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;
    
    xmlsocket_as_object* ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    const std::string object = fn.env->bottom( fn.first_arg_bottom_index).to_string();
    //  log_msg("%s: host=%s, port=%g\n", __FUNCTION__, host, port);
    return as_value(ptr->obj.send(object));
}

as_value
xmlsocket_close(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;
    
    xmlsocket_as_object* ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    // Since the return code from close() doesn't get used by Shockwave,
    // we don't care either.
    ptr->obj.close();
    return as_value();
}

as_value
xmlsocket_xml_new(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    //log_msg("%s: nargs=%d\n", __FUNCTION__, nargs);
    
    xml_new(fn);
    return as_value();
}

as_value
xmlsocket_new(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
    //log_msg("%s: nargs=%d\n", __FUNCTION__, nargs);
    
    as_object*	xmlsock_obj = new xmlsocket_as_object;
    //log_msg("\tCreated New XMLSocket object at 0x%X\n", (unsigned int)xmlsock_obj);
    xmlsock_obj->init_member("connect",
                             new builtin_function(xmlsocket_connect));
    xmlsock_obj->init_member("send", new builtin_function(xmlsocket_send));
    xmlsock_obj->init_member("close", new builtin_function(xmlsocket_close));
    xmlsock_obj->init_member("Connected", true);
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
    
    
#if 0 // TODO: setInterval and clearInterval shall be _global methods
    //
    //as_c_function_ptr int_handler = (as_c_function_ptr)&timer_setinterval;
    //env->set_member("setInterval", int_handler);
    fn.env->set_member("setInterval", timer_setinterval);
    
    //as_c_function_ptr clr_handler = timer_clearinterval;
    // TODO:  check this, sounds suspicious
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
#endif
    
    return as_value(xmlsock_obj);
    
    // Tune malloc for the best performance
    //mallopt(M_MMAP_MAX,0);
    //mallopt(M_TRIM_THRESHOLD,-1);
    //mallopt(M_MMAP_THRESHOLD,16);
    
}


as_value
xmlsocket_event_ondata(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    as_value	method;
    as_value	val;
    as_value      datain;
    std::vector<const char *> msgs;
    char          *messages[200];
    int           i;
    
    xmlsocket_as_object* ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    if (ptr->obj.processingData()) {
        log_msg("Still processing data!\n");
        return as_value(false);
    }
    
    memset(messages, 0, sizeof(char *)*200);
    
#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "memory checkpoint");
#endif
    
    if (ptr->obj.anydata(messages)) {
        if (fn.this_ptr->get_member("onData", &method)) {
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
        	val = call_method(method, env, fn.this_ptr, 1, 0);
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
  return as_value(true);
}

as_value
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
  return as_value();
}

as_value
xmlsocket_event_connect(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;
    static bool first = true;     // This event handler should only be executed once.
    
    if (!first) {
        return as_value(true);
    }
    
    xmlsocket_as_object* ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    
    log_msg("%s: connected = %d\n", __FUNCTION__, ptr->obj.connected());
    if ((ptr->obj.connected()) && (first)) {
        first = false;
        //env->set_variable("success", true, 0);
        //env->bottom(0) = true;
        
        if (fn.this_ptr->get_member("onConnect", &method)) {
	    val = call_method0(method, fn.env, fn.this_ptr);
        } else {
            log_msg("FIXME: Couldn't find onConnect!\n");
        }
    }
    
    return as_value(val.to_bool()); 
}
as_value
xmlsocket_event_xml(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
#if 0
    as_value* result = fn.result;
    as_object* this_ptr = fn.this_ptr;
    int nargs = fn.nargs;
    int first_arg = fn.first_arg_bottom_index;
#else
    log_error("%s: unimplemented!\n", __FUNCTION__);
#endif  
    return as_value();
}

static XMLSocket xs;

int
check_sockets(int x)
{
    GNASH_REPORT_FUNCTION;
    if (xml_fd == 0) {
        return -1;
    }
    
    return xs.checkSockets(x);
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
