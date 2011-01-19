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

#include "NetConnection_as.h"

#include <string>
#include <utility>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/mem_fn.hpp>
#include <iomanip>

#include "GnashSystemNetHeaders.h"
#include "log.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "movie_root.h"
#include "StreamProvider.h"
#include "URL.h"
#include "VM.h"
#include "SimpleBuffer.h"
#include "namedStrings.h"
#include "GnashAlgorithm.h"
#include "fn_call.h"
#include "Global_as.h"
#include "AMFConverter.h"
#include "AMF.h"
#include "smart_ptr.h"
#include "RunResources.h"
#include "IOChannel.h"
#include "RTMP.h"

//#define GNASH_DEBUG_REMOTING

namespace gnash {

// Forward declarations.
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
    as_value local_onResult(const fn_call& fn);
    std::pair<std::string, std::string>
        getStatusCodeInfo(NetConnection_as::StatusCode code);

    /// Parse and send any invoke messages from an HTTP connection.
    void handleAMFInvoke(amf::Reader& rd, const boost::uint8_t*& b,
            const boost::uint8_t* end, as_object& owner);

    void replyBWCheck(rtmp::RTMP& r, double txn);

}

/// Abstract connection handler class
//
/// This class abstract operations on network connections,
/// specifically RPC and streams fetching.
class Connection : boost::noncopyable
{
public:
    
    typedef std::map<size_t, as_object*> CallbacksMap;

    /// @param methodName      A string identifying the remote procedure to call
    /// @param responseHandler  Object to invoke response methods on.
    /// @param args             A vector of arguments
    virtual void call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args) = 0;

    /// Get an stream by name
    //
    /// @param name     Stream identifier
    virtual std::auto_ptr<IOChannel> getStream(const std::string& /*name*/) {
        log_unimpl("%s doesn't support fetching streams", typeName(*this));
        return std::auto_ptr<IOChannel>(0);
    }

    /// Process pending traffic, out or in bound
    //
    /// Handles all networking for NetConnection::call() and dispatches
    /// callbacks when needed. 
    //
    /// @return         false if no further advance is needed. The only
    ///                 case for this is an error.
    virtual bool advance() = 0;

    /// Connections may store references to as_objects
    void setReachable() const {
        foreachSecond(_callbacks.begin(), _callbacks.end(),
                std::mem_fun(&as_object::setReachable));
    }

    /// Return true if the connection has pending calls 
    //
    /// This will be used on NetConnection.close(): if current
    /// connection has pending calls to process it will be
    /// queued and only really dropped when advance returns false
    virtual bool hasPendingCalls() const = 0;

    size_t callNo() {
        return ++_numCalls;
    }

    virtual ~Connection() {}

    as_object* popCallback(size_t id) {
        CallbacksMap::iterator it = _callbacks.find(id);
        if (it != _callbacks.end()) {
            as_object* callback = it->second;
            _callbacks.erase(it);
            return callback;
        }
        return 0;
    }

protected:

    /// Construct a connection handler bound to the given NetConnection object
    //
    /// The binding is used to notify statuses and errors
    //
    /// The NetConnection_as owns all Connections, so there is no
    /// need to mark it reachable.
    Connection(NetConnection_as& nc)
        :
        _nc(nc),
        _numCalls(0)
    {
    }

    void pushCallback(size_t id, as_object* callback) {
        _callbacks[id] = callback;
    }

    // Object handling connection status messages
    NetConnection_as& _nc;

private:

    CallbacksMap _callbacks;

    size_t _numCalls;
};

// Connection types.
namespace {

class HTTPRequest
{
public:
    HTTPRequest(Connection& h)
        :
        _handler(h),
        _calls(0)
    {
        // leave space for header
        _data.append("\000\000\000\000\000\000", 6);
        _headers["Content-Type"] = "application/x-amf";
    }

    /// Add AMF data to this request.
    void addData(const SimpleBuffer& amf) {
        _data.append(amf.data(), amf.size());
        ++_calls;
    }

    void send(const URL& url, NetConnection_as& nc);

    bool process(NetConnection_as& nc);

private:

    static const size_t NCCALLREPLYCHUNK = 1024 * 200;

    /// Handle replies to server functions we invoked with a callback.
    //
    /// This needs access to the stored callbacks.
    void handleAMFReplies(amf::Reader& rd, const boost::uint8_t*& b,
            const boost::uint8_t* end);

    Connection& _handler;

    /// The data to be sent by POST with this request.
    SimpleBuffer _data;

    /// A buffer for the reply.
    SimpleBuffer _reply;

    /// The number of separate remoting calls to be encoded in this request.
    size_t _calls;

    /// A single HTTP request.
    boost::scoped_ptr<IOChannel> _connection;
    
    /// Headers to be sent with this request.
    NetworkAdapter::RequestHeaders _headers;

};

/// Queue of remoting calls 
//
/// This is a single conception HTTP remoting connection, which in reality
/// comprises a queue of separate HTTP requests.
class HTTPConnection : public Connection
{
public:
    /// Create a handler for HTTP remoting
    //
    /// @param nc   The NetConnection AS object to send status/error events to
    /// @param url  URL to post calls to
    HTTPConnection(NetConnection_as& nc, const URL& url)
        :
        Connection(nc),
        _url(url)
    {
    }

    virtual bool hasPendingCalls() const {
        return _currentRequest.get() || !_requestQueue.empty();
    }

    /// Queue current request, process all live requests.
    virtual bool advance();

    /// Check if there is a current request. If not, make one.
    virtual void call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args);

private:

    const URL _url;

    /// The queue of sent requests.
    std::vector<boost::shared_ptr<HTTPRequest> > _requestQueue;

    /// The current request.
    boost::shared_ptr<HTTPRequest> _currentRequest;
    
};

class RTMPConnection : public Connection
{
public:

    RTMPConnection(NetConnection_as& nc, const URL& url)
        :
        Connection(nc),
        _connectionComplete(false),
        _url(url)
    {
        // Throw exception if this fails.
        const bool ret = _rtmp.connect(url);
        if (!ret) throw GnashException("Connection failed");
    }

    virtual void call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args)
    {
        SimpleBuffer buf;
        amf::Writer aw(buf);
        aw.writeString(methodName);
        const size_t id = asCallback ? callNo() : 0;
        aw.writeNumber(id);

        for (size_t i = 0; i < args.size(); ++i) {
            args[i].writeAMF0(aw);
        }
        _rtmp.call(buf);
        if (asCallback) {
            pushCallback(id, asCallback);
        }
    }

    bool hasPendingCalls() const {
        return false;
    }

    virtual bool advance() {

        _rtmp.update();

        if (_rtmp.error() && !_connectionComplete) {
            _nc.notifyStatus(NetConnection_as::CONNECT_FAILED);
            return false;
        }
        if (_connectionComplete && _rtmp.error()) {
            _nc.notifyStatus(NetConnection_as::CONNECT_CLOSED);
            return false;
        }

        // Nothing to do yet, but we don't want to be dropped.
        if (!_connectionComplete) {

            if (!_rtmp.connected()) return true;
            
            _connectionComplete = true;
            log_debug("Initial connection complete");

            const RunResources& r = getRunResources(_nc.owner());
            Global_as& gl = getGlobal(_nc.owner());
 
            // Connection object
            as_object* o = createObject(gl);

            const int flags = 0;
            o->init_member("app", _url.path().substr(1), flags);

            // TODO: check where it gets these data from.
            o->init_member("flashVer", getVM(_nc.owner()).getPlayerVersion(),
                    flags);
            o->init_member("swfUrl", r.streamProvider().baseURL().str(),
                    flags);
            o->init_member("tcUrl", _url.str(), flags);

            // TODO: implement this properly
            o->init_member("fpad", false, flags);
            o->init_member("capabilities", 15.0, flags);
            o->init_member("audioCodecs", 3191.0, flags);
            o->init_member("videoCodecs", 252.0, flags);
            o->init_member("videoFunction", 1.0, flags);
            o->init_member("pageUrl", as_value(), flags);

            // Set up the callback object.
            as_object* cb = createObject(getGlobal(_nc.owner()));
            cb->init_member(NSV::PROP_ON_RESULT,
                    gl.createFunction(local_onResult), 0);

            cb->init_member("_conn", &_nc.owner(), 0);

            std::vector<as_value> args;
            args.push_back(o);

            call(cb, "connect", args);

            // Send bandwidth check; the pp appears to do this
            // automatically.
            sendServerBW(_rtmp);

        }
        
        boost::shared_ptr<SimpleBuffer> b = _rtmp.getMessage();

        if (b && !_nc.isConnected()) {
            _nc.setConnected();
        }

        /// Retrieve messages.
        while (b.get()) {
            handleInvoke(b->data() + rtmp::RTMPHeader::headerSize,
                    b->data() + b->size());
            b = _rtmp.getMessage();
        }

        return true;
    }

private:

    void handleInvoke(const boost::uint8_t* payload, const boost::uint8_t* end);

    rtmp::RTMP _rtmp;
    bool _connectionComplete;
    const URL _url;

};

} // anonymous namespace

NetConnection_as::NetConnection_as(as_object* owner)
    :
    ActiveRelay(owner),
    _isConnected(false)
{
}

// here to have HTTPConnection definition available
NetConnection_as::~NetConnection_as()
{
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
    std::for_each(_oldConnections.begin(), _oldConnections.end(),
            boost::mem_fn(&Connection::setReachable));
    if (_currentConnection.get()) _currentConnection->setReachable();
}

/// FIXME: this should not use _uri, but rather take a URL argument.
/// Validation should probably be done on connect() only and return a 
/// bool indicating validity. That can be used to return a failure
/// for invalid or blocked URLs.
std::string
NetConnection_as::validateURL() const
{
    const RunResources& r = getRunResources(owner());
    URL uri(_uri, r.streamProvider().baseURL());

    std::string uriStr(uri.str());
    assert(uriStr.find("://") != std::string::npos);

    // Check if we're allowed to open url
    if (!r.streamProvider().allow(uri)) {
        log_security(_("Gnash is not allowed to open this url: %s"), uriStr);
        return "";
    }

    log_debug(_("Connection to movie: %s"), uriStr);

    return uriStr;
}

void
NetConnection_as::notifyStatus(StatusCode code)
{
    std::pair<std::string, std::string> info = getStatusCodeInfo(code);

    /// This is a new normal object each time (see NetConnection.as)
    as_object* o = createObject(getGlobal(owner()));

    const int flags = 0;

    o->init_member("code", info.first, flags);
    o->init_member("level", info.second, flags);

    callMethod(&owner(), NSV::PROP_ON_STATUS, o);
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


bool
NetConnection_as::connect(const std::string& uri)
{
    // Close any current connections. 
    close();
    assert(!_isConnected);

    // TODO: check for other kind of invalidities here...
    if (uri.empty()) {
        notifyStatus(CONNECT_FAILED);
        return false;
    }
    
    const RunResources& r = getRunResources(owner());
    URL url(_uri, r.streamProvider().baseURL());

    if (!r.streamProvider().allow(url)) {
        log_security(_("Gnash is not allowed to connect " "to %s"), url);
        notifyStatus(CONNECT_FAILED);
        return false;
    }

    // Attempt connection.
    if (url.protocol() == "https" || url.protocol() == "http") {
        _currentConnection.reset(new HTTPConnection(*this, url));
    }
    else if (url.protocol() == "rtmp") {
        try {
            _currentConnection.reset(new RTMPConnection(*this, url));
        }
        catch (const GnashException&) {
            // This happens if the connect cannot even be attempted.
            notifyStatus(CONNECT_FAILED);
            return false;
        }
        startAdvanceTimer();
    }
    else if (url.protocol() == "rtmpt" || url.protocol() == "rtmpts") {
        log_unimpl("NetConnection.connect(%s): unsupported connection "
                 "protocol", url);
        notifyStatus(CONNECT_FAILED);
        return false;
    }
    else {
        log_error("NetConnection.connect(%s): unknown connection "
             "protocol", url);
        notifyStatus(CONNECT_FAILED);
        return false;
    }
    return true;
}


/// FIXME: This should close an active connection as well as setting the
/// appropriate properties.
void
NetConnection_as::close()
{
    // Send close event if a connection is in progress or connected is true.
    const bool needSendClosedStatus = _currentConnection.get() || _isConnected;

    /// Queue the current call queue if it has pending calls
    if (_currentConnection.get() && _currentConnection->hasPendingCalls()) {
        boost::shared_ptr<Connection> c(_currentConnection.release());
        _oldConnections.push_back(c);
    }

    /// TODO: what should actually happen here? Should an attached
    /// NetStream object be interrupted?
    _isConnected = false;

    if (needSendClosedStatus) {
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
        const std::vector<as_value>& args)
{
    if (!_currentConnection.get()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("NetConnection.call: can't call while not connected");
        );
        return;
    }

    _currentConnection->call(asCallback, methodName, args);

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
    const RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    URL url(name, streamProvider.baseURL());

    return streamProvider.getStream(url, rcfile.saveStreamingMedia());

}

void
NetConnection_as::startAdvanceTimer() 
{
    getRoot(owner()).addAdvanceCallback(this);
}

void
NetConnection_as::stopAdvanceTimer() 
{
    getRoot(owner()).removeAdvanceCallback(this);
}

void
NetConnection_as::update()
{

    // Handle unfinished actions in any closed connections. Currently we
    // only expect HTTP connections here, as RTMP is closed immediately.
    // This may change!
    for (Connections::iterator i = _oldConnections.begin();
            i != _oldConnections.end(); ) {

        Connection& ch = **i;
        // Remove on error or if there are no more actions.
        if (!ch.advance() || !ch.hasPendingCalls()) {
            i = _oldConnections.erase(i);
        }
        else ++i;
    }

    // Advance current connection, but reset if there's an error.
    //
    // TODO: notify relevant status.
    if (_currentConnection.get()) {
        if (!_currentConnection->advance()) {
            _currentConnection.reset();
        }
    }

    /// If there are no connections we can stop the timer.
    if (_oldConnections.empty() && !_currentConnection.get()) {
        stopAdvanceTimer();
    }
}

// Anonymous namespace for NetConnection interface implementation.
namespace {

/// NetConnection.call()
//
/// Documented to return void, and current tests suggest this might be
/// correct, though they don't test with any calls that might succeed.
as_value
netconnection_call(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);

    if (fn.nargs < 1) {
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
    as_object* asCallback(0);
    if (fn.nargs > 1) {

        if (fn.arg(1).is_object()) {
            asCallback = (toObject(fn.arg(1), getVM(fn)));
        }
        else {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror("NetConnection.call(%s): second argument must be "
                    "an object", ss.str());
            );
        }
    }

    std::vector<as_value> args;
    if (fn.nargs > 2) {
        args = std::vector<as_value>(fn.getArgs().begin() + 2,
                fn.getArgs().end());
    }
    ptr->call(asCallback, methodName, args);

    return as_value();
}

as_value
netconnection_close(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);
    ptr->close();
    return as_value();
}

// Read-only
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
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new NetConnection_as(obj));
    attachProperties(*obj);
    return as_value();
}


/// For remoting, NetConnection.connect() takes a URL. For all other streams,
/// it takes null.
//
/// For non-remoting streams:
//
/// Returns undefined if there are no arguments, true if the first
/// argument is null, otherwise whether the connection is allowed. The
/// actual result of the connection is sent with an onStatus call later.
//
/// Undefined is also a valid argument for SWF7 and above.
//
/// The isConnected property is set to the result of connect().
as_value
netconnection_connect(const fn_call& fn)
{

    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);
    
    if (fn.nargs < 1) {
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
        return as_value(true);
    }

    if (fn.nargs > 1) {
        std::stringstream ss; fn.dump_args(ss);
        log_unimpl("NetConnection.connect(%s): args after the first are "
                "not supported", ss.str());
    }

    return as_value(ptr->connect(uriStr));

}


as_value
netconnection_addHeader(const fn_call& fn)
{
    NetConnection_as* ptr = ensure<ThisIsNative<NetConnection_as> >(fn);
    UNUSED(ptr);

    log_unimpl("NetConnection.addHeader()");
    return as_value();
}

/// This creates a local callback function to handle the return from connect()
//
/// NetStream does this using a builtin function, but this is more complicated
/// because it needs the native NetConnection type.
//
/// This stores the NetConnection object so that it can be updated when
/// connect returns.
//
/// We don't know if this is the best way to do it, but:
//
/// 1. the connect call *does* return a callback ID.
as_value
local_onResult(const fn_call& fn)
{
    as_object* obj = fn.this_ptr;

    if (obj) {
        const ObjectURI conn = getURI(getVM(fn), "_conn");
        as_value f = getMember(*obj, conn);
        as_object* nc = toObject(f, getVM(fn));
        if (nc) {
        }
        const as_value arg = fn.nargs ? fn.arg(0) : as_value();
        callMethod(nc, NSV::PROP_ON_STATUS, arg);
    }
    return as_value();
}


std::pair<std::string, std::string>
getStatusCodeInfo(NetConnection_as::StatusCode code)
{
    /// The Call statuses do exist, but this implementation is a guess.
    switch (code) {
        case NetConnection_as::CONNECT_SUCCESS:
            return std::make_pair("NetConnection.Connect.Success", "status");
        case NetConnection_as::CONNECT_FAILED:
            return std::make_pair("NetConnection.Connect.Failed", "error");
        case NetConnection_as::CONNECT_APPSHUTDOWN:
            return std::make_pair("NetConnection.Connect.AppShutdown", "error");
        case NetConnection_as::CONNECT_REJECTED:
            return std::make_pair("NetConnection.Connect.Rejected", "error");
        case NetConnection_as::CONNECT_CLOSED:
            return std::make_pair("NetConnection.Connect.Closed", "status");
        case NetConnection_as::CALL_FAILED:
            return std::make_pair("NetConnection.Call.Failed", "error");
        case NetConnection_as::CALL_BADVERSION:
            return std::make_pair("NetConnection.Call.BadVersion", "status");
        default:
            std::abort();
    }

}

void
handleAMFInvoke(amf::Reader& rd, const boost::uint8_t*& b,
        const boost::uint8_t* end, as_object& owner)
{

    const boost::uint16_t invokecount = amf::readNetworkShort(b);
    b += 2; 

    if (!invokecount) return;

    for (size_t i = invokecount; i > 0; --i) {
        if (b + 2 > end) {
            throw amf::AMFException("Invoke buffer too short");
        }
        const boost::uint16_t namelength = amf::readNetworkShort(b);
        b += 2;
        if (b + namelength > end) {
            throw amf::AMFException("Invoke buffer too short");
        }
        std::string headerName((char*)b, namelength);

#ifdef GNASH_DEBUG_REMOTING
        log_debug("Invoke name %s", headerName);
#endif
        b += namelength;
        if (b + 5 > end) {
            throw amf::AMFException("Invoke buffer too short");
        }
        b += 5; // skip past bool and length long

        // It seems there must be exactly one argument.
        as_value arg;
        if (!rd(arg)) {
            throw amf::AMFException("Invoke argument not present");
        }

        VM& vm = getVM(owner);
        ObjectURI key = getURI(vm, headerName);
#ifdef GNASH_DEBUG_REMOTING
        log_debug("Invoking %s(%s)", headerName, arg);
#endif
        callMethod(&owner, key, arg);
    }

}

/// Process any replies to server functions we invoked.
//
/// Note that fatal errors will throw an amf::AMFException.
void
HTTPRequest::handleAMFReplies(amf::Reader& rd, const boost::uint8_t*& b,
        const boost::uint8_t* end)
{
    const boost::uint16_t numreplies = amf::readNetworkShort(b);
    b += 2; // number of replies

    // TODO: test if this value is relevant at all.
    if (!numreplies) return;

    // There should be only three loop control mechanisms in this loop:
    // 1. continue: there was an error, but parsing is still okay.
    // 2. return: we've finished, but there was no problem.
    // 3. amf::AMFException: parsing failed and we can do nothing more.
    //
    // We haven't tested this very rigorously.
    while (b < end) {

        if (b + 2 > end) return;

        const boost::uint16_t replylength = amf::readNetworkShort(b);
        b += 2; 

        if (replylength < 4 || b + replylength > end) {
            throw amf::AMFException("Reply message too short");
        }

        // Reply message is: '/id/methodName'
        int ns = 1; // next slash position
        while (ns < replylength - 1 && *(b + ns) != '/') ++ns;
        if (ns >= replylength - 1) {
            throw amf::AMFException("Invalid reply message name");
        }

        std::string id(reinterpret_cast<const char*>(b + 1), ns - 1);
        size_t callbackID = 0;
        try {
            callbackID = boost::lexical_cast<size_t>(id);
        }
        catch (const boost::bad_lexical_cast&) {
            // Do we need to abort parsing here?
            throw amf::AMFException("Invalid callback ID");
        }

        const std::string methodName(reinterpret_cast<const char*>(b + ns + 1),
                replylength - ns - 1);

        b += replylength;

        // parse past unused string in header
        if (b + 2 > end) return;
        const boost::uint16_t unusedlength = amf::readNetworkShort(b);

        b += 2; 
        if (b + unusedlength > end) return;
        b += unusedlength;

        // this field is supposed to hold the total number of bytes in the
        // rest of this particular reply value, but openstreetmap.org
        // (which works great in the adobe player) sends 0xffffffff.
        // So we just ignore it.
        if (b + 4 > end) break;
        b += 4; 

        // this updates b to point to the next unparsed byte
        as_value replyval;
        if (!rd(replyval)) {
            throw amf::AMFException("Could not parse argument value");
        }

        // if actionscript specified a callback object,
        // call it
        as_object* callback = _handler.popCallback(callbackID);

        if (!callback) {
            log_error("Unknown HTTP Remoting response identifier '%s'", id);
            // There's no parsing error, so continue.
            continue;
        }

        ObjectURI methodKey;
        if (methodName == "onResult") {
            methodKey = NSV::PROP_ON_RESULT;
        }
        else if (methodName == "onStatus") {
            methodKey = NSV::PROP_ON_STATUS;
        }
        else {
            // NOTE: the pp is known to actually
            // invoke the custom method, but with 7
            // undefined arguments (?)
            log_error("Unsupported HTTP Remoting response callback: '%s' "
                    "(size %d)", methodName, methodName.size());
            continue;
        }

#ifdef GNASH_DEBUG_REMOTING
        log_debug("callback called");
#endif

        callMethod(callback, methodKey, replyval);
    } 

}

bool
HTTPConnection::advance()
{
    // If there is data waiting to be sent, send it and push it
    // to the queue.
    if (_currentRequest.get()) {
        _currentRequest->send(_url, _nc);
        _requestQueue.push_back(_currentRequest);

        // Clear the current request for the next go.
        _currentRequest.reset();
    }

    // Process all replies and clear finished requests.
    for (std::vector<boost::shared_ptr<HTTPRequest> >::iterator i = 
            _requestQueue.begin(); i != _requestQueue.end();) {
        if (!(*i)->process(_nc)) i = _requestQueue.erase(i);
        else ++i;
    }

    return true;
}

void
HTTPRequest::send(const URL& url, NetConnection_as& nc)
{
    // We should never have a request without any calls.
    assert(_calls);
    log_debug("creating connection");

    // Fill in header
    (reinterpret_cast<boost::uint16_t*>(_data.data() + 4))[0] = htons(_calls);
    std::string postdata(reinterpret_cast<char*>(_data.data()), _data.size());

#ifdef GNASH_DEBUG_REMOTING
        log_debug("NetConnection.call(): encoded args from %1% calls: %2%",
            _calls, hexify(_data.data(), _data.size(), false));
#endif

    const StreamProvider& sp = getRunResources(nc.owner()).streamProvider();
    _connection.reset(sp.getStream(url, postdata, _headers).release());
}


/// An AMF remoting reply comprises two main sections: first the invoke
/// commands to be called on the NetConnection object, and second the
/// replies to any client invoke messages that requested a callback.
bool
HTTPRequest::process(NetConnection_as& nc)
{
    assert(_connection);

    // Fill last chunk before reading in the next
    size_t toRead = _reply.capacity() - _reply.size();
    if (!toRead) toRead = NCCALLREPLYCHUNK;

#ifdef GNASH_DEBUG_REMOTING
    log_debug("Attempt to read %d bytes", toRead);
#endif

    // See if we need to allocate more bytes for the next
    // read chunk
    if (_reply.capacity() < _reply.size() + toRead) {
        const size_t newCapacity = _reply.size() + toRead;

#ifdef GNASH_DEBUG_REMOTING
        log_debug("NetConnection.call: reply buffer capacity (%d) "
                "is too small to accept next %d bytes of chunk "
                "(current size is %d). Reserving %d bytes.",
                _reply.capacity(), toRead, _reply.size(), newCapacity);
#endif

        _reply.reserve(newCapacity);
    }

    const int read = _connection->readNonBlocking(_reply.data() + _reply.size(),
            toRead);

    if (read > 0) {
#ifdef GNASH_DEBUG_REMOTING
        log_debug("read '%1%' bytes: %2%", read, 
                hexify(_reply.data() + _reply.size(), read, false));
#endif
        _reply.resize(_reply.size() + read);
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

        // If the connection fails, it is manually verified
        // that the pp calls onStatus with 1 undefined argument.
        callMethod(&nc.owner(), NSV::PROP_ON_STATUS, as_value());
        return false;
    }
 
    // Not all data was received, so carry on.
    if (!_connection->eof()) return true;

    // If it's less than 8 we didn't expect a response, so just ignore
    // it.
    if (_reply.size() > 8) {

#ifdef GNASH_DEBUG_REMOTING
        log_debug("hit eof");
#endif
        const boost::uint8_t *b = _reply.data();
        const boost::uint8_t *end = _reply.data() + _reply.size();
        
        amf::Reader rd(b, end, getGlobal(nc.owner()));
        
        // skip version indicator and client id
        b += 2; 

        try {
            handleAMFInvoke(rd, b, end, nc.owner());
            handleAMFReplies(rd, b, end);
        }
        catch (const amf::AMFException& e) {

            // Any fatal error should be signalled by throwing an
            // exception. In this case onStatus is called with an
            // undefined argument.
            log_error("Error parsing server AMF: %s", e.what());
            callMethod(&nc.owner(), NSV::PROP_ON_STATUS, as_value());
        }
    }

    // We've finished with this connection.
    return false;
}

void
HTTPConnection::call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args)
{
    if (!_currentRequest.get()) {
        _currentRequest.reset(new HTTPRequest(*this));
    }

    // Create AMF buffer for this call.
    SimpleBuffer buf(32);

    amf::writePlainString(buf, methodName, amf::STRING_AMF0);

    const size_t callID = callNo();

    // client id (result number) as counted string
    // the convention seems to be / followed by a unique (ascending) number
    std::ostringstream os;
    os << "/";
    // Call number is not used if the callback is undefined
    if (asCallback) os << callID;

    // Encode callback number.
    amf::writePlainString(buf, os.str(), amf::STRING_AMF0);

    size_t total_size_offset = buf.size();
    buf.append("\000\000\000\000", 4); // total size to be filled in later

    // encode array of arguments to remote method
    buf.appendByte(amf::STRICT_ARRAY_AMF0);
    buf.appendNetworkLong(args.size());

    // STRICT_ARRAY encoding is allowed for remoting
    amf::Writer w(buf, true);

    for (size_t i = 0; i < args.size(); ++i) {
        const as_value& arg = args[i];
        if (!arg.writeAMF0(w)) {
            log_error("Could not serialize NetConnection.call argument %d", i);
        }
    }

    // Set the "total size" parameter.
    *(reinterpret_cast<uint32_t*>(buf.data() + total_size_offset)) = 
        htonl(buf.size() - 4 - total_size_offset);

    // Add data to the current HTTPRequest.
    _currentRequest->addData(buf);

    // Remember the callback object.
    if (asCallback) {
        pushCallback(callID, asCallback);
    }
}

void
RTMPConnection::handleInvoke(const boost::uint8_t* payload,
        const boost::uint8_t* end)
{
    // TODO: clean up the logic in this function to reduce duplication.

    assert(payload != end);

    // make sure it is a string method name we start with
    if (payload[0] != 0x02) {
        log_error( "Sanity failed. no string method in invoke packet");
        return;
    }

    ++payload;
    std::string method = amf::readString(payload, end);

    log_debug("Invoke: read method string %s", method);
    if (*payload != amf::NUMBER_AMF0) return;
    ++payload;

    log_debug( "Server invoking <%s>", method);
    
    const ObjectURI methodname = getURI(getVM(_nc.owner()), method);

    // _result means it's the answer to a remote method call initiated
    // by us.
    if (method == "_result") {
        const double id = amf::readNumber(payload, end);
        log_debug("Received result for method call %s",
                boost::io::group(std::setprecision(15), id));

        as_value arg;

        amf::Reader rd(payload, end, getGlobal(_nc.owner()));
        // TODO: use all args and check the order! We currently only use
        // the last one!
        while (rd(arg)) {
            log_debug("Value: %s", arg);
        }

        as_object* o = popCallback(id);
        callMethod(o, NSV::PROP_ON_RESULT, arg);
        return;
    }
    
    /// These are remote function calls initiated by the server.
    const double id = amf::readNumber(payload, end);
    log_debug("Received server call %s %s",
            boost::io::group(std::setprecision(15), id),
            id ? "" : "(no reply expected)");

    /// If the server sends this, we reply (the call should contain a
    /// callback object!).
    if (method == "_onbwcheck") {
        if (id) replyBWCheck(_rtmp, id);
        else {
            log_error("Server called _onbwcheck without a callback");
        }
        return;
    }

    if (method == "_onbwdone") {

        if (*payload != amf::NULL_AMF0) return;
        ++payload;
#ifdef GNASH_DEBUG_REMOTING
        log_debug("AMF buffer for _onbwdone: %s\n",
                hexify(payload, end - payload, false));
#endif
        double latency = amf::readNumber(payload, end);
        double bandwidth = amf::readNumber(payload, end);
        log_debug("Latency: %s, bandwidth %s", latency, bandwidth);
        return;
    }

    if (method ==  "_error") {

        as_value arg;

        amf::Reader rd(payload, end, getGlobal(_nc.owner()));
        // TODO: use all args and check the order! We currently only use
        // the last one!
        while (rd(arg)) {
            log_debug("Value: %s", arg);
        }

        log_error( "rtmp server sent error");

        callMethod(&_nc.owner(), NSV::PROP_ON_STATUS, arg);
        return;
    }

    // Parse any arguments.
    as_value arg;

    amf::Reader rd(payload, end, getGlobal(_nc.owner()));
    // TODO: use all args and check the order! We currently only use
    // the last one!
    while (rd(arg)) {
        log_debug("Value: %s", arg);
    }
    
    // Call method on the NetConnection object.    
    callMethod(&_nc.owner(), methodname, arg);
    
}

void
replyBWCheck(rtmp::RTMP& r, double txn)
{
    SimpleBuffer buf;
    amf::write(buf, "_result");
    amf::write(buf, txn);
    buf.appendByte(amf::NULL_AMF0);
    amf::write(buf, 0.0);
    r.call(buf);
}

} // anonymous namespace
} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
