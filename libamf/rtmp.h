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

#ifndef _RTMP_H_
#define _RTMP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <boost/cstdint.hpp>
#include <vector>

#include "amf.h"
#include "protocol.h"

namespace gnash
{

#define RTMP_HANDSHAKE 0x3
#define RTMP_BODY_SIZE 1536
#define MAX_AMF_INDEXES 64

// These are the textual responses
const char *response_str[] = {
    "/onStatus",
    "/onResult",
    "/onDebugEvents"
};

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
    typedef enum {
        RTMP_STATE_HANDSHAKE_SEND,
        RTMP_STATE_HANDSHAKE_RECV,
        RTMP_STATE_HANDSHAKE_ACK,
        RTMP_STATE_CONNECT,
        RTMP_STATE_NETCONNECT,
        RTMP_STATE_NETSTREAM,
        RTMP_STATE_HEADER,
        RTMP_STATE_DONE
    } rtmp_state_t;
    typedef enum {
        RTMP_ERR_UNDEF=0,
        RTMP_ERR_NOTFOUND,
        RTMP_ERR_PERM,
        RTMP_ERR_DISKFULL,
        RTMP_ERR_ILLEGAL,
        RTMP_ERR_UNKNOWNID,
        RTMP_ERR_EXISTS,
        RTMP_ERR_NOSUCHUSER,
        RTMP_ERR_TIMEOUT,
        RTMP_ERR_NORESPONSE
    } rtmp_error_t;

// Each header consists of the following:
//
// * UTF string (including length bytes) - name
// * Boolean - specifies if understanding the header is `required'
// * Long - Length in bytes of header
// * Variable - Actual data (including a type code)
    typedef struct {
        amf::amfutf8_t name;
	boost::uint8_t required;
	boost::uint32_t length;
        void *data;
    } rtmp_head_t;
    
// Each body consists of the following:
//
// * UTF String - Target
// * UTF String - Response
// * Long - Body length in bytes
// * Variable - Actual data (including a type code)
    typedef struct {
        amf::amfutf8_t target;
        amf::amfutf8_t response;
	boost::uint32_t length;
        void *data;
    } rtmp_body_t;
    
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

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

