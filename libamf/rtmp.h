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

#ifndef _RTMP_H_
#define _RTMP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "amf.h"
#include "protocol.h"
#include <vector>

namespace rtmp
{

#define RTMP_HANDSHAKE 0x3
#define RTMP_HEADERSIZE 1536
#define RTMP_HEADSIZE_MASK 0xc0
#define RTMP_AMF_INDEX 0x3f
#define MAX_AMF_INDEXES 64
#define AMFBODY_PACKET_SIZE 128

class RTMPproto : public Protocol
{
public:
    typedef enum {
        HEADER_12 = 0x0,
        HEADER_8  = 0x1,
        HEADER_4  = 0x2,
        HEADER_1  = 0x3
    } rtmp_headersize_e;    
    
    typedef enum {
        CHUNK_SIZE = 0x1,
//    UNKNOWN = 0x2,
        BYTES_READ = 0x3,
        PING = 0x4,
        SERVER = 0x5,
        CLIENT = 0x6,
//    UNKNOWN2 = 0x7,
        AUDIO_DATA = 0x8,
        VIDEO_DATA = 0x9,
//    UNKNOWN3 = 0xa,
        NOTIFY = 0x12,
        SHARED_OBJ = 0x13,
        INVOKE = 0x14
    } rtmp_types_e;
    
    typedef enum {
        CONNECT = 0x1,
        DISCONNECT = 0x2,
        SET_ATTRIBUTE = 0x3,
        UPDATE_DATA = 0x4,
        UPDATE_ATTRIBUTE = 0x5,
        SEND_MESSAGE = 0x6,
        STATUS = 0x7,
        CLEAR_DATA = 0x8,
        DELETE_DATA = 0x9,
        DELETE_ATTRIBUTE = 0xa,
        INITIAL_DATA = 0xb
    } sharedobj_types_e;
    RTMPproto();
    virtual ~RTMPproto();
    virtual bool handShakeWait();
    virtual bool handShakeRequest();
    virtual bool handShakeResponse();
    virtual bool clientFinish();
    virtual bool serverFinish();
    virtual bool packetRequest();
    virtual bool packetSend();
    virtual bool packetRead();
    int headerSize(char header);
    int packetReadAMF(int bytes);
private:
    int         _headersize;
    int         _amf_number;
    char        *_body;
    int         _bodysize;
    rtmp_types_e _type;
    unsigned char *_amf_data[MAX_AMF_INDEXES];
    int         _amf_size[MAX_AMF_INDEXES];
    int         _mystery_word;
    int         _src_dest;
};

} // end of rtmp namespace

// end of _RTMP_H_
#endif
