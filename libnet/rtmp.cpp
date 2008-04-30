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

/// \brief \ Each RTMP header consists of the following:
///
/// * Index & header size - The header size and amf channel index.
/// * Total size - The total size of the message
/// * Type - The type of the message
/// * Routing - The source/destination of the message
Network::byte_t *
RTMP::encodeRTMPHeader(int amf_index, rtmp_headersize_e head_size,
			    size_t total_size, content_types_e type,
			    rtmp_source_e routing)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *out = new Network::byte_t[total_size + 12 + 4];
    memset(out, 0, total_size + 12 + 4);
    char *tmpptr = reinterpret_cast<char *>(out);
    // Make the index & header size byte
    *tmpptr = head_size & RTMP_HEADSIZE_MASK;    
    *tmpptr += amf_index  & RTMP_INDEX_MASK;
    tmpptr++;

    // Add the unknown bytes. These seem to be used by video and
    // audio, and only when the header size is 4 or more.
    if (head_size <= HEADER_4) {
        memset(tmpptr, 0, 3);
        tmpptr += 3;
    }

    // Add the size of the message if the header size is 8 or more.
    if (head_size <= HEADER_8) {
        int length = total_size;
        swapBytes(&length, 4);
        memcpy(tmpptr, ((char *)&length +1), 3);
        tmpptr += 3;
    }
    
    // Add the type of the objectif the header size is 8 or more.
    if (head_size <= HEADER_8) {
        *tmpptr = type;
        tmpptr++;
    }

    // Add the routing of the message if the header size is 12 or more.
    if (head_size == HEADER_12) {
        memcpy(tmpptr, &routing, 4);
        tmpptr += 4;
    }

    return out;
}

bool
RTMP::packetRead(amf::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

    int packetsize = 0;
    unsigned int amf_index, headersize;
    Network::byte_t *ptr = buf->reference();
    AMF amf;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634
    amf_index = *buf->reference() & RTMP_INDEX_MASK;
    headersize = headerSize(*buf->reference());
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

#if 1
    Network::byte_t *end = buf->remove(0xc3);
#else
    Network::byte_t *end = buf->find(0xc3);
    log_debug("END is %x", (void *)end);
    *end = '*';
#endif
    ptr = parseHeader(ptr);
//     ptr += headersize;
    
    amf::Element *el = amf.extractAMF(ptr);
    el->dump();
    el = amf.extractAMF(ptr) + 1;
    el->dump();
    log_debug (_("Reading AMF packets till we're done..."));
    buf->dump();
    while (ptr < end) {
	amf::Element *el = amf.extractProperty(ptr);
	addProperty(el);
	el->dump();
    }
    ptr += 1;
    size_t actual_size = static_cast<size_t>(_total_size - AMF_HEADER_SIZE);
    log_debug("Total size in header is %d, buffer size is: %d", _total_size, buf->size());
//    buf->dump();
    if (buf->size() < actual_size) {
	log_debug("FIXME: MERGING");
	buf = _handler->merge(buf);
    }
    while ((ptr - buf->begin()) < actual_size) {
	amf::Element *el = amf.extractProperty(ptr);
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

Network::byte_t *
RTMP::parseHeader(Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;

    Network::byte_t *tmpptr = in;
    
    _amf_index = *tmpptr & RTMP_INDEX_MASK;
    log_debug (_("The AMF channel index is %d"), _amf_index);
    
    _header_size = headerSize(*tmpptr++);
    printf (_("The header size is %d"), _header_size);

    if (_header_size >= 4) {
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;
        log_debug(_("The mystery word is: %d"), _mystery_word);
    }

    if (_header_size >= 8) {
        _total_size = *tmpptr++;
        _total_size = (_total_size << 12) + *tmpptr++;
        _total_size = (_total_size << 8) + *tmpptr++;
        _total_size = _total_size & 0xffffff;
        log_debug(_("The body size is: %d"), _total_size);
    }

    if (_header_size >= 8) {
        _type = *(content_types_e *)tmpptr;
        tmpptr++;
        log_debug(_("The type is: %s"), content_str[_type]);
    }

//     switch(_type) {
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
    
    if (_header_size == 12) {
        _src_dest = *(reinterpret_cast<rtmp_source_e *>(tmpptr));
        tmpptr += sizeof(unsigned int);
        log_debug(_("The source/destination is: %x"), _src_dest);
    }

    return tmpptr;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
