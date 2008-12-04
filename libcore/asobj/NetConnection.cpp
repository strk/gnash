// NetConnection.cpp:  Open local connections for FLV files or URLs.
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

#include <iostream>
#include <string>
#include <boost/scoped_ptr.hpp>

// FIXME: Get rid of this crap.
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock.h>
#else
#include <arpa/inet.h> // for htons
#endif

#include "NetConnection.h"
#include "log.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "movie_root.h"
#include "Object.h" // for getObjectInterface

#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "URL.h"

// for NetConnection.call()
#include "VM.h"
#include "amf.h"
#include "SimpleBuffer.h"
#include "timers.h"
#include "namedStrings.h"


// #define GNASH_DEBUG_REMOTING

// Forward declarations.

namespace gnash {

namespace {
    void attachProperties(as_object& o);
    void attachNetConnectionInterface(as_object& o);
    as_object* getNetConnectionInterface();
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
/// @todo move this somewhere more appropriate, perhaps by merging with libamf.
class AMFQueue {
private:

    NetConnection& _nc;
    static const int NCCALLREPLYMAX=200000;

    typedef std::map<std::string, boost::intrusive_ptr<as_object> > 
        CallbacksMap;
    CallbacksMap callbacks;

    SimpleBuffer postdata;
    URL url;
    boost::scoped_ptr<IOChannel> _connection;
    SimpleBuffer reply;
    int reply_start;
    int reply_end;
    int queued_count;
    unsigned int ticker;

public:
    AMFQueue(NetConnection& nc, URL url)
        :
        _nc(nc),
        postdata(),
        url(url),
        _connection(0),
        reply(NCCALLREPLYMAX),
        reply_start(0),
        reply_end(0),
        queued_count(0),
        ticker(0)
    {
        // leave space for header
        postdata.append("\000\000\000\000\000\000", 6);
    }

    ~AMFQueue() {
        stop_ticking();
    }
    
    void enqueue(const SimpleBuffer &amf, const std::string& identifier,
            boost::intrusive_ptr<as_object> callback) {
        push_amf(amf);
        push_callback(identifier, callback);
    };
    void enqueue(const SimpleBuffer &amf) {
        push_amf(amf);
    };
    
    // tick is called automatically on intervals (hopefully only between
    // actionscript frames)
    //
    // it handles all networking for NetConnection::call() and dispatches
    // callbacks when needed
    void tick() {

#ifdef GNASH_DEBUG_REMOTING
        log_debug("tick running");
#endif
        if(_connection)
        {

            VM& vm = _nc.getVM();

#ifdef GNASH_DEBUG_REMOTING
            log_debug("have connection");
#endif
            int read = _connection->readNonBlocking(reply.data() + reply_end,
                    NCCALLREPLYMAX - reply_end);
            if(read > 0) {
#ifdef GNASH_DEBUG_REMOTING
                log_debug("read '%1%' bytes: %2%", read, 
                        hexify(reply.data() + reply_end, read, false));
#endif
                reply_end += read;
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

            if(_connection->get_error())
            {
                log_debug("connection is in error condition, calling "
                        "NetConnection.onStatus");
                reply_start = 0;
                reply_end = 0;
                // reset connection before calling the callback
                _connection.reset();

                // This is just a guess, but is better than sending
                // 'undefined'
                _nc.notifyStatus(NetConnection::CALL_FAILED);
            }
            else if(_connection->eof() )
            {
                if ( reply_end > 8)
                {
                    std::vector<as_object*> objRefs;

#ifdef GNASH_DEBUG_REMOTING
                    log_debug("hit eof");
#endif
                    boost::int16_t si;
                    boost::uint16_t li;
                    boost::uint8_t *b = reply.data() + reply_start;
                    boost::uint8_t *end = reply.data() + reply_end;

                    // parse header
                    b += 2; // skip version indicator and client id

                    // NOTE: this looks much like parsing of an OBJECT_AMF0
                    si = readNetworkShort(b); b += 2; // number of headers
                    uint8_t headers_ok = 1;
                    if (si != 0)
                    {
#ifdef GNASH_DEBUG_REMOTING
                        log_debug("NetConnection::call(): amf headers "
                                "section parsing");
#endif
                        as_value tmp;
                        for(int i = si; i > 0; --i)
                        {
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
                            if( !tmp.readAMF0(b, end, -1, objRefs, vm) )
                            {
                                headers_ok = 0;
                                break;
                            }
#ifdef GNASH_DEBUG_REMOTING
                            log_debug("Header value %s", tmp);
#endif

                            { // method call for each header
                              // FIXME: it seems to me that the call should happen
                                VM& vm = _nc.getVM();
                                string_table& st = vm.getStringTable();
                                string_table::key key = st.find(headerName);
#ifdef GNASH_DEBUG_REMOTING
                                log_debug("Calling NetConnection.%s(%s)",
                                        headerName, tmp);
#endif
                                _nc.callMethod(key, tmp);
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
                                if(si < 11) {
                                    log_error("NetConnection::call(): reply message name too short");
                                    break;
                                }
                                if(b + si > end) break;
                                // TODO check that the last 9 bytes are "/onResult"
                                // this should either split on the 2nd / or require onResult or onStatus
                                std::string id(reinterpret_cast<char*>(b), si - 9);
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
                                as_value reply_as_value;
                                if ( ! reply_as_value.readAMF0(b, end, -1, objRefs, vm) )
                                {
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

                                // if actionscript specified a callback object, call it
                                boost::intrusive_ptr<as_object> callback = pop_callback(id);
                                if(callback) {
#ifdef GNASH_DEBUG_REMOTING
                                    log_debug("calling onResult callback");
#endif
                                    // FIXME check if above line can fail and we have to react
                                    callback->callMethod(NSV::PROP_ON_RESULT, reply_as_value);
#ifdef GNASH_DEBUG_REMOTING
                                    log_debug("callback called");
#endif
                                } else {
#ifdef GNASH_DEBUG_REMOTING
                                    log_debug("couldn't find callback object");
#endif
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
                reply_start = 0;
                reply_end = 0;
            }
        }

        if(!_connection && queued_count > 0) {
#ifdef GNASH_DEBUG_REMOTING
            log_debug("creating connection");
#endif
            // set the "number of bodies" header
            (reinterpret_cast<boost::uint16_t*>(postdata.data() + 4))[0] = htons(queued_count);
            std::string postdata_str(reinterpret_cast<char*>(postdata.data()), postdata.size());
#ifdef GNASH_DEBUG_REMOTING
            log_debug("NetConnection.call(): encoded args from %1% calls: %2%", queued_count, hexify(postdata.data(), postdata.size(), false));
#endif
            queued_count = 0;
            _connection.reset(StreamProvider::getDefaultInstance().getStream(url, postdata_str).release());
            postdata.resize(6);
#ifdef GNASH_DEBUG_REMOTING
            log_debug("connection created");
#endif
        }

        if(_connection == 0 && queued_count == 0) {
#ifdef GNASH_DEBUG_REMOTING
            log_debug("stopping ticking");
#endif
            stop_ticking();
#ifdef GNASH_DEBUG_REMOTING
            log_debug("ticking stopped");
#endif
        }
    };

    static as_value amfqueue_tick_wrapper(const fn_call& fn)
    {
        boost::intrusive_ptr<NetConnection> ptr = 
            ensureType<NetConnection>(fn.this_ptr);
        // FIXME check if it's possible for the URL of a NetConnection to change between call()s
        ptr->_callQueue->tick();
        return as_value();
    };

    void markReachableResources() const
    {
        for (CallbacksMap::const_iterator i=callbacks.begin(),
                e=callbacks.end(); i!=e; ++i)
        {
            i->second->setReachable();
        }
    }

private:
    void start_ticking() 
    {

        if (ticker) return;

        boost::intrusive_ptr<builtin_function> ticker_as = 
                new builtin_function(&AMFQueue::amfqueue_tick_wrapper);

        std::auto_ptr<Timer> timer(new Timer);
        unsigned long delayMS = 50; // FIXME crank up to 50 or so
        timer->setInterval(*ticker_as, delayMS, &_nc);
        ticker = _nc.getVM().getRoot().add_interval_timer(timer, true);
    }

    void push_amf(const SimpleBuffer &amf) 
    {
        GNASH_REPORT_FUNCTION;

        postdata.append(amf.data(), amf.size());
        queued_count++;

        start_ticking();
    }

    void stop_ticking() 
    {
        if (!ticker) return;
        _nc.getVM().getRoot().clear_interval_timer(ticker);
        ticker=0;
    }

    void push_callback(const std::string& id,
            boost::intrusive_ptr<as_object> callback) {
        callbacks.insert(std::pair<std::string,
                boost::intrusive_ptr<as_object> >(id, callback));
    }

    boost::intrusive_ptr<as_object> pop_callback(std::string id)
    {
        CallbacksMap::iterator it = callbacks.find(id);
        if (it != callbacks.end()) {
            boost::intrusive_ptr<as_object> callback = it->second;
            //boost::intrusive_ptr<as_object> callback;
            //callback = it.second;
            callbacks.erase(it);
            return callback;
        }
        else {
            return 0;
        }
    }
};

/// \class NetConnection
/// \brief Opens a local connection through which you can play
/// back video (FLV) files from an HTTP address or from the local file
/// system, using curl.
NetConnection::NetConnection()
    :
    as_object(getNetConnectionInterface()),
    _callQueue(0),
    _isConnected(false),
    _inError(false),
    _uriValid(false)
{
    attachProperties(*this);
}

// extern (used by Global.cpp)
void
netconnection_class_init(as_object& global)
{
    // This is going to be the global NetConnection "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&netconnection_new,
                getNetConnectionInterface());
        // replicate all interface to class, to be able to access
        // all methods as static functions
        attachNetConnectionInterface(*cl);
             
    }

    // Register _global.String
    global.init_member("NetConnection", cl.get());
}

// here to have AMFQueue definition available
NetConnection::~NetConnection()
{
}


void
NetConnection::markReachableResources() const
{
    if ( _callQueue.get() ) _callQueue->markReachableResources();
    markAsObjectReachable();
}


std::string
NetConnection::validateURL() const
{

    const movie_root& mr = _vm.getRoot();
    URL uri(_uri, mr.runInfo().baseURL());

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
NetConnection::notifyStatus(StatusCode code)
{
    std::pair<std::string, std::string> info;
    getStatusCodeInfo(code, info);

    /// This is a new normal object each time (see NetConnection.as)
    as_object* o = new as_object(getObjectInterface());

    const int flags = 0;

    o->init_member("code", info.first, flags);
    o->init_member("level", info.second, flags);

    callMethod(NSV::PROP_ON_STATUS, o);

}

void
NetConnection::getStatusCodeInfo(StatusCode code, NetConnectionStatus& info)
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


void
NetConnection::connect()
{
    _isConnected = true;
    _inError = false;
    _uriValid = false;
    notifyStatus(CONNECT_SUCCESS);
}


void
NetConnection::connect(const std::string& uri)
{

    // FIXME: RTMP URLs should attempt a connection (warning: this seems
    // to be different for SWF8). Would probably return true on success and
    // set isConnected.
    //
    // For URLs starting with anything other than "rtmp://" no connection is
    // initiated, but the uri is still set.
    setURI(uri);

    _isConnected = false;
    _inError = true;
    _uriValid = true;
    notifyStatus(CONNECT_FAILED);
}


void
NetConnection::close()
{
    /// TODO: what should actually happen here? Should an attached
    /// NetStream object be interrupted?
    _isConnected = false;

    // If a previous connect() attempt failed, close() will not send
    // an onStatus event.
    if (!_inError) notifyStatus(CONNECT_CLOSED);
}


void
NetConnection::setURI(const std::string& uri)
{
    init_readonly_property("uri", &netconnection_uri);
    _uri = uri;
}

void
NetConnection::call(as_object* asCallback, const std::string& callNumber,
        const SimpleBuffer& buf)
{

#ifdef GNASH_DEBUG_REMOTING
    log_debug(_("NetConnection.call(): encoded args: %s"),
            hexify(buf->data(), buf->size(), false));
#endif

    // FIXME check that ptr->_uri is valid
    URL url(validateURL());

    // FIXME check if it's possible for the URL of a NetConnection
    // to change between call()s
    if (!_callQueue.get()) {
        _callQueue.reset(new AMFQueue(*this, url));
    }

    if (asCallback) {
        //boost::intrusive_ptr<as_object> intrusive_callback(asCallback);
#ifdef GNASH_DEBUG_REMOTING
        log_debug("calling enqueue with callback");
#endif
        _callQueue->enqueue(buf, callNumber, asCallback);
        //? delete asCallback;
    }
    
    else {
#ifdef GNASH_DEBUG_REMOTING
        log_debug("calling enqueue without callback");
#endif
        _callQueue->enqueue(buf);
    }
#ifdef GNASH_DEBUG_REMOTING
    log_debug("called enqueue");
#endif

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
    boost::intrusive_ptr<NetConnection> ptr = 
        ensureType<NetConnection>(fn.this_ptr); 

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetConnection.call(): needs at least one argument"));
        );
        return as_value(); 
    }

    const as_value& methodName_as = fn.arg(0);
    if (!methodName_as.is_string()) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("NetConnection.call(%s): first argument "
                "(methodName) must be a string"), ss.str());
        );
        return as_value(); 
    }

    std::stringstream ss; fn.dump_args(ss);
#ifdef GNASH_DEBUG_REMOTING
    log_debug("NetConnection.call(%s)", ss.str());
#endif

    // TODO: arg(1) is the response object. let it know when data comes back
    boost::intrusive_ptr<as_object> asCallback;
    if (fn.nargs > 1) {

        if (fn.arg(1).is_object()) {
            asCallback = (fn.arg(1).to_object());
        }
        else {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror("NetConnection.call(%s): second argument must be "
                    "an object", ss.str());
            );
        }
    }
    std::string methodName = methodName_as.to_string();

    static int call_number = 0;

    boost::scoped_ptr<SimpleBuffer> buf (new SimpleBuffer(32));

    // method name
    buf->appendNetworkShort(methodName.size());
    buf->append(methodName.c_str(), methodName.size());

    // client id (result number) as counted string
    // the convention seems to be / followed by a unique (ascending) number
    ++call_number;

    std::ostringstream os;
    os << "/" << call_number;
    const std::string callNumberString = os.str();

    buf->appendNetworkShort(callNumberString.size());
    buf->append(callNumberString.c_str(), callNumberString.size());

    size_t total_size_offset = buf->size();
    buf->append("\000\000\000\000", 4); // total size to be filled in later

    std::map<as_object*, size_t> offsetTable;

    // encode array of arguments to remote method
    buf->appendByte(amf::Element::STRICT_ARRAY_AMF0);
    buf->appendNetworkLong(fn.nargs - 2);

    VM& vm = ptr->getVM();

    if (fn.nargs > 2)
    {
        for (unsigned int i = 2; i < fn.nargs; ++i)
        {
            const as_value& arg = fn.arg(i);
            if ( ! arg.writeAMF0(*buf, offsetTable, vm) )
            {
                log_error("Could not serialize NetConnection.call argument %d",
                        i);
            }
        }
    }

    // Set the "total size" parameter.
    *(reinterpret_cast<uint32_t*>(buf->data() + total_size_offset)) = 
        htonl(buf->size() - 4 - total_size_offset);

    ptr->call(asCallback.get(), callNumberString, *buf);

    return as_value();
}

as_value
netconnection_close(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection> ptr =
        ensureType<NetConnection>(fn.this_ptr); 

    ptr->close();

    return as_value();
}


/// Read-only
as_value
netconnection_isConnected(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection> ptr =
        ensureType<NetConnection>(fn.this_ptr); 

    return as_value(ptr->isConnected());
}

as_value
netconnection_uri(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection> ptr =
        ensureType<NetConnection>(fn.this_ptr); 

    return as_value(ptr->getURI());
}

void
attachNetConnectionInterface(as_object& o)
{
    o.init_member("connect", new builtin_function(netconnection_connect));
    o.init_member("addHeader", new builtin_function(netconnection_addHeader));
    o.init_member("call", new builtin_function(netconnection_call));
    o.init_member("close", new builtin_function(netconnection_close));
}

void
attachProperties(as_object& o)
{
    o.init_readonly_property("isConnected", &netconnection_isConnected);
}

as_object*
getNetConnectionInterface()
{

    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL )
    {
        o = new as_object(getObjectInterface());
        attachNetConnectionInterface(*o);
    }

    return o.get();
}

/// \brief callback to instantiate a new NetConnection object.
/// \param fn the parameters from the Flash movie
/// \return nothing from the function call.
/// \note The return value is returned through the fn.result member.
as_value
netconnection_new(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;

    NetConnection* nc = new NetConnection;

    return as_value(nc);
}


/// For rtmp, NetConnect.connect() takes an RTMP URL. For all other streams,
/// it takes null or undefined.
//
/// RTMP is untested.
//
/// For non-rtmp streams:
//
/// Returns undefined if there are no arguments. Otherwise true if the first
/// argument is null, false if it is anything else. Undefined is also valid
/// for SWF7 and above.
//
/// The isConnected property is set to the result of connect().
as_value
netconnection_connect(const fn_call& fn)
{

    boost::intrusive_ptr<NetConnection> ptr =
        ensureType<NetConnection>(fn.this_ptr); 
    
    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetConnection.connect(): needs at least "
                    "one argument"));
        );
        return as_value();
    }

    const as_value& uri = fn.arg(0);

    const VM& vm = ptr->getVM();
    const std::string& uriStr = uri.to_string_versioned(vm.getSWFVersion());

    // Check first arg for validity 
    if (uri.is_null() || (vm.getSWFVersion() > 6 && uri.is_undefined())) {
        ptr->connect();
        ptr->setURI(uriStr);
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
    boost::intrusive_ptr<NetConnection> ptr =
        ensureType<NetConnection>(fn.this_ptr); 
    UNUSED(ptr);

    log_unimpl("NetConnection.addHeader()");
    return as_value();
}

} // anonymous namespace

} // end of gnash namespace

