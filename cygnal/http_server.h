// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <string>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <sstream>

#include "amf.h"
#include "cque.h"
#include "rtmp.h"
#include "http.h"
#include "handler.h"
#include "network.h"
#include "buffer.h"
#include "diskstream.h"
#include "dsodefs.h"

namespace cygnal
{
    
class DSOEXPORT HTTPServer : public gnash::HTTP
{
public:
    HTTPServer();
    ~HTTPServer();

    // These are for the protocol itself
    http_method_e processClientRequest(int fd);
    bool processGetRequest(int fd);
    bool processPostRequest(int fd);
    bool processPutRequest(int fd);
    bool processDeleteRequest(int fd);
    bool processConnectRequest(int fd);
    bool processOptionsRequest(int fd);
    bool processHeadRequest(int fd);
    bool processTraceRequest(int fd);

    // Handle the response for the request.
    boost::shared_ptr<amf::Buffer> formatServerReply(http_status_e code);
    amf::Buffer &formatGetReply(gnash::DiskStream::filetype_e type, size_t size, http_status_e code); 
    amf::Buffer &formatGetReply(size_t size, http_status_e code); 
    amf::Buffer &formatGetReply(http_status_e code); 
    amf::Buffer &formatPostReply(rtmpt_cmd_e code);
    amf::Buffer &formatErrorResponse(http_status_e err);

    // These methods extract data from an RTMPT message. RTMP is an
    // extension to HTTP that adds commands to manipulate the
    // connection's persistance.
    rtmpt_cmd_e extractRTMPT(boost::uint8_t *data);
    rtmpt_cmd_e extractRTMPT(amf::Buffer &data)
	{ return extractRTMPT(data.reference()); };    

    // Examine the beginning of the data for an HTTP request command
    // like GET or POST, etc...
    http_method_e extractCommand(boost::uint8_t *data);
    http_method_e extractCommand(amf::Buffer &data)
	{ return extractCommand(data.reference()); };    
    
    // process all the header fields in the Buffer, storing them internally
    // in _fields. The address returned is the address where the Content data
    // starts, and is "Content-Length" bytes long, of "Content-Type" data.
    boost::uint8_t *processHeaderFields(amf::Buffer &buf);

#if 0
    // Parse an Echo Request message coming from the Red5 echo_test.
    std::vector<boost::shared_ptr<amf::Element > > parseEchoRequest(gnash::amf::Buffer &buf) { return parseEchoRequest(buf.reference(), buf.size()); };
    std::vector<boost::shared_ptr<amf::Element > > parseEchoRequest(boost::uint8_t *buf, size_t size);
    
    // format a response to the 'echo' test used for testing Gnash.
    gnash::amf::Buffer &formatEchoResponse(const std::string &num, amf::Element &el);
    gnash::amf::Buffer &formatEchoResponse(const std::string &num, amf::Buffer &data);
    gnash::amf::Buffer &formatEchoResponse(const std::string &num, uint8_t *data, size_t size);
#endif

    void dump();
    
private:
    
};

// This is the thread for all incoming HTTP connections
extern "C" {
    bool DSOEXPORT http_handler(gnash::Network::thread_params_t *args);
}

} // end of gnash namespace

// end of _HTTP_SERVER_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
