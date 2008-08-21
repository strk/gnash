// xmlsocket.cpp:  Network socket for XML-encoded information, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "network.h"
#include "utility.h"
#include "xml.h"
#include "xmlsocket.h"
#include "timers.h"
#include "as_function.h"
#include "fn_call.h"
#include "sprite_instance.h"
#include "VM.h"
#include "builtin_function.h" // for setting timer, should likely avoid that..
#include "URLAccessManager.h"
#include "Object.h" // for getObjectInterface

#include "log.h"

// For select() and read()
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#else
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
#endif

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/scoped_array.hpp>

#define GNASH_XMLSOCKET_DEBUG

namespace gnash {

static as_value xmlsocket_connect(const fn_call& fn);
static as_value xmlsocket_send(const fn_call& fn);
static as_value xmlsocket_new(const fn_call& fn);
static as_value xmlsocket_close(const fn_call& fn);

// These are the event handlers called for this object
static as_value xmlsocket_inputChecker(const fn_call& fn);

static as_value xmlsocket_onData(const fn_call& fn);

static as_object* getXMLSocketInterface();
static void attachXMLSocketInterface(as_object& o);
static void attachXMLSocketProperties(as_object& o);

class XMLSocket_as : public as_object
{

public:

        XMLSocket_as()
                :
                as_object(getXMLSocketInterface())
        {
            attachXMLSocketProperties(*this);
        }

		/// This function should be called everytime we're willing
		/// to check if any data is available on the socket.
		//
		/// The method will take care of polling from the
		/// socket and invoking any onData handler.
		///
		void checkForIncomingData();

        XMLSocket obj;

		/// Return the as_function with given name, converting case if needed
		boost::intrusive_ptr<as_function> getEventHandler(const std::string& name);

};

  
XMLSocket::XMLSocket()
    :
    _data(false),
    _xmldata(false),
    _closed(false),
    _processing(false)
{
    GNASH_REPORT_FUNCTION;
}

XMLSocket::~XMLSocket()
{
//    GNASH_REPORT_FUNCTION;
}

bool
XMLSocket::connect(const char *host, short port)
{
    GNASH_REPORT_FUNCTION;

    if ( ! URLAccessManager::allowXMLSocket(host, port) )
    {
	    return false;
    }
	    

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
XMLSocket::anydata(MessageList& msgs)
{
    //GNASH_REPORT_FUNCTION;
    assert(connected());
    assert(_sockfd > 0);
    return anydata(_sockfd, msgs);
}

bool XMLSocket::processingData()
{
    //GNASH_REPORT_FUNCTION;
    //log_debug(_("%s: processing flag is %d"), __FUNCTION__, _processing);
    return _processing;
}

void XMLSocket::processing(bool x)
{
    GNASH_REPORT_FUNCTION;
    //log_debug(_("%s: set processing flag to %d"), __FUNCTION__, x);
    _processing = x;
}

bool
XMLSocket::anydata(int fd, MessageList& msgs)
{
   
    if (fd <= 0) {
	log_error(_("%s: fd <= 0, returning false (timer not unregistered while socket disconnected?"), __FUNCTION__);
        return false;
    }


    //GNASH_REPORT_FUNCTION;


    fd_set                fdset;
    struct timeval        tval;
    size_t retries = 10;

    const int bufSize = 10000;
    boost::scoped_array<char> buf(new char[bufSize]);

    while (retries-- > 0) {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        
        tval.tv_sec = 0;
        tval.tv_usec = 103;
        
        int ret = select(fd + 1, &fdset, NULL, NULL, &tval);
        
        // If interupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
            log_debug(_("The socket for fd #%d was interupted by a system call"),
                    fd);
            continue;
        }
        if (ret == -1) {
            log_error(_("%s: The socket for fd #%d never was available"),
                __FUNCTION__, fd);
            return false;
        }
        if (ret == 0) {
            //log_debug(_("%s: There is no data in the socket for fd #%d"),
            //   __FUNCTION__, fd);
            return false;
        }
        if (ret > 0) {
            //log_debug(_("%s: There is data in the socket for fd #%d"),
            //    __FUNCTION__, fd);
        }

        processing(true);

        ret = read(_sockfd, buf.get(), bufSize - 1);
        
        if (buf[ret] != 0)
        {
            // We received a partial message, so bung
            // a null-terminator on the end.
            buf[ret + 1] = 0;
        }

        char* ptr = buf.get();
        while (ptr - buf.get() < ret )
        {
            msgs.push_back(ptr);
            ptr += strlen(ptr) + 1;
        }
        
        processing(false);
        log_debug(_("%s: Returning %d messages"), __FUNCTION__, index);
        return true;
        
    }
    
    return true;
}

bool
XMLSocket::send(std::string str)
{
    //GNASH_REPORT_FUNCTION;
    
    if ( ! connected() )
    {
        log_error(_("%s: socket not initialized"), __FUNCTION__);
	assert(_sockfd <= 0);
	return false;
    }
    
    // We have to write the NULL terminator as well.
    int ret = write(_sockfd, str.c_str(), str.size() + 1);
    
    log_debug(_("%s: sent %d bytes, data was %s"), __FUNCTION__, ret, str);
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
        log_debug(_("%s: The socket for fd #%d was interupted by a system call in this thread"),
                __FUNCTION__, fd);
    }
    if (ret == -1) {
        log_error(_("%s: The socket for fd #%d never was available"),
            __FUNCTION__, fd);
    }
    if (ret == 0) {
        log_debug(_("%s: There is no data in the socket for fd #%d"),
            __FUNCTION__, fd);
    }
    if (ret > 0) {
        log_debug(_("%s: There is data in the socket for fd #%d"),
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
    log_debug(_("XMLSocket.connect(%s) called"), ss.str().c_str());
#endif

    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);

    if (ptr->obj.connected())
    {
        log_error(_("XMLSocket.connect() called while already connected, ignored"));
    }
    
    as_value hostval = fn.arg(0);
    const std::string& host = hostval.to_string();
    int port = int(fn.arg(1).to_number());
    
    bool success = ptr->obj.connect(host.c_str(), port);

    VM& vm = ptr->getVM();
    string_table& st = vm.getStringTable();
    
    // Actually, if first-stage connection was successful, we
    // should NOT invoke onConnect(true) here, but postpone
    // that event call to a second-stage connection checking,
    // to be done in a separate thread. The visible effect to
    // confirm this is that onConnect is invoked *after* 
    // XMLSocket.connect() returned in these cases.
    //
    log_debug(_("XMLSocket.connect(): tring to call onConnect"));
    ptr->callMethod(st.find(PROPNAME("onConnect")), success);
	    
    if ( success )
    {
        log_debug(_("Setting up timer for calling XMLSocket.onData()"));

	std::auto_ptr<Timer> timer(new Timer);
        boost::intrusive_ptr<builtin_function> ondata_handler = new builtin_function(&xmlsocket_inputChecker, NULL);
        unsigned interval = 50; // just make sure it's expired at every frame iteration (20 FPS used here)
        timer->setInterval(*ondata_handler, interval, boost::dynamic_pointer_cast<as_object>(ptr));
        vm.getRoot().add_interval_timer(timer, true);

        log_debug(_("Timer set"));
    }

    return as_value(success);
}


as_value
xmlsocket_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);
    const std::string& object = fn.arg(0).to_string();
    //  log_debug(_("%s: host=%s, port=%g"), __FUNCTION__, host, port);
    return as_value(ptr->obj.send(object));
}

as_value
xmlsocket_close(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);
    // Since the return code from close() doesn't get used by Shockwave,
    // we don't care either.
    ptr->obj.close();
    return as_value();
}

as_value
xmlsocket_new(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    //log_debug(_("%s: nargs=%d"), __FUNCTION__, nargs);
    
    boost::intrusive_ptr<as_object> xmlsock_obj = new XMLSocket_as;

#ifdef GNASH_XMLSOCKET_DEBUG
    std::stringstream ss;
    fn.dump_args(ss);
    log_debug(_("new XMLSocket(%s) called - created object at %p"), ss.str().c_str(), (void*)xmlsock_obj.get());
#else
    UNUSED(fn);
#endif

    return as_value(xmlsock_obj);
}


as_value
xmlsocket_inputChecker(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;

    as_value	method;
    as_value	val;
    
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);
    if ( ! ptr->obj.connected() )
    {
        log_error(_("%s: not connected"), __FUNCTION__);
        return as_value();
    }

    ptr->checkForIncomingData();

    return as_value();
}

as_value
xmlsocket_onData(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    as_value	method;
    as_value	val;
    
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Builtin XMLSocket.onData() needs an argument"));
        );
        return as_value();
    }

    const std::string& xmlin = fn.arg(0).to_string();

    if ( xmlin.empty() )
    {
            log_error(_("Builtin XMLSocket.onData() called with an argument "
                        "that resolves to the empty string: %s"), fn.arg(0));
            return as_value();
    }

    boost::intrusive_ptr<as_object> xml = new XML(xmlin);
    as_value arg(xml.get());

    VM& vm = VM::get();
    string_table& st = vm.getStringTable();
    
    ptr->callMethod(st.find(PROPNAME("onXML")), arg);

    return as_value();


}

static as_object*
getXMLSocketInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL )
    {
        o = new as_object(getObjectInterface());
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


    // all this crap to satisfy swfdec testsuite... (xml-socket-properties*)
    as_object* onDataIface = new as_object(getObjectInterface());
    as_function* onDataFun = new builtin_function(xmlsocket_onData, onDataIface);
    o.init_member("onData", onDataFun);
    onDataIface->init_member(NSV::PROP_CONSTRUCTOR, as_value(onDataFun));
}

static void
attachXMLSocketProperties(as_object& /*o*/)
{
    // this is attached to proto
    //o.init_member("onData", new builtin_function(xmlsocket_onData));
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
        // Do not replicate all interface to class!
        //attachXMLSocketInterface(*cl);
    }
    
    // Register _global.String
    global.init_member("XMLSocket", cl.get());

}

boost::intrusive_ptr<as_function>
XMLSocket_as::getEventHandler(const std::string& name)
{
	boost::intrusive_ptr<as_function> ret;

	as_value tmp;
	string_table& st = getVM().getStringTable();
	if (!get_member(st.find(PROPNAME(name)), &tmp) ) return ret;
	ret = tmp.to_as_function();
	return ret;
}

void
XMLSocket_as::checkForIncomingData()
{
    assert(obj.connected());

    if (obj.processingData()) {
        log_debug(_("Still processing data"));
    }
    
    std::vector<std::string > msgs;
    if (obj.anydata(msgs))
    {
        log_debug(_("Got %d messages: "), msgs.size());
        for (size_t i=0; i<msgs.size(); ++i)
        {
            log_debug(_(" Message %d: %s "), i, msgs[i].c_str());
        }

        boost::intrusive_ptr<as_function> onDataHandler = getEventHandler("onData");
        if ( onDataHandler )
        {
            //log_debug(_("Got %d messages from XMLsocket"), msgs.size());
            for (XMLSocket::MessageList::iterator it=msgs.begin(),
							itEnd=msgs.end();
			    it != itEnd; ++it)
            {
				std::string& s = *it;
				as_value datain( s );

				as_environment env;
				env.push(datain);
				fn_call call(this, &env, 1, env.stack_size() - 1);
				onDataHandler->call(call);
//                call_method(as_value(onDataHandler.get()), &env, this, 1, env.stack_size()-1);

            }
            obj.processing(false);
        }
        else
        {
            log_error(_("%s: Couldn't find onData"), __FUNCTION__);
        }

    }
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
