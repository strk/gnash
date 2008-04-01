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
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
//       _length(0),
//       _data(0)
{
//    GNASH_REPORT_FUNCTION;
}


Element::~Element()
{
//    GNASH_REPORT_FUNCTION;
//     if (_buffer) {
// 	if (_buffer->size() > 0) {
// 	    delete _buffer;
// 	}
//     }
    for (size_t i=0; i< _children.size(); i++) {
	delete _children[i];
    }
}

Element::Element(Network::byte_t *indata) 
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

Element::Element(double indata)
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
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
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

Element::Element(const string &name, const string &indata)
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
{
    GNASH_REPORT_FUNCTION;
    init(name, indata);
}

Element::Element(const string &name, bool indata)
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
{
    GNASH_REPORT_FUNCTION;
    init(name, indata);
}

Element::Element(bool indata)
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

// Create a function block for AMF
Element::Element(bool flag, double unknown1, double unknown2,
		 const string &methodname)
    : _type(Element::NOTYPE),
      _buffer(0),
      _name(0)
{
    GNASH_REPORT_FUNCTION;
    init(flag, unknown1, unknown2, methodname);
}

// Return the total size of the data plus the headers for byte math
size_t
Element::totalsize()
{
//    GNASH_REPORT_FUNCTION;
    size_t size = 0;
    
    if (_name == 0) {
	size = _buffer->size() + AMF_HEADER_SIZE;
    } else {
	size = _buffer->size() + AMF_VAR_HEADER_SIZE + strlen(_name);
    }
    
    return size;
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
    _buffer = new Buffer(AMF_NUMBER_SIZE);
//     _data = reinterpret_cast<Network::byte_t *>(new char[sizeof(double)]);
//     Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&indata);

    _buffer->append(num);
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
    _buffer = new Buffer(str.size());
    _buffer->append(str);
    
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
    _buffer = new Buffer(sizeof(bool));
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
    if (_buffer) {
	return *(reinterpret_cast<double *>(_buffer->reference()));
    }
//    return ::nan("NaN");
    return -1.0;
}

const char *
Element::to_string()
{
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
    if (_buffer) {
	return *(reinterpret_cast<bool *>(_buffer->reference()));
    }
    return false;
};

void *
Element::to_reference()
{
    if (_buffer) {
	return reinterpret_cast<void *>(_buffer->reference());
    }
    return 0;
};

bool
Element::operator==(bool x)
{
    if (_buffer) {
	_buffer->append(x);
    }
    return false;
};
Network::byte_t
Element::operator[](int x)
{
    if (_buffer) {
	return *_buffer->at(x);
    }
    return 0;
};


Element &
Element::operator=(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    _type = el.getType();
    if (el.getNameSize()) {
        _name = el.getName();
    }
    _buffer = new Buffer(el.getLength());
    _buffer = el.getBuffer();
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
    _buffer = new Buffer(size+1);
    _buffer->copy(data, size);
    // Unlike other buffers, people like to print strings, so we must add
    // a NULL terminator to the string. When encoding, we are careful to
    // to adjust the byte count down by one, as the NULL terminator doesn't
    // get written.
    *(_buffer->end()) = 0;
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
    _buffer = new Buffer(AMF_NUMBER_SIZE);
    _buffer->append(num);

    return *this;
}

Element &
Element::makeNumber(const string &name, double innum)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    Network::byte_t *num = reinterpret_cast<Network::byte_t *>(&innum);
    return makeNumber(num);
}

Element &
Element::makeBoolean(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::BOOLEAN;
    _buffer = new Buffer(sizeof(bool));
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
    Network::byte_t val = 0;
    _buffer = new Buffer(sizeof(Network::byte_t));
    _buffer->append(val);
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
    GNASH_REPORT_FUNCTION;
    setName(name);
    _type = OBJECT;
    return *this;
}

Element &
Element::makeObject(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeObjectEnd()
{
    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT_END;
    return *this;
}

Element &
Element::makeXMLObject(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::XML_OBJECT;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeTypedObject(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeReference(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeMovieClip(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;    
}

Element &
Element::makeECMAArray(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;    
}

Element &
Element::makeLongString(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;    
    _type = Element::LONG_STRING;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeRecordSet(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::RECORD_SET;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeDate(Network::byte_t *date)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::DATE;
    return makeNumber(date);
}

Element &
Element::makeStrictArray(Network::byte_t *indata, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
    _type = Element::STRICT_ARRAY;
    _buffer = new Buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

size_t
Element::getNameSize()
{
    if (_name) {
	return strlen(_name);
    }
    return 0;
}

void
Element::setName(const string &str)
{
    GNASH_REPORT_FUNCTION;
    _name = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), _name);
    *(_name + str.size()) = 0;
}

void
Element::setName(Network::byte_t *name, size_t x)
{
    GNASH_REPORT_FUNCTION;
    char *_name = new char[x+1];
    std::copy(name, name+x, _name);
    *(_name + x) = 0;
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
