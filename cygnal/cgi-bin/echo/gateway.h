// Red5 server side support for the echo_test via HTTP
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

#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <string>
#include <vector>
#include <boost/shared_array.hpp>
#include <sstream>

#include "amf.h"
#include "buffer.h"
#include "element.h"
#include "rtmp.h"

#include "http.h"

namespace cygnal
{
  
class GatewayTest : public gnash::HTTP
{
public:
    GatewayTest ();
    ~GatewayTest ();

    // Parse an Echo Request message coming from the Red5 echo_test.
    std::vector<std::shared_ptr<amf::Element > > parseEchoRequest(amf::Buffer &buf)
        { return parseEchoRequest(buf.reference(), buf.size()); };
    std::vector<std::shared_ptr<amf::Element > > parseEchoRequest(boost::uint8_t *buf, size_t size);

    // format a response to the 'echo' test used for testing Gnash.
    amf::Buffer &formatEchoResponse(const std::string &num, amf::Element &el);
    amf::Buffer &formatEchoResponse(const std::string &num, amf::Buffer &data);
    amf::Buffer &formatEchoResponse(const std::string &num, boost::uint8_t *data, size_t size);
private:
};

} // end of cygnal namespace
#endif  // end of __GATEWAY_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
