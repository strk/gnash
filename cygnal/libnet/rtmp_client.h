// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef _RTMP_CLIENT_H_
#define _RTMP_CLIENT_H_

#include <deque>
#include <boost/cstdint.hpp>
#include <string>
#include <time.h>

#include "rtmp.h"
#include "rtmp_msg.h"
#include "amf.h"
#include "element.h"
// #include "handler.h"
#include "network.h"
#include "buffer.h"
#include "dsodefs.h"
#include "URL.h"

namespace gnash
{

class DSOEXPORT RTMPClient : public RTMP
{
public:
    DSOEXPORT RTMPClient();
    DSOEXPORT ~RTMPClient();

    bool handShakeWait();
//    bool handShakeResponse();
    std::shared_ptr<cygnal::Buffer> clientFinish();
    DSOEXPORT  std::shared_ptr<cygnal::Buffer> clientFinish(cygnal::Buffer &data);
    DSOEXPORT std::shared_ptr<cygnal::Buffer> handShakeRequest();
    
    // These are used for creating the primary objects
    // Create the initial object sent to the server, which
    // is NetConnection::connect()
    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeConnect();
    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeConnect(const char *uri);
    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeConnect(const char *uri, double audioCodecs, double videoCodecs,
		   double videoFunction);
    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeConnect(const char *app,
		   const char *swfUrl, const char *tcUrl,
                   double audioCodecs, double videoCodecs, double videoFunction,
                    const char *pageUrl);
    
    DSOEXPORT bool connectToServer(const std::string &url);

    // Create the second object sent to the server, which is NetStream():;NetStream()
    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeStream(double id);
    std::shared_ptr<cygnal::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag);
    std::shared_ptr<cygnal::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag, double pos);
    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name);
    std::shared_ptr<cygnal::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name, double pos);

    bool isConnected() { return _connected; };

    std::string &getPath() { return _path; };
    void setPath(std::string &x) { _path = x; };

    DSOEXPORT std::shared_ptr<cygnal::Buffer> encodeEchoRequest(const std::string &method, double id, cygnal::Element &el);

    typedef std::deque<std::shared_ptr<RTMPMsg> > msgque_t;
    msgque_t recvResponse();

    void dump();
  private:
    std::string _path;
    bool   _connected;
    double _connections;
};

#if 0
// This is the thread for all incoming RTMP connections
void rtmp_handler(Network::thread_params_t *args);
#endif

} // end of gnash namespace
// end of _RTMP_CLIENT_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

