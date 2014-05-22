// XMLSocket_as.cpp:  Network socket for data (usually XML) transfer for Gnash.
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
//

#include "XMLSocket_as.h"

#include <memory>
#include <string>

#include "namedStrings.h"
#include "GnashSystemFDHeaders.h"
#include "utility.h"
#include "Socket.h"
#include "as_function.h"
#include "movie_root.h"
#include "fn_call.h"
#include "Global_as.h"
#include "VM.h"
#include "NativeFunction.h" 
#include "URLAccessManager.h"
#include "Global_as.h" 
#include "log.h"

#undef GNASH_XMLSOCKET_DEBUG

namespace gnash {

namespace {
    as_value xmlsocket_connect(const fn_call& fn);
    as_value xmlsocket_send(const fn_call& fn);
    as_value xmlsocket_new(const fn_call& fn);
    as_value xmlsocket_close(const fn_call& fn);

    // These are the event handlers called for this object
    as_value xmlsocket_onData(const fn_call& fn);

    void attachXMLSocketInterface(as_object& o);
}


class XMLSocket_as : public ActiveRelay
{
public:

    typedef std::vector<std::string> MessageList;

    XMLSocket_as(as_object* owner);
    ~XMLSocket_as();
    
    /// True only when the XMLSocket is ready for read/write.
    //
    /// This should always match the known state of the Socket.
    bool ready() const {
        return _ready;
    }

    /// Attempt a connection.
    //
    /// @param host     The host name to connect to.
    /// @param port     The port to connect to.
    /// @return         false if the connection is not allowed, otherwise true.
    ///                 Note that a return of true does not mean a successful
    ///                 connection.
    bool connect(const std::string& host, std::uint16_t port);

    /// Send a string with a null-terminator to the socket.
    //
    /// Actionscript doesn't care about the result.
    void send(std::string str);

    /// Close the XMLSocket
    //
    /// This removes the core callback and instructs the Socket to close
    /// in case it isn't already. After close() is called, the XMLSocket is
    /// no longer ready.
    //
    /// You must call close() to ensure that the Socket object is ready for
    /// a new connection.
    void close();

    /// Called on advance() when socket is connected
    //
    /// This handles reading of data and error checking. If the Socket is
    /// in error, it is closed and this XMLSocket object is no longer ready.
    virtual void update();

private:

	void checkForIncomingData();
    
    /// The connection
    Socket _socket;

    bool _ready;
    
    std::string _remainder;

};

  
XMLSocket_as::XMLSocket_as(as_object* owner)
    :
    ActiveRelay(owner),
    _ready(false)
{
}


XMLSocket_as::~XMLSocket_as()
{
}

void
XMLSocket_as::update()
{
    // This function should never be called unless a connection is active 
    // or a connection attempt is being made.

    // If the connection was not complete, check to see if it is.
    if (!ready()) {
        
        if (_socket.bad()) {
            // Connection failed during connect()
            // Notify onConnect and stop callback. This means update()
            // will not be called again until XMLSocket.connect() is invoked.
            callMethod(&owner(), NSV::PROP_ON_CONNECT, false);
            getRoot(owner()).removeAdvanceCallback(this);
            return;
        }

        // Not yet ready.
        if (!_socket.connected()) return;

        // Connection succeeded.
        _ready = true;
        callMethod(&owner(), NSV::PROP_ON_CONNECT, true);

    }

    // Now the connection is established we can receive data.
    checkForIncomingData();
}


bool
XMLSocket_as::connect(const std::string& host, std::uint16_t port)
{

    if (!URLAccessManager::allowXMLSocket(host, port)) {
	    return false;
    }

    _socket.connect(host, port);
    
    // Start callbacks on advance.
    getRoot(owner()).addAdvanceCallback(this);
    
    return true;
}

void
XMLSocket_as::close()
{
    getRoot(owner()).removeAdvanceCallback(this);
    _socket.close();
    _ready = false;
}


void
XMLSocket_as::checkForIncomingData()
{
    assert(ready());
    
    std::vector<std::string> msgs;
    
    const int bufSize = 10000;
    std::unique_ptr<char[]> buf(new char[bufSize]);

    const size_t bytesRead = _socket.readNonBlocking(buf.get(), bufSize - 1);

    // Return if there's no data.
    if (!bytesRead) return;

    if (buf[bytesRead - 1] != 0) {
        // We received a partial message, so bung
        // a null-terminator on the end.
        buf[bytesRead] = 0;
    }

    char* ptr = buf.get();
    while (static_cast<size_t>(ptr - buf.get()) < bytesRead) {

#ifdef GNASH_XMLSOCKET_DEBUG
        log_debug("read: %d, this string ends: %d", bytesRead,
                ptr + std::strlen(ptr) - buf.get());
#endif

        std::string msg(ptr);

        // If the string reaches to the final byte read, it's
        // incomplete. Store it and continue. The buffer is 
        // NULL-terminated, so this cannot read past the end.
        if (static_cast<size_t>(
            ptr + std::strlen(ptr) - buf.get()) == bytesRead) {
            _remainder += msg;
            break;
        }

        if (!_remainder.empty()) {
            msgs.push_back(_remainder + msg);
            ptr += msg.size() + 1;
            _remainder.clear();
            continue;
        }
        
        // Don't do anything if nothing is received.
        msgs.push_back(msg);
        
        ptr += msg.size() + 1;
    }
   
    if (msgs.empty()) return;
    
#ifdef GNASH_XMLSOCKET_DEBUG
    for (size_t i = 0, e = msgs.size(); i != e; ++i) {
        log_debug(" Message %d: %s ", i, msgs[i]);
    }
#endif

    for (XMLSocket_as::MessageList::const_iterator it=msgs.begin(),
                    itEnd=msgs.end(); it != itEnd; ++it) {
        callMethod(&owner(), NSV::PROP_ON_DATA, *it);
    }
    
    if (_socket.eof()) {
        callMethod(&owner(), NSV::PROP_ON_CLOSE);
        close();
        return;
    }

}

// XMLSocket.send doesn't return anything, so we don't need
// to here either.
void
XMLSocket_as::send(std::string str)
{
    if (!ready()) {
        log_error(_("XMLSocket.send(): socket not initialized"));
	    return;
    }
    
    _socket.write(str.c_str(), str.size() + 1);
    
    return;
}


// extern (used by Global.cpp)
void
xmlsocket_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, xmlsocket_new, attachXMLSocketInterface,
            nullptr, uri);
}

void
registerXMLSocketNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(xmlsocket_connect, 400, 0);
    vm.registerNative(xmlsocket_send, 400, 1);
    vm.registerNative(xmlsocket_close, 400, 2);
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
    log_debug("XMLSocket.connect(%s) called", ss.str());
#endif

    XMLSocket_as* ptr = ensure<ThisIsNative<XMLSocket_as> >(fn);

    if (ptr->ready()) {
        log_error(_("XMLSocket.connect() called while already "
                    "connected, ignored"));
        return as_value(false);
    }
 
    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("XMLSocket.connect() needs two arguments"));
        );
        // TODO: check expected return values!
        return as_value();
    }

    as_value hostval = fn.arg(0);
    const std::string& host = hostval.to_string();
    const double port = toNumber(fn.arg(1), getVM(fn));
    
    // Port numbers above 65535 are rejected always, but not port numbers below
    // 0. It's not clear what happens with them.
    // TODO: find out.
    // Other ports and hosts are checked against security policy before
    // acceptance or rejection.
    if (port > std::numeric_limits<std::uint16_t>::max()) {
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
    XMLSocket_as* ptr = ensure<ThisIsNative<XMLSocket_as> >(fn);
    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("XMLSocket.send() needs at least one argument"));
        );
        return as_value();
    }
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
    XMLSocket_as* ptr = ensure<ThisIsNative<XMLSocket_as> >(fn);
    ptr->close();
    return as_value();
}

as_value
xmlsocket_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new XMLSocket_as(obj));
    return as_value();
}

as_value
xmlsocket_onData(const fn_call& fn)
{
    const as_value& xmlin = fn.nargs ? fn.arg(0).to_string() : as_value();

    Global_as& gl = getGlobal(fn);
    as_function* ctor = getMember(gl, NSV::CLASS_XML).to_function();

    fn_call::Args args;
    args += xmlin;

    as_value xml;
    if (ctor) {
        xml = constructInstance(*ctor, fn.env(), args);
    }

    // The built-in function calls:
    //
    //      this.onXML(new XML(src));
    callMethod(fn.this_ptr, NSV::PROP_ON_XML, xml);

    return as_value();
}

void
attachXMLSocketInterface(as_object& o)
{

    VM& vm = getVM(o);
    o.init_member("connect", vm.getNative(400, 0));
    o.init_member("send", vm.getNative(400, 1));
    o.init_member("close", vm.getNative(400, 2));

    Global_as& gl = getGlobal(o);
    o.init_member("onData", gl.createFunction(xmlsocket_onData));
}

} // anonymous namespace
} // gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
