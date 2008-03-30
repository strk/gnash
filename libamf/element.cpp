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
      _length(0),
      _data(0)
{
//    GNASH_REPORT_FUNCTION;
}


Element::~Element()
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
        delete [] _data;
    }
    for (size_t i=0; i< _children.size(); i++) {
	delete _children[i];
    }
}

Element::Element(Network::byte_t *indata)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

Element::Element(double indata)
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
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

Element::Element(const string &name, const string &indata)
{
    GNASH_REPORT_FUNCTION;
    init(name, indata);
}

Element::Element(const string &name, bool indata)
{
    GNASH_REPORT_FUNCTION;
    init(name, indata);
}

Element::Element(bool indata)
{
    GNASH_REPORT_FUNCTION;
    init(indata);
}

// Create a function block for AMF
Element::Element(bool flag, double unknown1, double unknown2,
		 const string &methodname)
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
	_name = methodname;
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
    
    _length = 3
	+ ((AMF_HEADER_SIZE + AMF_NUMBER_SIZE) * 2)
	+ methodname.size() + AMF_HEADER_SIZE;
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
Element::init(const string &name, double indata)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NUMBER;
    if (name.size()) {
        _name = name;
    }
    _length = AMF_NUMBER_SIZE;
    _data = reinterpret_cast<Network::byte_t *>(new char[sizeof(double)]);
    memcpy(_data, &indata, _length);
    return *this;
}

Element &
Element::init(const string &indata)
{
//    GNASH_REPORT_FUNCTION;
    return init("", indata);
}

Element &
Element::init(const string &name, const string &indata)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::STRING;
    if (name.size()) {
        _name = name;
    }
    _length = indata.size();
    // add a byte for a NULL string terminator byte.
    _data = new Network::byte_t[indata.size() + 1];
    memset(_data, 0, indata.size() + 1);
    memcpy(_data, indata.c_str(), indata.size());
    return *this;
}

Element &
Element::init(bool indata)
{
//    GNASH_REPORT_FUNCTION;
    return init("", indata);
}

Element &
Element::init(const string &name, bool indata)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::BOOLEAN;
    if (name.size()) {
        _name = name;
    }
    _length = 1;
    _data = new Network::byte_t[1];
    *_data = indata;
    return *this;
}

void
Element::clear()
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
        delete[] _data;
        _data = 0;
    }
    if (_name.size()) {
	_name.clear();
    }
    _length = 0;
    _type = Element::NOTYPE;
    
}

double
Element::to_number()
{
    if (_data) {
	return *(reinterpret_cast<double *>(_data));
    }
//    return ::nan("NaN");
    return -1.0;
}

const char *
Element::to_string()
{
    return reinterpret_cast<const char *>(_data);
};

bool
Element::to_bool()
{
    if (_data) {
	return *(reinterpret_cast<bool *>(_data));
    }
    return false;
};

void *
Element::to_reference()
{
    return reinterpret_cast<void *>(_data);
};

Element &
Element::operator=(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    _type = el.getType();
    _length = el.getLength();
    if (el.getName().size()) {
        _name = el.getName();
    }
    _data = new Network::byte_t[_length + 1];
    memcpy(_data, el.getData(), _length);
    
    return *this;
}
/// \brief Extract an AMF element from the byte stream
///
/// All Numbers are 64 bit, big-endian (network byte order) entities.
///
/// All strings are in multibyte format, which is to say, probably
/// normal ASCII. It may be that these need to be converted to wide
/// characters, but for now we just leave them as standard multibyte
/// characters.
Network::byte_t *
Element::init(Network::byte_t *indata)
{
//    GNASH_REPORT_FUNCTION;

    Network::byte_t *ptr = indata;
    // Extract the type
    _type = (Element::astype_e)((*ptr++) & 0xff);
    // For doubles, the length value is never set, but we might as
    // well put in a legit value anyway.
    _length = AMF_NUMBER_SIZE;
    _data = new Network::byte_t[AMF_NUMBER_SIZE + 1];
    memset(_data, 0, AMF_NUMBER_SIZE + 1);
    memcpy(_data, &indata, AMF_NUMBER_SIZE);

    return indata + AMF_NUMBER_SIZE;
}

Element &
Element::makeString(Network::byte_t *data, int size)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::STRING;
    _length = size;
    _data = new Network::byte_t[size+1];
    memset(_data, 0, size+1);
    memcpy(_data, data, size);
    return *this;
}

Element &
Element::makeString(const string &name, const string &indata)
{
    GNASH_REPORT_FUNCTION;
    _type = Element::STRING;
    if (name.size()) {
        _name = name;
    }
    _length = indata.size();
    // add a byte for a NULL string terminator byte.
    _data = new Network::byte_t[indata.size() + 1];
    memcpy(_data, indata.c_str(),_length);
    *(_data + _length) = 0;	// terminate the string for printing
    return *this;
}

Element &
Element::makeNumber(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::NUMBER;
    _length = amf::AMF_NUMBER_SIZE;
    _data = new Network::byte_t[amf::AMF_NUMBER_SIZE];
    memcpy(_data, data, amf::AMF_NUMBER_SIZE);
    return *this;
}

Element &
Element::makeNumber(const string &name, double data)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        _name = name;
    }    
    _type = Element::NUMBER;
    _length = amf::AMF_NUMBER_SIZE;
    _data = new Network::byte_t[amf::AMF_NUMBER_SIZE];
    memcpy(_data, &data, amf::AMF_NUMBER_SIZE);
//    std::copy(&data, &data + amf::AMF_NUMBER_SIZE, _data);
    return *this;
}

Element &
Element::makeBoolean(bool data)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::BOOLEAN;
    _length = 1;
    _data = new Network::byte_t[2];
    memset(_data, 0, 2);
    _data[1]= data;
    return *this;
}

Element &
Element::makeBoolean(const string &name, bool data)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        _name = name;
    }
    _type = Element::BOOLEAN;
    _length = 1;
    _data = new Network::byte_t[2];
    memset(_data, 0, 2);
    _data[1]= data;
    return *this;
}

Element &
Element::makeBoolean(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::BOOLEAN;
    _length = 1;
    _data = new Network::byte_t[2];
    memset(_data, 0, 2);
    memcpy(_data, data+1, 1);
    return *this;
}

Element &
Element::makeUndefined()
{
    return makeUndefined("");
}

Element &
Element::makeUndefined(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::UNDEFINED;
    if (name.size()) {
        _name = name;
    }
    _length = 0;
    _data = 0;
    return *this;
}

Element &
Element::makeNull()
{
//    GNASH_REPORT_FUNCTION;
    return makeNull("");
}

Element &
Element::makeNull(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::NULL_VALUE;
    if (name.size()) {
        _name = name;
    }
    _length = 0;
    _data = 0;
    return *this;
}

Element &
Element::makeObject(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::OBJECT;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);
    return *this;
}

Element &
Element::makeObjectEnd()
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::OBJECT_END;
    _length = 0;
    _data = 0;
    return *this;
}

Element &
Element::makeXMLObject(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::XML_OBJECT;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);
    return *this;
}

Element &
Element::makeTypedObject(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::TYPED_OBJECT;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);
    return *this;
}

Element &
Element::makeReference(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);    
    return *this;
}

Element &
Element::makeMovieClip(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);    
    return *this;    
}

Element &
Element::makeECMAArray(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);    
    return *this;    
}

Element &
Element::makeLongString(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::LONG_STRING;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);    

    return *this;
}

Element &
Element::makeRecordSet(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::RECORD_SET;
    
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);    

    return *this;
}

Element &
Element::makeDate(Network::byte_t *indata)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::DATE;
    _length = AMF_NUMBER_SIZE;
    _data = new Network::byte_t[AMF_NUMBER_SIZE + 1];
    memset(_data, 0, AMF_NUMBER_SIZE + 1);
    memcpy(_data, indata, AMF_NUMBER_SIZE);    

    return *this;
}

Element &
Element::makeStrictArray(Network::byte_t *indata, int size)
{
//    GNASH_REPORT_FUNCTION;
    
    _type = Element::STRICT_ARRAY;
    _length = size;
    _data = new Network::byte_t[size + 1];
    memset(_data, 0, size + 1);
    memcpy(_data, indata, size);    

    return *this;
}

void
Element::setName(Network::byte_t *name, int x)
{
//    GNASH_REPORT_FUNCTION;

    char *buf = new char[x+1];
    *(buf+x) = 0;			// terminate the string, we don't need to memcpy the whole thing
    std::copy(name, name+x, buf);
    _name = buf;
}

void
Element::dump()
{
//    GNASH_REPORT_FUNCTION;
    
    if (_name.size()) {
	cerr << "AMF object name: " << _name << ", length is " << _length << endl;
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
	  cerr << "(" << _length << " bytes): ";
	  if (_length > 0) {
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
