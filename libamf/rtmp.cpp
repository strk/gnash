
// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

// #ifdef HAVE_CONFIG_H
// #include "config.h"
// #endif

#include <iostream>
#include <netinet/in.h>
#include <new>
#include "log.h"
#include "rtmp.h"
#include "log.h"
#include "new"

#if ! (defined(_WIN32) || defined(WIN32))
#include <netinet/in.h>
#endif

using namespace amf;
using namespace std;

namespace gnash
{

RTMPproto::RTMPproto() 
{
    GNASH_REPORT_FUNCTION;  
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
        dbglogfile << "Read initial Handshake Request" << endl;
    } else {
        dbglogfile << "Couldn't read initial Handshake Request" << endl;
        return false;
    }
    if (*buffer == 0x3) {
        dbglogfile << "Handshake is correct" << endl;
    } else {
        dbglogfile << "Handshake isn't correct" << endl;
        dbglogfile << "Data read is: " << buffer << endl;
//        return false;
    }
    
    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        dbglogfile << "Read Handshake Data" << endl;
//        _body = new char(RTMP_BODY_SIZE+1);
        memcpy(_body, buffer, RTMP_BODY_SIZE);        
    } else {
        dbglogfile << "Couldn't read Handshake Data" << endl;
        dbglogfile << "Data read is: " << buffer << endl;
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
             
    ret = writeNet(buffer, RTMP_BODY_SIZE);

    return true;
}

// The response is the giobberish sent back twice, followed by a byte
// with the value of 0x3.
bool
RTMPproto::handShakeResponse()
{
    GNASH_REPORT_FUNCTION;

    char c = 0x3;
    writeNet(&c, 1);
    writeNet(_body, RTMP_BODY_SIZE);
    writeNet(_body, RTMP_BODY_SIZE);
    
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
        dbglogfile << "Read first data block in handshake" << endl;
    } else {
        dbglogfile << "ERROR: Couldn't read first data block in handshake!"
                   << endl;
        return false;
    }
    if (readNet(buffer, RTMP_BODY_SIZE) == RTMP_BODY_SIZE) {        
        dbglogfile << "Read second data block in handshake" << endl;
//         _body = new char(RTMP_BODY_SIZE+1);
//         memcpy(_body, buffer, RTMP_BODY_SIZE);
    } else {
        dbglogfile << "ERROR: Couldn't read second data block in handshake!"
                   << endl;
        return false;
    }

    writeNet(buffer, RTMP_BODY_SIZE);

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
        dbglogfile << "Read Handshake Finish Data" << endl;
    } else {
        dbglogfile << "ERROR: Couldn't read Handshake Finish Data!" << endl;
        return false;
    }

// FIXME: These should match, and appear to in GDB, but this triggers
// an error of some kind.    
//     if (memcmp(buffer, _body, 10) == 0) {
//         dbglogfile << "Handshake Finish Data matches" << endl;
//     } else {
//         dbglogfile << "ERROR: Handshake Finish Data doesn't match!" << endl;
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
    char buffer[AMF_VIDEO_PACKET_SIZE+1];
    memset(buffer, 0, AMF_VIDEO_PACKET_SIZE+1);
    unsigned char hexint[1024];
    int packetsize = 0;
    char *tmpptr;
    char *amfdata;
    int amf_index, headersize;
    AMF *amf;
    
    tmpptr = buffer;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634

    if ((ret = readNet(buffer, 1)) > 0) {
        dbglogfile << "Read first RTMP header byte"<< endl;
    } else {
        dbglogfile << "ERROR: Couldn't read first RTMP header byte!" << endl;
        return false;
    }
    
    amf_index = *tmpptr & AMF_INDEX_MASK;
    headersize = AMF::headerSize(*tmpptr++);
    dbglogfile << "The Header size is: " << headersize << endl;
    dbglogfile << "The AMF index is: 0x" << amf_index << endl;

    if (headersize > 1) {
        if ((ret = readNet(tmpptr, headersize-1)) > 0) {
            dbglogfile << "Read first RTMP packet header of " << ret
                       << " headersize." << endl;
        } else {
            dbglogfile << "ERROR: Couldn't read first RTMP packet header!" << endl;
            return false;
        }
    }
    if (_amfs.size() < headersize) {
        amf = new AMF;
    }
    
    packetsize = amf->parseHeader(buffer);
    
    tmpptr += headersize;
    
    while ((ret = readNet(buffer, packetsize)) > 0) {
        amf->parseBody(buffer, ret);
        dbglogfile << "Reading AMF packets till we're done..." << endl;
    }
     
    return true;
}

} // end of cygnal namespace
