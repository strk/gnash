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
#include <boost/shared_ptr.hpp>

#include "buffer.h"
#include "log.h"
#include "amf.h"
#include "amfutf8.h"
#include "utility.h"
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
    "AMF3 Data"
};

Element::Element()
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
}


Element::~Element()
{
//    GNASH_REPORT_FUNCTION;
    for (size_t i=0; i< _properties.size(); i++) {
	delete _properties[i];
    }
    // FIXME: for some odd reason, on rare occasions deleting this buffer
    // makes valgrind complain. It looks like memory corruption caused by something
    // else, but neither valgrind nor GDB can find it. We could always not delete
    // the buffer to keep valgrind happy, but then we leak memory. As the problem
    // appears to be that _buffer has a bogus address that doesn't match any allocated
    // Element, we assume this is a bug in our test case, but add comment here to be
    // paranoid.
//    delete _buffer;
    delete[] _name;
}

// Each Element has two main components, the optional name, and a value. All the properties
// of an ActionScript class have the name set to a non null value, and without a name, the
// Element just holds the lowest level AMF types.
Element::Element(double indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeNumber(indata);
}

Element::Element(const string &name, double num)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeNumber(name, num);
}


// Element(vector<double> &indata)
// {
//     GNASH_REPORT_FUNCTION;
//     init(indata);
// }

Element::Element(const string &indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeString(indata);
}

Element::Element(const char *indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeString(indata);
}

Element::Element(const string &name, const string &indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeString(name, indata);
}

Element::Element(bool indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeBoolean(indata);
}

Element::Element(const string &name, bool indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeBoolean(name, indata);
}

// Create a function block for AMF
Element::Element(bool flag, double unknown1, double unknown2,
		 const string &methodname)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("Can't create remote function calls yet");
}

#if 0
Element &
Element::init(bool flag, double unknown1, double unknown2,
	      const string &methodname)
{
//    GNASH_REPORT_FUNCTION;
//    _type = Element::FUNCTION_AMF0;
    if (methodname.size()) {
	setName(methodname);
    }

    // Build up the properties for the function block
    Element *el = new Element(flag);
    _properties.push_back(el);
    
    el = new Element(unknown1);
    _properties.push_back(el);
    
    el = new Element(unknown2);
    _properties.push_back(el);
    
    el = new Element(methodname);
    _properties.push_back(el);
    
    _buffer = new Buffer(3
	+ ((AMF_HEADER_SIZE + AMF0_NUMBER_SIZE) * 2)
	       + methodname.size() + AMF_HEADER_SIZE);
//     memcpy(_data, &indata, _length);
    return *this;
}
#endif

void
Element::clear()
{
//    GNASH_REPORT_FUNCTION;
	delete[] _name;
	_name = 0;
	_buffer.reset();
}

size_t
Element::getDataSize() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->size();
    }
    return 0;
};

double
Element::to_number() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<double *>(_buffer->reference()));
    }
//    return ::nan("NaN");
    return -1.0;
}

const char *
Element::to_string() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	if (_buffer->size() > 0) {
#if 0
	    char *foo = new char[_buffer->size() + 1];
	    memset(foo, 0, _buffer->size() + 1);
	    memcpy(foo, _buffer->reference(), _buffer->size());
	    return foo;
#else
	    return reinterpret_cast<const char *>(_buffer->reference());
#endif
	}
	return "NULL";
    }
    return 0;
};

bool
Element::to_bool() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<bool *>(_buffer->reference()));
    }
    return false;
};

gnash::Network::byte_t *
Element::to_reference()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->reference();
    }
    return 0;
};

// Test to see if Elements are the same
bool
Element::operator==(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    int count = 0;

    // See if the names match
    if (_name) {
	if (strcmp(_name, el.getName()) == 0) {
	    count++;
	}
    } else {
	if (el.getNameSize() == 0) {
	    count++;
	}
    }

    // See if the types match
    if (_type == el.getType()) {
	count++;
    }

    if (_buffer && el.getDataSize()) {
	if (memcmp(_buffer->reference(), el.to_reference(), _buffer->size()) == 0) {
	    count++;
	}
    } else {
	count++;
    }
    
    // FIXME: make this test more exhaustive
    if (_properties.size() == el.propertySize()) {
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
	*_buffer += x;
    }
    return false;
};

boost::shared_ptr<Buffer>
Element::encode()
{
//    GNASH_REPORT_FUNCTION;
    size_t size = 0;
    boost::shared_ptr<Buffer> buf;
    if (_type == Element::OBJECT_AMF0) {
	// FIXME: we probably want a better size, to avoid the other
	// appends from having to resize and copy the data all the time.
	for (size_t i=0; i<_properties.size(); i++) {
	    size += _properties[i]->getDataSize() + _properties[i]->getNameSize() + AMF_VAR_HEADER_SIZE;
	}
	buf.reset(new Buffer(size));
	buf->clear();		// FIXME: temporary, makes buffers cleaner in gdb.
	*buf = Element::OBJECT_AMF0;
	if (_name > 0) {
	    size_t length = getNameSize();
	    boost::uint16_t enclength = length;
	    swapBytes(&enclength, 2);
	    *buf += enclength;
	    string str = _name;
	    *buf += str;
	    Network::byte_t byte = static_cast<Network::byte_t>(0x5);
	    *buf += byte;
	}

	for (size_t i=0; i<_properties.size(); i++) {
	    boost::shared_ptr<Buffer> partial = AMF::encodeElement(_properties[i]);
//	    log_debug("Encoded partial size for is %d", partial->size());
//	    partial->dump();
	    if (partial) {
		*buf += partial;
//		delete partial;
	    } else {
		break;
	    }
	}
//	log_debug("FIXME: Terminating object");
	Network::byte_t pad = 0;
	*buf += pad;
	*buf += pad;
	*buf += TERMINATOR;
	_buffer = buf;
	return buf;
    } else {
	return AMF::encodeElement(this);
    }
    
    return buf;
}

Element *
Element::operator[](size_t index)
{
//    GNASH_REPORT_FUNCTION;
    if (index <= _properties.size()) {
	return _properties[index];
    }
    
    return 0;
};

Element &
Element::operator=(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    return operator=(&el);
//    GNASH_REPORT_FUNCTION;
    _type = el.getType();
    if (el.getNameSize()) {
        _name = strdup(el.getName());
    }
    _buffer.reset(new Buffer(el.getDataSize()));
    _buffer->copy(el.to_reference(), el.getDataSize());
    return *this;
}

Element &
Element::operator=(double num)
{
    return makeNumber(num);
}

Element &
Element::operator=(const string &str)
{
    return makeString(str);
}

Element &
Element::operator=(bool flag)
{
    return makeBoolean(flag);
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
    _type = Element::STRING_AMF0;

    // Make room for an additional NULL terminator
    check_buffer(size+1);
    _buffer->clear();		// FIXME: this could be a performance issue
    _buffer->copy(data, size);
    
    // Unlike other buffers, people like to print strings, so we must add
    // a NULL terminator to the string. When encoding, we are careful to
    // to adjust the byte count down by one, as the NULL terminator doesn't
    // get written.
//     *(_buffer->end() - 1) = 0;
    _buffer->setSize(size);
    return *this;
}

// A Null string is a string with no length. The data is only one byte, which
// always has the value of zero of course.
Element &
Element::makeNullString()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING_AMF0;
    check_buffer(sizeof(Network::byte_t));
    *(_buffer->reference()) = 0;
    return *this;
}

Element &
Element::makeString(const char *str, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING_AMF0;
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
Element::makeNumber(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    return makeNumber(buf->reference());
}

Element &
Element::makeNumber(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    double num = *reinterpret_cast<const double*>(data);
    _type = Element::NUMBER_AMF0;
    check_buffer(AMF0_NUMBER_SIZE);
    *_buffer = num;
    
    return *this;
}

Element &
Element::makeNumber(double num)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NUMBER_AMF0;
    check_buffer(AMF0_NUMBER_SIZE);
    *_buffer = num;

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
Element::makeNumber(const std::string &name, gnash::Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    _type = Element::NUMBER_AMF0;
    check_buffer(AMF0_NUMBER_SIZE);
    *_buffer = data;
    return *this;
}

Element &
Element::makeBoolean(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::BOOLEAN_AMF0;
    check_buffer(sizeof(bool));
    *(_buffer->reference()) = flag;

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
    _type = Element::UNDEFINED_AMF0;
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

// a NULL amf Object consists of a single byte, which is the type
Element &
Element::makeNull()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NULL_AMF0;
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
Element::makeObject()
{
//    GNASH_REPORT_FUNCTION;
    _type = OBJECT_AMF0;
    return *this;
}

Element &
Element::makeObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeObject();
}

Element &
Element::makeObject(const std::string &name, std::vector<Element *> &data)
{
//    GNASH_REPORT_FUNCTION;
    _type = OBJECT_AMF0;
    if (name.size()) {
        setName(name);
    }
    return makeObject(data);
}

Element &
Element::makeObject(std::vector<Element *> &data)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT_AMF0;
    vector<amf::Element *>::const_iterator ait;
    for (ait = data.begin(); ait != data.end(); ait++) {
	amf::Element *el = (*(ait));
	addProperty(el);
//	el->dump(os);
    }
}

Element &
Element::makeXMLObject()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}

Element &
Element::makeXMLObject(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    makeString(data);
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}

Element &
Element::makeXMLObject(const string &name, const string &data)
{
//    GNASH_REPORT_FUNCTION;
    makeXMLObject(name, data);
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}

Element &
Element::makeObjectEnd()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT_END_AMF0;
    return *this;
}


Element &
Element::makeTypedObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT_AMF0;  
    if (name.size()) {
        setName(name);
    }
    return *this;
}

Element &
Element::makeTypedObject(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT_AMF0;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeReference()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE_AMF0;
    return *this;
}

Element &
Element::makeReference(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE_AMF0;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeMovieClip()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP_AMF0;
    return *this;
}

Element &
Element::makeMovieClip(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP_AMF0;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;    
}

Element &
Element::makeECMAArray()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY_AMF0;
    return *this;
}

Element &
Element::makeECMAArray(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeECMAArray();
}

Element &
Element::makeECMAArray(const std::string &name, std::vector<Element *> &data)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY_AMF0;
    makeObject(name, data);
    _type = Element::ECMA_ARRAY_AMF0;
    return *this;
}

Element &
Element::makeECMAArray(std::vector<Element *> &data)
{
//    GNASH_REPORT_FUNCTION;
    makeObject(data);
    _type = Element::ECMA_ARRAY_AMF0;
    return *this;
}

Element &
Element::makeStrictArray()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRICT_ARRAY_AMF0;
    return *this;
}

Element &
Element::makeStrictArray(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeStrictArray();
}

Element &
Element::makeStrictArray(const std::string &name, std::vector<Element *> &data)
{
//    GNASH_REPORT_FUNCTION;
    makeObject(name, data);
    _type = Element::STRICT_ARRAY_AMF0;
    return *this;
}

Element &
Element::makeStrictArray(std::vector<Element *> &data)
{
//    GNASH_REPORT_FUNCTION;
    makeObject(data);
    _type = Element::STRICT_ARRAY_AMF0;
    return *this;
}

Element &
Element::makeUnsupported()
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::UNSUPPORTED_AMF0;
    return *this;
}

Element &
Element::makeUnsupported(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::UNSUPPORTED_AMF0;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeLongString()
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::LONG_STRING_AMF0;
    return *this;
}

Element &
Element::makeLongString(Network::byte_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::LONG_STRING_AMF0;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;
}

Element &
Element::makeRecordSet()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::RECORD_SET_AMF0;
    return *this;
}

Element &
Element::makeDate(Network::byte_t *date)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::DATE_AMF0;
    size_t size = sizeof(long);
    check_buffer(size);
    _buffer->copy(date, sizeof(long));
    return makeNumber(date);
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

// The name is for properties, which in AMF land is a string, followed by the AMF
// element.
void
Element::setName(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    _name = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), _name);
    *(_name + str.size()) = 0;
}

void
Element::setName(const char *name, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(const_cast<char *>(name));
    return setName(ptr, size);
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
	    log_error("Got unprintable characters for the element name!");
	}
    }
}

// check the Buffer to make sure it's had memory allocated.
void
Element::check_buffer(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer == 0) {
	_buffer.reset(new Buffer(size));
    } else {
	if (_buffer->size() != size) {
	    _buffer->resize(size);
	}
    }
}

void
Element::dump(std::ostream& os) const
{
//    GNASH_REPORT_FUNCTION;
    
    if (_name) {
 	os << "AMF object name: " << _name << ", length is " << getDataSize() << endl;
    }

    os << astype_str[_type] << ": ";

    switch (_type) {
      case Element::NUMBER_AMF0:
	  os << to_number() << endl;
	  break;
      case Element::BOOLEAN_AMF0:
	  os << (to_bool() ? "true" : "false") << endl;
	  break;
      case Element::STRING_AMF0:
	  os << "(" << getDataSize() << " bytes): ";
	  if (getDataSize() > 0) {
	      cerr << "\t\"" << to_string() << "\"";
	  }
	  cerr << endl;
	  break;
      case Element::OBJECT_AMF0:
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
      case Element::AMF3_DATA:
	  if (getDataSize() != 0) {
	      log_debug("FIXME: got AMF3 data!");
	  }
//	  cerr << "AMF3 data is: 0x" << hexify(_data, _length, false) << endl;
	  break;
//       case Element::VARIABLE:
//       case Element::FUNCTION:
//  	  os << "# of properties in object: " << properties.size() << endl;
// 	  for (size_t i=0; i< properties.size(); i++) {
// 	      properties[i]->dump();
// 	  }
// 	  break;
      default:
//	  log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)_type);
	  break;
    }

//     if (_buffer) {
// 	_buffer->dump();
//     }

    if (_properties.size() > 0) {
	vector<amf::Element *>::const_iterator ait;
	os << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ait++) {
	    const amf::Element *el = (*(ait));
	    el->dump(os);
	}
    }
}

Element *
Element::findProperty(const std::string &name)
{
    if (_properties.size() > 0) {
	vector<amf::Element *>::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ait++) {
	    amf::Element *el = (*(ait));
	    if (el->getName() == name) {
		return el;
	    }
//	    el->dump();
	}
    }
    return 0;
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
