// Red5 server side support for the echo_test via RTMP
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

#ifndef _ECHO_H_
#define _ECHO_H_

#include <string>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <sstream>

// gnash headers
#include "amf.h"
#include "buffer.h"
#include "element.h"
#include "http.h"

// cygnal headers
#include "rtmp_server.h"

namespace cygnal
{
    
class EchoTest : public cygnal::RTMPServer
{
public:
    EchoTest ();
    ~EchoTest ();
  
    // Parse an Echo Request message coming from the Red5 echo_test.
    std::vector<boost::shared_ptr<amf::Element > > parseEchoRequest(amf::Buffer &buf)
        { return parseEchoRequest(buf.reference(), buf.size()); };
    std::vector<boost::shared_ptr<amf::Element > > parseEchoRequest(boost::uint8_t *buf, size_t size);
    
    // format a response to the 'echo' test used for testing Gnash.
    boost::shared_ptr<amf::Buffer> formatEchoResponse(double num, amf::Element &el);
    boost::shared_ptr<amf::Buffer> formatEchoResponse(double num, amf::Buffer &data);
    boost::shared_ptr<amf::Buffer> formatEchoResponse(double num, boost::uint8_t *data, size_t size);
private:
    
};  

extern "C" {
    void echo_class_init(); 
    /// Return an  instance
}

} // end of cygnal namespace
#endif  // end of __ECHO_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
