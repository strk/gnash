// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
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
//

#include "log.h"
#include "GnashException.h"
#include "buffer.h"
#include "amf.h"
#include "amf_msg.h"
#include "element.h"
#include "network.h"
#include "GnashSystemNetHeaders.h"

#include <string>
#include <cstdint> // For C99 int types

using gnash::GnashException;
using gnash::log_error;
using std::cout;
using std::endl;


namespace cygnal
{

std::shared_ptr<cygnal::Buffer>
AMF_msg::encodeContextHeader(std::uint16_t version, std::uint16_t headers,
			     std::uint16_t messages)
{
//    GNASH_REPORT_FUNCTION;
    size_t size = sizeof(AMF_msg::context_header_t);
    std::shared_ptr<cygnal::Buffer> buf (new cygnal::Buffer(size));

    // use a short as a temporary, as it turns out htons() returns a 32bit int
    // instead when compiling with -O2. This forces appending bytes to get the
    // right size.
    std::uint16_t swapped = htons(version);
    *buf = swapped;
    swapped = htons(headers);
    *buf += swapped;
    swapped = htons(messages);
    *buf += swapped;
        
    return buf;
}

std::shared_ptr<cygnal::Buffer>
AMF_msg::encodeContextHeader(AMF_msg::context_header_t *head)
{
//    GNASH_REPORT_FUNCTION;
    return encodeContextHeader(head->version, head->headers, head->messages);
}

//  example message header::
//  00 06 67 65 74 77 61 79                <- getway, message #1
//  00 04 2f 32 32 39                      <- /229, operation name
//  00 00 00 0e				   <- byte length of message
std::shared_ptr<cygnal::Buffer>
AMF_msg::encodeMsgHeader(AMF_msg::message_header_t *head)
{
//    GNASH_REPORT_FUNCTION;
    // The size of the buffer are the two strings, their lenght fields, and the integer.
//     size_t size = head->target.size() + head->response.size() + sizeof(std::uint32_t)
//         + (sizeof(std::uint16_t) * 2);
    std::shared_ptr<cygnal::Buffer> buf (new cygnal::Buffer(sizeof(AMF_msg::message_header_t)));

    // Encode the target URI, which usually looks something like ."getway"
    std::uint16_t length = head->target.size();
    *buf = length;
    *buf += head->target;

    // Encode the response URI, which usually looks something like "/229"
    length = head->response.size();
    *buf += length;
    *buf += head->target;

    // Encode the size of the encoded message
    *buf += static_cast<std::uint32_t>(head->size);
    
    return buf;
}

// These methods parse the raw data of the AMF packet
std::shared_ptr<AMF_msg::context_header_t>
AMF_msg::parseContextHeader(cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return parseContextHeader(data.reference(), data.size());
}

std::shared_ptr<AMF_msg::context_header_t>
AMF_msg::parseContextHeader(std::uint8_t *data, size_t /* size */)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<AMF_msg::context_header_t> msg (new AMF_msg::context_header_t);

    std::uint16_t tmpnum = *reinterpret_cast<std::uint16_t *>(data);
    msg->version  = tmpnum;
    tmpnum = *reinterpret_cast<std::uint16_t *>(data + sizeof(std::uint16_t));
    msg->headers   = ntohs(tmpnum);
    tmpnum = *reinterpret_cast<std::uint16_t *>(data + sizeof(std::uint32_t));
    msg->messages = ntohs(tmpnum);

    return msg;
}

std::shared_ptr<AMF_msg::message_header_t>
AMF_msg::parseMessageHeader(cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return parseMessageHeader(data.reference(), data.size());
}

std::shared_ptr<AMF_msg::message_header_t>
AMF_msg::parseMessageHeader(std::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    AMF amf;
    std::uint8_t *tmpptr = data;
    std::shared_ptr<AMF_msg::message_header_t> msg (new AMF_msg::message_header_t);

    // The target is a standard length->bytes field
    std::uint16_t length = ntohs((*(std::uint16_t *)tmpptr) & 0xffff);
    if (length == 0) {
        boost::format msg("Length of string shouldn't be zero! amf_msg.cpp::%1%(): %2%");
        msg % __FUNCTION__ % __LINE__;
        throw GnashException(msg.str());
    }
    tmpptr += sizeof(std::uint16_t);
    std::string str1(reinterpret_cast<const char *>(tmpptr), length);
    msg->target = str1;
    if ((tmpptr - data) > static_cast<int>(size)) {
        boost::format msg("Trying to read past the end of data! Wants %1% bytes, given %2% bytes");
        msg % length % size;
        throw GnashException(msg.str());
    } else {
        tmpptr += length;
    }
    
    // The response is a standard length->bytes field
    length = ntohs((*(std::uint16_t *)tmpptr) & 0xffff);
    if (length == 0) {
        boost::format msg("Length of string shouldn't be zero! amf_msg.cpp::%1%(): %2%");
        msg % __FUNCTION__ % __LINE__;
        throw GnashException(msg.str());
    }
    tmpptr += sizeof(std::uint16_t);
    std::string str2(reinterpret_cast<const char *>(tmpptr), length);
    msg->response = str2;
    tmpptr += length;
    if ((tmpptr - data) > static_cast<int>(size)) {
        boost::format msg("Trying to read past the end of data! Wants %1% bytes, given %2% bytes");
        msg % length % size;
        throw GnashException(msg.str());
    }    

    // The length is a 4 word integer
    msg->size = ntohl((*(std::uint32_t *)tmpptr));

    if (msg->target.empty()) {
        log_error(_("AMF Message \'target\' field missing!"));
    }
    if (msg->response.empty()) {
        log_error(_("AMF Message \'reply\' field missing!"));
    }
    if (msg->size == 0) {
        log_error(_("AMF Message \'size\' field missing!"));
    } else {
        msg->size = size;
    }

//    AMF_msg::dump(*msg);
    return msg;
}

std::shared_ptr<AMF_msg::context_header_t>
AMF_msg::parseAMFPacket(cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return parseAMFPacket(data.reference(), data.size());
}

std::shared_ptr<AMF_msg::context_header_t>
AMF_msg::parseAMFPacket(std::uint8_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
//    _messages.push_back();
    std::uint8_t *ptr = data + sizeof(AMF_msg::context_header_t);
    std::shared_ptr<context_header_t> header = AMF_msg::parseContextHeader(data, size);

//     log_debug("%s: %s", __PRETTY_FUNCTION__, hexify(data, size, true));
    
    AMF amf;
    /// Read all the messages from the AMF packet
    try {
        for (int i=0; i<header->messages; i++) {
            std::shared_ptr<AMF_msg::amf_message_t> msgpkt(new AMF_msg::amf_message_t);
            std::shared_ptr<AMF_msg::message_header_t> msghead = AMF_msg::parseMessageHeader(ptr, size);
            if (msghead) {
                ptr += msghead->target.size() + msghead->response.size()
                    + (sizeof(std::uint16_t) * 2)
                    + (sizeof(std::uint32_t));
                std::shared_ptr<cygnal::Element> el = amf.extractAMF(ptr, ptr+size);
                msgpkt->header.target = msghead->target;
                msgpkt->header.response = msghead->response;
                msgpkt->header.size = msghead->size;
                msgpkt->data = el;
                ptr += amf.totalsize();
                
                _messages.push_back(msgpkt);
            }
        }
    } catch(std::exception& e) {
        log_error(_("Error parsing the AMF packet: \n\t%s"), e.what());
    }
        
    return header;
}

std::shared_ptr<cygnal::Buffer>
AMF_msg::encodeAMFPacket(const std::string & /* target */,
                         const std::string & /*response */, size_t /* size */)
{
//    GNASH_REPORT_FUNCTION;

    return encodeAMFPacket();
}

std::shared_ptr<cygnal::Buffer>
AMF_msg::encodeAMFPacket()
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer);

    // Encode the packet header
    std::shared_ptr<cygnal::Buffer> buf1 = encodeContextHeader(0, 0, _messages.size());
    *buf = buf1;

    // Now encode all the messages

    std::vector<std::shared_ptr<AMF_msg::amf_message_t> >::iterator it;
    for (it = _messages.begin(); it != _messages.end(); ++it) {
        std::shared_ptr<AMF_msg::amf_message_t> msg = (*(it));

        std::shared_ptr<cygnal::Buffer> buf2 = encodeMsgHeader(msg->header.target,
							     msg->header.response,
							     msg->header.size);

// 	AMF_msg::dump(msg->header);
// 	msg->data->dump();
        std::shared_ptr<cygnal::Buffer> buf3 = msg->data->encode();
	*buf += buf2;
	*buf += buf3;
    }

    return buf;
}

std::shared_ptr<cygnal::Buffer>
AMF_msg::encodeMsgHeader(const std::string &target,
                         const std::string &response, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    size_t total = target.size() + sizeof(std::uint16_t);
    total += response.size() + sizeof(std::uint16_t);
    total += sizeof(std::uint32_t);
    
    std::shared_ptr<cygnal::Buffer> buf (new cygnal::Buffer(total));
    std::uint16_t length = target.size();
    swapBytes(&length, sizeof(std::uint16_t));
    *buf += length;
    *buf += target;

    length = response.size();
    swapBytes(&length, sizeof(std::uint16_t));
    *buf += length;
    *buf += response;

    std::uint32_t swapped = htonl(size);
    *buf += swapped;
    
    return buf;
}    

void
AMF_msg::dump(AMF_msg::message_header_t &data)
{
//    GNASH_REPORT_FUNCTION;
    cout << "Target is: " << data.target << endl;
    cout << "Response is: " << data.response << endl;
    cout << "Data size is: " << data.size << endl;
}

void
AMF_msg::dump(AMF_msg::context_header_t &data)
{
//    GNASH_REPORT_FUNCTION;
    cout << "AMF Version: " << data.version << endl;
    cout << "Number of headers: " << data.headers << endl;
    cout << "Number of messages: " << data.messages << endl;
}

void
AMF_msg::dump()
{
//    GNASH_REPORT_FUNCTION;
    cout << "AMF Packet has " << _messages.size() << " messages." << endl;
    std::vector<std::shared_ptr<AMF_msg::amf_message_t> >::iterator it;
    for (it = _messages.begin(); it != _messages.end(); ++it) {
        std::shared_ptr<AMF_msg::amf_message_t> msg = (*(it));
        AMF_msg::dump(msg->header);
        msg->data->dump();
    }
}


} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
