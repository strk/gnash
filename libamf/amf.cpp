//
// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#if defined(_WIN32) || defined(WIN32)
#	include <Winsock2.h>
#else
#	include <netinet/in.h>
#endif

#include "log.h"
#include "tu_swap.h"
#include <cstdlib>

#include "amf.h"
#include "amfutf8.h"

using namespace std;
using namespace gnash;

namespace amf 
{

// These are used to print more intelligent debug messages
const char *astype_str[] = {
    "Number",
    "Boolean",
    "String",
    "Object",
    "MovieClip",
    "Null",
    "Undefined",
    "Reference",
    "ECMAArray",
    "ObjectEnd",
    "StrictArray",
    "Date",
    "LongString",
    "Unsupported",
    "Recordset",
    "XMLObject",
    "TypedObject"
};

// These are the textual responses
const char *response_str[] = {
    "/onStatus",
    "/onResult",
    "/onDebugEvents"
};

AMF::AMF() 
    : _type(NONE),
      _amf_index(0),
      _header_size(0),
      _total_size(0),
      _packet_size(0),
      _amf_data(0),
      _seekptr(0),
      _mystery_word(0),
      _src_dest(0)
{
    GNASH_REPORT_FUNCTION;
}

AMF::AMF(int size) 
    : _type(NONE),
      _amf_index(0),
      _header_size(0),
      _total_size(0),
      _packet_size(0),
      _amf_data(0),
      _mystery_word(0),
      _src_dest(0)
{
    GNASH_REPORT_FUNCTION;
    if (!_amf_data) {
        _amf_data = new unsigned char(size+1);
        memset(_amf_data, 0, size+1);
    }
    _seekptr = _amf_data;
}

AMF::~AMF()
{
    GNASH_REPORT_FUNCTION;
}


/// \brief Swap bytes from big to little endian.
///
/// All Numberic values for AMF files are 64bit big endian, so we have
/// to swap the bytes to be little endian for most machines. Don't do
/// anything if we happend to be on a big-endian machine.
void *
AMF::swapBytes(void *word, int size)
{
#if    __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char       c;
    unsigned short      s;
    unsigned long       l;
    char *x = static_cast<char *>(word);

    switch (size) {
      case 2: // swap two bytes
          c = *x;
          *x = *(x+1);
          *(x+1) = c;
          break;
      case 4: // swap two shorts (2-byte words)
          s = *(unsigned short *)x;
          *(unsigned short *)x = *((unsigned short *)x + 1);
          *((unsigned short *)x + 1) = s;
          swapBytes((char *)x, 2);
          swapBytes((char *)((unsigned short *)x+1), 2);
          break;
      case 8: // swap two longs (4-bytes words)
          l = *(unsigned long *)x;
          *(unsigned long *)x = *((unsigned long *)x + 1);
          *((unsigned long *)x + 1) = l;
          swapBytes((char *)x, 4);
          swapBytes((char *)((unsigned long *)x+1), 4);
          break;
    }
#endif

    return word;
}


bool
AMF::parseAMF(char *in)
{
    GNASH_REPORT_FUNCTION;

    char *x = static_cast<char *>(in);

    while (*x != ObjectEnd) {
        x = readElement(x);
    }
		return true;
}

char *
AMF::readElement(void *in)
{
    GNASH_REPORT_FUNCTION;

    char *x = static_cast<char *>(in);
    astype_e type = (astype_e)*x;
    bool boolshift;
    const char *mstr = NULL;
    amfnum_t num;
    amfnum_t nanosecs;
    short length;
    
    log_msg("Type is %s\n", astype_str[type]);

    x++;                        // skip the type byte
    switch (type) {
      case Number:
          num = *(amfnum_t *)swapBytes(x+1, 8);
          log_msg("Number is " AMFNUM_F, num);
          break;
      case Boolean:
          boolshift = *x;
          log_msg("Boolean is %d\n", boolshift);
          break;
      case String:
//        int length = *(short *)swapBytes(x, 2);
          length = *(short *)x;
          x+=2;                  // skip the length bytes
          mstr = new char[length+1];
//           memset(mstr, 0, length+1);
//           memcpy(mstr, x, length);
          // The function converts the multibyte string beginning at
          // *src to a sequence of wide characters as if by repeated
          // calls of the form:
//          mbsrtowcs
          log_msg("String is %s\n", mstr);
          break;
      case Object:
//          readElement();
          log_msg("Object is unimplemented\n");
          break;
      case MovieClip:
        log_msg("MovieClip is unimplemented\n");
          break;
      case Unsupported:
        log_msg("MovieClip is unimplemented\n");
          break;
      case Null: 
          log_msg("Null is unimplemented\n");
          break;
      case Undefined:
          log_msg("Endefined element");
          break;
      case Reference:
          log_msg("Reference is unimplemented\n");
          break;
      case ECMAArray:
          log_msg("ECMAArray is unimplemented\n");
          break;
      case ObjectEnd:
          log_msg("ObjectEnd is unimplemented\n");
          break;
      case StrictArray:
          log_msg("StrictArray is unimplemented\n");
          break;
      case Date:
          nanosecs = *(amfnum_t *)swapBytes(x+1, 8);
          log_msg("Date is " AMFNUM_F " nanoseconds\n", nanosecs);
          break;
      case LongString:
//          int length = *(short *)swapBytes(x, 4);
          x+=4;                  // skip the length bytes
//        mstr = new char[length+1];
//          memcpy(mstr, x, length);
          log_msg("String is %s\n", mstr);
          break;
//       case Unsupported:
          log_msg("Unsupported is unimplemented\n");
//           break;
      case Recordset:
          log_msg("Recordset is unimplemented\n");
          break;
      case XMLObject:
          log_msg("XMLObject is unimplemented\n");
          break;
      case TypedObject:
          log_msg("TypedObject is unimplemented\n");
          break;
    }
    
    return x;
}

bool
AMF::readObject(void */* in */)
{
    GNASH_REPORT_FUNCTION;
		return true;
    
}

/// \brief Write an AMF element
///
/// This encodes the data supplied to an AMF formatted one. As the
/// memory is allocatd within this function, you *must* free the
/// memory used for each element or you'll leak memory.
///
/// A "packet" (or element) in AMF is a byte code, followed by the
/// data. Sometimes the data is prefixed by a count, and sometimes
/// it's terminated with a 0x09. Ya gotta love these flaky ad-hoc
/// formats.
///
/// All Numbers are 64 bit, big-endian (network byte order) entities.
///
/// All strings are in multibyte format, which is to say, probably
/// normal ASCII. It may be that these need to be converted to wide
/// characters, but for now we just leave them as standard multibyte
/// characters.
void *
AMF::encodeElement(astype_e type, void *in, int nbytes)
{
    GNASH_REPORT_FUNCTION;

    char *out = NULL, *x;
    amfnum_t num;
    int pktsize;

    // Packets are of varying lenght. A few pass in a byte count, but
    // most packets have a hardcoded size.
    if (nbytes == 0) {
        switch (type) {
            // Encode the data as a 64 bit, big-endian, numeric value
          case Number:
              pktsize = 9;          // one 64 bit number
              break;
          case Boolean:
              pktsize = 2;          // just one data byte
              break;
          case String:
              pktsize = nbytes + 3; // two length bytes after the header
              break;
          case Object:
              pktsize = 0;          // look for the terminator
              break;
          case MovieClip:
              pktsize = -1;         // FIXME: no clue
              break;
          case Null: 
              pktsize = -1;         // FIXME: no clue
              break;
          case Undefined:
              pktsize = 1;          // just the header, no data
              break;
          case Reference:
              pktsize = -1;         // FIXME: no clue
              break;
          case ECMAArray:
              pktsize = 0;          // look for the terminator
              break;
          case ObjectEnd:
              pktsize = -1;         // FIXME: no clue
              break;
          case StrictArray:
              pktsize = nbytes + 5; // 4 length bytes, then data
              break;
          case Date:
              pktsize = 9;          // one 64 bit number
              break;
          case LongString:
              pktsize = nbytes + 5; // 4 length bytes, then data
              break;
          case Unsupported:
              pktsize = -1;         // FIXME: no clue
              break;
          case Recordset:
              pktsize = -1;         // FIXME: no clue
              break;
          case XMLObject:
              pktsize = nbytes + 5;// 4 length bytes, then data
              break;
          case TypedObject:
              pktsize = 0;          // look for the terminator
              break;
        };
    }
    
    switch (type) {
        // Encode the data as a 64 bit, big-endian, numeric value
      case Number:
          out = (char *)new char[pktsize];
          memset(out, 0, pktsize);
          x = out;    
          *x++ = (char)AMF::Number;
          num = *(amfnum_t *)in;
          swapBytes(&num, 8);
          memcpy(x, &num, 8);
          break;
      case Boolean:
          // Encode a boolean value. 0 for false, 1 for true
          out = (char *)new char[pktsize];
          x = out;    
          *x++ = (char)AMF::Boolean;
          *x = *(char *)in;
          break;
      case String:
          // Encode a string value. The data follows a 2 byte length
          // field. (which must be big-endian)
          out = (char *)new char[pktsize];
          memset(out, 0, pktsize);
          *out++ = String;
          num = nbytes;
          swapBytes(&num, 2);
          memcpy(out, in, nbytes);
          break;
      case Object:
          log_msg("Object unimplemented\n");
          break;
      case MovieClip:
          log_msg("MovieClip unimplemented\n");
          break;
      case Null: 
          log_msg("Null unimplemented\n");
          break;
      case Undefined:
          out = (char *)new char[pktsize];
          memset(out, 0, pktsize);
          *out++ = Undefined;
          break;
      case Reference:
          log_msg("Reference unimplemented\n");
          break;
      case ECMAArray:
          log_msg("ECMAArray unimplemented\n");
          break;
      case ObjectEnd:
          log_msg("ObjectEnd unimplemented\n");
          break;
      case StrictArray:
          log_msg("StrictArray unimplemented\n");
          break;
          // Encode the date as a 64 bit, big-endian, numeric value
      case Date:
          out = (char *)new char[pktsize];
          memset(out, 0, pktsize);
          *out++ = Date;
          num = *(amfnum_t *)in;
          swapBytes(&num, 8);
          memcpy(out, &num, 8);
          break;
      case LongString:
          log_msg("LongString unimplemented\n");
          break;
      case Unsupported:
          log_msg("Unsupported unimplemented\n");
          break;
      case Recordset:
          log_msg("Recordset unimplemented\n");
          break;
      case XMLObject:
          // Encode an XML object. The data follows a 4 byte length
          // field. (which must be big-endian)
          out = (char *)new char[pktsize];
          memset(out, 0, pktsize);
          *out++ = String;
          num = nbytes;
          swapBytes(&num, 4);
          memcpy(out, in, nbytes);
          break;
      case TypedObject:
          log_msg("TypedObject unimplemented\n");
          break;
    };
    
    return out;
}
/// \brief Each header consists of the following:
///
/// * UTF string (including length bytes) - name
/// * Boolean - specifies if understanding the header is `required'
/// * Long - Length in bytes of header
/// * Variable - Actual data (including a type code)
amfhead_t *
AMF::encodeHeader(amfutf8_t *name, bool required, int nbytes, void *data)
{
    GNASH_REPORT_FUNCTION;

    AMF_Int_t length = sizeof(amfhead_t) + nbytes + name->length + 1;
    char *buf = new char[length];
    memset(buf, 0, length);
    char *ptr = buf;

    // The first two bytes are the byte count for the UTF8 string,
    // which is in big-endian format.
    length = name->length;
    swapBytes(&length, sizeof(AMF_Int_t));
    memcpy(ptr, &length, sizeof(AMF_Int_t));
    ptr += sizeof(AMF_Int_t);

    // Now the data part of the UTF8 string
    memcpy(ptr, name->data, name->length);
    ptr += name->length;

    // Then the "required" flag, whatever this does...
    memcpy(ptr, &required, 1);
    ptr += 1;

    // Then the byte count of the data, which is an ActionScript
    // object
    length = nbytes;
    swapBytes(&length, 2);
    memcpy(ptr, &length, 2);
    ptr += 2;

    // And finally all the data
    memcpy(ptr, data, nbytes);

    return (amfhead_t *)buf;
}

/// \brief Each body consists of the following:
///
/// * UTF String - Target
/// * UTF String - Response
/// * Long - Body length in bytes
/// * Variable - Actual data (including a type code)
amfbody_t *
AMF::encodeBody(amfutf8_t *target, amfutf8_t *response, int nbytes, void *data)
{
    GNASH_REPORT_FUNCTION;

    char *buf = new char[sizeof(amfbody_t) + nbytes];
    memset(buf, 0, sizeof(amfbody_t) + nbytes );
    amfbody_t *body = (amfbody_t *)buf;
    memcpy(&body->target, target, sizeof(amfutf8_t));
    memcpy(&body->response, response, sizeof(amfutf8_t));    
    body->length = nbytes;
    memcpy(body->data, data, nbytes);

    return body;
}

/// \brief Each packet consists of the following:
///
/// The first byte of the AMF file/stream is believed to be a version
/// indicator. So far the only valid value for this field that has been
/// found is 0×00. If it is anything other than 0×00 (zero), your
/// system should consider the AMF file/stream to be
/// 'cmalformed'd. This can happen in the IDE if AMF calls are put
/// on the stack but never executed and the user exits the movie from the
/// IDE; the two top bytes will be random and the number of headers will
/// be unreliable.

/// The second byte of the AMF file/stream is appears to be 0×00 if the
/// client is the Flash Player and 0×01 if the client is the FlashCom
/// server. 

/// The third and fourth bytes form an integer value that specifies the
/// number of headers. 
amfpacket_t *
AMF::encodePacket(std::vector<amfhead_t *> messages)
{
    GNASH_REPORT_FUNCTION;

    int total = 0;
    amfpacket_t pkt;
    pkt.version = AMF_VERSION;
    pkt.source = CLIENT;
    pkt.count = (AMF_Int_t)messages.size();

    // figure out how big the output buffer has to be
    for (unsigned int i=0; i<messages.size(); i++ ) {
        total += messages[i]->name.length + // the UTF8 string length
            messages[i]->length  // the data length
            + sizeof(amfhead_t); // the header length
    }

    amfpacket_t *out = (amfpacket_t *)new char[total];
    char *ptr = (char *)out;
    memset(ptr, 0, total);

    // Copy the header
    memcpy(ptr, &pkt, sizeof(amfpacket_t));
    ptr += sizeof(amfpacket_t);
    
    // Add the messages
    for (unsigned int i=0; i<messages.size(); i++ ) {
        memcpy(ptr, messages[i], messages[i]->length + sizeof(amfhead_t));
        ptr += messages[i]->length + sizeof(amfhead_t);
    }

    return out;
}


AMF::astype_e
AMF::extractElementHeader(void *in)
{
    GNASH_REPORT_FUNCTION;

    return (AMF::astype_e)*(char *)in;
}

int
AMF::extractElementLength(void *in)
{
    GNASH_REPORT_FUNCTION;

    char *x = (char *)in;
    astype_e type = (astype_e)*x;
    x++;                        // skip the header byte
    
    switch (type) {
      case Number:              // a 64 bit numeric value
          return 8;
          break;
      case Boolean:             // a single byte
          return 1;
          break;
      case String:              // the length is a 2 byte value
          return (short)*(short *)x;
          break;
      case Object:
          return x - strchr(x, TERMINATOR);
          break;
      case MovieClip:
          return -1;
          log_msg("MovieClip unimplemented");
          break;
      case Null: 
          return -1;
          log_msg("Null unimplemented");
          break;
      case Undefined:
          return 0;
          break;
      case Reference:
          return -1;
          log_msg("Reference unimplemented");
          break;
      case ECMAArray:
          return x - strchr(x, TERMINATOR);
          break;
      case ObjectEnd:
          return -1;
          log_msg("ObjectEnd unimplemented");
          break;
      case StrictArray:         // the length is a 4 byte value
//          return (int *)x;
          break;
      case Date:              // a 64 bit numeric value
          return 8;
          break;
      case LongString:
          return -1;
          log_msg("LongString unimplemented");
          break;
      case Unsupported:
          return -1;
          log_msg("Unsupported unimplemented");
          break;
      case Recordset:
          return -1;
          log_msg("Recordset unimplemented");
          break;
      case XMLObject:           // the length is a 4 byte value
//          return (int)*(int *)x;
          break;
      case TypedObject:
          return x - strchr(x, TERMINATOR);
          break;
    };
    
    return 0;
}

std::string
AMF::extractString(const char *in)
{
    GNASH_REPORT_FUNCTION;

    int length;
    
    if (*in == 0) {
        length = *(in+1);        
    }
		return "";
}

int
AMF::extractNumber(const char */* in */)
{
    GNASH_REPORT_FUNCTION;    
		return 0;	
}

int
AMF::headerSize(char header)
{
//    GNASH_REPORT_FUNCTION;
    
    int headersize = -1;
    
    unsigned char hexint[2];
    switch (header & AMF_HEADSIZE_MASK) {
      case HEADER_12:
          headersize = 12;
          break;
      case HEADER_8:
          headersize = 8;
          break;
      case HEADER_4:
          headersize = 4;
          break;
      case HEADER_1:
          headersize = 11;
          break;
      default:
          char buf = header & AMF_HEADSIZE_MASK;
          hexify((unsigned char *)hexint, (unsigned char *)&buf, 1, false);
          dbglogfile << "ERROR: Header size bits out of range! was 0x"
                     << hexint << endl;
          headersize = 1;
          break;
    };

    return headersize;
}

int
AMF::parseHeader(char *in)
{
    GNASH_REPORT_FUNCTION;

    char *tmpptr = in;
    unsigned char hexint[32];
    
    hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 1, false);
    dbglogfile << "AMF header byte is: 0x" << hexint << endl;

    _amf_index = *tmpptr & AMF_INDEX_MASK;
    _header_size = headerSize(*tmpptr++);

#if 1
    hexify((unsigned char *)hexint, (unsigned char *)in, _header_size, false);
    dbglogfile << "The packet head is: 0x" << hexint << endl;
#endif
    if (_header_size >= 4) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3, false);
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;
         dbglogfile << "The mystery word is: " << _mystery_word
                    << " Hex value is: 0x" << hexint << endl;
    }

    if (_header_size >= 8) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3, false);
        _total_size = *tmpptr++;
        _total_size = (_total_size << 12) + *tmpptr++;
        _total_size = (_total_size << 8) + *tmpptr++;
        _total_size = _total_size & 0x0000ff;
        dbglogfile << "The body size is: " << _total_size
                   << " Hex value is: 0x" << hexint << endl;
    }

    if (_header_size >= 8) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 1, false);
        _type = *(content_types_e *)tmpptr;
        tmpptr++;
        dbglogfile << "The type is: " << _type
                   << " Hex value is: 0x" << hexint << endl;
    }

    switch(_type) {
      case CHUNK_SIZE:
      case BYTES_READ:
      case PING:
      case SERVER:
      case CLIENT:
      case VIDEO_DATA:
      case NOTIFY:
      case SHARED_OBJ:
      case INVOKE:
          _packet_size = AMF_VIDEO_PACKET_SIZE;
          break;
      case AUDIO_DATA:
          _packet_size = AMF_AUDIO_PACKET_SIZE;
          break;
      default:
          dbglogfile << "ERROR: Unidentified data type!" << endl;
          break;
    };
    
    if (_header_size == 12) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3, false);
        _src_dest = ntohl(*(unsigned int *)tmpptr);
        tmpptr += sizeof(unsigned int);
        dbglogfile << "The source/destination is: " << _src_dest
                   << " Hex value is: 0x" << hexint << endl;
    }

    return _packet_size;
}

int
AMF::parseBody(char *in, int bytes)
{
    GNASH_REPORT_FUNCTION;


//    unsigned char hexint[(bytes*2)+1];
    unsigned char* hexint;

    char buffer[AMF_VIDEO_PACKET_SIZE];
    //char *name;
    short length;
    int data_bytes = bytes;
    amf_element_t el;

    if (in == 0) {
        dbglogfile << "ERROR: input data is NULL!" << endl;
        return -1;
    }

		hexint =  (unsigned char*) malloc((bytes * 2) + 1);

    memset(buffer, 0, AMF_VIDEO_PACKET_SIZE);
    
//     memcpy(_amf_data +_read_size, in, AMF_VIDEO_PACKET_SIZE);
//     _read_size += bytes;
#if 1
    hexify((unsigned char *)hexint, (unsigned char *)in, bytes, true);
    dbglogfile << "The packet body is: 0x" << hexint << endl;
#endif

//    tmpptr = in;
     char *tmpptr = in;
    
//    if (!_amf_data) {
//         dbglogfile << "new amf_data block, size is: " << _total_size << endl;
//        _amf_data = new unsigned char(_total_size+1);
//        _seekptr = _amf_data;
//        memset(_amf_data, 0, _total_size);
//    }
//        memcpy(_seekptr, tmpptr, bytes);
//        _seekptr += bytes;

// All elements look like this:
// the first two bytes is the length of name of the element
// Then the next bytes are the element name
// After the element name there is a type byte. If it's a Number type, then 8 bytes are read
// If it's a String type, then there is a count of characters, then the string value    
    
    while (data_bytes) {

        // Check the type of the element data
        char type = *(astype_e *)tmpptr;
        tmpptr++;                        // skip the header byte
        data_bytes--;

        switch ((astype_e)type) {
          case Number:
//              memcpy(buffer, tmpptr, 8);
              tmpptr += 8;
              data_bytes -= 8;
              continue;
              break;
          case Boolean:
          case String:
              dbglogfile << "AMF type: " << astype_str[(int)type] << ": a work in progress!" << endl;
              break;
          case Object:
              dbglogfile << "AMF type: " << astype_str[(int)type] << ": a work in progress!" << endl;
              do {
                  tmpptr = extractVariables(el, tmpptr);
              } while (el.length > 0);
              break;
          case MovieClip:
          case Null: 
          case Undefined:
          case Reference:
          case ECMAArray:
          case ObjectEnd:
          case StrictArray:
          case Date:
          case LongString:
          case Unsupported:
          case Recordset:
          case XMLObject:
          case TypedObject:
          default:
//          dbglogfile << astype_str[(int)type] << ": unimplemented!" << endl;
              dbglogfile << (int)type << ": unimplemented!" << endl;
              break;
        }
        
        // get the length of the name
        length = ntohs(*(short *)tmpptr);
#if 0
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, length+2, true);
        dbglogfile << "The element is: 0x" << hexint << endl;
#endif
        tmpptr += 2;
        dbglogfile << "AMF element length is: " << length << endl;
        // get the name of the element
        if (length) {
            memcpy(buffer, tmpptr, length);
        }
        tmpptr += length;

        dbglogfile << "AMF element name is: " << buffer << endl;

        el.name = buffer;
        data_bytes -= length - 2;
        
//}
    }

		free(hexint);

    return -1;
}

char *
AMF::extractVariables(amf_element_t &el, const char *in)
{
    GNASH_REPORT_FUNCTION;
    
    char buffer[AMF_VIDEO_PACKET_SIZE];
    const char *tmpptr = in;
    short length = 0;

    el.length = 0;
    el.name.erase();
    if (el.data) {
        el.data = 0;
    }
    
    memset(buffer, 0, AMF_VIDEO_PACKET_SIZE);
    // @@ casting generic pointers to bigger types may be dangerous
    //    due to memory alignment constraints
    length = ntohs(*(const short *)tmpptr);
    el.length = length;
    if (length == 0) {
        if (*(tmpptr+2) == ObjectEnd) {
            dbglogfile << "End of Object definition." << endl; 
        }
    }
    
#if 0
    hexify((unsigned char *)hexint, (unsigned char *)tmpptr, length+2, true);
    dbglogfile << "The element is: 0x" << hexint << endl;
#endif
    tmpptr += 2;
    dbglogfile << "AMF element length is: " << length << endl;
    // get the name of the element
    if (length) {
        memcpy(buffer, tmpptr, length);
        el.name = buffer;
        tmpptr += length;
    } else {
        // when reading in an object definition, a byte of 0x9 after
        // a length of zero instead of the variable name.
        if (*tmpptr == ObjectEnd) {
            dbglogfile << "End of Object definition." << endl;
            tmpptr++;
            el.type = ObjectEnd;
            el.name.erase();
            el.length = 0;
            el.data = 0;
	    // @@ we're dropping constness of input here..
	    //    it might be a problem, but strchr does it as well...
            return const_cast<char*>(tmpptr);
        }
    }
    
//    dbglogfile << "AMF element name is: " << buffer << endl;
    char type = *(const astype_e *)tmpptr++;

    if (type <= TypedObject) {
        dbglogfile << "AMF type is: " << astype_str[(int)type] << endl;
    }
    
    switch ((astype_e)type) {
      case Number:
          memcpy(buffer, tmpptr, 8);
          swap64((uint64)*buffer);
          dbglogfile << "Number \"" << el.name.c_str() << "\" is: " << (long)buffer << endl;
          tmpptr += 8;
          break;
      case Boolean:
      case String:
          length = ntohs(*(const short *)tmpptr); // @@ this cast is dangerous
          tmpptr += sizeof(short);
          el.data = (unsigned char*)tmpptr; // @@ this cast is dangerous
          dbglogfile << "Variable \"" << el.name.c_str() << "\" is: " << el.data << endl;
          tmpptr += length;
          el.length = length;
          break;
      case Object:
      case MovieClip:
      case Null: 
      case Undefined:
      case Reference:
      case ECMAArray:
      case ObjectEnd:
      case StrictArray:
      case Date:
      case LongString:
      case Unsupported:
      case Recordset:
      case XMLObject:
      case TypedObject:
//       default:
//           dbglogfile << "astype_e of value: " << (int)type << " is unimplemented!" << endl;
          break;
    }
    
    return const_cast<char*>(tmpptr); // we're dropping const specification
}

} // end of amf namespace
