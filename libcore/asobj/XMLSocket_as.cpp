// xmlsocket.cpp:  Network socket for XML-encoded information, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "GnashSystemFDHeaders.h"
#include "network.h"
#include "utility.h"
#include "XML_as.h"
#include "XMLSocket_as.h"
#include "as_function.h"
#include "fn_call.h"
#include "VM.h"
#include "builtin_function.h" // for setting timer, should likely avoid that..
#include "URLAccessManager.h"
#include "Object.h" // for getObjectInterface
#include "log.h"
#include <boost/scoped_array.hpp>
#include <string>

#define GNASH_XMLSOCKET_DEBUG

namespace gnash {

namespace {
    as_value xmlsocket_connect(const fn_call& fn);
    as_value xmlsocket_send(const fn_call& fn);
    as_value xmlsocket_new(const fn_call& fn);
    as_value xmlsocket_close(const fn_call& fn);

    // These are the event handlers called for this object
    as_value xmlsocket_onData(const fn_call& fn);

    as_object* getXMLSocketInterface();
    void attachXMLSocketInterface(as_object& o);
}

/// Connection object
//
/// connect()
/// connected() - connected or not
/// finished connection - connection either successful or not.
/// read.
/// close.

class SocketConnection
{
public:

    SocketConnection()
        :
        _complete(false)
    {}

    /// Initiate a connection.
    void connect(const std::string& host, short port) {
        makeConnection(host, port);
    }

    /// The state of the connection.
    //
    /// Until complete() is true, this may change.
    bool connected() const {
        return _socket.connected();
    }

    /// Whether an initiated connection is finished
    //
    /// @return true if a connection attempt is complete.
    ///         The connection attempt may have failed. Check
    ///         connected() to find out.
    bool complete() const {
        return _complete;
    }

    void setComplete() {
        _complete = true;
    }

    size_t writeMessage(const std::string& str) {
        // We have to write the NULL terminator as well.
        return write(_socket.getFileFd(), str.c_str(), str.size() + 1);
    }

    /// Read from the socket.
    void readMessages(std::vector<std::string>& msgs) {
        
        assert(_socket.connected());
    
        const int fd = _socket.getFileFd();
   
        if (fd <= 0) {
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

            const size_t bytesRead = read(fd, buf.get(), bufSize - 1);

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
                    ptr + std::strlen(ptr) - buf.get()) == bytesRead) {
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
                
                // Don't do anything if nothing is received.
                msgs.push_back(ptr);
                
                ptr += std::strlen(ptr) + 1;
            }
        }
    }
    
    /// Close the connection.
    void close() {
        _socket.closeNet();
        assert(!_socket.getFileFd());
        assert(!_socket.connected());

        // Reset for next connection.
        _complete = false;
    }

private:

    void makeConnection(const std::string& host, short port) {
        _socket.createClient(host, port);
        _complete = true;
    }

    Network _socket;

    bool _complete;

    std::string _remainder;

};


class XMLSocket_as : public as_object {

public:

    typedef std::vector<std::string> MessageList;

    XMLSocket_as();
    ~XMLSocket_as();
    
    /// Is this XMLSocket connected()
    //
    /// This may not correspond to the real status.
    bool connected() const {
        return _connected;
    }

    bool connect(const std::string& host, short port);

    // Actionscript doesn't care about the result.
    void send(std::string str);

    // Actionscript doesn't care about the result.
    void close();

    virtual void advanceState();

private:

	void checkForIncomingData();
    
	/// Return the as_function with given name, converting case if needed
	boost::intrusive_ptr<as_function> getEventHandler(const std::string& name);

    SocketConnection _connection;

    bool _connected;

};

  
XMLSocket_as::XMLSocket_as()
    :
    as_object(getXMLSocketInterface())
{
}


XMLSocket_as::~XMLSocket_as()
{
    getVM().getRoot().removeAdvanceCallback(this);
}

void
XMLSocket_as::advanceState()
{
    // Wait until something has happened with the connection
    if (!_connection.complete()) return;
    
    // If this XMLSocket hadn't established a connection handle onConnect()
    if (!connected()) {

        if (!_connection.connected()) {

            /// If connection failed, notify onConnect and stop callback
            callMethod(NSV::PROP_ON_CONNECT, false);
            _vm.getRoot().removeAdvanceCallback(this);
            return;    
        }

        // Connection succeeded.
        callMethod(NSV::PROP_ON_CONNECT, true);
        _connected = true;
    }

    // Now the connection is established we can receive data.
    checkForIncomingData();
}


bool
XMLSocket_as::connect(const std::string& host, short port)
{

    if (!URLAccessManager::allowXMLSocket(host, port)) {
	    return false;
    }

    _connection.connect(host, port);
    
    // Start callbacks on advance.
    _vm.getRoot().addAdvanceCallback(this);
    
    return true;
}

void
XMLSocket_as::close()
{
    assert(connected());
    _connection.close();
    _connected = false;
}


boost::intrusive_ptr<as_function>
XMLSocket_as::getEventHandler(const std::string& name)
{
	boost::intrusive_ptr<as_function> ret;

	as_value tmp;
	string_table& st = getVM().getStringTable();
	if (!get_member(st.find(name), &tmp) ) return ret;
	ret = tmp.to_as_function();
	return ret;
}

void
XMLSocket_as::checkForIncomingData()
{
    assert(connected());
    
    std::vector<std::string> msgs;
    _connection.readMessages(msgs);
   
    if (msgs.empty()) return;
    
    log_debug(_("Got %d messages: "), msgs.size());

#ifdef GNASH_DEBUG
    for (size_t i = 0, e = msgs.size(); i != e; ++i)
    {
        log_debug(_(" Message %d: %s "), i, msgs[i]);
    }
#endif

    as_environment env(_vm); 

    for (XMLSocket_as::MessageList::const_iterator it=msgs.begin(),
                    itEnd=msgs.end(); it != itEnd; ++it) {
 
        // This should be checked on every iteration in case one call
        // changes the handler.       
        boost::intrusive_ptr<as_function> onDataHandler =
            getEventHandler("onData");

        if (!onDataHandler) break;

        const std::string& s = *it;

        std::auto_ptr<std::vector<as_value> > args(
                new std::vector<as_value>);

        args->push_back(s);
        
        fn_call call(this, env, args);

        onDataHandler->call(call);
    }

}


// XMLSocket.send doesn't return anything, so we don't need
// to here either.
void
XMLSocket_as::send(std::string str)
{
    if (!connected())
    {
        log_error(_("XMLSocket.send(): socket not initialized"));
	    //assert(_sockfd <= 0);
	    return;
    }
    
    size_t ret = _connection.writeMessage(str);
    
    log_debug(_("XMLSocket.send(): sent %d bytes, data was %s"), ret, str);
    return;
}


// extern (used by Global.cpp)
void
xmlsocket_class_init(as_object& global)
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


namespace {

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

    boost::intrusive_ptr<XMLSocket_as> ptr =
        ensureType<XMLSocket_as>(fn.this_ptr);

    if (ptr->connected())
    {
        log_error(_("XMLSocket.connect() called while already "
                    "connected, ignored"));
    }
    
    as_value hostval = fn.arg(0);
    const std::string& host = hostval.to_string();
    int port = int(fn.arg(1).to_number());
    
    const bool ret = ptr->connect(host, port);

    if (!ret) {
        log_error(_("XMLSocket.connect(): connection failed"));
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
    log_debug(_("XMLSocket.connect(): trying to call onConnect"));
	    

    return as_value(true);
}


/// XMLSocket.send()
//
/// Does not return anything.
as_value
xmlsocket_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLSocket_as> ptr =
        ensureType<XMLSocket_as>(fn.this_ptr);

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
    
    boost::intrusive_ptr<XMLSocket_as> ptr =
        ensureType<XMLSocket_as>(fn.this_ptr);

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
xmlsocket_onData(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
   
    boost::intrusive_ptr<XMLSocket_as> ptr = 
        ensureType<XMLSocket_as>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Builtin XMLSocket.onData() needs an argument"));
        );
        return as_value();
    }

    const std::string& xmlin = fn.arg(0).to_string();

    log_debug("Arg: %s, val: %s", xmlin, fn.arg(0));

    if (xmlin.empty())
    {
        log_error(_("Builtin XMLSocket.onData() called with an argument "
                        "that resolves to an empty string: %s"), fn.arg(0));
        return as_value();
    }

    boost::intrusive_ptr<as_object> xml = new XML_as(xmlin);
    as_value arg(xml.get());

    ptr->callMethod(NSV::PROP_ON_XML, arg);

    return as_value();

}

as_object*
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

void
attachXMLSocketInterface(as_object& o)
{
    o.init_member("connect", new builtin_function(xmlsocket_connect));
    o.init_member("send", new builtin_function(xmlsocket_send));
    o.init_member("close", new builtin_function(xmlsocket_close));


    // all this crap to satisfy swfdec testsuite... (xml-socket-properties*)
    as_object* onDataIface = new as_object(getObjectInterface());
    as_function* onDataFun = new builtin_function(xmlsocket_onData,
            onDataIface);
    o.init_member("onData", onDataFun);
    onDataIface->init_member(NSV::PROP_CONSTRUCTOR, onDataFun);
}

} // anonymous namespace
} // gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
