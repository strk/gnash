// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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
#include <vector>
#include <sstream>

#include "amf.h"
#include "cque.h"
#include "rtmp.h"
#include "http.h"
#include "handler.h"
#include "network.h"
#include "buffer.h"
#include "diskstream.h"

namespace cygnal
{
    
class DSOEXPORT HTTPServer : public gnash::HTTP
{
public:
    HTTPServer();
    ~HTTPServer();

    // These are for the protocol itself
    http_method_e processClientRequest(int fd);
    http_method_e processClientRequest(Handler *hand, int fd, cygnal::Buffer *buf);
    cygnal::Buffer &processGetRequest(Handler *hand, int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processPostRequest(int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processPutRequest(int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processDeleteRequest(int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processConnectRequest(int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processOptionsRequest(int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processHeadRequest(int fd, cygnal::Buffer *buf);
    std::shared_ptr<cygnal::Buffer> processTraceRequest(int fd, cygnal::Buffer *buf);

    // Handle the response for the request.
    std::shared_ptr<cygnal::Buffer> formatServerReply(http_status_e code);
    cygnal::Buffer &formatGetReply(gnash::DiskStream::filetype_e type, size_t size, http_status_e code); 
    cygnal::Buffer &formatGetReply(size_t size, http_status_e code); 
    cygnal::Buffer &formatGetReply(http_status_e code); 
    cygnal::Buffer &formatPostReply(rtmpt_cmd_e code);
    cygnal::Buffer &formatErrorResponse(http_status_e err);

    // These methods extract data from an RTMPT message. RTMP is an
    // extension to HTTP that adds commands to manipulate the
    // connection's persistance.
    rtmpt_cmd_e extractRTMPT(boost::uint8_t *data);
    rtmpt_cmd_e extractRTMPT(cygnal::Buffer &data)
	{ return extractRTMPT(data.reference()); };    

#if 0
    // Examine the beginning of the data for an HTTP request command
    // like GET or POST, etc...
    http_method_e extractCommand(boost::uint8_t *data);
    http_method_e extractCommand(cygnal::Buffer &data)
	{ return extractCommand(data.reference()); };    

    // process all the header fields in the Buffer, storing them internally
    // in _fields. The address returned is the address where the Content data
    // starts, and is "Content-Length" bytes long, of "Content-Type" data.
    boost::uint8_t *processHeaderFields(cygnal::Buffer &buf);
#endif
    
#if 0
    // Parse an Echo Request message coming from the Red5 echo_test.
    std::vector<std::shared_ptr<cygnal::Element > > parseEchoRequest(gnash::cygnal::Buffer &buf) { return parseEchoRequest(buf.reference(), buf.size()); };
    std::vector<std::shared_ptr<cygnal::Element > > parseEchoRequest(boost::uint8_t *buf, size_t size);
    
    // format a response to the 'echo' test used for testing Gnash.
    gnash::cygnal::Buffer &formatEchoResponse(const std::string &num, cygnal::Element &el);
    gnash::cygnal::Buffer &formatEchoResponse(const std::string &num, cygnal::Buffer &data);
    gnash::cygnal::Buffer &formatEchoResponse(const std::string &num, uint8_t *data, size_t size);
#endif

    bool http_handler(Handler *hand, int netfd, cygnal::Buffer *buf);
    std::shared_ptr<gnash::DiskStream> getDiskStream() { return _diskstream; };

    void dump();    
private:
    cygnal::Buffer _buf;
    std::shared_ptr<gnash::DiskStream> _diskstream;
};

} // end of gnash namespace

// end of _HTTP_SERVER_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
