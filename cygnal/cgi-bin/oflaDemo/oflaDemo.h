// Red5 server side support for the oflaDemo_test via RTMP
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

#include <string>
#include <vector>
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

    typedef struct {
	std::string name;
	std::string last;
	std::string size;
    } filestats_t ;
    demoService();
    ~demoService();

    /// return the list of FLV files we've found
    std::vector<std::shared_ptr<filestats_t> > &getListOfAvailableFiles(const std::string &path);

    /// return the list of FLV files we've found of the specified type
    std::vector<std::shared_ptr<filestats_t> > &getListOfAvailableFiles(const std::string &path,
									  const std::string &type);
    std::vector<std::shared_ptr<filestats_t> > &getFileStats() { return _stats; };
    
private:
    std::string				_path;
    std::vector<std::shared_ptr<filestats_t> >	_stats;
};
    
class OflaDemoTest : public cygnal::RTMPServer
{
public:
    OflaDemoTest ();
    ~OflaDemoTest ();
  
    // Parse an OflaDemo Request message coming from the Red5 oflaDemo_test.
    std::vector<std::shared_ptr<cygnal::Element > > parseOflaDemoRequest(cygnal::Buffer &buf)
        { return parseOflaDemoRequest(buf.reference(), buf.size()); };
    std::vector<std::shared_ptr<cygnal::Element > > parseOflaDemoRequest(boost::uint8_t *buf, size_t size);
    
    // format a response to the 'oflaDemo' test used for testing Gnash.
    std::shared_ptr<cygnal::Buffer> formatOflaDemoResponse(double num, cygnal::Element &el);
    std::shared_ptr<cygnal::Buffer> formatOflaDemoResponse(double num, cygnal::Buffer &data);
    std::shared_ptr<cygnal::Buffer> formatOflaDemoResponse(double num, boost::uint8_t *data, size_t size);

    std::shared_ptr<cygnal::Buffer> getResponse() { return _response; };
    void setResponse(std::shared_ptr<cygnal::Buffer> &x) { _response = x; };
    
    void setNetConnection(gnash::RTMPMsg *msg) { _netconnect.reset(msg); };
    void setNetConnection(std::shared_ptr<gnash::RTMPMsg> msg) { _netconnect = msg; };
    std::shared_ptr<gnash::RTMPMsg> getNetConnection() { return _netconnect;};

    /// \var _netconnect
    ///    This store the data from the NetConnection ActionScript
    ///    object we get as the final part of the handshake process
    ///    that is used to set up the connection. This has all the
    ///    file paths and other information needed by the server.
    std::shared_ptr<gnash::RTMPMsg>	_netconnect;
private:
    std::shared_ptr<cygnal::Buffer> _response;
    std::shared_ptr<Handler::cygnal_init_t> _info;
}; 

// the standard API
extern "C" {
    std::shared_ptr<Handler::cygnal_init_t>oflaDemo_init_func(std::shared_ptr<gnash::RTMPMsg> &msg);
    
    std::shared_ptr<cygnal::Buffer> oflaDemo_read_func();
    size_t oflaDemo_write_func(boost::uint8_t *data, size_t size);
}

} // end of cygnal namespace
#endif  // end of __FITCDEMO_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
