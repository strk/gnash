// NetConnection_as.cpp:  Open local connections for FLV files or URLs.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "GnashSystemNetHeaders.h"
#include "NetConnection_as.h"
#include "log.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "movie_root.h"
#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "URL.h"
#include "VM.h"
#include "SimpleBuffer.h"
#include "namedStrings.h"
#include "GnashAlgorithm.h"
#include "fn_call.h"
#include "Global_as.h"
#include "AMFConverter.h"
#include "smart_ptr.h"
#include "RunResources.h"
#include "IOChannel.h"

#include <iostream>
#include <string>
#include <boost/scoped_ptr.hpp>

//#define GNASH_DEBUG_REMOTING

// Forward declarations.

namespace gnash {

namespace {
    void attachProperties(as_object& o);
    void attachNetConnectionInterface(as_object& o);
    as_value netconnection_isConnected(const fn_call& fn);
    as_value netconnection_uri(const fn_call& fn);
    as_value netconnection_connect(const fn_call& fn);
    as_value netconnection_close(const fn_call& fn);
    as_value netconnection_call(const fn_call& fn);
    as_value netconnection_addHeader(const fn_call& fn);
    as_value netconnection_new(const fn_call& fn);

}

namespace {

    boost::uint16_t readNetworkShort(const boost::uint8_t* buf);
    boost::uint32_t readNetworkLong(const boost::uint8_t* buf);

}

//---- ConnectionHandler -------------------------------------------------

/// Abstract connection handler class
//
/// This class abstract operations on network connections,
/// specifically RPC and streams fetching.
///
class ConnectionHandler
{
public:

    /// @param methodName
    ///     A string identifying the remote procedure to call
    ///
    /// @param responseHandler
    ///     Object to invoke response methods on.
    ///
    /// @param args
    ///     A vector of arguments
    ///
    /// @param firstArg
    ///     Index of first argument in the args vector
    ///
    ///
    /// @return true if the call is queued, false otherwise
    ///
    virtual void call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args, size_t firstArg)=0;

    /// Get an stream by name
    //
    /// @param name
    ///     Stream identifier
    ///
    virtual std::auto_ptr<IOChannel> getStream(const std::string& name);

    /// Process pending traffic, out or in bound
    //
    /// Handles all networking for NetConnection::call() and dispatches
    /// callbacks when needed. 
    ///
    /// Return true if wants to be advanced again, false otherwise.
    ///
    virtual bool advance() = 0;

    /// ConnectionHandlers may store references to as_objects
    //
    /// These include callback objects.
    virtual void setReachable() const = 0;

    /// Return true if the connection has pending calls 
    //
    /// This will be used on NetConnection.close(): if current
    /// connection has pending calls to process it will be
    /// queued and only really dropped when advance returns
    /// false
    ///
    virtual bool hasPendingCalls() const = 0;

    virtual ~ConnectionHandler() {}

protected:

    /// Construct a connection handler bound to the given NetConnection object
    //
    /// The binding is used to notify statuses and errors
    //
    /// The NetConnection_as owns all ConnectionHandlers, so there is no
    /// need to mark it reachable.
    ConnectionHandler(NetConnection_as& nc)
        :
        _nc(nc)
    {}

    // Object handling connection status messages
    NetConnection_as& _nc;
};

std::auto_ptr<IOChannel>
ConnectionHandler::getStream(const std::string&)
{
    log_unimpl("%s doesn't support fetching streams", typeName(*this));
    return std::auto_ptr<IOChannel>(0);
}

//---- HTTPRemotingHandler (HTTPConnectionHandler) -----------------------------------------------

/// Queue of remoting calls 
//
/// This class in made to handle data and do defered processing for
/// NetConnection::call()
///
/// Usage:
///
/// pass a URL to the constructor
///
/// call enqueue with a SimpleBuffer containing an encoded AMF call. If action
/// script specified a callback function, use the optional parameters to specify
/// the identifier (which must be unique) and the callback object as an as_value
///
class HTTPRemotingHandler : public ConnectionHandler
{

public:

    /// Create an handler for HTTP remoting
    //
    /// @param nc
    ///     The NetConnection AS object to send status/error events to
    ///
    /// @param url
    ///     URL to post calls to
    ///
    HTTPRemotingHandler(NetConnection_as& nc, const URL& url);

    // See dox in ConnectionHandler
    virtual bool hasPendingCalls() const
    {
        return _connection || queued_count;
    }

    // See dox in ConnectionHandler
    virtual bool advance();

    // See dox in ConnectionHandler
    virtual void setReachable() const
    {
        for (CallbacksMap::const_iterator i=callbacks.begin(),
                e=callbacks.end(); i!=e; ++i)
        {
            i->second->setReachable();
        }
    }

    // See dox in NetworkHandler class
    virtual void call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args, size_t firstArg);

private:

    static const int NCCALLREPLYCHUNK=1024*200;

    typedef std::map<std::string, as_object* > CallbacksMap;
    CallbacksMap callbacks;

    SimpleBuffer _postdata;
    URL _url;
    boost::scoped_ptr<IOChannel> _connection;
    SimpleBuffer reply;
    int reply_start;
    int queued_count;
    unsigned int _numCalls; // === queued_count ?

    // Quick hack to send Content-Type: application/x-amf
    // TODO: check if we should take headers on a per-call basis
    //       due to NetConnection.addHeader.
    //
    NetworkAdapter::RequestHeaders _headers;

    void push_amf(const SimpleBuffer &amf) 
    {
        //GNASH_REPORT_FUNCTION;

        _postdata.append(amf.data(), amf.size());
        queued_count++;
    }

    void push_callback(const std::string& id, as_object* callback)
    {
        callbacks[id] = callback;
    }

    as_object* pop_callback(const std::string& id)
    {
        CallbacksMap::iterator it = callbacks.find(id);
        if (it != callbacks.end()) {
            as_object* callback = it->second;
            callbacks.erase(it);
            return callback;
        }
        else return 0;
    }

    void enqueue(const SimpleBuffer &amf, const std::string& identifier,
                 as_object* callback)
    {
        push_amf(amf);
        push_callback(identifier, callback);
    }

    void enqueue(const SimpleBuffer &amf)
    {
        push_amf(amf);
    }
    
};

HTTPRemotingHandler::HTTPRemotingHandler(NetConnection_as& nc, const URL& url)
        :
        ConnectionHandler(nc),
        _postdata(),
        _url(url),
        _connection(0),
        reply(),
        reply_start(0),
        queued_count(0),
        _numCalls(0) // TODO: replace by queued count ?
{
    // leave space for header
    _postdata.append("\000\000\000\000\000\000", 6);
    assert(reply.size() == 0);

    _headers["Content-Type"] = "application/x-amf";
}

bool
HTTPRemotingHandler::advance()
{

#ifdef GNASH_DEBUG_REMOTING
    log_debug("advancing HTTPRemotingHandler");
#endif
    if (_connection) {

#ifdef GNASH_DEBUG_REMOTING
        log_debug("have connection");
#endif

        // Fill last chunk before reading in the next
        size_t toRead = reply.capacity() - reply.size();
        if (! toRead) toRead = NCCALLREPLYCHUNK;

#ifdef GNASH_DEBUG_REMOTING
        log_debug("Attempt to read %d bytes", toRead);
#endif

        // See if we need to allocate more bytes for the next
        // read chunk
        if (reply.capacity() < reply.size() + toRead) {
            // if _connection->size() >= 0, reserve for it, so
            // if HTTP Content-Length response header is correct
            // we'll be allocating only once for all.
            const size_t newCapacity = reply.size() + toRead;

#ifdef GNASH_DEBUG_REMOTING
            log_debug("NetConnection.call: reply buffer capacity (%d) "
                      "is too small to accept next %d bytes of chunk "
                      "(current size is %d). Reserving %d bytes.",
                reply.capacity(), toRead, reply.size(), newCapacity);
#endif

            reply.reserve(newCapacity);

#ifdef GNASH_DEBUG_REMOTING
            log_debug(" after reserve, new capacity is %d", reply.capacity());
#endif
        }

        const int read =
            _connection->readNonBlocking(reply.data() + reply.size(), toRead);

        if (read > 0) {
#ifdef GNASH_DEBUG_REMOTING
            log_debug("read '%1%' bytes: %2%", read, 
                    hexify(reply.data() + reply.size(), read, false));
#endif
            reply.resize(reply.size() + read);
        }

        // There is no way to tell if we have a whole amf reply without
        // parsing everything
        //
        // The reply format has a header field which specifies the
        // number of bytes in the reply, but potlatch sends 0xffffffff
        // and works fine in the proprietary player
        //
        // For now we just wait until we have the full reply.
        //
        // FIXME make this parse on other conditions, including: 1) when
        // the buffer is full, 2) when we have a "length in bytes" value
        // thas is satisfied

        if (_connection->bad()) {
            log_debug("connection is in error condition, calling "
                    "NetConnection.onStatus");
            reply.resize(0);
            reply_start = 0;
            // reset connection before calling the callback
            _connection.reset();

            // This is just a guess, but is better than sending
            // 'undefined'
            _nc.notifyStatus(NetConnection_as::CALL_FAILED);
        }
        else if (_connection->eof()) {

            if (reply.size() > 8) {


#ifdef GNASH_DEBUG_REMOTING
                log_debug("hit eof");
#endif
                boost::uint16_t li;
                const boost::uint8_t *b = reply.data() + reply_start;
                const boost::uint8_t *end = reply.data() + reply.size();
                
                amf::Reader rd(b, end, getGlobal(_nc.owner()));

                // parse header
                b += 2; // skip version indicator and client id

                // NOTE: this looks much like parsing of an OBJECT_AMF0
                boost::int16_t si = readNetworkShort(b);
                b += 2; // number of headers
                uint8_t headers_ok = 1;
                if (si != 0) {

#ifdef GNASH_DEBUG_REMOTING
                    log_debug("NetConnection::call(): amf headers "
                            "section parsing");
#endif
                    as_value tmp;
                    for (size_t i = si; i > 0; --i) {
                        if(b + 2 > end) {
                            headers_ok = 0;
                            break;
                        }
                        si = readNetworkShort(b); b += 2; // name length
                        if(b + si > end) {
                            headers_ok = 0;
                            break;
                        }
                        std::string headerName((char*)b, si); // end-b);
#ifdef GNASH_DEBUG_REMOTING
                        log_debug("Header name %s", headerName);
#endif
                        b += si;
                        if ( b + 5 > end ) {
                            headers_ok = 0;
                            break;
                        }
                        b += 5; // skip past bool and length long
                        if(!rd(tmp)) {
                            headers_ok = 0;
                            break;
                        }
#ifdef GNASH_DEBUG_REMOTING
                        log_debug("Header value %s", tmp);
#endif

                        { // method call for each header
                          // FIXME: it seems to me that the call should happen
                            VM& vm = getVM(_nc.owner());
                            string_table& st = vm.getStringTable();
                            string_table::key key = st.find(headerName);
#ifdef GNASH_DEBUG_REMOTING
                            log_debug("Calling NetConnection.%s(%s)",
                                    headerName, tmp);
#endif
                            callMethod(&_nc.owner(), key, tmp);
                        }
                    }
                }

                if(headers_ok == 1) {

                    si = readNetworkShort(b); b += 2; // number of replies

                    // TODO consider counting number of replies we
                    // actually parse and doing something if it
                    // doesn't match this value (does it matter?
                    if(si > 0) {
                        // parse replies until we get a parse error or we reach the end of the buffer
                        while(b < end) {
                            if(b + 2 > end) break;
                            si = readNetworkShort(b); b += 2; // reply length
                            if(si < 4) { // shorted valid response is '/1/a'
                                log_error("NetConnection::call(): reply message name too short");
                                break;
                            }
                            if(b + si > end) break;

                            // Reply message is: '/id/methodName'

                            int ns = 1; // next slash position
                            while (ns<si-1 && *(b+ns) != '/') ++ns;
                            if ( ns >= si-1 ) {
                                std::string msg(
                                        reinterpret_cast<const char*>(b), si);
                                log_error("NetConnection::call(): invalid "
                                        "reply message name (%s)", msg);
                                break;
                            }

                            std::string id(reinterpret_cast<const char*>(b),
                                    ns);

                            std::string methodName(
                                    reinterpret_cast<const char*>(b+ns+1),
                                    si-ns-1);

                            b += si;

                            // parse past unused string in header
                            if(b + 2 > end) break;
                            si = readNetworkShort(b); b += 2; // reply length
                            if(b + si > end) break;
                            b += si;

                            // this field is supposed to hold the
                            // total number of bytes in the rest of
                            // this particular reply value, but
                            // openstreetmap.org (which works great
                            // in the adobe player) sends
                            // 0xffffffff. So we just ignore it
                            if(b + 4 > end) break;
                            li = readNetworkLong(b); b += 4; // reply length

#ifdef GNASH_DEBUG_REMOTING
                            log_debug("about to parse amf value");
#endif
                            // this updates b to point to the next unparsed byte
                            as_value replyval;
                            if (!rd(replyval)) {
                                log_error("parse amf failed");
                                // this will happen if we get
                                // bogus data, or if the data runs
                                // off the end of the buffer
                                // provided, or if we get data we
                                // don't know how to parse
                                break;
                            }
#ifdef GNASH_DEBUG_REMOTING
                            log_debug("parsed amf");
#endif

                            // update variable to show how much we've parsed
                            reply_start = b - reply.data();

                            // if actionscript specified a callback object,
                            // call it
                            as_object* callback = pop_callback(id);
                            if (callback) {

                                string_table::key methodKey;
                                if ( methodName == "onResult" ) {
                                    methodKey = NSV::PROP_ON_RESULT;
                                }
                                else if (methodName == "onStatus") {
                                    methodKey = NSV::PROP_ON_STATUS;
                                }
                                else {
                                    // NOTE: the pp is known to actually
                                    // invoke the custom method, but with 7
                                    // undefined arguments (?)
                                    log_error("Unsupported HTTP Remoting "
                                            "response callback: '%s' "
                                            "(size %d)", methodName,
                                            methodName.size());
                                    continue;
                                }

#ifdef GNASH_DEBUG_REMOTING
                                log_debug("calling onResult callback");
#endif
                                // FIXME check if above line can fail and we
                                // have to react
                                callMethod(callback, methodKey, replyval);
#ifdef GNASH_DEBUG_REMOTING
                                log_debug("callback called");
#endif
                            } else {
                                log_error("Unknown HTTP Remoting response identifier '%s'", id);
                            }
                        }
                    }
                }
            }
            else
            {
                log_error("Response from remoting service < 8 bytes");
            }

#ifdef GNASH_DEBUG_REMOTING
            log_debug("deleting connection");
#endif
            _connection.reset();
            reply.resize(0);
            reply_start = 0;
        }
    }

    if(!_connection && queued_count > 0) {
//#ifdef GNASH_DEBUG_REMOTING
        log_debug("creating connection");
//#endif
        // set the "number of bodies" header

        (reinterpret_cast<boost::uint16_t*>(_postdata.data() + 4))[0] = htons(queued_count);
        std::string postdata_str(reinterpret_cast<char*>(_postdata.data()), _postdata.size());
#ifdef GNASH_DEBUG_REMOTING
        log_debug("NetConnection.call(): encoded args from %1% calls: %2%", queued_count, hexify(postdata.data(), postdata.size(), false));
#endif
        queued_count = 0;

        // TODO: it might be useful for a Remoting Handler to have a 
        // StreamProvider member
        const StreamProvider& sp =
            getRunResources(_nc.owner()).streamProvider();

        _connection.reset(sp.getStream(_url, postdata_str, _headers).release());

        _postdata.resize(6);
#ifdef GNASH_DEBUG_REMOTING
        log_debug("connection created");
#endif
    }

    if (_connection == 0) {
        // nothing more to do
        return false;
    }

    return true;
}

void
HTTPRemotingHandler::call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args, size_t firstArg)
{
    SimpleBuffer buf(32);

    // method name
    buf.appendNetworkShort(methodName.size());
    buf.append(methodName.c_str(), methodName.size());

    // client id (result number) as counted string
    // the convention seems to be / followed by a unique (ascending) number
    std::ostringstream os;
    os << "/";
    // Call number is not used if the callback is undefined
    if (asCallback) {
        os << ++_numCalls; 
    }
    const std::string callNumberString = os.str();

    buf.appendNetworkShort(callNumberString.size());
    buf.append(callNumberString.c_str(), callNumberString.size());

    size_t total_size_offset = buf.size();
    buf.append("\000\000\000\000", 4); // total size to be filled in later

    // encode array of arguments to remote method
    buf.appendByte(amf::STRICT_ARRAY_AMF0);
    buf.appendNetworkLong(args.size() - firstArg);

    // STRICT_ARRAY encoding is allowed for remoting
    amf::Writer w(buf, true);

    for (unsigned int i = firstArg; i < args.size(); ++i)
    {
        const as_value& arg = args[i];
        if (!arg.writeAMF0(w)) {
            log_error("Could not serialize NetConnection.call argument %d",
                    i);
        }
    }

    // Set the "total size" parameter.
    *(reinterpret_cast<uint32_t*>(buf.data() + total_size_offset)) = 
        htonl(buf.size() - 4 - total_size_offset);

#ifdef GNASH_DEBUG_REMOTING
    log_debug(_("NetConnection.call(): encoded args: %s"),
            hexify(buf.data(), buf.size(), false));
#endif

    if (asCallback) {
#ifdef GNASH_DEBUG_REMOTING
        log_debug("calling enqueue with callback");
#endif
        enqueue(buf, callNumberString, asCallback);
    }
    
    else {
#ifdef GNASH_DEBUG_REMOTING
        log_debug("calling enqueue without callback");
#endif
        enqueue(buf);
    }
}

//----- NetConnection_as ----------------------------------------------------

NetConnection_as::NetConnection_as(as_object* owner)
    :
    ActiveRelay(owner),
    _queuedConnections(),
    _currentConnection(0),
    _uri(),
    _isConnected(false)
{
}

// here to have HTTPRemotingHandler definition available
NetConnection_as::~NetConnection_as()
{
    deleteChecked(_queuedConnections.begin(), _queuedConnections.end());
}

// extern (used by Global.cpp)
void
netconnection_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, netconnection_new,
            attachNetConnectionInterface, 0, uri);
}


void
NetConnection_as::markReachableResources() const
{
    owner().setReachable();

    std::for_each(_queuedConnections.begin(), _queuedConnections.end(),
            std::mem_fun(&ConnectionHandler::setReachable));

    if (_currentConnection.get()) _currentConnection->setReachable();
}


/// FIXME: this should not use _uri, but rather take a URL argument.
/// Validation should probably be done on connect() only and return a 
/// bool indicating validity. That can be used to return a failure
/// for invalid or blocked URLs.
std::string
NetConnection_as::validateURL() const
{

    URL uri(_uri, getRunResources(owner()).baseURL());

    std::string uriStr(uri.str());
    assert(uriStr.find("://") != std::string::npos);

    // Check if we're allowed to open url
    if (!URLAccessManager::allow(uri)) {
        log_security(_("Gnash is not allowed to open this url: %s"), uriStr);
        return "";
    }

    log_debug(_("Connection to movie: %s"), uriStr);

    return uriStr;
}

void
NetConnection_as::notifyStatus(StatusCode code)
{
    std::pair<std::string, std::string> info;
    getStatusCodeInfo(code, info);

    /// This is a new normal object each time (see NetConnection.as)
    as_object* o = getGlobal(owner()).createObject();

    const int flags = 0;

    o->init_member("code", info.first, flags);
    o->init_member("level", info.second, flags);

    callMethod(&owner(), NSV::PROP_ON_STATUS, o);

}

void
NetConnection_as::getStatusCodeInfo(StatusCode code, NetConnectionStatus& info)
{
    /// The Call statuses do exist, but this implementation is a guess.
    switch (code)
    {
        case CONNECT_SUCCESS:
            info.first = "NetConnection.Connect.Success";
            info.second = "status";
            return;

        case CONNECT_FAILED:
            info.first = "NetConnection.Connect.Failed";
            info.second = "error";
            return;

        case CONNECT_APPSHUTDOWN:
            info.first = "NetConnection.Connect.AppShutdown";
            info.second = "error";
            return;

        case CONNECT_REJECTED:
            info.first = "NetConnection.Connect.Rejected";
            info.second = "error";
            return;

        case CALL_FAILED:
            info.first = "NetConnection.Call.Failed";
            info.second = "error";
            return;

        case CALL_BADVERSION:
            info.first = "NetConnection.Call.BadVersion";
            info.second = "status";
            return;

        case CONNECT_CLOSED:
            info.first = "NetConnection.Connect.Closed";
            info.second = "status";
    }

}


/// Called on NetConnection.connect(null).
//
/// The status notification happens immediately, isConnected becomes true.
void
NetConnection_as::connect()
{
    // Close any current connections.
    close();
    _isConnected = true;
    notifyStatus(CONNECT_SUCCESS);
}


void
NetConnection_as::connect(const std::string& uri)
{
    // Close any current connections. (why?) Because that's what happens.
    close();

    // TODO: check for other kind of invalidities here...
    if ( uri.empty() )
    {
        _isConnected = false;
        notifyStatus(CONNECT_FAILED);
        return;
    }

    URL url(uri, getRunResources(owner()).baseURL());

    if ((url.protocol() != "rtmp")
        && (url.protocol() != "rtmpt")
        && (url.protocol() != "rtmpts")
        && (url.protocol() != "https")
        && (url.protocol() != "http")) {

        IF_VERBOSE_ASCODING_ERRORS(
		 log_aserror("NetConnection.connect(%s): invalid connection "
			     "protocol", url);
				   );
        notifyStatus(CONNECT_FAILED);
        return;
    }
    
    // This is for HTTP remoting

    if (!URLAccessManager::allow(url)) {
        log_security(_("Gnash is not allowed to NetConnection.connect "
                    "to %s"), url);
        notifyStatus(CONNECT_FAILED);
        return;
    }

    _currentConnection.reset(new HTTPRemotingHandler(*this, url));


    // FIXME: We should attempt a connection here (this is called when an
    // argument is passed to NetConnection.connect(url).
    // Would probably return true on success and set isConnected.
    //
    // Under certain circumstances, an an immediate failure notification
    // happens. These are:
    // a) sandbox restriction
    // b) invalid URL? NetConnection.connect(5) fails straight away, but
    //    could be either because a URL has to be absolute, perhaps including
    //    a protocol, or because the load is attempted from the filesystem
    //    and fails immediately.
    // TODO: modify validateURL for doing this.
    _isConnected = false;
}


/// FIXME: This should close an active connection as well as setting the
/// appropriate properties.
void
NetConnection_as::close()
{
    bool needSendClosedStatus = _currentConnection.get() || _isConnected;

    /// Queue the current call queue if it has pending calls
    if ( _currentConnection.get() && _currentConnection->hasPendingCalls() )
    {
        _queuedConnections.push_back(_currentConnection.release());
    }

    /// TODO: what should actually happen here? Should an attached
    /// NetStream object be interrupted?
    _isConnected = false;

    if ( needSendClosedStatus )
    {
        notifyStatus(CONNECT_CLOSED);
    }
}


void
NetConnection_as::setURI(const std::string& uri)
{
    owner().init_readonly_property("uri", &netconnection_uri);
    _uri = uri;
}

void
NetConnection_as::call(as_object* asCallback, const std::string& methodName,
        const std::vector<as_value>& args, size_t firstArg)
{
    if ( ! _currentConnection.get() )
    {
        log_aserror("NetConnection.call: can't call while not connected");
        return;
    }

    _currentConnection->call(asCallback, methodName, args, firstArg);

#ifdef GNASH_DEBUG_REMOTING
    log_debug("called enqueue");
#endif

    startAdvanceTimer();

}

std::auto_ptr<IOChannel>
NetConnection_as::getStream(const std::string& name)
{
    const RunResources& ri = getRunResources(owner());

    const StreamProvider& streamProvider = ri.streamProvider();

    // Construct URL with base URL (assuming not connected to RTMP server..)
    // TODO: For RTMP return the named stream from an existing RTMP connection.
    // If name is a full or relative URL passed from NetStream.play(), it
    // must be constructed against the base URL, not the NetConnection uri,
    // which should always be null in this case.
    const URL url(name, ri.baseURL());

    const RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    return streamProvider.getStream(url, rcfile.saveStreamingMedia());

}

void
NetConnection_as::startAdvanceTimer() 
{
    getRoot(owner()).addAdvanceCallback(this);
    log_debug("startAdvanceTimer: registered NetConnection timer");
}

void
NetConnection_as::stopAdvanceTimer() 
{
    getRoot(owner()).removeAdvanceCallback(this);
    log_debug("stopAdvanceTimer: deregistered NetConnection timer");
}

void
NetConnection_as::update()
{
    // Advance

#ifdef GNASH_DEBUG_REMOTING
    log_debug("NetConnection_as::advance: %d calls to advance",
            _queuedConnections.size());
#endif

    while (!_queuedConnections.empty()) {
        ConnectionHandler* ch = _queuedConnections.front();

        if (!ch->advance()) {
            log_debug("ConnectionHandler done, dropping");
            _queuedConnections.pop_front();
            delete ch;
        }

        else break; // queues handling is serialized
    }

    if (_currentConnection.get()) {
        _currentConnection->advance();
    }

    // Advancement of a connection might trigger creation
    // of a new connection, so we won't stop the advance
    // timer in that case
    if (_queuedConnections.empty() && ! _currentConnection.get()) {
#ifdef GNASH_DEBUG_REMOTING
        log_debug("stopping advance timer");
#endif
        stopAdvanceTimer();
#ifdef GNASH_DEBUG_REMOTING
        log_debug("advance timer stopped");
#endif
    }
}

/// Anonymous namespace for NetConnection AMF-reading helper functions
/// (shouldn't be here).

namespace {

boost::uint16_t
readNetworkShort(const boost::uint8_t* buf) {
    boost::uint16_t s = buf[0] << 8 | buf[1];
    return s;
}

boost::uint32_t
readNetworkLong(const boost::uint8_t* buf) {
    boost::uint32_t s = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    return s;
}

}


/// Anonymous namespace for NetConnection interface implementation.

namespace {


/// NetConnection.call()
//
/// Documented to return void, and current tests suggest this might be
/// correct, though they don't test with any calls that might succeed.
as_value
netconnection_call(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetConnection.call(): needs at least one argument"));
        );
        return as_value(); 
    }

    const as_value& methodName_as = fn.arg(0);
    std::string methodName = methodName_as.to_string();

#ifdef GNASH_DEBUG_REMOTING
    std::stringstream ss; fn.dump_args(ss);
    log_debug("NetConnection.call(%s)", ss.str());
#endif

    // TODO: arg(1) is the response object. let it know when data comes back
    boost::intrusive_ptr<as_object> asCallback;
    if (fn.nargs > 1) {

        if (fn.arg(1).is_object()) {
            asCallback = (fn.arg(1).to_object(getGlobal(fn)));
        }
        else {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror("NetConnection.call(%s): second argument must be "
                    "an object", ss.str());
            );
        }
    }

    const std::vector<as_value>& args = fn.getArgs();
    ptr->call(asCallback.get(), methodName, args, 2);

    return as_value();
}

as_value
netconnection_close(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);

    ptr->close();

    return as_value();
}


/// Read-only
as_value
netconnection_isConnected(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);

    return as_value(ptr->isConnected());
}

as_value
netconnection_uri(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);

    return as_value(ptr->getURI());
}

void
attachNetConnectionInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);

    o.init_member("connect", gl.createFunction(netconnection_connect));
    o.init_member("addHeader", gl.createFunction(netconnection_addHeader));
    o.init_member("call", gl.createFunction(netconnection_call));
    o.init_member("close", gl.createFunction(netconnection_close));
}

void
attachProperties(as_object& o)
{
    o.init_readonly_property("isConnected", &netconnection_isConnected);
}

/// \brief callback to instantiate a new NetConnection object.
/// \param fn the parameters from the Flash movie
/// \return nothing from the function call.
/// \note The return value is returned through the fn.result member.
as_value
netconnection_new(const fn_call& fn)
{

    as_object* obj = fn.this_ptr;
    obj->setRelay(new NetConnection_as(obj));
    attachProperties(*obj);
    return as_value();
}


/// For rtmp, NetConnect.connect() takes an RTMP URL. For all other streams,
/// it takes null or undefined.
//
/// RTMP is untested.
//
/// For non-rtmp streams:
//
/// Returns undefined if there are no arguments, true if the first
/// argument is null, otherwise the result of the attempted connection.
/// Undefined is also a valid argument for SWF7 and above.
//
/// The isConnected property is set to the result of connect().
as_value
netconnection_connect(const fn_call& fn)
{

    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);
    
    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetConnection.connect(): needs at least "
                    "one argument"));
        );
        return as_value();
    }

    const as_value& uri = fn.arg(0);

    const VM& vm = getVM(fn);
    const std::string& uriStr = uri.to_string(vm.getSWFVersion());
    
    // This is always set without validification.
    ptr->setURI(uriStr);

    // Check first arg for validity 
    if (uri.is_null() || (getSWFVersion(fn) > 6 && uri.is_undefined())) {
        ptr->connect();
    }
    else {
        if ( fn.nargs > 1 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_unimpl("NetConnection.connect(%s): args after the first are "
                    "not supported", ss.str());
        }
        ptr->connect(uriStr);
    }

    return as_value(ptr->isConnected());

}


as_value
netconnection_addHeader(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);
    UNUSED(ptr);

    log_unimpl("NetConnection.addHeader()");
    return as_value();
}

} // anonymous namespace

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
