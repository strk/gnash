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

/* $Id: NetConnection.cpp,v 1.54 2008/01/21 20:55:56 rsavoye Exp $ */

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <string>
#include <new>
#include "NetConnection.h"
#include "log.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "movie_root.h"
#include "Object.h" // for getObjectInterface

#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "URL.h"

using namespace std;

namespace gnash {

static as_value netconnection_new(const fn_call& fn);
//static as_object* getNetConnectionInterface();

/// \class NetConnection
/// \brief Opens a local connection through which you can play
/// back video (FLV) files from an HTTP address or from the local file
/// system, using curl.


NetConnection::NetConnection()
	:
	as_object(getNetConnectionInterface())
{
	attachProperties();
}

NetConnection::~NetConnection()
{
}

/*public*/
std::string NetConnection::validateURL(const std::string& url)
{

	std::string completeUrl;
	if (_prefixUrl.size() > 0) {
		completeUrl += _prefixUrl + "/" + url;
	} else {
		completeUrl += url;
	}

	URL uri(completeUrl, get_base_url());

	std::string uriStr(uri.str());
	assert(uriStr.find("://")!=string::npos);

	// Check if we're allowed to open url
	if (!URLAccessManager::allow(uri)) {
		log_security(_("Gnash is not allowed to open this url: %s"), uriStr.c_str());
		return "";
	}

	log_msg(_("Connecting to movie: %s"), uriStr.c_str());

	log_msg(_("Connection etablished to movie: %s"), uriStr.c_str());

	return uriStr;
}

/*public*/
void
NetConnection::addToURL(const std::string& url)
{
	// What is this ? It is NOT documented in the header !!
	//if (url == "null" || url == "NULL") return;

	// If there already is something in _prefixUrl, then we already have a url,
	// so no need to renew it. This may not correct, needs some testing.
	if (_prefixUrl.size() > 0) return;

	_prefixUrl += url;
}


/// \brief callback to instantiate a new NetConnection object.
/// \param fn the parameters from the Flash movie
/// \return nothing from the function call.
/// \note The return value is returned through the fn.result member.
static as_value
netconnection_new(const fn_call& /* fn */)
{
	GNASH_REPORT_FUNCTION;

	NetConnection *netconnection_obj = new NetConnection;

	return as_value(netconnection_obj);
}

as_value
NetConnection::connect_method(const fn_call& fn)
{
	// NOTE:
	//
	// NetConnection::connect() is *documented*, I repeat, *documented*, to require the
	// "url" argument to be NULL in AS <= 2. This is *legal* and *required*. Anything
	// other than NULL is undocumented behaviour, and I would like to know if there
	// are any movies out there relying on it. --bjacques.

	GNASH_REPORT_FUNCTION;

	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 
    
	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("NetConnection.connect(): needs at least one argument"));
		);
		return as_value(false);
	}

	as_value& url_val = fn.arg(0);

	// Check first arg for validity 
	if ( url_val.is_null())
	{
		// Null URL was passed. This is expected. Of course, it also makes this
		// function (and, this class) rather useless. We return true, even though
		// returning true has no meaning.
		
		return as_value(true);
	}

	// The remainder of this function is undocumented.
	
	if (url_val.is_undefined()) {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("NetConnection.connect(): first argument shouldn't be undefined"));
                );
		return as_value(false);
	}


	/// .. TODO: checkme ... addToURL ?? shoudnl't we attempt a connection ??
	ptr->addToURL(url_val.to_string());

	if ( fn.nargs > 1 )
	{
		std::stringstream ss; fn.dump_args(ss);
		log_unimpl("NetConnection.connect(%s): args after the first are not supported", ss.str().c_str());
	}


	// TODO: FIXME: should return true *or false* for RTMP connections
	return as_value(true);
}

as_value
NetConnection::addHeader_method(const fn_call& fn)
{
	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 
	UNUSED(ptr);

	log_unimpl("NetConnection.addHeader()");
	return as_value();
}

as_value
NetConnection::call_method(const fn_call& fn)
{
	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 
	UNUSED(ptr);

	log_unimpl("NetConnection.call()");
	return as_value();
}

as_value
NetConnection::close_method(const fn_call& fn)
{
	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 
	UNUSED(ptr);

	log_unimpl("NetConnection.close()");
	return as_value();
}

as_value
NetConnection::isConnected_getset(const fn_call& fn)
{
	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 
	UNUSED(ptr);

	if ( fn.nargs == 0 ) // getter
	{
		log_unimpl("NetConnection.isConnected get");
	  return as_value();
	}
	else // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Tried to set read-only property NetConnection.isConnected");
		);
		return as_value();
	}
}

as_value
NetConnection::uri_getset(const fn_call& fn)
{
	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 
	UNUSED(ptr);

	if ( fn.nargs == 0 ) // getter
	{
		log_unimpl("NetConnection.uri get");
		return as_value();
	}
	else // setter
	{
		log_unimpl("NetConnection.uri set");
		return as_value();
	}

}

void
NetConnection::attachNetConnectionInterface(as_object& o)
{
	o.init_member("connect", new builtin_function(NetConnection::connect_method));
	o.init_member("addHeader", new builtin_function(NetConnection::addHeader_method));
	o.init_member("call", new builtin_function(NetConnection::call_method));
	o.init_member("close", new builtin_function(NetConnection::close_method));

}

void
NetConnection::attachProperties()
{
	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(NetConnection::isConnected_getset, NULL);
	init_property("isConnected", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(NetConnection::uri_getset, NULL);
	init_property("uri", *gettersetter, *gettersetter);

}

as_object*
NetConnection::getNetConnectionInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(getObjectInterface());
		NetConnection::attachNetConnectionInterface(*o);
	}

	return o.get();
}

void
NetConnection::registerConstructor(as_object& global)
{

	// This is going to be the global NetConnection "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&netconnection_new, getNetConnectionInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		// TODO: this is probably wrong !
		NetConnection::attachNetConnectionInterface(*cl);
		     
	}

	// Register _global.String
	global.init_member("NetConnection", cl.get());

}

// extern (used by Global.cpp)
void netconnection_class_init(as_object& global)
{
	NetConnection::registerConstructor(global);
}


} // end of gnash namespace

