// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#define _RTMP_SERVER_H_

#include <vector>
#include <cstdint>
#include <boost/array.hpp>
#include <string>
#include <map>

#include "rtmp.h"
#include "amf.h"
#include "handler.h"
#include "network.h"
#include "buffer.h"
#include "diskstream.h"
#include "rtmp_msg.h"
#include "dsodefs.h"

namespace cygnal
{

// Define this if you want to use numeric client ID. Undefining this
// create ASCII based client IDs instead.
// #define CLIENT_ID_NUMERIC 1

class RTMPServer : public gnash::RTMP
{
public:
    RTMPServer();
    ~RTMPServer();

    /// \method 
    ///     This method is called after the initial network connection
    ///     is established. It reads in the handshake from the client,
    ///     responds appropriately, and then extracts the initial AMF
    ///     object, which is always of type NetConnection, doing an
    ///     INVOKE operation of ::connect(). serverFinish() is
    ///     actually used to extract the AMF data from the packet, and
    ///     handShakeResponse() is used to construct the response packet.
    std::shared_ptr<cygnal::Element> processClientHandShake(int fd);

    bool packetSend(cygnal::Buffer &buf);
    bool packetRead(cygnal::Buffer &buf);
    
    // These are handlers for the various types
    std::shared_ptr<cygnal::Buffer> encodeResult(gnash::RTMPMsg::rtmp_status_e status);
    std::shared_ptr<cygnal::Buffer> encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename);
    std::shared_ptr<cygnal::Buffer> encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename, double &transid);
    std::shared_ptr<cygnal::Buffer> encodeResult(gnash::RTMPMsg::rtmp_status_e status, double &transid);
    std::shared_ptr<cygnal::Buffer> encodeResult(gnash::RTMPMsg::rtmp_status_e status, const std::string &filename, double &transid, double &clientid);

    // Encode a Ping for the client
    std::shared_ptr<cygnal::Buffer> encodePing(rtmp_ping_e type, std::uint32_t milliseconds);
    std::shared_ptr<cygnal::Buffer> encodePing(rtmp_ping_e type);
    // std::shared_ptr<cygnal::Buffer> encodeUser(user_control_e type, std::uint32_t milliseconds);
    std::shared_ptr<cygnal::Buffer> encodeAudio(std::uint8_t *data, size_t size);
    std::shared_ptr<cygnal::Buffer> encodeVideo(std::uint8_t *data, size_t size);

    // Encode a onBWDone message for the client
    std::shared_ptr<cygnal::Buffer> encodeBWDone(double id);

    // Parse an Echo Request message coming from the Red5 echo_test.
    std::vector<std::shared_ptr<cygnal::Element > > parseEchoRequest(cygnal::Buffer &buf) { return parseEchoRequest(buf.reference(), buf.size()); };
    std::vector<std::shared_ptr<cygnal::Element > > parseEchoRequest(std::uint8_t *buf, size_t size);
    // format a response to the 'echo' test used for testing Gnash.
    std::shared_ptr<cygnal::Buffer> formatEchoResponse(double num, cygnal::Element &el);
    std::shared_ptr<cygnal::Buffer> formatEchoResponse(double num, cygnal::Buffer &data);
    std::shared_ptr<cygnal::Buffer> formatEchoResponse(double num, std::uint8_t *data, size_t size);
    void addReference(std::uint16_t index, cygnal::Element &el) { _references[index] = el; };
    cygnal::Element &getReference(std::uint16_t index) { return _references[index]; };

    bool sendFile(int fd, const std::string &filespec);

    // Create a new client ID
#ifdef CLIENT_ID_NUMERIC
    double createClientID();
#else
    std::string createClientID();
#endif

    // Create a new stream ID, which is simply an incrementing counter.
    double createStreamID();

    void setStreamID(double id) { _streamid = id; };
    double getStreamID() { return _streamid; };

    size_t sendToClient(std::vector<int> &fds, std::uint8_t *data,
			size_t size);
    size_t sendToClient(std::vector<int> &fds,cygnal::Buffer &data);

    void setNetConnection(gnash::RTMPMsg *msg) { _netconnect.reset(msg); };
    void setNetConnection(std::shared_ptr<gnash::RTMPMsg> msg) { _netconnect = msg; };
    std::shared_ptr<gnash::RTMPMsg> getNetConnection() { return _netconnect;};
    void dump();

private:
    /// \method serverFinish
    ///     This is only called by processClientHandshake() to compare
    ///     the handshakes to make sure they match, and to extract the
    ///     initial AMF data from packet.
    std::shared_ptr<cygnal::Buffer> serverFinish(int fd,
			cygnal::Buffer &handshake1, cygnal::Buffer &handshake2);
    /// \method handShakeResponse
    ///     This is only called by processClientHandshake() to
    ///     construct the handshake response to the client.
    bool handShakeResponse(int fd, cygnal::Buffer &buf);
    
    /// This is used by the boost tokenizer functions, and is defined
    /// here purely for convienience.
    typedef boost::char_separator<char> Sep;
    typedef boost::tokenizer<Sep> Tok;


    gnash::DiskStream::filetype_e  _filetype;
    std::string		_docroot;
    std::string		_filespec;
    std::uint32_t     _filesize;
    std::map<std::uint16_t, cygnal::Element> _references;
#ifdef CLIENT_ID_NUMERIC
    std::array<double>	_clientids;
#else
    boost::array<std::string, 1000>	_clientids;
#endif
    double		_streamid;
    /// \var _netconnect
    ///    This store the data from the NetConnection ActionScript
    ///    object we get as the final part of the handshake process
    ///    that is used to set up the connection. This has all the
    ///    file paths and other information needed by the server.
    std::shared_ptr<gnash::RTMPMsg>	_netconnect;
};

// This is the thread for all incoming RTMP connections
bool DSOEXPORT rtmp_handler(gnash::Network::thread_params_t *args);

} // end of gnash namespace
// end of _RTMP_SERVER_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

