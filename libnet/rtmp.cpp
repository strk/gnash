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
#include "amf.h"
#include "rtmp.h"
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

const char *content_str[] = {
    "None",
    "Chunk Size",
    "Unknown",
    "Bytes Read",
    "Ping",
    "Server",
    "Client",
    "Unknown2",
    "Audio Data",
    "Video Data",
    "Unknown3",
    "Blank 0xb",
    "Blank 0xc",
    "Blank 0xd",
    "Blank 0xe",
    "Blank 0xf",
    "Blank 0x10",
    "Blank 0x11",
    "Notify",
    "Shared object",
    "Invoke"
};

// These are the textual responses
const char *response_str[] = {
    "/onStatus",
    "/onResult",
    "/onDebugEvents"
};

int
RTMP::headerSize(Network::byte_t header)
{
//    GNASH_REPORT_FUNCTION;
    
    int headersize = -1;
    
    switch (header & RTMP_HEADSIZE_MASK) {
      case HEADER_12:
          headersize = 12;
          break;
      case HEADER_8:
          headersize = 8;
          break;
      case HEADER_4:
          headersize = 4;
          break;
      case HEADER_1:
          headersize = 11;
          break;
      default:
          log_error(_("AMF Header size bits (0x%X) out of range"),
          		header & RTMP_HEADSIZE_MASK);
          headersize = 1;
          break;
    };

    return headersize;
}

RTMP::RTMP() 
    : _handshake(0), _handler(0)
{
//    GNASH_REPORT_FUNCTION;
//     _inbytes = 0;
//     _outbytes = 0;
    
//    _body = new unsigned char(RTMP_BODY_SIZE+1);
//    memset(_body, 0, RTMP_BODY_SIZE+1);
}

RTMP::~RTMP()
{
//    GNASH_REPORT_FUNCTION;
    _variables.clear();
//    delete _body;
}

void
RTMP::addProperty(amf::Element *el)
{
//    GNASH_REPORT_FUNCTION;
    _variables[el->getName()] = el;
}

void
RTMP::addProperty(char *name, amf::Element *el)
{ 
//    GNASH_REPORT_FUNCTION;
    _variables[name] = el;
}

amf::Element *
RTMP::getProperty(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
//    return _variables[name.c_str()];
    map<const char *, amf::Element *>::iterator it;
    for (it = _variables.begin(); it != _variables.end(); it++) {
	const char *title = it->first;
	amf::Element *el = it->second;
	if (name == title) {
// 	    log_debug("found variable in RTMP packet: %s", name);
	    return el;
	}
    }
}

// A request for a handshake is initiated by sending a byte with a
// value of 0x3, followed by a message body of unknown format.
bool
RTMP::handShakeRequest()
{
    GNASH_REPORT_FUNCTION;

#if 0
    char buffer[RTMP_BODY_SIZE+1];
    char c = 0x3;
    int  i, ret;
    
    ret = writeNet(&c, 1);
    _outbytes += 1;
    // something went wrong, chances are the other end of the network
    // connection is down, or never initialized.
    if (ret <= 0) {
        return false;
    }

    // Since we don't know what the format is, create a pattern we can
    // recognize if we stumble across it later on.
    for (i=0; i<RTMP_BODY_SIZE; i++) {
        buffer[i] = i^256;
    }
    
    _outbytes += RTMP_BODY_SIZE;
    ret = writeNet(buffer, RTMP_BODY_SIZE);
#endif
    
    return true;
}

// The client finished the handshake process by sending the second
// data block we get from the server as the response
bool
RTMP::clientFinish()
{
    GNASH_REPORT_FUNCTION;

#if 0
    char buffer[RTMP_BODY_SIZE+1];
    memset(buffer, 0, RTMP_BODY_SIZE+1);

    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        log_debug (_("Read first data block in handshake"));
    } else {
        log_error (_("Couldn't read first data block in handshake"));
        return false;
    }
    _inbytes += RTMP_BODY_SIZE;
    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        log_debug (_("Read second data block in handshake"));
//         _body = new char(RTMP_BODY_SIZE+1);
//         memcpy(_body, buffer, RTMP_BODY_SIZE);
    } else {
        log_error (_("Couldn't read second data block in handshake"));
        return false;
    }
    _inbytes += RTMP_BODY_SIZE;

    writeNet(buffer, RTMP_BODY_SIZE);
    _outbytes += RTMP_BODY_SIZE;
#endif
    
    return true;
}

bool
RTMP::packetRequest()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

#if 0
bool
RTMP::packetSend(amf::Buffer * /* buf */)
{
    GNASH_REPORT_FUNCTION;
    return false;
}
#endif

RTMP::rtmp_head_t *
RTMP::decodeHeader(Network::byte_t *in)
{
    GNASH_REPORT_FUNCTION;
    
    Network::byte_t *tmpptr = in;
    
    _header.channel = *tmpptr & RTMP_INDEX_MASK;
    log_debug (_("The AMF channel index is %d"), _header.channel);
    
    _header.head_size = headerSize(*tmpptr++);
    printf (_("The header size is %d"), _header.head_size);

    if (_header.head_size >= 4) {
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;
        log_debug(_("The mystery word is: %d"), _mystery_word);
    }

    if (_header.head_size >= 8) {
        _header.bodysize = *tmpptr++;
        _header.bodysize = (_header.bodysize << 12) + *tmpptr++;
        _header.bodysize = (_header.bodysize << 8) + *tmpptr++;
        _header.bodysize = _header.bodysize & 0xffffff;
        log_debug(_("The body size is: %d"), _header.bodysize);
    }

    if (_header.head_size >= 8) {
        _header.type = *(content_types_e *)tmpptr;
        tmpptr++;
        log_debug(_("The type is: %s"), content_str[_header.type]);
    }

//     switch(_header.type) {
//       case CHUNK_SIZE:
//       case BYTES_READ:
//       case PING:
//       case SERVER:
//       case CLIENT:
//       case VIDEO_DATA:
//       case NOTIFY:
//       case SHARED_OBJ:
//       case INVOKE:
//           _packet_size = RTMP_VIDEO_PACKET_SIZE;
//           break;
//       case AUDIO_DATA:
//           _packet_size = RTMP_AUDIO_PACKET_SIZE;
//           break;
//       default:
//           log_error (_("ERROR: Unidentified AMF header data type 0x%x"), _type);
//           break;
//     };
    
    if (_header.head_size == 12) {
        _header.src_dest = *(reinterpret_cast<rtmp_source_e *>(tmpptr));
        tmpptr += sizeof(unsigned int);
        log_debug(_("The source/destination is: %x"), _header.src_dest);
    }

    return &_header;
}

/// \brief \ Each RTMP header consists of the following:
///
/// * Index & header size - The header size and amf channel index.
/// * Total size - The total size of the message
/// * Type - The type of the message
/// * Routing - The source/destination of the message
//
// There are 3 size of RTMP headers, 1, 4, 8, and 12.
amf::Buffer *
RTMP::encodeHeader(int amf_index, rtmp_headersize_e head_size,
		       size_t total_size, content_types_e type,
		       rtmp_source_e routing)
{
    GNASH_REPORT_FUNCTION;

    amf::Buffer *buf;
    switch(head_size) {
      case HEADER_1:
	  buf = new Buffer(1);
	  break;
      case HEADER_4:
	  buf = new Buffer(4);
	  break;
      case HEADER_8:
	  buf = new Buffer(8);
	  break;
      case HEADER_12:
	  buf = new Buffer(12);
	  break;
    }
	
// FIXME: this is only to make this more readeable with GDB, and is a performance hit.
//    buf->clear();
    Network::byte_t *ptr = buf->reference();
    
    // Make the channel index & header size byte
    *ptr = head_size & RTMP_HEADSIZE_MASK;  
    *ptr += amf_index  & RTMP_INDEX_MASK;
    ptr++;
    
    // Add the unknown bytes. These seem to be used by video and
    // audio, and only when the header size is 4 or more.
    if ((head_size == HEADER_4) || (head_size == HEADER_8) || (head_size == HEADER_12)) {
	memset(ptr, 0, 3);
	ptr += 3;
    }
    
    // Add the size of the message if the header size is 8 or more.
    // and add the type of the object if the header size is 8 or more.
    if ((head_size == HEADER_8) || (head_size == HEADER_12)) {
        int length = total_size;
	Network::byte_t *lenptr = reinterpret_cast<Network::byte_t *>(&length);
//#ifndef	BOOST_BIG_ENDIAN
//	swapBytes(&length, 4);
	*ptr++ = *(lenptr + 2);
	*ptr++ = *(lenptr + 1);
	*ptr++ = *lenptr;	
//	*(lenptr + 3) = *(lenptr);
//	memcpy(ptr, lenptr, 3);
// #else
// #ifdef BOOST_BIG_ENDIAN
// 	memcpy(ptr, &length, 3);
// #else
// #error "No Endianess specified!"
// #endif
//#endif
//      swapBytes(&length, 4);
//        ptr += 3;
        *ptr = type;
        ptr++;
    }
    
    // Add the routing of the message if the header size is 12, the maximum.
    if (head_size == HEADER_12) {
        memcpy(ptr, &routing, 4);
        ptr += 4;
    }
    
    return buf;
}

bool
RTMP::packetRead(amf::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

//    int packetsize = 0;
    unsigned int amf_index, headersize;
    Network::byte_t *ptr = buf->reference();
    Network::byte_t *tooFar = ptr+buf->size();
    AMF amf;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634
    amf_index = *buf->reference() & RTMP_INDEX_MASK;
    headersize = headerSize(*buf->reference());
    log_debug (_("The Header size is: %d"), headersize);
    log_debug (_("The AMF index is: 0x%x"), amf_index);

//     if (headersize > 1) {
// 	packetsize = decodeHeader(ptr);
//         if (packetsize) {
//             log_debug (_("Read first RTMP packet header of size %d"), packetsize);
//         } else {
//             log_error (_("Couldn't read first RTMP packet header"));
//             return false;
//         }
//     }

#if 1
    Network::byte_t *end = buf->remove(0xc3);
#else
    Network::byte_t *end = buf->find(0xc3);
    log_debug("END is %x", (void *)end);
    *end = '*';
#endif
    
//    ptr = decodeHeader(ptr);
//    ptr += headersize;
    
    amf::Element *el = amf.extractAMF(ptr, tooFar);
    el->dump();
    el = amf.extractAMF(ptr, tooFar) + 1; // @@strk@@ : what's the +1 for ?
    el->dump();
    log_debug (_("Reading AMF packets till we're done..."));
    buf->dump();
    while (ptr < end) {
	amf::Element *el = amf.extractProperty(ptr, tooFar);
	addProperty(el);
	el->dump();
    }
    ptr += 1;
    size_t actual_size = static_cast<size_t>(_header.bodysize - AMF_HEADER_SIZE);
    log_debug("Total size in header is %d, buffer size is: %d", _header.bodysize, buf->size());
//    buf->dump();
    if (buf->size() < actual_size) {
	log_debug("FIXME: MERGING");
	buf = _handler->merge(buf);
    }
    while ((ptr - buf->begin()) < actual_size) {
	amf::Element *el = amf.extractProperty(ptr, tooFar);
	addProperty(el);
	el->dump();		// FIXME: dump the AMF objects as they are read in
    }

    dump();
    
    amf::Element *url = getProperty("tcUrl");
    amf::Element *file = getProperty("swfUrl");
    amf::Element *app = getProperty("app");
    
    if (file) {
	log_debug("SWF file %s", file->getData());
    }
    if (url) {
	log_debug("is Loading video %s", url->getData());
    }
    if (app) {
	log_debug("is file name is %s", app->getData());
    }
    
    return true;
}

void
RTMP::dump()
{
    cerr << "RTMP packet contains " << _variables.size() << " variables." << endl;
    map<const char *, amf::Element *>::iterator it;
    for (it = _variables.begin(); it != _variables.end(); it++) {
//	const char *name = it->first;
	amf::Element *el = it->second;
	el->dump();
    }
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
amf::Buffer *
RTMP::encodeChunkSize()
{
    GNASH_REPORT_FUNCTION;
}
void
RTMP::decodeChunkSize()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeBytesRead()
{
    GNASH_REPORT_FUNCTION;
}
void
RTMP::decodeBytesRead()
{
    GNASH_REPORT_FUNCTION;
}

// A RTMP Ping packet looks like this: "03 00 00 00 00 00 00 0B B8", which is the
// Ping type byte, followed by two shorts that are the parameters. Only the first
// two paramters are required.
amf::Buffer *
RTMP::encodePing(rtmp_ping_e type, boost::uint16_t milliseconds)
{
    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(sizeof(boost::uint16_t) * 4);
    Network::byte_t *ptr = buf->reference();
    buf->clear();		// default everything to zeros, real data gets optionally added.
    boost::uint16_t typefield = *reinterpret_cast<boost::uint16_t *>(&type);
    ptr += sizeof(boost::uint16_t); // go past the first short

    boost::uint16_t swapped = 0;
    buf->copy(typefield);
    switch (type) {
        // These two don't appear to have any paramaters
      case PING_CLEAR:
      case PING_PLAY:
	  break;
	  // the third parameter is the buffer time in milliseconds
      case PING_TIME:
      {
	  ptr += sizeof(boost::uint16_t); // go past the second short
	  swapped = htons(milliseconds);
	  buf->append(swapped);
	  break;
      }
      // reset doesn't have any parameters
      case PING_RESET:
	  break;
	  // For Ping and Pong, the second parameter is always the milliseconds
      case PING_CLIENT:
      case PONG_CLIENT:
      {
	  swapped = htons(milliseconds);
//	  std::copy(&swapped, &swapped + sizeof(boost::uint16_t), ptr);
	  buf->append(swapped);
	  break;
      }
      default:
	  return 0;
	  break;
    };
    
    return buf;
}
RTMP::rtmp_ping_t *
RTMP::decodePing(Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;
    
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(data);
    rtmp_ping_t *ping = new rtmp_ping_t;
    memset(ping, 0, sizeof(rtmp_ping_t));
    
    boost::uint16_t type = *reinterpret_cast<rtmp_ping_e *>(ptr);
    ping->type = static_cast<rtmp_ping_e>(type);
    ptr += sizeof(boost::uint16_t);

    ping->target = *reinterpret_cast<boost::uint16_t *>(ptr);
    ptr += sizeof(boost::uint16_t);
    
    ping->param1 = ntohs(*reinterpret_cast<boost::uint16_t *>(ptr));
    ptr += sizeof(boost::uint16_t);
    
    ping->param1 = ntohs(*reinterpret_cast<boost::uint16_t *>(ptr));

    return ping;    
}
RTMP::rtmp_ping_t *
RTMP::decodePing(amf::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;
    return decodePing(buf->reference());
}

amf::Buffer *
RTMP::encodeServer()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeServer()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeClient()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeClient()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeAudioData()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeAudioData()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeVideoData()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeVideoData()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeNotify()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeNotify()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeSharedObj()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeSharedObj()
{
    GNASH_REPORT_FUNCTION;
}
    
amf::Buffer *
RTMP::encodeInvoke()
{
    GNASH_REPORT_FUNCTION;
}
void 
RTMP::decodeInvoke()
{
    GNASH_REPORT_FUNCTION;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
