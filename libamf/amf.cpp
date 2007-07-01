// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: amf.cpp,v 1.39 2007/07/01 10:54:05 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <cstring> //For memcpy(), commented out atm.
#include <string>
#include <vector>

#if defined(_WIN32) || defined(WIN32)
#	include <winsock2.h>
#else
#	include <netinet/in.h>
#endif

#include "log.h"
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
      _mystery_word(0)
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
      _mystery_word(0)
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
/// All Numeric values for AMF files are big endian, so we have
/// to swap the bytes to be little endian for most machines. Don't do
/// anything if we happen to be on a big-endian machine.
///
/// Returns its first parameter, pointing to the (maybe-byte-swapped) data.
static void *
swapBytes(void *word, int size)
{
    union {
	uint16_t s;
	struct {
	    uint8_t c0;
	    uint8_t c1;
	} c;
    } u;
	   
    u.s = 1;
    if (u.c.c0 == 0) {
	// Big-endian machine: do nothing
	return word;
    }

    // Little-endian machine: byte-swap the word

    // A conveniently-typed pointer to the source data
    unsigned char *x = static_cast<unsigned char *>(word);

    switch (size) {
    case 2: // 16-bit integer
      {
	unsigned char c;
	c=x[0]; x[0]=x[1]; x[1]=c;
	break;
      }
    case 4: // 32-bit integer
      {
	unsigned char c;
	c=x[0]; x[0]=x[3]; x[3]=c;
	c=x[1]; x[1]=x[2]; x[2]=c;
	break;
      }
    case 8: // 64-bit integer
      {
	unsigned char c;
	c=x[0]; x[0]=x[7]; x[7]=c;
	c=x[1]; x[1]=x[6]; x[6]=c;
	c=x[2]; x[2]=x[5]; x[5]=c;
	c=x[3]; x[3]=x[4]; x[4]=c;
	break;
      }
    }

    return word;
}


bool
AMF::parseAMF(char *in)
{
    GNASH_REPORT_FUNCTION;

    char *x = static_cast<char *>(in);

    while (*x != OBJECT_END) {
        x = readElement(x);
    }
    return true;
}

// @@ I don't believe this works, since it only advances x over the
// @@ object's metafields (length etc), but not over its contents. -martin
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
    
    log_msg(_("Type is %s"), astype_str[type]);

    x++;                        // skip the type byte
    switch (type) {
      case NUMBER:
	  // AMF numbers are 64-bit big-endian integers.
          num = *(amfnum_t *)swapBytes(x+1, 8);
          log_msg(_("Number is " AMFNUM_F), num);
          break;
      case BOOLEAN:
          boolshift = *x;
          log_msg(_("Boolean is %d"), boolshift);
          break;
      case STRING:
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
          log_msg(_("String is %s"), mstr);
          break;
      case OBJECT:
//          readElement();
          log_unimpl("Object AMF decoder");
          break;
      case MOVIECLIP:
        log_unimpl("MovieClip AMF decoder");
          break;
      case UNSUPPORTED:
        log_unimpl("Unsupported AMF decoder");
          break;
      case NULL_VALUE: 
          log_unimpl("Null AMF decoder");
          break;
      case UNDEFINED:
          log_msg(_("Undefined element"));
          break;
      case REFERENCE:
          log_unimpl("Reference AMF decoder");
          break;
      case ECMA_ARRAY:
          log_unimpl("ECMAArray AMF decoder");
          break;
      case OBJECT_END:
          log_unimpl("ObjectEnd AMF decoder");
          break;
      case STRICT_ARRAY:
          log_unimpl("StrictArray AMF decoder");
          break;
      case DATE:
          nanosecs = *(amfnum_t *)swapBytes(x+1, 8);
          log_msg(_("Date is " AMFNUM_F " nanoseconds"), nanosecs);
          break;
      case LONG_STRING:
//          int length = *(short *)swapBytes(x, 4);
          x+=4;                  // skip the length bytes
//        mstr = new char[length+1];
//          memcpy(mstr, x, length);
          log_msg(_("String is %s"), mstr);
          break;
      case RECORD_SET:
          log_unimpl("Recordset AMF decoder");
          break;
      case XML_OBJECT:
          log_unimpl("XMLObject AMF decoder");
          break;
      case TYPED_OBJECT:
          log_unimpl("TypedObject AMF decoder");
          break;
      default:
          log_msg("Warning: Unknown AMF element type %d\n", type);
          break;
    }
    
    return x;
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
AMF::encodeElement(astype_e type, const void *in, int nbytes)
{
    GNASH_REPORT_FUNCTION;

    char *out = NULL, *x;
    amfnum_t num;
    int pktsize = -1;

    // Packets are of varying length. A few pass in a byte count, but
    // most packets have a hardcoded size.
    switch (type) {
        // Encode the data as a 64 bit, big-endian, numeric value
      case NUMBER:
          // one 64 bit number
          pktsize = AMF_NUMBER_SIZE + AMF_HEADER_SIZE;
          break;
      case BOOLEAN:
          pktsize = 2;          // just one data byte
          break;
      case STRING:
          pktsize = nbytes + 3; // two length bytes after the header
          break;
      case OBJECT:
          pktsize = 0;          // look for the terminator
          break;
      case MOVIECLIP:
          pktsize = -1;         // FIXME: no clue
          break;
      case NULL_VALUE: 
          pktsize = -1;         // FIXME: no clue
          break;
      case UNDEFINED:
	  // just the header, no data
          pktsize = nbytes + 3; // two length bytes after the header
          break;
      case REFERENCE:
          pktsize = -1;         // FIXME: no clue
          break;
      case ECMA_ARRAY:
          pktsize = 0;          // look for the terminator
          break;
      case OBJECT_END:
          pktsize = -1;         // FIXME: no clue
          break;
      case STRICT_ARRAY:
          pktsize = nbytes + 5; // 4 length bytes, then data
          break;
      case DATE:
          pktsize = 9;          // one 64 bit number
          break;
      case LONG_STRING:
          pktsize = nbytes + 5; // 4 length bytes, then data
          break;
      case UNSUPPORTED:
          pktsize = -1;         // FIXME: no clue
          break;
      case RECORD_SET:
          pktsize = -1;         // FIXME: no clue
          break;
      case XML_OBJECT:
          pktsize = nbytes + 5;// 4 length bytes, then data
          break;
      case TYPED_OBJECT:
          pktsize = 0;          // look for the terminator
          break;
// FIXME, shouldn't there be a default case here?
    };
    
    switch (type) {
        // Encode the data as a 64 bit, big-endian, numeric value
      case NUMBER:
          x = out = new char[pktsize];
          memset(x, 0, pktsize);
          *x++ = (char)AMF::NUMBER;
          memcpy(&num, in, AMF_NUMBER_SIZE);
          swapBytes(&num, AMF_NUMBER_SIZE);
          memcpy(x, &num, AMF_NUMBER_SIZE);
          break;
      case BOOLEAN:
          // Encode a boolean value. 0 for false, 1 for true
          out = new char[pktsize];
          x = out;    
          *x++ = (char)AMF::BOOLEAN;
          *x = *static_cast<const char *>(in);
          break;
      case STRING:
          // Encode a string value. The data follows a 2 byte length
          // field. (which must be big-endian)
          x = out = new char[pktsize];
          memset(x, 0, pktsize);
          *x++ = AMF::STRING;
          num = nbytes;
          swapBytes(&num, 2);
          memcpy(x, &num, 2);
          x+=2;
          memcpy(x, in, nbytes);
          break;
      case OBJECT:
          log_unimpl("Object AMF encoder");
          break;
      case MOVIECLIP:
          log_unimpl("MovieClip AMF encoder");
          break;
      case NULL_VALUE: 
          log_unimpl("Null AMF encoder");
          break;
      case UNDEFINED:
          x = out = new char[pktsize];
          memset(x, 0, pktsize);
          *x++ = AMF::UNDEFINED;
          num = nbytes;
          swapBytes(&num, 2);
          memcpy(x, &num, 2);
          x+=2;
          memcpy(x, in, nbytes);
	  break;
      case REFERENCE:
          log_unimpl("Reference AMF encoder");
          break;
      case ECMA_ARRAY:
          log_unimpl("ECMAArray AMF encoder");
          break;
      case OBJECT_END:
          log_unimpl("ObjectEnd AMF encoder");
          break;
      case STRICT_ARRAY:
          log_unimpl("StrictArray AMF encoder");
          break;
          // Encode the date as a 64 bit, big-endian, numeric value
      case DATE:
          x = out = new char[pktsize];
          memset(x, 0, pktsize);
          *x++ = AMF::DATE;
          num = *static_cast<const amfnum_t*>(in);
          swapBytes(&num, 8);
          memcpy(x, &num, 8);
          break;
      case LONG_STRING:
          log_unimpl("LongString AMF encoder");
          break;
      case UNSUPPORTED:
          log_unimpl("Unsupported AMF encoder");
          break;
      case RECORD_SET:
          log_unimpl("Recordset AMF encoder");
          break;
      case XML_OBJECT:
          // Encode an XML object. The data follows a 4 byte length
          // field. (which must be big-endian)
          x = out = new char[pktsize];
          memset(x, 0, pktsize);
          *x++ = AMF::STRING;
          num = nbytes;
          swapBytes(&num, 4);
          memcpy(x, in, nbytes);
          break;
      case TYPED_OBJECT:
          log_unimpl("TypedObject AMF encoder");
          break;
    };
    
    return out;
}

/// \brief \ Each RTMP header consists of the following:
///
/// * Index & header size - The header size and amf channel index.
/// * Total size - The total size of the message
/// * Type - The type of the message
/// * Routing - The source/destination of the message
void *
AMF::encodeRTMPHeader(int amf_index, amf_headersize_e head_size,
		      int total_size, content_types_e type,
		      amfsource_e routing)
{
    GNASH_REPORT_FUNCTION;
    void *out = new char[total_size + 12 + 4];
    memset(out, 0, total_size + 12 + 4);
    char *tmpptr = reinterpret_cast<char *>(out);
    // Make the index & header size byte
    *tmpptr = head_size & AMF_HEADSIZE_MASK;    
    *tmpptr += amf_index  & AMF_INDEX_MASK;
    tmpptr++;

    // Add the unknown bytes. These seem to be used by video and
    // audio, and only when the header size is 4 or more.
    if (head_size <= HEADER_4) {
	memset(tmpptr, 0, 3);
	tmpptr += 3;
    }

    // Add the size of the message if the header size is 8 or more.
    if (head_size <= HEADER_8) {
	int length = total_size;
	swapBytes(&length, 4);
	memcpy(tmpptr, ((char *)&length +1), 3);
	tmpptr += 3;
    }
    
    // Add the type of the objectif the header size is 8 or more.
    if (head_size <= HEADER_8) {
	*tmpptr = type;
	tmpptr++;
    }

    // Add the routing of the message if the header size is 12 or more.
    if (head_size == HEADER_12) {
	memcpy(tmpptr, &routing, 4);
	tmpptr += 4;
    }

    return out;
}

#if 0
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
    ptr++;

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
AMF::encodePacket(std::vector<amfpacket_t *> messages)
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
#endif


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
      case NUMBER:              // a 64 bit numeric value
          return 8;
          break;
      case BOOLEAN:             // a single byte
          return 1;
          break;
      case STRING:              // the length is a 2 byte value
		//FIXME, there are all kinds of byte order problems in this code.
          return (short)*(short *)x;
          break;
      case OBJECT:
          return x - strchr(x, TERMINATOR);
          break;
      case MOVIECLIP:
          return -1;
          log_unimpl("MovieClip AMF extractor");
          break;
      case NULL_VALUE: 
          return -1;
          log_unimpl("Null AMF extractor");
          break;
      case UNDEFINED:
          return 0;
          break;
      case REFERENCE:
          return -1;
          log_unimpl("Reference AMF extractor");
          break;
      case ECMA_ARRAY:
          return x - strchr(x, TERMINATOR);
          break;
      case OBJECT_END:
          return -1;
          log_unimpl("ObjectEnd AMF extractor");
          break;
      case STRICT_ARRAY:         // the length is a 4 byte value
//          return (int *)x;
          break;
      case DATE:              // a 64 bit numeric value
          return 8;
          break;
      case LONG_STRING:
          return -1;
          log_unimpl("LongString AMF extractor");
          break;
      case UNSUPPORTED:
          return -1;
          log_unimpl("Unsupported AMF extractor");
          break;
      case RECORD_SET:
          return -1;
          log_unimpl("Recordset AMF extractor");
          break;
      case XML_OBJECT:           // the length is a 4 byte value
//          return (int)*(int *)x;
          break;
      case TYPED_OBJECT:
          return x - strchr(x, TERMINATOR);
          break;
    };
    
    return 0;
}

char *
AMF::extractString(const char *in)
{
    GNASH_REPORT_FUNCTION;
    char *buf = NULL;
    char *x = const_cast<char*>(in);
    short length;
    
    if (*x == AMF::STRING) {
        x++;
        length = *(reinterpret_cast<short *>(x));
        x += sizeof(short);
        buf = new char[length+1];
        memset(buf, 0, length+1);
        memcpy(buf, x, length);
    } else {
        log_error("Tried to extract AMF string from non String object!");
    }
    
    return buf;
}

amfnum_t *
AMF::extractNumber(const char *in)
{
    GNASH_REPORT_FUNCTION;    
    char *x = const_cast<char *>(in);
    amfnum_t *num = new amfnum_t;
    memset(num, 0, AMF_NUMBER_SIZE+1);
    
    if (*x == AMF::NUMBER) {
        x++;
        memcpy(num, x, AMF_NUMBER_SIZE);
        swapBytes(num, AMF_NUMBER_SIZE);
    } else {
        log_error("Tried to extract AMF Number from non Number object!");
    }

    return num;
}

void *
AMF::encodeVariable(amf_element_t & /* el */)
{
    GNASH_REPORT_FUNCTION;
    return NULL;
}

void *
AMF::encodeVariable(const char *name, bool flag)
{
    GNASH_REPORT_FUNCTION;
    
    int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
    char *out = new char[outsize];
    char *tmpptr = out;
    short length;

    length = strlen(name);
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    strcpy(tmpptr, name);
    tmpptr += strlen(name);
    *tmpptr = AMF::BOOLEAN;
    tmpptr++;
    *tmpptr = flag;

    return out;    
}

void *
AMF::encodeVariable(const char *name)
{
    GNASH_REPORT_FUNCTION;
    int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
    char *out = new char[outsize];
    char *tmpptr = out;
    short length;

    length = strlen(name);
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    strcpy(tmpptr, name);
    tmpptr += strlen(name);
    *tmpptr = AMF::UNDEFINED;
    tmpptr++;

    return out;    
}

void *
AMF::encodeVariable(const char *name, amfnum_t bignum)
{
    GNASH_REPORT_FUNCTION;
    int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
    char *out = new char[outsize];
    char *tmpptr = out;
    short length;
    amfnum_t newnum = bignum;
    char *numptr = (char *)&newnum;

    length = strlen(name);
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    strcpy(tmpptr, name);
    tmpptr += strlen(name);
    *tmpptr = AMF::NUMBER;
    tmpptr++;
//    swapBytes(numptr, AMF_NUMBER_SIZE);
    memcpy(tmpptr, numptr, AMF_NUMBER_SIZE);

    return out;    
}

void *
AMF::encodeVariable(const char *name, const char *val)
{
    GNASH_REPORT_FUNCTION;

    int outsize = strlen(name) + strlen(val) + 5;
    char *out = new char[outsize];
    char *tmpptr = out;
    short length;

    length = strlen(name);
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    strcpy(tmpptr, name);
    tmpptr += strlen(name);
    *tmpptr = AMF::STRING;
    tmpptr++;
    length = strlen(val);
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    strcpy(tmpptr, val);

    return out;
}

void *
AMF::encodeVariable(std::string &name, std::string &val)
{
    GNASH_REPORT_FUNCTION;

    int outsize = name.size() + val.size() + 5;
    unsigned char *out = new unsigned char[outsize];
    unsigned char *tmpptr = out;
    short length;

    length = name.size() && 0xffff;
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    memcpy(tmpptr, name.c_str(), name.size());
    tmpptr += name.size();
    *tmpptr = AMF::STRING;
    tmpptr++;
    length = val.size() && 0xffff;
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    memcpy(tmpptr, val.c_str(), name.size());

    return out;
}

int
AMF::headerSize(char header)
{
//    GNASH_REPORT_FUNCTION;
    
    int headersize = -1;
    
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
          log_error(_("AMF Header size bits (0x%X) out of range"),
          		header & AMF_HEADSIZE_MASK);
          headersize = 1;
          break;
    };

    return headersize;
}

int
AMF::parseHeader(unsigned char *in)
{
    GNASH_REPORT_FUNCTION;

    unsigned char *tmpptr = in;
    unsigned char hexint[32];
    
    log_msg (_("AMF header byte is: 0x%X"), *in);

    _amf_index = *tmpptr & AMF_INDEX_MASK;
    log_msg (_("The AMF channel index is %d"), _amf_index);
    
    _header_size = headerSize(*tmpptr++);
    log_msg (_("The header size is %d"), _header_size);

#if 1
    hexify((unsigned char *)hexint, (unsigned char *)in, _header_size, false);
    log_msg(_("The packet head is: 0x%s"), hexint);
#endif
    if (_header_size >= 4) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3, false);
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;
        log_msg(_("The mystery word is: %d or 0x%s"), _mystery_word, hexint);
    }

    if (_header_size >= 8) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3, false);
        _total_size = *tmpptr++;
        _total_size = (_total_size << 12) + *tmpptr++;
        _total_size = (_total_size << 8) + *tmpptr++;
        _total_size = _total_size & 0xffffff;
        log_msg(_("The body size is: %d, or 0x%s"), _total_size, hexint);
        _amf_data = new unsigned char(_total_size+1);
        _seekptr = _amf_data;
//        memset(_amf_data, 0, _total_size+1);
    }

    if (_header_size >= 8) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 1, false);
        _type = *(content_types_e *)tmpptr;
        tmpptr++;
        log_msg(_("The type is: %d, or 0x%s"), _type, hexint);
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
          log_error (_("ERROR: Unidentified AMF header data type %d"), _type);
          break;
    };
    
    if (_header_size == 12) {
        hexify((unsigned char *)hexint, (unsigned char *)tmpptr, 3, false);
        _src_dest = *(reinterpret_cast<amfsource_e *>(tmpptr));
        tmpptr += sizeof(unsigned int);
        log_msg(_("The source/destination is: %d, or 0x%s"), _src_dest, hexint);
    }

    return _packet_size;
}

unsigned char *
AMF::addPacketData(unsigned char *data, int bytes)
{
    GNASH_REPORT_FUNCTION;
    memcpy(_seekptr, data, bytes);
    _seekptr+=bytes;
    return _seekptr;
}

int
AMF::parseBody()
{
    GNASH_REPORT_FUNCTION;

    return parseBody(_amf_data, _total_size);
}

int
AMF::parseBody(unsigned char *in, int bytes)
{
    GNASH_REPORT_FUNCTION;

    unsigned char *tmpptr;

//    unsigned char hexint[(bytes*2)+1];
    unsigned char* hexint;

    char buffer[500];
//    char *name;
    short length;
    amf_element_t el;

    if (bytes == 0) {
        return 0;
    }
    
    if (in == 0) {
        log_error(_("AMF body input data is NULL"));
        return -1;
    }

    hexint =  (unsigned char*) malloc((bytes * 3) + 12);

//     memcpy(_amf_data +_read_size, in, AMF_VIDEO_PACKET_SIZE);
//     _read_size += bytes;
#if 1
    hexify((unsigned char *)hexint, (unsigned char *)in, bytes, true);
    log_msg(_("The packet body is: 0x%s"), hexint);
#endif

//    tmpptr = in;
    tmpptr = in;
    
// All elements look like this:
// the first two bytes is the length of name of the element
// Then the next bytes are the element name
// After the element name there is a type byte. If it's a Number type, then 8 bytes are read
// If it's a String type, then there is a count of characters, then the string value    
    
    while (tmpptr  != (in + bytes)) {
        memset(buffer, 0, sizeof(buffer));	//FIXME, slow
        // Check the type of the element data
        char type = *(astype_e *)tmpptr;
        tmpptr++;                        // skip the header byte

        switch ((astype_e)type) {
          case NUMBER:
//              memcpy(buffer, tmpptr, 8);
              tmpptr += 8;
              continue;
              break;
          case BOOLEAN:
          case STRING:
              // get the length of the name
              length = ntohs((*(short *)tmpptr) & 0xffff);
              tmpptr += 2;
              log_msg(_("AMF String length is: %d"), length);
              // get the name of the element
              if (length) {
                  memcpy(buffer, tmpptr, length);
              }
              tmpptr += length;
              log_msg(_("AMF String is: %s"), buffer);              
              el.name = buffer;
              break;
          case OBJECT:
              do {
                  tmpptr = extractVariable(&el, tmpptr);
              } while (el.type != AMF::OBJECT_END);
              break;
          case MOVIECLIP:
          case NULL_VALUE: 
          case UNDEFINED:
          case REFERENCE:
          case ECMA_ARRAY:
          case OBJECT_END:
          case STRICT_ARRAY:
          case DATE:
          case LONG_STRING:
          case UNSUPPORTED:
          case RECORD_SET:
          case XML_OBJECT:
          case TYPED_OBJECT:
          default:
	    log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)type);
              return -1;
        }
    }

    free(hexint);

    return -1;
}

unsigned char *
AMF::extractVariable(amf_element_t *el, unsigned char *in)
{
    GNASH_REPORT_FUNCTION;
    
    unsigned char buffer[AMF_PACKET_SIZE];
    unsigned char *tmpptr = in;
    short length;

    el->length = 0;
    el->name.erase();
    if (el->data) {
        el->data = 0;
    }
    
    memset(buffer, 0, AMF_PACKET_SIZE);
    // @@ casting generic pointers to bigger types may be dangerous
    //    due to memory alignment constraints
    length = *((short *)tmpptr);
    swapBytes(&length, 2);
//    length = ntohs((*(const short *)tmpptr) & 0xffff);
    el->length = length;
    if (length == 0) {
        if (*(tmpptr+2) == AMF::OBJECT_END) {
            log_msg(_("End of Object definition"));
            el->length = 0;
            el->type = AMF::OBJECT_END;
            tmpptr+=3;
            return tmpptr;
        }
    }
    
#if 0
    unsigned char hexint[AMF_PACKET_SIZE];
    hexify((unsigned char *)hexint, (unsigned char *)tmpptr, length*3, true);
    log_msg(_("The element is: 0x%s", hexint);
#endif
    tmpptr += 2;
    // get the name of the element
    if (length > 0) {
        log_msg(_("AMF element length is: %d"), length);
        memcpy(buffer, tmpptr, length);
        el->name = reinterpret_cast<char *>(buffer);
        tmpptr += length;
    }
    
//    log_msg(_("AMF element name is: %s", buffer);
    astype_e type = (astype_e)((*tmpptr++) & 0xff);

    if (type <= AMF::TYPED_OBJECT) {
        log_msg(_("AMF type is: %s"), astype_str[(int)type]);
	el->type = type;
    }

    
    switch (type) {
      case NUMBER:
        {
          memcpy(buffer, tmpptr, AMF_NUMBER_SIZE);
          swapBytes(buffer, AMF_NUMBER_SIZE);
          unsigned char* tmp = new unsigned char[AMF_NUMBER_SIZE+1];
          memset(tmp, 0, AMF_NUMBER_SIZE+1);
          memcpy(tmp, buffer, AMF_NUMBER_SIZE);
          el->data = tmp;
          unsigned char hexint[AMF_NUMBER_SIZE*3];
          hexify((unsigned char *)hexint, (unsigned char *)buffer,
		 AMF_NUMBER_SIZE, false);
          log_msg(_("Number \"%s\" is: 0x%s"), el->name.c_str(), hexint);
//          amfnum_t *num = extractNumber(tmpptr);
          tmpptr += 8;
          break;
        }
      case BOOLEAN:
        {
//          int value = *tmpptr;
          unsigned char* tmp = new unsigned char[1];
          memcpy(tmp, tmpptr, 1); 
          el->data = tmp;
	  log_msg((*tmpptr == 0) ? 
		  _("Boolean \"%s\" is: true"):
		  _("Boolean \"%s\" is: false"), el->name.c_str());
	  tmpptr += 1;
	  break;
        }
      case STRING:
	  length = ntohs((*(const short *)tmpptr) & 0xffff);
          tmpptr += sizeof(short);
          el->data = (const unsigned char*)tmpptr; 
          log_msg(_("Variable \"%s\" is: %s"), el->name.c_str(), el->data);
          tmpptr += length;
          el->length = length;
          break;
      case OBJECT:
      case MOVIECLIP:
      case NULL_VALUE:
	  // Undefined types have a name, but no value
		//FIXME this shouldn't fall through!
      case UNDEFINED:
          log_msg(_("Undefined type"));
	  length = ntohs((*(const short *)tmpptr) & 0xffff);
          el->data = (const unsigned char*)tmpptr; 
          log_msg(_("Variable \"%s\" is: %s"), el->name.c_str(), el->data);
//          tmpptr += length;
          el->length = length;
          el->type = AMF::UNDEFINED;
          break;
      case REFERENCE:
      case ECMA_ARRAY:
			// FIXME this shouldn't fall thru
      case OBJECT_END:
          log_msg(_("End of Object definition"));
          el->name.erase();
          el->length = 0;
          el->data = 0;
          el->type = AMF::OBJECT_END;
          break;
      case STRICT_ARRAY:
      case DATE:
      case LONG_STRING:
      case UNSUPPORTED:
      case RECORD_SET:
      case XML_OBJECT:
      case TYPED_OBJECT:
      default:
          log_unimpl(_("astype_e of value: %d"), (int)type);
          break;
    }
    
    return tmpptr; // we're dropping const specification
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
