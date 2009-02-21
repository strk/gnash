// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
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
#include <map>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/detail/endian.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include "log.h"
#include "amf.h"
#include "rtmp.h"
#include "rtmp_server.h"
#include "network.h"
#include "element.h"
#include "handler.h"
#include "utility.h"
#include "buffer.h"
#include "GnashSleep.h"
#include "crc.h"
#include "cache.h"
#include "diskstream.h"

using namespace gnash;
using namespace std;
using namespace amf;

namespace cygnal
{

// Get access to the global config data for Cygnal
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

// Get access to the global Cygnal cache
static Cache& cache = Cache::getDefaultInstance();

extern map<int, Handler *> handlers;

RTMPServer::RTMPServer() 
    : _filesize(0),
      _streamid(1)
{
//    GNASH_REPORT_FUNCTION;
//     _inbytes = 0;
//     _outbytes = 0;
    
//    _body = new unsigned char(RTMP_HANDSHAKE_SIZE+1);
//    memset(_body, 0, RTMP_HANDSHAKE_SIZE+1);
}

RTMPServer::~RTMPServer()
{
//    GNASH_REPORT_FUNCTION;
    _properties.clear();
//    delete _body;
}

#if 0
// The handshake is a byte with the value of 0x3, followed by 1536
// bytes of gibberish which we need to store for later.
bool
RTMPServer::processClientHandShake(int fd, amf::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    if (buf.reference() == 0) {
	log_debug("no data in buffer, net connection dropped for fd #%d", fd);
	return false;
    }

    cerr << buf.hexify(false) << endl;
    
    if (*buf.reference() == RTMP_HANDSHAKE) {
        log_debug (_("Handshake request is correct"));
    } else {
        log_error (_("Handshake request isn't correct"));
        return false;
    }

//     if (buf->size() >= RTMP_HANDSHAKE_SIZE) {
// 	secret = _handler->merge(buf->reference());
//     }

    if (buf.size() >= static_cast<size_t>(RTMP_HANDSHAKE_SIZE)) {
	_handshake = new amf::Buffer(RTMP_HANDSHAKE_SIZE);
	_handshake->copy(buf.reference() + 1, RTMP_HANDSHAKE_SIZE);
	log_debug (_("Handshake Data matched"));
//	return true;
    } else {
 	log_error (_("Handshake Data didn't match"));
// 	return false;
    }
    
    return true;
}
#endif

// The response is the gibberish sent back twice, preceeded by a byte
// with the value of 0x3. We have to very carefully send the handshake
// in one big packet as doing otherwise seems to cause subtle timing
// problems with the Adobe player. Tis way it connects every time.
bool
RTMPServer::handShakeResponse(int fd, amf::Buffer &handshake)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t byte;
    byte = RTMP_HANDSHAKE;

    boost::shared_ptr<amf::Buffer> zeros(new amf::Buffer(RTMP_HANDSHAKE_SIZE*2+1));
    zeros->clear();

    boost::uint8_t *ptr = zeros->reference();
    *ptr =  RTMP_HANDSHAKE;
    zeros->setSeekPointer(ptr + RTMP_HANDSHAKE_SIZE+1);

    zeros->append(handshake.reference()+1, handshake.allocated() - 1);
    size_t ret = writeNet(fd, *zeros);
    
    if (ret == zeros->allocated()) {
	log_debug("Sent RTMP Handshake response");
    } else {
	log_error("Couldn't sent RTMP Handshake response!");
    }

    return true;    
}

boost::shared_ptr<amf::Buffer>
RTMPServer::serverFinish(int fd, amf::Buffer &handshake1, amf::Buffer &handshake2)
{
    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<amf::Buffer> buf;

    if ((handshake1.reference() == 0) || (handshake2.reference() == 0)) {
	log_debug("Que empty, net connection dropped for fd #%d", fd);
	return buf;
    }

    int diff = std::memcmp(handshake1.begin(), handshake2.begin(), RTMP_HANDSHAKE_SIZE);
    if (diff <= 1) {
	log_debug (_("Handshake Finish Data matched"));
    } else {
	log_error (_("Handshake Finish Data didn't match by %d bytes"), diff);
    }

    // Copy the extra data from the end of the handshake to the new buffer. Normally we
    // try to avoid copying anything around, but as this is only used once for each connection,
    // there isn't a real performance hit from it.
    if (handshake2.allocated() >= static_cast<size_t>(RTMP_HANDSHAKE_SIZE)) {
	log_debug("Got extra data in handshake, %d bytes for fd #%d",
		  handshake2.allocated() - RTMP_HANDSHAKE_SIZE, fd);
	buf.reset(new Buffer(handshake2.allocated() - RTMP_HANDSHAKE_SIZE));
	buf->copy(handshake2.reference() + RTMP_HANDSHAKE_SIZE, handshake2.allocated() - RTMP_HANDSHAKE_SIZE);
    }
    
//    packetRead(*buf);
    return buf;
}

bool
RTMPServer::packetSend(amf::Buffer &/* buf */)
{
    GNASH_REPORT_FUNCTION;
    return false;
}

// This overrides using same method from the base RTMP class.
bool
RTMPServer::packetRead(amf::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t amf_index, headersize;
    boost::uint8_t *ptr = buf.reference();
    AMF amf;
    
    if (ptr == 0) {
	return false;
    }

    cerr << "FIXME3: " << buf.hexify(true) << endl;
    
//    ptr += 1;			// skip past the header byte
    
    amf_index = *ptr & RTMP_INDEX_MASK;
    headersize = headerSize(*ptr);
    log_debug (_("The Header size is: %d"), headersize);
    log_debug (_("The AMF index is: 0x%x"), amf_index);

//     if (headersize > 1) {
// 	packetsize = parseHeader(ptr);
//         if (packetsize) {
//             log_debug (_("Read first RTMP packet header of size %d"), packetsize);
//         } else {
//             log_error (_("Couldn't read first RTMP packet header"));
//             return false;
//         }
//     }

// #if 1
//     boost::uint8_t *end = buf->remove(0xc3);
// #else
//     boost::uint8_t *end = buf->find(0xc3);
//     log_debug("END is %x", (void *)end);
//     *end = '*';
// #endif
    decodeHeader(ptr);
    ptr += headersize;

    boost::uint8_t* tooFar = ptr+300+sizeof(int); // FIXME:
    
    AMF amf_obj;
    boost::shared_ptr<amf::Element> el1 = amf_obj.extractAMF(ptr, tooFar);
    ptr += amf_obj.totalsize();
    boost::shared_ptr<amf::Element> el2 = amf_obj.extractAMF(ptr, tooFar);

    int size = 0;
    boost::shared_ptr<amf::Element> el;
    while ( size < static_cast<boost::uint16_t>(_header.bodysize) - 24 ) {
	if (ptr) {
	    el = amf_obj.extractProperty(ptr, tooFar);
	    if (el != 0) {
		size += amf_obj.totalsize();
		ptr += amf_obj.totalsize();
//		_properties[el->getName()] = el;
	    } else {
		break;
	    }
//		log_debug("Bodysize is: %d size is: %d for %s", _total_size, size, el->getName());
	} else {
	    break;
	}
    }
    
# if 0
    Element el;
    ptr = amf.extractElement(&el, ptr);
    el.dump();
    ptr = amf.extractElement(&el, ptr) + 1;
    el.dump();
    log_debug (_("Reading AMF packets till we're done..."));
    buf->dump();
    while (ptr < end) {
	boost::shared_ptr<amf::Element> el(new amf::Element);
	ptr = amf.extractProperty(el, ptr);
	addProperty(el);
	el->dump();
    }
    ptr += 1;
    size_t actual_size = _total_size - RTMP_HEADER_SIZE;
    log_debug("Total size in header is %d, buffer size is: %d", _total_size, buf->size());
//    buf->dump();
    if (buf->size() < actual_size) {
	log_debug("FIXME: MERGING");
	buf = _que->merge(buf);
    }
    while ((ptr - buf->begin()) < static_cast<int>(actual_size)) {
	boost::shared_ptr<amf::Element> el(new amf::Element);
	if (ptr) {
	    ptr = amf.extractProperty(el, ptr);
	    addProperty(el);
	} else {
	    return true;
	}
	el->dump();		// FIXME: dump the AMF objects as they are read in
    }
    
    RTMPproto::dump();
#endif
    switch(_header.type) {
      case CHUNK_SIZE:
	  decodeChunkSize();
	  break;
      case BYTES_READ:
	  decodeBytesRead();
	  break;
      case PING:
      {
	  boost::shared_ptr<rtmp_ping_t> ping = decodePing(ptr);
	  switch (ping->type) {
	    case PING_CLEAR:
		break;
	    case PING_PLAY:
		break;
	    case PING_TIME:
		break;
	    case PING_RESET:
		break;
	    case PING_CLIENT:
		break;
	    case PONG_CLIENT:
		break;
	    default:
		return 0;
		break;
	  };
	  break;
      }
      case SERVER:
	  decodeServer();
	  break;
      case CLIENT:
	  decodeClient();
	  break;
      case VIDEO_DATA:
	  decodeVideoData();
	  break;
      case NOTIFY:
	  decodeNotify();
	  break;
      case SHARED_OBJ:
	  decodeSharedObj();
	  break;
      case INVOKE:
	  decodeInvoke();
          break;
      case AUDIO_DATA:	  
	  decodeAudioData();
          break;
      default:
          log_error (_("ERROR: Unidentified RTMP message content type 0x%x"), _header.type);
          break;
    };

    return true;
}

// A result packet looks like this:
// 
// 03 00 00 00 00 00 81 14 00 00 00 00 02 00 07 5f   ..............._
// 72 65 73 75 6c 74 00 3f f0 00 00 00 00 00 00 05   result.?........
// 03 00 0b 61 70 70 6c 69 63 61 74 69 6f 6e 05 00   ...application..
// 05 6c 65 76 65 6c 02 00 06 73 74 61 74 75 73 00   .level...status.
// 0b 64 65 73 63 72 69 70 74 69 6f 6e 02 00 15 43   .description...C
// 6f 6e 6e 65 63 74 69 6f 6e 20 73 75 63 63 65 65   onnection succee
// 64 65 64 2e 00 04 63 6f 64 65 02 00 1d 4e 65 74   ded...code...Net
// 43 6f 6e 6e 65 63 74 69 6f 6e 2e 43 6f 6e 6e 65   Connection.Conne
// 63 74 2e 53 75 63 63 65 73 73 00 00 c3 09         ct.Success....
//
// _result(double ClientStream, NULL, double ServerStream)
// These are handlers for the various types
boost::shared_ptr<Buffer>
RTMPServer::encodeResult(RTMPMsg::rtmp_status_e status)
{
//    GNASH_REPORT_FUNCTION;
    return encodeResult(status, _filespec, _streamid);
}
    
boost::shared_ptr<amf::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename)
{
//    GNASH_REPORT_FUNCTION;
    double clientid = 0.0;
    return encodeResult(status, filename, _streamid, clientid);
}

boost::shared_ptr<amf::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename, double &clientid)
{
//    GNASH_REPORT_FUNCTION;
    return encodeResult(status, filename, _streamid, clientid);
}

boost::shared_ptr<amf::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, double &streamid)
{
//    GNASH_REPORT_FUNCTION;
    double clientid = 0.0;
    return encodeResult(status, "", streamid, clientid);
}

boost::shared_ptr<amf::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename, double &streamid, double &clientid)
{
//    GNASH_REPORT_FUNCTION;
//    Buffer *buf = new Buffer;
//     boost::uint8_t *ptr = buf->reference();
//     buf->clear();		// default everything to zeros, real data gets optionally added.
//    ptr += sizeof(boost::uint16_t); // go past the first short
//     const char *capabilities = 0;
//     const char *description = 0;
//     const char *code = 0;
//     const char *status = 0;
    bool notobject = false;

    Element *str = new Element;
    str->makeString("_result");

    Element *number = new Element;
    // add The server ID
    number->makeNumber(streamid);	// FIXME: needs a real value, which should increment

    Element top;
//    top.makeObject("application");
    top.makeObject();
    
    switch (status) {
      case RTMPMsg::APP_GC:
      case RTMPMsg::APP_RESOURCE_LOWMEMORY:
      case RTMPMsg::APP_SCRIPT_ERROR:
      case RTMPMsg::APP_SCRIPT_WARNING:
      case RTMPMsg::APP_SHUTDOWN:
      case RTMPMsg::NC_CALL_BADVERSION:
      case RTMPMsg::NC_CALL_FAILED:
//	  status = 0;
//	  code = "NetConnection.Call.Failed";
      case RTMPMsg::NC_CONNECT_APPSHUTDOWN:
      case RTMPMsg::NC_CONNECT_CLOSED:
      case RTMPMsg::NC_CONNECT_FAILED:
      {
// 	  errstr = new Element;
// 	  errstr->makeString("error");
	  boost::shared_ptr<amf::Element> level(new Element);
	  level->makeString("level", "error");
	  top.addProperty(level);

	  boost::shared_ptr<amf::Element> description(new Element);
	  description->makeString("description", "Connection Failed.");
	  top.addProperty(description);
	  
	  boost::shared_ptr<amf::Element> code(new Element);
	  code->makeString("code", "Connection.Connect.Failed");
	  top.addProperty(code);
      }
      case RTMPMsg::NC_CONNECT_INVALID_APPLICATION:
      case RTMPMsg::NC_CONNECT_REJECTED:
      {
// 	  delete str;
// 	  str = new Element;
// 	  str->makeString("error");
	  boost::shared_ptr<amf::Element> level(new Element);
	  level->makeString("level", "error");
	  top.addProperty(level);

	  boost::shared_ptr<amf::Element> description(new Element);
	  description->makeString("description", "Connection Rejected.");
	  top.addProperty(description);
	  
	  boost::shared_ptr<amf::Element> code(new Element);
	  code->makeString("code", "NetConnection.Connect.Rejected");
	  top.addProperty(code);
      }
      case RTMPMsg::NC_CONNECT_SUCCESS:
      {
	  boost::shared_ptr<amf::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);
	  
	  boost::shared_ptr<amf::Element> code(new Element);
	  code->makeString("code", "NetConnection.Connect.Success");
	  top.addProperty(code);

	  boost::shared_ptr<amf::Element> description(new Element);
	  description->makeString("description", "Connection succeeded.");
	  top.addProperty(description);
      }
      break;
      case RTMPMsg::NS_CLEAR_FAILED:
      case RTMPMsg::NS_CLEAR_SUCCESS:
	  // After a successful NetConnection, we get a
	  // NetStream::createStream.
      case RTMPMsg::NS_DATA_START:
      case RTMPMsg::NS_FAILED:
      case RTMPMsg::NS_INVALID_ARGUMENT:
      case RTMPMsg::NS_PAUSE_NOTIFY:
      case RTMPMsg::NS_PLAY_COMPLETE:
      case RTMPMsg::NS_PLAY_FAILED:
      case RTMPMsg::NS_PLAY_FILE_STRUCTURE_INVALID:
      case RTMPMsg::NS_PLAY_INSUFFICIENT_BW:
      case RTMPMsg::NS_PLAY_NO_SUPPORTED_TRACK_FOUND:
      case RTMPMsg::NS_PLAY_PUBLISHNOTIFY:
	  break;
      // Reset the stream. We also do this after receiving a
      // NetStream::createStream() packet
      case RTMPMsg::NS_PLAY_RESET:
      {
	  str->makeString("onStatus");
//	  "clientid"
	  boost::shared_ptr<amf::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);

	  boost::shared_ptr<amf::Element> code(new Element);
	  code->makeString("code", "NetStream.Play.Reset");
	  top.addProperty(code);

	  boost::shared_ptr<amf::Element> description(new Element);
	  string field = "Playing and resetting ";
	  if (!filename.empty()) {
	      field += filename;
	  }
	  description->makeString("description", field);
	  top.addProperty(description);
	  
	  boost::shared_ptr<amf::Element> details(new Element);
	  details->makeString("details", filename);
	  top.addProperty(details);
	  
	  boost::shared_ptr<amf::Element> cid(new Element);
	  cid->makeNumber("clientid", clientid);
	  top.addProperty(cid);

	  break;
      }
      case RTMPMsg::NS_PLAY_START:
      {
	  str->makeString("onStatus");

	  boost::shared_ptr<amf::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);

	  boost::shared_ptr<amf::Element> code(new Element);
	  code->makeString("code", "NetStream.Play.Start");
	  top.addProperty(code);

	  boost::shared_ptr<amf::Element> description(new Element);
	  string field = "Started playing ";
	  if (!filename.empty()) {
	      field += filename;
	  }
	  description->makeString("description", field);
	  top.addProperty(description);
	  
	  boost::shared_ptr<amf::Element> details(new Element);
	  details->makeString("details", filename);
	  top.addProperty(details);
  
	  boost::shared_ptr<amf::Element> cid(new Element);
	  cid->makeNumber("clientid", clientid);
	  top.addProperty(cid);

	  break;
      }
      case RTMPMsg::NS_PLAY_STOP:
      case RTMPMsg::NS_PLAY_STREAMNOTFOUND:
      case RTMPMsg::NS_PLAY_SWITCH:
      case RTMPMsg::NS_PLAY_UNPUBLISHNOTIFY:
      case RTMPMsg::NS_PUBLISH_BADNAME:
      case RTMPMsg::NS_PUBLISH_START:
      case RTMPMsg::NS_RECORD_FAILED:
      case RTMPMsg::NS_RECORD_NOACCESS:
      case RTMPMsg::NS_RECORD_START:
      case RTMPMsg::NS_RECORD_STOP:
      case RTMPMsg::NS_SEEK_FAILED:
      case RTMPMsg::NS_SEEK_NOTIFY:
      case RTMPMsg::NS_UNPAUSE_NOTIFY:
      case RTMPMsg::NS_UNPUBLISHED_SUCCESS:
      case RTMPMsg::SO_CREATION_FAILED:
      case RTMPMsg::SO_NO_READ_ACCESS:
      case RTMPMsg::SO_NO_WRITE_ACCESS:
      case RTMPMsg::SO_PERSISTENCE_MISMATCH:
	  break;
      // Anything below here is specific to Gnash's implementation
      case RTMPMsg::NS_CREATE_STREAM:
      {
	  // Don't encode as an object, just the properties
	  notobject = true;

	  boost::shared_ptr<amf::Element> id2(new Element);
	  double sid = createStreamID();
	  id2->makeNumber(sid);
	  top.addProperty(id2);
	  
	  break;
      }
      case RTMPMsg::NS_DELETE_STREAM:
      default:
	  break;
    };
    
    boost::shared_ptr<amf::Buffer> strbuf = str->encode();
    boost::shared_ptr<amf::Buffer> numbuf = number->encode();
    boost::shared_ptr<amf::Buffer> topbuf = top.encode(notobject);

    boost::shared_ptr<amf::Buffer> buf(new Buffer(strbuf->size() + numbuf->size() + topbuf->size()));
    *buf += strbuf;
    *buf += numbuf;
    boost::uint8_t byte = static_cast<boost::uint8_t>(RTMP::SERVER & 0x000000ff);
    *buf += byte;
    *buf += topbuf;

    delete str;
    delete number;
    
    return buf;
}
// A Ping packet has two parameters that ae always specified, and 2 that are optional.
// The first two bytes are the ping type, as in rtmp_ping_e, the second is the ping
// target, which is always zero as far as we can tell.
//
// More notes from: http://jira.red5.org/confluence/display/docs/Ping
// type 0: Clear the stream. No third and fourth parameters. The second parameter could be 0.
// After the connection is established, a Ping 0,0 will be sent from server to client. The
// message will also be sent to client on the start of Play and in response of a Seek or
// Pause/Resume request. This Ping tells client to re-calibrate the clock with the timestamp
// of the next packet server sends.
// type 1: Tell the stream to clear the playing buffer.
// type 3: Buffer time of the client. The third parameter is the buffer time in millisecond.
// type 4: Reset a stream. Used together with type 0 in the case of VOD. Often sent before type 0.
// type 6: Ping the client from server. The second parameter is the current time.
// type 7: Pong reply from client. The second parameter is the time the server sent with his
//         ping request.

// A RTMP Ping packet looks like this: "02 00 00 00 00 00 06 04 00 00 00 00 00 00 00 00 00 0",
// which is the Ping type byte, followed by two shorts that are the parameters. Only the first
// two paramters are required.
boost::shared_ptr<Buffer>
RTMPServer::encodePing(rtmp_ping_e type)
{
//    GNASH_REPORT_FUNCTION;
    return encodePing(type, 0);
}

boost::shared_ptr<Buffer>
RTMPServer::encodePing(rtmp_ping_e type, boost::uint32_t milliseconds)
{
//    GNASH_REPORT_FUNCTION;

    // An encoded ping message 
    boost::shared_ptr<amf::Buffer> buf(new Buffer(sizeof(boost::uint16_t) * 3));
//    boost::uint8_t *ptr = buf->reference();

    // Set the type of this ping message
    boost::uint16_t typefield = htons(type);
    *buf = typefield;
    
//     // go past the first short, which is the type field
//    ptr += sizeof(boost::uint16_t);

    boost::uint32_t swapped = 0;
    switch (type) {
        // These two don't appear to have any paramaters
      case PING_CLEAR:
      case PING_PLAY:
	  break;
	  // the third parameter is the buffer time in milliseconds
      case PING_TIME:
      {
//	  ptr += sizeof(boost::uint16_t); // go past the second short
	  swapped = milliseconds;
	  swapBytes(&swapped, sizeof(boost::uint32_t));
	  *buf += swapped;
	  break;
      }
      // reset doesn't have any parameters but zeros
      case PING_RESET:
      {
	  boost::uint16_t zero = 0;
	  *buf += zero;
	  *buf += zero;
	  break;
      }
      // For Ping and Pong, the second parameter is always the milliseconds
      case PING_CLIENT:
      case PONG_CLIENT:
      {
//	  swapped = htonl(milliseconds);
	  swapped = milliseconds;
	  swapBytes(&swapped, sizeof(boost::uint32_t));
	  *buf += swapped;
	  break;
      }
      default:
	  break;
    };
    
    // Manually adjust the seek pointer since we added the data by
    // walking ou own temporary pointer, so none of the regular ways
    // of setting the seek pointer are appropriate.
//    buf->setSeekPointer(buf->reference() + buf->size());
    
    return buf;
}

// Parse an Echo Request message coming from the Red5 echo_test. This
// method should only be used for testing purposes.
vector<boost::shared_ptr<amf::Element > >
RTMPServer::parseEchoRequest(boost::uint8_t *ptr, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    AMF amf;
    vector<boost::shared_ptr<amf::Element > > headers;

    // The first element is the name of the test, 'echo'
    boost::shared_ptr<amf::Element> el1 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el1);

    // The second element is the number of the test,
    boost::shared_ptr<amf::Element> el2 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el2);

    // This one has always been a NULL object from my tests
    boost::shared_ptr<amf::Element> el3 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el3);

    // This one has always been an NULL or Undefined object from my tests
    boost::shared_ptr<amf::Element> el4 = amf.extractAMF(ptr, ptr+size);
    if (!el4) {
	log_error("Couldn't reliably extract the echo data!");
    }
    ptr += amf.totalsize();
    headers.push_back(el4);
    
    return headers;
}

// format a response to the 'echo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// a string '_result', followed by the number of the test, and then two
// NULL objects.
boost::shared_ptr<amf::Buffer>
RTMPServer::formatEchoResponse(double num, amf::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<amf::Buffer> data = amf::AMF::encodeElement(el);
    return formatEchoResponse(num, data->reference(), data->allocated());
}

boost::shared_ptr<amf::Buffer>
RTMPServer::formatEchoResponse(double num, amf::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

boost::shared_ptr<amf::Buffer>
RTMPServer::formatEchoResponse(double num, boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    string result = "_result";
    Element echo;
    echo.makeString(result);

    Element index;
    index.makeNumber(num);

    Element null;
    null.makeNull();

    boost::shared_ptr<amf::Buffer> encecho = echo.encode();
    boost::shared_ptr<amf::Buffer> encidx  = index.encode();   
    boost::shared_ptr<amf::Buffer> encnull  = null.encode();   

    boost::shared_ptr<amf::Buffer> buf(new amf::Buffer(encecho->size()
						       + encidx->size()
						       + encnull->size() + size));

    *buf = encecho;
    *buf += encidx;
    *buf += encnull;
    buf->append(data, size);

    return buf;
}

// Create a new client ID, which appears to be a random double.
double 
RTMPServer::createClientID()
{
//    GNASH_REPORT_FUNCTION;
    boost::mt19937 seed;
    // Pick the number of errors to create based on the Buffer's data size
    boost::uniform_real<> numbers(1, 65535);

    double id = numbers(seed);
    _clientids.push_back(id);

    return id;
}

// Get the next streamID
double 
RTMPServer::createStreamID()
{
//    GNASH_REPORT_FUNCTION;
    return _streamid++;
}

bool
RTMPServer::sendFile(int fd, const std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
    // See if the file is in the cache and already opened.
    boost::shared_ptr<DiskStream> filestream(cache.findFile(filespec));
    if (filestream) {
	cerr << "FIXME: found file in cache!" << endl;
    } else {
	filestream.reset(new DiskStream);
//	    cerr << "New Filestream at 0x" << hex << filestream.get() << endl;
	
//	    cache.addFile(url, filestream);	FIXME: always reload from disk for now.
	
	// Oopen the file and read the first chunk into memory
	if (!filestream->open(filespec)) {
	    return false;
	} else {
	    // Get the file size for the HTTP header
	    if (filestream->getFileType() == DiskStream::FILETYPE_NONE) {
		return false;
	    } else {
		cache.addPath(filespec, filestream->getFilespec());
	    }
	}
    }
    
    size_t filesize = filestream->getFileSize();
    size_t bytes_read = 0;
    int ret;
    size_t page = 0;
    if (filesize) {
#ifdef USE_STATS_CACHE
	struct timespec start;
	clock_gettime (CLOCK_REALTIME, &start);
#endif
	size_t getbytes = 0;
	if (filesize <= filestream->getPagesize()) {
	    getbytes = filesize;
	} else {
	    getbytes = filestream->getPagesize();
	}
	if (filesize >= CACHE_LIMIT) {
	    if (sendMsg(fd, getChannel(), RTMP::HEADER_12, filesize,
			RTMP::NOTIFY, RTMPMsg::FROM_SERVER, filestream->get(),
			filesize)) {
	    }
	    do {
		filestream->loadToMem(page);
//		ret = writeNet(fd, filestream->get(), getbytes);
// 		if (ret <= 0) {
// 		    break;
// 		}
		if (sendMsg(fd, getChannel(), RTMP::HEADER_4, filesize,
			    RTMP::NOTIFY, RTMPMsg::FROM_SERVER, filestream->get(),
			    getbytes)) {
		}
		bytes_read += ret;
		page += filestream->getPagesize();
	    } while (bytes_read <= filesize);
	} else {
	    filestream->loadToMem(filesize, 0);
//	    ret = writeNet(fd, filestream->get(), filesize);
	    if (sendMsg(fd, getChannel(), RTMP::HEADER_12, filesize,
			RTMP::NOTIFY, RTMPMsg::FROM_SERVER, filestream->get()+24,
			filesize-24)) {
	    }
					
	}
	filestream->close();
#ifdef USE_STATS_CACHE
	struct timespec end;
	clock_gettime (CLOCK_REALTIME, &end);
	double time = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9);
	cerr << "File " << _filespec
	     << " transferred " << filesize << " bytes in: " << fixed
	     << time << " seconds for net fd #" << fd << endl;
#endif
    }    

    return true;
}

// This is the thread for all incoming RTMP connections
bool
rtmp_handler(Network::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
//    Handler *hand = reinterpret_cast<Handler *>(args->handler);
    RTMPServer *rtmp = new RTMPServer;
    string docroot = args->filespec;
    string url, filespec;
    url = docroot;
    bool done = false;
    RTMPMsg *body = 0;
    static bool initialize = true;
    static bool echo = false;
    bool sendfile = false;
    log_debug(_("Starting RTMP Handler for fd #%d, tid %ld"),
	      args->netfd, get_thread_id());
    
#ifdef USE_STATISTICS
    struct timespec start;
    clock_gettime (CLOCK_REALTIME, &start);
#endif

    // Adjust the timeout
    rtmp->setTimeout(10);
    
    boost::shared_ptr<amf::Buffer>  pkt;
    boost::shared_ptr<amf::Element> tcurl;
    boost::shared_ptr<amf::Element> swfurl;
    boost::shared_ptr<amf::Buffer> response;

    string basepath = docroot;

    RTMP::rtmp_headersize_e response_head_size = RTMP::HEADER_12;
    
    // This handler is called everytime there is RTMP data on a socket to process the
    // messsage. Unlike HTTP, RTMP always uses persistant network connections, so we
    // only want to initialize the handshake once. This becomes important as the handshake
    // is always sent as a large data block, 1536 bytes. Once we start reading packets,
    // the default size is adjustable via the ChunkSize command.
    if (initialize) {
	// Read the handshake bytes sent by the client when requesting
	// a connection.
	boost::shared_ptr<amf::Buffer> handshake1 = rtmp->recvMsg(args->netfd);
	// See if we have data in the handshake, we should have 1537 bytes
	if (handshake1 == 0) {
	    log_error("failed to read the handshake from the client.");
	    return false;
	}

	// Send our response to the handshake, which primarily is the bytes
	// we just recieved.
	rtmp->handShakeResponse(args->netfd, *handshake1);
    
	boost::shared_ptr<amf::Buffer> handshake2 = rtmp->recvMsg(args->netfd);
	// See if we have data in the handshake, we should have 1536 bytes
	if (handshake2 == 0) {
	    log_error("failed to read the handshake from the client.");
	    return false;
	}
	// Don't assume the data we just read is a handshake.
	pkt = rtmp->serverFinish(args->netfd, *handshake1, *handshake2);
	if (pkt == 0) {
	    log_error("failed to read data from the end of the handshake from the client!");
	    return false;
	}
	// We got data
	if (pkt->allocated() > 0) {
	    initialize = false;
	}

	// Send a ping to reset the new stream
	boost::shared_ptr<amf::Buffer> ping_reset = rtmp->encodePing(RTMP::PING_RESET, 0);
	if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL, RTMP::HEADER_12,
			  ping_reset->size(), RTMP::PING, RTMPMsg::FROM_SERVER, *ping_reset)) {
	    log_debug("Sent Ping to client");
	} else {
	    log_error("Couldn't send Ping to client!");
	}

    } else {
	// Read the handshake bytes sent by the client when requesting
	// a connection.
    }

    // See if this is a Red5 style echo test.
    string::size_type pos;
    if (tcurl) {
	filespec = tcurl->to_string();
	pos = filespec.rfind("/");
	if (pos != string::npos) {
	    if (filespec.substr(pos, filespec.size()-pos) == "/echo") {
		log_debug("Red5 echo test request!");
		echo = true;
	    }
	}
    }
    
    // Keep track of the network statistics
    // See if we have any messages waiting. After the initial connect, this is
    // the main loop for processing messages.
    
    // Adjust the timeout
    rtmp->setTimeout(30);
//    boost::shared_ptr<amf::Buffer> buf;
 
    // This is the main message processing loop for rtmp. All message received require
    // a response.
    do {
	// If there is no data left from the previous chunk, process that before
	// reading more data.
	if (pkt != 0) {
	    log_debug("data left from previous packet");
	} else {
	    pkt = rtmp->recvMsg(args->netfd);
	}
	
	if (pkt != 0) {
	    boost::uint8_t *tmpptr = 0;
	    if (pkt->allocated()) {
		boost::shared_ptr<RTMP::queues_t> que = rtmp->split(*pkt);
		boost::shared_ptr<RTMP::rtmp_head_t> qhead;
		cerr << "FIXME1 incoming Que has " << que->size() << " messages." << endl;
		for (size_t i=0; i<que->size(); i++) {
		    boost::shared_ptr<amf::Buffer> bufptr = que->at(i)->pop();
//			que->at(i)->dump();
		    if (bufptr) {
			bufptr->dump();
			qhead = rtmp->decodeHeader(bufptr->reference());
			log_debug("Message for channel #%d", qhead->channel);
//			tmpptr = bufptr->reference();
			tmpptr = bufptr->reference() + qhead->head_size;
			if (qhead->channel == RTMP_SYSTEM_CHANNEL) {
			    boost::shared_ptr<RTMP::rtmp_ping_t> ping = rtmp->decodePing(tmpptr);
			    log_debug("Processed Ping message from client, type %d", ping->type);
			} else {
			    if (echo) {
				log_debug("Got an echo request");
				// process the echo test request
				vector<boost::shared_ptr<amf::Element> > request = rtmp->parseEchoRequest(
													  bufptr->reference() + qhead->head_size, bufptr->allocated() - qhead->head_size);
				if (request[3]) {
				    boost::shared_ptr<amf::Buffer> result = rtmp->formatEchoResponse(request[1]->to_number(), *request[3]);
				    if (rtmp->sendMsg(args->netfd, qhead->channel, RTMP::HEADER_8, result->allocated(),
						      RTMP::INVOKE, RTMPMsg::FROM_SERVER, *result)) {
					// If we're in single threaded mode, we Just want to stay in
					// this thread for now and do everything all at once. Otherwise
					// we're done, so we return to the dispatch handler waiting for
					// the next packet. Single threaded mode is primarily used by
					// developers for debugging protocols.
					log_debug("Sent echo test response response to client.");
				    }
				} else {
				    log_error("Couldn't send echo test response to client!");
				    done = true;
				}
			    } else {
				body = rtmp->decodeMsgBody(tmpptr, qhead->bodysize);
				if (body) {
				    body->setChannel(qhead->channel);
				    // Invoke the NetConnection::connect() method
				    if (body->getMethodName() == "connect") {
					response_head_size = RTMP::HEADER_12;
					tcurl  = body->findProperty("tcUrl");
					if (tcurl) {
					    log_debug("Client request for remote file is: %s", tcurl->to_string());
					    string path = tcurl->to_string();
					    string::size_type start = path.find("://", 0) + 4;
					    if (start != string::npos) {
						string::size_type end = path.find("/", start);
						if (end != string::npos) {
						    basepath += path.substr(end, path.size());
						}
					    }
					    log_debug("Base path to files from connect is: %s", basepath);
										
					}
					swfurl = body->findProperty("swfUrl");
					if (swfurl) {
					    log_debug("SWF filename making request is: %s", swfurl->to_string());
					    // See if this is a Red5 style echo test.
					    string::size_type pos;
					    filespec = swfurl->to_string();
					    pos = filespec.rfind("/");
					    if (pos != string::npos) {
						if (filespec.substr(pos, filespec.size()-pos) == "/echo_test.swf") {
						    log_debug("Red5 echo test request!");
						    echo = true;
						}
					    }
					}
					
					response = rtmp->encodeResult(RTMPMsg::NC_CONNECT_SUCCESS);
					
					// Send a ping to reset the new stream
					boost::shared_ptr<amf::Buffer> ping_reset = rtmp->encodePing(RTMP::PING_RESET, 0);
					if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL, RTMP::HEADER_12,
							  ping_reset->size(), RTMP::PING, RTMPMsg::FROM_SERVER, *ping_reset)) {
					    log_debug("Sent Ping to client");
					} else {
					    log_error("Couldn't send Ping to client!");
					}
					
				    }
				    // Invoke the NetStream::createStream() method
				    if (body->getMethodName() == "createStream") {
					double streamid  = body->getStreamID();
					log_debug("The streamID from NetStream::createStream() is: %d", streamid);
					response_head_size = RTMP::HEADER_8;
					response = rtmp->encodeResult(RTMPMsg::NS_CREATE_STREAM, streamid);
//					body->dump();
				    }
				    if (body->getMethodName() == "deleteStream") {
					double streamid  = body->getStreamID();
					log_debug("The streamID from NetStream::deleyeStream() is: %d", streamid);
					response_head_size = RTMP::HEADER_8;
					response = rtmp->encodeResult(RTMPMsg::NS_DELETE_STREAM, streamid);
					body->dump();
				    }
				    // Invoke the NetStream::play() method
				    if (body->getMethodName() == "play") {
					double streamid  = body->getStreamID();
					log_debug("The streamID from NetStream::plays: %d", streamid);
					filespec = body->at(1)->to_string();
					response_head_size = RTMP::HEADER_8;
					double clientid = rtmp->createClientID();
//  					response = rtmp->encodeResult(RTMPMsg::NS_PLAY_RESET, filespec);
					body->setChannel(4);
//  					rtmp->sendMsg(args->netfd, body->getChannel(), response_head_size, response->allocated(),
//  							  RTMP::INVOKE, RTMPMsg::FROM_SERVER, *response);
//  					response.reset();
// 					rtmp->setChannel(4);
//  					rtmp->sendMsg(args->netfd, body->getChannel(), response_head_size, response->allocated(),
					response = rtmp->encodeResult(RTMPMsg::NS_PLAY_START, filespec, clientid);
//					body->dump();
					sendfile = true;
				    }
				    if (body->getMethodName() == "recData") {
				    }
				    if (body->getMethodName() == "onEchoonServ") {
				    }
				    if (rtmp->sendMsg(args->netfd, body->getChannel(), response_head_size, response->allocated(),
						      RTMP::INVOKE, RTMPMsg::FROM_SERVER, *response)) {
					log_error("Sent response to client.");
				    } else {
					log_error("Couldn't send response to client!");
				    }
				    if (sendfile) {
					string fullspec = basepath;
					fullspec += filespec;
					rtmp->sendFile(args->netfd, fullspec);
					sendfile = false;
				    } 
				}
			    }
			}
		    } else {
			log_error("Message contains no data!");
		    }
		} // end of processing all the messages in the que
		
		// we're done processing these packets, so get rid of them
		pkt.reset();


		
#if 0    
		// This is support for the Red5 'echo_test', which exercises encoding and
		// decoding of complex and nested AMF data types. FIXME: this should be
		// moved to a CGI type of thing that executes this as a separate process,
		// using a socket to pass output back to the client.
		if (echo) {
		    boost::shared_ptr<RTMP::queues_t> que = rtmp->split(*pkt);
		    boost::shared_ptr<amf::Buffer> bufptr;
		    if (que->size() > 0) {
			cerr << "FIXME2 echo Que size is: " << que->size() << endl;
 			bufptr = que->at(0)->pop();
		    }
		    // process the echo test request
		    vector<boost::shared_ptr<amf::Element> > request = rtmp->parseEchoRequest(
			bufptr->reference() + rthead->head_size, bufptr->allocated() - rthead->head_size);
		    // now build a result
		    if (request[3]) {
			boost::shared_ptr<amf::Buffer> result = rtmp->formatEchoResponse(request[1]->to_number(), *request[3]);
			if (rtmp->sendMsg(args->netfd, rthead->channel, RTMP::HEADER_8, result->allocated(),
					  RTMP::INVOKE, RTMPMsg::FROM_SERVER, *result)) {
			    // If we're in single threaded mode, we Just want to stay in
			    // this thread for now and do everything all at once. Otherwise
			    // we're done, so we return to the dispatch handler waiting for
			    // the next packet. Single threaded mode is primarily used by
			    // developers for debugging protocols.
			    log_debug("Sent echo test response response to client.");
			}
		    } else {
			log_error("Couldn't send echo test response to client!");
			done = true;
		    }
		} else {	// end of Red5 echo test support
//		    buf->dump();
		    // This is a non-Red5 message, which should be the normal mode of operating.
//		    boost::shared_ptr<RTMP::queues_t> que = rtmp->split(*pkt);
		    if (que->size() > 0) {
			boost::shared_ptr<amf::Buffer> bufptr;
			if (que->size() > 0) {
			    bufptr = que->at(0)->pop();
			}
			if (bufptr) {
			    boost::shared_ptr<RTMP::rtmp_head_t> qhead = rtmp->decodeHeader(bufptr->reference());
			    
			    for (size_t i=0; i<que->size(); i++) {
				boost::uint8_t *tmpptr = bufptr->reference() + qhead->head_size;
				body = rtmp->decodeMsgBody(tmpptr, qhead->bodysize);
				boost::shared_ptr<amf::Buffer> response;
				if (body) {
				    if (body->getMethodName() == "connect") {
					response = rtmp->encodeResult(RTMPMsg::NC_CONNECT_SUCCESS);
				    } else if (body->getMethodName() == "createStream") {
					response = rtmp->encodeResult(RTMPMsg::NS_DATA_START);
				    } else {
					response = rtmp->encodeResult(RTMPMsg::NS_FAILED);
				    }
				} else {
				    response = rtmp->encodeResult(RTMPMsg::NS_FAILED);
				}
				
				if (rtmp->sendMsg(args->netfd, qhead->channel, RTMP::HEADER_8, response->allocated(),
						  RTMP::INVOKE, RTMPMsg::FROM_SERVER, *response)) {
				    log_error("Sent response to client.");
				} else {
				    log_error("Couldn't send response to client!");
				}
	
				
			    }
			} else {
			    log_error("%s:%d Message contains no data!", __FUNCTION__, __LINE__);
			}
		    }
		}
#endif

		
	    } else {
		log_error("Never read any data from fd #%d", args->netfd);
#if 0
		// Send a ping to reset the new stream
		boost::shared_ptr<amf::Buffer> ping_reset = rtmp->encodePing(RTMP::PING_CLEAR, 0);
		if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL, RTMP::HEADER_12,
				  ping_reset->size(), RTMP::PING, RTMPMsg::FROM_SERVER, *ping_reset)) {
		    log_debug("Sent Ping to client");
		} else {
		    log_error("Couldn't send Ping to client!");
		}
#endif
		initialize = true;
		return false;
	    }
	} else {
	    log_error("Communication error with client using fd #%d", args->netfd);
	    initialize = true;
	    return false;
	}
    } while (!done);
    
    return true;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
