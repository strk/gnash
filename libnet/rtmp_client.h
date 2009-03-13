// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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
#define _RTMP_CLIENT_H_ 1

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <time.h>

#include "rtmp.h"
#include "amf.h"
#include "element.h"
#include "handler.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{

class DSOEXPORT RTMPClient : public RTMP
{
public:
    RTMPClient();
    ~RTMPClient();

    bool handShakeWait();
//    bool handShakeResponse();
    bool clientFinish();
    bool handShakeRequest();
    
    // These are used for creating the primary objects
    // Create the initial object sent to the server, which is NetConnection::connect()
    boost::shared_ptr<amf::Buffer> encodeConnect(const char *app, const char *swfUrl, const char *tcUrl,
                                   double audioCodecs, double videoCodecs, double videoFunction,
                                   const char *pageUrl);
    // Create the second object sent to the server, which is NetStream():;NetStream()
    boost::shared_ptr<amf::Buffer> encodeStream(double id);
    boost::shared_ptr<amf::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag);
    boost::shared_ptr<amf::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag, double pos);
    boost::shared_ptr<amf::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name);
    boost::shared_ptr<amf::Buffer> encodeStreamOp(double id, rtmp_op_e op, bool flag, const std::string &name, double pos);

    bool isConnected() { return _connected; };

    std::string &getPath() { return _path; };
    void setPath(std::string &x) { _path = x; };

    void dump();
  private:
    std::string _path;
    bool   _connected;
    double _connections;
};

// This is the thread for all incoming RTMP connections
void rtmp_handler(Network::thread_params_t *args);

} // end of gnash namespace
// end of _RTMP_CLIENT_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

