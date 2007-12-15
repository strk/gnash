// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include <iostream>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include <new>
#include "log.h"
#include "rtmp.h"

using namespace amf;
using namespace std;

namespace gnash
{

RTMPproto::RTMPproto() 
{
//    GNASH_REPORT_FUNCTION;
    _inbytes = 0;
    _outbytes = 0;
    
//    _body = new unsigned char(RTMP_BODY_SIZE+1);
    memset(_body, 0, RTMP_BODY_SIZE+1);
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

    char buffer[RTMP_BODY_SIZE+16];
    memset(buffer, 0, RTMP_BODY_SIZE+16);
    
    if (readNet(buffer, 1) == 1) {
        log_msg (_("Read initial Handshake Request"));
    } else {
        log_error (_("Couldn't read initial Handshake Request"));
        return false;
    }
    _inbytes += 1;
    
    if (*buffer == 0x3) {
        log_msg (_("Handshake is correct"));
    } else {
        log_error (_("Handshake isn't correct; "
        	     "Data read is: 0x%x"), *buffer);
        return false;
    }
    
    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        _inbytes += RTMP_BODY_SIZE;
        log_msg (_("Read Handshake Data"));
//        _body = new char(RTMP_BODY_SIZE+1);
        memcpy(_body, buffer, RTMP_BODY_SIZE);        
    } else {
        log_error (_("Couldn't read Handshake Data" 
        	     "Data read is: %s"), buffer);
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

    return true;
}

// The response is the giobberish sent back twice, preceeded by a byte
// with the value of 0x3.
bool
RTMPproto::handShakeResponse()
{
    GNASH_REPORT_FUNCTION;

    char c = 0x3;
    writeNet(&c, 1);
    _outbytes += 1;
    writeNet(_body, RTMP_BODY_SIZE);
    _outbytes += RTMP_BODY_SIZE;
    writeNet(_body, RTMP_BODY_SIZE);
    _outbytes += RTMP_BODY_SIZE;
   
    return true;    
}

// The client finished the handshake process by sending the second
// data block we get from the server as the response
bool
RTMPproto::clientFinish()
{
    GNASH_REPORT_FUNCTION;
    
    char buffer[RTMP_BODY_SIZE+1];
    memset(buffer, 0, RTMP_BODY_SIZE+1);

    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        log_msg (_("Read first data block in handshake"));
    } else {
        log_error (_("Couldn't read first data block in handshake"));
        return false;
    }
    _inbytes += RTMP_BODY_SIZE;
    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        log_msg (_("Read second data block in handshake"));
//         _body = new char(RTMP_BODY_SIZE+1);
//         memcpy(_body, buffer, RTMP_BODY_SIZE);
    } else {
        log_error (_("Couldn't read second data block in handshake"));
        return false;
    }
    _inbytes += RTMP_BODY_SIZE;

    writeNet(buffer, RTMP_BODY_SIZE);
    _outbytes += RTMP_BODY_SIZE;

    return true;
}

bool
RTMPproto::serverFinish()
{
    GNASH_REPORT_FUNCTION;

//    int ret;
     char buffer[RTMP_BODY_SIZE+1];
     memset(buffer, 0, RTMP_BODY_SIZE+1);
    
    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {
        log_msg (_("Read Handshake Finish Data"));
    } else {
        log_error (_("Couldn't read Handshake Finish Data"));
        return false;
    }

    _inbytes += RTMP_BODY_SIZE;
// FIXME: These should match, and appear to in GDB, but this triggers
// an error of some kind.    
//     if (memcmp(buffer, _body, 10) == 0) {
//         log_msg (_("Handshake Finish Data matches"));
//     } else {
//         log_error (_("Handshake Finish Data doesn't match"));
//         return false;
//     }
        

    packetRead();
    
    return true;
}

bool
RTMPproto::packetRequest()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
RTMPproto::packetSend()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
RTMPproto::packetRead()
{
    GNASH_REPORT_FUNCTION;

    int ret;
    unsigned char buffer[AMF_VIDEO_PACKET_SIZE+1];
    memset(buffer, 0, AMF_VIDEO_PACKET_SIZE+1);
    //unsigned char hexint[1024];
    int packetsize = 0;
    unsigned char *tmpptr;
    //char *amfdata;
    unsigned int amf_index, headersize;
    AMF *amf=NULL;
#if 1
    unsigned char hexint[512];
#endif
    
    tmpptr = buffer;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634

    if ((ret = readNet(reinterpret_cast<char *>(buffer), 1)) > 0) {
        log_msg (_("Read first RTMP header byte"));
    } else {
        log_error (_("Couldn't read first RTMP header byte"));
        return false;
    }
    
    amf_index = *tmpptr & AMF_INDEX_MASK;
    headersize = AMF::headerSize(*tmpptr++);
    log_msg (_("The Header size is: %d"), headersize);
    log_msg (_("The AMF index is: 0x%x"), amf_index);

    if (headersize > 1) {
        if ((ret = readNet(reinterpret_cast<char *>(tmpptr), headersize-1)) > 0) {
            log_msg (_("Read first RTMP packet header of header size %d"),
                       ret);
            _inbytes += ret;
        } else {
            log_error (_("Couldn't read first RTMP packet header"));
            return false;
        }
    }
    if (_amfs.size() < headersize) {
        amf = new AMF;
    }
    
    packetsize = amf->parseHeader(buffer);
    tmpptr += headersize;
    tmpptr = buffer;
    
    while ((ret = readNet(reinterpret_cast<char *>(buffer), packetsize, 1)) > 0) {
        log_msg (_("Reading AMF packets till we're done..."));
        amf->addPacketData(tmpptr, ret);
        tmpptr = buffer + ret;
        _inbytes += ret;
#if 1
	hexify(hexint, buffer, packetsize, true);
	log_msg (_("The packet data is: 0x%s"), (char *)hexint);
	hexify(hexint, buffer, packetsize, false);
	log_msg (_("The packet data is: 0x%s"), (char *)hexint);
#endif    
    }
    
    log_msg (_("Done reading packet"));
    amf->parseBody();
    
    return true;
}

} // end of cygnal namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
