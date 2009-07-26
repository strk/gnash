// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
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
#include <map>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include <boost/shared_ptr.hpp>
#include "log.h"
#include "rc.h"
#include "amf.h"
#include "rtmp.h"
#include "rtmp_client.h"
#include "network.h"
#include "element.h"
// #include "handler.h"
#include "utility.h"
#include "buffer.h"
#include "GnashSleep.h"

using namespace gnash;
using namespace std;
using namespace amf;

typedef boost::shared_ptr<amf::Element> ElementSharedPtr;

namespace gnash
{

extern const char *ping_str[];

// The rcfile is loaded and parsed here:
static RcInitFile& rcfile = RcInitFile::getDefaultInstance();

// extern map<int, Handler *> handlers;

RTMPClient::RTMPClient()
    : _connected(false),
      _connections(0)
{
//    GNASH_REPORT_FUNCTION;
}

RTMPClient::~RTMPClient()
{
//    GNASH_REPORT_FUNCTION;
    _connected = false;

    _properties.clear();
//    delete _body;
}


// These are used for creating the primary objects

// Make the NetConnection object that is used to connect to the
// server.
boost::shared_ptr<Buffer> 
RTMPClient::encodeConnect(const char *app, const char *swfUrl, const char *tcUrl,
                          double audioCodecs, double videoCodecs, double videoFunction,
                          const char *pageUrl)
{
//    GNASH_REPORT_FUNCTION;
    
    AMF amf_obj;

    ElementSharedPtr connect(new amf::Element);
    connect->makeString("connect");

    ElementSharedPtr connum(new amf::Element);
    // update the counter for the number of connections. This number is used heavily
    // in RTMP to help keep communications clear when there are multiple streams.
    _connections++;
    connum->makeNumber(_connections);
    
    // Make the top level object
    ElementSharedPtr obj(new amf::Element);
    obj->makeObject();
    
    ElementSharedPtr appnode(new amf::Element);
    appnode->makeString("app", app);
    obj->addProperty(appnode);

    const char *version = 0;
    if (rcfile.getFlashVersionString().size() > 0) {
        version = rcfile.getFlashVersionString().c_str();
    } else {
        version = "LNX 9,0,31,0";
    }  

    ElementSharedPtr flashVer(new amf::Element);
    flashVer->makeString("flashVer", "LNX 9,0,31,0");
    obj->addProperty(flashVer);
    
    ElementSharedPtr swfUrlnode(new amf::Element);
//    swfUrl->makeString("swfUrl", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf");
    swfUrlnode->makeString("swfUrl", swfUrl);
    obj->addProperty(swfUrlnode);

//    filespec = "rtmp://localhost:5935/oflaDemo";
    ElementSharedPtr tcUrlnode(new amf::Element);
    tcUrlnode->makeString("tcUrl", tcUrl);
    obj->addProperty(tcUrlnode);

    ElementSharedPtr fpad(new amf::Element);
    fpad->makeBoolean("fpad", false);
    obj->addProperty(fpad);

    ElementSharedPtr audioCodecsnode(new Element);
//    audioCodecsnode->makeNumber("audioCodecs", 615);
    audioCodecsnode->makeNumber("audioCodecs", audioCodecs);
    obj->addProperty(audioCodecsnode);
    
    ElementSharedPtr videoCodecsnode(new Element);
//    videoCodecsnode->makeNumber("videoCodecs", 124);
    videoCodecsnode->makeNumber("videoCodecs", videoCodecs);
    obj->addProperty(videoCodecsnode);

    ElementSharedPtr videoFunctionnode(new Element);
//    videoFunctionnode->makeNumber("videoFunction", 0x1);
    videoFunctionnode->makeNumber("videoFunction", videoFunction);
    obj->addProperty(videoFunctionnode);

    ElementSharedPtr pageUrlnode(new Element);
//    pageUrlnode->makeString("pageUrl", "http://x86-ubuntu/software/gnash/tests/");
    pageUrlnode->makeString("pageUrl", pageUrl);
    obj->addProperty(pageUrlnode);

#if 0
    ElementSharedPtr objencodingnode(new Element);
    objencodingnode->makeNumber("objectEncoding", 0.0);
    obj->addProperty(objencodingnode);
#endif
//    size_t total_size = 227;
//     Buffer *out = encodeHeader(0x3, RTMP::HEADER_12, total_size,
//                                      RTMP::INVOKE, RTMP::FROM_CLIENT);
//     const char *rtmpStr = "03 00 00 04 00 01 1f 14 00 00 00 00";
//     Buffer *rtmpBuf = hex2mem(rtmpStr);
    boost::shared_ptr<Buffer> conobj = connect->encode();
    boost::shared_ptr<Buffer> numobj = connum->encode();
    boost::shared_ptr<Buffer> encobj = obj->encode();

    boost::shared_ptr<Buffer> buf(new Buffer(conobj->size() + numobj->size() + encobj->size()));
    *buf += conobj;
    *buf += numobj;
    *buf += encobj;
		   
    return buf;
}

boost::shared_ptr<amf::Buffer>
RTMPClient::encodeEchoRequest(const std::string &method, double id, amf::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<amf::Element> str(new amf::Element);
    str->makeString(method);
    boost::shared_ptr<Buffer> strobj = str->encode();

    // Encod ethe stream ID
    boost::shared_ptr<amf::Element>  num(new amf::Element);
    num->makeNumber(id);
    boost::shared_ptr<Buffer> numobj = num->encode();

    // Set the NULL object element that follows the stream ID
    boost::shared_ptr<amf::Element> null(new amf::Element);
    null->makeNull();
    boost::shared_ptr<Buffer> nullobj = null->encode();

    boost::shared_ptr<Buffer> elobj = el.encode();

    size_t totalsize = strobj->size() + numobj->size() + nullobj->size() + elobj->size();

    boost::shared_ptr<Buffer> buf(new Buffer(totalsize));
    
    *buf += strobj;
    *buf += numobj;
    *buf += nullobj;
    *buf += elobj;

    return buf;
}

// 43 00 1a 21 00 00 19 14 02 00 0c 63 72 65 61 74  C..!.......creat
// 65 53 74 72 65 61 6d 00 40 08 00 00 00 00 00 00  eStream.@.......
// 05                                                    .               
boost::shared_ptr<Buffer> 
RTMPClient::encodeStream(double id)
{
//    GNASH_REPORT_FUNCTION;
    
    struct timespec now;
    clock_gettime (CLOCK_REALTIME, &now);

    boost::shared_ptr<amf::Element> str(new amf::Element);
    str->makeString("createStream");
    boost::shared_ptr<Buffer> strobj = str->encode();
  
    boost::shared_ptr<amf::Element>  num(new amf::Element);
    num->makeNumber(id);
    boost::shared_ptr<Buffer> numobj = num->encode();

    // Set the NULL object element that follows the stream ID
    boost::shared_ptr<amf::Element> null(new amf::Element);
    null->makeNull();
    boost::shared_ptr<Buffer> nullobj = null->encode();    

    size_t totalsize = strobj->size() + numobj->size() + nullobj->size();

    boost::shared_ptr<Buffer> buf(new Buffer(totalsize));

    *buf += strobj;
    *buf += numobj;
    *buf += nullobj;

    return buf;
}

// 127.0.0.1:38167 -> 127.0.0.1:1935 [AP]
// 08 00 1b 1b 00 00 2a 14 01 00 00 00 02 00 04 70  ......*........p
// 6c 61 79 00 00 00 00 00 00 00 00 00 05 02 00 16  lay.............
// 6f 6e 32 5f 66 6c 61 73 68 38 5f 77 5f 61 75 64  on2_flash8_w_aud
// 69 6f 2e 66 6c 76 c2 00 03 00 00 00 01 00 00 27  io.flv.........'
// 10
boost::shared_ptr<Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, "", 0);
}    

boost::shared_ptr<Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag, double pos)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, "", pos);
}    

boost::shared_ptr<Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, name, 0);
}

// A seek packet is the operation name "seek", followed by the
// stream ID, then a NULL object, followed by the location to seek to.
//
// A pause packet is the operation name "pause", followed by the stream ID,
// then a NULL object, a boolean (always true from what I can tell), and then
// a location, which appears to always be 0.
boost::shared_ptr<Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name, double pos)
{
//    GNASH_REPORT_FUNCTION;

    // Set the operations command name
    Element str;
    switch (op) {
      case STREAM_PLAY:		// play the existing stream
	  str.makeString("play");
	  break;
      case STREAM_PAUSE:	// pause the existing stream
	  str.makeString("pause");
	  break;
      case STREAM_PUBLISH:	// publish the existing stream
	  str.makeString("publish");
	  break;
      case STREAM_STOP:		// stop the existing stream
	  str.makeString("stop");
	  break;
      case STREAM_SEEK:		// seek in the existing stream
	  str.makeString("seek");
	  break;
      default:
	  boost::shared_ptr<Buffer> foo;
	  return foo;
    };

    boost::shared_ptr<Buffer> strobj = str.encode();

    // Set the stream ID, which follows the command
    Element strid;
    strid.makeNumber(id);
    boost::shared_ptr<Buffer> stridobj = strid.encode();

    // Set the NULL object element that follows the stream ID
    Element null;
    null.makeNull();
    boost::shared_ptr<Buffer> nullobj = null.encode();    

    // Set the BOOLEAN object element that is the last field in the packet
    // (SEEK and PLAY don't use the boolean flag)
    boost::shared_ptr<Buffer> boolobj;
    if ((op != STREAM_SEEK) && (op != STREAM_PLAY)) {
        Element boolean;
        boolean.makeBoolean(flag);
        boolobj = boolean.encode();    
    }

    // The seek command also may have an optional location to seek to
    boost::shared_ptr<Buffer> posobj;
    if ((op == STREAM_PAUSE) || (op == STREAM_SEEK)) {
        Element seek;
        seek.makeNumber(pos);
        posobj = seek.encode();
    }

    // The play command has an optional field, which is the name of the file
    // used for the stream. A Play command without this name set play an
    // existing stream that is already open.
    boost::shared_ptr<Buffer> fileobj; 
    if (!name.empty()) {
        Element filespec;
        filespec.makeString(name);
        fileobj = filespec.encode();
    }

    // Calculate the packet size, rather than use the default as we want to
    // to be concious of the memory usage. The command name and the optional
    // file name are the only two dynamically sized fields.
    size_t pktsize = strobj->size() + stridobj->size() + nullobj->size();
    if ( boolobj ) pktsize += boolobj->size();
    if ( fileobj ) pktsize += fileobj->size();
    if ( posobj ) pktsize += posobj->size();

    boost::shared_ptr<Buffer> buf(new Buffer(pktsize));    
    *buf += strobj;
    *buf += stridobj;
    *buf += nullobj;
    if ( boolobj ) *buf += boolobj;
    if ( fileobj ) *buf += fileobj;
    if ( posobj ) *buf += posobj;

    return buf;
}

// A request for a handshake is initiated by sending a byte with a
// value of 0x3, followed by a message body of unknown format.
bool
RTMPClient::handShakeRequest()
{
    GNASH_REPORT_FUNCTION;

    // Make a buffer to hold the handshake data.
    _handshake = new Buffer(RTMP_HANDSHAKE_SIZE+1);
    if (!_handshake) {
	return false;
    }

    // All RTMP connections start with a 0x3
    *_handshake = RTMP_HANDSHAKE;

    // Since we don't know what the format is, create a pattern we can
    // recognize if we stumble across it later on.
    for (int i=0; i<RTMP_HANDSHAKE_SIZE; i++) {
	boost::uint8_t pad = i^256;
        *_handshake += pad;
    }
    
    int ret = writeNet(_handshake);
    if (ret) {
	return true;
    } else {
	return false;
    }
}

// The client finishes the handshake process by sending the second
// data block we get from the server as the response
bool
RTMPClient::clientFinish()
{
    GNASH_REPORT_FUNCTION;
    amf::Buffer data;
    return clientFinish(data);
}

bool
RTMPClient::clientFinish(amf::Buffer &data)
{
    GNASH_REPORT_FUNCTION;
    bool done = false;
    int ret = 0;
    int offset = 0;

    
    // The total size of incoming bytes is twice the handshake size, plus the handshake
    // header byte. Then we append the NetConnection::connect packet as well.
    size_t maxsize = (RTMP_HANDSHAKE_SIZE*2)+1+data.size();
    boost::shared_ptr<amf::Buffer> buf(new amf::Buffer(maxsize));
    do {
	ret = readNet(buf->reference() + offset, buf->size() - offset);
	offset += ret;
	buf->setSeekPointer(buf->reference() + offset);
	if (offset == (RTMP_HANDSHAKE_SIZE*2)+1) {
	    done = true;
	}
	if (ret < 0) {
	    log_error (_("Couldn't read data block in handshake!"));
	    done = true;
	}
    } while (!done);    
    
    if (buf->allocated() > RTMP_HANDSHAKE_SIZE) {
	log_debug (_("Read first data block in handshake"));
    } else {
	log_error (_("Couldn't read first data block in handshake"));
    }
    if (buf->allocated() > RTMP_HANDSHAKE_SIZE*2) {
	log_debug (_("Read second data block in handshake"));
    } else {
	log_error (_("Couldn't read second data block in handshake"));
	return false;
    }

#if 1
    if (memcmp(buf->reference() + RTMP_HANDSHAKE_SIZE + 1, _handshake->reference(), RTMP_HANDSHAKE_SIZE) == 0) {
	log_debug("Handshake matched");
    } else {
	log_debug("Handshake didn't match");
// 	return false;
    }
    *buf += data;
#endif
    
    // For some reason, Red5 won't connect unless the connect packet is
    // part of the final handshake packet. Sending the identical data with
    // two writeNet()s won't connect. Go figure...
    _handshake->resize(RTMP_HANDSHAKE_SIZE + data.size());
    // FIXME: unless the handshake is all zeros, Gnash won't connect to
    // Red5 for some reason. Cygnal isn't so picky.
    _handshake->clear();
    _handshake->setSeekPointer(_handshake->reference() + RTMP_HANDSHAKE_SIZE);
    // Add the NetConnection::connect() packet
    _handshake->append(data.reference(), data.allocated());
    ret = writeNet(_handshake->reference(), _handshake->allocated());
    if ( ret <= 0 ) {
	return false;
    }
    
    // Since the handshake completed sucessfully, we're connected.
    _connected == true;

    return true;
}

// Get and process an RTMP response. After reading all the data, then we have
// split it up on the chunksize boudaries, and into the respective queues
// for each channel.
RTMPClient::msgque_t
RTMPClient::recvResponse()
{
    GNASH_REPORT_FUNCTION;

    RTMPClient::msgque_t msgque;
    
    // Read the responses back from the server.  This is usually a series of system
    // messages on channel 2, and the response message on channel 3 from our request.
    boost::shared_ptr<amf::Buffer> response = recvMsg();
    if (!response) {
	log_error("Got no response from the RTMP server");
    }

    // when doing remoting calls I don't see this problem with an empty packet from Red5,
    // but when I do streaming, it's always there, so we need to remove it.
    boost::uint8_t *pktstart = response->reference();
    if (*pktstart == 0xff) {
	log_debug("Got empty packet in buffer.");
	pktstart++;
    }

    // The response packet contains multiple messages for multiple channels, so we
    // we have to split the Buffer into seperate messages on a chunksize boundary.
    boost::shared_ptr<RTMP::rtmp_head_t> rthead;
    boost::shared_ptr<RTMP::queues_t> que = split(pktstart, response->allocated()-1);

    // If we got no responses, something obviously went wrong.
    if (!que->size()) {
        log_error("No response from INVOKE of NetConnection connect");
	
    }

    // There is a queue of queues used to hold all the messages. The first queue
    // is indexed by the channel number, the second queue is all the messages that
    // have arrived for that channel.
    while (que->size()) {	// see if there are any messages at all
	log_debug("%s: There are %d channel queues in the RTMP input queue, %d messages in front queue",
		  __PRETTY_FUNCTION__, que->size(), que->front()->size());
	// Get the CQue for the first channel
	CQue *channel_q = que->front();
	que->pop_front();	// remove this Cque from the top level que

	while (channel_q->size()) {
	    // Get the first message in the channel queue
	    boost::shared_ptr<amf::Buffer> ptr = channel_q->pop();
  	    ptr->dump();
	    if (ptr) {		// If there is legit data
		rthead = decodeHeader(ptr->reference());
		if (!rthead) {
		    log_error("Couldn't decode RTMP message header");
		    continue;
		}
		switch (rthead->type) {
		  case RTMP::NONE:
		      log_error("RTMP packet can't be of none type!");
		      break;
		  case RTMP::CHUNK_SIZE:
		      log_unimpl("Server message data packet");
		      break;
		  case RTMP::UNKNOWN:	
		      log_unimpl("Unknown data packet");
		      break;
		  case RTMP::BYTES_READ:
		      log_unimpl("Bytes Read data packet");
		      break;
		  case RTMP::PING:
		  {
		      boost::shared_ptr<RTMP::rtmp_ping_t> ping = decodePing(ptr->reference() + rthead->head_size);
		      log_debug("Got a Ping type %s", ping_str[ping->type]);
		      break;
		  }
		  case RTMP::SERVER:
		      log_unimpl("Server message data packet");
		      break;
		  case RTMP::CLIENT:
		      log_unimpl("Client message data packet");
		      break;
		  case RTMP::UNKNOWN2:
		      log_unimpl("Unknown2 data packet");
		      break;
		  case RTMP::AUDIO_DATA:
		  {
		      boost::shared_ptr<RTMPMsg> msg = decodeMsgBody(ptr->reference() + rthead->head_size, rthead->bodysize);
		      if (msg) {
			  msgque.push_back(msg);
		      }
		      break;
		  }
		  case RTMP::VIDEO_DATA:
		  {
		      boost::shared_ptr<RTMPMsg> msg = decodeMsgBody(ptr->reference() + rthead->head_size, rthead->bodysize);
		      if (msg) {
			  msgque.push_back(msg);
		      }
		      break;
		  }
		  case RTMP::UNKNOWN3:
		      log_unimpl("Unknown3 data packet message");
		      break;
		  case RTMP::NOTIFY:
		      log_unimpl("Notify data packet message");
		      break;
		  case RTMP::SHARED_OBJ:
		      log_unimpl("Shared Object data packet message");
		      break;
		  case RTMP::INVOKE:
		  {
		      boost::shared_ptr<RTMPMsg> msg = decodeMsgBody(ptr->reference() + rthead->head_size, rthead->bodysize);
		      if (msg) {
			  msgque.push_back(msg);
		      }
		      break;
		  }
		  case RTMP::FLV_DATA:
		      log_unimpl("Flv data packet message");
		      break;
		  default :
		      log_error("Couldn't decode RTMP message Body");
		      break;
		}
	    }
	}
    }
    
    return msgque;
}


// bool
// RTMPClient::packetRequest()
// {
//     GNASH_REPORT_FUNCTION;
//     return false;
// }

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
