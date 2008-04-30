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
    : _totalsize(0)
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
swapBytes(void *word, size_t size)
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
    Buffer *buf = new Buffer(AMF0_NUMBER_SIZE + AMF_HEADER_SIZE);
    buf->append(Element::NUMBER_AMF0);
    num = indata;
    swapBytes(&num, AMF0_NUMBER_SIZE);
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
    buf->append(Element::BOOLEAN_AMF0);
    boost::uint16_t x = flag;
    swapBytes(&x, 2);
    buf->append(x);
    
    return buf;
}

/// Encode the end of an object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeObjectEnd()
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(1);
    buf->append(TERMINATOR);

    return buf;
}

#if 0
/// Encode an object
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeObject(Element *el)
{
    GNASH_REPORT_FUNCTION;
//    AMF amf_obj;
    
    for (size_t i=0; i< el->propertiesSize(); i++) {
//	Buffer *var = amf_obj.encodeProperty();
//	Element *child = el[i];
#if 0
	Buffer *buf = new Buffer(AMF_HEADER_SIZE + size);
	buf->append(Element::OBJECT);
	boost::uint32_t num = size;
	swapBytes(&num, 4);
	buf->append(num);
	buf->append(data, size);
	
	return buf;
#endif
//	child.dump();
    }

}
#endif

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
    buf->append(Element::UNDEFINED_AMF0);
    
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
    buf->append(Element::UNSUPPORTED_AMF0);
    
    return buf;
}

/// Encode a Date
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeDate(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(AMF_HEADER_SIZE);
    buf->append(Element::DATE_AMF0);
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
AMF::encodeXMLObject(Network::byte_t * /*data */, size_t /* size */)
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
AMF::encodeTypedObject(Network::byte_t * /* data */, size_t /* size */)
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
AMF::encodeReference(Network::byte_t * /* data */, size_t /* size */)
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
AMF::encodeMovieClip(Network::byte_t * /*data */, size_t /* size */)
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
AMF::encodeECMAArray(Network::byte_t * /*data */, size_t /* size */)
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
AMF::encodeLongString(Network::byte_t * /* data */, size_t /* size */)
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
AMF::encodeRecordSet(Network::byte_t * /* data */, size_t /* size */)
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
AMF::encodeStrictArray(Network::byte_t * /* data */, size_t /* size */)
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
    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str.c_str()));
    return encodeString(ptr, str.size());
}

Buffer *
AMF::encodeString(Network::byte_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
    boost::uint16_t length;
    
    Buffer *buf = new Buffer(size + AMF_HEADER_SIZE);
    buf->append(Element::STRING_AMF0);
    // when a string is stored in an element, we add a NULL terminator so
    // it can be printed by to_string() efficiently. The NULL terminator
    // doesn't get written when encoding a string as it has a byte count
    // instead.
    length = size;
//    log_debug("Encoded data size is going to be %d", length);
    swapBytes(&length, 2);
    buf->append(length);
    buf->append(data, size);
    
    return buf;
}

/// Encode a NULL string object, which is a string with no data.
///
/// @return a binary AMF packet in big endian format (header,data) which
/// needs to be deleted[] after being used.
///
Buffer *
AMF::encodeNullString()
{
//    GNASH_REPORT_FUNCTION;
    boost::uint16_t length;
    
    Buffer *buf = new Buffer(AMF_HEADER_SIZE);
    buf->append(Element::STRING_AMF0);
    // when a string is stored in an element, we add a NULL terminator so
    // it can be printed by to_string() efficiently. The NULL terminator
    // doesn't get written when encoding a string as it has a byte count
    // instead.
    length = 0;
    buf->append(length);
    
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
    Buffer *buf = 0;
    Buffer *tmp;
    
    size_t outsize = el->getNameSize() + AMF_VAR_HEADER_SIZE;
    buf = new Buffer(outsize);
    buf->clear();		// FIXME: temporary, makes buffers cleaner in gdb.
    // If the name field is set, it's a "variable", followed by the data
    if (el->getName()) {
	// Add the length of the string for the name of the variable
	size_t length = el->getNameSize();
	boost::uint16_t enclength = length;
	swapBytes(&enclength, 2);
	buf->append(enclength);
	// Now the name itself
	string name = el->getName();
	if (name.size() > 0) {
	    buf->append(name);
	}
    }

    // Encode the element's data
    switch (el->getType()) {
      case Element::NOTYPE:
	  return 0;
	  break;
      case Element::NUMBER_AMF0:
	  tmp = encodeNumber(el->to_number());
          break;
      case Element::BOOLEAN_AMF0:
	  tmp = encodeBoolean(el->to_bool());
          break;
      case Element::STRING_AMF0:
	  tmp = encodeString(el->getData(), el->getLength()-1);
	  break;
      case Element::OBJECT_AMF0:
	  tmp = el->encode();
          break;
      case Element::MOVIECLIP_AMF0:
	  tmp = encodeMovieClip(el->getData(), el->getLength());
          break;
      case Element::NULL_AMF0:
	  tmp = encodeNull();
          break;
      case Element::UNDEFINED_AMF0:
	  tmp = encodeUndefined();
	  break;
      case Element::REFERENCE_AMF0:
	  tmp = encodeReference(el->getData(), el->getLength());
          break;
      case Element::ECMA_ARRAY_AMF0:
	  tmp = encodeECMAArray(el->getData(), el->getLength());
          break;
	  // The Object End gets added when creating the object, so we can jusy ignore it here.
      case Element::OBJECT_END_AMF0:
	  tmp = encodeObjectEnd();
          break;
      case Element::STRICT_ARRAY_AMF0:
	  tmp = encodeStrictArray(el->getData(), el->getLength());
          break;
      case Element::DATE_AMF0:
	  tmp = encodeDate(el->getData());
          break;
      case Element::LONG_STRING_AMF0:
	  tmp = encodeLongString(el->getData(), el->getLength());
          break;
      case Element::UNSUPPORTED_AMF0:
	  tmp = encodeUnsupported();
          break;
      case Element::RECORD_SET_AMF0:
	  tmp = encodeRecordSet(el->getData(), el->getLength());
          break;
      case Element::XML_OBJECT_AMF0:
	  tmp = encodeXMLObject(el->getData(), el->getLength());
          // Encode an XML object. The data follows a 4 byte length
          // field. (which must be big-endian)
          break;
      case Element::TYPED_OBJECT_AMF0:
//	  tmp = encodeTypedObject(el->getData(), el->getLength());
          break;
// 	  // This is a Gnash specific value
//       case Element::VARIABLE:
//       case Element::FUNCTION:
//          break;
    };

    buf->append(tmp);
//    log_debug("Encoded buf size is %d", buf->size());
    delete tmp;

    return buf;
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
//    GNASH_REPORT_FUNCTION;

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
	    size = AMF0_NUMBER_SIZE + 1;
	    pad = true;
	}
	if (el->getType() == Element::STRING) {
	    if (pad) {
		vec->push_back('\0');
		pad = false;
	    }
	    size = el->getLength() + AMF_HEADER_SIZE;
	}
// 	if (el->getType() == Element::FUNCTION) {
// 	    // _properties
// 	}
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

Buffer *
AMF::encodeProperty(amf::Element *el)
{
//    GNASH_REPORT_FUNCTION;
    
    size_t outsize = el->getNameSize() + el->getLength() + AMF_VAR_HEADER_SIZE;

    Buffer *buf = new Buffer(outsize);
    _totalsize += outsize;
//     Network::byte_t *out = new Network::byte_t[outsize + 4]; // why +4 here ?
//     Network::byte_t *end = out + outsize+4; // why +4 ?

//    memset(out, 0, outsize + 2); // why +2 here ?
//    Network::byte_t *tmpptr = out;

    // Add the length of the string for the name of the variable
    size_t length = el->getNameSize();
    boost::uint16_t enclength = length;
    swapBytes(&enclength, 2);
    buf->copy(enclength);

    if (el->getName()) {
	string name = el->getName();
	if (name.size() > 0) {
	    buf->append(name);
	}
    }

    // Add the type of the variable's data
    buf->append(el->getType());
    // Booleans appear to be encoded weird. Just a short after
    // the type byte that's the value.
    switch (el->getType()) {
      case Element::BOOLEAN_AMF0:
	  enclength = el->to_bool();
	  buf->append(enclength);
	  break;
      case Element::NUMBER_AMF0:
	  if (el->getData()) {
	      swapBytes(el->getData(), AMF0_NUMBER_SIZE);
	      buf->append(el->getData(), AMF0_NUMBER_SIZE);
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

Element *
AMF::extractAMF(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    return extractAMF(buf->reference());
}

Element *
AMF::extractAMF(Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;

    Element *el = new Element;    
    Network::byte_t *tmpptr;
    boost::uint16_t length;

    if (in == 0) {
        log_error(_("AMF body input data is NULL"));
        return 0;
    }

    tmpptr = in;
    
    // All elements look like this:
    // the first two bytes is the length of name of the element
    // Then the next bytes are the element name
    // After the element name there is a type byte. If it's a Number type, then
    // 8 bytes are read If it's a String type, then there is a count of
    // characters, then the string value    
    
    // Get the type of the element, which is a single byte.
    // This type casting looks like a stupid mistake, but it's
    // mostly to make valgrind shut up, as it has a tendency to
    // complain about legit code when it comes to all this byte
    // manipulation stuff.
    char c = *(reinterpret_cast<char *>(tmpptr));
    Element::amf0_type_e type = static_cast<Element::amf0_type_e>(c);
    tmpptr++;                        // skip the header byte

    AMF amf_obj;
    switch (type) {
      case Element::NUMBER_AMF0:
	  el->makeNumber(tmpptr);
	  tmpptr += AMF0_NUMBER_SIZE; // all numbers are 8 bit big endian
	  break;
      case Element::BOOLEAN_AMF0:
	  el->makeBoolean(tmpptr);
	  tmpptr += sizeof(boost::uint16_t); // although a bool is one byte, it's stored as a short
	  break;
      case Element::STRING_AMF0:
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
	      el->setType(Element::STRING_AMF0);
	  };
	  break;
      case Element::OBJECT_AMF0:
	  el->makeObject();
	  Element *child;
	  do {
	      child = amf_obj.extractProperty(tmpptr);
	      tmpptr += amf_obj.totalsize() - 1;
	      el->addProperty(child);
	  } while (*tmpptr != Element::OBJECT_END_AMF0);
	  break;
      case Element::MOVIECLIP_AMF0:
      case Element::NULL_AMF0:
      case Element::UNDEFINED_AMF0:
      case Element::REFERENCE_AMF0:
      case Element::ECMA_ARRAY_AMF0:
      case Element::OBJECT_END_AMF0:
      case Element::STRICT_ARRAY_AMF0:
      case Element::DATE_AMF0:
      case Element::LONG_STRING_AMF0:
      case Element::UNSUPPORTED_AMF0:
      case Element::RECORD_SET_AMF0:
      case Element::XML_OBJECT_AMF0:
      case Element::TYPED_OBJECT_AMF0:
      default:
//	  log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)type);
	  return 0;
    }
    
    // Calculate the offset for the next read
    _totalsize = (tmpptr - in) + 1;

    return el;
}

Element *
AMF::extractProperty(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    return extractProperty(buf->reference());
}

Element *
AMF::extractProperty(Network::byte_t *in)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *tmpptr = in;
    boost::uint16_t length = 0;
    Network::byte_t len = 0;

//    length = *(reinterpret_cast<boost::uint16_t *>(tmpptr));
    length = tmpptr[1];
//    swapBytes(&length, sizeof(boost::uint16_t));
    tmpptr += sizeof(boost::uint16_t);
    
    if (length <= 0) {
//	if (*(in+2) == Element::OBJECT_END) {
	    log_debug(_("End of Object definition"));
//            el->setType(Element::OBJECT_END);
//             return el;
//         }
//	delete el;
	return 0;
    }    
    
    Element *el = new Element;
    // get the name of the element, the length of which we just decoded
    if (length > 0) {
//        log_debug(_("AMF element length is: %d"), length);
        el->setName(tmpptr, length);
//	log_debug(_("AMF element name is: %s"), el->getName());
	tmpptr += length;
    }
    
    // get the type of the element, which is a single byte.
    char c = *(reinterpret_cast<char *>(tmpptr));
    Element::amf0_type_e type = static_cast<Element::amf0_type_e>(c);
    tmpptr++;
    if (type != Element::TYPED_OBJECT_AMF0) {
        log_debug(_("AMF type is: %s"), amftype_str[(int)type]);
	el->setType(type);
    }

    switch (type) {
      case Element::NUMBER_AMF0:
      {
	  double num = *reinterpret_cast<const double*>(tmpptr);
          swapBytes(&num, AMF0_NUMBER_SIZE);
	  el->makeNumber(num);
          tmpptr += AMF0_NUMBER_SIZE;
	  break;
      }
      case Element::BOOLEAN_AMF0:
      {
	  bool sheet = *(reinterpret_cast<bool *>(tmpptr));
          el->makeBoolean(sheet);
	  tmpptr += 1;
	  break;
      }
      case Element::STRING_AMF0:
	  // extractString returns a printable char *. First 2 bytes for the length,
	  // and then read the string, which is NOT NULL terminated.
	  length = *reinterpret_cast<boost::uint16_t *>(tmpptr);
	  swapBytes(&length, sizeof(boost::uint16_t));
	  tmpptr += sizeof(boost::uint16_t);
	  if (length > 0) {
	      el->makeString(tmpptr, length);
	      tmpptr += length;
	  }
	  if (length == 0) {
//	      log_debug("NullString");
	      el->makeNullString();
	  }
//  	  log_debug(_("Property \"%s\" is: %s"), el->getName(), el->to_string());
          break;
      case Element::OBJECT_AMF0:
  	  while (*(tmpptr++) != Element::OBJECT_END_AMF0) {
//	      log_debug("Looking for end of object...");
  	  }
	  break;
      case Element::MOVIECLIP_AMF0:
      case Element::NULL_AMF0:
	  el->makeUndefined();
	  break;
      case Element::UNDEFINED_AMF0:
	  el->makeUndefined();
          break;
      case Element::REFERENCE_AMF0:
      case Element::ECMA_ARRAY_AMF0:
			// FIXME this shouldn't fall thru
      case Element::OBJECT_END_AMF0:
//          log_debug(_("End of Object definition"));
	  el->makeObjectEnd();
          break;
      case Element::TYPED_OBJECT_AMF0:
	  el->makeTypedObject(tmpptr, 0);
          break;
      case Element::STRICT_ARRAY_AMF0:
      case Element::DATE_AMF0:
	  el->makeDate(tmpptr);
          break;
      case Element::LONG_STRING_AMF0:
      case Element::UNSUPPORTED_AMF0:
      case Element::RECORD_SET_AMF0:
      case Element::XML_OBJECT_AMF0:
      default:
          log_unimpl(_("amf0_type_e of value: %x"), (int)type);
	  delete el;
	  return 0;
    }

    // Calculate the offset for the next read
    _totalsize = (tmpptr - in) + 1;
//    log_debug("Total number of bytes read from byte stream is: %d", _totalsize);
    
    return el;
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
