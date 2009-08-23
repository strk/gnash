// Red5 server side support for the oflaDemo_test via RTMP
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

#ifndef _FITCDEMO_H_
#define _FITCDEMO_H_

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
#include "cygnal.h"

// cygnal headers
#include "rtmp_server.h"

namespace cygnal
{

    /// \var class demoService
    ///    This class support the Red5 server functions used by some
    ///    of their demos.
class demoService {
public:    
    demoService();
    ~demoService();

    /// return the list of FLV files we've found
    std::vector<std::string> &getListOfAvailableFiles(const std::string &path);
    /// return the list of FLV files we've found of the specified type
    std::vector<std::string> &getListOfAvailableFiles(const std::string &path,
						     const std::string &type);

private:
    std::string _path;
    std::vector<std::string> _media;
};
    
class OflaDemoTest : public cygnal::RTMPServer
{
public:
    OflaDemoTest ();
    ~OflaDemoTest ();
  
    // Parse an OflaDemo Request message coming from the Red5 oflaDemo_test.
    std::vector<boost::shared_ptr<amf::Element > > parseOflaDemoRequest(amf::Buffer &buf)
        { return parseOflaDemoRequest(buf.reference(), buf.size()); };
    std::vector<boost::shared_ptr<amf::Element > > parseOflaDemoRequest(boost::uint8_t *buf, size_t size);
    
    // format a response to the 'oflaDemo' test used for testing Gnash.
    boost::shared_ptr<amf::Buffer> formatOflaDemoResponse(double num, amf::Element &el);
    boost::shared_ptr<amf::Buffer> formatOflaDemoResponse(double num, amf::Buffer &data);
    boost::shared_ptr<amf::Buffer> formatOflaDemoResponse(double num, boost::uint8_t *data, size_t size);

    boost::shared_ptr<amf::Buffer> getResponse() { return _response; };
    void setResponse(boost::shared_ptr<amf::Buffer> &x) { _response = x; };

private:
    boost::shared_ptr<amf::Buffer> _response;    
}; 

extern "C" {
    boost::shared_ptr<Handler::cygnal_init_t> oflaDemo_class_init(); 
    // the standard API
    size_t oflaDemo_read_func(boost::uint8_t *data, size_t size);
    size_t oflaDemo_write_func(boost::uint8_t *data, size_t size);
}

} // end of cygnal namespace
#endif  // end of __FITCDEMO_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
