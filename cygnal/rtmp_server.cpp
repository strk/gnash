// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
#include <cstdlib>
#include <cstdio>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/detail/endian.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/lexical_cast.hpp>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include "log.h"
#include "URL.h"
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
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif 
using namespace gnash;
using namespace std;

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


boost::shared_ptr<cygnal::Element>
RTMPServer::processClientHandShake(int fd)
{
    GNASH_REPORT_FUNCTION;

    log_network(_("Processing RTMP Handshake for fd #%d"), fd);
    
#ifdef USE_STATISTICS
    struct timespec start;
    clock_gettime (CLOCK_REALTIME, &start);
#endif
    
    // Adjust the timeout for reading from the network
    RTMP::setTimeout(10);
    
    // These store the information we need from the initial
    /// NetConnection object.
    boost::scoped_ptr<cygnal::Element> nc;
    boost::shared_ptr<cygnal::Buffer>  pkt;
    boost::shared_ptr<cygnal::Element> tcurl;
    boost::shared_ptr<cygnal::Element> swfurl;
    boost::shared_ptr<cygnal::Element> encoding;

//     RTMP::rtmp_headersize_e response_head_size = RTMP::HEADER_12;
    
    // Read the handshake bytes sent by the client when requesting
    // a connection.
    boost::shared_ptr<cygnal::Buffer> handshake1 = RTMP::recvMsg(fd);
    // See if we have data in the handshake, we should have 1537 bytes
    if (!handshake1) {
	log_error("Failed to read the handshake from the client.");
	return tcurl;		// nc is empty
    } else {
	log_network("Read first handshake from the client.");
    }
    
    // Send our response to the handshake, which primarily is the bytes
    // we just received.
    handShakeResponse(fd, *handshake1);
    
    // Read the response from the client from the handshale reponse we
    // just sent.
    boost::shared_ptr<cygnal::Buffer> handshake2 = RTMP::recvMsg(fd);
    // See if we have data in the handshake, we should have 1536 bytes
    if (handshake2 == 0) {
	log_error("failed to read the handshake from the client.");
	return tcurl;		// nc is empty
    } else {
	log_network("Read second handshake from the client.");
    }
    
    // Don't assume the data we just read is a handshake.
    pkt = serverFinish(fd, *handshake1, *handshake2);
    // Wmake sure we got data before trying to process it
    if (!pkt) {
	log_error("Didn't receive any data in handshake!");
	tcurl.reset(new cygnal::Element);
	return tcurl;		// nc is empty
    }
    
    // the packet is a raw RTMP message. Since the header can be a
    // variety of sizes, and this effects the data size, we need to
    // decode that first.
    boost::shared_ptr<RTMP::rtmp_head_t> qhead = RTMP::decodeHeader(pkt->reference());

    if (!qhead) {
	log_error("RTMP header had parsing error!");
	return tcurl;		// nc is empty
    }

    // We know the first packet is always a NetConnection INVOKE of
    // the connect() method. These are usually around 300-400 bytes in
    // testing, so anything larger than that is suspicios.
    if (qhead->bodysize > 1024) {
	log_error("NetConnection unusually large! %d", qhead->bodysize);
    }

    // Get the actual start of the data
    boost::uint8_t *ptr = pkt->reference() + qhead->head_size;

    // See if we have enough data to go past the chunksize, which is
    // probable. If so, all chunks are the default size of 128, the
    // same size as used for video packets. This means every chunksize
    // boundary is an RTMP header byte that must be removed, or the
    // data in the NetConnection::connect() packet will be
    // corrupted. There is probably a better way to do this, but for
    // now build a copy of the data but skip over the RTMP header
    // bytes every chunk size biundary. All RTMP headers at this stage
    // are 1 byte ones.
    boost::scoped_ptr<cygnal::Buffer> newptr(new cygnal::Buffer(qhead->bodysize));
    if (qhead->bodysize > RTMP_VIDEO_PACKET_SIZE) {
	log_network("De chunkifying the NetConnection packet.");
	int nbytes = 0;
	while (nbytes < qhead->bodysize) {
	    size_t chunk = RTMP_VIDEO_PACKET_SIZE;
	    if ((qhead->bodysize - nbytes) < RTMP_VIDEO_PACKET_SIZE) {
		chunk = qhead->bodysize - nbytes;
	    }
	    newptr->append(ptr + nbytes, chunk);
	    nbytes += chunk + 1;
	}
    } else {
	newptr->copy(ptr, qhead->bodysize);
    }

    // extract the body of the message from the packet
    _netconnect = RTMP::decodeMsgBody(newptr->begin(), qhead->bodysize);
    if (!_netconnect) {
	log_error("failed to read the body of the handshake data from the client.");
	return tcurl;		// nc is empty
    } else {
	log_network("Read handshake data body from the client.");
    }

    // make sure this is actually a NetConnection packet.
    if (_netconnect->getMethodName() != "connect") {
	log_error("Didn't receive NetConnection object in handshake!");
	return tcurl;		// nc is empty
    } else {
	log_network("Got NetConnection ::connect() INVOKE.");
	_netconnect->dump();	// FIXME: debug crap
    }
    
    // Get the data for the fields we want.
    tcurl  = _netconnect->findProperty("tcUrl");
    swfurl  = _netconnect->findProperty("swfUrl");
    encoding  = _netconnect->findProperty("objectEncoding");

    // based on the Red5 tests, I see two behaviours with this next
    // packet. If only gets sent when the "objectEncoding" field of
    // the NetConnection object is in the initial packet. When this is
    // supplied, it's more remoting than streaming, so sending this
    // causes Async I/O errors in the client.
    if (!encoding) {
	// Send a onBWDone to the client to start the new NetConnection,
	boost::shared_ptr<cygnal::Buffer> bwdone = encodeBWDone(2.0);
	if (RTMP::sendMsg(fd, qhead->channel, RTMP::HEADER_8,
			  bwdone->size(), RTMP::INVOKE, RTMPMsg::FROM_SERVER, *bwdone)) {
	    log_network("Sent onBWDone to client");
	} else {
	    log_error("Couldn't send onBWDone to client!");
	    tcurl.reset();
	    return tcurl;		// nc is empty
	}
    }
    
    // Send a Set Client Window Size to the client
    boost::shared_ptr<cygnal::Buffer> winsize(new cygnal::Buffer(sizeof(boost::uint32_t)));
    boost::uint32_t swapped = 0x20000;
    swapBytes(&swapped, sizeof(boost::uint32_t));
    *winsize += swapped;
    if (RTMP::sendMsg(fd, RTMP_SYSTEM_CHANNEL, RTMP::HEADER_12,
		      winsize->size(), RTMP::WINDOW_SIZE, RTMPMsg::FROM_CLIENT, *winsize)) {
	log_network("Sent set Client Window Size to client");
    } else {
	log_error("Couldn't send set Client Window Size to client!");
	tcurl.reset();
	return tcurl;		// nc is empty
    }

    // Send a ping to the client to reset the new NetConnection,
    boost::shared_ptr<cygnal::Buffer> ping_reset =
	encodePing(RTMP::PING_RESET, 0);
    if (RTMP::sendMsg(fd, RTMP_SYSTEM_CHANNEL, RTMP::HEADER_8,
		      ping_reset->size(), RTMP::USER, RTMPMsg::FROM_SERVER, *ping_reset)) {
	log_network("Sent Ping to client");
    } else {
	log_error("Couldn't send Ping to client!");
	tcurl.reset();
	return tcurl;		// nc is empty
    }

    // Send the packet to notify the client that the
    // NetConnection::connect() was sucessful. After the client
    // receives this, the handhsake is completed.
    boost::shared_ptr<cygnal::Buffer> response =
	encodeResult(RTMPMsg::NC_CONNECT_SUCCESS);
    if (RTMP::sendMsg(fd, 3, RTMP::HEADER_8, response->allocated(),
		      RTMP::INVOKE, RTMPMsg::FROM_SERVER, *response)) {
	log_network("Sent response to client.");
    } else {
	log_error("Couldn't send response to client!");
	tcurl.reset();
	return tcurl;		// nc is empty
    }

    return tcurl;
}

// The response is the gibberish sent back twice, preceeded by a byte
// with the value of 0x3. We have to very carefully send the handshake
// in one big packet as doing otherwise seems to cause subtle timing
// problems with the Adobe player. This way it connects every time.
bool
RTMPServer::handShakeResponse(int fd, cygnal::Buffer &handshake)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t byte;
    byte = RTMP_VERSION;

    // the response handshake is twice the size of the one we just
    // received for a total of 3072 bytes, plus room for the version.
    boost::scoped_ptr<cygnal::Buffer> zeros(new cygnal::Buffer(RTMP_HANDSHAKE_SIZE*2
					 + RTMP_HANDSHAKE_VERSION_SIZE));
    zeros->clear();		// set entire buffer to zeros

    boost::uint8_t *ptr = zeros->reference();

    // the first byte of the handshake response is the RTMP version
    // number.
    *ptr =  RTMP_VERSION;

    // the first half we make all zeros, as it doesn't appear to be
    // used for anything. More data is the second half of the
    // response.
    zeros->setSeekPointer(ptr + RTMP_HANDSHAKE_VERSION_SIZE + 
			  RTMP_HANDSHAKE_SIZE);

    // the handhshake has a two field header, which appears to be
    // timestamp, followed by another field that appears to be another
    // timestamp or version number, which is probably ignored.
    // the first field of the header is the timestamp
    boost::uint32_t timestamp;
    // Get the timestamp of when this message was read
    timestamp = RTMP::getTime();
    *zeros += timestamp;

    // the second field is always zero
    boost::uint32_t pad = 0;
    *zeros += pad;

    // the data starts after the vesion and header bytes
    size_t offset = RTMP_HANDSHAKE_VERSION_SIZE + RTMP_HANDSHAKE_HEADER_SIZE;

    // add the handshake data, which is 1528 byte of random stuff.
    zeros->append(handshake.reference() + offset, RTMP_RANDOM_SIZE);

    // send the handshake to the client
    size_t ret = writeNet(fd, *zeros);
    
    if (ret == zeros->allocated()) {
	log_network("Sent RTMP Handshake response at %d", timestamp);
    } else {
	log_error("Couldn't sent RTMP Handshake response at %d!", timestamp);
    }

    return true;    
}

boost::shared_ptr<cygnal::Buffer>
RTMPServer::serverFinish(int fd, cygnal::Buffer &handshake1, cygnal::Buffer &handshake2)
{
    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Buffer> buf;

    // sanity check our input data. We do this seperately as an empty
    // buffer means data wasn't read correctly from the network. We
    // should never get this far with bad data, but when it comes to
    // network programming, a little caution is always good.
    if (handshake1.empty()) {
	log_error("No data in original handshake buffer.");
	return buf;		// return empty buffer
    }
    if (handshake2.empty()) {
	log_error("No data in response handshake buffer.");
	return buf;		// return empty buffer
    }

    // the first field of the header is the timestamp of the original
    // packet sent by this server.
    boost::uint32_t timestamp1 = *reinterpret_cast<boost::uint32_t *>
	(handshake1.reference() + RTMP_HANDSHAKE_VERSION_SIZE);

    // the second field of the header is the timestamp of the previous
    // packet sent by this server.
    boost::uint32_t timestamp2 = *reinterpret_cast<boost::uint32_t *>
	(handshake1.reference() + RTMP_HANDSHAKE_VERSION_SIZE + sizeof(boost::uint32_t));

    log_network("The timestamp delta is %d", timestamp2 - timestamp1);

    // This is the location in the second handshake to the random data
    // block used in the handshake.
    size_t pkt_size = RTMP_HANDSHAKE_VERSION_SIZE + RTMP_HANDSHAKE_SIZE;
    // the handshakes are supposed to match.
    int diff = std::memcmp(handshake1.begin()
	   + RTMP_HANDSHAKE_VERSION_SIZE + RTMP_HANDSHAKE_HEADER_SIZE,
			   handshake2.begin()
	   + pkt_size + RTMP_HANDSHAKE_HEADER_SIZE,
			   RTMP_RANDOM_SIZE);
    if (diff <= 1) {
	log_network (_("Handshake Finish Data matched"));
    } else {
	log_error (_("Handshake Finish Data didn't match by %d bytes"), diff);
// 	return buf;		// return empty buffer
    }

    // Copy the extra data from the end of the handshake to the new
    // buffer. Normally we  try to avoid copying anything around, but
    // as this is only used once for each connection, there isn't a
    // real performance hit from it.
    size_t amf_size = handshake2.allocated() - pkt_size;
    if (handshake2.allocated() >= pkt_size) {
	log_network("Got AMF data in handshake, %d bytes for fd #%d",
		    amf_size, fd);
	buf.reset(new Buffer(amf_size));
	// populate the buffer with the AMF data
	boost::uint8_t *ptr = handshake2.reference() + RTMP_HANDSHAKE_SIZE;
	buf->copy(ptr, amf_size);
    }
    
    return buf;
}

bool
RTMPServer::packetSend(cygnal::Buffer &/* buf */)
 {
    GNASH_REPORT_FUNCTION;
    return false;
}

// This overrides using same method from the base RTMP class.
bool
RTMPServer::packetRead(cygnal::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t amf_index, headersize;
    boost::uint8_t *ptr = buf.reference();
    AMF amf;
    
    if (ptr == 0) {
	return false;
    }

//     cerr << "FIXME3: " << buf.hexify(true) << endl;
    
//    ptr += 1;			// skip past the header byte
    
    amf_index = *ptr & RTMP_INDEX_MASK;
    headersize = headerSize(*ptr);
    log_network (_("The Header size is: %d"), headersize);
    log_network (_("The AMF index is: 0x%x"), amf_index);

//     if (headersize > 1) {
// 	packetsize = parseHeader(ptr);
//         if (packetsize) {
//             log_network (_("Read first RTMP packet header of size %d"), packetsize);
//         } else {
//             log_error (_("Couldn't read first RTMP packet header"));
//             return false;
//         }
//     }

// #if 1
//     boost::uint8_t *end = buf->remove(0xc3);
// #else
//     boost::uint8_t *end = buf->find(0xc3);
//     log_network("END is %x", (void *)end);
//     *end = '*';
// #endif
    decodeHeader(ptr);
    ptr += headersize;

    boost::uint8_t* tooFar = ptr+300+sizeof(int); // FIXME:
    
    AMF amf_obj;
    boost::shared_ptr<cygnal::Element> el1 = amf_obj.extractAMF(ptr, tooFar);
    ptr += amf_obj.totalsize();
    boost::shared_ptr<cygnal::Element> el2 = amf_obj.extractAMF(ptr, tooFar);

    int size = 0;
    boost::shared_ptr<cygnal::Element> el;
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
//		log_network("Bodysize is: %d size is: %d for %s", _total_size, size, el->getName());
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
    log_network (_("Reading AMF packets till we're done..."));
    // buf->dump();
    while (ptr < end) {
	boost::shared_ptr<cygnal::Element> el(new cygnal::Element);
	ptr = amf.extractProperty(el, ptr);
	addProperty(el);
	// el->dump();
    }
    ptr += 1;
    size_t actual_size = _total_size - RTMP_HEADER_SIZE;
    log_network("Total size in header is %d, buffer size is: %d", _total_size, buf->size());
//    buf->dump();
    if (buf->size() < actual_size) {
	log_network("FIXME: MERGING");
	buf = _que->merge(buf);
    }
    while ((ptr - buf->begin()) < static_cast<int>(actual_size)) {
	boost::shared_ptr<cygnal::Element> el(new cygnal::Element);
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
      case USER:
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
      case WINDOW_SIZE:
	  decodeServer();
	  break;
      case SET_BANDWITH:
	  decodeClient();
	  break;
      case ROUTE:
	  log_unimpl("Route");
	  break;
      case AUDIO_DATA:	  
	  decodeAudioData();
          break;
      case VIDEO_DATA:
	  decodeVideoData();
	  break;
      case SHARED_OBJ:
	  decodeSharedObj();
	  break;
      case AMF3_NOTIFY:
	  log_unimpl("AMF3 Notify");
	  break;
      case AMF3_SHARED_OBJ:
	  log_unimpl("AMF3 Shared Object");
	  break;
      case AMF3_INVOKE:
	  log_unimpl("AMF3 Invoke");
	  break;
      case NOTIFY:
	  decodeNotify();
	  break;
      case INVOKE:
	  decodeInvoke();
          break;
      case FLV_DATA:
	  log_unimpl("FLV Dat");
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
    
boost::shared_ptr<cygnal::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename)
{
//    GNASH_REPORT_FUNCTION;
    double clientid = 0.0;
    return encodeResult(status, filename, _streamid, clientid);
}

boost::shared_ptr<cygnal::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename, double &clientid)
{
//    GNASH_REPORT_FUNCTION;
    return encodeResult(status, filename, _streamid, clientid);
}

boost::shared_ptr<cygnal::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, double &transid)
{
//    GNASH_REPORT_FUNCTION;
    double clientid = 0.0;
    return encodeResult(status, "", transid, clientid);
}

boost::shared_ptr<cygnal::Buffer> 
RTMPServer::encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename, double &transid, double &clientid)
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
    // add the transaction ID
    number->makeNumber(transid);

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
	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "error");
	  top.addProperty(level);

	  boost::shared_ptr<cygnal::Element> description(new Element);
	  description->makeString("description", "Connection Failed.");
	  top.addProperty(description);
	  
	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "Connection.Connect.Failed");
	  top.addProperty(code);
      }
      case RTMPMsg::NC_CONNECT_INVALID_APPLICATION:
      case RTMPMsg::NC_CONNECT_REJECTED:
      {
// 	  delete str;
// 	  str = new Element;
// 	  str->makeString("error");
	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "error");
	  top.addProperty(level);

	  boost::shared_ptr<cygnal::Element> description(new Element);
	  description->makeString("description", "Connection Rejected.");
	  top.addProperty(description);
	  
	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "NetConnection.Connect.Rejected");
	  top.addProperty(code);
      }
      case RTMPMsg::NC_CONNECT_SUCCESS:
      {
	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);
	  
	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "NetConnection.Connect.Success");
	  top.addProperty(code);

	  boost::shared_ptr<cygnal::Element> description(new Element);
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
	  // The response to a successful pauseStream command is this
	  // message.
      case RTMPMsg::NS_PAUSE_NOTIFY:
      {
	  str->makeString("onStatus");

	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);

	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "NetStream.Pause.Notify");
	  top.addProperty(code);

	  boost::shared_ptr<cygnal::Element> description(new Element);
	  string field = "Pausing ";
	  if (!filename.empty()) {
	      field += filename;
	  }
	  description->makeString("description", field);
	  top.addProperty(description);
	  
	  boost::shared_ptr<cygnal::Element> details(new Element);
	  details->makeString("details", filename);
	  top.addProperty(details);
  
	  boost::shared_ptr<cygnal::Element> cid(new Element);
	  cid->makeNumber("clientid", clientid);
	  top.addProperty(cid);

	  break;
      }
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
	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);

	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "NetStream.Play.Reset");
	  top.addProperty(code);

	  boost::shared_ptr<cygnal::Element> description(new Element);
	  string field = "Playing and resetting ";
	  if (!filename.empty()) {
	      field += filename;
	  }
	  description->makeString("description", field);
	  top.addProperty(description);
	  
	  boost::shared_ptr<cygnal::Element> details(new Element);
	  details->makeString("details", filename);
	  top.addProperty(details);
	  
	  boost::shared_ptr<cygnal::Element> cid(new Element);
#ifdef CLIENT_ID_NUMERIC
	  double clientid = createClientID();
	  cid->makeNumber("clientid", clientid);
#else
	  string clientid;
	  if (!_clientids[transid].empty()) {
	      clientid =_clientids[transid].c_str();
	  } else {
	      clientid = createClientID();
	      _clientids[transid] = clientid;
	  }
	  cid->makeString("clientid", _clientids[transid]);
#endif
	  top.addProperty(cid);

	  break;
      }
      case RTMPMsg::NS_PLAY_START:
      {
	  str->makeString("onStatus");

	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "status");
	  top.addProperty(level);

	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "NetStream.Play.Start");
	  top.addProperty(code);

	  boost::shared_ptr<cygnal::Element> description(new Element);
	  string field = "Started playing ";
	  if (!filename.empty()) {
	      field += filename;
	  }
	  description->makeString("description", field);
	  top.addProperty(description);
	  
	  boost::shared_ptr<cygnal::Element> details(new Element);
	  details->makeString("details", filename);
	  top.addProperty(details);
  
	  boost::shared_ptr<cygnal::Element> cid(new Element);
#ifdef CLIENT_ID_NUMERIC
	  double clientid = createClientID();
	  cid->makeNumber("clientid", clientid);
#else
	  string clientid;
	  if (!_clientids[transid].empty()) {
	      clientid =_clientids[transid].c_str();
	  } else {
	      clientid = createClientID();
	      _clientids[transid] = clientid;
	  }
	  cid->makeString("clientid", _clientids[transid]);
#endif
	  top.addProperty(cid);

	  break;
      }
      case RTMPMsg::NS_PLAY_STOP:
      case RTMPMsg::NS_PLAY_STREAMNOTFOUND:
      {
	  boost::shared_ptr<cygnal::Element> level(new Element);
	  level->makeString("level", "error");
	  top.addProperty(level);

	  boost::shared_ptr<cygnal::Element> description(new Element);
	  description->makeString("description", "NetStream.Play.StreamNotFound.");
	  top.addProperty(description);
	  
	  boost::shared_ptr<cygnal::Element> code(new Element);
	  code->makeString("code", "NetStream.Play.StreamNotFound");
	  top.addProperty(code);
	  break;
      }
      case RTMPMsg::NS_PLAY_SWITCH:
      case RTMPMsg::NS_PLAY_UNPUBLISHNOTIFY:
      case RTMPMsg::NS_PUBLISH_BADNAME:
      case RTMPMsg::NS_PUBLISH_START:
      case RTMPMsg::NS_RECORD_FAILED:
      case RTMPMsg::NS_RECORD_NOACCESS:
      case RTMPMsg::NS_RECORD_START:
      case RTMPMsg::NS_RECORD_STOP:
	  // The reponse to a failed seekStream is this message.
      case RTMPMsg::NS_SEEK_FAILED:
	  // The reponse to a successful seekStream is this message.
      case RTMPMsg::NS_SEEK_NOTIFY:
	  break;
	  // The response to a successful pauseStream command is this
	  // message when the stream is started again.
      case RTMPMsg::NS_UNPAUSE_NOTIFY:
      case RTMPMsg::NS_UNPUBLISHED_SUCCESS:
      case RTMPMsg::SO_CREATION_FAILED:
      case RTMPMsg::SO_NO_READ_ACCESS:
      case RTMPMsg::SO_NO_WRITE_ACCESS:
      case RTMPMsg::SO_PERSISTENCE_MISMATCH:
	  break;
	  // The response for a createStream message is the
	  // transaction ID, followed by the command object (usually a
	  // NULL object), and the Stream ID. The Stream ID is just a
	  // simple incrementing counter of streams.
      case RTMPMsg::NS_CREATE_STREAM:
      {
	  // Don't encode as an object, just the properties
	  notobject = true;

	  boost::shared_ptr<cygnal::Element> id2(new Element);
	  
	  double sid = createStreamID();
	  id2->makeNumber(sid);
	  top.addProperty(id2);
	  
	  break;
      }
      // There is no response to a deleteStream request.
      case RTMPMsg::NS_DELETE_STREAM:
      default:
	  break;
    };
    
    boost::shared_ptr<cygnal::Buffer> strbuf = str->encode();
    boost::shared_ptr<cygnal::Buffer> numbuf = number->encode();
    boost::shared_ptr<cygnal::Buffer> topbuf = top.encode(notobject);

    boost::shared_ptr<cygnal::Buffer> buf(new Buffer(strbuf->size() + numbuf->size() + topbuf->size()));
    *buf += strbuf;
    *buf += numbuf;
    boost::uint8_t byte = static_cast<boost::uint8_t>(RTMP::WINDOW_SIZE & 0x000000ff);
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
    boost::shared_ptr<cygnal::Buffer> buf(new Buffer(sizeof(boost::uint16_t) * 3));
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

// Encode a onBWDone message for the client. These are of a fixed size.
boost::shared_ptr<cygnal::Buffer>
RTMPServer::encodeBWDone(double id)
{
//    GNASH_REPORT_FUNCTION;
    string command = "onBWDone";

    Element cmd;
    cmd.makeString(command);

    Element num;
    num.makeNumber(id);

    Element null;
    null.makeNull();

    boost::shared_ptr<cygnal::Buffer> enccmd  = cmd.encode();
    boost::shared_ptr<cygnal::Buffer> encnum  = num.encode();
    boost::shared_ptr<cygnal::Buffer> encnull  = null.encode();

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(enccmd->size()
						       + encnum->size()
						       + encnull->size()));

    *buf += enccmd;
    *buf += encnum;
    *buf += encnull;
	
    return buf;
}

boost::shared_ptr<cygnal::Buffer>
RTMPServer::encodeAudio(boost::uint8_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
    boost::shared_ptr<cygnal::Buffer> buf;
    
    if (size) {
	if (data) {
	    buf.reset(new cygnal::Buffer(size));
	    buf->copy(data, size);
	}
    }

    return buf;
}

boost::shared_ptr<cygnal::Buffer>
RTMPServer::encodeVideo(boost::uint8_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
}

#if 0
// Parse an Echo Request message coming from the Red5 echo_test. This
// method should only be used for testing purposes.
vector<boost::shared_ptr<cygnal::Element > >
RTMPServer::parseEchoRequest(boost::uint8_t *ptr, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    AMF amf;
    vector<boost::shared_ptr<cygnal::Element > > headers;

    // The first element is the name of the test, 'echo'
    boost::shared_ptr<cygnal::Element> el1 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el1);

    // The second element is the number of the test,
    boost::shared_ptr<cygnal::Element> el2 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el2);

    // This one has always been a NULL object from my tests
    boost::shared_ptr<cygnal::Element> el3 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el3);

    // This one has always been an NULL or Undefined object from my tests
    boost::shared_ptr<cygnal::Element> el4 = amf.extractAMF(ptr, ptr+size);
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
boost::shared_ptr<cygnal::Buffer>
RTMPServer::formatEchoResponse(double num, cygnal::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Buffer> data = amf::AMF::encodeElement(el);
    return formatEchoResponse(num, data->reference(), data->allocated());
}

boost::shared_ptr<cygnal::Buffer>
RTMPServer::formatEchoResponse(double num, cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

boost::shared_ptr<cygnal::Buffer>
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

    boost::shared_ptr<cygnal::Buffer> encecho = echo.encode();
    boost::shared_ptr<cygnal::Buffer> encidx  = index.encode();   
    boost::shared_ptr<cygnal::Buffer> encnull  = null.encode();   

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(encecho->size()
						       + encidx->size()
						       + encnull->size() + size));

    *buf = encecho;
    *buf += encidx;
    *buf += encnull;
    buf->append(data, size);

    return buf;
}
#endif

// Create a new client ID, which appears to be a random double,
// although I also see a temporary 8 character string used often as
// well.
#ifdef CLIENT_ID_NUMERIC
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
#else
std::string
RTMPServer::createClientID()
{
//    GNASH_REPORT_FUNCTION;
    string id;

// FIXME: This turns out to be a crappy random number generator,
    // and should be replaced with something less repititous.
#if 0
    boost::mt19937 seed;
    for (size_t i=0; i < 8; i++) {
	boost::uniform_int<> numbers(0x30, 0x7a);
	id += numbers(seed);
    }
#else
    char letters[] =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    boost::uint64_t random_time_bits = 0;
    boost::uint64_t value = 0;
# ifdef HAVE_GETTIMEOFDAY
    timeval tv;
    gettimeofday(&tv, NULL);
    random_time_bits = ((uint64_t)tv.tv_usec << 16) ^ tv.tv_sec;
# else
    random_time_bits = time(NULL);
# endif
    value += random_time_bits ^ getpid();
    boost::uint64_t v = value; 
    id = letters[v % 62];
    v /= 62;
    id += letters[v % 62];
    v /= 62;
    id += letters[v % 62];
    v /= 62;
    id += letters[v % 62];
    v /= 62;
    id += letters[v % 62];
    v /= 62;
    id += letters[v % 62];
    v /= 62;
    id += letters[v % 62];
    v /= 62;
#endif
    
    return id;
}
#endif

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
	
	// Open the file and read the first chunk into memory
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
    int ret = 0;
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

size_t
RTMPServer::sendToClient(std::vector<int> &fds, cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return sendToClient(fds, data.reference(), data.allocated());
}

size_t
RTMPServer::sendToClient(std::vector<int> &fds, boost::uint8_t *data,
		      size_t size)
{
//    GNASH_REPORT_FUNCTION;
    size_t ret = 0;
    
    std::vector<int>::iterator it;
    for (it=fds.begin(); it< fds.end(); it++) {
	ret = writeNet(data, size);
    }
    
    return ret;
}

// This is the thread for all incoming RTMP connections
bool
rtmp_handler(Network::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;

    Handler *hand = reinterpret_cast<Handler *>(args->handler);
    RTMPServer *rtmp = reinterpret_cast<RTMPServer *>(args->entry);
    // RTMPServer *rtmp = new RTMPServer;
    
    string docroot = args->filespec;
    string url, filespec;
    url = docroot;
    bool done = false;
    boost::shared_ptr<RTMPMsg> body;
    static bool initialize = true;
//     bool sendfile = false;
    log_network(_("Starting RTMP Handler for fd #%d, cgi-bin is \"%s\""),
		args->netfd, args->filespec);
    
#ifdef USE_STATISTICS
    struct timespec start;
    clock_gettime (CLOCK_REALTIME, &start);
#endif

    // Adjust the timeout
    rtmp->setTimeout(10);
    
    boost::shared_ptr<cygnal::Buffer>  pkt;
    boost::shared_ptr<cygnal::Element> tcurl;
    boost::shared_ptr<cygnal::Element> swfurl;
    boost::shared_ptr<cygnal::Buffer> response;

    // Keep track of the network statistics
    // See if we have any messages waiting. After the initial connect, this is
    // the main loop for processing messages.
    
    // Adjust the timeout
    rtmp->setTimeout(30);
//    boost::shared_ptr<cygnal::Buffer> buf;

    // If we have active disk streams, send those packets first.
    // 0 is a reserved stream, so we start with 1, as the reserved
    // stream isn't one we care about here.
    log_network("%d active disk streams", hand->getActiveDiskStreams());
    for (int i=1; i <= hand->getActiveDiskStreams(); i++) {
	hand->getDiskStream(i)->dump();
	if (hand->getDiskStream(i)->getState() == DiskStream::PLAY) {
	    boost::uint8_t *ptr = hand->getDiskStream(i)->get();
	    if (ptr) {
		if (rtmp->sendMsg(hand->getClient(i), 8,
			RTMP::HEADER_8, 4096,
			RTMP::NOTIFY, RTMPMsg::FROM_SERVER,
			ptr, 4096)) {
		}
	    } else {
		log_network("ERROR: No stream for client %d", i);
	    }
	}
    }
    
    // This is the main message processing loop for rtmp. Most
    // messages received require a response.
    do {
	// If there is no data left from the previous chunk, process
	// that before reading more data.
	if (pkt != 0) {
	    log_network("data left from previous packet");
	} else {
	    pkt = rtmp->recvMsg(args->netfd);
	}
	
	if (pkt != 0) {
	    boost::uint8_t *tmpptr = 0;
	    if (pkt->allocated()) {
		boost::shared_ptr<RTMP::queues_t> que = rtmp->split(*pkt);
		if (!que) {
		    // FIXME: send _error result
		    return false;
		}
		boost::shared_ptr<RTMP::rtmp_head_t> qhead;
		for (size_t i=0; i<que->size(); i++) {
		    boost::shared_ptr<cygnal::Buffer> bufptr = que->at(i)->pop();
		    // que->at(i)->dump();
		    if (bufptr) {
			// bufptr->dump();
			qhead = rtmp->decodeHeader(bufptr->reference());
			if (!qhead) {
			    return false;
			}
 			// log_network("Message for channel #%d", qhead->channel);
			tmpptr = bufptr->reference() + qhead->head_size;
			if (qhead->channel == RTMP_SYSTEM_CHANNEL) {
			    if (qhead->type == RTMP::USER) {
				boost::shared_ptr<RTMP::user_event_t> user
				    = rtmp->decodeUserControl(tmpptr);
				switch (user->type) {
				  case RTMP::STREAM_START:
				      log_unimpl("Stream Start");
				      break;
				  case RTMP::STREAM_EOF:
				      log_unimpl("Stream EOF");
				      break;
				  case RTMP::STREAM_NODATA:
				      log_unimpl("Stream No Data");
				      break;
				  case RTMP::STREAM_BUFFER:
				      log_unimpl("Stream Set Buffer: %d", user->param2);
				      break;
				  case RTMP::STREAM_LIVE:
				      log_unimpl("Stream Live");
				      break;
				  case RTMP::STREAM_PING:
				  {
				      boost::shared_ptr<RTMP::rtmp_ping_t> ping
					  = rtmp->decodePing(tmpptr);
				      log_network("Processed Ping message from client, type %d",
						  ping->type);
				      break;
				  }
				  case RTMP::STREAM_PONG:
				      log_unimpl("Stream Pong");
				      break;
				  default:
				      break;
				};
			    } else if (qhead->type == RTMP::AUDIO_DATA) {
				log_network("Got the 1st Audio packet!");
			    } else if (qhead->type == RTMP::VIDEO_DATA) {
				log_network("Got the 1st Video packet!");
			    } else if (qhead->type == RTMP::WINDOW_SIZE) {
				log_network("Got the Window Set Size packet!");
			    } else {
				log_network("Got unknown system message!");
				bufptr->dump();
			    }
			}
		    }
		    switch (qhead->type) {
		      case RTMP::CHUNK_SIZE:
			  log_unimpl("Set Chunk Size");
			  break;
		      case RTMP::BYTES_READ:
			  log_unimpl("Bytes Read");
			  break;
		      case RTMP::ABORT:
		      case RTMP::USER:
			  // already handled as this is a system channel message
			  return true;
			  break;
		      case RTMP::WINDOW_SIZE:
			  log_unimpl("Set Window Size");
			  break;
		      case RTMP::SET_BANDWITH:
			  log_unimpl("Set Bandwidth");
			  break;
		      case RTMP::ROUTE:
		      case RTMP::AUDIO_DATA:
		      case RTMP::VIDEO_DATA:
		      case RTMP::SHARED_OBJ:
			  body = rtmp->decodeMsgBody(tmpptr, qhead->bodysize);
			  log_network("SharedObject name is \"%s\"", body->getMethodName());
			  break;
		      case RTMP::AMF3_NOTIFY:
			  log_unimpl("RTMP type %d", qhead->type);
			  break;
		      case RTMP::AMF3_SHARED_OBJ:
			  log_unimpl("RTMP type %d", qhead->type);
			  break;
		      case RTMP::AMF3_INVOKE:
			  log_unimpl("RTMP type %d", qhead->type);
			  break;
		      case RTMP::NOTIFY:
			  log_unimpl("RTMP type %d", qhead->type);
			  break;
		      case RTMP::INVOKE:
		      {
			  body = rtmp->decodeMsgBody(tmpptr, qhead->bodysize);
			  if (!body) {
			      log_error("Error INVOKING method \"%s\"!", body->getMethodName());
			      continue;
			  }
			  log_network("INVOKEing method \"%s\"", body->getMethodName());
			  // log_network("%s", hexify(tmpptr, qhead->bodysize, true));
			  
			  // These next Invoke methods are for the
			  // NetStream class, which like NetConnection,
			  // is a speacial one handled directly by the
			  // server instead of any cgi-bin plugins.
			  double transid  = body->getTransactionID();
			  log_network("The Transaction ID from the client is: %g", transid);
			  if (body->getMethodName() == "createStream") {
			      hand->createStream(transid);
			      response = rtmp->encodeResult(RTMPMsg::NS_CREATE_STREAM, transid);
			      if (rtmp->sendMsg(args->netfd, qhead->channel,
					RTMP::HEADER_8, response->allocated(),
					RTMP::INVOKE, RTMPMsg::FROM_SERVER,
					*response)) {
			      }
			  } else if (body->getMethodName() == "play") {
			      string filespec;
			      boost::shared_ptr<gnash::RTMPMsg> nc = rtmp->getNetConnection();
			      boost::shared_ptr<cygnal::Element> tcurl = nc->findProperty("tcUrl");
			      URL url(tcurl->to_string());
			      filespec += url.hostname() + url.path();
			      filespec += '/';
			      filespec += body->at(1)->to_string();

			      if (hand->playStream(filespec)) {
				  // Send the Set Chunk Size response
#if 1
				  response = rtmp->encodeChunkSize(4096);
				  if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL,
					RTMP::HEADER_12, response->allocated(),
					RTMP::CHUNK_SIZE, RTMPMsg::FROM_SERVER,
					*response)) {
				  }
#endif
			      // Send the Play.Resetting response
				  response = rtmp->encodeResult(RTMPMsg::NS_PLAY_RESET, body->at(1)->to_string(), transid);
				  if (rtmp->sendMsg(args->netfd, qhead->channel,
					RTMP::HEADER_8, response->allocated(),
					RTMP::INVOKE, RTMPMsg::FROM_SERVER,
					*response)) {
				  }
				  // Send the Play.Start response
				  response = rtmp->encodeResult(RTMPMsg::NS_PLAY_START, body->at(1)->to_string(), transid);
				  if (rtmp->sendMsg(args->netfd, qhead->channel,
					RTMP::HEADER_8, response->allocated(),
					RTMP::INVOKE, RTMPMsg::FROM_SERVER,
					*response)) {
				  }
			      } else {
				  response = rtmp->encodeResult(RTMPMsg::NS_PLAY_STREAMNOTFOUND, body->at(1)->to_string(), transid);
				  if (rtmp->sendMsg(args->netfd, qhead->channel,
					RTMP::HEADER_8, response->allocated(),
					RTMP::INVOKE, RTMPMsg::FROM_SERVER,
					*response)) {
				  }
			      }
			      sleep(1); // FIXME: debugging crap
			      // Send the User Control - Stream Live
			      response = rtmp->encodeUserControl(RTMP::STREAM_LIVE, 1);
			      if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL,
					RTMP::HEADER_12, response->allocated(),
					RTMP::USER, RTMPMsg::FROM_SERVER,
					*response)) {
			      }
			      sleep(1); // FIXME: debugging crap
			      // Send an empty Audio packet to get
			      // things started.
			      if (rtmp->sendMsg(args->netfd, 6,
					RTMP::HEADER_12, 0,
					RTMP::AUDIO_DATA, RTMPMsg::FROM_SERVER,
					0, 0)) {
			      }
			      // Send an empty Video packet to get
			      // things started.
			      if (rtmp->sendMsg(args->netfd, 5,
					RTMP::HEADER_12, 0,
					RTMP::VIDEO_DATA, RTMPMsg::FROM_SERVER,
					0, 0)) {
			      }
			      sleep(1); // FIXME: debugging crap
			      // Send the User Control - Stream Start
			      response = rtmp->encodeUserControl(RTMP::STREAM_START, 1);
			      if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL,
					RTMP::HEADER_12, response->allocated(),
					RTMP::USER, RTMPMsg::FROM_SERVER,
					*response)) {
			      }			      
			      int active_stream = hand->getActiveDiskStreams();
			      boost::uint8_t *ptr = hand->getDiskStream(active_stream)->get();
			      if (ptr) {
				  log_network("Sending %s to client",
					      hand->getDiskStream(active_stream)->getFilespec());
				  if (rtmp->sendMsg(args->netfd, 5,
					RTMP::HEADER_12, 400,
					RTMP::NOTIFY, RTMPMsg::FROM_SERVER,
					ptr, 400)) {
				      log_network("Sent first page to client");
				  }
			      }  
			  } else if (body->getMethodName() == "seek") {
			      hand->seekStream();
			  } else if (body->getMethodName() == "pause") {
			      hand->pauseStream(transid);
			  } else if (body->getMethodName() == "close") {
			      hand->closeStream(transid);
			  } else if (body->getMethodName() == "resume") {
			      hand->resumeStream(transid);
			  } else if (body->getMethodName() == "delete") {
			      hand->deleteStream(transid);
			  } else if (body->getMethodName() == "publish") {
			      hand->publishStream();
			  } else if (body->getMethodName() == "togglePause") {
			      hand->togglePause(transid);
			      // This is a server installation specific  method.
			  } else if (body->getMethodName() == "FCSubscribe") {
			      hand->setFCSubscribe(body->at(0)->to_string());
			  } else if (body->getMethodName() == "_error") {
			      log_error("Received an _error message from the client!");
			  } else {
			      /* size_t ret = */ hand->writeToPlugin(tmpptr, qhead->bodysize);
			      boost::shared_ptr<cygnal::Buffer> result = hand->readFromPlugin();
			      if (result) {
				  if (rtmp->sendMsg(args->netfd, qhead->channel,
						    RTMP::HEADER_8, result->allocated(),
						    RTMP::INVOKE, RTMPMsg::FROM_SERVER,
						    *result)) {
				      log_network("Sent response to client.");
				  }
			      }
			      done = true;
			  }
			  break;
		      }
		      case RTMP::FLV_DATA:
			  log_unimpl("RTMP type %d", qhead->type);
			  break;
		      default:
			  log_error (_("ERROR: Unidentified AMF header data type 0x%x"), qhead->type);
			  break;
		    };
    
// 		    body->dump();

		    // size_t ret = hand->writeToPlugin(tmpptr, qhead->bodysize);
#if 0
		    boost::shared_ptr<cygnal::Buffer> result = hand->readFromPlugin();
 		    if (result) { // FIXME: this needs a real channel number
			if (rtmp->sendMsg(args->netfd, 0x3, RTMP::HEADER_8, ret,
					  RTMP::INVOKE, RTMPMsg::FROM_SERVER, *result)) {
			    log_network("Sent response to client.");
			}
 		    }
#endif		    
// 		    log_network("RET is: %d", ret);
		} // end of processing all the messages in the que
		
		// we're done processing these packets, so get rid of them
		pkt.reset();

		
	    } else {
		log_network("Never read any data from fd #%d", args->netfd);
#if 0
		// Send a ping to reset the new stream
		boost::shared_ptr<cygnal::Buffer> ping_reset =
		    rtmp->encodePing(RTMP::PING_CLEAR, 0);
		if (rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL,
			  RTMP::HEADER_12, ping_reset->size(),
			  RTMP::PING, RTMPMsg::FROM_SERVER, *ping_reset)) {
		    log_network("Sent Ping to client");
		} else {
		    log_error("Couldn't send Ping to client!");
		}
#endif
		initialize = true;
		return true;
	    }
	} else {
	    // log_error("Communication error with client using fd #%d", args->netfd);
	    rtmp->closeNet(args->netfd);
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
