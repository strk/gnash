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
#include "XML_as.h"
#include "XMLSocket_as.h"
#include "timers.h"
#include "as_function.h"
#include "fn_call.h"
#include "MovieClip.h"
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

#include <boost/scoped_array.hpp>
#include <string>

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


class XMLSocket_as : public Network, public as_object {

public:

    typedef std::vector<std::string> MessageList;

    XMLSocket_as();
    ~XMLSocket_as();
    
    bool connect(const std::string& host, short port);

    // Actionscript doesn't care about the result of either of these
    // operations.
    void send(std::string str);
    void close();

	void checkForIncomingData();

private:

    void fillMessageList(MessageList& msgs);

	/// Return the as_function with given name, converting case if needed
	boost::intrusive_ptr<as_function> getEventHandler(const std::string& name);

    bool _data;

    MessageList _messages;

    std::string _remainder;

};

  
XMLSocket_as::XMLSocket_as()
    :
    as_object(getXMLSocketInterface()),
    _data(false)
{
    attachXMLSocketProperties(*this);
}

XMLSocket_as::~XMLSocket_as()
{
}

bool
XMLSocket_as::connect(const std::string& host, short port)
{

    if ( ! URLAccessManager::allowXMLSocket(host, port) )
    {
	    return false;
    }

    bool success = createClient(host, port);

    assert( success || ! _connected );

    return success;
}

void
XMLSocket_as::close()
{
    GNASH_REPORT_FUNCTION;

    assert(_connected);

    closeNet();
    // dunno why Network::closeNet() returns false always
    // doesn't make much sense to me...
    // Anyway, let's make sure we're clean
    assert(!_sockfd);
    assert(!_connected);
}


void
XMLSocket_as::fillMessageList(MessageList& msgs)
{

    const int fd = _sockfd;
   
    if (fd <= 0) {
	log_error(_("XMLSocket: fd <= 0, returning false (timer not unregistered "
		"while socket disconnected?"));
        return;
    }

    fd_set fdset;
    struct timeval tval;
    size_t retries = 10;

    const int bufSize = 10000;
    boost::scoped_array<char> buf(new char[bufSize]);

    while (retries-- > 0) {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        
        tval.tv_sec = 0;
        tval.tv_usec = 103;
        
        const int ret = select(fd + 1, &fdset, NULL, NULL, &tval);
        
        // If interupted by a system call, try again
        if (ret == -1 && errno == EINTR) {
            log_debug(_("The socket for fd #%d was interupted by a "
                        "system call"), fd);
            continue;
        }
        if (ret == -1) {
            log_error(_("XMLSocket: The socket for fd #%d was never "
                        "available"), fd);
            return;
        }
 
        // Return if timed out.
        if (ret == 0) return;

        const size_t bytesRead = read(_sockfd, buf.get(), bufSize - 1);

        // Return if there's no data.
        if (!bytesRead) return;

        if (buf[bytesRead - 1] != 0)
        {
            // We received a partial message, so bung
            // a null-terminator on the end.
            buf[bytesRead] = 0;
        }

        char* ptr = buf.get();
        while (static_cast<size_t>(ptr - buf.get()) < bytesRead - 1)
        {
            log_debug ("read: %d, this string ends: %d",
			    bytesRead, ptr + std::strlen(ptr) - buf.get());
            // If the string reaches to the final byte read, it's
            // incomplete. Store it and continue. The buffer is 
            // NULL-terminated, so this cannot read past the end.
            if (static_cast<size_t>(
                ptr + std::strlen(ptr) - buf.get()) == bytesRead)
            {
                log_debug ("Setting remainder");
                _remainder += std::string(ptr);
                break;
            }
            if (!_remainder.empty())
            {
                log_debug ("Adding and clearing remainder");
                msgs.push_back(_remainder + std::string(ptr));
                ptr += std::strlen(ptr) + 1;
                _remainder.clear();
                continue;
            }
            
            msgs.push_back(ptr);
            ptr += std::strlen(ptr) + 1;
        }
        
    }
    
}


// XMLSocket.send doesn't return anything, so we don't need
// to here either.
void
XMLSocket_as::send(std::string str)
{
    if (!_connected)
    {
        log_error(_("XMLSocket.send(): socket not initialized"));
	    assert(_sockfd <= 0);
	    return;
    }
    
    // We have to write the NULL terminator as well.
    int ret = write(_sockfd, str.c_str(), str.size() + 1);
    
    log_debug(_("XMLSocket.send(): sent %d bytes, data was %s"), ret, str);
    return;
}


// XMLSocket.connect() returns true if the initial connection was
// successful, false if no connection was established.
as_value
xmlsocket_connect(const fn_call& fn)
{

#ifdef GNASH_XMLSOCKET_DEBUG
    std::stringstream ss;
    fn.dump_args(ss);
    log_debug(_("XMLSocket.connect(%s) called"), ss.str());
#endif

    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);

    if (ptr->connected())
    {
        log_error(_("XMLSocket.connect() called while already connected, ignored"));
    }
    
    as_value hostval = fn.arg(0);
    const std::string& host = hostval.to_string();
    int port = int(fn.arg(1).to_number());
    
    if (!ptr->connect(host, port))
    {
        return as_value(false);
        // onConnect(false) should not be called here, but rather
        // only if a failure occurs after the initial connection.
    }

    // Actually, if first-stage connection was successful, we
    // should NOT invoke onConnect(true) here, but postpone
    // that event call to a second-stage connection checking,
    // to be done in a separate thread. The visible effect to
    // confirm this is that onConnect is invoked *after* 
    // XMLSocket.connect() returned in these cases.
    // The same applies to onConnect(false), which will never
    // be called at the moment.
    //
    log_debug(_("XMLSocket.connect(): tring to call onConnect"));
    ptr->callMethod(NSV::PROP_ON_CONNECT, true);
	    

    // This is bad and should be rewritten.
    log_debug(_("Setting up timer for calling XMLSocket.onData()"));

    std::auto_ptr<Timer> timer(new Timer);
    boost::intrusive_ptr<builtin_function> ondata_handler = new builtin_function(&xmlsocket_inputChecker, NULL);
    // just make sure it's expired at every frame iteration (20 FPS used here)
    unsigned interval = 50;
    timer->setInterval(*ondata_handler, interval, boost::dynamic_pointer_cast<as_object>(ptr));

    VM& vm = ptr->getVM();
    vm.getRoot().add_interval_timer(timer, true);

    log_debug(_("Timer set"));

    return as_value(true);
}


/// XMLSocket.send()
//
/// Does not return anything.
as_value
xmlsocket_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);
    const std::string& str = fn.arg(0).to_string();
    ptr->send(str);
    return as_value();
}


/// XMLSocket.close()
//
/// Always returns void
as_value
xmlsocket_close(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);

    // If we're not connected, there's nothing to do
    if (!ptr->connected()) return as_value();

    ptr->close();
    return as_value();
}

as_value
xmlsocket_new(const fn_call& fn)
{

    boost::intrusive_ptr<as_object> xmlsock_obj = new XMLSocket_as;

#ifdef GNASH_XMLSOCKET_DEBUG
    std::stringstream ss;
    fn.dump_args(ss);
    log_debug(_("new XMLSocket(%s) called - created object at "
            "%p"), ss.str(), static_cast<void*>(xmlsock_obj.get()));
#else
    UNUSED(fn);
#endif

    return as_value(xmlsock_obj);
}


as_value
xmlsocket_inputChecker(const fn_call& fn)
{
    boost::intrusive_ptr<XMLSocket_as> ptr = ensureType<XMLSocket_as>(fn.this_ptr);
    if ( ! ptr->connected() )
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

    boost::intrusive_ptr<as_object> xml = new XML_as(xmlin);
    as_value arg(xml.get());

    ptr->callMethod(NSV::PROP_ON_XML, arg);

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

void
attachXMLSocketProperties(as_object& /*o*/)
{
    // Has no own properties
}

// extern (used by Global.cpp)
void xmlsocket_class_init(as_object& global)
{
    // This is the global XMLSocket class
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&xmlsocket_new, getXMLSocketInterface());
        // Do not replicate all interface to class!
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
    assert(_connected);
    
    std::vector<std::string> msgs;
    fillMessageList(msgs);
   
    if (msgs.empty()) return;
    
    log_debug(_("Got %d messages: "), msgs.size());

#ifdef GNASH_DEBUG
    for (size_t i = 0, e = msgs.size(); i != e; ++i)
    {
        log_debug(_(" Message %d: %s "), i, msgs[i]);
    }
#endif

    boost::intrusive_ptr<as_function> onDataHandler = getEventHandler("onData");
    if ( onDataHandler )
    {
	as_environment env(_vm); // TODO: set target !

        for (XMLSocket_as::MessageList::iterator it=msgs.begin(),
						itEnd=msgs.end();
		    it != itEnd; ++it)
        {
			std::string& s = *it;
			as_value datain( s );

			std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
			args->push_back(datain);
			
			fn_call call(this, &env, args);

			onDataHandler->call(call);

			// TODO: clear the stack ?
        }
    }

}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
