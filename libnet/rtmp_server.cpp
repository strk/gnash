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

using namespace gnash;
using namespace std;
using namespace amf;

namespace gnash
{

extern map<int, Handler *> handlers;

RTMPServer::RTMPServer() 
    : _filesize(0)
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
// with the value of 0x3.
bool
RTMPServer::handShakeResponse(int fd, amf::Buffer &handshake)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t byte;
    byte = RTMP_HANDSHAKE;
    
    int ret1 = writeNet(fd, &byte, 1);
    int ret2 = writeNet(fd, handshake);
    int ret3 = writeNet(fd, handshake);
    
    if ((ret2 == handshake.allocated()) && (ret3 == handshake.allocated())) {
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
    if (diff == 0) {
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

#if 0
    boost::shared_ptr<amf::Element> url = getProperty("tcUrl");
    boost::shared_ptr<amf::Element> file = getProperty("swfUrl");
    boost::shared_ptr<amf::Element> app = getProperty("app");
    
    if (file) {
	log_debug("SWF file %s", file->to_string());
    }
    if (url) {
	log_debug("is Loading video %s", url->to_string());
    }
    if (app) {
	log_debug("is file name is %s", app->to_string());
    }
#endif
    
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
    GNASH_REPORT_FUNCTION;
    
//    Buffer *buf = new Buffer;
//     boost::uint8_t *ptr = buf->reference();
//     buf->clear();		// default everything to zeros, real data gets optionally added.
//    ptr += sizeof(boost::uint16_t); // go past the first short
//     const char *capabilities = 0;
//     const char *description = 0;
//     const char *code = 0;
//     const char *status = 0;

    Element *str = new Element;
    str->makeString("_result");

    Element *number = new Element;
    // add The server ID
    number->makeNumber(1);	// FIXME: needs a real value, which should increment

    Element top;
    top.makeObject("application");
    
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

	  boost::shared_ptr<amf::Element> description(new Element);
	  description->makeString("description", "Connection succeeded.");
	  top.addProperty(description);
	  
	  boost::shared_ptr<amf::Element> code(new Element);
	  code->makeString("code", "NetConnection.Connect.Success");
	  top.addProperty(code);
      }
      break;
      case RTMPMsg::NS_CLEAR_FAILED:
      case RTMPMsg::NS_CLEAR_SUCCESS:
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
      case RTMPMsg::NS_PLAY_RESET:
      case RTMPMsg::NS_PLAY_START:
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
      default:
	  break;
    };
    
    boost::shared_ptr<amf::Buffer> strbuf = str->encode();
    boost::shared_ptr<amf::Buffer> numbuf = number->encode();
    boost::shared_ptr<amf::Buffer> topbuf = top.encode();

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
    static bool initialize = true;
    
    log_debug(_("Starting RTMP Handler for fd #%d, tid %ld"),
	      args->netfd, get_thread_id());
    
#ifdef USE_STATISTICS
    struct timespec start;
    clock_gettime (CLOCK_REALTIME, &start);
#endif

    // Adjust the timeout
    rtmp->setTimeout(5);
    
    boost::shared_ptr<amf::Buffer> pkt;
    
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
	if (handshake1->allocated() == 0) {
	    log_error("failed to read the handshake from the client.");
	    return false;
	}

	// Send our response to the handshake, which primarily is the bytes
	// we just recieved.
	rtmp->handShakeResponse(args->netfd, *handshake1);
    
	boost::shared_ptr<amf::Buffer> handshake2 = rtmp->recvMsg(args->netfd);
	// See if we have data in the handshake, we should have 1536 bytes
	if (handshake2->allocated() == 0) {
	    log_error("failed to read the handshake from the client.");
	    return false;
	}
	// Don't assume the data we just read is a handshake.
	pkt = rtmp->serverFinish(args->netfd, *handshake1, *handshake2);
	// We got data
	if (pkt->allocated() > 0) {
	    initialize = false;
	}
    } else {
	// Read the handshake bytes sent by the client when requesting
	// a connection.
	pkt = rtmp->recvMsg(args->netfd);
	// See if we have data in the handshake, we should have 1537 bytes
	if (pkt->allocated() == 0) {
	    log_error("failed to read RTMP data from the client.");
	    return false;
	}
    }

    // The very first message after the handshake is the Invoke call of
    // NetConnection::connect().
    boost::shared_ptr<RTMP::rtmp_head_t> head = rtmp->decodeHeader(*pkt);
    RTMPMsg *body = 0;
    RTMP::queues_t *que = rtmp->split(*pkt);
//    RTMP::queues_t *que = rtmp->split(start->reference() + head->head_size, start->size());
    if (que->size() > 0) {
	boost::shared_ptr<amf::Buffer> bufptr = que->at(0)->pop();
	body = rtmp->decodeMsgBody(bufptr->reference() + head->head_size, head->bodysize);
	body->dump();
    }

    // Send a ping to clear the new stream
    boost::shared_ptr<amf::Buffer> ping_reset = rtmp->encodePing(RTMP::PING_CLEAR, 0);
    bool what1 = rtmp->sendMsg(args->netfd, RTMP_SYSTEM_CHANNEL, RTMP::HEADER_12,
			       ping_reset->size(), RTMP::PING, RTMPMsg::FROM_SERVER, *ping_reset);
    
    // send a response
    boost::shared_ptr<amf::Buffer> response = rtmp->encodeResult(RTMPMsg::NC_CONNECT_SUCCESS);
    bool what = rtmp->sendMsg(args->netfd, head->channel, RTMP::HEADER_12,
			      response->allocated(), RTMP::INVOKE,
			      RTMPMsg::FROM_SERVER, *response);

    // Keep track of the network statistics
//    Statistics st;
//    st.setFileType(NetStats::RTMP);
// 	st.stopClock();
//  	log_debug (_("Bytes read: %d"), proto.getBytesIn());
//  	log_debug (_("Bytes written: %d"), proto.getBytesOut());
// 	st.setBytes(proto.getBytesIn() + proto.getBytesOut());
// 	st.addStats();
// 	proto.resetBytesIn();
// 	proto.resetBytesOut();	
    
//	st.dump(); 
    // See if we have any messages waiting
    boost::shared_ptr<amf::Buffer> buf = rtmp->recvMsg(args->netfd);
    if (buf->allocated()) {
	boost::uint8_t *ptr = buf->reference();
	if (ptr == 0) {
	    log_debug("Que empty, net connection dropped for fd #%d", args->netfd);
	    return false;
	}
	boost::shared_ptr<RTMP::rtmp_head_t> rthead = rtmp->decodeHeader(ptr);
	body = rtmp->decodeMsgBody(*buf);
    } else {
	done = true;
    }

    return false;
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
    GNASH_REPORT_FUNCTION;
    return encodePing(type, 0);
}

boost::shared_ptr<Buffer>
RTMPServer::encodePing(rtmp_ping_e type, boost::uint32_t milliseconds)
{
    GNASH_REPORT_FUNCTION;
    
    boost::shared_ptr<amf::Buffer> buf(new Buffer(sizeof(boost::uint16_t) * 3));
    boost::uint8_t *ptr = buf->reference();
    buf->clear();	// default everything to zeros, real data gets optionally added.
    // Manually adjust the seek pointer since we add the data by
    // walking ou own temporary pointer, so none of the regular ways
    // of setting the seek pointer are appropriate.
    buf->setSeekPointer(buf->reference() + buf->size());

    boost::uint16_t typefield = htons(type);
    ptr += sizeof(boost::uint16_t); // go past the first short

    boost::uint32_t swapped = 0;
    *buf = typefield;
    switch (type) {
        // These two don't appear to have any paramaters
      case PING_CLEAR:
      case PING_PLAY:
	  break;
	  // the third parameter is the buffer time in milliseconds
      case PING_TIME:
      {
	  ptr += sizeof(boost::uint16_t); // go past the second short
	  swapped = milliseconds;
	  swapBytes(&swapped, sizeof(boost::uint32_t));
	  *buf += swapped;
	  break;
      }
      // reset doesn't have any parameters but zeros
      case PING_RESET:
	  break;
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
    buf->setSeekPointer(buf->reference() + buf->size());
    
    return buf;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
