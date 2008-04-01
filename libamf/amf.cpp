// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>

#if defined(_WIN32) || defined(WIN32)
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

#include "log.h"
#include "buffer.h"
#include "amf.h"
#include "network.h"
#include "element.h"
#include "amfutf8.h"
#include <boost/cstdint.hpp> // for boost::?int??_t

using namespace std;
using namespace gnash;

namespace amf 
{

// These are used to print more intelligent debug messages
const char *amftype_str[] = {
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

AMF::AMF() 
#if 0
      _amf_index(0),
      _header_size(0),
      _total_size(0),
      _packet_size(0),
      _amf_data(0),
      _seekptr(0),
#endif
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
	    Network::byte_t c0;
	    Network::byte_t c1;
	} c;
    } u;
	   
    u.s = 1;
    if (u.c.c0 == 0) {
	// Big-endian machine: do nothing
	return word;
    }

    // Little-endian machine: byte-swap the word

    // A conveniently-typed pointer to the source data
    Network::byte_t *x = static_cast<Network::byte_t *>(word);

    switch (size) {
    case 2: // 16-bit integer
      {
	Network::byte_t c;
	c=x[0]; x[0]=x[1]; x[1]=c;
	break;
      }
    case 4: // 32-bit integer
      {
	Network::byte_t c;
	c=x[0]; x[0]=x[3]; x[3]=c;
	c=x[1]; x[1]=x[2]; x[2]=c;
	break;
      }
    case 8: // 64-bit integer
      {
	Network::byte_t c;
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
AMF::parseAMF(Network::byte_t * /* in */)
{
//    GNASH_REPORT_FUNCTION;

#if 0
    Network::byte_t *x = in;

    while (*x != Element::OBJECT_END) {
        x = readElements(x);
    }
    return true;
#endif
    return false;
}

vector<AMF::amf_element_t *> *
AMF::readElements(Network::byte_t *in)
{
    GNASH_REPORT_FUNCTION;
    Network::byte_t *x = in;
    astype_e type = (astype_e)*x;
    bool boolshift;
    const char *mstr = NULL;
    double *num;
    double nanosecs;
    short length;

    vector<AMF::amf_element_t> list;
    AMF::amf_element_t el;
    
    
    log_debug(_("Type is %s"), astype_str[type]);

//    x++;                        // skip the type byte
    switch (type) {
      case Element::NUMBER:
	  // AMF numbers are 64-bit big-endian integers.
	  num = extractNumber(x);
//	  log_debug(_("Number is " AMFNUM_F), num));
          break;
      case Element::BOOLEAN:
          boolshift = *x;
          log_debug(_("Boolean is %d"), boolshift);
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
		  log_debug(_("String is %s"), mstr);
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
          log_debug(_("Undefined element"));
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
//          log_debug(_("Date is " AMFNUM_F " nanoseconds"), nanosecs);
          break;
      case Element::LONG_STRING:
//          int length = *(short *)swapBytes(x, 4);
          x+=4;                  // skip the length bytes
//        mstr = new char[length+1];
//          memcpy(mstr, x, length);
//          log_debug(_("String is %s"), mstr);
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

/// Encode a 64 bit number
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeNumber(double indata)
{
//    GNASH_REPORT_FUNCTION;
    double num;
    // Encode the data as a 64 bit, big-endian, numeric value
    Buffer *buf = new Buffer(AMF_NUMBER_SIZE + AMF_HEADER_SIZE);
    buf->append(Element::NUMBER);
    num = indata;
    swapBytes(&num, AMF_NUMBER_SIZE);
    buf->append(num);
    
    return buf;
}

/// Encode a Boolean object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
/// Although a boolean is one byte in size, swf uses 16bit short integers
/// heavily, so this value is also a short.
Buffer *
AMF::encodeBoolean(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    // Encode a boolean value. 0 for false, 1 for true
    Buffer *buf = new Buffer(AMF_HEADER_SIZE);
    buf->append(Element::BOOLEAN);
    bool x = flag;
    swapBytes(&x, 2);
    buf->append(x);
    
    return buf;
}

/// Encode an object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeObject(Network::byte_t *data, int size)
{
//    GNASH_REPORT_FUNCTION;
    // Encode an XML object. The data follows a 4 byte length
    // field. (which must be big-endian)
    Buffer *buf = new Buffer(AMF_HEADER_SIZE + size);
    buf->append(Element::OBJECT);
    boost::uint32_t num = size;
    swapBytes(&num, 4);
    buf->append(num);
    buf->append(data, size);
    
    return buf;
}

/// Encode an "Undefined" object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeUndefined()
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(AMF_HEADER_SIZE);
    buf->append(Element::UNDEFINED);
    
    return buf;
}

/// Encode an "Undefined" object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeUnsupported()
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(AMF_HEADER_SIZE);
    buf->append(Element::UNSUPPORTED);
    
    return buf;
}

/// Encode a Date
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *encodeDate(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(AMF_HEADER_SIZE);
    buf->append(Element::DATE);
    double num = *reinterpret_cast<const double*>(data);
    swapBytes(&num, 8);
    buf->append(num);
    
    return buf;
}
/// Encode a "NULL" object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
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
Buffer *
AMF::encodeXMLObject(Network::byte_t * /*data */, int /* size */)
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
Buffer *
AMF::encodeTypedObject(Network::byte_t * /* data */, int /* size */)
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
Buffer *
AMF::encodeReference(Network::byte_t * /* data */, int /* size */)
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
Buffer *
AMF::encodeMovieClip(Network::byte_t * /*data */, int /* size */)
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
Buffer *
AMF::encodeECMAArray(Network::byte_t * /*data */, int /* size */)
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
Buffer *
AMF::encodeLongString(Network::byte_t * /* data */, int /* size */)
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
Buffer *
AMF::encodeRecordSet(Network::byte_t * /* data */, int /* size */)
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
Buffer *
AMF::encodeStrictArray(Network::byte_t * /* data */, int /* size */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Strict Array AMF objects not supported yet");
    
    return 0;
}

/// Encode a string object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeString(const string &str)
{
    GNASH_REPORT_FUNCTION;
    boost::uint16_t length;
    
    Buffer *buf = new Buffer(str.size() + AMF_HEADER_SIZE);
    buf->append(Element::STRING);
    // when a string is stored in an element, we add a NULL terminator so
    // it can be printed by to_string() efficiently. The NULL terminator
    // doesn't get written when encoding a string as it has a byte count
    // instead.
    length = str.size() - 1;
    log_debug("Encoded data size is going to be %d", length);
    swapBytes(&length, 2);
    buf->append(length);
    buf->append(str);
    
    return buf;
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
Buffer *
AMF::encodeElement(Element *el)
{
//    GNASH_REPORT_FUNCTION;

    switch (el->getType()) {
      case Element::NOTYPE:
	  return 0;
	  break;
      case Element::NUMBER:
	  return encodeNumber(el->to_number());
          break;
      case Element::BOOLEAN:
	  return encodeBoolean(el->to_bool());
          break;
      case Element::STRING:
	  return encodeString(el->to_string());
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
      case Element::FUNCTION:
	  return 0;
          break;
    };

    // you should never get here
    return 0;
}

#if 0
/// Encode an array of elements. 
///
/// @return a binary AMF packet in big endian format (header,data)

/// @return a newly allocated byte array.
/// to be deleted by caller using delete [] operator, or NULL
///
vector<Buffer> *
AMF::encodeElement(vector<amf::Element *> &data)
{
    GNASH_REPORT_FUNCTION;

    int size = 0;
    bool pad = false;

    // Calculate how large the buffer has to be.
    vector<amf::Element *>::iterator ait;
    cerr << "# of Elements in file: " << data.size() << endl;
    for (ait = data.begin(); ait != data.end(); ait++) {
	amf::Element *el = (*(ait));
	size += el->getLength() + AMF_HEADER_SIZE;
//        el->dump();
    }
    vector<Network::byte_t> *vec = new vector<Network::byte_t>;
    Network::byte_t* ptr = new Network::byte_t[size + 1];
    memset(ptr, 0, size + 1);
    
//    Network::byte_t *x = ptr;
    size = 0;
    for (ait = data.begin(); ait != data.end(); ait++) {
	amf::Element *el = (*(ait));
//	el->dump();
	Network::byte_t *tmp = encodeElement(el);
	Network::byte_t *y = tmp;
#if 0
	Network::byte_t *hexint;
	hexint = new Network::byte_t[(el->getLength() + 4) *3];
	hexify((Network::byte_t *)hexint, (Network::byte_t *)tmp,
	       el->getLength() + AMF_HEADER_SIZE, true);
	log_debug(_("The packet head is: 0x%s"), hexint);
#endif
	// The 'pad' in this case is a serious hack. I think it
	// may be an artifact, but one guess is it's a 16bit word
	// aligned memory segment due to some ancient heritage in
	// the other player, so after a 3 byte bool, and two 9 byte
	// numbers, another byte is needed for padding.
	// My guess is the pattern of boolean->number->number->methodname
	// is a function block ID. I need to dp more testing with a
	// wider variety of sef movies that use LocalConnection to
	// really tell.
	if (el->getType() == Element::NUMBER) {
	    size = AMF_NUMBER_SIZE + 1;
	    pad = true;
	}
	if (el->getType() == Element::STRING) {
	    if (pad) {
		vec->push_back('\0');
		pad = false;
	    }
	    size = el->getLength() + AMF_HEADER_SIZE;
	}
	if (el->getType() == Element::FUNCTION) {
	    // _children
	}
	if (el->getType() == Element::BOOLEAN) {
	    size = 3;
	}
 	for (int i=0; i<size; i++) {
	    Network::byte_t c = *y;
	    y++;
//	    printf("0x%x(%c) ", c, (isalpha(c)) ? c : '.');
	    vec->push_back(c);
	}
//	delete[] tmp;
    }
    return vec;
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
AMF::extractString(const Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;
    boost::int8_t *buf = NULL;
    Network::byte_t *x = const_cast<Network::byte_t *>(in);
    
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
AMF::extractNumber(const Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;    
    Network::byte_t *x = const_cast<uint8_t *>(in);
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
		  const std::string &name, Network::byte_t *data, int nbytes)
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
//     el->data = new Network::byte_t[AMF_NUMBER_SIZE + 1];
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
    el->data = new Network::byte_t[AMF_NUMBER_SIZE + 1];
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
    el->data = reinterpret_cast<Network::byte_t *>(str);
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
    el->data = reinterpret_cast<Network::byte_t *>(str);
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
    el->data = new Network::byte_t[sizeof(uint16_t)];
    memset(el->data, 0, sizeof(uint16_t));
    *el->data = data;
    return el;
}

#endif

Buffer *
AMF::encodeVariable(amf::Element *el)
{
    GNASH_REPORT_FUNCTION;
    
    size_t outsize = el->getNameSize() + el->getLength() + AMF_VAR_HEADER_SIZE;

    Buffer *buf = new Buffer(outsize);
//     Network::byte_t *out = new Network::byte_t[outsize + 4]; // why +4 here ?
//     Network::byte_t *end = out + outsize+4; // why +4 ?

//    memset(out, 0, outsize + 2); // why +2 here ?
//    Network::byte_t *tmpptr = out;

    // Add the length of the string for the name of the variable
    size_t length = el->getNameSize();
    boost::uint16_t enclength = length;
    swapBytes(&enclength, 2);
    buf->copy(enclength);

    buf->append(el->getName());
    // Add the type of the variable's data
    buf->append(el->getType());
    // Booleans appear to be encoded weird. Just a short after
    // the type byte that's the value.
    switch (el->getType()) {
      case Element::BOOLEAN:
	  enclength = el->to_bool();
	  buf->append(enclength);
	  break;
      case Element::NUMBER:
	  if (el->getData()) {
	      swapBytes(el->getData(), AMF_NUMBER_SIZE);
	      buf->append(el->getData(), AMF_NUMBER_SIZE);
	  }
	  break;
      default:
	  enclength = el->getLength();
	  swapBytes(&enclength, 2);
	  buf->append(enclength);
	  // Now the data for the variable
	  buf->append(el->getData(), el->getLength());
    }
    
    return buf;
}

// #if 0
// Network::byte_t *
// AMF::encodeVariable(const char *name, bool flag)
// {
// //    GNASH_REPORT_FUNCTION;
    
//     int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
//     Network::byte_t *out = new uint8_t[outsize];
//     Network::byte_t *tmpptr = out;

//     size_t length = strlen(name);
//     short enclength = length;
//     swapBytes(&enclength, 2);
//     memcpy(tmpptr, &enclength, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, name, length);
//     tmpptr += length;
//     *tmpptr = Element::BOOLEAN;
//     tmpptr++;
//     *tmpptr = flag;

//     return out;    
// }

// Network::byte_t *
// AMF::encodeVariable(const char *name)
// {
// //    GNASH_REPORT_FUNCTION;
//     size_t outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
//     Network::byte_t *out = new Network::byte_t[outsize];
//     Network::byte_t *tmpptr = out;

//     size_t length = strlen(name);
//     short enclength = length;
//     swapBytes(&enclength, 2);
//     memcpy(tmpptr, &enclength, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, name, length);
//     tmpptr += length;
//     *tmpptr = Element::UNDEFINED;
//     tmpptr++;

//     return out;    
// }

// Network::byte_t *
// AMF::encodeVariable(const char *name, double bignum)
// {
// //    GNASH_REPORT_FUNCTION;
//     int outsize = strlen(name) + AMF_NUMBER_SIZE + 5;
//     Network::byte_t *out = new Network::byte_t[outsize];
//     Network::byte_t *tmpptr = out;
//     double newnum = bignum;
//     char *numptr = (char *)&newnum;

//     size_t length = strlen(name);
//     short enclength = length;
//     swapBytes(&enclength, 2);
//     memcpy(tmpptr, &enclength, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, name, length);
//     tmpptr += length;
//     *tmpptr = Element::NUMBER;
//     tmpptr++;
// //    swapBytes(numptr, AMF_NUMBER_SIZE);
//     memcpy(tmpptr, numptr, AMF_NUMBER_SIZE);

//     return out;    
// }

// uint8_t *
// AMF::encodeVariable(const char *name, const char *val)
// {
// //    GNASH_REPORT_FUNCTION;

//     int outsize = strlen(name) + strlen(val) + 5;
//     Network::byte_t *out = new Network::byte_t[outsize];
//     Network::byte_t *tmpptr = out;

//     size_t length = strlen(name);
//     short enclength = length;
//     swapBytes(&enclength, 2);
//     memcpy(tmpptr, &enclength, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, name, length);
//     tmpptr += length;
//     *tmpptr = Element::STRING;
//     tmpptr++;
//     length = strlen(val);
//     enclength = length;
//     swapBytes(&enclength, 2);
//     memcpy(tmpptr, &enclength, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, val, length);

//     return out;
// }

// Network::byte_t *
// AMF::encodeVariable(std::string &name, std::string &val)
// {
// //    GNASH_REPORT_FUNCTION;

//     int outsize = name.size() + val.size() + 5;
//     Network::byte_t *out = new Network::byte_t[outsize];
//     Network::byte_t *tmpptr = out;
//     short length;

//     length = name.size() && 0xffff;
//     swapBytes(&length, 2);
//     memcpy(tmpptr, &length, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, name.c_str(), name.size());
//     tmpptr += name.size();
//     *tmpptr = Element::STRING;
//     tmpptr++;
//     length = val.size() && 0xffff;
//     swapBytes(&length, 2);
//     memcpy(tmpptr, &length, 2);
//     tmpptr += 2;
//     memcpy(tmpptr, val.c_str(), name.size());

//     return out;
// }

// Network::byte_t *
// AMF::addPacketData(Network::byte_t *data, int bytes)
// {
// //    GNASH_REPORT_FUNCTION;
//     memcpy(_seekptr, data, bytes);
//     _seekptr+=bytes;
//     return _seekptr;
// }

// int
// AMF::parseBody()
// {
// //    GNASH_REPORT_FUNCTION;

// //    return parseBody(_amf_data, _total_size);
// }
// #endif

Element *
AMF::extractAMF(Network::byte_t *in)
{
    GNASH_REPORT_FUNCTION;

    Element *el = new Element;    
    Network::byte_t *tmpptr;
    boost::uint16_t length;

    if (in == 0) {
        log_error(_("AMF body input data is NULL"));
        return 0;
    }
//     if (reinterpret_cast<int>(el) == 0)) {
//         log_error(_("Got NULL instead of amf_element_t!"));
//         return 0;
//     }

    tmpptr = in;
    
// All elements look like this:
// the first two bytes is the length of name of the element
// Then the next bytes are the element name
// After the element name there is a type byte. If it's a Number type, then 8 bytes are read
// If it's a String type, then there is a count of characters, then the string value    
    
    // Check the type of the element data
    Element::amf_type_e type = *(Element::amf_type_e *)tmpptr;
    tmpptr++;                        // skip the header byte
    
    switch (type) {
      case Element::NUMBER:
	  el->makeNumber(tmpptr);
	  tmpptr += AMF_NUMBER_SIZE; // all numbers are 8 bit big endian
	  break;
      case Element::BOOLEAN:
	  el->makeBoolean(tmpptr);
	  tmpptr += sizeof(boost::uint16_t); // although a bool is one byte, it's stored as a short
	  break;
      case Element::STRING:
	  // get the length of the name
	  length = ntohs((*(boost::uint16_t *)tmpptr) & 0xffff);
	  tmpptr += sizeof(boost::uint16_t);
	  log_debug(_("AMF String length is: %d"), length);
	  if (length > 0) {
	      // get the name of the element
	      el->makeString(tmpptr, length);
	      log_debug(_("AMF String is: %s"), el->to_string());
	      tmpptr += length;
	  } else {
	      el->setType(Element::STRING);
	  };
	  break;
      case Element::OBJECT:
	  do {
	      el = extractVariable(tmpptr);
	      tmpptr += el->totalsize();
	  } while (el->getType() != Element::OBJECT_END);
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
//	  log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)type);
	  return 0;
    }
    
    return el;
}

Element *
AMF::extractVariable(Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;
    Element *el;
    boost::uint16_t length;

    Network::byte_t *tmpptr = in;
    length = *(reinterpret_cast<boost::uint16_t *>(in));
    tmpptr += sizeof(boost::uint16_t);
    swapBytes(&length, sizeof(boost::uint16_t));
    
    if (length <= 0) {
	if (*(in+2) == Element::OBJECT_END) {
	    log_debug(_("End of Object definition"));
            el->setType(Element::OBJECT_END);
            return el;
        }
	return 0;
    }    
    // get the name of the element, the length of which we just decoded
    if (length > 0) {
        log_debug(_("AMF element length is: %d"), length);
        el->setName(tmpptr, length);
//	el->setName("fixme");
	tmpptr += length;
	log_debug(_("AMF element name is: %s"), el->getName());
    }
    
    // get the type of the element, which is a single byte
    Element::amf_type_e type = (Element::amf_type_e)(*(tmpptr) & 0xff);
    tmpptr++;
    if (type <= Element::TYPED_OBJECT) {
        log_debug(_("AMF type is: %s"), amftype_str[(int)type]);
	el->setType(type);
    }
    
    switch (type) {
      case Element::NUMBER: {
          swapBytes(tmpptr, AMF_NUMBER_SIZE);
	  el->makeNumber(tmpptr);
          tmpptr += AMF_NUMBER_SIZE;
          break;
      }
      case Element::BOOLEAN:
      {
	  bool sheet = *(reinterpret_cast<bool *>(tmpptr));
          el->makeBoolean(sheet);
	  tmpptr += 1;
	  break;
    }
      case Element::STRING:
	  // extractString returns a printable char *
	      length = ntohs((*(const boost::uint16_t *)tmpptr) & 0xffff);
          tmpptr += sizeof(boost::uint16_t);
	  el->makeString(tmpptr, length);
// 	  string v(reinterpret_cast<const char *>(str) + 3, (int)length);
// 	  log_debug(_("Variable \"%s\" is: %s"), el->getName().c_str(), v.c_str());
          tmpptr += length;
          break;
      case Element::OBJECT:
  	  while (*(tmpptr++) != Element::OBJECT_END) {
	      log_debug("Look for end of object...");
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
//          log_debug(_("End of Object definition"));
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
          log_unimpl(_("amf_type_e of value: %x"), (int)type);
          break;
    }
    
    return el;
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
