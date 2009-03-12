// NetConnection_as.cpp:  Open local connections for FLV files or URLs.
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

#include <iostream>
#include <string>
#include <boost/scoped_ptr.hpp>

// FIXME: Get rid of this crap.
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock.h>
#else
#include <arpa/inet.h> // for htons
#endif

#include "NetConnection_as.h"
#include "log.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "movie_root.h"
#include "Object.h" // for getObjectInterface

#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "URL.h"

// for NetConnection_as.call()
#include "VM.h"
#include "amf.h"
#include "SimpleBuffer.h"
#include "timers.h"
#include "namedStrings.h"


//#define GNASH_DEBUG_REMOTING

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

//----- NetConnection_as ----------------------------------------------------

NetConnection_as::NetConnection_as()
    :
    as_object(getNetConnectionInterface()),
    _uri(),
    _isConnected(false)
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

// here to have HTTPRemotingHandler definition available
NetConnection_as::~NetConnection_as()
{
}

void
NetConnection_as::markReachableResources() const
{
}


/// FIXME: this should not use _uri, but rather take a URL argument.
/// Validation should probably be done on connect() only and return a 
/// bool indicating validity. That can be used to return a failure
/// for invalid or blocked URLs.
std::string
NetConnection_as::validateURL() const
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
NetConnection_as::notifyStatus(StatusCode code)
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
    if (uri.empty()) {
        _isConnected = false;
        notifyStatus(CONNECT_FAILED);
        return;
    }

    const movie_root& mr = _vm.getRoot();
    URL url(uri, mr.runInfo().baseURL());

    if (url.protocol() == "rtmp") {
        LOG_ONCE( log_unimpl("NetConnection.connect(%s): RTMP not yet supported", url) );
        notifyStatus(CONNECT_FAILED);
        return;
    }

    if ( url.protocol() != "http" ) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("NetConnection.connect(%s): invalid connection protocol", url);
        );
        notifyStatus(CONNECT_FAILED);
        return;
    }

    // This is for HTTP remoting

    if (!URLAccessManager::allow(url)) {
        log_security(_("Gnash is not allowed to NetConnection.connect to %s"), url);
        notifyStatus(CONNECT_FAILED);
        return;
    }

    _isConnected = false;
}


/// FIXME: This should close an active connection as well as setting the
/// appropriate properties.
void
NetConnection_as::close()
{

    /// TODO: what should actually happen here? Should an attached
    /// NetStream object be interrupted?
    _isConnected = false;

    notifyStatus(CONNECT_CLOSED);
}


void
NetConnection_as::setURI(const std::string& uri)
{
    init_readonly_property("uri", &netconnection_uri);
    _uri = uri;
}

void
NetConnection_as::call(as_object* asCallback, const std::string& methodName,
        const std::vector<as_value>& args, size_t firstArg)
{
}

std::auto_ptr<IOChannel>
NetConnection_as::getStream(const std::string& name)
{
    const RunInfo& ri = _vm.getRoot().runInfo();

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

/// Anonymous namespace for NetConnection interface implementation.

namespace {


/// NetConnection.call()
//
/// Documented to return void, and current tests suggest this might be
/// correct, though they don't test with any calls that might succeed.
as_value
netconnection_call(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr = 
        ensureType<NetConnection_as>(fn.this_ptr); 

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

    const std::vector<as_value>& args = fn.getArgs();
    ptr->call(asCallback.get(), methodName, args, 2);

    return as_value();
}

as_value
netconnection_close(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr); 

    ptr->close();

    return as_value();
}


/// Read-only
as_value
netconnection_isConnected(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr); 

    return as_value(ptr->isConnected());
}

as_value
netconnection_uri(const fn_call& fn)
{
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr); 

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

    NetConnection_as* nc = new NetConnection_as;

    return as_value(nc);
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

    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr); 
    
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
    
    // This is always set without validification.
    ptr->setURI(uriStr);

    // Check first arg for validity 
    if (uri.is_null() || (vm.getSWFVersion() > 6 && uri.is_undefined())) {
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
    boost::intrusive_ptr<NetConnection_as> ptr =
        ensureType<NetConnection_as>(fn.this_ptr); 
    UNUSED(ptr);

    log_unimpl("NetConnection.addHeader()");
    return as_value();
}

} // anonymous namespace

} // end of gnash namespace

