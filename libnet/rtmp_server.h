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

#ifndef _RTMP_SERVER_H_
#define _RTMP_SERVER_H_ 1

#include <boost/cstdint.hpp>
#include <string>
#include <map>

#include "rtmp.h"
#include "amf.h"
#include "element.h"
#include "handler.h"
#include "network.h"

namespace gnash
{
  
  class DSOEXPORT RTMPServer : public RTMPproto
{
public:
    RTMPServer();
    ~RTMPServer();
    bool handShakeWait();
    bool handShakeResponse();
    bool serverFinish();
    bool packetSend(Buffer *buf);
    bool packetRead(Buffer *buf);

    // These process the incoming RTMP message content types from the header
    gnash::Network::byte_t *decodeChunkSize(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeBytesRead(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodePing(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeServer(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeClient(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeAudioData(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeVideoData(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeNotify(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeSharedObj(gnash::Network::byte_t *buf);
    gnash::Network::byte_t *decodeInvoke(gnash::Network::byte_t *buf);
    void dump();
  private:
};

// This is the thread for all incoming RTMP connections
void rtmp_handler(Handler::thread_params_t *args);

} // end of gnash namespace
// end of _RTMP_SERVER_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

