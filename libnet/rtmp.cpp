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

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include "log.h"
#include "amf.h"
#include "rtmp.h"
#include "network.h"
#include "handler.h"

using namespace amf;
using namespace gnash;
using namespace std;

namespace gnash
{

extern map<int, Handler *> handlers;

// These are the textual responses
const char *response_str[] = {
    "/onStatus",
    "/onResult",
    "/onDebugEvents"
};

int
RTMPproto::headerSize(Network::byte_t header)
{
//    GNASH_REPORT_FUNCTION;
    
    int headersize = -1;
    
    switch (header & AMF_HEADSIZE_MASK) {
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
          		header & AMF_HEADSIZE_MASK);
          headersize = 1;
          break;
    };

    return headersize;
}

RTMPproto::RTMPproto() 
    : _handshake(0), _handler(0)
{
//    GNASH_REPORT_FUNCTION;
//     _inbytes = 0;
//     _outbytes = 0;
    
//    _body = new unsigned char(RTMP_BODY_SIZE+1);
//    memset(_body, 0, RTMP_BODY_SIZE+1);
}

RTMPproto::~RTMPproto()
{
//    GNASH_REPORT_FUNCTION;
    _variables.clear();
//    delete _body;
}

void
RTMPproto::addVariable(char *name, char *value)
{
    _variables[name] = value;
}

std::string
RTMPproto::getVariable(char *name)
{
    return _variables[name];
}

// The handshake is a byte with the value of 0x3, followed by 1536
// bytes of gibberish which we need to store for later.
bool
RTMPproto::handShakeWait()
{
    GNASH_REPORT_FUNCTION;

//     char buffer[RTMP_BODY_SIZE+16];
//     memset(buffer, 0, RTMP_BODY_SIZE+16);
    Buffer *buf = _handler->pop();

    if (buf == 0) {
	log_debug("Que empty, net connection dropped for fd #%d", _handler->getFileFd());
	return false;
    }    
//     if (readNet(buffer, 1) == 1) {
    log_debug (_("Read initial Handshake Request"));
//     } else {
//         log_error (_("Couldn't read initial Handshake Request"));
//         return false;
//     }
//    _inbytes += 1;

    if (*(buf->reference()) == 0x3) {
        log_debug (_("Handshake is correct"));
    } else {
        log_error (_("Handshake isn't correct"));
        return false;
    }

//     if (buf->size() >= RTMP_BODY_SIZE) {
// 	secret = _handler->merge(buf->reference());
//     }

    if (buf->size() >= RTMP_BODY_SIZE) {
	_handshake = new Buffer(RTMP_BODY_SIZE);
	_handshake->copy(buf->reference() + 1, RTMP_BODY_SIZE);
	log_debug (_("Handshake Data matched"));
	delete buf;			// we're done with the buffer
	return true;
    } else {
	delete buf;			// we're done with the buffer
 	log_error (_("Handshake Data didn't match"));
 	return false;
    }
    
    return true;
}

// A request for a handshake is initiated by sending a byte with a
// value of 0x3, followed by a message body of unknown format.
bool
RTMPproto::handShakeRequest()
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

// The response is the gibberish sent back twice, preceeded by a byte
// with the value of 0x3.
bool
RTMPproto::handShakeResponse()
{
    GNASH_REPORT_FUNCTION;

    Buffer *buf = new Buffer((RTMP_BODY_SIZE * 2) + 1);
    Network::byte_t *ptr = buf->reference();
    *ptr = 0x3;

    std::copy(_handshake->begin(), _handshake->end(), (ptr + 1));
    std::copy(_handshake->begin(), _handshake->end(), ptr + _handshake->size() + 1);
    _handler->pushout(buf);
    _handler->notifyout();

    log_debug("Sent RTMP Handshake response");

    return true;    
}

// The client finished the handshake process by sending the second
// data block we get from the server as the response
bool
RTMPproto::clientFinish()
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
RTMPproto::serverFinish()
{
    GNASH_REPORT_FUNCTION;

    Buffer *buf = _handler->pop();
    Buffer *obj = buf;
    
    if (buf == 0) {
	log_debug("Que empty, net connection dropped for fd #%d", _handler->getFileFd());
	return false;
    }
    
    // The first data packet is often buried in with the end of the handshake.
    // So after the handshake block, we strip that part off, and just pass on
    // the remainder for processing.
    if (buf->size() > RTMP_BODY_SIZE) {
	int size = buf->size() - RTMP_BODY_SIZE;  
	obj = new Buffer[size];
	obj->copy(buf->begin()+RTMP_BODY_SIZE, size);
    } else {
	_handler->wait();
	obj = _handler->pop();
    }
    
    int diff = std::memcmp(buf->begin(), _handshake->begin(), RTMP_BODY_SIZE);
    delete buf;			// we're done with the buffer
    if (diff == 0) {
	log_debug (_("Handshake Finish Data matched"));
    } else {
	log_error (_("Handshake Finish Data didn't match by %d bytes"), diff);
//        return false;
    }
    
    packetRead(obj);
    
    return true;
}

bool
RTMPproto::packetRequest()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
RTMPproto::packetSend(Buffer *buf)
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
RTMPproto::packetRead(Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

    int ret;
    int packetsize = 0;
    unsigned int amf_index, headersize;
    Network::byte_t *ptr = buf->reference();
    AMF amf;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634
    amf_index = *buf->reference() & AMF_INDEX_MASK;
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

    Network::byte_t *end = buf->find(0xc3);
    log_debug("END is 0x%x", (void *)end);
    *end = '*';
    packetsize = parseHeader(ptr);
    ptr += headersize;

    Element el;
    ptr = amf.extractElement(&el, ptr);
    el.dump();
    ptr = amf.extractElement(&el, ptr) + 1;
    el.dump();
    log_debug (_("Reading AMF packets till we're done..."));
//    buf->dump();
    while (ptr < end) {
	amf::Element *el = new amf::Element;
	ptr = amf.extractVariable(el, ptr);
	el->dump();
// 	if (ptr != 0) {
// 	    ptr += 1;    
// //	    addObj(el);
// 	} else {
// 	    break;
// 	    }
    }
    ptr += 1;
    while (ptr < buf->end()) {
	amf::Element *el = new amf::Element;
	ptr = amf.extractVariable(el, ptr);
	el->dump();
    }
    
    return true;
}

int
RTMPproto::parseHeader(Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;

    Network::byte_t *tmpptr = in;
    
    _amf_index = *tmpptr & AMF_INDEX_MASK;
    log_debug (_("The AMF channel index is %d"), _amf_index);
    
    _header_size = headerSize(*tmpptr++);
    log_debug (_("The header size is %d"), _header_size);

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
//        _amf_data = new uint8_t(_total_size+1);
//        _seekptr = _amf_data;
//        memset(_amf_data, 0, _total_size+1);
        log_debug(_("The body size is: %d"), _total_size);
    }

    if (_header_size >= 8) {
        _type = *(AMF::content_types_e *)tmpptr;
        tmpptr++;
        log_debug(_("The type is: 0x%x"), _type);
    }

    switch(_type) {
      case AMF::CHUNK_SIZE:
      case AMF::BYTES_READ:
      case AMF::PING:
      case AMF::SERVER:
      case AMF::CLIENT:
      case AMF::VIDEO_DATA:
      case AMF::NOTIFY:
      case AMF::SHARED_OBJ:
      case AMF::INVOKE:
          _packet_size = AMF_VIDEO_PACKET_SIZE;
          break;
      case AMF::AUDIO_DATA:
          _packet_size = AMF_AUDIO_PACKET_SIZE;
          break;
      default:
          log_error (_("ERROR: Unidentified AMF header data type 0x%x"), _type);
          break;
    };
    
    if (_header_size == 12) {
//        hexify((Network::byte_t *)hexint, (Network::byte_t *)tmpptr, 3, false);
        _src_dest = *(reinterpret_cast<rtmp_source_e *>(tmpptr));
        tmpptr += sizeof(unsigned int);
//        log_debug(_("The source/destination is: %d, or 0x%s"), _src_dest, hexint);
    }

    return _packet_size;
}

// This is the thread for all incoming RTMP connections
void
rtmp_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
    Handler *hand = reinterpret_cast<Handler *>(args->handle);
    RTMPproto rtmp;

    rtmp.setHandler(hand);
    string docroot = args->filespec;

    log_debug(_("Starting RTMP Handler for fd #%d, tid %ld"),
	      args->netfd, pthread_self());
    
    while (!hand->timetodie()) {	
 	log_debug(_("Waiting for RTMP request on fd #%d..."), args->netfd);
	hand->wait();
	// This thread is the last to wake up when the browser
	// closes the network connection. When browsers do this
	// varies, elinks and lynx are very forgiving to a more
	// flexible HTTP protocol, which Firefox/Mozilla & Opera
	// are much pickier, and will hang or fail to load if
	// you aren't careful.
	if (hand->timetodie()) {
	    log_debug("Not waiting no more, no more for RTMP data for fd #%d...", args->netfd);
	    map<int, Handler *>::iterator hit = handlers.find(args->netfd);
	    if ((*hit).second) {
		log_debug("Removing handle %x for RTMP on fd #%d", (void *)hand), args->netfd;
		handlers.erase(args->netfd);
	    }

	    return;
	}
#ifdef USE_STATISTICS
	struct timespec start;
	clock_gettime (CLOCK_REALTIME, &start);
#endif
	if (!rtmp.handShakeWait()) {
	    hand->clearout();	// remove all data from the outgoing que
	    hand->die();	// tell all the threads for this connection to die
	    hand->notifyin();
	    log_debug("Net RTMP done for fd #%d...", args->netfd);
// 	    hand->closeNet(args->netfd);
	    return;
	}
	string url, filespec;
	url = docroot;
	
	rtmp.handShakeResponse();

	hand->wait();
	// This thread is the last to wake up when the browser
	// closes the network connection. When browsers do this
	// varies, elinks and lynx are very forgiving to a more
	// flexible HTTP protocol, which Firefox/Mozilla & Opera
	// are much pickier, and will hang or fail to load if
	// you aren't careful.
	if (hand->timetodie()) {
	    log_debug("Not waiting no more, no more for RTMP data for fd #%d...", args->netfd);
	    map<int, Handler *>::iterator hit = handlers.find(args->netfd);
	    if ((*hit).second) {
		log_debug("Removing handle %x for RTMP on fd #%d", (void *)hand), args->netfd;
		handlers.erase(args->netfd);
	    }

	    return;
	}
	rtmp.serverFinish();
    
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
    }
}
    
} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
