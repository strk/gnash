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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <vector>

#if defined(_WIN32) || defined(WIN32)
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

#include "log.h"
#include "amf.h"
#include "element.h"
#include "amfutf8.h"
#include <boost/cstdint.hpp> // for boost::?int??_t

using namespace std;
using namespace gnash;

namespace amf 
{

AMF::AMF() 
    : _type(NONE),
#if 0
      _amf_index(0),
      _header_size(0),
      _total_size(0),
      _packet_size(0),
      _amf_data(0),
      _seekptr(0),
#endif
      _mystery_word(0)
{
//    GNASH_REPORT_FUNCTION;
}

// AMF::AMF(int size) 
//     : _type(NONE),
// #if 0
//       _amf_index(0),
//       _header_size(0),
//       _total_size(0),
//       _packet_size(0),
//       _amf_data(0),
// #endif
//       _mystery_word(0)
// {
// //    GNASH_REPORT_FUNCTION;
// #if 0
//     if (!_amf_data) {
//         _amf_data = new uint8_t(size+1);
//         memset(_amf_data, 0, size+1);
//     }
//     _seekptr = _amf_data;
// #endif
// }

AMF::~AMF()
{
//    GNASH_REPORT_FUNCTION;
}


/// \brief Swap bytes from big to little endian.
///
/// All Numeric values for AMF files are big endian, so we have
/// to swap the bytes to be little endian for most machines. Don't do
/// anything if we happen to be on a big-endian machine.
///
/// Returns its first parameter, pointing to the (maybe-byte-swapped) data.
void *
swapBytes(void *word, int size)
{
    union {
	boost::uint16_t s;
	struct {
	    boost::uint8_t c0;
	    boost::uint8_t c1;
	} c;
    } u;
	   
    u.s = 1;
    if (u.c.c0 == 0) {
	// Big-endian machine: do nothing
	return word;
    }

    // Little-endian machine: byte-swap the word

    // A conveniently-typed pointer to the source data
    boost::uint8_t *x = static_cast<boost::uint8_t *>(word);

    switch (size) {
    case 2: // 16-bit integer
      {
	boost::uint8_t c;
	c=x[0]; x[0]=x[1]; x[1]=c;
	break;
      }
    case 4: // 32-bit integer
      {
	boost::uint8_t c;
	c=x[0]; x[0]=x[3]; x[3]=c;
	c=x[1]; x[1]=x[2]; x[2]=c;
	break;
      }
    case 8: // 64-bit integer
      {
	boost::uint8_t c;
	c=x[0]; x[0]=x[7]; x[7]=c;
	c=x[1]; x[1]=x[6]; x[6]=c;
	c=x[2]; x[2]=x[5]; x[5]=c;
	c=x[3]; x[3]=x[4]; x[4]=c;
	break;
      }
    }

    return word;
}

#if 0
bool
AMF::parseAMF(boost::uint8_t * /* in */)
{
//    GNASH_REPORT_FUNCTION;

#if 0
    boost::uint8_t *x = in;

    while (*x != Element::OBJECT_END) {
        x = readElements(x);
    }
    return true;
#endif
    return false;
}

vector<AMF::amf_element_t *> *
AMF::readElements(boost::uint8_t *in)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *x = in;
    astype_e type = (astype_e)*x;
    bool boolshift;
    const char *mstr = NULL;
    double *num;
    double nanosecs;
    short length;

    vector<AMF::amf_element_t> list;
    AMF::amf_element_t el;
    
    
    log_msg(_("Type is %s"), astype_str[type]);

//    x++;                        // skip the type byte
    switch (type) {
      case Element::NUMBER:
	  // AMF numbers are 64-bit big-endian integers.
	  num = extractNumber(x);
//	  log_msg(_("Number is " AMFNUM_F), num));
          break;
      case Element::BOOLEAN:
          boolshift = *x;
          log_msg(_("Boolean is %d"), boolshift);
          break;
      case Element::STRING:
          string str = extractString(x);
//        int length = *(short *)swapBytes(x, 2);
//           length = *(short *)x;
//           x+=2;                  // skip the length bytes
//           mstr = new char[length+1];
//           memset(mstr, 0, length+1);
//           memcpy(mstr, x, length);
          // The function converts the multibyte string beginning at
          // *src to a sequence of wide characters as if by repeated
          // calls of the form:
//          mbsrtowcs
		  log_msg(_("String is %s"), mstr);
          break;
      case Element::OBJECT:
//          return reinterpret_cast<uint8_t *>(extractObject(x));
//          readElement();
          log_unimpl("Object AMF decoder");
          break;
      case Element::MOVIECLIP:
        log_unimpl("MovieClip AMF decoder");
          break;
      case Element::UNSUPPORTED:
        log_unimpl("Unsupported AMF decoder");
          break;
      case Element::NULL_VALUE: 
          log_unimpl("Null AMF decoder");
          break;
      case Element::UNDEFINED:
          log_msg(_("Undefined element"));
          break;
      case Element::REFERENCE:
          log_unimpl("Reference AMF decoder");
          break;
      case Element::ECMA_ARRAY:
          log_unimpl("ECMAArray AMF decoder");
          break;
      case Element::OBJECT_END:
          log_unimpl("ObjectEnd AMF decoder");
          break;
      case Element::STRICT_ARRAY:
          log_unimpl("StrictArray AMF decoder");
          break;
      case Element::DATE:
          nanosecs = *(double *)swapBytes(x+1, 8);
//          log_msg(_("Date is " AMFNUM_F " nanoseconds"), nanosecs);
          break;
      case Element::LONG_STRING:
//          int length = *(short *)swapBytes(x, 4);
          x+=4;                  // skip the length bytes
//        mstr = new char[length+1];
//          memcpy(mstr, x, length);
//          log_msg(_("String is %s"), mstr);
          break;
      case Element::RECORD_SET:
          log_unimpl("Recordset AMF decoder");
          break;
      case Element::XML_OBJECT:
          log_unimpl("XMLObject AMF decoder");
          break;
      case Element::TYPED_OBJECT:
          log_unimpl("TypedObject AMF decoder");
          break;
      default:
          log_error("Warning: Unknown AMF element type %d\n", type);
          break;
    }
    
    return x;
}
#endif


//
// Methods for encoding data into big endian formatted raw AMF data.
//

/// Encode a string object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeString(const char *str)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint16_t length;
    
    int pktsize = strlen(str) + AMF_HEADER_SIZE;
    // Encode a string value. The data follows a 2 byte length
    // field. (which must be big-endian)
    boost::uint8_t* x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    *x++ = Element::STRING;
    log_debug("Encoded data size is going to be %d", length);
    swapBytes(&length, 2);
    memcpy(x, &length, 2);
    x += 2;
    memcpy(x, str, pktsize - AMF_HEADER_SIZE);
    x += pktsize - AMF_HEADER_SIZE;
    
    return x;
}

/// Encode a 64 bit number
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeNumber(double indata)
{
//    GNASH_REPORT_FUNCTION;
    int pktsize = AMF_NUMBER_SIZE + AMF_HEADER_SIZE;
    double num;
    // Encode the data as a 64 bit, big-endian, numeric value
    boost::uint8_t* x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    *x++ = (char)Element::NUMBER;
    memcpy(&num, &indata, AMF_NUMBER_SIZE);
    swapBytes(&num, AMF_NUMBER_SIZE);
    memcpy(x, &num, AMF_NUMBER_SIZE);
    x += pktsize - AMF_HEADER_SIZE;
    
    return x;
}

/// Encode a Boolean object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
/// Although a boolean is one byte in size, swf uses 16bit short integers
/// heavily, so this value is also a short.
boost::uint8_t *
AMF::encodeBoolean(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    int pktsize = AMF_HEADER_SIZE;

    boost::uint8_t* x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    // Encode a boolean value. 0 for false, 1 for true
    *x++ = (char)Element::BOOLEAN;
    *x = flag;
    swapBytes(x, 2);
    x += sizeof(boost::uint16_t);
    
    return x;
}

/// Encode an object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeObject(const boost::uint8_t *data, int size)
{
//    GNASH_REPORT_FUNCTION;
    int pktsize = AMF_HEADER_SIZE + size;

    // Encode an XML object. The data follows a 4 byte length
    // field. (which must be big-endian)
    boost::uint8_t *x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    *x++ = Element::OBJECT;
    uint32_t num = size;
    swapBytes(&num, 4);
    memcpy(x, data, size);
    
    return x;
}

/// Encode an "Undefined" object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeUndefined()
{
//    GNASH_REPORT_FUNCTION;
    int pktsize = AMF_HEADER_SIZE;;
    boost::uint8_t* x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    *x++ = (char)Element::UNDEFINED;
//    *x = *static_cast<const char *>(flag);
    x += pktsize - AMF_HEADER_SIZE;
    
    return x;
}

/// Encode an "Undefined" object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeUnsupported()
{
//    GNASH_REPORT_FUNCTION;
    int pktsize = AMF_HEADER_SIZE;;
    boost::uint8_t* x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    *x++ = (char)Element::UNSUPPORTED;
//    *x = *static_cast<const char *>(flag);
    x += pktsize - AMF_HEADER_SIZE;
    
    return x;
}

/// Encode a Date
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *encodeDate(boost::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    int pktsize = AMF_HEADER_SIZE;;
    boost::uint8_t *x = new boost::uint8_t[pktsize + 1];
    memset(x, 0, pktsize);
    *x++ = Element::DATE;
    double num = *reinterpret_cast<const double*>(data);
    swapBytes(&num, 8);
    memcpy(x, &num, 8);
    
    return x;
}
/// Encode a "NULL" object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeNull()
{
//    GNASH_REPORT_FUNCTION;

    log_unimpl("NULL AMF object not supported yet");
    return 0;
}

/// Encode an XML object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeXMLObject(boost::uint8_t * /*data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("XML AMF objects not supported yet");
    
    return 0;
}

/// Encode a Typed Object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeTypedObject(boost::uint8_t * /* data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Typed AMF objects not supported yet");

    return 0;
}

/// Encode a Reference to an object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeReference(boost::uint8_t * /* data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Reference AMF objects not supported yet");
    
    return 0;
}

/// Encode a Movie Clip
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeMovieClip(boost::uint8_t * /*data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Movie Clip AMF objects not supported yet");
    
    return 0;
}

/// Encode an ECMA Array
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeECMAArray(boost::uint8_t * /*data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("ECMA Array AMF objects not supported yet");
    
    return 0;
}

/// Encode a long string
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeLongString(boost::uint8_t * /* data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Long String AMF objects not supported yet");
    
    return 0;
}

/// Encode a Record Set
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeRecordSet(boost::uint8_t * /* data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Reecord Set AMF objects not supported yet");
    
    return 0;
}


/// Encode a Strict Array
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
boost::uint8_t *
AMF::encodeStrictArray(boost::uint8_t * /* data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Strict Array AMF objects not supported yet");
    
    return 0;
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
boost::uint8_t *
AMF::encodeElement(Element *el)
{
//    GNASH_REPORT_FUNCTION;

    switch (el->getType()) {
      case Element::NOTYPE:
	  return 0;
	  break;
      case Element::NUMBER:
	  return encodeNumber(*(reinterpret_cast<double *>(el->getData())));
          break;
      case Element::BOOLEAN:
	  return encodeBoolean(*(reinterpret_cast<bool *>(el->getData())));
          break;
      case Element::STRING:
	  return encodeBoolean(*(reinterpret_cast<const char *>(el->getData())));
          break;
      case Element::OBJECT:
	  return encodeObject(el->getData(), el->getLength());
          break;
      case Element::MOVIECLIP:
	  return encodeMovieClip(el->getData(), el->getLength());
          break;
      case Element::NULL_VALUE: 
	  return encodeNull();
          break;
      case Element::UNDEFINED:
	  return encodeUndefined();
	  break;
      case Element::REFERENCE:
	  return encodeReference(el->getData(), el->getLength());
          break;
      case Element::ECMA_ARRAY:
	  return encodeECMAArray(el->getData(), el->getLength());
          break;
	  // The Object End gets added when creating the object, so we can jusy ignore it here.
      case Element::OBJECT_END:
          break;
      case Element::STRICT_ARRAY:
	  return encodeStrictArray(el->getData(), el->getLength());
          break;
      case Element::DATE:
//	  return encodeDate(el->getData());
          break;
      case Element::LONG_STRING:
	  return encodeLongString(el->getData(), el->getLength());
          break;
      case Element::UNSUPPORTED:
	  return encodeUnsupported();
          break;
      case Element::RECORD_SET:
	  return encodeRecordSet(el->getData(), el->getLength());
          break;
      case Element::XML_OBJECT:
	  return encodeXMLObject(el->getData(), el->getLength());
          // Encode an XML object. The data follows a 4 byte length
          // field. (which must be big-endian)
          break;
      case Element::TYPED_OBJECT:
	  return encodeTypedObject(el->getData(), el->getLength());
          break;
	  // This is a Gnash specific value
      case Element::VARIABLE:
	  return 0;
          break;
    };

    // you should never get here
    return 0;
}

#if 0
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
//    GNASH_REPORT_FUNCTION;
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

/// \brief Each header consists of the following:
///
/// * UTF string (including length bytes) - name
/// * Boolean - specifies if understanding the header is `required'
/// * Long - Length in bytes of header
/// * Variable - Actual data (including a type code)
amfhead_t *
AMF::encodeHeader(amfutf8_t *name, bool required, int nbytes, void *data)
{
//    GNASH_REPORT_FUNCTION;

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
//    GNASH_REPORT_FUNCTION;

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
//    GNASH_REPORT_FUNCTION;

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

#if 0
AMF::astype_e
AMF::extractElementHeader(void *in)
{
//    GNASH_REPORT_FUNCTION;

    return (AMF::astype_e)*(char *)in;
}

int
AMF::extractElementLength(void *in)
{
//    GNASH_REPORT_FUNCTION;

    char *x = (char *)in;
    Element::astype_e type = (Element::astype_e)*x;
    x++;                        // skip the header byte
    
    switch (type) {
      case Element::NUMBER:              // a 64 bit numeric value
          return 8;
          break;
      case Element::BOOLEAN:             // a single byte
          return 1;
          break;
      case Element::STRING:              // the length is a 2 byte value
		//FIXME, there are all kinds of byte order problems in this code.
          return (short)*(short *)x;
          break;
      case Element::OBJECT:
          return x - strchr(x, TERMINATOR);
          break;
      case Element::MOVIECLIP:
          return -1;
          log_unimpl("MovieClip AMF extractor");
          break;
      case Element::NULL_VALUE: 
          return -1;
          log_unimpl("Null AMF extractor");
          break;
      case Element::UNDEFINED:
          return 0;
          break;
      case Element::REFERENCE:
          return -1;
          log_unimpl("Reference AMF extractor");
          break;
      case Element::ECMA_ARRAY:
          return x - strchr(x, TERMINATOR);
          break;
      case Element::OBJECT_END:
          return -1;
          log_unimpl("ObjectEnd AMF extractor");
          break;
      case Element::STRICT_ARRAY:         // the length is a 4 byte value
//          return (int *)x;
          break;
      case Element::DATE:              // a 64 bit numeric value
          return 8;
          break;
      case Element::LONG_STRING:
          return -1;
          log_unimpl("LongString AMF extractor");
          break;
      case Element::UNSUPPORTED:
          return -1;
          log_unimpl("Unsupported AMF extractor");
          break;
      case Element::RECORD_SET:
          return -1;
          log_unimpl("Recordset AMF extractor");
          break;
      case Element::XML_OBJECT:           // the length is a 4 byte value
//          return (int)*(int *)x;
          break;
      case Element::TYPED_OBJECT:
          return x - strchr(x, TERMINATOR);
          break;
    };
    
    return 0;
}

char *
AMF::extractString(const boost::uint8_t *in)
{
//    GNASH_REPORT_FUNCTION;
    boost::int8_t *buf = NULL;
    boost::uint8_t *x = const_cast<boost::uint8_t *>(in);
    
    if (*x == Element::STRING) {
        x++;
        short length = *(reinterpret_cast<const short *>(x));
        swapBytes(&length, 2);
	log_debug("Encoded length of string: %hd", length);
        x += sizeof(short);
        buf = new int8_t[length+1];
        memset(buf, 0, length+1);
        memcpy(buf, x, length); /* x is not long enough */
    } else {
        log_error("Tried to extract AMF string from non String object!");
    }
    
    return reinterpret_cast<char *>(buf);
}

double
AMF::extractNumber(const boost::uint8_t *in)
{
//    GNASH_REPORT_FUNCTION;    
    boost::uint8_t *x = const_cast<uint8_t *>(in);
//    double *num = new double;
    double num = 0.0;
//    memset(num, 0, AMF_NUMBER_SIZE);
    
    if (*x == Element::NUMBER) {
        x++;
        memcpy(&num, x, AMF_NUMBER_SIZE);
        swapBytes(&num, AMF_NUMBER_SIZE);
    } else {
        log_error("Tried to extract AMF Number from non Number object!");
    }

    return num;
}

Element &
AMF::createElement(amf_element_t *el, astype_e type,
		  const std::string &name, boost::uint8_t *data, int nbytes)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Creating element %s", name.c_str());
    
    el->type = type;
    el->name = name;
    el->length = nbytes;
    el->data = data;
    return el;
}

// AMF::amf_element_t *
// AMF::createElement(amf_element_t *el, const char *name, double data)
// {
// //    GNASH_REPORT_FUNCTION;
//     string str = name;
//     return createElement(el, str, data);
// }

// AMF::amf_element_t *
// AMF::createElement(amf_element_t *el, const std::string &name, double data)
// {
// //    GNASH_REPORT_FUNCTION;
//     log_debug("Creating element %s", name.c_str());

//     el->type = AMF::NUMBER;
//     el->name = name;
//     el->length = AMF_NUMBER_SIZE;
// //    char *numptr = (char *)&data;
//     el->data = new boost::uint8_t[AMF_NUMBER_SIZE + 1];
//     memset(el->data, 0, AMF_NUMBER_SIZE + 1);
//     memcpy(el->data, &data, AMF_NUMBER_SIZE);

//     return el;
// }

AMF::amf_element_t *
AMF::createElement(amf_element_t *el, const char *name, double data)
{
//    GNASH_REPORT_FUNCTION;
    string str = name;
    return createElement(el, str, data);
}

AMF::amf_element_t *
AMF::createElement(amf_element_t *el, const std::string &name, double data)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Creating element %s", name.c_str());

    el->type = AMF::NUMBER;
    el->name = name;
    el->length = AMF_NUMBER_SIZE;
//    char *numptr = (char *)&data;
    el->data = new boost::uint8_t[AMF_NUMBER_SIZE + 1];
    memset(el->data, 0, AMF_NUMBER_SIZE + 1);
    memcpy(el->data, &data, AMF_NUMBER_SIZE);

    return el;
}

AMF::amf_element_t *
AMF::createElement(amf_element_t *el, const char *name, const char *data)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Creating element %s", name);

    el->type = AMF::STRING;
    el->name = name;
    el->length = strlen(data);
    char *str = const_cast<char *>(data);
    el->data = reinterpret_cast<boost::uint8_t *>(str);
    return el;
}

AMF::amf_element_t *
AMF::createElement(amf_element_t *el, const std::string &name, std::string &data)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Creating element %s", name.c_str());

    el->type = AMF::STRING;
    el->name = name;
    el->length = data.size();
    char *str = const_cast<char *>(data.c_str());
    el->data = reinterpret_cast<boost::uint8_t *>(str);
    return el;
}

AMF::amf_element_t *
AMF::createElement(amf_element_t *el, const char *name, bool data)
{
//    GNASH_REPORT_FUNCTION;
    string str = name;
    return createElement(el, str, data);
}

AMF::amf_element_t *
AMF::createElement(AMF::amf_element_t *el, const std::string &name, bool data)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Creating element %s", name.c_str());

    el->type = AMF::BOOLEAN;
    el->name = name;
    el->length = 1;
    el->data = new boost::uint8_t[sizeof(uint16_t)];
    memset(el->data, 0, sizeof(uint16_t));
    *el->data = data;
    return el;
}


// AMF::amf_element_t *
// createElement(AMF::amf_element_t *el, std::string &name,
// 	      boost::intrusive_ptr<gnash::as_object> &data)
// {
//     GNASH_REPORT_FUNCTION;
//     log_debug("Creating element %s", name.c_str());

//     el->type = AMF::OBJECT;
//     el->name = name;
//     el->length = sizeof(data);
//     el->data = new uint8_t[sizeof(uint16_t)];
//     memset(el->data, 0, sizeof(uint16_t));
//     memcpy(el->data, (char *)&data, el->length);
//     return el;
// }

// AMF::amf_element_t *
// createElement(AMF::amf_element_t *el, const char *name,
// 	      boost::intrusive_ptr<gnash::as_object> &data)
// {
//     GNASH_REPORT_FUNCTION;
//     string str = name;
//     return createElement(el, str, data);
// }
#endif

uint8_t *
AMF::encodeVariable(amf::Element *el)
{
//    GNASH_REPORT_FUNCTION;
    int outsize = el->getName().size() + el->getLength() + 5;
    boost::uint8_t *out = new boost::uint8_t[outsize + 2];
    memset(out, 0, outsize + 2);
    boost::uint8_t *tmpptr = out;

    // Add the length of the string for the name of the variable
    size_t length = el->getName().size();
    boost::uint16_t enclength = length;
    swapBytes(&enclength, 2);
    memcpy(tmpptr, &enclength, 2);

    // Add the actual name
    tmpptr += sizeof(uint16_t);
    memcpy(tmpptr, el->getName().c_str(), length);
    tmpptr += length;
    // Add the type of the variable's data
    *tmpptr++ = el->getType();
    // Booleans appear to be encoded weird. Just a short after
    // the type byte that's the value.
    switch (el->getType()) {
      case Element::BOOLEAN:
	  enclength = el->to_bool();
	  memcpy(tmpptr, &enclength, 2);
	  tmpptr += sizeof(uint16_t);
	  break;
      case Element::NUMBER:
	  if (el->getData()) {
	      swapBytes(el->getData(), AMF_NUMBER_SIZE);
	      memcpy(tmpptr, el->getData(), AMF_NUMBER_SIZE);
	  }
	  break;
      default:
	  enclength = el->getLength();
	  swapBytes(&enclength, 2);
	  memcpy(tmpptr, &enclength, 2);
	  tmpptr += sizeof(uint16_t);
	  // Now the data for the variable
	  memcpy(tmpptr, el->getData(), el->getLength());
    }
    
    return out;    
}

#if 0
boost::uint8_t *
AMF::encodeVariable(const char *name, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    
    int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
    boost::uint8_t *out = new uint8_t[outsize];
    boost::uint8_t *tmpptr = out;

    size_t length = strlen(name);
    short enclength = length;
    swapBytes(&enclength, 2);
    memcpy(tmpptr, &enclength, 2);
    tmpptr += 2;
    memcpy(tmpptr, name, length);
    tmpptr += length;
    *tmpptr = Element::BOOLEAN;
    tmpptr++;
    *tmpptr = flag;

    return out;    
}

boost::uint8_t *
AMF::encodeVariable(const char *name)
{
//    GNASH_REPORT_FUNCTION;
    size_t outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
    boost::uint8_t *out = new boost::uint8_t[outsize];
    boost::uint8_t *tmpptr = out;

    size_t length = strlen(name);
    short enclength = length;
    swapBytes(&enclength, 2);
    memcpy(tmpptr, &enclength, 2);
    tmpptr += 2;
    memcpy(tmpptr, name, length);
    tmpptr += length;
    *tmpptr = Element::UNDEFINED;
    tmpptr++;

    return out;    
}

boost::uint8_t *
AMF::encodeVariable(const char *name, double bignum)
{
//    GNASH_REPORT_FUNCTION;
    int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
    boost::uint8_t *out = new boost::uint8_t[outsize];
    boost::uint8_t *tmpptr = out;
    double newnum = bignum;
    char *numptr = (char *)&newnum;

    size_t length = strlen(name);
    short enclength = length;
    swapBytes(&enclength, 2);
    memcpy(tmpptr, &enclength, 2);
    tmpptr += 2;
    memcpy(tmpptr, name, length);
    tmpptr += length;
    *tmpptr = Element::NUMBER;
    tmpptr++;
//    swapBytes(numptr, AMF_NUMBER_SIZE);
    memcpy(tmpptr, numptr, AMF_NUMBER_SIZE);

    return out;    
}

uint8_t *
AMF::encodeVariable(const char *name, const char *val)
{
//    GNASH_REPORT_FUNCTION;

    int outsize = strlen(name) + strlen(val) + 5;
    boost::uint8_t *out = new boost::uint8_t[outsize];
    boost::uint8_t *tmpptr = out;

    size_t length = strlen(name);
    short enclength = length;
    swapBytes(&enclength, 2);
    memcpy(tmpptr, &enclength, 2);
    tmpptr += 2;
    memcpy(tmpptr, name, length);
    tmpptr += length;
    *tmpptr = Element::STRING;
    tmpptr++;
    length = strlen(val);
    enclength = length;
    swapBytes(&enclength, 2);
    memcpy(tmpptr, &enclength, 2);
    tmpptr += 2;
    memcpy(tmpptr, val, length);

    return out;
}

boost::uint8_t *
AMF::encodeVariable(std::string &name, std::string &val)
{
//    GNASH_REPORT_FUNCTION;

    int outsize = name.size() + val.size() + 5;
    boost::uint8_t *out = new boost::uint8_t[outsize];
    boost::uint8_t *tmpptr = out;
    short length;

    length = name.size() && 0xffff;
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    memcpy(tmpptr, name.c_str(), name.size());
    tmpptr += name.size();
    *tmpptr = Element::STRING;
    tmpptr++;
    length = val.size() && 0xffff;
    swapBytes(&length, 2);
    memcpy(tmpptr, &length, 2);
    tmpptr += 2;
    memcpy(tmpptr, val.c_str(), name.size());

    return out;
}

int
AMF::headerSize(boost::int8_t header)
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
AMF::parseHeader(boost::uint8_t *in)
{
//    GNASH_REPORT_FUNCTION;

    boost::uint8_t *tmpptr = in;
    
    log_msg (_("AMF header byte is: 0x%X"), *in);

    _amf_index = *tmpptr & AMF_INDEX_MASK;
    log_msg (_("The AMF channel index is %d"), _amf_index);
    
    _header_size = headerSize(*tmpptr++);
    log_msg (_("The header size is %d"), _header_size);

#if 1
    boost::uint8_t *hexint;
    hexint = new boost::uint8_t[(_header_size + 3) *3];
    hexify((boost::uint8_t *)hexint, (uint8_t *)in, _header_size, false);
    log_msg(_("The packet head is: 0x%s"), hexint);
#endif
    if (_header_size >= 4) {
        hexify((boost::uint8_t *)hexint, (boost::uint8_t *)tmpptr, 3, false);
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;
        log_msg(_("The mystery word is: %d or 0x%s"), _mystery_word, hexint);
    }

    if (_header_size >= 8) {
        hexify((boost::uint8_t *)hexint, (boost::uint8_t *)tmpptr, 3, false);
        _total_size = *tmpptr++;
        _total_size = (_total_size << 12) + *tmpptr++;
        _total_size = (_total_size << 8) + *tmpptr++;
        _total_size = _total_size & 0xffffff;
        log_msg(_("The body size is: %d, or 0x%s"), _total_size, hexint);
        _amf_data = new uint8_t(_total_size+1);
        _seekptr = _amf_data;
//        memset(_amf_data, 0, _total_size+1);
    }

    if (_header_size >= 8) {
        hexify((boost::uint8_t *)hexint, (boost::uint8_t *)tmpptr, 1, false);
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
        hexify((boost::uint8_t *)hexint, (boost::uint8_t *)tmpptr, 3, false);
        _src_dest = *(reinterpret_cast<amfsource_e *>(tmpptr));
        tmpptr += sizeof(unsigned int);
        log_msg(_("The source/destination is: %d, or 0x%s"), _src_dest, hexint);
    }

    return _packet_size;
}

boost::uint8_t *
AMF::addPacketData(boost::uint8_t *data, int bytes)
{
//    GNASH_REPORT_FUNCTION;
    memcpy(_seekptr, data, bytes);
    _seekptr+=bytes;
    return _seekptr;
}

int
AMF::parseBody()
{
//    GNASH_REPORT_FUNCTION;

//    return parseBody(_amf_data, _total_size);
}

uint8_t *
AMF::extractElement(Element &el, boost::uint8_t *in)
{
//    GNASH_REPORT_FUNCTION;

    boost::uint8_t *tmpptr;

//    uint8_t hexint[(bytes*2)+1];
    boost::uint8_t* hexint;

    short length;

    if (in == 0) {
        log_error(_("AMF body input data is NULL"));
        return 0;
    }
//     if (reinterpret_cast<int>(el) == 0)) {
//         log_error(_("Got NULL instead of amf_element_t!"));
//         return 0;
//     }

#if 0
    hexint =  (boost::uint8_t*) malloc((bytes * 3) + 12);
    hexify((boost::uint8_t *)hexint, (boost::uint8_t *)in, bytes, true);
    log_msg(_("The packet body is: 0x%s"), hexint);
#endif

    tmpptr = in;
    
// All elements look like this:
// the first two bytes is the length of name of the element
// Then the next bytes are the element name
// After the element name there is a type byte. If it's a Number type, then 8 bytes are read
// If it's a String type, then there is a count of characters, then the string value    
    
    while (tmpptr  <= (in + bytes)) {
        memset(buffer, 0, sizeof(buffer));	//FIXME, slow
        // Check the type of the element data
        char type = *(Element::astype_e *)tmpptr;
        tmpptr++;                        // skip the header byte

        switch ((Element::astype_e)type) {
          case Element::NUMBER:
//              memcpy(buffer, tmpptr, 8);
              tmpptr += 8;
              continue;
              break;
          case Element::BOOLEAN:
          case Element::STRING:
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
          case Element::OBJECT:
              do {
                  tmpptr = extractVariable(&el, tmpptr);
              } while (el.type != Element::OBJECT_END);
              break;
          case Element::MOVIECLIP:
          case Element::NULL_VALUE: 
          case Element::UNDEFINED:
          case Element::REFERENCE:
          case Element::ECMA_ARRAY:
          case Element::OBJECT_END:
          case Element::STRICT_ARRAY:
          case Element::DATE:
          case Element::LONG_STRING:
          case Element::UNSUPPORTED:
          case Element::RECORD_SET:
          case Element::XML_OBJECT:
          case Element::TYPED_OBJECT:
          default:
	    log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)type);
              return 0;
        }
    }

    free(hexint);

    return 0;
}
#endif

boost::uint8_t *
AMF::extractVariable(Element *el, boost::uint8_t *in)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::uint8_t buffer[AMF_PACKET_SIZE];
    boost::uint8_t *tmpptr = in;
    boost::int16_t length;

//     if (el == 0) {
// 	return 0;
//     }
    
    el->clear();
    
    memset(buffer, 0, AMF_PACKET_SIZE);
    // @@ casting generic pointers to bigger types may be dangerous
    //    due to memory alignment constraints
    length = *((short *)tmpptr);
    swapBytes(&length, 2);
//    length = ntohs((*(const short *)tmpptr) & 0xffff);
    el->setLength(length);
    if (length == 0) {
        if (*(tmpptr+2) == Element::OBJECT_END) {
//            log_msg(_("End of Object definition"));
            el->setLength(0);
            el->setType(Element::OBJECT_END);
            tmpptr+=3;
            return tmpptr;
        }
	return 0;
    }
    
#if 0
    uint8_t hexint[AMF_PACKET_SIZE];
    hexify((uint8_t *)hexint, (uint8_t *)tmpptr, length*3, true);
    log_msg(_("The element is: 0x%s"), hexint);
#endif
    tmpptr += 2;
    // get the name of the element
    if (length > 0) {
	if (length > 20000) {
	    log_error("Length field corrupted! parsed value is: %hd", length);
	    return 0;
	}
	
//        log_msg(_("AMF element length is: %d"), length);
        memcpy(buffer, tmpptr, length);
        el->setName(buffer);
        tmpptr += length;
    }
    
//    log_msg(_("AMF element name is: %s"), buffer);
    Element::astype_e type = (Element::astype_e)((*tmpptr++) & 0xff);

    if (type <= Element::TYPED_OBJECT) {
//        log_msg(_("AMF type is: %s"), Element::astype_str[(int)type]);
	el->setType(type);
    }
    
    switch (type) {
      case Element::NUMBER:
      {
          memcpy(buffer, tmpptr, AMF_NUMBER_SIZE);
          swapBytes(buffer, AMF_NUMBER_SIZE);
	  uint8_t* tmp = new uint8_t[AMF_NUMBER_SIZE+1];
	  memset(tmp, 0, AMF_NUMBER_SIZE+1);
	  memcpy(tmp, buffer, AMF_NUMBER_SIZE);
	  el->setData(tmp);
#if 0
          uint8_t hexint[AMF_NUMBER_SIZE*3];
          hexify((uint8_t *)hexint, (uint8_t *)buffer,
		 AMF_NUMBER_SIZE, false);
          log_msg(_("Number \"%s\" is: 0x%s"), el->getName().c_str(), hexint);
//          double *num = extractNumber(tmpptr);
#endif
          tmpptr += 8;
	  el->setLength(AMF_NUMBER_SIZE);
          break;
      }
      case Element::BOOLEAN:
      {
	  bool sheet = *tmpptr;
          el->init(sheet);
	  tmpptr += 1;
	  break;
    }
      case Element::STRING:
	  // extractString returns a printable char *
	  length = ntohs((*(const short *)tmpptr) & 0xffff);
          tmpptr += sizeof(short);
          el->setLength(length);
	  boost::uint8_t *str;
	  str = new boost::uint8_t[length + 1];
 	  memset(str, 0, length + 1);
 	  memcpy(str, tmpptr, length);
	  el->setData(str);
// 	  string v(reinterpret_cast<const char *>(str) + 3, (int)length);
// 	  log_msg(_("Variable \"%s\" is: %s"), el->getName().c_str(), v.c_str());
          tmpptr += length;
          break;
      case Element::OBJECT:
  	  while (*(tmpptr++) != Element::OBJECT_END) {
	      log_msg("Look for end of object...");
  	  }
	  
	  break;
      case Element::MOVIECLIP:
      case Element::NULL_VALUE:
	  el->makeUndefined();
	  break;
      case Element::UNDEFINED:
	  el->makeUndefined();
          break;
      case Element::REFERENCE:
      case Element::ECMA_ARRAY:
			// FIXME this shouldn't fall thru
      case Element::OBJECT_END:
//          log_msg(_("End of Object definition"));
	  el->makeObjectEnd();
          break;
      case Element::TYPED_OBJECT:
	  el->makeTypedObject(tmpptr, 0);
          break;
      case Element::STRICT_ARRAY:
      case Element::DATE:
	  el->makeDate(tmpptr);
          break;
      case Element::LONG_STRING:
      case Element::UNSUPPORTED:
      case Element::RECORD_SET:
      case Element::XML_OBJECT:
      default:
          log_unimpl(_("astype_e of value: %x"), (int)type);
          break;
    }
    
    return tmpptr; // we're dropping const specification
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
