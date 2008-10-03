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

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include "log.h"
#include "rc.h"
#include "amf.h"
#include "rtmp.h"
#include "rtmp_client.h"
#include "network.h"
#include "element.h"
#include "handler.h"
#include "utility.h"
#include "buffer.h"
#include "GnashSleep.h"

using namespace gnash;
using namespace std;
using namespace amf;

namespace gnash
{

// The rcfile is loaded and parsed here:
static RcInitFile& rcfile = RcInitFile::getDefaultInstance();

extern map<int, Handler *> handlers;

RTMPClient::RTMPClient()
    : _connections(0)
{
//    GNASH_REPORT_FUNCTION;
}

RTMPClient::~RTMPClient()
{
//    GNASH_REPORT_FUNCTION;
    _properties.clear();
//    delete _body;
}


// These are used for creating the primary objects

// Make the NetConnection object that is used to connect to the
// server.
amf::Buffer *
RTMPClient::encodeConnect(const char *app, const char *swfUrl, const char *tcUrl,
                          double audioCodecs, double videoCodecs, double videoFunction,
                          const char *pageUrl)
{
//    GNASH_REPORT_FUNCTION;
    
    AMF amf_obj;

    Element connect;
    connect.makeString("connect");

    Element connum;
    // update the counter for the number of connections. This number is used heavily
    // in RTMP to help keep communications clear when there are multiple streams.
    _connections++;
    connum.makeNumber(_connections);
    
    // Make the top level object
    Element obj;
    obj.makeObject();
    
    Element *appnode = new Element;
    appnode->makeString("app", app);
    obj.addProperty(appnode);

    const char *version = 0;
    if (rcfile.getFlashVersionString().size() > 0) {
        version = rcfile.getFlashVersionString().c_str();
    } else {
        version = "LNX 9,0,31,0";
    }  

    Element *flashVer = new Element;
    flashVer->makeString("flashVer", "LNX 9,0,31,0");
    obj.addProperty(flashVer);
    
    Element *swfUrlnode = new Element;
//    swfUrl->makeString("swfUrl", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf");
    swfUrlnode->makeString("swfUrl", swfUrl);
    obj.addProperty(swfUrlnode);

//    filespec = "rtmp://localhost/oflaDemo";
    Element *tcUrlnode = new Element;
    tcUrlnode->makeString("tcUrl", tcUrl);
    obj.addProperty(tcUrlnode);

    Element *fpad = new Element;
    fpad->makeBoolean("fpad", false);
    obj.addProperty(fpad);

    Element *audioCodecsnode = new Element;
//    audioCodecsnode->makeNumber("audioCodecs", 615);
    audioCodecsnode->makeNumber("audioCodecs", audioCodecs);
    obj.addProperty(audioCodecsnode);
    
    Element *videoCodecsnode = new Element;
//    videoCodecsnode->makeNumber("videoCodecs", 124);
    videoCodecsnode->makeNumber("videoCodecs", videoCodecs);
    obj.addProperty(videoCodecsnode);

    Element *videoFunctionnode = new Element;
//    videoFunctionnode->makeNumber("videoFunction", 0x1);
    videoFunctionnode->makeNumber("videoFunction", videoFunction);
    obj.addProperty(videoFunctionnode);

    Element *pageUrlnode = new Element;
//    pageUrlnode->makeString("pageUrl", "http://x86-ubuntu/software/gnash/tests/");
    pageUrlnode->makeString("pageUrl", pageUrl);
    obj.addProperty(pageUrlnode);

    Element *objencodingnode = new Element;
    objencodingnode->makeNumber("objectEncoding", 0.0);
    obj.addProperty(objencodingnode);
    
//    size_t total_size = 227;
//     Buffer *out = encodeHeader(0x3, RTMP::HEADER_12, total_size,
//                                      RTMP::INVOKE, RTMP::FROM_CLIENT);
//     const char *rtmpStr = "03 00 00 04 00 01 1f 14 00 00 00 00";
//     Buffer *rtmpBuf = hex2mem(rtmpStr);
    Buffer *conobj = connect.encode();
    Buffer *numobj = connum.encode();
    Buffer *encobj = obj.encode();

    Buffer *buf = new Buffer(conobj->size() + numobj->size() + encobj->size());
//    buf->append(out);
    buf->append(conobj);
    buf->append(numobj);
    buf->append(encobj);

    delete conobj;
    delete numobj;
    delete encobj;
    
    return buf;
}

// 43 00 1a 21 00 00 19 14 02 00 0c 63 72 65 61 74  C..!.......creat
// 65 53 74 72 65 61 6d 00 40 08 00 00 00 00 00 00  eStream.@.......
// 05                                                    .               
amf::Buffer *
RTMPClient::encodeStream(double id)
{
//    GNASH_REPORT_FUNCTION;
    
    struct timespec now;
    clock_gettime (CLOCK_REALTIME, &now);

    Element str = new Element;
    str.makeString("createStream");
    Buffer *strobj = str.encode();
    if (!strobj) {
	return 0;
    }
  
    Element num = new Element;
    num.makeNumber(id);
    Buffer *numobj = num.encode();
    if (!numobj) {
	return 0;
    }

    Buffer *buf = new Buffer(strobj->size() + numobj->size());
    if (!buf) {
	return 0;
    }

    // Set the NULL object element that follows the stream ID
    Element null;
    null.makeNull();
    Buffer *nullobj = null.encode();    
    if (!nullobj) {
	return 0;
    }

    buf->append(strobj);
    buf->append(numobj);
    buf->append(nullobj);

    delete strobj;
    delete numobj;
    delete nullobj;
    
    return buf;
}

// 127.0.0.1:38167 -> 127.0.0.1:1935 [AP]
// 08 00 1b 1b 00 00 2a 14 01 00 00 00 02 00 04 70  ......*........p
// 6c 61 79 00 00 00 00 00 00 00 00 00 05 02 00 16  lay.............
// 6f 6e 32 5f 66 6c 61 73 68 38 5f 77 5f 61 75 64  on2_flash8_w_aud
// 69 6f 2e 66 6c 76 c2 00 03 00 00 00 01 00 00 27  io.flv.........'
// 10
amf::Buffer *
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, "", 0);
}    

amf::Buffer *
RTMPClient::encodeStreamOp(double id, rtmp_op_e op, bool flag, double pos)
{
//    GNASH_REPORT_FUNCTION;
    return encodeStreamOp(id, op, flag, "", pos);
}    

amf::Buffer *
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
amf::Buffer *
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
	  return 0;
    };

    Buffer *strobj = str.encode();
    if (!strobj) {
	return 0;
    }

    // Set the stream ID, which follows the command
    Element strid;
    strid.makeNumber(id);
    Buffer *stridobj = strid.encode();
    if (!stridobj) {
	return 0;
    }

    // Set the NULL object element that follows the stream ID
    Element null;
    null.makeNull();
    Buffer *nullobj = null.encode();    
    if (!nullobj) {
	return 0;
    }

    // Set the BOOLEAN object element that is the last field in the packet
    Element boolean;
    boolean.makeBoolean(flag);
    Buffer *boolobj = boolean.encode();    
    if (!boolobj) {
	return 0;
    }
    
    // Calculate the packet size, rather than use the default as we want to
    // to be concious of the memory usage. The command name and the optional
    // file name are the only two dynamically sized fields.
    size_t pktsize = strobj->size() + name.size();
    // Add 2 bytes for the Boolean, and 16 bytes for the two doubles, which are
    // 8 bytes apiece.
    pktsize += (sizeof(double) * 2) + 2;
    Buffer *buf = new Buffer(pktsize);
    buf->clear();
//    Buffer *buf = new Buffer;
    
    if (!buf) {
	return 0;
    }
    buf->append(strobj);
    delete strobj;
    buf->append(stridobj);
    delete stridobj;    
    buf->append(nullobj);
    delete nullobj;
    // Seek doesn't use the boolean flag
    if ((op != STREAM_SEEK) && (op != STREAM_PLAY)) {
	buf->append(boolobj);
    }
    delete boolobj;

    // The play command has an optional field, which is the name of the file
    // used for the stream. A Play command without this name set play an
    // existing stream that is already open.
    if (!name.empty()) {
	Element filespec;
	filespec.makeString(name);
	Buffer *fileobj = filespec.encode();
	buf->append(fileobj);
	delete fileobj;
    }
    
    // The seek command also may have an optional location to seek to
    if ((op == STREAM_PAUSE) || (op == STREAM_SEEK)) {
	Element seek;
	seek.makeNumber(pos);
	Buffer *posobj = seek.encode();
	if (!posobj) {
	    return 0;
	}
	buf->append(posobj);
	delete posobj;
    }

    return buf;
}

// A request for a handshake is initiated by sending a byte with a
// value of 0x3, followed by a message body of unknown format.
bool
RTMPClient::handShakeRequest()
{
    GNASH_REPORT_FUNCTION;

    // Make a buffer to hold the handshake data.
    _handshake = new Buffer(RTMP_BODY_SIZE+1);
    if (!_handshake) {
	return false;
    }

    // All RTMP connections start with a 0x3
    _handshake->copy(RTMP_HANDSHAKE);

    // Since we don't know what the format is, create a pattern we can
    // recognize if we stumble across it later on.
    for (int i=0; i<RTMP_BODY_SIZE; i++) {
	Network::byte_t pad = i^256;
        _handshake->append(pad);
    }
    
    int ret = writeNet(_handshake);
    if (ret) {
	return true;
    } else {
	return false;
    }
}

// The client finished the handshake process by sending the second
// data block we get from the server as the response
bool
RTMPClient::clientFinish()
{
    GNASH_REPORT_FUNCTION;

    int ret = 0;
    _handshake->clear();
    
    gnashSleep(1000000); // FIXME: why do we still need a delay here, when readNet() does a select ?
    ret = readNet(_handshake->reference(), RTMP_BODY_SIZE);
    if (ret == RTMP_BODY_SIZE) {
        log_debug (_("Read first data block in handshake"));
    } else {
        log_error (_("Couldn't read first data block in handshake"));
//        return false;
    }
    if (ret > RTMP_BODY_SIZE) {
	ret = readNet(_handshake->reference(), RTMP_BODY_SIZE);
	if (ret == RTMP_BODY_SIZE) {        
	    log_debug (_("Read second data block in handshake"));
	} else {
	    log_error (_("Couldn't read second data block in handshake"));
//        return false;
	}
    }
    ret = readNet(_handshake->reference(), RTMP_BODY_SIZE);
    if (ret == RTMP_BODY_SIZE) {        
        log_debug (_("Read second data block in handshake"));
    } else {
        log_error (_("Couldn't read second data block in handshake"));
//        return false;
    }
    if (ret > RTMP_BODY_SIZE) {
	ret = readNet(_handshake->reference(), RTMP_BODY_SIZE);
	if (ret == RTMP_BODY_SIZE) {        
	    log_debug (_("Read second data block in handshake"));
	} else {
	    log_error (_("Couldn't read second data block in handshake"));
//        return false;
	}
    }

    writeNet(_handshake->reference(), RTMP_BODY_SIZE);

    return true;
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
