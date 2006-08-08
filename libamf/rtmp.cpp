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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include "rtmp.h"
#include "log.h"
#include "new"

#if ! (defined(_WIN32) || defined(WIN32))
#include <netinet/in.h>
#endif

using namespace gnash;
using namespace amf;
using namespace std;

namespace rtmp
{

RTMPproto::RTMPproto() 
    : _body(0)
{
//    GNASH_REPORT_FUNCTION;  
//    _amf_data.reserve(MAX_AMF_INDEXES);
//    memset(_amf_data, 0, MAX_AMF_INDEXES);
    memset(_amf_size, 0, MAX_AMF_INDEXES);
}

RTMPproto::~RTMPproto()
{
//    GNASH_REPORT_FUNCTION;
    delete _body;
}


// The handshake is a byte with the value of 0x3, followed by 1536
// bytes of gibberish which we need to store for later.
bool
RTMPproto::handShakeWait()
{
    GNASH_REPORT_FUNCTION;

    char buffer[RTMP_HEADERSIZE+1];
    memset(buffer, 0, RTMP_HEADERSIZE+1);
    
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
    
    if (readNet(buffer, RTMP_HEADERSIZE) == RTMP_HEADERSIZE) {        
        dbglogfile << "Read Handshake Data" << endl;
        _body = new char(RTMP_HEADERSIZE+1);
        memcpy(_body, buffer, RTMP_HEADERSIZE);        
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

    char buffer[RTMP_HEADERSIZE+1];
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
    for (i=0; i<RTMP_HEADERSIZE; i++) {
        buffer[i] = i^256;
    }
             
    ret = writeNet(buffer, RTMP_HEADERSIZE);

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
    writeNet(_body, RTMP_HEADERSIZE);
    writeNet(_body, RTMP_HEADERSIZE);
    
    return true;    
}

// The client finished the handshake process by sending the second
// data block we get from the server as the response
bool
RTMPproto::clientFinish()
{
    GNASH_REPORT_FUNCTION;
    
    char buffer[RTMP_HEADERSIZE+1];
    memset(buffer, 0, RTMP_HEADERSIZE+1);

    if (readNet(buffer, RTMP_HEADERSIZE) == RTMP_HEADERSIZE) {        
        dbglogfile << "Read first data block in handshake" << endl;
    } else {
        dbglogfile << "ERROR: Couldn't read first data block in handshake!"
                   << endl;
        return false;
    }
    if (readNet(buffer, RTMP_HEADERSIZE) == RTMP_HEADERSIZE) {        
        dbglogfile << "Read second data block in handshake" << endl;
//         _body = new char(RTMP_HEADERSIZE+1);
//         memcpy(_body, buffer, RTMP_HEADERSIZE);
    } else {
        dbglogfile << "ERROR: Couldn't read second data block in handshake!"
                   << endl;
        return false;
    }

    writeNet(buffer, RTMP_HEADERSIZE);

    return true;
}

bool
RTMPproto::serverFinish()
{
    GNASH_REPORT_FUNCTION;

    int ret;
    char buffer[RTMP_HEADERSIZE+1];
    memset(buffer, 0, RTMP_HEADERSIZE+1);
    
    if (readNet(buffer, RTMP_HEADERSIZE) == RTMP_HEADERSIZE) {
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

int
RTMPproto::headerSize(char header)
{
    unsigned char hexint[2];
    switch (header & RTMP_HEADSIZE_MASK) {
      case HEADER_12:
          return(12);
          break;
      case HEADER_8:
          return(8);
          break;
      case HEADER_4:
          return(4);
          break;
      case HEADER_1:
          return(1);
          break;
      default:
          hexify((unsigned char *)hexint, (unsigned char *)&header, 1);
          dbglogfile << "ERROR: Header size bits out of range! was 0x"
                     << hexint << endl;
          return -1;
          break;
    };

    return -1;
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
    char buffer[AMFBODY_PACKET_SIZE+1];
    memset(buffer, 0, AMFBODY_PACKET_SIZE+1);
    unsigned char hexint[512];
    int blocksize;
    char *tmpptr;
    char *amfdata;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634
    if ((ret = readNet(buffer, 1)) > 0) {
        dbglogfile << "Read first RTMP header byte"<< endl;
    } else {
        dbglogfile << "ERROR: Couldn't read first RTMP header byte!" << endl;
        return false;
    }
    
    _headersize = headerSize(*buffer);
    _amf_number = *buffer & RTMP_AMF_INDEX;
    dbglogfile << "The Header size is: " << _headersize << endl;
    dbglogfile << "The AMF index is: 0x" << _amf_number << endl;

    
    if ((ret = readNet(buffer, _headersize-1)) > 0) {
        dbglogfile << "Read first RTMP packet body of " << ret
                   << " bytes." << endl;
    } else {
        dbglogfile << "ERROR: Couldn't read first RTMP packet body!" << endl;
        return false;
    }
    tmpptr = buffer;
    
    if (_headersize >= 4) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3);
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;
         dbglogfile << "The mystery word is: " << _mystery_word
                    << " Hex value is: 0x" << hexint << endl;
    }

    if (_headersize >= 8) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3);
        _bodysize = *tmpptr++;
        _bodysize = (_bodysize << 12) + *tmpptr++;
        _bodysize = (_bodysize << 8) + *tmpptr++;
        _bodysize = _bodysize & 0x0000ff;
        dbglogfile << "The body size is: " << _bodysize
                   << " Hex value is: 0x" << hexint << endl;
    }
//    _amf_data[_amf_number] = new unsigned char(_bodysize);
    _amf_size[_amf_number] = 0;

    if (_headersize >= 8) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 1);
        _type = *(rtmp_types_e *)tmpptr;
        tmpptr++;
        dbglogfile << "The type is: " << _type
                   << " Hex value is: 0x" << hexint << endl;
    }

    if (_headersize == 12) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3);
        _src_dest = ntohl(*(unsigned int *)tmpptr);
        tmpptr += sizeof(unsigned int);
        dbglogfile << "The source/destination is: " << _src_dest
                   << " Hex value is: 0x" << hexint << endl;
    }

#if 0
    if ((ret = readNet(buffer, AMFBODY_PACKET_SIZE - _headersize)) > 0) {
        dbglogfile << "Read first RTMP packet body of " << ret
                   << " bytes." << endl;
    } else {
        dbglogfile << "ERROR: Couldn't read first RTMP packet body!" << endl;
        return false;
    }
    tmpptr = buffer;
    
//     hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 50);
//     dbglogfile << "The AMF header is: " << hexint << endl;

    if (_amf_number < MAX_AMF_INDEXES) {
        if (_amf_size[_amf_number] == 0) {
            dbglogfile << "New AMF object, index: " << _amf_number << endl;
//      amfdata = new char(AMFBODY_PACKET_SIZE+1);
//         memcpy(amfdata, tmpptr, _bodysize);
//         _amf_data[_amf_number] = amfdata;
            _amf_data[_amf_number] = (unsigned char *)tmpptr ;
            _amf_size[_amf_number] = _bodysize;
        } else {
            int amd_headersize = headerSize(*tmpptr);
            int amf_index = *tmpptr & RTMP_AMF_INDEX;
            dbglogfile << "The AMF Header size is: " << amd_headersize << endl;
            dbglogfile << "The AMF index is: 0x" << amf_index << endl;
//         amfdata = new unsigned char(
//             _amf_size[amf_index]+AMFBODY_PACKET_SIZE+1);
//         memcpy(amfdata, _amf_data[amf_index],
//                _amf_size[amf_index]+AMFBODY_PACKET_SIZE+1);
//         memcpy(amfdata+_amf_size[amf_index], tmpptr,
//                _amf_size[amf_index]+AMFBODY_PACKET_SIZE+1);
//       _amf_size[_amf_number] = _bodysize;
        }
        
    }
#endif

    while ((ret = packetReadAMF(0)) > 0) {
        dbglogfile << "Reading AMF packetes till we're done..." << endl;
    }
    
//    _amf_data
    return true;
}

int
RTMPproto::packetReadAMF(int bytes)
{
    GNASH_REPORT_FUNCTION;

    int ret;
    char buffer[AMFBODY_PACKET_SIZE+1];
    unsigned char hexint[(AMFBODY_PACKET_SIZE*2)+1];
    char *tmpptr;
    unsigned char *amfdata;
    int amf_index;

    if (bytes == 0) {
        bytes = AMFBODY_PACKET_SIZE;
    }
    
    memset(buffer, 0, AMFBODY_PACKET_SIZE+1);
    if ((ret = readNet(buffer, bytes)) >= 0) {
        if (ret == 0) {
            dbglogfile << "Done reading AMF message" << endl;
            return ret;
        }
        dbglogfile << "Read AMF packet, " << ret
                   << " bytes in size." << endl;
    } else {
        dbglogfile << "ERROR: Couldn't read AMF packet!" << endl;
        return -1;
    }
    
    tmpptr = buffer;

    hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 40);
    dbglogfile << "AMF packet: 0x" << hexint << endl;

    switch (*tmpptr & RTMP_HEADSIZE_MASK) {
      case 0x02:
      case 0x03:
          _headersize = 12;
          break;
      case 0x43:
          _headersize  = 8;
          break;
      case 0x83:
          _headersize  = 4;
          break;
      case 0xc3:
          _headersize = 1;
          break;
      default:
          hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 1);
          dbglogfile << "ERROR: Header size bits out of range! was 0x"
                     << hexint << endl;
          _headersize = 1;
          break;
    };

    amf_index = *tmpptr & RTMP_AMF_INDEX;
    tmpptr++;
    dbglogfile << "The Header size is: " << _headersize << endl;
    dbglogfile << "The AMF index is: 0x" << amf_index << endl;
    dbglogfile << "Read " << ret << " bytes" << endl;

//     memcpy(_amf_data[_amf_index]+_amf_size[amf_index],
//            AMFBODY_PACKET_SIZE);
    _amf_size[amf_index] += ret;
        
    return ret;
}


} // end of cygnal namespace
