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

// This file is for the low level support for encoding and decoding AMF objects.
// As this class has no data associated with it, all the methods are static as
// they are for convenience only.
// All the encoding methods return a Buffer class, which is simply an array on
// of unsigned bytes, and a byte count.
// The only extraction classes parse either a raw AMF object or the larger
// "variable"

#ifndef _AMF_MSG_H_
#define _AMF_MSG_H_

#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include "element.h"
#include "dsodefs.h"

/// Action Message Format specific classes of libamf.
namespace cygnal
{

// forward declaration
class Buffer;
class Element;

/// All numbers in AMF format are 8 byte doubles.
/// Binary representation of an ActionScript object.
//
/// AMF is used to send objects, whether to a SharedObject .sol file,
/// a memory based LocalConnection segment, or over an RTMP connection
/// for streaming.
///
class DSOEXPORT AMF_msg {
  public:
    typedef enum {
        AMF0 = 0x00,
        AMF3 = 0x11
    } amf_version_e;
    typedef struct {
        boost::uint16_t version;
        boost::uint16_t headers;
        boost::uint16_t messages;
    } context_header_t;
    typedef struct {
        std::string     target;
        std::string     response;
        size_t          size;
    } message_header_t;
    typedef struct {
        message_header_t header;
        std::shared_ptr<cygnal::Element> data;
    } amf_message_t;

    size_t addMessage(std::shared_ptr<amf_message_t> msg)
    {
        _messages.push_back(msg); return _messages.size();
    };
    std::shared_ptr<amf_message_t> &getMessage(int x) { return _messages[x]; };
    size_t messageCount() { return _messages.size(); };
    
    // These methods create the raw data of the AMF packet from Elements
    static std::shared_ptr<cygnal::Buffer> encodeContextHeader(context_header_t *head);
    static std::shared_ptr<cygnal::Buffer> encodeContextHeader(boost::uint16_t version,
							      boost::uint16_t headers,
							      boost::uint16_t messages);

    static std::shared_ptr<cygnal::Buffer> encodeMsgHeader(message_header_t *head);
    static std::shared_ptr<cygnal::Buffer> encodeMsgHeader(const std::string &target,
                                          const std::string &response, size_t size);
    
    // These methods parse the raw data of the AMF packet into data structures
    static std::shared_ptr<context_header_t> parseContextHeader(cygnal::Buffer &data);
    static std::shared_ptr<context_header_t> parseContextHeader(boost::uint8_t *data, size_t size);
    
    static std::shared_ptr<message_header_t> parseMessageHeader(cygnal::Buffer &data);
    static std::shared_ptr<message_header_t> parseMessageHeader(boost::uint8_t *data, size_t size);

    // These methods parse the entire packet. which consists of multiple messages
    std::shared_ptr<context_header_t> parseAMFPacket(cygnal::Buffer &buf);
    std::shared_ptr<context_header_t> parseAMFPacket(boost::uint8_t *data,
						       size_t size);

    // This methods create an entire packet from multiple messages, already parsed in
    std::shared_ptr<cygnal::Buffer> encodeAMFPacket();
    std::shared_ptr<cygnal::Buffer> encodeAMFPacket(const std::string &target,
				     const std::string &response, size_t size);
    
    static void dump(context_header_t &data);
    static void dump(message_header_t &data);
    void dump();
    
private:
    std::vector<std::shared_ptr<amf_message_t> > _messages;
//     context_header_t    _context_header;
};

} // end of amf namespace

// end of _AMF_MSG_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
