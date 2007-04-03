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

#define GNASH_XMLSOCKET_DEBUG

int xml_fd = 0;                 // FIXME: This file descriptor is used by
                                // XML::checkSocket() when called from the main
                                // processing loop. 

namespace gnash {

static as_value xmlsocket_connect(const fn_call& fn);
static as_value xmlsocket_send(const fn_call& fn);
static as_value xmlsocket_new(const fn_call& fn);
static as_value xmlsocket_close(const fn_call& fn);

// These are the event handlers called for this object
static as_value xmlsocket_event_ondata(const fn_call& fn);

static as_object* getXMLSocketInterface();
static void attachXMLSocketInterface(as_object& o);

const int SOCKET_DATA = 1;
  
const int INBUF = 10000;

class DSOLOCAL xmlsocket_as_object : public gnash::as_object
{

public:

        xmlsocket_as_object()
                :
                as_object(getXMLSocketInterface())
        {}

        XMLSocket obj;
};

  
XMLSocket::XMLSocket()
{
//    GNASH_REPORT_FUNCTION;
    _data = false;
    _xmldata = false;
    _closed = false;
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
    bool success = createClient(host, port);

    assert( success || ! connected() );

    return success;
}

void
XMLSocket::close()
{
    GNASH_REPORT_FUNCTION;

    closeNet();
    // dunno why Network::closeNet() returns false always
    // doesn't make much sense to me...
    // Anyway, let's make sure we're clean
    assert(!_sockfd);
    assert(!_connected);
    assert(!connected());
}

// Return true if there is data in the socket, otherwise return false.
bool
XMLSocket::anydata(char **msgs)
{
    //GNASH_REPORT_FUNCTION;
    assert(connected());
    assert(_sockfd > 0);
    return anydata(_sockfd, msgs);
}

bool XMLSocket::processingData()
{
    //GNASH_REPORT_FUNCTION;
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
    GNASH_REPORT_FUNCTION;

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
    
    
    if (fd <= 0) {
	    log_msg("fd <= 0, returning false");
	    assert(!connected());
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
            log_msg("The socket for fd #%d was interupted by a system call!",
                    fd);
            continue;
        }
        if (ret == -1) {
            log_error("%s: The socket for fd #%d never was available!",
                __FUNCTION__, fd);
            return false;
        }
        if (ret == 0) {
            log_msg("%s: There is no data in the socket for fd #%d!",
                __FUNCTION__, fd);
            return false;
        }
        if (ret > 0) {
            log_msg("%s: There is data in the socket for fd #%d!",
                __FUNCTION__, fd);
            break;
        }
        memset(buf, 0, INBUF);
        ret = ::read(_sockfd, buf, INBUF-2);
        cr = strlen(buf);
        log_msg("%s: read %d bytes, first msg terminates at %d\n", __FUNCTION__, ret, cr);
        //log_msg("%s: read (%d,%d) %s\n", __FUNCTION__, buf[0], buf[1], buf);
        ptr = buf;
        // If we get a single XML message, do less work
        if (ret == cr + 1) {
            adjusted_size = memadjust(ret + 1);
            packet = new char[adjusted_size];
            printf("Packet size is %d at %p\n", ret + 1, packet);
            memset(packet, 0, adjusted_size);
            strcpy(packet, ptr);
            eom = strrchr(packet, '\n'); // drop the CR off the end if there is one
            if (eom) {
                *eom = 0;
            }
            //data.push_back(packet);
            msgs[index] = packet;
            msgs[index+1] = 0;
            printf("%d: Pushing Packet of size %d at %p\n", __LINE__, strlen(packet), packet);
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
    
    if ( ! connected() )
    {
	assert(!_sockfd);
        log_warning("socket not initialized at XMLSocket.send() call time");
	return false;
    }
    
    int ret = write(_sockfd, str.c_str(), str.size());
    
    log_msg("%s: sent %d bytes, data was %s\n", __FUNCTION__, ret, str.c_str());
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
        log_msg("%s: The socket for fd #%d was interupted by a system call in this thread!",
                __FUNCTION__, fd);
    }
    if (ret == -1) {
        log_error("%s: The socket for fd #%d never was available!",
            __FUNCTION__, fd);
    }
    if (ret == 0) {
        log_msg("%s: There is no data in the socket for fd #%d!",
            __FUNCTION__, fd);
    }
    if (ret > 0) {
        log_msg("%s: There is data in the socket for fd #%d!",
            __FUNCTION__, fd);
    }
    
    return ret;
}

as_value
xmlsocket_connect(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;

    as_value	method;
    as_value	val;

#ifdef GNASH_XMLSOCKET_DEBUG
    std::stringstream ss;
    fn.dump_args(ss);
    log_msg("XMLSocket.connect(%s) called", ss.str().c_str());
#endif

    boost::intrusive_ptr<xmlsocket_as_object> ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);

    if (ptr->obj.connected())
    {
        log_warning("XMLSocket.connect() called while already connected, ignored");
    }
    
    as_value hostval = fn.arg(0);
    std::string host = hostval.to_std_string(&fn.env());
    int port = int(fn.arg(1).to_number(&fn.env()));
    
    bool success = ptr->obj.connect(host.c_str(), port);
    
    // Actually, if first-stage connection was successful, we
    // should NOT invoke onConnect(true) here, but postpone
    // that event call to a second-stage connection checking,
    // to be done in a separate thread. The visible effect to
    // confirm this is that onConnect is invoked *after* 
    // XMLSocket.connect() returned in these cases.
    //
    if (fn.this_ptr->get_member("onConnect", &method))
    {
        log_msg("XMLSocket.connect(): calling onConnect");
        as_environment env;
        env.push(success);
        val = call_method(method, &env, ptr.get(), 1, env.stack_size()-1); 
    }
	    
    if ( success )
    {
        log_warning("Setting up timer for calling XMLSocket.onData()");

        Timer timer;
        boost::intrusive_ptr<builtin_function> ondata_handler = new builtin_function(&xmlsocket_event_ondata, NULL);
        unsigned interval = 50; // just make sure it's expired at every frame iteration (20 FPS used here)
        timer.setInterval(*ondata_handler, interval, boost::dynamic_pointer_cast<as_object>(ptr), &fn.env());
        VM::get().getRoot().add_interval_timer(timer);

        log_warning("Timer set");
    }

    return as_value(success);
}


as_value
xmlsocket_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<xmlsocket_as_object> ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    std::string object = fn.arg(0).to_std_string(&fn.env());
    //  log_msg("%s: host=%s, port=%g\n", __FUNCTION__, host, port);
    return as_value(ptr->obj.send(object));
}

as_value
xmlsocket_close(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<xmlsocket_as_object> ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    // Since the return code from close() doesn't get used by Shockwave,
    // we don't care either.
    ptr->obj.close();
    return as_value();
}

as_value
xmlsocket_new(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    //log_msg("%s: nargs=%d\n", __FUNCTION__, nargs);
    
    boost::intrusive_ptr<as_object> xmlsock_obj = new xmlsocket_as_object;

#ifdef GNASH_XMLSOCKET_DEBUG
    std::stringstream ss;
    fn.dump_args(ss);
    log_msg("new XMLSocket(%s) called - created object at %p", ss.str().c_str(), (void*)xmlsock_obj.get());
#else
    UNUSED(fn);
#endif

    return as_value(xmlsock_obj);
}


as_value
xmlsocket_event_ondata(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    //log_msg("doing nothing as this function is completely broken");
    //return as_value();
    
    as_value	method;
    as_value	val;
    as_value      datain;
    std::vector<const char *> msgs;
    char          *messages[200];
    int           i;
    
    boost::intrusive_ptr<xmlsocket_as_object> ptr = ensureType<xmlsocket_as_object>(fn.this_ptr);
    if ( ! ptr->obj.connected() )
    {
	    log_warning("XMLSocket not connected at xmlsocket_event_ondata call");
	    return as_value();
    }

    if (ptr->obj.processingData()) {
        log_msg("Still processing data!\n");
        return as_value(false);
    }
    
    memset(messages, 0, sizeof(char *)*200);
    
#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "memory checkpoint");
#endif
    
    assert(ptr->obj.connected());

    if (ptr->obj.anydata(messages))
    {
        //log_msg("Got message #%d, %d bytes long at %p: %s", i,
         //   strlen(messages[i]), messages[i], messages[i]);

        if (fn.this_ptr->get_member("onData", &method))
        {
            //log_msg("Got %d messages from XMLsocket", msgs.size());
            for (i=0; messages[i] != 0; i++)
            {
//              log_msg("Got message #%d, %d bytes long at %p: %s: \n", i,
//                  strlen(messages[i]), messages[i], messages[i]);
                datain = messages[i];
                //fn.env().push(datain);

#ifndef USE_DMALLOC
                //dump_memory_stats(__FUNCTION__, __LINE__, "start");
#endif
                as_environment& env = fn.env();
                env.push(datain);
                val = call_method(method, &env, fn.this_ptr.get(), 1, 0);

#ifndef USE_DMALLOC
                //dump_memory_stats(__FUNCTION__, __LINE__, "end");
#endif  
                //log_msg("Deleting message #%d at %p\n", i, messages[i]);
                //delete messages[i];
                //fn.env().pop();
                datain.set_undefined();
            }
            ptr->obj.processing(false);
        }
        else
        {
            log_error("Couldn't find onData!");
        }

        // Delete this in a batch for now so we can track memory allocation
        for (i=0; messages[i] != 0; i++)
        {
            //log_msg("Deleting message #%d at %p\n", i, messages[i]);
            delete messages[i];
        }
    }

    //malloc_trim(0);
  
    return as_value();
}

static as_object*
getXMLSocketInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL )
    {
        o = new as_object();
        attachXMLSocketInterface(*o);
    }
    return o.get();
}

static void
attachXMLSocketInterface(as_object& o)
{
    o.init_member("connect", new builtin_function(xmlsocket_connect));
    o.init_member("send", new builtin_function(xmlsocket_send));
    o.init_member("close", new builtin_function(xmlsocket_close));
}

// extern (used by Global.cpp)
void xmlsocket_class_init(as_object& global)
{
//    GNASH_REPORT_FUNCTION;
    // This is going to be the global XMLSocket "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&xmlsocket_new, getXMLSocketInterface());
        // Do not replicate all interface to class !
        //attachXMLSocketInterface(*cl);
    }
    
    // Register _global.String
    global.init_member("XMLSocket", cl.get());

}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
