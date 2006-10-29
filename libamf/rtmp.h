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

// 
//

#ifndef _RTMP_H_
#define _RTMP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <vector>

#include "amf.h"
#include "protocol.h"

namespace gnash
{

#define RTMP_HANDSHAKE 0x3
#define RTMP_BODY_SIZE 1536
#define MAX_AMF_INDEXES 64

class DSOEXPORT RTMPproto : public Protocol
{
public:
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

    void addVariable(char *name, char *value);
    std::string getVariable(char *name);
private:
    std::map<char *, std::string> _variables;
    unsigned char               _body[RTMP_BODY_SIZE+1];
    std::vector<amf::AMF *>     _amfs;
};

} // end of gnash namespace

// end of _RTMP_H_
#endif
