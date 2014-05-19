// Red5 server side support for the fitcDemo_test via RTMP
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

#ifndef _FITCDEMO_H_
#define _FITCDEMO_H_

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
#include "cygnal.h"

// cygnal headers
#include "rtmp_server.h"

namespace cygnal
{
    
class FitcDemoTest : public cygnal::RTMPServer
{
public:
    FitcDemoTest ();
    ~FitcDemoTest ();
  
    // Parse an FitcDemo Request message coming from the Red5 fitcDemo_test.
    std::vector<std::shared_ptr<amf::Element > > parseFitcDemoRequest(amf::Buffer &buf)
        { return parseFitcDemoRequest(buf.reference(), buf.size()); };
    std::vector<std::shared_ptr<amf::Element > > parseFitcDemoRequest(boost::uint8_t *buf, size_t size);
    
    // format a response to the 'fitcDemo' test used for testing Gnash.
    std::shared_ptr<amf::Buffer> formatFitcDemoResponse(double num, amf::Element &el);
    std::shared_ptr<amf::Buffer> formatFitcDemoResponse(double num, amf::Buffer &data);
    std::shared_ptr<amf::Buffer> formatFitcDemoResponse(double num, boost::uint8_t *data, size_t size);

    std::shared_ptr<amf::Buffer> getResponse() { return _response; };
    void setResponse(std::shared_ptr<amf::Buffer> &x) { _response = x; };

private:
    std::shared_ptr<amf::Buffer> _response;
};  

extern "C" {
    std::shared_ptr<Handler::cygnal_init_t> fitcDemo_class_init();
    // the standard API
    size_t fitcDemo_read_func(boost::uint8_t *data, size_t size);
    size_t fitcDemo_write_func(boost::uint8_t *data, size_t size);
}

} // end of cygnal namespace
#endif  // end of __FITCDEMO_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
