// XMLSocket_as.cpp:  Network socket for data (usually XML) transfer for Gnash.
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
#include "xml/XMLDocument_as.h"
#include "net/XMLSocket_as.h"
#include "as_function.h"
#include "movie_root.h"
#include "fn_call.h"
#include "Global_as.h"
#include "VM.h"
#include "builtin_function.h" 
#include "URLAccessManager.h"
#include "Object.h" // for getObjectInterface
#include "Global_as.h" 
#include "log.h"

#include <boost/thread.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

#undef GNASH_XMLSOCKET_DEBUG

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
/// A wrapper round a Network object that adds specific functions needed
/// by XMLSocket.
namespace {

class SocketConnection
{
public:

    SocketConnection()
        :
        _complete(false)
    {}

    /// Initiate a connection.
    void connect(const std::string& host, boost::uint16_t port) {
        _start.reset(new boost::thread(
            boost::bind(&SocketConnection::makeConnection, this, host, port)));
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
        // We have to write the null terminator as well.
        return write(_socket.getFileFd(), str.c_str(), str.size() + 1);
    }

    /// Read from the socket.
    void readMessages(std::vector<std::string>& msgs) {
        
        assert(_socket.connected());
    
        const int fd = _socket.getFileFd();
        assert(fd > 0);

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

            if (buf[bytesRead - 1] != 0) {
                // We received a partial message, so bung
                // a null-terminator on the end.
                buf[bytesRead] = 0;
            }

            char* ptr = buf.get();
            while (static_cast<size_t>(ptr - buf.get()) < bytesRead - 1) {

#ifdef GNASH_XMLSOCKET_DEBUG
                log_debug ("read: %d, this string ends: %d", bytesRead,
                        ptr + std::strlen(ptr) - buf.get());
#endif

                // If the string reaches to the final byte read, it's
                // incomplete. Store it and continue. The buffer is 
                // NULL-terminated, so this cannot read past the end.
                if (static_cast<size_t>(
                    ptr + std::strlen(ptr) - buf.get()) == bytesRead) {

                    _remainder += std::string(ptr);
                    break;
                }

                if (!_remainder.empty()) {
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
    //
    /// This also cancels any connection attempt in progress.
    void close() {
        if (_start) _start.reset();
        _socket.closeNet();
        
        // Reset for next connection.
        _complete = false;

        assert(_socket.getFileFd() <= 0);
        assert(!_socket.connected());
    }

private:

    void makeConnection(const std::string& host, boost::uint16_t port) {
        _socket.createClient(host, port);
        _complete = true;
    }

    Network _socket;

    bool _complete;

    std::string _remainder;

    boost::scoped_ptr<boost::thread> _start;

};

}

class XMLSocket_as : public as_object {

public:

    typedef std::vector<std::string> MessageList;

    XMLSocket_as();
    ~XMLSocket_as();
    
    /// True when the XMLSocket is not trying to connect.
    //
    /// If this is true but the socket is not connected, the connection
    /// has failed.
    bool ready() const {
        return _ready;
    }

    /// Whether a connection exists.
    //
    /// This is not final until ready() is true.
    bool connected() const {
        return _connection.connected();
    }

    bool connect(const std::string& host, boost::uint16_t port);

    /// Send a string with a null-terminator to the socket.
    //
    /// Actionscript doesn't care about the result.
    void send(std::string str);

    /// Close the socket
    //
    /// Actionscript doesn't care about the result.
    void close();

    /// Called on advance() when socket is connected
    virtual void advanceState();

private:

	void checkForIncomingData();
    
	/// Return the as_function with given name.
	boost::intrusive_ptr<as_function> getEventHandler(
            const std::string& name);

    /// The connection
    SocketConnection _connection;

    bool _ready;

};

  
XMLSocket_as::XMLSocket_as()
    :
    as_object(getXMLSocketInterface()),
    _ready(false)
{
}


XMLSocket_as::~XMLSocket_as()
{
    // Remove advance callback and close network connections.
    close();
}

void
XMLSocket_as::advanceState()
{
    // Wait until something has happened with the connection
    if (!_connection.complete()) return;
    
    // If this XMLSocket hadn't finished a connection, check whether it
    // has now.
    if (!ready()) {

        if (!connected()) {

            // If connection failed, notify onConnect and stop callback.
            // This means advanceState() will not be called again until
            // XMLSocket.connect() is invoked.
            callMethod(NSV::PROP_ON_CONNECT, false);
            getRoot(*this).removeAdvanceCallback(this);
            return;    
        }

        // Connection succeeded.
        callMethod(NSV::PROP_ON_CONNECT, true);
        _ready = true;
    }

    // Now the connection is established we can receive data.
    checkForIncomingData();
}


bool
XMLSocket_as::connect(const std::string& host, boost::uint16_t port)
{

    if (!URLAccessManager::allowXMLSocket(host, port)) {
	    return false;
    }

    _connection.connect(host, port);
    
    // Start callbacks on advance.
    getRoot(*this).addAdvanceCallback(this);
    
    return true;
}

void
XMLSocket_as::close()
{
    getRoot(*this).removeAdvanceCallback(this);
    _connection.close();
    _ready = false;
}


boost::intrusive_ptr<as_function>
XMLSocket_as::getEventHandler(const std::string& name)
{
	boost::intrusive_ptr<as_function> ret;

	as_value tmp;
	string_table& st = getStringTable(*this);
	if (!get_member(st.find(name), &tmp) ) return ret;
	ret = tmp.to_as_function();
	return ret;
}

void
XMLSocket_as::checkForIncomingData()
{
    assert(ready() && connected());
    
    std::vector<std::string> msgs;
    _connection.readMessages(msgs);
   
    if (msgs.empty()) return;
    
    //log_debug(_("Got %d messages: "), msgs.size());

#ifdef GNASH_XMLSOCKET_DEBUG
    for (size_t i = 0, e = msgs.size(); i != e; ++i) {
        log_debug(_(" Message %d: %s "), i, msgs[i]);
    }
#endif

    as_environment env(getVM(*this)); 

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
    if (!ready() || !connected()) {
        log_error(_("XMLSocket.send(): socket not initialized"));
	    return;
    }
    
    _connection.writeMessage(str);
    
    return;
}


// extern (used by Global.cpp)
void
xmlsocket_class_init(as_object& global, const ObjectURI& uri)
{
    // This is the global XMLSocket class
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&xmlsocket_new, getXMLSocketInterface());
    }
    
    // Register _global.XMLSocket
    global.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));

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

    if (ptr->ready()) {
        log_error(_("XMLSocket.connect() called while already "
                    "connected, ignored"));
        return as_value(false);
    }
    
    as_value hostval = fn.arg(0);
    const std::string& host = hostval.to_string();
    const double port = fn.arg(1).to_number();
    
    // Port numbers above 65535 are rejected always, but not port numbers below
    // 0. It's not clear what happens with them.
    // TODO: find out.
    // Other ports and hosts are checked against security policy before
    // acceptance or rejection.
    if (port > std::numeric_limits<boost::uint16_t>::max()) {
        return as_value(false);
    }
    
    // XMLSocket.connect() returns false only if the connection is
    // forbidden. The result of the real connection attempt is
    // notified via onConnect().
    const bool ret = ptr->connect(host, port);

    if (!ret) {
        log_error(_("XMLSocket.connect(): connection failed"));
    }

    return as_value(ret);
}


/// XMLSocket.send()
//
/// Does not return anything.
as_value
xmlsocket_send(const fn_call& fn)
{
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

    ptr->close();
    return as_value();
}

as_value
xmlsocket_new(const fn_call& /*fn*/)
{

    boost::intrusive_ptr<as_object> xmlsock_obj = new XMLSocket_as;
    return as_value(xmlsock_obj);
}

as_value
xmlsocket_onData(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
   
    boost::intrusive_ptr<XMLSocket_as> ptr = 
        ensureType<XMLSocket_as>(fn.this_ptr);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Builtin XMLSocket.onData() needs an argument"));
        );
        return as_value();
    }

    const std::string& xmlin = fn.arg(0).to_string();

#ifdef GNASH_XMLSOCKET_DEBUG
    log_debug("Arg: %s, val: %s", xmlin, fn.arg(0));
#endif

    if (xmlin.empty()) {
        log_error(_("Builtin XMLSocket.onData() called with an argument "
                        "that resolves to an empty string: %s"), fn.arg(0));
        return as_value();
    }

    boost::intrusive_ptr<as_object> xml = new XMLDocument_as(xmlin);
    as_value arg(xml.get());

    ptr->callMethod(NSV::PROP_ON_XML, arg);

    return as_value();
}

as_object*
getXMLSocketInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if (!o) {
        o = new as_object(getObjectInterface());
        attachXMLSocketInterface(*o);
    }
    return o.get();
}

void
attachXMLSocketInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("connect", gl->createFunction(xmlsocket_connect));
    o.init_member("send", gl->createFunction(xmlsocket_send));
    o.init_member("close", gl->createFunction(xmlsocket_close));


    // all this crap to satisfy swfdec testsuite... (xml-socket-properties*)
    as_object* onDataIface = new as_object(getObjectInterface());

    // It's not really a class, but a constructor function with an object
    // prototype, so looks in every way like an AS2 class.
    as_object* onDataFun = gl->createClass(xmlsocket_onData, onDataIface);
    o.init_member("onData", onDataFun);
    onDataIface->init_member(NSV::PROP_CONSTRUCTOR, onDataFun);
}

} // anonymous namespace
} // gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
