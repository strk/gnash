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

#include <ctime>
#include <iostream>
#include <string>
#include <map>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
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
#include "URL.h"

typedef boost::shared_ptr<cygnal::Element> ElementSharedPtr;

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
boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeConnect()
{
//     GNASH_REPORT_FUNCTION;
    
    return encodeConnect(_path.c_str());
}

boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeConnect(const char *uri)
{
//     GNASH_REPORT_FUNCTION;
    
    return encodeConnect(uri, RTMPClient::DEFAULT_AUDIO_SET,
			 RTMPClient::DEFAULT_VIDEO_SET,
			 RTMPClient::SEEK);
}

boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeConnect(const char *uri,
			  double audioCodecs, double videoCodecs,
			  double videoFunction)
{
    GNASH_REPORT_FUNCTION;
    using std::string;
    
    URL url(uri);
    string portstr;

    short port = 0;
    string protocol;		// the network protocol, rtmp or http
    string query;		// any queries for the host
    string app;			// the application name
    string path;		// the path to the file on the server
    string tcUrl;		// the tcUrl field
    string swfUrl;		// the swfUrl field
    string filename;		// the filename to play
    string pageUrl;		// the pageUrl field
    string hostname;		// the hostname of the server


    protocol = url.protocol();
    hostname = url.hostname();
    portstr = url.port();
    query = url.querystring();

    if (portstr.empty()) {
        if ((protocol == "http") || (protocol == "rtmpt")) {
            port = RTMPT_PORT;
        }
        if (protocol == "rtmp") {
            port = RTMP_PORT;
        }
    } else {
        port = strtol(portstr.c_str(), NULL, 0) & 0xffff;
    }


    path = url.path();

    string::size_type end = path.rfind('/');
    if (end != string::npos) {
	filename = path.substr(end + 1);
    }  
    
    tcUrl = uri;
    app = filename;    
    swfUrl = "http://localhost:1935/demos/videoConference.swf";
    pageUrl = "http://gnashdev.org";
    
    log_network(_("URL is %s"), url);
    log_network(_("Protocol is %s"), protocol);
    log_network(_("Host is %s"), hostname);
    log_network(_("Port is %s"), port);
    log_network(_("Path is %s"), path);
    log_network(_("Filename is %s"), filename);
    log_network(_("App is %s"), app);
    log_network(_("Query is %s"), query);
    log_network(_("tcUrl is %s"), tcUrl);
    log_network(_("swfUrl is %s"), swfUrl);
    log_network(_("pageUrl is %s"), pageUrl);

    return encodeConnect(app.c_str(), swfUrl.c_str(), tcUrl.c_str(),
			 audioCodecs, videoCodecs, videoFunction,
			 pageUrl.c_str());
}

boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeConnect(const char *app, const char *swfUrl, const char *tcUrl,
                          double audioCodecs, double videoCodecs, double videoFunction,
                          const char *pageUrl)
{
    GNASH_REPORT_FUNCTION;
    
    cygnal::AMF amf_obj;

    ElementSharedPtr connect(new cygnal::Element);
    connect->makeString("connect");

    ElementSharedPtr connum(new cygnal::Element);
    // update the counter for the number of connections. This number is used heavily
    // in RTMP to help keep communications clear when there are multiple streams.
    _connections++;
    connum->makeNumber(_connections);
    
    // Make the top level object
    ElementSharedPtr obj(new cygnal::Element);
    obj->makeObject();
    
    ElementSharedPtr appnode(new cygnal::Element);
    appnode->makeString("app", app);
    obj->addProperty(appnode);

    const char *version = 0;
    if (rcfile.getFlashVersionString().size() > 0) {
        version = rcfile.getFlashVersionString().c_str();
    } else {
        version = "LNX 9,0,31,0";
    }  

    ElementSharedPtr flashVer(new cygnal::Element);
    flashVer->makeString("flashVer", version);
    obj->addProperty(flashVer);
    
    ElementSharedPtr swfUrlnode(new cygnal::Element);
//    swfUrl->makeString("swfUrl", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf");
    swfUrlnode->makeString("swfUrl", swfUrl);
    obj->addProperty(swfUrlnode);

//    filespec = "rtmp://localhost:5935/oflaDemo";
    ElementSharedPtr tcUrlnode(new cygnal::Element);
    tcUrlnode->makeString("tcUrl", tcUrl);
    obj->addProperty(tcUrlnode);

    ElementSharedPtr fpad(new cygnal::Element);
    fpad->makeBoolean("fpad", false);
    obj->addProperty(fpad);

    ElementSharedPtr audioCodecsnode(new cygnal::Element);
//    audioCodecsnode->makeNumber("audioCodecs", 615);
    audioCodecsnode->makeNumber("audioCodecs", audioCodecs);
    obj->addProperty(audioCodecsnode);
    
    ElementSharedPtr videoCodecsnode(new cygnal::Element);
//    videoCodecsnode->makeNumber("videoCodecs", 124);
    videoCodecsnode->makeNumber("videoCodecs", videoCodecs);
    obj->addProperty(videoCodecsnode);

    ElementSharedPtr videoFunctionnode(new cygnal::Element);
//    videoFunctionnode->makeNumber("videoFunction", 0x1);
    videoFunctionnode->makeNumber("videoFunction", videoFunction);
    obj->addProperty(videoFunctionnode);

    ElementSharedPtr pageUrlnode(new cygnal::Element);
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
    boost::shared_ptr<cygnal::Buffer> conobj = connect->encode();
    boost::shared_ptr<cygnal::Buffer> numobj = connum->encode();
    boost::shared_ptr<cygnal::Buffer> encobj = obj->encode();

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(conobj->size() + numobj->size() + encobj->size()));
    *buf += conobj;
    *buf += numobj;
    *buf += encobj;
		   
    return buf;
}
    
bool
RTMPClient::connectToServer(const std::string &url)
{
    GNASH_REPORT_FUNCTION;

    URL uri(url);

    // If we're currently not connected, build and send the
    // initial handshake packet.
    if (connected() == false) {
	short port = strtol(uri.port().c_str(), NULL, 0) & 0xffff;
	if (!createClient(uri.hostname(), port)) {
	    return false;
	}

	// Build the NetConnection Packet, which seems to need
	// to be on the end of the second block of handshake data.
	// We build this here so we can get the total encoded
	// size of the object.
	boost::shared_ptr<cygnal::Buffer> ncbuf = encodeConnect();

	// As at this point we don't have an RTMP connection,
	// we can't use the regular sendMsg(), that handles the RTMP
	// headers for continuation packets. We know a this point it's
	// always one by, so we just add it by hand. It doesn't matter
	// as long as the channel number matches the one used to
	// create the initial RTMP packet header.
	boost::scoped_ptr<cygnal::Buffer> newbuf(new cygnal::Buffer(ncbuf->size() + 5));
	size_t nbytes = 0;
	size_t chunk = RTMP_VIDEO_PACKET_SIZE;
	do {
	    // The last packet is smaller
	    if ((ncbuf->allocated() - nbytes) < static_cast<size_t>(RTMP_VIDEO_PACKET_SIZE)) {
		chunk = ncbuf->allocated() - nbytes;
	    }
	    newbuf->append(ncbuf->reference() + nbytes, chunk);
 	    nbytes  += chunk;
	    if (chunk == static_cast<size_t>(RTMP_VIDEO_PACKET_SIZE)) {
		boost::uint8_t headone = 0xc3;
		*newbuf += headone;
	    }
	} while (nbytes < ncbuf->allocated());

	boost::shared_ptr<cygnal::Buffer> head = encodeHeader(0x3,
			    RTMP::HEADER_12, ncbuf->allocated(),
			    RTMP::INVOKE, RTMPMsg::FROM_CLIENT);

	// Build the first handshake packet, and send it to the
	// server.
	boost::shared_ptr<cygnal::Buffer> handshake1 = handShakeRequest();
	if (!handshake1) {
	    log_error(_("RTMP handshake request failed"));
	    return false;
	}
	
	boost::scoped_ptr<cygnal::Buffer> handshake2(new cygnal::Buffer
		  ((RTMP_HANDSHAKE_SIZE * 2) + newbuf->allocated()
		   + RTMP_MAX_HEADER_SIZE));

	// Finish the handshake process, which has to have the
	// NetConnection::connect() as part of the buffer, or Red5
	// refuses to answer.
	setTimeout(20);
#if 0
	*handshake2 = handshake1;
	*handshake2 += head;
 	*handshake2 += ncbuf;
	if (!clientFinish(*handshake2)) {
#else
	*handshake2 = head;
	handshake2->append(newbuf->reference(), newbuf->allocated());
	handshake2->dump();
        if (!clientFinish(*handshake2)) {
#endif
	    log_error(_("RTMP handshake completion failed!"));
//	    return (false);
	}
	
	// give the server time to process our NetConnection::connect() request	
	boost::shared_ptr<cygnal::Buffer> response;
	boost::shared_ptr<RTMP::rtmp_head_t> rthead;
	boost::shared_ptr<RTMP::queues_t> que;
	
	RTMPClient::msgque_t msgque = recvResponse();
	while (msgque.size()) {
	    boost::shared_ptr<RTMPMsg> msg = msgque.front();
	    msgque.pop_front();
	    if (msg->getStatus() ==  RTMPMsg::NC_CONNECT_SUCCESS) {
		log_network(_("Sent NetConnection Connect message successfully"));
	    }		    
	    if (msg->getStatus() ==  RTMPMsg::NC_CONNECT_FAILED) {
		log_error(_("Couldn't send NetConnection Connect message,"));
	    }
	}
    }

    return true;
}
    
boost::shared_ptr<cygnal::Buffer>
RTMPClient::encodeEchoRequest(const std::string &method, double id, cygnal::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Element> str(new cygnal::Element);
    str->makeString(method);
    boost::shared_ptr<cygnal::Buffer> strobj = str->encode();

    // Encod ethe stream ID
    boost::shared_ptr<cygnal::Element>  num(new cygnal::Element);
    num->makeNumber(id);
    boost::shared_ptr<cygnal::Buffer> numobj = num->encode();

    // Set the NULL object element that follows the stream ID
    boost::shared_ptr<cygnal::Element> null(new cygnal::Element);
    null->makeNull();
    boost::shared_ptr<cygnal::Buffer> nullobj = null->encode();

    boost::shared_ptr<cygnal::Buffer> elobj = el.encode();

    size_t totalsize = strobj->size() + numobj->size() + nullobj->size() + elobj->size();

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(totalsize));
    
    *buf += strobj;
    *buf += numobj;
    *buf += nullobj;
    *buf += elobj;

    return buf;
}

// 43 00 1a 21 00 00 19 14 02 00 0c 63 72 65 61 74  C..!.......creat
// 65 53 74 72 65 61 6d 00 40 08 00 00 00 00 00 00  eStream.@.......
// 05                                                    .               
boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeStream(double id)
{
//    GNASH_REPORT_FUNCTION;
    
    struct timespec now;
    clock_gettime (CLOCK_REALTIME, &now);

    boost::shared_ptr<cygnal::Element> str(new cygnal::Element);
    str->makeString("createStream");
    boost::shared_ptr<cygnal::Buffer> strobj = str->encode();
  
    boost::shared_ptr<cygnal::Element>  num(new cygnal::Element);
    num->makeNumber(id);
    boost::shared_ptr<cygnal::Buffer> numobj = num->encode();

    // Set the NULL object element that follows the stream ID
    boost::shared_ptr<cygnal::Element> null(new cygnal::Element);
    null->makeNull();
    boost::shared_ptr<cygnal::Buffer> nullobj = null->encode();    

    size_t totalsize = strobj->size() + numobj->size() + nullobj->size();

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(totalsize));

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
boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, "", 0);
}    

boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag, double pos)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, "", pos);
}    

boost::shared_ptr<cygnal::Buffer> 
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
boost::shared_ptr<cygnal::Buffer> 
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name, double pos)
{
//    GNASH_REPORT_FUNCTION;

    // Set the operations command name
    cygnal::Element str;
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
	  boost::shared_ptr<cygnal::Buffer> foo;
	  return foo;
    };

    boost::shared_ptr<cygnal::Buffer> strobj = str.encode();

    // Set the stream ID, which follows the command
    cygnal::Element strid;
    strid.makeNumber(id);
    boost::shared_ptr<cygnal::Buffer> stridobj = strid.encode();

    // Set the NULL object element that follows the stream ID
    cygnal::Element null;
    null.makeNull();
    boost::shared_ptr<cygnal::Buffer> nullobj = null.encode();    

    // Set the BOOLEAN object element that is the last field in the packet
    // (SEEK and PLAY don't use the boolean flag)
    boost::shared_ptr<cygnal::Buffer> boolobj;
    if ((op != STREAM_SEEK) && (op != STREAM_PLAY)) {
        cygnal::Element boolean;
        boolean.makeBoolean(flag);
        boolobj = boolean.encode();    
    }

    // The seek command also may have an optional location to seek to
    boost::shared_ptr<cygnal::Buffer> posobj;
    if ((op == STREAM_PAUSE) || (op == STREAM_SEEK)) {
        cygnal::Element seek;
        seek.makeNumber(pos);
        posobj = seek.encode();
    }

    // The play command has an optional field, which is the name of the file
    // used for the stream. A Play command without this name set play an
    // existing stream that is already open.
    boost::shared_ptr<cygnal::Buffer> fileobj; 
    if (!name.empty()) {
        cygnal::Element filespec;
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

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(pktsize));    
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
boost::shared_ptr<cygnal::Buffer>
RTMPClient::handShakeRequest()
{
    GNASH_REPORT_FUNCTION;
    boost::uint32_t zero = 0;

    // Make a buffer to hold the handshake data.
    boost::shared_ptr<cygnal::Buffer> handshake(new cygnal::Buffer(RTMP_HANDSHAKE_SIZE+1));
    if (!handshake) {
	return handshake;
    }

    // All RTMP connections start with the RTMP version number
    // which should always be 0x3.
    *handshake = RTMP_VERSION;

    *handshake += RTMP::getTime();

    // This next field in the header for the RTMP handshake should be zeros
    *handshake += zero;

    // The handshake contains random data after the initial header
    for (int i=0; i<RTMP_RANDOM_SIZE; i++) {
	boost::uint8_t pad = i^256;
        *handshake += pad;
    }
    
    int ret = writeNet(*handshake);
    if (ret <= 0) {
	handshake.reset();
    }
					     
    return handshake;
}

// The client finishes the handshake process by sending the second
// data block we get from the server as the response

boost::shared_ptr<cygnal::Buffer>
RTMPClient::clientFinish()
{
//     GNASH_REPORT_FUNCTION;

    cygnal::Buffer data;
    return clientFinish(data);
}

boost::shared_ptr<cygnal::Buffer>
RTMPClient::clientFinish(cygnal::Buffer &data)
{
    GNASH_REPORT_FUNCTION;
    bool done = false;
    int ret = 0;
    int retries = 5;
    int offset = 0;
    
    // Create the initial buffer to hold the response, and keep reading data
    // until it is populated
    // The handhake for this phase is twice the size of the initial handshake
    // we sent previously, plus one byte for the RTMP version header.
    int max_size = (RTMP_HANDSHAKE_SIZE*2) + 1;
    boost::shared_ptr<cygnal::Buffer> handshake1(new cygnal::Buffer(
			      max_size + data.allocated()));
    do {
	ret = readNet(handshake1->end(), max_size - offset);
	offset += ret;
	handshake1->setSeekPointer(handshake1->reference() + offset);
	if ((offset >= max_size) || (ret >= max_size)) {
	    handshake1->setSeekPointer(handshake1->reference() + max_size);
// 	    log_network("Read entire packet of %d bytes", ret);
	    done = true;
	}
	if (ret < 0) {
	    log_error(_("Couldn't read data block in handshake!"));
	    handshake1.reset();
	    return handshake1;
	}
	// if retries equals zero, then we're done trying
	if (retries == 0) {
	    done = true;
	} else {
	    --retries;
	}
    } while (!done);

    if (handshake1->allocated() == boost::lexical_cast<size_t>(max_size)) {
	log_network(_("Read data block in handshake, got %d bytes."),
		   handshake1->allocated());
    } else {
	log_error(_("Couldn't read data block in handshake, read %d bytes!"),
		  handshake1->allocated());
    }    

    _handshake_header.uptime = ntohl(*reinterpret_cast<boost::uint32_t *>
				     (handshake1->reference() + 1));

    log_network(_("RTMP Handshake header: Uptime: %u"),
		_handshake_header.uptime);

#if 0
    if (memcmp(handshake2->reference() + RTMP_HANDSHAKE_SIZE + 8,
	       _handshake->reference() + 8, RTMP_RANDOM_SIZE-8) == 0) {
	log_network("Handshake matched");
    } else {
	log_network("Handshake didn't match");
// 	return false;
    }
#endif

    // Make a new buffer big enough to hold the handshake, data, and header byte
    boost::shared_ptr<cygnal::Buffer> handshake2(new cygnal::Buffer(
			     RTMP_HANDSHAKE_SIZE + data.allocated()));
    
    // Copy the timestamp from the message we just received.
    handshake2->copy(handshake1->reference()+1, sizeof(boost::uint32_t));

#if 1
    // The next timestamp is the one we just received bumped up a tiny bit.
    // I have no clue if this is correct, but fom hex dumps the previous
    // timestamp should be the baseline, and this is just that time plus
    // whatever it took to get the message. The 7 is a bit randomly chosen.
    boost::uint32_t tt = htonl(_handshake_header.uptime + 7);
#else
    // Get the uptime for the header
    // yes, we loose precision here but it's only a 4 byte field
    time_t t;
    time(&t);
    boost::uint32_t tt = t;
#endif
    *handshake2 += tt;

    // Add the handshake data
    boost::uint8_t *start = handshake1->reference() + RTMP_HANDSHAKE_SIZE
	+ 1 + 8;
    handshake2->append(start, RTMP_RANDOM_SIZE);
    // Add the NetConnection::connect() packet
    *handshake2 += data;

    // Write the second chunk to the server
    log_network(_("About to write %d bytes, data is: %d bytes."),
	      handshake2->allocated(),
	      data.allocated());
    log_network(_("Client response header for handshake 2: %s"),
		hexify(handshake2->reference(), 12, false));
    log_network(_("Data in response for handshake 2: %s"),
		hexify(handshake1->reference() + RTMP_HANDSHAKE_SIZE + 1, 12, false));
#if 0
    ret = writeNet(handshake2->reference()+RTMP_HANDSHAKE_SIZE,
		   RTMP_HANDSHAKE_SIZE + data.allocated() + 1);
#else
    ret = writeNet(*handshake2);
#endif
    if ( ret <= 0 ) {
	log_error(_("Couldn't write the second handshake packet!"));
	handshake1.reset();
	return handshake1;
    } else {
	_connected = false;
    }

    // Since the handshake completed successfully, we're connected.
    _connected = true;

    return handshake1;
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
    boost::shared_ptr<cygnal::Buffer> response = recvMsg();
    if (!response) {
	log_error(_("Got no response from the RTMP server"));
	return msgque;
    }

    // when doing remoting calls I don't see this problem with an empty packet from Red5,
    // but when I do streaming, it's always there, so we need to remove it.
    boost::uint8_t *pktstart = response->reference();
    if (*pktstart == 0xff) {
	log_network(_("Got empty packet in buffer."));
	pktstart++;
    }

    // The response packet contains multiple messages for multiple channels, so we
    // we have to split the Buffer into seperate messages on a chunksize boundary.
    boost::shared_ptr<RTMP::rtmp_head_t> rthead;
    boost::shared_ptr<RTMP::queues_t> que = split(pktstart, response->allocated()-1);

    // If we got no responses, something obviously went wrong.
    if (!que->size()) {
        log_error(_("No response from INVOKE of NetConnection connect"));
    }

    // There is a queue of queues used to hold all the messages. The first queue
    // is indexed by the channel number, the second queue is all the messages that
    // have arrived for that channel.
    while (que->size()) {	// see if there are any messages at all
	log_network(_("%s: There are %d channel queues in the RTMP input queue, %d messages in front queue"),
		  __PRETTY_FUNCTION__, que->size(), que->front()->size());
	// Get the CQue for the first channel
	CQue *channel_q = que->front();
	que->pop_front();	// remove this Cque from the top level que

	while (channel_q->size()) {
	    // Get the first message in the channel queue
	    boost::shared_ptr<cygnal::Buffer> ptr = channel_q->pop();
  	    ptr->dump();
	    if (ptr) {		// If there is legit data
		rthead = decodeHeader(ptr->reference());
		if (!rthead) {
		    log_error(_("Couldn't decode RTMP message header"));
		    continue;
		}
		switch (rthead->type) {
		  case RTMP::NONE:
		      log_error(_("RTMP packet can't be of none type!"));
                      
		      break;
		  case RTMP::CHUNK_SIZE:
		      log_unimpl(_("Server message data packet"));
                      
		      break;
		  case RTMP::ABORT:
		      log_unimpl(_("Abort packet"));
		      break;
		  case RTMP::BYTES_READ:
		      log_unimpl(_("Bytes Read data packet"));
		      break;
		  case RTMP::USER:
		  {
		      boost::shared_ptr<RTMP::rtmp_ping_t> ping = decodePing(ptr->reference() + rthead->head_size);
		      log_network(_("Got a Ping type %s"), ping_str[ping->type]);
		      break;
		  }
		  case RTMP::WINDOW_SIZE:
		      log_unimpl(_("Set Window Size message data packet"));
		      break;
		  case RTMP::SET_BANDWITH:
		      log_unimpl(_("Set Bandwidthmessage data packet"));
		      break;
		  case RTMP::ROUTE:
		      log_unimpl(_("Route from other server packet"));
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
		  case RTMP::SHARED_OBJ:
		      log_unimpl(_("AMF0 Shared Object data packet message"));
		      break;
		  case RTMP::AMF3_NOTIFY:
		      log_unimpl(_("AMF3 Notify data packet message"));
		      break;
		  case RTMP::AMF3_SHARED_OBJ:
		      log_unimpl(_("AMF3 Shared Object data packet message"));
		      break;
		  case RTMP::AMF3_INVOKE:
		      log_unimpl(_("AMF0 Invoke packet message"));
		      break;
		  case RTMP::NOTIFY:
		      log_unimpl(_("AMF0 Notify data packet message"));
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
		      log_unimpl(_("Flv data packet message"));
		      break;
		  default :
		      log_error(_("Couldn't decode RTMP message Body"));
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
// indent-tabs-mode: nil
// End:
