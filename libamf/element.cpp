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
#include <cmath>
#include <climits>

#include "buffer.h"
#include "log.h"
#include "amf.h"
#include "amfutf8.h"
#include "element.h"
#include <boost/cstdint.hpp> // for boost::?int??_t

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
    "TypedObject",
    "Varible (gnash)",
    "Function (gnash)"
};

Element::Element()
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
}


Element::~Element()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	delete _buffer;
    }
    for (size_t i=0; i< _children.size(); i++) {
	delete _children[i];
    }
    if (_name) {
	delete[] _name;
    }
}

Element::Element(Network::byte_t *indata) 
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

Element::Element(double indata)
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

// Element(vector<double> &indata)
// {
//     GNASH_REPORT_FUNCTION;
//     init(indata);
// }

Element::Element(const string &indata)
    : _name(),
      _buffer(0),
    _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

Element::Element(const string &name, const string &indata)
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(name, indata);
}

Element::Element(const string &name, bool indata)
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(name, indata);
}

Element::Element(bool indata)
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

// Create a function block for AMF
Element::Element(bool flag, double unknown1, double unknown2,
		 const string &methodname)
    : _name(),
      _buffer(0),
      _type(Element::NOTYPE)
{
    GNASH_REPORT_FUNCTION;
    init(flag, unknown1, unknown2, methodname);
}

Element &
Element::init(bool flag, double unknown1, double unknown2,
	      const string &methodname)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::FUNCTION;
    if (methodname.size()) {
	setName(methodname);
    }

    // Build up the children for the function block
    Element *el = new Element(flag);
    _children.push_back(el);
    
    el = new Element(unknown1);
    _children.push_back(el);
    
    el = new Element(unknown2);
    _children.push_back(el);
    
    el = new Element(methodname);
    _children.push_back(el);
    
    _buffer = new Buffer(3
	+ ((AMF_HEADER_SIZE + AMF_NUMBER_SIZE) * 2)
	       + methodname.size() + AMF_HEADER_SIZE);
//     memcpy(_data, &indata, _length);
    return *this;
}

Element &
Element::init(double indata)
{
//    GNASH_REPORT_FUNCTION;
    return init("", indata);
}

Element &
Element::init(const string &name, double num)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NUMBER;
    if (name.size()) {
        setName(name);
    }
    if (_buffer == 0) {
	_buffer = new Buffer(AMF_NUMBER_SIZE);
    } else {
	_buffer->resize(AMF_NUMBER_SIZE);
    }
    _buffer->copy(num);
    
    return *this;
}

Element &
Element::init(const string &indata)
{
//    GNASH_REPORT_FUNCTION;
    return init("", indata);
}

Element &
Element::init(const string &name, const string &str)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::STRING;
    if (name.size()) {
        setName(name);
    }
    if (_buffer == 0) {
	_buffer = new Buffer(str.size());
    } else {
	_buffer->resize(str.size());
    }
    _buffer->copy(str);
    
    return *this;
}

Element &
Element::init(bool indata)
{
//    GNASH_REPORT_FUNCTION;
    return init("", indata);
}

Element &
Element::init(const string &name, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::BOOLEAN;
    if (name.size()) {
        setName(name);
    }
    if (_buffer == 0) {
	_buffer = new Buffer(sizeof(bool));
    } else {
	_buffer->resize(sizeof(bool));
    }
    _buffer->append(flag);
    
    return *this;
}

void
Element::clear()
{
//    GNASH_REPORT_FUNCTION;
    if (_name) {
	delete [] _name;
    }
    if (_buffer) {
	delete _buffer;
    }
}

Network::byte_t *
Element::getData()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->reference();
    }
    return 0;
};

boost::uint16_t
Element::getLength()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->size();
    }
    return 0;
};

double
Element::to_number()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<double *>(_buffer->reference()));
    }
//    return ::nan("NaN");
    return -1.0;
}

const char *
Element::to_string()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	if (_buffer->size() > 0) {
	    return reinterpret_cast<const char *>(_buffer->reference());
	}
	return "NULL";
    }
    return 0;
};

bool
Element::to_bool()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<bool *>(_buffer->reference()));
    }
    return false;
};

void *
Element::to_reference()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return reinterpret_cast<void *>(_buffer->reference());
    }
    return 0;
};

// Test to see if Elements are the same
bool
Element::operator==(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    return operator==(&el);
}

bool
Element::operator==(Element *el)
{
//    GNASH_REPORT_FUNCTION;
    int count = 0;
    if (_name) {
	if (strcmp(_name, el->getName()) == 0) {
	    count++;
	}
    } else {
	if (el->getName() == 0) {
	    count++;
	}
    }
    if (_buffer) {
	if (_buffer == el->getBuffer()) {
	    count++;
	}
    } else {
	if (el->getBuffer() == 0) {
	    count++;
	}
    }

    if (_type == el->getType()) {
	count++;
    }

    // FIXME: make this test more exhaustive
    if (_children.size() == el->childrenSize()) {
	count++;
    }

    if (count == 4) {
	return true;
    }
    return false;;
}

bool
Element::operator==(bool x)
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	_buffer->append(x);
    }
    return false;
};

Buffer *
Element::encode()
{
    GNASH_REPORT_FUNCTION;
    Buffer *buf = 0;
    if (_type == Element::OBJECT) {
	// FIXME: we probably want a better size, to avoid the other
	// appends from having to resize and copy the data all the time.
	buf = new Buffer(AMF_HEADER_SIZE);
//	buf->clear();
	buf->append(Element::OBJECT);
	size_t length = getNameSize();
	boost::uint16_t enclength = length;
	swapBytes(&enclength, 2);
	buf->append(enclength);
	
	string name = _name;
	if (name.size() > 0) {
	    buf->append(name);
	}

	for (size_t i=0; i<_children.size(); i++) {
	    Buffer *partial = AMF::encodeElement(_children[i]);
//	    log_debug("Encoded partial size is %d", partial->size());
	    if (partial) {
		buf->append(partial);
		delete partial;
	    } else {
		break;
	    }
	}
	buf->append(TERMINATOR);
	_buffer = buf;
	return buf;
    } else {
	return AMF::encodeElement(this);
    }
    
    return 0;
}

Element *
Element::operator[](int index)
{
    GNASH_REPORT_FUNCTION;
    if (index <= _children.size()) {
	return _children[index];
    }
    
    return 0;
};

Element &
Element::operator=(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    return operator=(&el);
}

Element &
Element::operator=(Element *el)
{
//    GNASH_REPORT_FUNCTION;
    _type = el->getType();
    if (el->getNameSize()) {
        _name = el->getName();
    }
    _buffer = new Buffer(el->getLength());
    _buffer->copy(el->getData(), el->getLength());
    return *this;
}

/// \brief Fill an element with data
///
/// All Numbers are 64 bit, big-endian (network byte order) entities.
///
/// All strings are in multibyte format, which is to say, probably
/// normal ASCII. It may be that these need to be converted to wide
/// characters, but for now we just leave them as standard multibyte
/// characters.
Element &
Element::makeString(Network::byte_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING;
    // Make room for an additional NULL terminator
    check_buffer(size+1);
    _buffer->copy(data, size);
    
    // Unlike other buffers, people like to print strings, so we must add
    // a NULL terminator to the string. When encoding, we are careful to
    // to adjust the byte count down by one, as the NULL terminator doesn't
    // get written.
    *(_buffer->end() - 1) = 0;

    return *this;
}

// A Null string is a string with no length. The data is only one byte, which
// always has the value of zero of course.
Element &
Element::makeNullString()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING;
    check_buffer(sizeof(Network::byte_t));
    *(_buffer->reference()) = 0;
    return *this;
}

Element &
Element::makeString(const char *str, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(const_cast<char *>(str));
    return makeString(ptr, size);
}

Element &
Element::makeString(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    return makeString(str.c_str(), str.size());
}

Element &
Element::makeString(const string &name, const string &str)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }    
    return makeString(str.c_str(), str.size());
}

Element &
Element::makeNumber(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    double num = *reinterpret_cast<const double*>(data);
    _type = Element::NUMBER;
    check_buffer(AMF_NUMBER_SIZE);
    _buffer->copy(num);
    
    return *this;
}

Element &
Element::makeNumber(double num)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NUMBER;
    check_buffer(AMF_NUMBER_SIZE);
    _buffer->copy(num);

    return *this;
}

Element &
Element::makeNumber(const string &name, double num)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeNumber(num);
}

Element &
Element::makeBoolean(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::BOOLEAN;
    check_buffer(sizeof(bool));
    _buffer->append(flag);
    return *this;
}

Element &
Element::makeBoolean(const string &name, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeBoolean(flag);
}

Element &
Element::makeBoolean(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    bool flag = *reinterpret_cast<const bool*>(data);
    
    return makeBoolean(flag);
}

Element &
Element::makeUndefined()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::UNDEFINED;
    return *this;
}

Element &
Element::makeUndefined(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeUndefined();
}

Element &
Element::makeNull()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NULL_VALUE;
    check_buffer(sizeof(Network::byte_t));
    *(_buffer->reference()) = 0;
    return *this;
}

Element &
Element::makeNull(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;    
    if (name.size()) {
        setName(name);
    }
    return makeNull();
}

Element &
Element::makeObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    _type = OBJECT;
    return *this;
}

Element &
Element::makeObject(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeObjectEnd()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT_END;
    return *this;
}

Element &
Element::makeXMLObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::XML_OBJECT;    
    if (name.size()) {
        setName(name);
    }
    return *this;
}

Element &
Element::makeXMLObject(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::XML_OBJECT;
    check_buffer(size);
    _buffer->copy(indata, size);
    
    return *this;
}

Element &
Element::makeTypedObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT;    
    if (name.size()) {
        setName(name);
    }
    return *this;
}

Element &
Element::makeTypedObject(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeReference()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE;
    return *this;
}

Element &
Element::makeReference(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeMovieClip()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP;
    return *this;
}

Element &
Element::makeMovieClip(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;    
}

Element &
Element::makeECMAArray()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY;
    return *this;
}

Element &
Element::makeECMAArray(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;    
}

Element &
Element::makeUnsupported()
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::UNSUPPORTED;
    return *this;
}

Element &
Element::makeUnsupported(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::UNSUPPORTED;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeLongString()
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::LONG_STRING;
    return *this;
}

Element &
Element::makeLongString(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::LONG_STRING;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeRecordSet()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::RECORD_SET;
    return *this;
}

Element &
Element::makeDate(Network::byte_t *date)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::DATE;
    size_t size = sizeof(long);
    check_buffer(size);
    _buffer->copy(date, sizeof(long));
    return makeNumber(date);
}

Element &
Element::makeStrictArray()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRICT_ARRAY;
    return *this;
}

Element &
Element::makeStrictArray(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::STRICT_ARRAY;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

size_t
Element::getNameSize()
{
//    GNASH_REPORT_FUNCTION;
    if (_name) {
	return strlen(_name);
    }
    return 0;
}

void
Element::setName(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    _name = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), _name);
    *(_name + str.size()) = 0;
}

void
Element::setName(Network::byte_t *name, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if ((size > 0) && (name != 0)) {
	if (isascii(*name)) {
	    _name = new char[size+1];
	    std::copy(name, name+size, _name);
	    *(_name + size) = 0;
	} else {
	    log_debug("Got unprintable characters for the element name!");
	}
    }
}

// check the Buffer to make sure it's had memory allocated.
void
Element::check_buffer(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer == 0) {
	_buffer = new Buffer(size);
    } else {
	if (_buffer->size() != size) {
	    _buffer->resize(size);
	}
    }
}

void
Element::dump()
{
//    GNASH_REPORT_FUNCTION;
    
    if (_name) {
 	cerr << "AMF object name: " << _name << ", length is " << getLength() << endl;
    }

    cerr << astype_str[_type] << ": ";

    switch (_type) {
      case Element::NOTYPE:
	  break;
      case Element::NUMBER:
	  cerr << to_number() << endl;
	  break;
      case Element::BOOLEAN:
	  cerr << (to_bool() ? "true" : "false") << endl;
	  break;
      case Element::STRING:
	  cerr << "(" << getLength() << " bytes): ";
	  if (getLength() > 0) {
	      cerr << "\t\"" << to_string() << "\"" << endl;
	  } else {
	      cerr << endl;
	  }
	  break;
      case Element::OBJECT:
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
//	  cerr << "AMF data is: 0x" << hexify(_data, _length, false) << endl;
	  log_debug("FIXME: got a typed object!");
	  break;
      case Element::VARIABLE:
      case Element::FUNCTION:
 	  cerr << "# of children in object: " << _children.size() << endl;
	  for (size_t i=0; i< _children.size(); i++) {
	      _children[i]->dump();
	  }
	  break;
      default:
//	  log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)_type);
	  break;
    }
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
