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
#include "array.h"
#include "amf.h"
#include "SimpleBuffer.h"
#include "timers.h"
#include "namedStrings.h"

using namespace amf;

namespace gnash {

static as_value netconnection_new(const fn_call& fn);

/// \class NetConnection
/// \brief Opens a local connection through which you can play
/// back video (FLV) files from an HTTP address or from the local file
/// system, using curl.
NetConnection::NetConnection()
	:
	as_object(getNetConnectionInterface()),
	_callQueue(0)
{
	attachProperties();
}


/*public*/
std::string NetConnection::validateURL(const std::string& url)
{
	std::string completeUrl;
	if (_prefixUrl.size() > 0) {
		if(url.size() > 0) {
			completeUrl += _prefixUrl + "/" + url;
		} else {
			completeUrl += _prefixUrl;
		}
	} else {
		completeUrl += url;
	}

	URL uri(completeUrl, get_base_url());

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

/*private*/
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
		log_unimpl("NetConnection.connect(%s): args after the first are not supported", ss.str());
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

boost::uint16_t
readNetworkShort(const boost::uint8_t* buf) {
	boost::uint16_t s = buf[0] << 8 | buf[1];
	return s;
}

boost::uint16_t
readNetworkLong(const boost::uint8_t* buf) {
	boost::uint32_t s = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
	return s;
}

// Pass pointer to buffer and pointer to end of buffer. Buffer is raw AMF
// encoded data. Must start with a type byte unless third parameter is set.
//
// On success, sets the given as_value and returns true.
// On error (premature end of buffer, etc.) returns false and leaves the given
// as_value untouched.
//
// IF you pass a fourth parameter, it WILL NOT READ A TYPE BYTE, but use what
// you passed instead.
//
// The l-value you pass as the first parameter (buffer start) is updated to
// point just past the last byte parsed
//
// TODO restore first parameter on parse errors
//
static bool
amf0_read_value(boost::uint8_t *&b, boost::uint8_t *end, as_value& ret, int inType = -1)
{
	boost::uint16_t si;
	boost::uint16_t li;
	double dub;
	int amf_type;

	if(b > end) {
		return false;
	}
	if(inType != -1) {
		amf_type = inType;
	} else {
		if(b < end) {
			amf_type = *b; b += 1;
		} else {
			return false;
		}
	}

	switch(amf_type) {
		case Element::NUMBER_AMF0:
			if(b + 8 > end) {
				log_error(_("NetConnection.call(): server sent us a number which goes past the end of the data sent"));
				return false;
			}
			dub = *(reinterpret_cast<double*>(b)); b += 8;
			swapBytes(&dub, 8);
			log_debug("nc read double: %e", dub);
			ret.set_double(dub);
			return true;
		case Element::STRING_AMF0:
			if(b + 2 > end) {
				log_error(_("NetConnection.call(): server sent us a string which goes past the end of the data sent"));
				return false;
			}
			si = readNetworkShort(b); b += 2;
			if(b + si > end) {
				log_error(_("NetConnection.call(): server sent us a string which goes past the end of the data sent"));
				return false;
			}

			{
				std::string str(reinterpret_cast<char *>(b), si); b += si;
				log_debug("nc read string: %s", str);
				ret.set_string(str);
				return true;

			}
			break;
		case Element::STRICT_ARRAY_AMF0:
			{
				boost::intrusive_ptr<as_array_object> array(new as_array_object());
				li = readNetworkLong(b); b += 4;
				log_debug("nc starting read of array with %i elements", li);
				as_value arrayElement;
				for(int i = 0; i < li; ++i)
				{
					if ( ! amf0_read_value(b, end, arrayElement) )
					{
						return false;
					}
					array->push(arrayElement);
				}

				ret.set_as_object(array);
				return true;
			}
		case Element::OBJECT_AMF0:
			{
				// need this? boost::intrusive_ptr<as_object> obj(new as_object(getObjectInterface()));
				boost::intrusive_ptr<as_object> obj(new as_object());
				log_debug("nc starting read of object");
				as_value tmp;
				std::string keyString;
				for(;;)
				{
					if ( ! amf0_read_value(b, end, tmp, Element::STRING_AMF0) )
					{
						return false;
					}
					keyString = tmp.to_string();

					if ( keyString.empty() )
					{
						if(b < end) {
							b += 1; // AMF0 has a redundant "object end" byte
						} else {
							log_error("AMF buffer terminated just before object end byte. continueing anyway.");
						}
						ret.set_as_object(obj);
						return true;
					}

					if ( ! amf0_read_value(b, end, tmp) )
					{
						return false;
					}
					obj->init_member(keyString, tmp);
				}
			}
		case Element::UNDEFINED_AMF0:
			{
				ret.set_undefined();
				return true;
			}
		case Element::NULL_AMF0:
			{
				ret.set_null();
				return true;
			}
		// TODO define other types (function, sprite, etc)
		default:
			log_unimpl("NetConnection.call(): server sent us a value of unsupported type: %i", amf_type);
			return false;
	}

	// this function was called with a zero-length buffer
	return false;
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
class AMFQueue {
private:
	NetConnection& _nc;
	static const int NCCALLREPLYMAX=200000;

	typedef std::map<std::string, boost::intrusive_ptr<as_object> > CallbacksMap;
	CallbacksMap callbacks;

	SimpleBuffer postdata;
	URL url;
	boost::scoped_ptr<IOChannel> connection;
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
		connection(0),
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
	
	void enqueue(const SimpleBuffer &amf, const std::string& identifier, boost::intrusive_ptr<as_object> callback) {
		push_amf(amf);
		push_callback(identifier, callback);
		//log_aserror("NetConnection::call(): called with a non-object as the callback");
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

		log_debug("tick running");
		if(connection)
		{
			log_debug("have connection");
			int read = connection->readNonBlocking(reply.data() + reply_end, NCCALLREPLYMAX - reply_end);
			if(read > 0) {
				log_debug("read '%1%' bytes: %2%", read, hexify(reply.data() + reply_end, read, false));
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

			if(connection->get_error())
			{
				log_debug("connection is in error condition, calling NetConnection.onStatus");
				reply_start = 0;
				reply_end = 0;
				log_debug("deleting connection");
				connection.reset(); // reset connection before calling the callback

				// FIXME: should only call NetConnection's onStatus
				//        if the IOChannel is in error condition.
				//        (tipically 404).
				//        When the response is empty, just nothing happens.
				_nc.callMethod(NSV::PROP_ON_STATUS, as_value());

			}
			else if(connection->eof() )
			{
				if ( reply_end > 8)
				{
					log_debug("hit eof");
					boost::int16_t si;
					boost::uint16_t li;
					boost::uint8_t *b = reply.data() + reply_start;
					boost::uint8_t *end = reply.data() + reply_end;

					// parse header
					b += 2; // skip version indicator and client id

					// NOTE: this looks much like parsing of an OBJECT_AMF0
					si = readNetworkShort(b); b += 2; // number of headers
					uint8_t headers_ok = 1;
					if(si != 0)
					{
						log_debug("NetConnection::call(): amf headers section parsing");
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
							//if( !amf0_read_value(b, end, tmp) )
							//{
							//	headers_ok = 0;
							//	break;
							//}
							log_debug("Header name %s", headerName);
							b += si;
							if ( b + 5 > end ) {
								headers_ok = 0;
								break;
							}
							b += 5; // skip past bool and length long
							if( !amf0_read_value(b, end, tmp) )
							{
								headers_ok = 0;
								break;
							}
							log_debug("Header value %s", tmp);

							{ // method call for each header
							  // FIXME: it seems to me that the call should happen
								VM& vm = _nc.getVM();
								string_table& st = vm.getStringTable();
								string_table::key key = st.find(headerName);
								log_debug("Calling NetConnection.%s(%s)", headerName, tmp);
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

								log_debug("about to parse amf value");
								// this updates b to point to the next unparsed byte
								as_value reply_as_value;
								if ( ! amf0_read_value(b, end, reply_as_value) )
								{
									log_error("parse amf failed");
									// this will happen if we get
									// bogus data, or if the data runs
									// off the end of the buffer
									// provided, or if we get data we
									// don't know how to parse
									break;
								}
								log_debug("parsed amf");

								// update variable to show how much we've parsed
								reply_start = b - reply.data();

								// if actionscript specified a callback object, call it
								boost::intrusive_ptr<as_object> callback = pop_callback(id);
								if(callback) {
									log_debug("calling callback");
									// FIXME check if above line can fail and we have to react
									callback->callMethod(NSV::PROP_ON_RESULT, reply_as_value);
									log_debug("callback called");
								} else {
									log_debug("couldn't find callback object");
								}
							}
						}
					}
				}
				else
				{
					log_error("Response from remoting service < 8 bytes");
				}

				log_debug("deleting connection");
				connection.reset();
				reply_start = 0;
				reply_end = 0;
			}
		}

		if(connection == 0 && queued_count > 0) {
			log_debug("creating connection");
			// set the "number of bodies" header
			(reinterpret_cast<boost::uint16_t*>(postdata.data() + 4))[0] = htons(queued_count);
			std::string postdata_str(reinterpret_cast<char*>(postdata.data()), postdata.size());
			log_debug("NetConnection.call(): encoded args from %1% calls: %2%", queued_count, hexify(postdata.data(), postdata.size(), false));
			queued_count = 0;
			connection.reset( StreamProvider::getDefaultInstance().getStream(url, postdata_str) );
			postdata.resize(6);
			log_debug("connection created");
		}

		if(connection == 0 && queued_count == 0) {
			log_debug("stopping ticking");
			stop_ticking();
			log_debug("ticking stopped");
		}
	};

	static as_value amfqueue_tick_wrapper(const fn_call& fn)
	{
		boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr);
		// FIXME check if it's possible for the URL of a NetConnection to change between call()s
		ptr->_callQueue->tick();
		return as_value();
	};

	void markReachableResources() const
	{
		for (CallbacksMap::const_iterator i=callbacks.begin(), e=callbacks.end(); i!=e; ++i)
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

	void push_callback(const std::string& id, boost::intrusive_ptr<as_object> callback) {
		callbacks.insert(std::pair<std::string, boost::intrusive_ptr<as_object> >(id, callback));
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

as_value
NetConnection::call_method(const fn_call& fn)
{
	static int call_number = 0;
	boost::intrusive_ptr<NetConnection> ptr = ensureType<NetConnection>(fn.this_ptr); 

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("NetConnection.call(): needs at least one argument"));
		);
		return as_value(false); // FIXME should we return true anyway?
	}

	as_value& methodName_as = fn.arg(0);
	if (!methodName_as.is_string()) {
                IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("NetConnection.call(%s): first argument (methodName) must be a string"), ss.str());
                );
		return as_value(false); // FIXME should we return true anyway?
	}

	std::stringstream ss; fn.dump_args(ss);
        log_debug("NetConnection.call(%s)", ss.str());

	// TODO: arg(1) is the response object. let it know when data comes back
	boost::intrusive_ptr<as_object> asCallback = 0;
	if(fn.nargs > 1) {

		if(fn.arg(1).is_object()) {
			asCallback = (fn.arg(1).to_object());
		}

		else {
            IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("NetConnection::call(%s): second argument must be an object", ss.str());
			);
		}
	}

	SimpleBuffer *buf = new SimpleBuffer(32);

	std::string methodName = methodName_as.to_string();

	// junk at the top (version, client id, 0 headers, 1 body)
	// this is done by AMFQueue now: buf->append("\000\000\000\000\000\001", 6);

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


	// encode array of arguments to remote method
	buf->appendByte(Element::STRICT_ARRAY_AMF0);
	buf->appendNetworkLong(fn.nargs - 2);
	if (fn.nargs > 2) {

		for (unsigned int i = 2; i < fn.nargs; ++i) {

			if (fn.arg(i).is_string()) {
				buf->appendByte(Element::STRING_AMF0);
				std::string str = fn.arg(i).to_string();
				buf->appendNetworkShort(str.size());
				buf->append(str.c_str(), str.size());
			// FIXME implement this
			//} else if(fn.arg(i).is_function()) {
			//	as_function f = fn.arg(i).to_function();
			//	tmp = AMF::encodefunction(f);
			}

			else if(fn.arg(i).is_number()) {
				double d = fn.arg(i).to_number();
				buf->appendByte(Element::NUMBER_AMF0);
				swapBytes(&d, 8); // this actually only swapps on little-endian machines
				buf->append(&d, 8);
 	 	 	// FIXME implement this
			//} else if(fn.arg(i).is_object()) {
			//	boost::intrusive_ptr<as_object> o = fn.arg(i).to_object();
			//	tmp = AMF::encodeObject(o);
			}
			
			else {
				log_error(_("NetConnection.call(): unknown argument type"));
				buf->appendByte(Element::UNDEFINED_AMF0);
			}
		}
	}

	// Set the "total size" parameter.
	*(reinterpret_cast<uint32_t*>(buf->data() + total_size_offset)) = htonl(buf->size() - 4 - total_size_offset);
	

	log_debug(_("NetConnection.call(): encoded args: %s"), hexify(buf->data(), buf->size(), false));

	// FIXME check that ptr->_prefixURL is valid
	URL url(ptr->validateURL(std::string()));


	// FIXME check if it's possible for the URL of a NetConnection to change between call()s
	if (! ptr->_callQueue.get()) {
		ptr->_callQueue.reset(new AMFQueue(*ptr, url));
	}

	if (asCallback) {
		//boost::intrusive_ptr<as_object> intrusive_callback(asCallback);
		log_debug("calling enqueue with callback");
		ptr->_callQueue->enqueue(*buf, callNumberString, asCallback);
		//? delete asCallback;
	}
	
	else {
		log_debug("calling enqueue without callback");
		ptr->_callQueue->enqueue(*buf);
	}
	log_debug("called enqueue");

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
	init_property("isConnected", &NetConnection::isConnected_getset, &NetConnection::isConnected_getset);
	init_property("uri", &NetConnection::uri_getset, &NetConnection::uri_getset);
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



} // end of gnash namespace

